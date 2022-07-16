/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:arp.c	1.3.1.1"

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

/*
 * Ethernet Address Resolution Protocol (ARP).  This program actually
 * consists of a streams driver ARP and module APP.  ARP implements the
 * Address Resolution Protocol itself using broadcasts and direct responses
 * to talk to other ARP implementations on the local net. APP functions
 * as the convergence module between an IP layer and an ethernet driver using
 * the link level interface based on LLC1.  Note that both sides can
 * understand arp ioctls.  It is recommended that users wishing to manipulate
 * the arp tables do so through the ARP side, but ioctls are allowed in
 * APP so that programs based on the sockets paradigm can manipulate
 * these tables through the higher level. 
 *
 * On initialization an ARP driver gets an ethernet interface linked below it.
 * The slink process should also have linked a corresponding APP module
 * between the same ethernet module and a bottom queue of IP.  These two then
 * function in tandem by sharing the arpcom structure.  If arpresolve can't
 * find a destination in the arp table, it will call arpwhohas, which will do
 * the transmission.  When a response comes in, the saved output packet will
 * be transmitted. 
 */

#define STRNET
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/sockio.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/log.h>
#include <sys/strlog.h>
#include <sys/dlpi.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/strioc.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <net/route.h>
#include <netinet/ip_str.h>
#include <netinet/arp.h>

struct arptab  *arptnew();
char           *ether_sprintf();
static int	memcmp();

/* configurable parameters */
extern struct arptab		arptab[];
extern int			arptab_bsiz;
extern int			arptab_nb;
extern int			arptab_size;

#define	SPLNULL		(-1)	/* unused spl level */

/*
 * ARP trailer negotiation.  Trailer protocol is not IP specific, but ARP
 * request/response use IP addresses. 
 */
#define ETHERTYPE_IPTRAILERS ETHERTYPE_TRAIL

#define	ARPTAB_HASH(a) \
	((u_long)(a) % arptab_nb)

#define	ARPTAB_LOOK(at,addr) { \
	register n; \
	at = &arptab[ARPTAB_HASH(addr) * arptab_bsiz]; \
	for (n = 0 ; n < arptab_bsiz ; n++,at++) \
		if (at->at_iaddr.s_addr == addr) \
			break; \
	if (n >= arptab_bsiz) \
		at = 0; \
}

#define	ARP_IDLE(a) \
	( ((a)->arp_qtop == NULL) && !((a)->arp_flags & ARPF_PLINKED))

/* timer values */
#define	ARPT_AGE	(60*1)	/* aging timer, 1 min. */
#define	ARPT_KILLC	20	/* kill completed entry in 20 mins. */
#define	ARPT_KILLI	3	/* kill incomplete entry in 3 minutes */

ether_addr_t etherbroadcastaddr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
ether_addr_t null_etheraddr = {0, 0, 0, 0, 0, 0};

extern struct ifnet loif;
int	arptimerid;
int	arpinited;


int             arpopen(), arpclose(), arpuwput(), arplrput();

static struct module_info arpm_info[MUXDRVR_INFO_SZ] = {
	ARPM_ID, "arp", 0, 8192, 8192, 1024,
	ARPM_ID, "arp", 0, 8192, 8192, 1024,
	ARPM_ID, "arp", 0, 8192, 8192, 1024,
	ARPM_ID, "arp", 0, 8192, 8192, 1024,
	ARPM_ID, "arp", 0, 8192, 8192, 1024
};

static struct qinit arpurinit =
{NULL, NULL, arpopen, arpclose, NULL, &arpm_info[IQP_RQ], NULL};

static struct qinit arpuwinit =
{arpuwput, NULL, arpopen, arpclose, NULL, &arpm_info[IQP_WQ], NULL};

static struct qinit arplrinit =
{arplrput, NULL, arpopen, arpclose, NULL, &arpm_info[IQP_MUXRQ], NULL};

static struct qinit arplwinit =
{NULL, NULL, arpopen, arpclose, NULL, &arpm_info[IQP_MUXWQ], NULL};

struct streamtab arpinfo = {&arpurinit, &arpuwinit, &arplrinit, &arplwinit};

extern struct app_pcb app_pcb[];
struct arp_pcb arp_pcb[N_ARP];

#define	ARPF_PLINKED	0x1		/* persistent links */

/*
 * Below is the code implementing the actual Address Resolution Protocol
 * which is spurred by failed arpresolve requests.  No one can ever actually
 * use arp from user level, so all the put routine supports is an ioctl
 * interface. 
 */

arpinit()
{
	ipregister();
	arpinited = 1;
}


