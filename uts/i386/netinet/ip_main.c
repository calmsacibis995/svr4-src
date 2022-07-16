/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:ip_main.c	1.3.1.1"

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
 * This is the main stream interface module for the DoD Internet Protocol.
 * This module handles primarily OS interface issues as opposed to the actual
 * protocol isuues which are addressed in ip_input.c and ip_output.c 
 */

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <netinet/nihdr.h>
#include <sys/dlpi.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/strioc.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/ip_str.h>
#include <netinet/ip_var.h>
#ifdef SYSV
#include <sys/cmn_err.h>
#endif SYSV
#include <sys/kmem.h>

#define ipdevnum(q) (((struct ip_pcb *) ((q)->q_ptr)) - ip_pcb)

int             nodev(), putq(), ipopen(), ipclose(), ipuwput(), ipuwsrv();
int             iplwput(), iplwsrv();
int             iplrput(), ipintr();

static struct module_info ipm_info[MUXDRVR_INFO_SZ]  = {
	IPM_ID, "ip", 0, 8192, 8192, 1024,
	IPM_ID, "ip", 0, 8192, 8192, 1024,
	IPM_ID, "ip", 0, 8192, 8192, 1024,
	IPM_ID, "ip", 0, 8192, 8192, 1024,
	IPM_ID, "ip", 0, 8192, 8192, 1024
};
static struct qinit ipurinit =
{NULL, NULL, ipopen, ipclose, NULL, &ipm_info[IQP_RQ], NULL};

static struct qinit ipuwinit =
{ipuwput, ipuwsrv, ipopen, ipclose, NULL, &ipm_info[IQP_WQ], NULL};

static struct qinit iplrinit =
{iplrput, ipintr, ipopen, ipclose, NULL, &ipm_info[IQP_MUXRQ], NULL};

static struct qinit iplwinit =
{iplwput, iplwsrv, ipopen, ipclose, NULL, &ipm_info[IQP_MUXWQ], NULL};

struct streamtab ipinfo = {&ipurinit, &ipuwinit, &iplrinit, &iplwinit};

extern struct ip_pcb ip_pcb[];
extern int      ipcnt, ipprovcnt;
extern struct ip_provider provider[];
struct ip_provider *lastprov;
int             ipsubusers;
extern int      iptimerid;

unsigned char   ip_protox[IPPROTO_MAX];
struct ipstat   ipstat;
u_short         ip_id;		/* ip packet ctr, for ids */

int             ipinited;

/* configurable parameters */
extern struct ip_pcb ip_pcb[];
extern struct ip_provider provider[];
extern int ipcnt;
extern int ipprovcnt;

static int	ipversprinted;

/* ARGSUSED */
ipopen(q, dev, flag, sflag)
	queue_t        *q;
{
	struct ip_pcb  *lp;
	mblk_t         *bp;
	struct stroptions *sop;

	if (!ipversprinted) {
		ipversion();
		ipversprinted = 1;
	}
	if (!ipinited && (ipinit(), !ipinited))
		return (OPENFAIL);
	dev = minor(dev);
	if (sflag == CLONEOPEN) {
		for (dev = 0; dev < ipcnt; dev++)
			if (!(ip_pcb[dev].ip_state & IPOPEN))
				break;
	}
	if ((dev < 0) || (dev >= ipcnt)) {
		setuerror(ENXIO);
		return (OPENFAIL);
	}
	STRLOG(IPM_ID, 0, 9, SL_TRACE, "ipopen: opening dev %x", dev);

	/*
	 * Set up the correct stream head flow control parameters 
	 */
	while ((bp = allocb(sizeof(struct stroptions), BPRI_HI)) == NULL)
#if (ATT > 30) || (INTEL > 30)
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI, 3)) {
#else
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI)) {
#endif
			STRLOG(IPM_ID, 1, 2, SL_TRACE,
			       "ipopen failed: no memory for stropts");
			return (OPENFAIL);
		}

	lp = &ip_pcb[dev];
	if (!(lp->ip_state & IPOPEN)) {
		lp->ip_state = IPOPEN;
		lp->ip_rdq = q;
		q->q_ptr = (caddr_t) lp;
		WR(q)->q_ptr = (caddr_t) lp;
	} else if (q != lp->ip_rdq) {
		freeb(bp);
		setuerror(EBUSY);
		return (OPENFAIL);	/* only one stream at a time! */
	}

	bp->b_datap->db_type = M_SETOPTS;
	bp->b_wptr += sizeof(struct stroptions);
	sop = (struct stroptions *) bp->b_rptr;
	sop->so_flags = SO_HIWAT | SO_LOWAT;
	sop->so_hiwat = ipm_info[IQP_HDRQ].mi_hiwat;
	sop->so_lowat = ipm_info[IQP_HDRQ].mi_lowat;
	putnext(q, bp);
	STRLOG(IPM_ID, 0, 9, SL_TRACE, "Ipopen succeeded");
	return (dev);
}

