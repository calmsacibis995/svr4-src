/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)kern-io:pts.c	1.3"

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
int pts_debug = 0;
#define DBG(a)	 if (pts_debug) cmn_err( CE_NOTE, a) 
#else
#define DBG(a)
#endif 


/*
 *   Slave Stream Pseudo Terminal
 */

STATIC struct module_info pts_info = {0xface, "pts", 0, 512, 512, 128};
STATIC int ptsopen(), ptsclose(), ptswput(), ptsrsrv(), ptswsrv();
STATIC struct qinit ptsrint = {NULL, ptsrsrv, ptsopen, ptsclose,NULL,&pts_info, NULL};
STATIC struct qinit ptswint = {ptswput, ptswsrv, NULL, NULL, NULL,&pts_info, NULL};
struct streamtab ptsinfo = {&ptsrint, &ptswint, NULL, NULL};

/*
 * ptms_tty[] and pt_cnt are defined in slave.d file 
 */
extern struct pt_ttys ptms_tty[];	
extern int pt_cnt;

/*
 * Open the master device. Reject a clone open and do not allow the
 * driver to be pushed. If the slave/master pair is locked or if
 * the slave is not open, return OPENFAIL. If cannot allocate zero
 * length data buffer, fail open.
 * Upon success, store the write queue pointer in private data and
 * set the PTSOPEN bit in the sflag field.
 */
/* ARGSUSED */
STATIC int 
ptsopen(rqp, dev, flag, sflag)
queue_t *rqp;
dev_t dev;
int flag;
int sflag;
{
        register struct pt_ttys *ptsp;
	register mblk_t *mp;
	register mblk_t *mop;		/* Pointer to a setopts message block */


	DBG(("entering ptsopen\n"));

        dev = minor(dev);
        if (sflag) {
		DBG(("sflag is set\n"));
		u.u_error = EINVAL;
		return(OPENFAIL);
	}
        if ( dev > pt_cnt) {
		DBG(("invalid minor number\n"));
		u.u_error = ENODEV;
                return(OPENFAIL);
	}
        ptsp = &ptms_tty[dev];
        if ((ptsp->pt_state & PTLOCK) || !(ptsp->pt_state & PTMOPEN)) {
		DBG(("master is locked or slave is closed\n"));
		u.u_error = EACCES;
		return(OPENFAIL);
	}
	/* 
	 * if already, open simply return...
	 */
	if (ptsp->pt_state & PTSOPEN) {
		DBG(("master already open\n"));
		return(dev);
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

	if (!(mp = allocb(0, BPRI_MED))) {
		DBG(("could not allocb(0, pri)\n"));
		u.u_error = EAGAIN;
		return(OPENFAIL);
	}
	ptsp->pt_bufp = mp;
	ptsp->pts_wrq = WR(rqp);
	WR(rqp)->q_ptr = (char *) ptsp;
	rqp->q_ptr = (char *) ptsp;
	ptsp->pt_state |= PTSOPEN;

	DBG(("returning from ptsopen\n"));
	return(dev);
}



/*
 * Find the address to private data identifying the slave's write 
 * queue. Send a 0-length msg up the slave's read queue to designate 
 * the master is closing. Uattach the master from the slave by nulling 
 * out master's write queue field in private data.
 */
STATIC int 
ptsclose(rqp)
queue_t *rqp;
{
        register struct pt_ttys *ptsp;

	DBG(("entering ptsclose\n"));
	/*
	 * if no private data...
	 */
	if(!rqp->q_ptr)
		return( 0);

        ptsp = (struct pt_ttys *)rqp->q_ptr;
	if((ptsp->ptm_wrq) && (ptsp->pt_bufp)) {
		DBG(("putnext() a zero length message\n"));
		putnext(RD(ptsp->ptm_wrq),ptsp->pt_bufp);
	} else
		if(ptsp->pt_bufp)
			freemsg(ptsp->pt_bufp);
	ptsp->pt_bufp = NULL;
        ptsp->pts_wrq = NULL;
	ptsp->pt_state &= ~PTSOPEN;
	ptsp->tty = 0;
	rqp->q_ptr = NULL;
	WR(rqp)->q_ptr = NULL;

	DBG(("returning from ptsclose\n"));
	return( 0);
}


/*
 * The wput procedure will only handle flush messages.
 * All other messages are queued and the write side
 * service procedure sends them off to the master side.
 */
STATIC int 
ptswput(qp, mp)
queue_t *qp;
mblk_t *mp;
{
        register struct pt_ttys *ptsp;
	
	DBG(("entering ptswput\n"));
	ASSERT(qp->q_ptr);

        ptsp = (struct pt_ttys *) qp->q_ptr;
	if(ptsp->ptm_wrq == NULL) {
		DBG(("in write put proc but no master\n"));
		freemsg(mp);
		return( 0);
	}
	switch(mp->b_datap->db_type) {
		/*
		 * if write queue request, flush slave's write
		 * queue and send FLUSHR to ptm. If read queue
		 * request, send FLUSHR to ptm.
		 */
		case M_FLUSH:
			DBG(("pts got flush request\n"));
			if (*mp->b_rptr & FLUSHW) {
				DBG(("flush pts write Q\n"));
				flushq(qp, FLUSHDATA);
				DBG(("putctl1 FLUSHR to ptm\n"));
				putctl1(RD(ptsp->ptm_wrq)->q_next, M_FLUSH, FLUSHR);
			}
			if (*mp->b_rptr & FLUSHR) {
				DBG(("putctl1 FLUSHW to ptm\n"));
				putctl1(RD(ptsp->ptm_wrq)->q_next, M_FLUSH, FLUSHW);
			}
			freemsg(mp);
			break;

		default:
			/*
			 * send other messages to the master
			 */
			DBG(("put msg on slave's write queue\n"));
			putq(qp, mp);
			break;
	}
	DBG(("return from ptswput()\n"));
	return( 0);
}


/*
 * enable the write side of the master. This triggers the 
 * master to send any messages queued on its write side to
 * the read side of this slave.
 */
STATIC int 
ptsrsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptsp;
	
	DBG(("entering ptsrsrv\n"));
	ASSERT(qp->q_ptr);

        ptsp = (struct pt_ttys *) qp->q_ptr;
	if(ptsp->ptm_wrq == NULL) {
		DBG(("in read srv proc but no master\n"));
		return( 0);
	}
	qenable(ptsp->ptm_wrq);
	DBG(("leaving ptsrsrv\n"));
	return( 0);
}
		