/* ARGSUSED */
arpopen(q, dev, flag, sflag)
	queue_t        *q;
{
	mblk_t *bp;
	register struct arp_pcb *ar;

	STRLOG(ARPM_ID, 0, 9, SL_TRACE, "arp open called");
	if (!arpinited)
		arpinit();
	dev = minor(dev);
	if (sflag == CLONEOPEN) {
		for (dev = 0; dev < N_ARP; dev++) {
			if (ARP_IDLE(&arp_pcb[dev])) {
				break;
			}
		}
	}
	if (dev < 0 || dev >= N_ARP) {
		return (OPENFAIL);
	}
	ar = (struct arp_pcb *) &arp_pcb[dev];
	if (ARP_IDLE(ar)) {
		ar->arp_qtop = q;
		ar->arp_qbot = NULL;
		q->q_ptr = (char *) ar;
		WR(q)->q_ptr = (char *) ar;
	} else if (q != ar->arp_qtop) {
		return (OPENFAIL);
	}
	/*
	 * Set up the correct stream head flow control parameters 
	 */
	if (bp = allocb(sizeof(struct stroptions), BPRI_MED)) {
		struct stroptions *sop = (struct stroptions *) bp->b_rptr;

		bp->b_datap->db_type = M_SETOPTS;
		bp->b_wptr += sizeof(struct stroptions);
		sop->so_flags = SO_HIWAT | SO_LOWAT;
		sop->so_hiwat = arpm_info[IQP_HDRQ].mi_hiwat;
		sop->so_lowat = arpm_info[IQP_HDRQ].mi_lowat;
		putnext(q, bp);
	}
	STRLOG(ARPM_ID, 0, 9, SL_TRACE, "open succeeded");
	return (dev);
}

arpclose(q)
	queue_t        *q;
{
	register struct arp_pcb *ar;
	register int    i;

	STRLOG(ARPM_ID, 0, 9, SL_TRACE, "arp close called");
	for (i = 0; i < N_ARP; i++) {
		if (arp_pcb[i].arp_qtop == q) {
			ar = (struct arp_pcb *) &arp_pcb[i];
			ar->arp_qtop = NULL;
			if (ar->arp_flags & ARPF_PLINKED)
				continue;
			if (ar->app_pcb) {
				ar->app_pcb->arp_pcb = NULL;
				ar->app_pcb = NULL;
			}
			ar->arp_uname[0] = NULL;
		}
	}
	if (!(ar->arp_flags & ARPF_PLINKED)) {
        	q->q_ptr = NULL;
		WR(q)->q_ptr = NULL;
		flushq(WR(q), 1);
	}
	STRLOG(ARPM_ID, 0, 9, SL_TRACE, "close succeeded");
	return;
}


/*
 * Handle downstream requests.  At the moment, this only means ioctls, since
 * we don't let the user send arp requests, etc. 
 */