ipclose(q)
	queue_t        *q;
{
	STRLOG(IPM_ID, 0, 9, SL_TRACE, "ipclose: closing dev %x",
	       ipdevnum(q));
	if (q->q_ptr == NULL) {
#ifdef SYSV
		cmn_err(CE_WARN, "ipclose: null q_qptr");
#else
		printf ("ipclose: null q_qptr");
#endif SYSV
		return;
	}
	((struct ip_pcb *) (q->q_ptr))->ip_state &= ~IPOPEN;
	((struct ip_pcb *) (q->q_ptr))->ip_rdq = NULL;
	flushq(WR(q), 1);
	q->q_ptr = NULL;
}


/*
 * ipuwput is the upper write put routine.  It takes messages from transport
 * protocols and decides what to do with them, controls and ioctls get
 * processed here, actual data gets queued and we let ip_output handle it. 
 */

ipuwput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct ip_pcb  *pcb = (struct ip_pcb *) q->q_ptr;
	int             i;

	STRLOG(IPM_ID, 0, 9, SL_TRACE, "ipuwput: received strbufs from above");

	switch (bp->b_datap->db_type) {

	case M_IOCTL:
		ipioctl(q, bp);
		break;

	case M_PROTO:
	case M_PCPROTO:{
			union N_primitives *op;

			op = (union N_primitives *) bp->b_rptr;
			switch (op->prim_type) {
			case N_INFO_REQ:
				op->error_ack.PRIM_type = N_ERROR_ACK;
				op->error_ack.ERROR_prim = N_INFO_REQ;
				op->error_ack.N_error = NSYSERR;
				op->error_ack.UNIX_error = ENXIO;
				qreply(q, bp);
				break;

			case N_BIND_REQ: {
				unsigned long    sap = op->bind_req.N_sap;
				
				for (i = 0; i < ipcnt; i++) {
					if ((ip_pcb[i].ip_state & IPOPEN)
					    && (ip_pcb[i].ip_proto == sap)) {
						op->error_ack.PRIM_type = N_ERROR_ACK;
						op->error_ack.ERROR_prim = N_BIND_REQ;
						op->error_ack.N_error = NBADSAP;
						qreply(q, bp);
						break;
					}
				}
				if (!sap || sap >= IPPROTO_MAX) {
					op->error_ack.PRIM_type
						= N_ERROR_ACK;
					op->error_ack.ERROR_prim
						= N_BIND_REQ;
					op->error_ack.N_error = NBADSAP;
					qreply(q, bp);
					return;
				}
                                if (sap == IPPROTO_RAW) {
                                        for (i=0; i<IPPROTO_MAX; i++)
						if (ip_protox[i] == ipcnt)
                                                        ip_protox[i] =
								ipdevnum(q);
				} else
					ip_protox[sap] = ipdevnum(q);
				pcb->ip_proto = sap;
				op->bind_ack.PRIM_type = N_BIND_ACK;
				op->bind_ack.N_sap = sap;
				op->bind_ack.ADDR_length = 0;
				qreply(q, bp);
				break;
			}

			case N_UNBIND_REQ:
                                if (pcb->ip_proto == IPPROTO_RAW) {
                                        for (i=0; i<IPPROTO_MAX; i++)
                                                if (ip_protox[i] == ipdevnum(q))
                                                        ip_protox[i] = ipcnt;
				} else
					if (pcb->ip_proto)
						ip_protox[pcb->ip_proto] =
							ip_protox[IPPROTO_RAW];
				pcb->ip_proto = 0;
				op->ok_ack.PRIM_type = N_OK_ACK;
				op->ok_ack.CORRECT_prim =
					N_UNBIND_REQ;
				qreply(q, bp);
				break;

			case N_UNITDATA_REQ:
				putq(q, bp);
				break;

			default:
				STRLOG(IPM_ID, 3, 5, SL_ERROR,
				       "ipuwput: unrecognized prim: %d", op->prim_type);
				freemsg(bp);
				break;
			}
		}
		break;

	case M_CTL:
		sendctl(bp);
		break;

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW) {
			flushq(q, FLUSHALL);
			*bp->b_rptr &= ~FLUSHW;
		}
		if (*bp->b_rptr & FLUSHR)
			qreply(q, bp);
		else
			freemsg(bp);
		break;

	default:
		STRLOG(IPM_ID, 0, 5, SL_ERROR,
		       "IP: unexpected type received in wput: %d.",
		       bp->b_datap->db_type);
		freemsg(bp);
		break;
	}
}


