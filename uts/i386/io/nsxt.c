/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:nsxt.c	1.3.1.1"
/*
 * SXT --  STREAMS Multiplexing Driver for Shell Layers
 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/signal.h"
#include "sys/debug.h"
#include "sys/dir.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/termio.h"
#include "sys/errno.h"
#include "sys/strtty.h"
#include "sys/nsxt.h"
#include "sys/fcntl.h"
#include "sys/cmn_err.h"
 
/*
 *  A real terminal's stream is linked below a multiplexor
 *  that has been opened by a control channel and (0 to 7)
 *  virtual TTYs.  The user types at the real terminal,
 *  and the MUX routes data up and down the appropriate streams.
 *
 *  Operating characteristics are described on the shl(1)
 *  manual page in the UNIX System V User Reference Manual.
 */

struct sxt_chan		/* used in sxtctl structure below, 1 @ channel */
{
	queue_t *schan_upq;		/* upstream read queue */
	struct sxtctl *schan_ctlp;	/* ptr to ctl struct */
	int schan_chflg;		/* flags */
	short schan_channo;		/* channel number for easy ref*/
};

struct sxtctl
{
	queue_t *sxt_actq;			/* active upstream queue */
	queue_t *sxt_ttyq;			/* downstream write queue */
	int sxt_next;				/* sched this chan next */
	struct sxt_chan sxt_chan[MAXPCHAN];	/* channels per active tty */
	int sxt_ctlflg;				/* control flags */
} sxtctl[MAXLINKS];

/*
 *  STREAMS declarations
 */
int nsxtopen(), nsxtclose(), sxtosrv(), sxtiput(), sxtwsrv();
extern int nulldev();
STATIC struct module_info sxt_info = { 189, "sxt", 0, INFPSZ, 512, 256 };
STATIC struct qinit sxtrinit = {sxtiput, NULL, nsxtopen, nsxtclose, NULL, &sxt_info};
STATIC struct qinit sxtwinit = {putq, sxtosrv, nsxtopen, nsxtclose, NULL, &sxt_info};
STATIC struct qinit smuxrinit = {sxtiput, nulldev, nulldev, nulldev, NULL, &sxt_info};
STATIC struct qinit smuxwinit = {nulldev, sxtwsrv, nulldev, nulldev, NULL, &sxt_info};
struct streamtab nsxtinfo = {&sxtrinit, &sxtwinit, &smuxrinit, &smuxwinit};

nsxtopen(q, dev, oflag, sflag)
queue_t *q;
{
	struct sxt_chan *chanp;
	int i;
	register chan;
	mblk_t * mop;

	if( sflag ) return(OPENFAIL);	/* being opened as a module */

	chan = CHAN(dev);

	if ( ( sxtctl[LINK(dev)].sxt_ttyq == (struct queue *)0 )
	     && !(chan == 0 && oflag & O_EXCL) ) {
		u.u_error = EINVAL;
		return(OPENFAIL);
	}
	if( chan == 0 )	/* opening control channel */
	{
		if( sxtctl[LINK(dev)].sxt_actq ) /* open this only once! */
		{
			u.u_error = EBUSY;
			return(OPENFAIL);
		}
		sxtctl[LINK(dev)].sxt_actq = q;	/* mark channel as open */

		chanp = &sxtctl[LINK(dev)].sxt_chan[chan];
		q->q_ptr = (char *)chanp;
		WR(q)->q_ptr = (char *)chanp;
		chanp->schan_upq = q;
		for( i=0; i<MAXPCHAN; i++ ) /* set ctl struct ptr & channo for all */
		{
			sxtctl[LINK(dev)].sxt_chan[i].schan_ctlp = &sxtctl[LINK(dev)];
			sxtctl[LINK(dev)].sxt_chan[i].schan_channo = i;
		}
		chanp->schan_chflg |= SXTCTL;
		return(1);
	}

	if( chan >= MAXPCHAN ) /* channel number out of range	*/
	{
		u.u_error = ENXIO;
		return(OPENFAIL);
	}

	chanp = &sxtctl[LINK(dev)].sxt_chan[chan];

	if( chanp->schan_upq )	/* already open */
	{
		return(1); /* OK to re-open existing channel */
	}

	/* open a new virtual channel to sxt */
	chanp->schan_upq = q;
	q->q_ptr = (char *)chanp;
	WR(q)->q_ptr = (char *)chanp;

	if (mop = allocb(sizeof(struct stroptions ), BPRI_MED)) {
		register struct stroptions *sop;

		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions );
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
		sop->so_hiwat = 512;
		sop->so_lowat = 256;
		putnext(q, mop);
	} else {
		u.u_error = EAGAIN;
		return(OPENFAIL);
	}
	return(1);
}