arpuwput(q, bp)
	queue_t        	*q;
	mblk_t         	*bp;
{
	struct linkblk	*lp;
	dl_bind_req_t	*bindr;
	dl_unbind_req_t *unbindr;
	mblk_t         	*nbp;
	struct iocblk  	*iocbp;
	struct ifreq   	*ifr;
	struct arp_pcb 	*ar;
	register int	i;
	int		s;

	if (bp->b_datap->db_type != M_IOCTL) {
		STRLOG(ARPM_ID, 0, 9, SL_TRACE, "Bad request for ARP, %d\n",
		       bp->b_datap->db_type);
		freemsg(bp);
		return;
	}
	ar = (struct arp_pcb *) q->q_ptr;
	iocbp = (struct iocblk *) bp->b_rptr;
	if (msgblen(bp) >= sizeof (struct iocblk_in))
		/* It probably came from from IP.  Pass our flags back up. */
		((struct iocblk_in *) iocbp)->ioc_ifflags |= IFF_BROADCAST;

	switch (iocbp->ioc_cmd) {
	case I_PLINK:
		ar->arp_flags |= ARPF_PLINKED;
		/* no break */
	case I_LINK:
		iocbp->ioc_error = 0;
		iocbp->ioc_rval = 0;
		iocbp->ioc_count = 0;

		/*
		 * If we've already used this bottom, clone a new pcb. 
		 */

		if (ar->arp_qbot != NULL) {
			for (i = 0; i < N_ARP; i++) {
				if (ARP_IDLE(&arp_pcb[i])) {
					break;
				}
			}
			if (i == N_ARP) {
				ar->arp_flags &= ~ARPF_PLINKED;
				iocbp->ioc_error = EBUSY;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(ARPM_ID, 0, 9, SL_TRACE,
				       "I_LINK failed: No free devices");
				qreply(q, bp);
				return;
			}
			arp_pcb[i].arp_qtop = ar->arp_qtop;
			ar = &arp_pcb[i];
		}
		lp = (struct linkblk *) bp->b_cont->b_rptr;
		ar->arp_qbot = lp->l_qbot;
		ar->arp_qbot->q_ptr = (char *) ar;
		OTHERQ(ar->arp_qbot)->q_ptr = (char *) ar;
		ar->arp_index = lp->l_index;
		if ((nbp = allocb(sizeof(dl_bind_req_t), BPRI_HI))
		    == NULL) {
			ar->arp_flags &= ~ARPF_PLINKED;
			iocbp->ioc_error = ENOSR;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(ARPM_ID, 0, 9, SL_TRACE,
			       "I_LINK failed: Can't alloc bind buf");
			qreply(q, bp);
			return;
		}
		nbp->b_datap->db_type = M_PROTO;
		nbp->b_wptr += sizeof(dl_bind_req_t);
		bindr = (dl_bind_req_t *) nbp->b_rptr;
		bindr->dl_primitive = DL_BIND_REQ;
		bindr->dl_sap = ARP_SAP;
		putnext(ar->arp_qbot, nbp);
		bp->b_datap->db_type = M_IOCACK;
		STRLOG(ARPM_ID, 0, 9, SL_TRACE, "I_LINK succeeded");
		qreply(q, bp);
		return;

	case I_PUNLINK:
		ar->arp_flags &= ~ARPF_PLINKED;
		/* no break */
	case I_UNLINK:
		iocbp->ioc_error = 0;
		iocbp->ioc_rval = 0;
		iocbp->ioc_count = 0;
		lp = (struct linkblk *) bp->b_cont->b_rptr;

		for (i = 0; i < N_ARP; i++) {
			if (!ARP_IDLE(&arp_pcb[i]) &&
			    arp_pcb[i].arp_index == lp->l_index) {
				ar = (struct arp_pcb *) &arp_pcb[i];
				break;
			}
		}
		if (i == N_ARP || ar->arp_qbot == NULL) {
			iocbp->ioc_error = EPROTO;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(ARPM_ID, 0, 9, SL_TRACE,
			       "I_UNLINK: bad unlink req");
			qreply(q, bp);
			return;
		}
		/* Do the link level unbind */

		if ((nbp = allocb(sizeof(dl_unbind_req_t),
				  BPRI_HI)) == NULL) {
			iocbp->ioc_error = ENOSR;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(ARPM_ID, 0, 9, SL_TRACE,
			       "I_UNLINK: no buf for unbind");
			qreply(q, bp);
			return;
		}
		nbp->b_datap->db_type = M_PROTO;
		nbp->b_wptr += sizeof(dl_unbind_req_t);
		unbindr = (dl_unbind_req_t *) nbp->b_rptr;
		unbindr->dl_primitive = DL_UNBIND_REQ;
		putnext(lp->l_qbot, nbp);
		ar->arp_qbot = NULL;
		ar->arp_index = 0;
		ar->arp_flags &= ~ARPF_PLINKED;
		bp->b_datap->db_type = M_IOCACK;
		STRLOG(ARPM_ID, 0, 9, SL_TRACE, "I_UNLINK succeeded");
		qreply(q, bp);
		return;

	case SIOCSARP:
	case SIOCDARP:
	case SIOCGARP:
		arpioctl(q, bp);
		return;

	case SIOCSIFNAME:
		ifr = (struct ifreq *) bp->b_cont->b_rptr;
		strcpy(ar->arp_uname, ifr->ifr_name);
		for (i = 0; i < N_ARP; i++) {
			if (app_pcb[i].app_q &&
			    !strcmp(app_pcb[i].app_uname, ar->arp_uname)) {
				app_pcb[i].arp_pcb = ar;
				ar->app_pcb = &app_pcb[i];
			}
		}
		bp->b_datap->db_type = M_IOCACK;
		qreply(q, bp);
		s = splstr();
		if (ar->arp_saved) {
			bp = ar->arp_saved;
			ar->arp_saved = NULL;
			in_arpinput(ar, bp);
		}
		splx(s);
		break;

	case INITQPARMS:
		if (iocbp->ioc_error = initqparms(bp, arpm_info, MUXDRVR_INFO_SZ))
			bp->b_datap->db_type = M_IOCNAK;
		else
			bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		return;

	default:
		/*
		 * Send everything else downstream. 
		 */
		if (ar->arp_qbot) {
			putnext(ar->arp_qbot, bp);
		} else {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
		}
		break;

	}
}

/*
 * arplrput accepts packets from downstream, and updates the ARP tables
 * approriately and generates responses to lower requests.  
 */

