/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:llcloop.c	1.3"

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
 * This is the module which implements the link level loopback driver. 
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/signal.h>
#ifdef SYSV
#include <sys/cred.h>
#include <sys/proc.h>
#endif /* SYSV */
#include <sys/user.h>		/* XXX - TOM */
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <netinet/nihdr.h>
#include <sys/dlpi.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/if.h>
#include <net/strioc.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/llcloop.h>
#include <sys/kmem.h>

int             nodev(), loopopen(), loopclose(), loopuwput();


static struct module_info loopm_info[DRVR_INFO_SZ] = {
	LOOPM_ID, "llc_loop", 0, 2048, 8192, 1024,
	LOOPM_ID, "llc_loop", 0, 2048, 8192, 1024,
	LOOPM_ID, "llc_loop", 0, 2048, 8192, 1024
};

static struct qinit loopurinit =
{NULL, NULL, loopopen, loopclose, NULL, &loopm_info[IQP_RQ], NULL};

static struct qinit loopuwinit =
{loopuwput, NULL, loopopen, loopclose, NULL, &loopm_info[IQP_WQ], NULL};

struct streamtab loopinfo = {&loopurinit, &loopuwinit, NULL, NULL};

extern struct ifstats *ifstats;

int             loopinited;

/* configurable parameters */
extern int loopcnt;
extern struct ifstats loopstats[];
extern struct loop_pcb loop_pcb[];

/*
 * These are the stream module routines for ip loopback 
 */


/* ARGSUSED */
loopopen(q, dev, flag, sflag)
	queue_t        *q;
{
	struct loop_pcb *lp;
	mblk_t         *bp;
	struct stroptions *sop;

	if (!loopinited && (loopinit(), !loopinited))
		return (OPENFAIL);
	dev = minor(dev);
	if (sflag == CLONEOPEN) {
		for (dev = 0; dev < loopcnt; dev++)
			if (loop_pcb[dev].loop_qtop == NULL)
				break;
	}
	if ((dev < 0) || (dev >= loopcnt)) {
		setuerror(ENXIO);
		return (OPENFAIL);
	}

	STRLOG(LOOPM_ID, 0, 9, SL_TRACE, "loopopen: opening dev %x", dev);

	/*
	 * Set up the correct stream head flow control parameters 
	 */
	while ((bp = allocb(sizeof(struct stroptions), BPRI_HI)) == NULL)
#if (ATT > 30) || (INTEL > 30)
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI, 3)) {
#else
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI)) {
#endif
			STRLOG(LOOPM_ID, 0, 9, SL_TRACE,
	       			"loopopen failed: no memory for stropts");
			return (OPENFAIL);
		}

	lp = &loop_pcb[dev];
	if (lp->loop_qtop == NULL) {
		lp->loop_state = 0;	/* until bindreq */
		lp->loop_qtop = q;
		q->q_ptr = (caddr_t) lp;
		OTHERQ(q)->q_ptr = (caddr_t) lp;
	} else if (q != lp->loop_qtop) {
		freeb(bp);
		setuerror(EBUSY);
		return (OPENFAIL);	/* only one stream at a time! */
	}
	if (!loopstats[dev].ifs_active) {
		loopstats[dev].ifs_name = "lo";
		loopstats[dev].ifs_active = 1;	/* Always up! */
		loopstats[dev].ifs_unit = dev;
		loopstats[dev].ifs_next = ifstats;
		loopstats[dev].ifs_mtu = loopm_info[IQP_WQ].mi_maxpsz;
		ifstats = &loopstats[dev];
	}

	bp->b_datap->db_type = M_SETOPTS;
	bp->b_wptr += sizeof(struct stroptions);
	sop = (struct stroptions *) bp->b_rptr;
	sop->so_flags = SO_HIWAT | SO_LOWAT;
	sop->so_hiwat = loopm_info[IQP_HDRQ].mi_hiwat;
	sop->so_lowat = loopm_info[IQP_HDRQ].mi_lowat;
	putnext(q, bp);
	STRLOG(LOOPM_ID, 0, 9, SL_TRACE, "loopopen succeeded");
	return (dev);
}