nsxtclose(q)
queue_t *q;
{
	queue_t *tq;
	struct sxt_chan *tchanp;
	struct sxt_chan *chanp;
	int i;
	mblk_t *bp;

        chanp = (struct sxt_chan *)q->q_ptr;
	if(!chanp) {
		return(-1);
	}

	if( chanp->schan_chflg&SXTCTL )	/* closing control channel */
	{
		/*
		 * Rip the whole thing down.
		 * Disconnect all channels from
		 * the mux, then send each a hangup
		 * message if it's still alive. 
		 * Finally, release control channel.
	 	 */
		for( i=1; i<MAXPCHAN; i++ )
		{
			tchanp = &chanp->schan_ctlp->sxt_chan[i];
			tq = tchanp->schan_upq;
			if( tq == NULL )
				continue;
			tchanp->schan_upq = NULL;
			tchanp->schan_ctlp = NULL;
			tchanp->schan_chflg = 0;
			putctl(tq->q_next, M_HANGUP);
		}
		/* release control channel */
		chanp->schan_upq = NULL;
		chanp->schan_ctlp->sxt_actq = NULL;
		chanp->schan_ctlp->sxt_ttyq = NULL;
		chanp->schan_ctlp->sxt_ctlflg = 0;
		chanp->schan_ctlp = NULL;
		chanp->schan_chflg = 0;
	}
	else	/* closing a virtual TTY */
	{
		if (chanp->schan_upq == NULL) {
			return;
		}
		putctl(chanp->schan_upq->q_next, M_HANGUP);
		chanp->schan_upq = NULL;
		chanp->schan_chflg = 0;
	}
	return;
}