arplrput(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	union DL_primitives *prim;
	register struct arphdr *ah;
	register struct arp_pcb *ar = (struct arp_pcb *) q->q_ptr;

	switch (bp->b_datap->db_type) {
	case M_FLUSH:
		if (*bp->b_rptr & FLUSHR)
			flushq(q, FLUSHDATA);
	/* fall through */
	default:
		if (ar->arp_qtop)
			putnext(ar->arp_qtop, bp);
		else
			freemsg(bp);
		break;

	case M_CTL:
		freemsg(bp);
		break;

	case M_PCPROTO:
	case M_PROTO:
		prim = (union DL_primitives *) bp->b_rptr;
		switch (prim->dl_primitive) {
		default:
			STRLOG(ARPM_ID, 0, 9, SL_ERROR,
			       "arplrput: unexpected prim type (%d)", 
			       prim->dl_primitive);
			break;

		case DL_UNITDATA_IND:
			if (ar->app_pcb == NULL) {
				break;
			}
			if (ar->app_pcb->app_ac.ac_if.if_flags & IFF_NOARP) {
				break;
			}
			if (pullupmsg(bp->b_cont, -1) == 0)
				break;
			if (bp->b_cont->b_wptr - bp->b_cont->b_rptr <
			    sizeof(struct arphdr)) {
				break;
			}
			ah = (struct arphdr *) bp->b_cont->b_rptr;
			if (ntohs(ah->ar_hrd) != ARPHRD_ETHER)
				break;
			if (bp->b_cont->b_wptr - bp->b_cont->b_rptr <
			    sizeof(struct arphdr) +
			    2 * ah->ar_hln + 2 * ah->ar_pln)
				break;

			switch (ntohs(ah->ar_pro)) {

			case ETHERTYPE_IP:
			case ETHERTYPE_IPTRAILERS:
				in_arpinput(ar, bp);
				return;

			default:
				break;
			}
			break;
		}
		freemsg(bp);
		break;
	}
}


/*
 * Timeout routine.  Age arp_tab entries once a minute. 
 */
arptimer()
{
	register struct arptab *at;
	register        i;
	int		s;

	arptimerid = timeout(arptimer, (caddr_t) 0, ARPT_AGE * HZ);
	at = &arptab[0];
	s = splstr();	/* just in case arptimer called < splstr */
	for (i = 0; i < arptab_size; i++, at++) {
		if (at->at_flags == 0 || (at->at_flags & ATF_PERM))
			continue;
		if (++at->at_timer < ((at->at_flags & ATF_COM) ?
				      ARPT_KILLC : ARPT_KILLI))
			continue;
		/* timer has expired, clear entry */
		arptfree(at);
	}
	splx(s);
}

/*
 * Broadcast an ARP packet, asking who has addr on interface ac. 
 */
arpwhohas(ap, addr)
	struct app_pcb *ap;
	struct in_addr *addr;
{
	register struct arpcom *ac = &ap->app_ac;
	register mblk_t *mp;
	register struct ether_arp *ea;
	register dl_unitdata_req_t *req;
	int addrlen = ac->ac_addrlen;

	if (ap->arp_pcb == NULL) {
		return;
	}
	if (!memcmp(&null_etheraddr, &ac->ac_enaddr, sizeof(null_etheraddr))) {
		STRLOG(IPM_ID, 0, 9, SL_ERROR, "null ethernet address, app %x", ap);
		return;
	}
	mp = allocb(sizeof(dl_unitdata_req_t)+addrlen, BPRI_HI);
	if (mp == NULL) {
		return;
	}
	mp->b_cont = allocb(max(ac->ac_mintu, sizeof(struct ether_arp)), BPRI_HI);
	if (mp->b_cont == NULL) {
		freeb(mp);
		return;
	}
	req = (dl_unitdata_req_t *) mp->b_rptr;
	mp->b_wptr += sizeof(dl_unitdata_req_t) + addrlen;
	mp->b_datap->db_type = M_PROTO;
	req->dl_primitive = DL_UNITDATA_REQ;
	req->dl_dest_addr_length = addrlen;
	req->dl_dest_addr_offset = sizeof(dl_unitdata_req_t);
	bcopy((caddr_t) &etherbroadcastaddr,
	      (caddr_t) mp->b_rptr + sizeof(dl_unitdata_req_t),
	      sizeof(etherbroadcastaddr));
	/* if addrlen is 8 (ethernet address length + 2), copy in type */
	if (addrlen == 8)
		*((ushort *) (mp->b_wptr - 2)) = htons(ARP_SAP);
	ea = (struct ether_arp *)
		(mp->b_cont->b_rptr =
	      mp->b_cont->b_datap->db_lim - max(ac->ac_mintu, sizeof(*ea)));
	mp->b_cont->b_wptr = mp->b_cont->b_datap->db_lim;
	bzero((caddr_t) ea, sizeof(*ea));
	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);	/* hardware address length */
	ea->arp_pln = sizeof(ea->arp_spa);	/* protocol address length */
	ea->arp_op = htons(ARPOP_REQUEST);
	bcopy((caddr_t) &ac->ac_enaddr, (caddr_t) &ea->arp_sha,
	      sizeof(ea->arp_sha));
	bcopy((caddr_t) &ac->ac_ipaddr, (caddr_t) ea->arp_spa,
	      sizeof(ea->arp_spa));
	bcopy((caddr_t) addr, (caddr_t) ea->arp_tpa, sizeof(ea->arp_tpa));
	putnext(ap->arp_pcb->arp_qbot, mp);
}