/*
 * If there are messages on this queue that can be sent to 
 * master, send them via putnext(). Else, if queued messages 
 * cannot be sent, leave them on this queue. If priority 
 * messages on this queue, send them to master no matter what.
 */
STATIC int 
ptswsrv(qp)
queue_t *qp;
{
        register struct pt_ttys *ptsp;
        register queue_t *ptm_rdq;
	mblk_t *mp;
	
	DBG(("entering ptswsrv\n"));
	ASSERT(qp->q_ptr);

        ptsp = (struct pt_ttys *) qp->q_ptr;
	if(ptsp->ptm_wrq == NULL) {
		DBG(("in write srv proc but no master\n"));
		return( 0);
	}
	ptm_rdq = RD(ptsp->ptm_wrq);
	/*
	 * while there are messages on this write queue...
	 */
	while ((mp = getq(qp)) != NULL) {
		/*
		 * if don't have control message and cannot put
		 * msg. on master's read queue, put it back on
		 * this queue.
		 */
		if (mp->b_datap->db_type<=QPCTL && 
					!canput(ptm_rdq->q_next)) {
			DBG(("put msg. back on Q\n"));
			putbq(qp, mp);
			break;
		}
		/*
		 * else send the message up master's stream
		 */
		DBG(("send message to master\n"));
		putnext(ptm_rdq, mp);
	}
	DBG(("leaving ptswsrv\n"));
	return( 0);
}
