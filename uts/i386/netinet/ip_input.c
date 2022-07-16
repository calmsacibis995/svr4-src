/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:ip_input.c	1.3"

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
#include <sys/systm.h>

#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>

#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/cmn_err.h>

#include <net/if.h>

#include <netinet/in.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_str.h>
#include <netinet/tcp.h>
#include <netinet/insrem.h>

#include <netinet/nihdr.h>
#include <sys/kmem.h>

extern int			ipcnt;
extern struct ip_provider	provider[], *lastprov, *prov_withaddr();
extern struct ip_pcb		ip_pcb[];
extern struct ip_provider	*in_onnetof();
extern unsigned char		ip_protox[];
extern int			in_interfaces;


#define satosin(sa)	((struct sockaddr_in *) (sa))

/*
 * We need to save the IP options in case a protocol wants to respond to an
 * incoming packet over the same route if the packet got here using IP source
 * routing.  This allows connection establishment and maintenance when the
 * remote end is on a network that is not known to us. 
 */
int             ip_nhops = 0;
static struct ip_srcrt {
	char            nop;	/* one NOP to align */
	char            srcopt[IPOPT_OFFSET + 1];	/* OPTVAL, OLEN and
							 * OFFSET */
	struct in_addr  route[MAX_IPOPTLEN];
}               ip_srcrt;

struct ipq      ipq;
int             iptimerid;
u_char          ipcksum = 1;
mblk_t         *ip_reass();
struct route    ipforward_rt;

/*
 * Initialize the fragmentation/reassembly structures.  Called from ipinit. 
 */

ipq_setup()
{
	ipq.next = &ipq;
	ipq.prev = &ipq;
}

int ipdprintf = 0;

/*
 * Ip input routine.  Checksum and byte swap header.  If fragmented try to
 * reassamble.  If complete and fragment queue exists, discard. Process
 * options.  Pass to next level. 
 */