ipioctl(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct iocblk  *iocbp;
	int             i;
	register struct ip_provider *prov;

	iocbp = (struct iocblk *) bp->b_rptr;

	/* screen out routing ioctls */
	if (((iocbp->ioc_cmd >> 8) & 0xFF) == 'r') {
		if (bp->b_cont == NULL) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}
		iocbp->ioc_error = rtioctl(iocbp->ioc_cmd, bp->b_cont);
		bp->b_datap->db_type = iocbp->ioc_error ? M_IOCNAK : M_IOCACK;
		qreply(q, bp);
		return;
	}
	switch (iocbp->ioc_cmd) {
	case I_PLINK:
	case I_LINK:
		STRLOG(IPM_ID, 0, 9, SL_TRACE,
		       "ipioctl: linking new provider");
		iocbp->ioc_error = 0;
		iocbp->ioc_rval = 0;
		iocbp->ioc_count = 0;
		for (i = 0; i < ipprovcnt; i++) {
			if (provider[i].qbot == NULL) {
				prov = &provider[i];
				break;
			}
		}
		if (i == ipprovcnt) {
			iocbp->ioc_error = ENOSR;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(IPM_ID, 0, 9, SL_TRACE,
			       "I_LINK failed: no provider slot available");
			qreply(q, bp);
			return;
		} else {
			struct linkblk	*lp;
			dl_bind_req_t	*bindr;
			mblk_t		*nbp;

			if (prov > lastprov)
				lastprov = prov;
			lp = (struct linkblk *) bp->b_cont->b_rptr;
			bzero(&provider[i], sizeof(struct ip_provider));
			prov->qbot = lp->l_qbot;
			prov->qbot->q_ptr = (char *) prov;
			RD(prov->qbot)->q_ptr = (char *) prov;
			prov->l_index = lp->l_index;
			prov->ia.ia_ifa.ifa_addr.sa_family = AF_INET;
			if ((nbp = allocb(sizeof(dl_bind_req_t), BPRI_HI))
			    == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(IPM_ID, 0, 9, SL_TRACE,
				     "I_LINK failed: Can't alloc bind buf");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(dl_bind_req_t);
			bindr = (dl_bind_req_t *) nbp->b_rptr;
			bindr->dl_primitive = DL_BIND_REQ;
			bindr->dl_sap = IP_SAP;
			putnext(prov->qbot, nbp);
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(IPM_ID, 0, 9, SL_TRACE, "I_LINK succeeded");
			qreply(q, bp);
			return;
		}
	case I_PUNLINK:
	case I_UNLINK:
		{
			struct linkblk	*lp;
			mblk_t		*nbp;
			dl_unbind_req_t	*bindr;

			iocbp->ioc_error = 0;
			iocbp->ioc_rval = 0;
			iocbp->ioc_count = 0;
			lp = (struct linkblk *) bp->b_cont->b_rptr;
			for (prov = provider; prov <= lastprov; prov++) {
				if (prov->qbot &&
				    prov->l_index == lp->l_index) {
					break;
				}
			}
			if (prov > lastprov) {
				iocbp->ioc_error = ENXIO;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(IPM_ID, 0, 9, SL_TRACE,
				    "I_UNLINK: no provider with index = %x",
				       lp->l_index);
				qreply(q, bp);
				return;
			}
			/* Do the link level unbind */

			if ((nbp = allocb(sizeof(dl_unbind_req_t),
					  BPRI_HI)) == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(IPM_ID, 0, 9, SL_TRACE,
				       "I_UNLINK: no buf for unbind");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(dl_unbind_req_t);
			bindr = (dl_unbind_req_t *) nbp->b_rptr;
			bindr->dl_primitive = DL_UNBIND_REQ;
			putnext(lp->l_qbot, nbp);

			if (prov->ia.ia_ifa.ifa_ifs != NULL) {
				if (prov->ia.ia_ifa.ifa_ifs->ifs_addrs ==
				    &prov->ia.ia_ifa) {
					prov->ia.ia_ifa.ifa_ifs->ifs_addrs =
						prov->ia.ia_ifa.ifa_next;
				} else {
					struct ifaddr  *ifa;

					for (ifa =
					 prov->ia.ia_ifa.ifa_ifs->ifs_addrs;
					     ifa && ifa->ifa_next !=
					     &prov->ia.ia_ifa;
					     ifa = ifa->ifa_next);
					if (ifa == NULL) {
						STRLOG(IPM_ID, 0, 9,
						       SL_ERROR, "ifaddr chain corrupt");
					} else {
						ifa->ifa_next = prov->ia.ia_ifa.ifa_next;
					}
				}
			}
			prov->qbot = NULL;
			prov->l_index = 0;
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(IPM_ID, 0, 9, SL_TRACE, "I_UNLINK succeeded");
			qreply(q, bp);
			return;
		}

	case SIOCGIFCONF:	/* return provider configuration */
		ifconf(q, bp);
		return;

	case INITQPARMS:
		if (iocbp->ioc_error = initqparms(bp, ipm_info, MUXDRVR_INFO_SZ))
			bp->b_datap->db_type = M_IOCNAK;
		else
			bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		return;

	default:
		in_control(q, bp);
		return;
	}
}

static
ifconf(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct iocblk  *ioc = (struct iocblk *) bp->b_rptr;
	register struct ifreq *ifr;
	register int    space;
	register struct ip_provider *prov;

	if (bp->b_cont == NULL) {
		bp->b_datap->db_type = M_IOCNAK;
		ioc->ioc_error = EINVAL;
		qreply(q, bp);
		return;
	}
	ifr = (struct ifreq *) bp->b_cont->b_rptr;
	space = msgblen(bp->b_cont);

	for (prov = provider; prov <= lastprov && space > sizeof(struct ifreq);
	     prov++) {
		if (prov->qbot == NULL) {
			continue;
		}
		bcopy(prov->name, ifr->ifr_name, sizeof(ifr->ifr_name));
		ifr->ifr_addr = prov->if_addr;
		space -= sizeof(struct ifreq);
		ifr++;
	}
	bp->b_datap->db_type = M_IOCACK;
	bp->b_cont->b_wptr -= space;
	ioc->ioc_count = msgblen(bp->b_cont);
	qreply(q, bp);
}

ipinit()
{
	register int    i;

	STRLOG(IPM_ID, 0, 9, SL_TRACE, "ipinit starting");

	if (ipinited)
		return;
#ifdef LATER
	if (!ipcnt)
		ipcnt = NIP;
	if (!ipprovcnt)
		ipprovcnt = IP_PROVIDERS;
	ip_pcb = (struct ip_pcb *)
		kmem_alloc((int) (ipcnt * sizeof(struct ip_pcb)), KM_NOSLEEP);
	provider = (struct ip_provider *) kmem_alloc((int) (ipprovcnt *
					       sizeof(struct ip_provider)),
					       KM_NOSLEEP);
#endif
	lastprov = provider;
	if (ip_pcb == NULL || provider == NULL) {
		/* == NULL if space unavailable */
		STRLOG(IPM_ID, 0, 9, SL_TRACE,
		       "ipinit: Can't get memory for control structures");
		setuerror(ENOSR);
		return;
	}
	bzero(provider, ipprovcnt * sizeof(struct ip_provider));
	ip_id = hrestime.tv_sec & 0xffff;
	for (i = 0; i < IPPROTO_MAX; i++) {
		ip_protox[i] = ipcnt;
	}
	ipq_setup();
	ip_slowtimo();
	ipinited = 1;
	icmpinit();		/* piggyback module */
	STRLOG(IPM_ID, 0, 9, SL_TRACE, "ipinit succeeded");
}

/*
 * iplrput is the lower read put routine.  It takes packets and examines
 * them.  Control packets are dealt with right away and data packets are
 * queued for ip_input to deal with.  The message formats understood by the
 * M_PROTO messages here are those used by the link level interface (see
 * dlpi.h). 
 */

iplrput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	union DL_primitives *op;

	switch (bp->b_datap->db_type) {
	case M_DATA:		/* Can't send pure data through LLC layer */
		freemsg(bp);
		break;

	case M_IOCACK:		/* ioctl's returning from link layer */
	case M_IOCNAK:
		in_upstream(q, bp);
		break;

	case M_PROTO:
	case M_PCPROTO:
		op = (union DL_primitives *) bp->b_rptr;
		switch (op->dl_primitive) {
		case DL_INFO_ACK:
			((struct ip_provider *) q->q_ptr)->if_maxtu =
				op->info_ack.dl_max_sdu;
			((struct ip_provider *) q->q_ptr)->if_mintu =
				op->info_ack.dl_min_sdu;
			STRLOG(IPM_ID, 0, 9, SL_TRACE, "Got Info ACK");
			freemsg(bp);
			break;

		case DL_BIND_ACK:
			STRLOG(IPM_ID, 0, 9, SL_TRACE, "got bind ack");
			bp->b_datap->db_type = M_PCPROTO;
			bp->b_wptr = bp->b_rptr + DL_INFO_REQ_SIZE;
			op->dl_primitive = DL_INFO_REQ;
			qreply(q, bp);
			break;

		case DL_ERROR_ACK:
			STRLOG(IPM_ID, 0, 9, SL_TRACE,
			       "ERROR ACK: prim = %d, net error = %d, unix error = %d",
			       op->error_ack.dl_error_primitive,
			       op->error_ack.dl_errno,
			       op->error_ack.dl_unix_errno);
			freemsg(bp);
			break;

		case DL_OK_ACK:
			STRLOG(IPM_ID, 0, 9, SL_TRACE,
			       "Got OK ack, prim = %x",
			       op->ok_ack.dl_correct_primitive);
			freemsg(bp);
			break;

		case DL_UNITDATA_IND:
			putq(q, bp);
			break;

		case DL_UDERROR_IND:
			STRLOG(IPM_ID, 0, 9, SL_TRACE,
			       "Link level error, type = %x",
			       op->uderror_ind.dl_errno);
			freemsg(bp);
			break;

		default:
			STRLOG(IPM_ID, 3, 5, SL_ERROR,
			   "iplrput: unrecognized prim: %d", op->dl_primitive);
			freemsg(bp);
			break;
		}
		break;

	case M_FLUSH:
		/*
		 * Flush read queue free msg (can't route upstream) 
		 */
		STRLOG(IPM_ID, 0, 9, SL_TRACE, "Got flush message type = %x",
		       *bp->b_rptr);
		if (*bp->b_rptr & FLUSHR)
			flushq(q, FLUSHALL);
		if (*bp->b_rptr & FLUSHW) {
			*bp->b_rptr &= ~FLUSHR;
			flushq(WR(q), FLUSHALL);
			qreply(q, bp);
		} else
			freemsg(bp);
		return;

	case M_ERROR: {
		/*
		 * Fatal error - mark interface down
		 */
		struct ip_provider *prov =
			(struct ip_provider *) q->q_ptr;
		prov->if_flags &= ~IFF_UP;
#ifdef SYSV
		cmn_err(CE_NOTE,
		    "IP: Fatal error (%d) on interface %s, marking down.\n",
		    (int) *bp->b_rptr, prov->name);
#else
		printf (
		    "IP: Fatal error (%d) on interface %s, marking down.\n",
		    (int) *bp->b_rptr, prov->name);
#endif SYSV
		freemsg(bp);
		break;
	}
	default:
		STRLOG(IPM_ID, 3, 5, SL_ERROR,
		       "IP: unexpected type received in rput: %d.",
		       bp->b_datap->db_type);
		freemsg(bp);
		break;
	}
}