loopclose(q)
	queue_t        *q;
{
	struct	ifstats	*ifsp;
	int	dev = 0;
	STRLOG(LOOPM_ID, 0, 9, SL_TRACE, "loopclose: closing dev %x",
	       ((struct loop_pcb *) (q->q_ptr)) - loop_pcb);
	((struct loop_pcb *) (q->q_ptr))->loop_qtop = NULL;
	flushq(WR(q), 1);

	dev = (struct loop_pcb *)q->q_ptr - loop_pcb;

	q->q_ptr = NULL;
	WR(q)->q_ptr = NULL;
	/*
	 * Remove us from stats list.
	 */

	ifsp = ifstats;
	while (ifsp) {
		if (ifsp->ifs_next != &loopstats[dev]) {
			ifsp = ifsp->ifs_next;
			continue;
		} else {
			ifsp->ifs_next = loopstats[dev].ifs_next;
			break;
		}
	}
}


loopuwput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{

	STRLOG(LOOPM_ID, 0, 9, SL_TRACE, "loopuwput: received strbufs from above");

	switch (bp->b_datap->db_type) {

	case M_IOCTL:
		loopioctl(q, bp);
		break;

	case M_PROTO:
	case M_PCPROTO:
		STRLOG(LOOPM_ID, 0, 9, SL_TRACE, "passing data through loop");
		loop_doproto(q, bp);
		break;

	case M_CTL:
		freemsg(bp);
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
		freemsg(bp);
		break;
	}
}


loopioctl(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct iocblk  *iocbp;

	iocbp = (struct iocblk *) bp->b_rptr;

	switch (iocbp->ioc_cmd) {

	case INITQPARMS:
		if (iocbp->ioc_error = initqparms(bp, loopm_info, MUXDRVR_INFO_SZ))
			bp->b_datap->db_type = M_IOCNAK;
		else
			bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		return;

	default:
	/*
	 * This is here so that we don't need a convergence module for IP. 
	 */
		((struct iocblk_in *) iocbp)->ioc_ifflags |= 
						IFF_LOOPBACK | IFF_RUNNING;
		bp->b_datap->db_type = M_IOCACK;
		qreply(q, bp);
		return;
	}
}