ipintr(q)
	queue_t        *q;
{
	register mblk_t *bp, *fbp;
	register struct ip *ip;
	register struct ipq *fp;
	mblk_t         *bp0;
	int             hlen, i;
	queue_t        *qp;	/* pointer to the next queue upstream */
	struct N_unitdata_ind *hdr;
	struct ip_provider *prov, *prov1 = (struct ip_provider *) q->q_ptr;

	while (bp = getq(q)) {
		if (ipdprintf) {
			printf ("ipintr: got buf.\n");
		}
		/*
		 * Toss packets coming off providers marked "down". 
		 */
		if ((prov1->if_flags & IFF_UP) == 0) {
			if (ipdprintf) {
				printf ("ipintr: drop - interface down\n");
			}
			freemsg(bp);
			continue;
		}
		ipstat.ips_total++;
		bp0 = bp;	/* Drop link level header */
		bp = bp->b_cont;
		freeb(bp0);
		if (pullupmsg(bp, sizeof(struct ip)) == 0) {
			cmn_err(CE_WARN, "ipintr: pullupmsg failed\n");
			goto bad;
		}
		if ((bp->b_wptr - bp->b_rptr) < sizeof(struct ip)) {
			ipstat.ips_tooshort++;
			if (ipdprintf) 
				printf ("ipintr: too short\n");
			goto bad;
		}
		ip = (struct ip *) bp->b_rptr;
		hlen = ip->ip_hl << 2;
		if (hlen < sizeof(struct ip)) {	/* minimum header length */
			ipstat.ips_badhlen++;
			if (ipdprintf)
				printf ("ip_input: header too short\n");
			goto bad;
		}
		if (hlen > (bp->b_wptr - bp->b_rptr)) {
			if (pullupmsg(bp, hlen) == 0) {
				ipstat.ips_badhlen++;
				if (ipdprintf)
					printf ("ip_input: header too short 2\n");
				goto bad;
			}
			ip = (struct ip *) bp->b_rptr;
		}
		if (ipcksum)
			if (ip->ip_sum = in_cksum(bp, hlen)) {
				ipstat.ips_badsum++;
				if (ipdprintf)
					printf ("ip_input: bad header checksum\n");
				goto bad;
			}
		/*
		 * Convert fields to host representation. 
		 */
		ip->ip_len = ntohs((u_short) ip->ip_len);
		if (ip->ip_len < hlen) {
			ipstat.ips_badlen++;
			if (ipdprintf)
				printf ("ip_input: bad length\n");
			goto bad;
		}
		ip->ip_id = ntohs(ip->ip_id);
		ip->ip_off = ntohs((u_short) ip->ip_off);

		/*
		 * Check that the amount of data in the buffers is as at
		 * least much as the IP header would have us expect. Trim
		 * buffers if longer than we expect. Drop packet if shorter
		 * than we expect. 
		 */
		i = -(u_short) ip->ip_len;
		bp0 = bp;
		for (;;) {
			i += (bp->b_wptr - bp->b_rptr);
			if (bp->b_cont == 0)
				break;
			bp = bp->b_cont;
		}
		if (i != 0) {
			if (i < 0) {
				ipstat.ips_toosmall++;
				bp = bp0;
				if (ipdprintf)
					printf ("ip_input: too small\n");
				goto bad;
			}
			if (i <= (bp->b_wptr - bp->b_rptr))
				bp->b_wptr -= i;
			else
				adjmsg(bp0, -i);
		}
		bp = bp0;

		/*
		 * Process options and, if not destined for us, ship it on.
		 * ip_dooptions returns 1 when an error was detected (causing
		 * an icmp message to be sent and the original packet to be
		 * freed). 
		 */
		ip_nhops = 0;	/* for source routed packets */
		if (hlen > sizeof(struct ip) && ip_dooptions(bp, q))
			continue;

		/*
		 * Check our list of addresses, to see if the packet is for
		 * us. 
		 */
		for (prov = provider; prov <= lastprov; prov++) {
			if (PROV_INADDR(prov)->s_addr == ip->ip_dst.s_addr) {
				goto ours;
			}
			if (
#ifdef	DIRECTED_BROADCAST
			    prov1 == prov &&
#endif
			    (prov->if_flags & IFF_BROADCAST)) {
				u_long          t;

				if (satosin(&prov->if_broadaddr)->sin_addr.s_addr
				    == ip->ip_dst.s_addr)
					goto ours;
				if (ip->ip_dst.s_addr ==
				    prov->ia.ia_netbroadcast.s_addr)
					goto ours;
				/*
				 * Look for all-0's host part (old broadcast
				 * addr), either for subnet or net. 
				 */
				t = ntohl(ip->ip_dst.s_addr);
				if (t == prov->ia.ia_subnet)
					goto ours;
				if (t == prov->ia.ia_net)
					goto ours;
			}
		}
		if (ip->ip_dst.s_addr == (u_long) INADDR_BROADCAST)
			goto ours;
		if (ip->ip_dst.s_addr == INADDR_ANY)
			goto ours;

		/*
		 * Not for us; forward if possible and desirable. 
		 */
		ip_forward(q, bp);
		continue;

ours:
		/*
		** If offset or IP_MF are set, must reassemble.
		** Otherwise, nothing need be done.
		** (We could look in the reassembly queue to see
		** if the packet was previously fragmented,
		** but it's not worth the time; just let them time out.
		*/

		if (ip->ip_off &~ IP_DF) {
			/*
			 * Look for queue of fragments of this datagram. 
			 */
			for (fp = ipq.next; fp != &ipq; fp = fp->next) {
				if (ip->ip_id == fp->ipq_id &&
				    ip->ip_src.s_addr == fp->ipq_src.s_addr &&
				    ip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
				    ip->ip_p == fp->ipq_p)
					goto found;
			}
			fp = 0;
found:

			/*
			 * Adjust ip_len to not reflect header, set ip_mff if
			 * more fragments are expected, convert offset of this
			 * to bytes. 
			 */
			ip->ip_len -= hlen;
			IPASFRAG(ip)->ipf_mff = 0;
			if (ip->ip_off & IP_MF)
				IPASFRAG(ip)->ipf_mff = 1;
			ip->ip_off <<= 3;

			/*
			 * If datagram marked as having more fragments or if
			 * this is not the first fragment, attempt reassembly;
			 * if it succeeds, proceed. 
			 */
			if (IPASFRAG(ip)->ipf_mff || ip->ip_off) {
				ipstat.ips_fragments++;
				bp = ip_reass(bp, fp);
				if (bp == 0)
					continue;
				ip = (struct ip *) bp->b_rptr;
			} else if (fp)
				ip_freef(fp);

		} else
			ip->ip_len -= hlen;
		/*
		 * Switch out to protocol's input routine. 
		 */
		if (ip_protox[ip->ip_p] == ipcnt) {
			if (ipdprintf) {
				printf ("ipintr: drop - proto %d not supported\n",
					ip->ip_p);
			}
			freemsg(bp);
			continue;
		}
		qp = ip_pcb[ip_protox[ip->ip_p]].ip_rdq;
		if ((fbp = allocb(sizeof(struct N_unitdata_ind) +
				  sizeof(struct ip_provider *),
				  BPRI_HI)) == NULL) {
			if (ipdprintf) {
				printf ("ipintr: drop - allocb failed\n");
			}
			freemsg(bp);
			continue;
		}
		hdr = (struct N_unitdata_ind *) fbp->b_rptr;
		fbp->b_wptr += sizeof(struct N_unitdata_ind) +
			sizeof(struct ip_provider *);
		fbp->b_datap->db_type = M_PROTO;
		hdr->PRIM_type = N_UNITDATA_IND;
		hdr->LA_length = 0;
		hdr->RA_offset = sizeof(struct N_unitdata_ind);
		hdr->RA_length = sizeof(struct ip_provider *);
		*((struct ip_provider **) (hdr + 1)) =
			(struct ip_provider *) q->q_ptr;
		fbp->b_cont = bp;
		if (!canput(qp)) {
			/* TOM - should send source quench here ? */
			STRLOG(IPM_ID, 2, 5, SL_TRACE, "client %d full",
			       ip->ip_p);
			if (ipdprintf) {
				printf ("ipintr: drop - can't put\n");
			}
			freemsg(fbp);
			continue;
		}
		if (ipdprintf) {
			printf ("ipintr: passing buf up\n");
		}
		putnext(qp, fbp);
		continue;

bad:
		if (ipdprintf) {
			printf ("ipinitr: drop - bad packet\n");
		}
		freemsg(bp);
	}
}