sxtiput(q, bp)
queue_t *q;
mblk_t *bp;
{
	struct sxtctl *ctlp;
	mblk_t *tmpb;
	struct iocblk *qryp;

	ctlp = (struct sxtctl *)WR(q)->q_ptr;

	if( ctlp->sxt_ctlflg&WAITSW && (bp->b_datap->db_type != M_IOCACK) &&
				       (bp->b_datap->db_type != M_IOCNAK) )
	{
		putq(q, bp);
		return;
	}

	switch (bp->b_datap->db_type)
	{
		case M_IOCACK:
		case M_IOCNAK:
			ctlp->sxt_ctlflg &= ~SXTIOCWAIT;
			break;
		case M_CTL:

		/* The M_CTL has been standardized to look like an M_IOCTL
		 * message, look for CSWTCH in the command field
		 */

			if ((bp->b_wptr - bp->b_rptr) != sizeof (struct iocblk)) {
				cmn_err (CE_NOTE,"Non standard M_CTL received by the sxt driver\n");
				/* May be for someone else; pass it on */
				break;
			}
			qryp = (struct iocblk *)bp->b_rptr;

			switch (qryp->ioc_cmd) {

			case SXTSWTCH:

			/*
			 * If waiting for an ioctl ACK/NAK, queue this
			 * "switch" message and all messages that follow it
			 * until the ACK or NAK is sent up the proper
			 * virtual TTY stream.
			 */
				if( ctlp->sxt_ctlflg&SXTIOCWAIT )
				{
					ctlp->sxt_ctlflg |= WAITSW;
					putq(q, bp);
					return;
				}

			/*
			 * User hit SWTCH character to get to ctl chan.
			 * Set active upper queue ptr to control channel.
			 */
				ctlp->sxt_actq = ctlp->sxt_chan[0].schan_upq;

			/* should check for outstanding ioctl's before sending
			 * this on up.
			 */
				bp->b_datap->db_type = M_DATA;
				if(bp->b_cont)
					freeb(unlinkb(bp->b_cont));

			/* make it look like we are starting from scratch */

				bp->b_rptr = bp->b_wptr = bp->b_datap->db_base;
				*bp->b_wptr++ = 'Z';
				break;
			default:
				cmn_err(CE_NOTE,"Unknown M_CTL command sxt driver\n");
				break;
			}
			break;
		
		case M_HANGUP:
			if (ctlp->sxt_actq != ctlp->sxt_chan[0].schan_upq) {
				putctl(ctlp->sxt_chan[0].schan_upq->q_next, M_HANGUP);
			}
		case M_BREAK:
		case M_PCSIG:
		case M_SIG:
			/* send it to the active channel */
				break;

		default:
			/* everything else goes up active channel */
			if( canput(ctlp->sxt_actq->q_next) == 0 )
			{
				/* channel clogged---drop msg */
				freemsg(bp);
				return;
			}
			break;
	}
	putnext(ctlp->sxt_actq, bp);

	/* Clean up the stuff queued waiting for the IOCACK/IOCNAK */
	if( (ctlp->sxt_ctlflg&WAITSW) && !(ctlp->sxt_ctlflg&SXTIOCWAIT) )
	{
		ctlp->sxt_ctlflg &= ~WAITSW;
		while( (tmpb=getq(q)) != NULL )
			sxtiput(q, tmpb);	/* recursion! */
	}
}

/*
 * Output service procedure for the TTY stream linked below
 * the MUX.  This routine only exists to maintain flow control
 * across the MUX.  Its queue is not used, but it is back-enabled
 * when a previously-clogged TTY output stream is cleared.
 * The routine then enables one of the queues "above" the MUX;
 * it doesn't matter which one, since they run round-robin
 * whenever any one of them is scheduled.
 */
sxtwsrv(q)
queue_t *q;
{
	struct sxtctl *ctlp;

	ctlp = (struct sxtctl *)q->q_ptr;
	qenable(WR(ctlp->sxt_actq));
	return;
}

/*
 * Output service procedure for the control channel stream, and for
 * all the virtual TTY streams that have opened the SXT driver.
 *
 * When any one of these queues is scheduled for service, all of them
 * are checked for output data.  The routine runs until all the write
 * queues are empty, or until the stream linked under SXT is clogged.
 */
sxtosrv(q)
queue_t *q;
{
	struct sxt_chan *chanp;
	int i, count, start;
	struct sxtctl *ctlp;

	chanp = (struct sxt_chan *)q->q_ptr;
	ctlp = chanp->schan_ctlp;
	/* maintain a circular ordering for emptying queues (round robin).
	 * if someone had previous blocked because downstream was full,
	 * try that channel again for starters.  Otherwise, if nobody was
	 * blocked, start with whomever is coming down the pike now.
	 */
	if( ctlp->sxt_next )
	{
		start = ctlp->sxt_next - 1;
		ctlp->sxt_next = 0;
	}
	else
		start = chanp->schan_channo;

    loop:
	count = 0;
	for( i=start; i<MAXPCHAN; i++)
	{
		chanp = &ctlp->sxt_chan[i];
		if( WR(chanp->schan_upq) )  /* upper write queue there? */
		{
			count += sxtqsrv(chanp);
			/* if we're blocking, give up and try again later
			 * when downstream unclogs.
			 */
			if (ctlp->sxt_next)
				return;
		}
	}

	/* if anyone did anything useful, or if we didn't try everyone */
	if( count || start )
	{
		start = 0;
		goto loop;
	}
}