/*
 * Resolve an IP address into an ethernet address.  If success, desten is
 * filled in.  If there is no entry in arptab, set one up and broadcast a
 * request for the IP address. Hold onto this mblk and resend it once the
 * address is finally resolved.  A return value of 1 indicates that desten
 * has been filled in and the packet should be sent normally; a 0 return
 * indicates that the packet has been taken over here, either now or for
 * later transmission. 
 *
 * Note that in the present implemenation, although we set *usetrailers,
 * nothing above actually looks at it.  We will never send a trailer.
 * We also can't process trailers because we can't bind to all of
 * those ptypes.  It wouldn't be efficient anyways.
 *
 */
arpresolve(ap, m, desten, usetrailers)
	struct app_pcb *ap;
	mblk_t         *m;
	register u_char *desten;
	int            *usetrailers;
{
	register struct in_addr *destip;
	dl_unitdata_req_t *req = (dl_unitdata_req_t *) m->b_rptr;
	register struct arpcom *ac = &ap->app_ac;
	register struct arptab *at;
	int             lna;
	int		s;

	destip = (struct in_addr *) (m->b_rptr + req->dl_dest_addr_offset);
	*usetrailers = 0;
	if (in_broadcast(*destip)) {	/* broadcast address */
		bcopy((caddr_t) &etherbroadcastaddr, (caddr_t) desten,
		      sizeof(etherbroadcastaddr));
		return (1);
	}
	lna = in_lnaof(*destip);
	/* if for us, U-turn */
	if (destip->s_addr == ac->ac_ipaddr.s_addr) {
		dl_unitdata_ind_t *ind;
		mblk_t         *hdr;

		hdr = allocb((int) (sizeof(dl_unitdata_ind_t)
				    + 2 * req->dl_dest_addr_length),
			     BPRI_HI);
		if (!hdr) {
			freemsg(m);
			return (0);
		}
		hdr->b_datap->db_type = M_PROTO;
		hdr->b_wptr += sizeof(dl_unitdata_ind_t) +
			2 * req->dl_dest_addr_length;
		ind = (dl_unitdata_ind_t *) hdr->b_rptr;
		ind->dl_primitive = DL_UNITDATA_IND;
		ind->dl_dest_addr_offset = sizeof(dl_unitdata_ind_t);
		ind->dl_dest_addr_length = req->dl_dest_addr_length;
		ind->dl_src_addr_offset = sizeof(dl_unitdata_ind_t) +
			req->dl_dest_addr_length;
		ind->dl_src_addr_length = req->dl_dest_addr_length;
		bcopy((char *) m->b_rptr + req->dl_dest_addr_offset,
		      (char *) hdr->b_rptr + ind->dl_dest_addr_offset,
		      (unsigned) req->dl_dest_addr_length);
		bcopy((char *) m->b_rptr + req->dl_dest_addr_offset,
		      (char *) hdr->b_rptr + ind->dl_src_addr_offset,
		      (unsigned) req->dl_dest_addr_length);
		hdr->b_cont = m->b_cont;
		freeb(m);
		putnext(ap->app_q, hdr);
		/*
		 * The packet has already been sent and freed. 
		 */
		return (0);
	}
	s = splstr();
	ARPTAB_LOOK(at, destip->s_addr);
	if (at == 0) {		/* not found */
		if (ac->ac_if.if_flags & IFF_NOARP) {
			bcopy((caddr_t) &ac->ac_enaddr, (caddr_t) desten, 3);
			desten[3] = (lna >> 16) & 0x7f;
			desten[4] = (lna >> 8) & 0xff;
			desten[5] = lna & 0xff;
			splx(s);
			return (1);
		} else {
			at = arptnew(destip);
			at->at_hold = m;
			arpwhohas(ap, destip);
			splx(s);
			return (0);
		}
	}
	at->at_timer = 0;	/* restart the timer */
	if (at->at_flags & ATF_COM) {	/* entry IS complete */
		bcopy((caddr_t) &at->at_enaddr, (caddr_t) desten,
		      sizeof(at->at_enaddr));
		if (at->at_flags & ATF_USETRAILERS)
			*usetrailers = 1;
		splx(s);
		return (1);
	}
	/*
	 * There is an arptab entry, but no ethernet address response yet.
	 * Replace the held mblk with this latest one. 
	 */
	if (at->at_hold)
		freemsg(at->at_hold);
	at->at_hold = m;
	splx(s);
	arpwhohas(ap, destip);	/* ask again */
	return (0);
}