/*
 * Take incoming datagram fragment and try to reassemble it into whole
 * datagram.  If a chain for reassembly of this datagram already exists, then
 * it is given as fp; otherwise have to make a chain. 
 */
mblk_t         *
ip_reass(bp, fp)
	register mblk_t *bp;
	register struct ipq *fp;
{
	register struct ipasfrag *ip;
	register struct ipasfrag *q, *qprev;
	mblk_t         *nbp;
	int             hlen = ((struct ip *) bp->b_rptr)->ip_hl << 2;
	int             i, next;

	ip = (struct ipasfrag *) bp->b_rptr;
	bp->b_datap->db_type = M_DATA;	/* we only send data up */

	STRLOG(IPM_ID, 2, 7, SL_TRACE,
	       "ip_reass fp = %x off = %d len = %d",
	       fp, ip->ip_off, ip->ip_len);
	/*
	 * Presence of header sizes in data blocks would confuse code below. 
	 */
	bp->b_rptr += hlen;

	/*
	 * If first fragment to arrive, create a reassembly queue. 
	 */
	if (fp == 0) {
		if ((fp = (struct ipq *)kmem_alloc(sizeof(struct ipq), KM_NOSLEEP)) == NULL)
			goto dropfrag;
		STRLOG(IPM_ID, 2, 9, SL_TRACE, "first frag, fp = %x", fp);
		insque((struct vq *) fp, (struct vq *) & ipq);
		fp->ipq_ttl = IPFRAGTTL;
		fp->ipq_p = ip->ip_p;
		fp->ipq_id = ip->ip_id;
		fp->ipq_next = qprev = (struct ipasfrag *) fp;
		fp->ipq_src = IPHDR(ip)->ip_src;
		fp->ipq_dst = IPHDR(ip)->ip_dst;
		q = (struct ipasfrag *) fp;
		goto insert;
	}
	/*
	 * Find a segment which begins after this one does. 
	 */
	for (qprev = (struct ipasfrag *) fp, q = fp->ipq_next; 
	     q != (struct ipasfrag *) fp; 
	     qprev = q, q = q->ipf_next)
		if (q->ip_off > ip->ip_off)
			break;

	/*
	 * If there is a preceding segment, it may provide some of our data
	 * already.  If so, drop the data from the incoming segment.  If it
	 * provides all of our data, drop us. 
	 */
	if (qprev != (struct ipasfrag *) fp) {
		i = qprev->ip_off + qprev->ip_len - ip->ip_off;
		if (i > 0) {
			if (i >= ip->ip_len)
				goto dropfrag;
			adjmsg(bp, i);
			ip->ip_off += i;
			ip->ip_len -= i;
		}
	}
	/*
	 * While we overlap succeeding segments trim them or, if they are
	 * completely covered, dequeue them. 
	 */
	while (q != (struct ipasfrag *) fp && ip->ip_off + ip->ip_len > q->ip_off) {
		i = (ip->ip_off + ip->ip_len) - q->ip_off;
		if (i < q->ip_len) {
			STRLOG(IPM_ID, 2, 9, SL_TRACE,
			       "frag overlap adj off %d len %d",
			       q->ip_off, q->ip_len);
			q->ip_len -= i;
			q->ip_off += i;
			adjmsg(q->ipf_mblk, i);
			break;
		}
		STRLOG(IPM_ID, 2, 9, SL_TRACE,
		       "frag overlap del off %d len %d",
		       q->ip_off, q->ip_len);
		ip_deqnxt(qprev);
		freemsg(q->ipf_mblk);
		q = qprev->ipf_next;
	}

insert:
	ip->ipf_mblk = bp;
	/*
	 * Stick new segment in its place; check for complete reassembly. 
	 */
	ip_enq(ip, qprev);
	next = 0;
	for (q = fp->ipq_next; 
	     q != (struct ipasfrag *) fp; 
	     qprev = q, q = q->ipf_next) {
		if (q->ip_off != next)
			return (0);
		next += q->ip_len;
	}
	if (qprev->ipf_mff)
		return (0);

	/*
	 * Reassembly is complete; concatenate fragments. 
	 */
	q = fp->ipq_next;
	bp = q->ipf_mblk;
	q = q->ipf_next;
	for (; q != (struct ipasfrag *)fp; bp = nbp, q = q->ipf_next) {
		nbp = q->ipf_mblk;
		linkb(bp, nbp);
	}

	/*
	 * Create header for new ip packet by modifying header of first
	 * packet; dequeue and discard fragment reassembly header. Make
	 * header visible. 
	 */
	ip = fp->ipq_next;
	ip->ip_len = next;
	bp = ip->ipf_mblk;
	bp->b_rptr -= (ip->ip_hl << 2);
	IPHDR(ip)->ip_src = fp->ipq_src;
	IPHDR(ip)->ip_dst = fp->ipq_dst;
	remque((struct vq *) fp);
	kmem_free((caddr_t)fp, sizeof(struct ipq));
	STRLOG(IPM_ID, 2, 5, SL_TRACE, "frag complete fp = %x", fp);
	return (bp);

dropfrag:
	STRLOG(IPM_ID, 2, 3, SL_TRACE,
	       "dropped frag fp = %x off = %d len = %d",
	       fp, ip ? ip->ip_off : 0, ip ? ip->ip_len : 0);
	ipstat.ips_fragdropped++;
	freemsg(bp);
	return (0);
}