/*
 * sendctl sends control messages (generated by icmp) to any or all of our
 * clients.  It does this by dup'ing the message a whole bunch of times. 
 */

sendctl(bp)
	mblk_t         *bp;
{
	mblk_t         *newbp;
	struct ip_ctlmsg *ipctl;
	register int    i;
	queue_t        *qp;

	ipctl = (struct ip_ctlmsg *) bp->b_rptr;
	if (ipctl->proto == -1) {
		for (i = 0; i < ipcnt; i++) {
			if ((ip_pcb[i].ip_state & IPOPEN)
			    && (newbp = dupmsg(bp))) {
				putnext(ip_pcb[i].ip_rdq, newbp);
			}
		}
		freemsg(bp);
	} else {
		if (ip_protox[ipctl->proto] == ipcnt) {
			freemsg(bp);
			return;
		}
		qp = ip_pcb[ip_protox[ipctl->proto]].ip_rdq;
		if (!canput(qp->q_next)) {
			freemsg(bp);
			return;
		}
		putnext(qp, bp);
	}
}

void
ipregister()
{
	ipsubusers++;
}

void
ipderegister()
{
	ipsubusers--;
}


#ifndef SYSV
int strlogprintf = 0;

/*VARARGS*/
strlog (mid, sid, level, flags, fmt, a0, a1, a2, a3, a4, a5, a6,a7, a8, a9)
	int mid, sid, level, flags;
	char *fmt;
	char *a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9;
{
	if (strlogprintf) {
		printf (fmt, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
		printf ("\n");
	}
}
#endif SYSV
