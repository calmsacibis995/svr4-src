/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:ip_output.c	1.3"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */


/*
 * System V STREAMS TCP - Release 3.0 
 *
 * Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI) 
 * All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/errno.h>
#ifdef SYSV
#include <sys/cmn_err.h>
#endif SYSV
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>

#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_pcb.h>
#include <sys/dlpi.h>
#include <netinet/nihdr.h>
#include <netinet/ip_str.h>

/*extern void     bcopy();*/

extern struct ip_provider *prov_withdstaddr();
extern struct ip_provider *in_onnetof();
extern struct ip_provider provider[], *lastprov;
mblk_t         *dup_range();
mblk_t         *ip_insertoptions();

#define satosin(sa)	((struct sockaddr_in *) (sa))

/*
 * IP output.  This is the upper write service routine for ip. The packets in
 * the queue contain a skeletal IP header (as ipovly).  The queue elements
 * will be freed, but the options pointed to in the unit data request won't
 * be. 
 */
ipuwsrv(q)
	queue_t        *q;
{
	mblk_t         *bp;

	while (bp = getq(q)) {

		(void) ip_output(q, bp);

	}
}

ip_snduderr(q, addr, error)
queue_t *q;
struct in_addr addr;
int error;
{
	mblk_t *bp;
	struct N_uderror_ind *uderr;

	if (!(bp = allocb(sizeof(*uderr)+sizeof(addr),BPRI_HI)))
		return;
	bp->b_datap->db_type = M_PROTO;
	uderr = (struct N_uderror_ind *) bp->b_rptr;
	uderr->PRIM_type = N_UDERROR_IND;
	uderr->RA_length = sizeof(addr);
	uderr->RA_offset = sizeof(*uderr);
	uderr->ERROR_type = error;
	bp->b_wptr += sizeof(*uderr);
	bcopy(&addr,bp->b_wptr,sizeof(addr));
	bp->b_wptr += sizeof(addr);
	qreply(q, bp);
}