/*
 * Free a fragment reassembly header and all associated datagrams. 
 */
ip_freef(fp)
	struct ipq     *fp;
{
	register struct ipasfrag *q;

	STRLOG(IPM_ID, 2, 9, SL_TRACE, "ip_freef fp %x", fp);
	while((q = fp->ipq_next) != (struct ipasfrag *)fp) {
		ip_deqnxt((struct ipasfrag *)fp);
		freemsg(q->ipf_mblk);
	}
	remque((struct vq *) fp);
	kmem_free((caddr_t)fp, sizeof(struct ipq));
}


/*
 * Put an ip fragment on a reassembly chain. 
 * This chain is no longer doubly linked.
 */
ip_enq(p, prev)
	register struct ipasfrag *p, *prev;
{

	p->ipf_next = prev->ipf_next;
	prev->ipf_next = p;
}

/*
 * Replaces old remque-like ip_deq which operated on doubly linked lists
 */
ip_deqnxt(p)
	register struct ipasfrag *p;
{

	p->ipf_next = p->ipf_next->ipf_next;
}

/*
 * IP timer processing; if a timer expires on a reassembly queue, discard it. 
 */
ip_slowtimo()
{
	register struct ipq *fp;

	sltimein();
	fp = ipq.next;
	if (fp == 0) {
		goto newtimer;
	}
	while (fp != &ipq) {
		--fp->ipq_ttl;
		fp = fp->next;
		if (fp->prev->ipq_ttl == 0) {
			ipstat.ips_fragtimeout++;
			ip_freef(fp->prev);
		}
	}
newtimer:
	iptimerid = timeout(ip_slowtimo, (caddr_t) 0, HZ / 2);
}

/*
 * Drain off all datagram fragments. 
 */
ip_drain()
{

	while (ipq.next != &ipq) {
		ipstat.ips_fragdropped++;
		ip_freef(ipq.next);
	}
}

struct ip_provider *ip_rtaddr();

/*
 * Do option processing on a datagram, possibly discarding it if bad options
 * are encountered. 
 */
