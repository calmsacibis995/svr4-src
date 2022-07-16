/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-io:ptm.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/ptms.h"

#ifdef DBUG
#define DBG(a)	 if (ptm_debug) cmn_err( CE_NOTE, a) 
#else
#define DBG(a)
#endif 

int ptm_debug = 0;


/*
 *   Master Stream Pipe
 */

STATIC struct module_info ptm_info = {0xdead, "ptm", 0, 512, 512, 128};
STATIC int ptmopen(), ptmclose(), ptmwput(), ptmrsrv(), ptmwsrv();
STATIC struct qinit ptmrint = {NULL, ptmrsrv, ptmopen, ptmclose,NULL,&ptm_info, NULL};
STATIC struct qinit ptmwint = {ptmwput, ptmwsrv, NULL, NULL, NULL,&ptm_info, NULL};
struct streamtab ptminfo = {&ptmrint, &ptmwint, NULL, NULL};

/*
 * ptms_tty[] and pt_cnt are defined in master.d file 
 */
extern struct pt_ttys ptms_tty[];	
extern int pt_cnt;

/*
 * Open a minor of the master device. Find an unused entry in the
 * ptms_tty array. Store the write queue pointer and set the
 * pt_state field to (PTMOPEN | PTLOCK).
 */
/* ARGSUSED */
STATIC int 
ptmopen(rqp, dev, flag, sflag)
queue_t *rqp;
dev_t dev;
int flag;
int sflag;
{
        register struct pt_ttys *ptmp;
	register mblk_t *mop;		/* Pointer to a setopts message block */


	DBG(("entering ptmopen\n"));
	for(dev=0; dev < pt_cnt; dev++)
		if (!(ptms_tty[dev].pt_state & 
			(PTMOPEN | PTSOPEN | PTLOCK)))
				break;

	if (sflag != CLONEOPEN) {
		DBG(("invalid sflag\n"));
		u.u_error = EINVAL;
		return(OPENFAIL);
	}

        if (dev >= pt_cnt) {
		DBG(("no more devices left to allocate\n"));
		u.u_error = ENODEV;
                return(OPENFAIL);
	}

        ptmp = &ptms_tty[dev];
	if (ptmp->pts_wrq)
		if ((ptmp->pts_wrq)->q_next) {
			DBG(("send hangup to an already existing slave\n"));
			putctl(RD(ptmp->pts_wrq)->q_next, M_HANGUP);
		}
	/*
	 * set up hi/lo water marks on stream head read queue
	 * and add controlling tty if not set
	 */
	if ( mop = allocb( sizeof( struct stroptions), BPRI_MED)) {
		register struct stroptions *sop;
		
		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
		sop->so_hiwat = 512;
		sop->so_lowat = 256;
		putnext( rqp, mop);
	} else {
		u.u_error = EAGAIN;
		return ( OPENFAIL);
	}

	/*
	 * set up the entries in the pt_ttys structure for this
	 * device.
	 */
	ptmp->pt_state = (PTMOPEN | PTLOCK);
	ptmp->ptm_wrq = WR(rqp);
	ptmp->pts_wrq = NULL;
	ptmp->tty = u.u_procp->p_pgrp;
	ptmp->pt_bufp = NULL;
	WR(rqp)->q_ptr = (char *) ptmp;
	rqp->q_ptr = (char *) ptmp;

	DBG(("returning from ptmopen()\n"));
	return(dev);
}


/*
 * Find the address to private data identifying the slave's write queue.
 * Send a hang-up message up the slave's read queue to designate the
 * master/slave pair is tearing down. Uattach the master and slave by 
 * nulling out the write queue fields in the private data structure.  
 * Finally, unlock the master/slave pair and mark the master as closed.
 */
STATIC int 
ptmclose(rqp)
queue_t *rqp;
{
        register struct pt_ttys *ptmp;
        register queue_t *pts_rdq;

	ASSERT(rqp->q_ptr);
		
	DBG(("entering ptmclose\n"));
        ptmp = (struct pt_ttys *)rqp->q_ptr;
	if (ptmp->pts_wrq) {
		pts_rdq = RD(ptmp->pts_wrq);
		if(pts_rdq->q_next) {
			DBG(("send hangup message to slave\n"));
			putctl(pts_rdq->q_next, M_HANGUP); 
		}
	}
	freemsg(ptmp->pt_bufp);
	ptmp->pt_bufp = NULL;
        ptmp->ptm_wrq = NULL;
	ptmp->pt_state &= ~(PTMOPEN | PTLOCK);
	ptmp->tty = 0;
	rqp->q_ptr = NULL;
	WR(rqp)->q_ptr = NULL;

	DBG(("returning from ptmclose\n"));
	return( 0);
}

/*
 * The wput procedure will only handle ioctl and flush messages.
 */