ip_output(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	int             flags;
	register struct ip *ip;
	register struct ip_provider *prov = NULL;
	int             len, hlen = sizeof(struct ip), off, error = 0;
	struct route    iproute, *ro = &iproute;
	struct in_addr *dst = &satosin(&iproute.ro_dst)->sin_addr;
	mblk_t         *bp1;
	queue_t        *qp;
	register struct ip_unitdata_req *ip_req;
	register        rtflags;

	ip_req = (struct ip_unitdata_req *) bp->b_rptr;
	if (ip_req->options)
		bp->b_cont = ip_insertoptions(bp->b_cont, ip_req->options, &hlen);
	ip = (struct ip *) bp->b_cont->b_rptr;
	flags = ip_req->flags;
	/*
	 * Fill in IP header. 
	 */
	if ((flags & IP_FORWARDING) == 0) {
		ip->ip_v = IPVERSION;
		ip->ip_off &= IP_DF;
		ip->ip_id = htons(ip_id++);
		ip->ip_hl = hlen >> 2;
		rtflags = SSF_SWITCH;
	} else {
		hlen = ip->ip_hl << 2;
		rtflags = SSF_SWITCH | SSF_REMOTE;
	}

	/*
	 * Route packet. 
	 */
	/*
	 * start by routing packets to us through loopback. 
	 */
	if (in_ouraddr(ip->ip_dst)) {
		for (prov = provider; prov <= lastprov; prov++) {
			if ((prov->if_flags & (IFF_LOOPBACK|IFF_UP)) ==
			(IFF_LOOPBACK|IFF_UP)) {
				break;
			}
		}
		if (prov > lastprov) {
			prov = NULL;
			*dst = satosin(&ip_req->route.ro_dst)->sin_addr;
		} else {
			*dst = ip->ip_dst;
		}
		ro->ro_rt = ip_req->route.ro_rt;
	} else {
		iproute = ip_req->route;
	}
	if (prov == NULL) {
		/*
		 * If there is a cached route, check that it is to the same
		 * destination and is still up.  If not, free it and try
		 * again. 
		 */
		if (ro->ro_rt && ((RT(ro->ro_rt)->rt_flags & RTF_UP) == 0 ||
				  dst->s_addr != ip->ip_dst.s_addr)) {
			RTFREE(ro->ro_rt);
			ro->ro_rt = (mblk_t *) 0;
		}
		if (ro->ro_rt == 0) {
			*dst = ip->ip_dst;
		}
		/*
		 * If routing to interface only, short circuit routing
		 * lookup. 
		 */
		if (flags & IP_ROUTETOIF) {
			prov = prov_withdstaddr(*dst);
			if (prov == 0)
				prov = in_onnetof(in_netof(ip->ip_dst));
			if (prov == 0) {
				error = ENETUNREACH;
				goto bad;
			}
		} else {
			if (ro->ro_rt == 0
			    && rtalloc(ro, rtflags) == RT_DEFER) {
				RTFREE(ro->ro_rt);
				ro->ro_rt = 0;
				sldefer(ip_output, q, bp);
				return (0);
			}
			if (ro->ro_rt == 0
			    || (prov = RT(ro->ro_rt)->rt_prov) == 0) {
				if (in_localaddr(ip->ip_dst))
					error = EHOSTUNREACH;
				else
					error = ENETUNREACH;
				goto bad;
			}
			RT(ro->ro_rt)->rt_use++;
			if (RT(ro->ro_rt)->rt_flags & (RTF_GATEWAY | RTF_HOST))
				*dst = satosin(&(RT(ro->ro_rt)->rt_gateway))->sin_addr;
		}
	}
	/*
	 * If this interface is overflowing, send a source quench to the
	 * appropriate customer.  Note that he may not be local, so we have
	 * to go the whole icmp route. 
	 */
	qp = prov->qbot;
	if (!canput(qp)) {
		icmp_error(ip, ICMP_SOURCEQUENCH, 0, qp, 0);
		goto bad;
	}

	/*
	 * If source address not specified yet, use address
	 * of outgoing interface.
	 */
	if (ip->ip_src.s_addr == INADDR_ANY)
		ip->ip_src = *PROV_INADDR(prov);

	/*
	 * Look for broadcast address and and verify user is allowed to send
	 * such a packet. 
	 */
	if (in_broadcast(*dst)) {
		if ((prov->if_flags & IFF_BROADCAST) == 0) {
			error = EADDRNOTAVAIL;
			goto bad;
		}
		if ((flags & IP_ALLOWBROADCAST) == 0) {
			error = EACCES;
			goto bad;
		}
		/* don't allow broadcast messages to be fragmented */
		if (ip->ip_len > prov->if_maxtu) {
			error = EMSGSIZE;
			goto bad;
		}
	}
	ip_req->dl_primitive = DL_UNITDATA_REQ;
	ip_req->dl_dest_addr_length = sizeof(struct in_addr);
	ip_req->dl_dest_addr_offset = sizeof(struct ip_unitdata_req)
		- sizeof(struct in_addr);
	ip_req->ip_addr = *dst;
	/*
	 * If small enough for interface, can just send directly. 
	 */
	if (ip->ip_len <= prov->if_maxtu) {
		padpckt(bp->b_cont, prov->if_mintu - ip->ip_len);
		ip->ip_len = htons((u_short) ip->ip_len);
		ip->ip_off = htons((u_short) ip->ip_off);
		ip->ip_sum = 0;
		ip->ip_sum = in_cksum(bp->b_cont, hlen);
		iplwput(qp, bp);
		goto done;
	}
	/*
	 * Too large for interface; fragment if possible. Must be able to put
	 * at least 8 bytes per fragment. 
	 */
	if (ip->ip_off & IP_DF) {
		error = EMSGSIZE;
		goto bad;
	}
	len = (prov->if_maxtu - hlen) & ~7;
	if (len < 8) {
		error = EMSGSIZE;
		goto bad;
	}
	/*
	 * Discard DL header from logical message for dup_range's sake. Loop
	 * through length of segment, make a copy of each part and output. 
	 */
	bp1 = bp->b_cont;
	if (!pullupmsg(bp1, hlen)) {
		error = ENOSR;
		goto bad;
	}
	bp1->b_rptr += sizeof(struct ip);

	for (off = 0; off < ip->ip_len - hlen; off += len) {
		mblk_t         *mh = allocb(hlen, BPRI_MED);
		mblk_t         *mh1;
		struct ip      *mhip;
		int		fhlen = hlen;

		if (mh == 0) {
			error = ENOSR;
			goto bad;
		}
		mhip = (struct ip *) mh->b_rptr;
		*mhip = *ip;
		if (hlen > sizeof(struct ip)) {
			fhlen = sizeof(struct ip) + ip_optcopy(ip, mhip, off);
			mhip->ip_hl = fhlen >> 2;
		}
		mh->b_wptr += fhlen;
		mhip->ip_off = (off >> 3) + (ip->ip_off & ~IP_MF);
		if (ip->ip_off & IP_MF)
			mhip->ip_off |= IP_MF;
		if (off + len >= ip->ip_len - hlen)
			len = mhip->ip_len = ip->ip_len - hlen - off;
		else {
			mhip->ip_len = len;
			mhip->ip_off |= IP_MF;
		}
		mhip->ip_len += fhlen;
		mhip->ip_len = htons((u_short) mhip->ip_len);
		mhip->ip_off = htons((u_short) mhip->ip_off);
		mhip->ip_sum = 0;
		mhip->ip_sum = in_cksum(mh, fhlen);
		mh->b_cont  = dup_range(bp1, off, len);
		if ( mh->b_cont == NULL ) {
			(void) freemsg(mh);
			error = ENOSR;	/* ??? */
			goto bad;
		}
		padpckt(mh, prov->if_mintu - ntohs((u_short) mhip->ip_len));
		if ( (mh1 = copyb(bp)) == NULL ) {
			(void) freemsg(mh);
			error = ENOSR;	/* ??? */
			goto bad;
		}
		mh1->b_cont = mh;
		iplwput(qp, mh1);
	}

bad:
	if (!(flags & IP_FORWARDING) && error)
		ip_snduderr(q, ip->ip_dst, error);
	freemsg(bp);
done:
	if ((flags & IP_ROUTETOIF) == 0 && ro->ro_rt)
		RTFREE(ro->ro_rt);
	return (error);
}