/*
 * ARP for Internet protocols on 10 Mb/s Ethernet. Algorithm is that given in
 * RFC 826. In addition, a sanity check is performed on the sender protocol
 * address, to catch impersonators. We also handle negotiations for use of
 * trailer protocol: ARP replies for protocol type ETHERTYPE_TRAIL are sent
 * along with IP replies if we want trailers sent to us, and also send them
 * in response to IP replies. This allows either end to announce the desire
 * to receive trailer packets. We reply to requests for ETHERTYPE_TRAIL
 * protocol as well, but don't normally send requests. 
 */
in_arpinput(ar, bp)
	struct arp_pcb *ar;
	mblk_t         *bp;
{
	register struct arpcom *ac = &ar->app_pcb->app_ac;
	register struct ether_arp *ea;
	register struct arptab *at = 0;	/* same as "merge" flag */
	mblk_t         *mcopy = 0, *newbp;
	struct in_addr  isaddr, itaddr, myaddr;
	int             proto, op;
	dl_unitdata_req_t *req;
	int addrlen = ac->ac_addrlen;
	int s = SPLNULL;

	if (!memcmp(&null_etheraddr, &ac->ac_enaddr, sizeof(null_etheraddr))) {
		STRLOG(IPM_ID, 0, 9, SL_ERROR,
			"null ethernet address, arp %x", ar);
		if (ar->arp_saved) {
			freemsg(ar->arp_saved);
		}
		ar->arp_saved = bp;
		return;
	}
	myaddr = ac->ac_ipaddr;
	ea = (struct ether_arp *) bp->b_cont->b_rptr;
	proto = ntohs(ea->arp_pro);
	op = ntohs(ea->arp_op);
	bcopy((caddr_t) ea->arp_spa, (caddr_t) & isaddr, sizeof(struct in_addr));
	bcopy((caddr_t) ea->arp_tpa, (caddr_t) & itaddr, sizeof(struct in_addr));
	STRLOG(ARPM_ID, 0, 8, SL_TRACE,
	     "arp req: spa: %x tpa: %x sha:", isaddr.s_addr, itaddr.s_addr);
	STRLOG(ARPM_ID, 0, 8, SL_TRACE, ether_sprintf(&ea->arp_sha));
	if (!memcmp((caddr_t) &ea->arp_sha, (caddr_t) &ac->ac_enaddr,
		    sizeof(ea->arp_sha)))
		goto out;	/* it's from me, ignore it. */
	if (!memcmp((caddr_t) &ea->arp_sha, (caddr_t) &etherbroadcastaddr,
		    sizeof(ea->arp_sha))) {
		STRLOG(IPM_ID, 0, 9, SL_ERROR,
		     "arp: ether address is broadcast for IP address %x!\n",
		       ntohl(isaddr.s_addr));
		goto out;
	}
	if (isaddr.s_addr == myaddr.s_addr) {
		itaddr = myaddr;
		if (op == ARPOP_REQUEST)
			goto reply;
		goto out;
	}
	s = splstr();
	ARPTAB_LOOK(at, isaddr.s_addr);
	if (at) {
		bcopy((caddr_t) &ea->arp_sha, (caddr_t) &at->at_enaddr,
		      sizeof(ea->arp_sha));
		at->at_flags |= ATF_COM;
		if (at->at_hold) {
			appwput(WR(ar->app_pcb->app_q), at->at_hold);
			at->at_hold = 0;
		}
	}
	if (at == 0 && itaddr.s_addr == myaddr.s_addr) {
		/* ensure we have a table entry */
		at = arptnew(&isaddr);
		bcopy((caddr_t) &ea->arp_sha, (caddr_t) &at->at_enaddr,
		      sizeof(ea->arp_sha));
		at->at_flags |= ATF_COM;
	}
reply:
	switch (proto) {

	case ETHERTYPE_IPTRAILERS:
		/* partner says trailers are OK */
		if (at)
			at->at_flags |= ATF_USETRAILERS;
		/*
		 * Reply to request iff we want trailers. 
		 */
		if (op != ARPOP_REQUEST || ac->ac_if.if_flags & IFF_NOTRAILERS)
			goto out;
		break;

	case ETHERTYPE_IP:
		/*
		 * Reply if this is an IP request, or if we want to send a
		 * trailer response. 
		 */
		if (op != ARPOP_REQUEST && ac->ac_if.if_flags & IFF_NOTRAILERS)
			goto out;
		break;
	default:
		break;
	}
	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		bcopy((caddr_t) &ea->arp_sha, (caddr_t) &ea->arp_tha,
		      sizeof(ea->arp_sha));
		bcopy((caddr_t) &ac->ac_enaddr, (caddr_t) &ea->arp_sha,
		      sizeof(ea->arp_sha));
	} else {
		ARPTAB_LOOK(at, itaddr.s_addr);
		if (at == NULL || (at->at_flags & ATF_PUBL) == 0)
			goto out;
		bcopy((caddr_t) &ea->arp_sha, (caddr_t) &ea->arp_tha,
		      sizeof(ea->arp_sha));
		bcopy((caddr_t) &at->at_enaddr, (caddr_t) &ea->arp_sha,
		      sizeof(ea->arp_sha));
	}
	if (s != SPLNULL) {
		splx(s);
		s = SPLNULL;
	}
	bcopy((caddr_t) ea->arp_spa, (caddr_t) ea->arp_tpa,
	      sizeof(ea->arp_spa));
	bcopy((caddr_t) & itaddr, (caddr_t) ea->arp_spa,
	      sizeof(ea->arp_spa));
	ea->arp_op = htons(ARPOP_REPLY);
	newbp = allocb(sizeof(dl_unitdata_req_t) + addrlen,
		       BPRI_MED);
	if (newbp == NULL)
		goto out;
	newbp->b_datap->db_type = M_PROTO;
	newbp->b_wptr += sizeof(dl_unitdata_req_t) + addrlen;
	req = (dl_unitdata_req_t *) newbp->b_rptr;
	newbp->b_cont = bp->b_cont;
	freeb(bp);
	bp = newbp;
	req->dl_primitive = DL_UNITDATA_REQ;
	req->dl_dest_addr_length = addrlen;
	req->dl_dest_addr_offset = sizeof(dl_unitdata_req_t);
	bcopy((caddr_t) &ea->arp_tha,
	      (caddr_t) (bp->b_rptr + sizeof(dl_unitdata_req_t)),
	      sizeof(ea->arp_tha));
	/* if addrlen is 8 (ethernet address length + 2), copy in type */
	if (addrlen == 8)
		*((ushort *) (newbp->b_wptr - 2)) = htons(ARP_SAP);

	/*
	 * If incoming packet was an IP reply, we are sending a reply for
	 * type IPTRAILERS. If we are sending a reply for type IP and we want
	 * to receive trailers, send a trailer reply as well. 
	 */
	if (op == ARPOP_REPLY)
		ea->arp_pro = htons(ETHERTYPE_IPTRAILERS);
	else if (proto == ETHERTYPE_IP &&
		 (ac->ac_if.if_flags & IFF_NOTRAILERS) == 0)
		mcopy = copymsg(bp);
	STRLOG(ARPM_ID, 0, 8, SL_TRACE, "arp rply: spa: %x sha:", itaddr.s_addr);
	STRLOG(ARPM_ID, 0, 8, SL_TRACE, ether_sprintf(&ea->arp_sha));
	putnext(ar->arp_qbot, bp);
	if (mcopy) {
		ea = (struct ether_arp *) mcopy->b_cont->b_rptr;
		ea->arp_pro = htons(ETHERTYPE_IPTRAILERS);
		putnext(ar->arp_qbot, mcopy);
	}
	return;