ip_dooptions(bp, q)
	mblk_t         *bp;
	queue_t        *q;
{
	register struct ip *ip;
	register u_char *cp;
	int             opt, optlen, cnt, off, code, type = ICMP_PARAMPROB;
	register struct ip_timestamp *ipt;
	register struct ip_provider *prov;
	struct in_addr *sin, in;
	n_time          ntime;

	ip = (struct ip *) bp->b_rptr;
	cp = (u_char *) (ip + 1);
	cnt = (ip->ip_hl << 2) - sizeof(struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[IPOPT_OPTVAL];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[IPOPT_OLEN];
			if (optlen <= 0 || optlen > cnt) {
				code = &cp[IPOPT_OLEN] - (u_char *) ip;
				goto bad;
			}
		}
		switch (opt) {

		default:
			break;

			/*
			 * Source routing with record. Find interface with
			 * current destination address. If none on this
			 * machine then drop if strictly routed, or do
			 * nothing if loosely routed. Record interface
			 * address and bring up next address component.  If
			 * strictly routed make sure next address on directly
			 * accessible net. 
			 */
		case IPOPT_LSRR:
		case IPOPT_SSRR:
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *) ip;
				goto bad;
			}
			prov = prov_withaddr(ip->ip_dst);
			if (prov == 0) {
				if (opt == IPOPT_SSRR) {
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_SRCFAIL;
					goto bad;
				}
				/*
				 * Loose routing, and not at next destination
				 * yet; nothing to do except forward. 
				 */
				break;
			}
			off--;	/* 0 origin */
			if (off > optlen - sizeof(struct in_addr)) {
				/*
				 * End of source route.  Should be for us. 
				 */
				save_rte(cp, ip->ip_src);
				break;
			}
			/*
			 * locate outgoing interface 
			 */
			bcopy((caddr_t) (&ip->ip_dst), (caddr_t) & in,
			      sizeof(in));
			if ((opt == IPOPT_SSRR &&
			     in_onnetof(in_netof(in)) == 0) ||
			    (prov = ip_rtaddr(in)) == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_HOST;
				goto bad;
			}
			ip->ip_dst = in;
			bcopy((caddr_t) PROV_INADDR(prov),
			      (caddr_t) (cp + off), sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			break;

		case IPOPT_RR:
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *) ip;
				goto bad;
			}
			/*
			 * If no space remains, ignore. 
			 */
			off--;	/* 0 origin */
			if (off > optlen - sizeof(struct in_addr))
				break;
			bcopy((caddr_t) (&ip->ip_dst), (caddr_t) & in,
			      sizeof(in));
			/*
			 * locate outgoing interface 
			 */
			if ((prov = ip_rtaddr(in)) == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_HOST;
				goto bad;
			}
			bcopy((caddr_t) PROV_INADDR(prov),
			      (caddr_t) (cp + off), sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			break;

		case IPOPT_TS:
			code = cp - (u_char *) ip;
			ipt = (struct ip_timestamp *) cp;
			if (ipt->ipt_len < 5)
				goto bad;
			if (ipt->ipt_ptr > ipt->ipt_len - sizeof(long)) {
				if (++ipt->ipt_oflw == 0)
					goto bad;
				break;
			}
			sin = (struct in_addr *) (cp + ipt->ipt_ptr - 1);
			switch (ipt->ipt_flg) {

			case IPOPT_TS_TSONLY:
				break;

			case IPOPT_TS_TSANDADDR:
				if (ipt->ipt_ptr + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len)
					goto bad;
				prov = (struct ip_provider *) q->q_ptr;
				bcopy((caddr_t) PROV_INADDR(prov),
				      (caddr_t) sin, sizeof(struct in_addr));
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			case IPOPT_TS_PRESPEC:
				if (ipt->ipt_ptr + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len)
					goto bad;
				bcopy((caddr_t) sin, (caddr_t) & in,
				      sizeof(struct in_addr));
				if (prov_withaddr(in) == 0)
					continue;
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			default:
				goto bad;
			}
			ntime = iptime();
			bcopy((caddr_t) & ntime, (caddr_t)cp + ipt->ipt_ptr - 1,
				sizeof(n_time));
			ipt->ipt_ptr += sizeof(n_time);
		}
	}
	return (0);
bad:
	icmp_error(ip, type, code, q, 0);
	freemsg(bp);
	return (1);
}

/*
 * Given address of next destination (final or next hop), return internet
 * address info of interface to be used to get there. 
 */
struct ip_provider *
ip_rtaddr(dst)
	struct in_addr  dst;
{
	register struct in_addr *sin;
	struct ip_provider *prov;
	int             rtret = RT_OK;

	sin = &satosin(&ipforward_rt.ro_dst)->sin_addr;