STATIC int 
ptmwput(qp, mp)
queue_t *qp;
mblk_t *mp;
{
        register struct pt_ttys *ptmp;
	struct iocblk *iocp;
	
	DBG(("entering ptmwput\n"));
	ASSERT(qp->q_ptr);

        ptmp = (struct pt_ttys *) qp->q_ptr;
		
	switch(mp->b_datap->db_type) {
		/*
		 * if write queue request, flush master's write
		 * queue and send FLUSHR up slave side. If read 
		 * queue request, convert to FLUSHW and putctl1().
		 */
		case M_FLUSH:
			DBG(("ptm got flush request\n"));
			if (*mp->b_rptr & FLUSHW) {
				DBG(("flush ptm write Q\n"));
				flushq(qp, FLUSHDATA);
				if(ptmp->pts_wrq && !(ptmp->pt_state & PTLOCK)) {
					DBG(("putctl1 FLUSHR to pts\n"));
					putctl1(RD(ptmp->pts_wrq)->q_next, M_FLUSH, FLUSHR);
				}
			}
			if (*mp->b_rptr & FLUSHR) {
				DBG(("got read, putctl1 FLUSHW\n"));
				if(ptmp->pts_wrq && !(ptmp->pt_state & PTLOCK))
					putctl1(RD(ptmp->pts_wrq)->q_next, M_FLUSH, FLUSHW);
			}
			freemsg(mp);
			break;

		case M_IOCTL:
			iocp = (struct iocblk *)mp->b_rptr;
			switch(iocp->ioc_cmd) {
			default:
				 if((ptmp->pt_state  & PTLOCK) || 
				    (ptmp->pts_wrq == NULL)) {
					DBG(("got M_IOCTL but no slave\n"));
					mp->b_datap->db_type = M_IOCNAK;
					freemsg(mp->b_cont);
					mp->b_cont = NULL;
					qreply(qp, mp);
					return( 0);
				}
				putq(qp, mp);
				break;
			case ISPTM:
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
				DBG(("ack the ISPTM\n"));
				qreply(qp, mp);
				break;
			case UNLKPT:
				ptmp->pt_state &= ~PTLOCK;
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_error = 0;
				iocp->ioc_count = 0;
				DBG(("ack the UNLKPT\n"));
				qreply(qp, mp);
				break;
			}
			break;
		
		/*
		 * send other messages to slave
		 */
		default:
			if((ptmp->pt_state  & PTLOCK) || (ptmp->pts_wrq == NULL)) {
				DBG(("got msg. but no slave\n"));
				putctl1(RD(qp)->q_next, M_ERROR,EINVAL);
				freemsg(mp);
				return( 0);
			}
			DBG(("put msg on master's write queue\n"));
			putq(qp, mp);
			break;
	}
	DBG(("return from ptmwput()\n"));
	return( 0);
}


/*
 * enable the write side of the slave. This triggers the 
 * slave to send any messages queued on its write side to
 * the read side of this master.
 */
STATIC int 
ptmrsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptmp;
	
	DBG(("entering ptmrsrv\n"));
	ASSERT(qp->q_ptr);

        ptmp = (struct pt_ttys *) qp->q_ptr;
	if ( ptmp->pts_wrq)
		qenable( ptmp->pts_wrq);
	DBG(("leaving ptmrsrv\n"));
	return( 0);
}
		


/*
 * If there are messages on this queue that can be sent to 
 * slave, send them via putnext(). Else, if queued messages 
 * cannot be sent, leave them on this queue. If priority 
 * messages on this queue, send them to slave no matter what.
 */
STATIC int 
ptmwsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptmp;
        register queue_t *pts_rdq;
	mblk_t *mp;
	
	DBG(("entering ptmwsrv\n"));
	ASSERT(qp->q_ptr);

        ptmp = (struct pt_ttys *) qp->q_ptr;
	if (ptmp->pts_wrq)
		pts_rdq = RD(ptmp->pts_wrq);
	else
		pts_rdq = NULL;
		
	if((ptmp->pt_state  & PTLOCK) || (pts_rdq == NULL)) {
		DBG(("in master write srv proc but no slave\n"));
		putctl1(RD(qp)->q_next, M_ERROR, EINVAL);
		return( 0);
	}
	/*
	 * while there are messages on this write queue...
	 */
	while ((mp = getq(qp)) != NULL) {
		/*
		 * if don't have control message and cannot put
		 * msg. on slave's read queue, put it back on 
		 * this queue.
		 */
		if (mp->b_datap->db_type <= QPCTL && 
				!canput(pts_rdq->q_next)) {
			DBG(("put msg. back on queue\n"));
			putbq(qp, mp);
			break;
		}
		/*
		 * else send the message up slave's stream
		 */
		DBG(("send message to slave\n"));
		putnext(pts_rdq, mp);
	}
	DBG(("leaving ptmwsrv\n"));
	return( 0);
}