sxtqsrv(chanp)
struct sxt_chan *chanp;
{
	mblk_t *bp, *bpt;
	queue_t *downq;
	queue_t *tq;
	struct linkblk *linkblkp;
	struct iocblk *iocbp;
	struct copyreq *reqp;
	struct sxt_chan *tchanp;
	struct sxtblock *psb;
	int channo, i;

	if( chanp->schan_upq == NULL )   return(0);

	tq = WR(chanp->schan_upq);

	if ( (bp = getq(tq)) == NULL )   return(0);

	downq = chanp->schan_ctlp->sxt_ttyq;

	if( downq )
	{
		
		if (bp->b_datap->db_type <= QPCTL && !canput(downq->q_next))  /* downq full? */
		{ 
			putbq(tq, bp);
			chanp->schan_ctlp->sxt_next = chanp->schan_channo + 1;
			return(0);
		}
	}

	switch( bp->b_datap->db_type )
	{
	default:
		freemsg(bp);
		return(1);

	case M_DATA:
		if( (chanp->schan_chflg&SXTBLK) &&
		    (RD(tq) != chanp->schan_ctlp->sxt_actq) )
		{
			/* blocking output and not active channel, continue */
			putbq(tq, bp);
			return(0);
		} 
		if (downq) putnext(downq, bp);
		return(1);

	case M_IOCDATA:
		if (!(chanp->schan_chflg & SXT_IOCTL)) {
			freemsg(bp);
			return(1);
		}
		sxtioccont(bp, chanp);
		return(1);

	case M_IOCTL:
		iocbp = (struct iocblk *)bp->b_rptr;
		if (((iocbp->ioc_cmd & SXTIOCLINK) == SXTIOCLINK)  &&  iocbp->ioc_count != TRANSPARENT) {
			iocbp->ioc_error = EINVAL;
			iocbp->ioc_count = 0;
			iocbp->ioc_rval = 0;
			bp->b_datap->db_type = M_IOCNAK;
			if (bp->b_cont) {
				freemsg(bp->b_cont);
				bp->b_cont = NULL;
			}
			qreply(tq, bp);
			return(1);
		}
		switch( iocbp->ioc_cmd )
		{
		case I_LINK:
			/* hook up tty q to ctl struct */
			linkblkp = (struct linkblk *)bp->b_cont->b_rptr;
			chanp->schan_ctlp->sxt_ttyq = linkblkp->l_qbot;
			chanp->schan_ctlp->sxt_ttyq->q_ptr = (char *)chanp->schan_ctlp;
			break;

		case I_UNLINK:
			linkblkp = (struct linkblk *)bp->b_cont->b_rptr;
			chanp->schan_ctlp->sxt_ttyq->q_ptr = 0;
			chanp->schan_ctlp->sxt_ttyq = NULL;
			break;

		default:
			/* block ioctl's not of active channel */
			if( RD(tq) != chanp->schan_ctlp->sxt_actq )
			{
				putbq(tq, bp);
				return(0);
			}
			chanp->schan_ctlp->sxt_ctlflg |= SXTIOCWAIT;
			/* SXTIOCWAIT flag used to ensure ACKs go to right top Q */
			if (downq) putnext(downq, bp);
			/* acks come from down below, so continue */
			return(1);

		case SXTIOCSWTCH:
			/* activate new channel */
			if( !(chanp->schan_chflg&SXTCTL) )
			{
				/* must be ctl chan for this ioctl */
				bp->b_datap->db_type = M_IOCNAK;
			}
			channo = *((int *)bp->b_cont->b_rptr);
			if( channo > MAXPCHAN )
			{
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
			tchanp = &chanp->schan_ctlp->sxt_chan[channo];
			chanp->schan_ctlp->sxt_actq = tchanp->schan_upq;
			break;

		case SXTIOCBLK:
			if( !(chanp->schan_chflg&SXTCTL) )
			{
				/* must be ctl chan for this ioctl */
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
			channo = *((int *)bp->b_cont->b_rptr);
			if( channo > MAXPCHAN )
			{
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
			tchanp = &chanp->schan_ctlp->sxt_chan[channo];
			tchanp->schan_chflg |= SXTBLK;
			break;

		case SXTIOCUBLK:
			if( !(chanp->schan_chflg&SXTCTL) )
			{
				/* must be ctl chan for this ioctl */
				bp->b_datap->db_type = M_IOCNAK;
			}
			channo = *((int *)bp->b_cont->b_rptr);
			if( channo > MAXPCHAN )
			{
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
			tchanp = &chanp->schan_ctlp->sxt_chan[channo];
			tchanp->schan_chflg &= ~SXTBLK;
			break;

		case SXTIOCSTAT:
#if 0
			if( !(chanp->schan_chflg&SXTCTL) )
			{
				/* must be ctl chan for this ioctl */
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
#endif
			if( (bpt = allocb(sizeof(struct sxtblock), BPRI_MED)) == NULL )
			{
				bp->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EAGAIN;
				break;
			}
			bp->b_datap->db_type = M_COPYOUT;
			bp->b_wptr = bp->b_rptr + sizeof(struct copyreq );
			reqp = (struct copyreq *)bp->b_rptr;

			/* cq_cmd overlays ioc_cmd */
			/* cq_id overlays ioc_id */

			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));
			reqp->cq_size = sizeof(struct sxtblock );
			reqp->cq_private = NULL;
			reqp->cq_flag = 0;
			freeb(bp->b_cont);
			bp->b_cont = bpt;
			chanp->schan_chflg |= SXT_IOCTL;
			channo = 0;
			for( i=0; i<MAXPCHAN; i++ )
			{
				if( chanp->schan_ctlp->sxt_chan[i].schan_chflg
						& SXTBLK )
				{
					channo |= 1 << i;
				}
			}
			psb = (struct sxtblock *)bp->b_cont->b_rptr;
			bp->b_cont->b_wptr = bp->b_cont->b_rptr + sizeof(struct sxtblock);
			psb->output = channo;
			psb->input = 0;		/* for now */
			qreply(tq, bp);
			return(1);
		}

		if( bp->b_datap->db_type != M_IOCNAK )
		{
			bp->b_datap->db_type = M_IOCACK;
			iocbp = (struct iocblk *)bp->b_rptr;
			iocbp->ioc_error = 0;
			iocbp->ioc_count = 0;
		}
		qreply(tq, bp);
		return(1);
	}
}
sxtioccont(bp, chanp)		/* continue ioctl processing */
mblk_t *bp;
struct sxt_chan *chanp;
{
	struct copyreq *reqp;
	struct copyresp *resp;
	struct iocblk *iocbp;
	mblk_t * mp, *tmp;

	resp = (struct copyresp *)bp->b_rptr;
	if (resp->cp_rval) {		/* failure */
		chanp->schan_chflg &= ~SXT_IOCTL;
		freemsg(bp);
		return;
	}
	switch (resp->cp_cmd) {

	case SXTIOCSTAT:
		iocbp = (struct iocblk *)bp->b_rptr;
		iocbp->ioc_count = 0;
		iocbp->ioc_error = 0;
		iocbp->ioc_rval = 0;
		bp->b_datap->db_type = M_IOCACK;
		chanp->schan_chflg &= ~SXT_IOCTL;
		break;

	default:
		freemsg(bp);
		return;
	}
	putnext(chanp->schan_upq, bp);
	return;
}