	if (ipforward_rt.ro_rt == 0 || dst.s_addr != sin->s_addr) {
		if (ipforward_rt.ro_rt) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		*sin = dst;
		/* SLIP: dial in ip_output, dont cache route */
		if ((rtret = rtalloc(&ipforward_rt, 0)) == RT_DEFER) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
	}
	if (ipforward_rt.ro_rt == 0)
		return ((struct ip_provider *) 0);

	prov = RT(ipforward_rt.ro_rt)->rt_prov;
	if (rtret == RT_SWITCHED) {
		RTFREE(ipforward_rt.ro_rt);
		ipforward_rt.ro_rt = 0;
	}
	return (prov);
}

/*
 * Save incoming source route for use in replies, to be picked up later by
 * ip_srcroute if the receiver is interested. 
 */
save_rte(option, dst)
	u_char         *option;
	struct in_addr  dst;
{
	unsigned        olen;
	extern          ipprintfs;

	olen = option[IPOPT_OLEN];
	if (olen > sizeof(ip_srcrt) - 1) {
		if (ipprintfs)
			printf("save_rte: olen %d\n", olen);
		return;
	}
	bcopy((caddr_t) option, (caddr_t) ip_srcrt.srcopt, olen);
	ip_nhops = (olen - IPOPT_OFFSET - 1) / sizeof(struct in_addr);
	ip_srcrt.route[ip_nhops++] = dst;
}

/*
 * Retrieve incoming source route for use in replies, in the same form used
 * by setsockopt. The first hop is placed before the options, will be removed
 * later. 
 */
mblk_t         *
ip_srcroute(len)
int	len;
{
	register struct in_addr *p, *q;
	register mblk_t *bp;

	if (ip_nhops == 0)
		return ((mblk_t *) 0);
	bp = allocb((int) (ip_nhops * sizeof(struct in_addr) +
			   IPOPT_OFFSET + 1 + 1 + len), BPRI_HI);
	if (bp == 0) {
		return ((mblk_t *) 0);
	}
	bp->b_wptr += ip_nhops * sizeof(struct in_addr) + IPOPT_OFFSET + 1 + 1;

	/*
	 * First save first hop for return route 
	 */
	p = &ip_srcrt.route[ip_nhops - 1];
	*((struct in_addr *) (bp->b_rptr)) = *p--;

	/*
	 * Copy option fields and padding (nop) to data block. 
	 */
	ip_srcrt.nop = IPOPT_NOP;
	bcopy((caddr_t) & ip_srcrt, (char *) (bp->b_rptr +
					      sizeof(struct in_addr)),
	      IPOPT_OFFSET + 1 + 1);
	q = (struct in_addr *) (bp->b_rptr +
			     sizeof(struct in_addr) + IPOPT_OFFSET + 1 + 1);
	/*
	 * Record return path as an IP source route, reversing the path
	 * (pointers are now aligned). 
	 */
	while (p >= ip_srcrt.route)
		*q++ = *p--;
	return (bp);
}

/*
 * Strip out IP options, at higher level protocol in the kernel. Second
 * argument is buffer to which options will be moved, and return value is
 * their length. 
 */
ip_stripoptions(bp, moptbp)
	mblk_t         *bp, *moptbp;
{
	struct ip      *ip;
	register int    i;
	register caddr_t opts;
	int             olen, optsoff = 0;;

	ip = (struct ip *) bp->b_rptr;
	olen = (ip->ip_hl << 2) - sizeof(struct ip);
	opts = (caddr_t) (ip + 1);
	/*
	 * Copy the options if there's a place to put them.
	 */

	if (moptbp) {

		/*
		 * If rptr == wptr, we're dealing with an option set
		 * that ip_srcroute found no source routing in.
		 * So, we've got an empty mblk, into the beginning
		 * of which we have to coerce a "first hop" address.
		 * In a packet with no source routing, this would be the
		 * destination address.  Otherwise, wptr != rptr, and
		 * we're just appending to the mblk coming out of
		 * ip_srcroute.
		 */

		if (moptbp->b_wptr == moptbp->b_rptr) {
			bcopy(&ip->ip_dst, moptbp->b_wptr, sizeof(struct in_addr));
			moptbp->b_wptr += sizeof(struct in_addr);
		}

		/*
		 * Push the rest of the options in.  We don't have
		 * to worry about the other IP level options like
		 * we do the source routing, so just search for them
		 * and insert them into the mblk.  Notice that anything dealing
		 * with source routing is ignored, since you would want to
		 * do that in ip_srcroute instead.
		 */
		
		while (optsoff + 1 <= olen) {
			switch((u_char)opts[optsoff]) {
			case IPOPT_LSRR:
			case IPOPT_SSRR:
				optsoff += opts[optsoff + IPOPT_OLEN];
				break;
			case IPOPT_EOL:
			case IPOPT_NOP:
				*moptbp->b_wptr = opts[optsoff++];
				moptbp->b_wptr++;
				break;
			default:
				bcopy((caddr_t)&opts[optsoff],
				      (caddr_t)moptbp->b_wptr,
				      opts[optsoff + IPOPT_OLEN]);
				moptbp->b_wptr += opts[optsoff + IPOPT_OLEN];
				optsoff += opts[optsoff + IPOPT_OLEN];
				break;
			}
		}
	}

	/*
	 * Actually strip out the old options data.
	 */
	i = (bp->b_wptr - bp->b_rptr) - (sizeof(struct ip) + olen);
	bcopy(opts + olen, opts, (unsigned) i);
	bp->b_wptr -= olen;
	ip->ip_hl = sizeof(struct ip) >> 2;
}