out:
	if (s != SPLNULL)
		splx(s);
	freemsg(bp);
	return;
}

/*
 * Free an arptab entry. 
 */
arptfree(at)
	register struct arptab *at;
{
	register int    s;

	s = splstr();

	if (at->at_hold)
		freemsg(at->at_hold);
	at->at_hold = 0;
	at->at_timer = at->at_flags = 0;
	at->at_iaddr.s_addr = 0;
	splx(s);
}

/*
 * Enter a new address in arptab, pushing out the oldest entry from the
 * bucket if there is no room. This always succeeds since no bucket can be
 * completely filled with permanent entries (except from arpioctl when
 * testing whether another permanent entry will fit). 
 */
struct arptab  *
arptnew(addr)
	struct in_addr *addr;
{
	register        n;
	int             oldest = 0;
	register struct arptab *at, *ato = NULL;

	static int      first = 1;

	if (first) {
		first = 0;
		arptimerid = timeout(arptimer, (caddr_t) 0, HZ);
	}
	at = &arptab[ARPTAB_HASH(addr->s_addr) * arptab_bsiz];
	for (n = 0; n < arptab_bsiz; n++, at++) {
		if (at->at_flags == 0)
			goto out;	/* found an empty entry */
		if (at->at_flags & ATF_PERM)
			continue;
		if (at->at_timer > oldest || !oldest) {
			oldest = at->at_timer;
			ato = at;
		}
	}
	if (ato == NULL)
		return (NULL);
	at = ato;
	arptfree(at);
out:
	at->at_iaddr = *addr;
	at->at_flags = ATF_INUSE;
	return (at);
}