/* This procedure is used to extend queue space for the interface driver,
 * since we aren't able to configure the interface driver queue.
 */
iplwput(q, bp)
        queue_t *q;
        mblk_t  *bp;
{
        if (canput(q->q_next))
                putnext(q, bp);
        else
                putq(q, bp);
}

iplwsrv(q)
        queue_t *q;
{
        mblk_t  *bp;

        while (bp = getq(q))
                if (canput(q->q_next))
                        putnext(q, bp);
                else {
                        putbq(q, bp);
                        return;
                }
}

/*
 * Insert IP options into preformed packet. Adjust IP destination as required
 * for IP source routing, as indicated by a non-zero in_addr at the start of
 * the options. 
 */
mblk_t         *
ip_insertoptions(bp, opt, phlen)
	register mblk_t *bp;
	mblk_t         *opt;
	int            *phlen;
{
	register struct ipoption *p = (struct ipoption *) opt->b_rptr;
	register struct ip *ip = (struct ip *) bp->b_rptr;
	unsigned        optlen;
	register mblk_t *bp1;
	mblk_t         *bpsave;

	optlen = (opt->b_wptr - opt->b_rptr) - sizeof(p->ipopt_dst);
	if (p->ipopt_dst.s_addr)
		ip->ip_dst = p->ipopt_dst;
	if ((bp1 = allocb((int) optlen + sizeof(struct ip), BPRI_HI)) == NULL) {
		return (bp);
	}
	bcopy((caddr_t) ip, (char *) bp1->b_wptr, sizeof(struct ip));
	bp1->b_wptr += sizeof(struct ip);
	bcopy((caddr_t) p->ipopt_list, (char *) bp1->b_wptr, optlen);
	bp1->b_wptr += optlen;
	bp->b_rptr += sizeof(struct ip);
	if (bp->b_rptr >= bp->b_wptr) {
		bpsave = bp->b_cont;
		freeb(bp);
		bp = bpsave;
	}
	bp1->b_cont = bp;
	ip = (struct ip *) bp1->b_rptr;
	*phlen = sizeof(struct ip) + optlen;
	ip->ip_len += optlen;
	return (bp1);
}

/*
 * Copy options from ip to jp. If off is 0 all options are copied otherwise
 * copy selectively. 
 */
ip_optcopy(ip, jp, off)
	struct ip      *ip, *jp;
	int             off;
{
	register u_char *cp, *dp;
	int             opt, optlen, cnt;

	cp = (u_char *) (ip + 1);
	dp = (u_char *) (jp + 1);
	cnt = (ip->ip_hl << 2) - sizeof(struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else
			optlen = cp[IPOPT_OLEN];
		if (optlen > cnt)	/* XXX */
			optlen = cnt;	/* XXX */
		if (off == 0 || IPOPT_COPIED(opt)) {
			bcopy((caddr_t) cp, (caddr_t) dp, (unsigned) optlen);
			dp += optlen;
		}
	}
	for (optlen = dp - (u_char *) (jp + 1); optlen & 0x3; optlen++)
		*dp++ = IPOPT_EOL;
	return (optlen);
}

mblk_t         *
dup_range(bp, off, len)
	register mblk_t *bp;
	register int    off, len;
{
	register mblk_t *head = NULL;
	register mblk_t *oldbp = NULL;
	register mblk_t *newbp;
	register int    size;

	while (off > 0) {
		if (off < (bp->b_wptr - bp->b_rptr)) {
			break;
		}
		off -= (bp->b_wptr - bp->b_rptr);
		bp = bp->b_cont;
		if (bp == 0) {
#ifdef SYSV
			cmn_err(CE_PANIC, "dup_range");
#else
			panic ("dup_range");
#endif SYSV
		}
	}
	while (len) {
		size = MIN(len, bp->b_wptr - bp->b_rptr - off);
		if (!(newbp = dupb(bp))) {
			freemsg(head);
			return (0);
		}
		if (!head)
			head = newbp;
		else
			oldbp->b_cont = newbp;
		newbp->b_rptr += off;
		newbp->b_wptr = newbp->b_rptr + size;
		len -= size;
		bp = bp->b_cont;
		oldbp = newbp;
		off = 0;
		if (len != 0 && bp == 0) {
#ifdef SYSV
			cmn_err(CE_PANIC, "dup_range 2");
#else
			panic ("dup_range 2");
#endif SYSV
		}
	}
	return (head);
}

padpckt(bp, cnt)
	mblk_t         *bp;
	int             cnt;
{
	int             n;

	if (cnt <= 0)
		return;
	while (bp->b_cont)
		bp = bp->b_cont;
	n = MIN(bp->b_datap->db_lim - bp->b_wptr, cnt);
	cnt -= n;
	bp->b_wptr += n;
	if (cnt && (bp->b_cont = allocb(cnt, BPRI_MED)))
		bp->b_cont->b_wptr += cnt;
}