u_char          inetctlerrmap[PRC_NCMDS] = {
					    0, 0, 0, 0,
					    0, 0, EHOSTDOWN, EHOSTUNREACH,
		      ENETUNREACH, EHOSTUNREACH, ECONNREFUSED, ECONNREFUSED,
					    EMSGSIZE, EHOSTUNREACH, 0, 0,
					    0, 0, 0, 0,
					    ENOPROTOOPT
};

int             ipprintfs = 0;

extern int		ipforwarding;		/* from /etc/master.d/ip */
extern int		ipsendredirects;	/* from /etc/master.d/ip */

/*
 * Forward a packet.  If some error occurs return the sender an icmp packet.
 * Note we can't always generate a meaningful icmp message because icmp
 * doesn't have a large enough repertoire of codes and types. 
 *
 * If not forwarding (possibly because we have only a single external network),
 * just drop the packet.  This could be confusing if ipforwarding was zero
 * but some routing protocol was advancing us as a gateway to somewhere.
 * However, we must let the routing protocol deal with that. 
 */
ip_forward(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	register struct ip *ip;
	register int    error = 0, type = 0, code;
	struct in_addr *in;
	register struct ip_provider *prov;
	mblk_t         *mcopy, *fbp;
	register int    i;
	struct in_addr  dest;
	struct ip_unitdata_req *ip_req;
	int             rtret = RT_OK;

	mcopy = bp;		/* in case we call icmp_error */
	dest.s_addr = 0;
	ip = (struct ip *) bp->b_rptr;
	if (ipprintfs)
		printf("forward: src %x dst %x ttl %x\n", ntohl(ip->ip_src),
		       ntohl(ip->ip_dst), ip->ip_ttl);
	ip->ip_id = htons(ip->ip_id);
	if (ipforwarding == 0 || in_interfaces <= 1) {
		if (ipdprintf) {
			printf ("ip_forward: cant forward\n");
		}
		ipstat.ips_cantforward++;
		freemsg(bp);
		return;
	}
	if (in_canforward(ip->ip_dst) == 0) {
		if (ipdprintf) {
			printf ("ip_forward: can't in_forward\n");
		}
		freemsg(bp);
		return;
	}
	if (ip->ip_ttl <= IPTTLDEC) {
		type = ICMP_TIMXCEED, code = ICMP_TIMXCEED_INTRANS;
		goto sendicmp;
	}
	ip->ip_ttl -= IPTTLDEC;

	/*
	 * Save at most 64 bytes of the packet in case we need to generate an
	 * ICMP message to the src. 
	 */
	mcopy = dupmsg(bp);

	in = &satosin(&ipforward_rt.ro_dst)->sin_addr;
	if (ipforward_rt.ro_rt == 0 ||
	    ip->ip_dst.s_addr != in->s_addr) {
		if (ipforward_rt.ro_rt) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		*in = ip->ip_dst;

		/* SLIP: dial in ip_output, dont cache route */
		if ((rtret = rtalloc(&ipforward_rt, SSF_REMOTE)) == RT_DEFER) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
	}
	/*
	 * If forwarding packet using same interface that it came in on,
	 * perhaps should send a redirect to sender to shortcut a hop. Only
	 * send redirect if source is sending directly to us, and if packet
	 * was not source routed (or has any options). 
	 * Also, don't send redirect if forwarding using a default route
	 * or a route modified by a redirect.
	 */
	prov = (struct ip_provider *) q->q_ptr;
	if (ipforward_rt.ro_rt && RT(ipforward_rt.ro_rt)->rt_prov == prov &&
	   (RT(ipforward_rt.ro_rt)->rt_flags & RTF_DYNAMIC|RTF_MODIFIED) == 0 &&
	    satosin(&(RT(ipforward_rt.ro_rt)->rt_dst))->sin_addr.s_addr != 0 &&
	    ipsendredirects && ip->ip_hl == (sizeof(struct ip) >> 2)) {
		u_long          src = ntohl(ip->ip_src.s_addr);
		u_long          dst = ntohl(ip->ip_dst.s_addr);

		if ((src & prov->ia.ia_subnetmask) == prov->ia.ia_subnet) {
			if (RT(ipforward_rt.ro_rt)->rt_flags & RTF_GATEWAY)
				dest = satosin(&(RT(ipforward_rt.ro_rt)->rt_gateway))->sin_addr;
			else
				dest = ip->ip_dst;
			/*
			 * If the destination is reached by a route to host,
			 * is on a subnet of a local net, or is directly on
			 * the attached net (!), use host redirect. (We may
			 * be the correct first hop for other subnets.) 
			 */
			type = ICMP_REDIRECT;
			code = ICMP_REDIRECT_NET;
			if ((RT(ipforward_rt.ro_rt)->rt_flags & RTF_HOST) ||
			    (RT(ipforward_rt.ro_rt)->rt_flags & RTF_GATEWAY) == 0)
				code = ICMP_REDIRECT_HOST;
			else
				for (prov = provider; prov <= lastprov; prov++) {
					if (prov->qbot == 0)
						continue;
					if ((dst & prov->ia.ia_netmask) == prov->ia.ia_net) {
						if (prov->ia.ia_subnetmask != prov->ia.ia_netmask)
							code = ICMP_REDIRECT_HOST;
						break;
					}
				}
			if (ipprintfs)
				printf("redirect (%d) to %x\n", code, dest);
		}
	}
	if ((fbp = allocb(sizeof(struct ip_unitdata_req), BPRI_MED)) == NULL) {
		freemsg(bp);
		if (mcopy) {
			freemsg(mcopy);
		}
		error = ENOSR;
		return;
	}
	ip_req = (struct ip_unitdata_req *) fbp->b_rptr;
	fbp->b_wptr += sizeof(struct ip_unitdata_req);
	fbp->b_cont = bp;
	fbp->b_datap->db_type = M_PROTO;
	ip_req->dl_primitive = N_UNITDATA_REQ;
	ip_req->dl_dest_addr_length = 0;
	ip_req->route = ipforward_rt;
	ip_req->flags = IP_FORWARDING;
	ip_req->options = (mblk_t *) NULL;
	for (i = 0; i < ipcnt; i++) {
		if (ip_pcb[i].ip_state & IPOPEN) {
			break;
		}
	}
	if (i >= ipcnt) {
		freemsg(fbp);
		if (mcopy) {
			freemsg(mcopy);
		}
		error = EINVAL;
		return;
	}
	if (ipforward_rt.ro_rt)
		RT(ipforward_rt.ro_rt)->rt_refcnt++;
	error = ip_output(WR(ip_pcb[i].ip_rdq), fbp);

	if (rtret == RT_SWITCHED) {
		RTFREE(ipforward_rt.ro_rt);
		ipforward_rt.ro_rt = 0;
	}
	if (error)
		ipstat.ips_cantforward++;
	else if (type)
		ipstat.ips_redirectsent++;
	else {
		if (mcopy)
			freemsg(mcopy);
		ipstat.ips_forward++;
		return;
	}
	if (mcopy == NULL)
		return;
	ip = (struct ip *) mcopy->b_rptr;
	type = ICMP_UNREACH;
	switch (error) {

	case 0:		/* forwarded, but need redirect */
		type = ICMP_REDIRECT;
		/* code set above */
		break;

	case ENETUNREACH:
	case ENETDOWN:
		if (in_localaddr(ip->ip_dst))
			code = ICMP_UNREACH_HOST;
		else
			code = ICMP_UNREACH_NET;
		break;

	case EMSGSIZE:
		code = ICMP_UNREACH_NEEDFRAG;
		break;

	case EPERM:
		code = ICMP_UNREACH_PORT;
		break;

	case ENOSR:	/* same as ENOBUFS */
		type = ICMP_SOURCEQUENCH;
		code = 0;
		break;

	case EHOSTDOWN:
	case EHOSTUNREACH:
		code = ICMP_UNREACH_HOST;
		break;

	default:
		STRLOG(IPM_ID, 3, 9, SL_ERROR,
		       "ip_forward: unrecognized error %d", error);
		break;
	}
sendicmp:
	icmp_error(ip, type, code, q, dest);
	if (mcopy)
		freemsg(mcopy);
}