arpioctl(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	int             cmd;
	struct iocblk  *iocbp;
	register struct arpreq *ar;
	register struct arptab *at;
	register struct sockaddr_in *sin;
	int		error;
	register int    i;
	register int    s = SPLNULL;

	iocbp = (struct iocblk *) bp->b_rptr;
	cmd = iocbp->ioc_cmd;
	if (msgblen(bp->b_cont) != sizeof(struct arpreq)) {
		error = ENXIO;
		STRLOG(ARPM_ID, 0, 9, SL_TRACE, "arpcom: bad size for arpreq");
		goto nak;
	}
	ar = (struct arpreq *) bp->b_cont->b_rptr;


	if (ar->arp_pa.sa_family != AF_INET ||
	    ar->arp_ha.sa_family != AF_UNSPEC) {
		error = EAFNOSUPPORT;
		STRLOG(ARPM_ID, 0, 9, SL_TRACE, "arpcom: bad addr family");
		goto nak;
	}
	sin = (struct sockaddr_in *) & ar->arp_pa;
	s = splstr();
	ARPTAB_LOOK(at, sin->sin_addr.s_addr);
	if (at == NULL) {	/* not found */
		if (cmd != SIOCSARP) {
			error = ENXIO;
			STRLOG(ARPM_ID, 0, 9, SL_TRACE,
			       "arpcom: no arptab entry");
			goto nak;
		}
		for (i = 0; i < N_ARP; i++) {
			if (inet_netmatch(sin->sin_addr,
					  app_pcb[i].app_ac.ac_ipaddr))
				break;
		}
		if (i == N_ARP) {
			error = ENETUNREACH;
			STRLOG(ARPM_ID, 0, 9, SL_TRACE,
			       "arpcom: no such network assigned");
			goto nak;
		}
	}
	switch (cmd) {

	case SIOCSARP:		/* set entry */
		if (iocbp->ioc_uid != 0) {
			error = EPERM;
			setuerror(0);
			goto nak;
		}
		if (at == NULL) {
			at = arptnew(&sin->sin_addr);
			if (ar->arp_flags & ATF_PERM) {
				/*
				 * never make all entries in a bucket
				 * permanent 
				 */
				register struct arptab *tat;

				/* try to re-allocate */
				tat = arptnew(&sin->sin_addr);
				if (tat == NULL) {
					arptfree(at);
					splx(s);
					iocbp->ioc_error = EADDRNOTAVAIL;
					bp->b_datap->db_type = M_IOCNAK;
					STRLOG(ARPM_ID, 0, 9, SL_TRACE,
					       "arpcom: can't add entry");
					qreply(q, bp);
					return;
				}
				arptfree(tat);
			}
		}
		bcopy((caddr_t) ar->arp_ha.sa_data, (caddr_t) &at->at_enaddr,
		      sizeof(at->at_enaddr));
		at->at_flags = ATF_COM | ATF_INUSE |
			(ar->arp_flags & (ATF_PERM | ATF_PUBL | ATF_USETRAILERS));
		at->at_timer = 0;
		break;

	case SIOCDARP:		/* delete entry */
		if (iocbp->ioc_uid != 0) {
			error = EPERM;
			setuerror(0);
			goto nak;
		}
		arptfree(at);
		break;

	case SIOCGARP:		/* get entry */
		bcopy((caddr_t) &at->at_enaddr, (caddr_t) ar->arp_ha.sa_data,
		      sizeof(at->at_enaddr));
		ar->arp_flags = at->at_flags;
		iocbp->ioc_count = sizeof(struct arpreq);
		break;

	default:
		error = EINVAL;
		goto nak;
	}
	splx(s);
	bp->b_datap->db_type = M_IOCACK;
	qreply(q, bp);
	return;
nak:
	if (s != SPLNULL)
		splx(s);
	iocbp->ioc_error = error;
	bp->b_datap->db_type = M_IOCACK;
	qreply(q, bp);
	return;
}

/*
 * Convert Ethernet address to printable (loggable) representation. 
 */
char           *
ether_sprintf(addr)
	register ether_addr_t *addr;
{
	register u_char *ap = (u_char *)addr;
	register        i;
	static char     etherbuf[18];
	register char  *cp = etherbuf;
	static char     digits[] = "0123456789abcdef";

	for (i = 0; i < 6; i++) {
		if (*ap > 16)
			*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;
	return (etherbuf);
}

static int
memcmp(cp1, cp2, n)
	caddr_t         cp1, cp2;
{
	while (n--)
		if (*cp1++ != *cp2++)
			return (n + 1);
	return (0);
}