loop_doproto(q, bp)
	queue_t		*q;
	mblk_t		*bp;
{
	register union DL_primitives *prim;
	int	dev;
	mblk_t		*respbp, *infobp, *bindbp;
	dl_ok_ack_t	*ok_ack;
	dl_info_ack_t	*info_ack;
	register dl_unitdata_ind_t *ind;
	dl_bind_ack_t	*bind_ack;
	mblk_t		*hdr;
	struct loop_pcb *pcb = (struct loop_pcb *) q->q_ptr;


	prim = (union DL_primitives *) bp->b_rptr;

	switch (prim->dl_primitive) {
	case DL_INFO_REQ:
		if ((infobp = allocb(sizeof(dl_info_ack_t), BPRI_HI))
		    == NULL) {
			loop_error(q, prim->dl_primitive, DL_SYSERR, ENOSR);
			freemsg(bp);
			return;
		}
		info_ack = (dl_info_ack_t *) infobp->b_rptr;
		infobp->b_wptr += sizeof(dl_info_ack_t);
		infobp->b_datap->db_type = M_PROTO;
		info_ack->dl_primitive = DL_INFO_ACK;
		info_ack->dl_max_sdu = q->q_maxpsz;
		info_ack->dl_min_sdu = q->q_minpsz;
		info_ack->dl_addr_length = sizeof(int);
		info_ack->dl_mac_type = DL_ETHER;
		info_ack->dl_current_state = pcb->loop_state == 0 ? DL_UNBOUND
			: DL_IDLE;
		info_ack->dl_service_mode = DL_CLDLS;
		info_ack->dl_provider_style = DL_STYLE1;
		freemsg(bp);
		qreply(q, infobp);
		return;		/* Avoid OK_ACK */

	case DL_UNITDATA_REQ:
		if (pcb->loop_state == 0) {
			prim->dl_primitive = DL_UDERROR_IND;
			prim->uderror_ind.dl_errno = EPROTO;
			qreply(q, bp);
			return;
		}
		hdr = allocb((int) (sizeof(dl_unitdata_ind_t) +
				    2 * prim->unitdata_req.dl_dest_addr_length),
			     BPRI_HI);
		if (!hdr) {
			freemsg(bp);
			loop_error(q, DL_UNITDATA_REQ, DL_SYSERR, ENOSR);
			return;
		}
		hdr->b_wptr += sizeof(dl_unitdata_ind_t) +
			2 * prim->unitdata_req.dl_dest_addr_length;
		hdr->b_datap->db_type = M_PROTO;
		ind = (dl_unitdata_ind_t *) hdr->b_rptr;
		ind->dl_primitive = DL_UNITDATA_IND;
		ind->dl_dest_addr_offset = sizeof(dl_unitdata_ind_t);
		ind->dl_dest_addr_length = 
			prim->unitdata_req.dl_dest_addr_length;
		ind->dl_src_addr_offset = sizeof(dl_unitdata_ind_t) +
			prim->unitdata_req.dl_dest_addr_length;
		ind->dl_src_addr_length = 
			prim->unitdata_req.dl_dest_addr_length;
		bcopy((char *) bp->b_rptr + 
		      prim->unitdata_req.dl_dest_addr_offset,
		      (char *) hdr->b_rptr + ind->dl_dest_addr_offset,
		      (unsigned) prim->unitdata_req.dl_dest_addr_length);
		bcopy((char *) bp->b_rptr + 
		      prim->unitdata_req.dl_dest_addr_offset,
		      (char *) hdr->b_rptr + ind->dl_src_addr_offset,
		      (unsigned) prim->unitdata_req.dl_dest_addr_length);
		hdr->b_cont = bp->b_cont;
		freeb(bp);
		qreply(q, hdr);
		dev = (struct loop_pcb *)q->q_ptr - loop_pcb;
		loopstats[dev].ifs_opackets++;
		loopstats[dev].ifs_ipackets++;
		return;		/* Avoid OK_ACK */

	case DL_BIND_REQ:
		if (pcb->loop_state == 1) {	/* Already bound */
			freemsg(bp);
			loop_error(q, DL_BIND_REQ, DL_OUTSTATE, 0);
			return;
		}
		if ((bindbp = allocb(sizeof(dl_bind_ack_t), BPRI_HI))
		    == NULL) {
			freemsg(bp);
			loop_error(q, DL_BIND_REQ, DL_SYSERR, ENOSR);
			return;
		}
		bind_ack = (dl_bind_ack_t *) bindbp->b_rptr;
		bindbp->b_wptr += sizeof(dl_bind_ack_t);
		bindbp->b_datap->db_type = M_PROTO;
		bind_ack->dl_primitive = DL_BIND_ACK;
		bind_ack->dl_addr_length = 0;
		bind_ack->dl_addr_offset = sizeof(dl_bind_ack_t);
		bind_ack->dl_sap = prim->bind_req.dl_sap;
		freemsg(bp);
		pcb->loop_state = 1;	/* BOUND!!! */
		qreply(q, bindbp);
		return;		/* Avoid OK_ACK */

	case DL_UNBIND_REQ:
		if (pcb->loop_state == 0) {
			freemsg(bp);
			loop_error(q, DL_BIND_REQ, DL_OUTSTATE, 0);
			return;
		}
		pcb->loop_state = 0;
		break;

	default:
		freemsg(bp);
		loop_error(q, DL_BIND_REQ, DL_SYSERR, EINVAL);
		return;
	}
	respbp = allocb(sizeof(dl_ok_ack_t), BPRI_HI);
	if (respbp) {
		ok_ack = (dl_ok_ack_t *) respbp->b_rptr;
		respbp->b_wptr += sizeof(dl_ok_ack_t);
		respbp->b_datap->db_type = M_PCPROTO;
		ok_ack->dl_primitive = DL_OK_ACK;
		ok_ack->dl_correct_primitive = prim->dl_primitive;
		qreply(q, respbp);
	}
	freemsg(bp);
}

loop_error(q, prim, dl_err, sys_err)
   queue_t	*q;
   long		prim;
   int		dl_err, sys_err;
{
	mblk_t		*errbp;
	dl_error_ack_t	*error_ack;

	errbp = allocb(sizeof(dl_error_ack_t), BPRI_HI);
	if (errbp) {
		error_ack = (dl_error_ack_t *) errbp->b_rptr;
		errbp->b_wptr += sizeof(dl_error_ack_t);
		errbp->b_datap->db_type = M_PCPROTO;
		error_ack->dl_primitive = DL_ERROR_ACK;
		error_ack->dl_error_primitive = prim;
		error_ack->dl_errno = dl_err;
		error_ack->dl_unix_errno = sys_err;
		qreply(q, errbp);
	}
}

loopinit()
{
#ifdef LATER
	loop_pcb = (struct loop_pcb *) kmem_alloc(sizeof(struct loop_pcb)
						  * loopcnt, KM_NOSLEEP);
#endif
	if (loop_pcb == NULL) {
		u.u_error = ENOMEM;
		return;
	}
	bzero((char *) loop_pcb, sizeof(struct loop_pcb) * loopcnt);
	bzero((char *) &loopstats, sizeof(struct ifstats) * loopcnt);
	loopinited = 1;
}
