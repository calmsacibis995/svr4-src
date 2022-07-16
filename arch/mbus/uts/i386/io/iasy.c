/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989, 1990 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/iasy.c	1.3.4.1"

#ifndef lint
static char iasy_copyright[] = "Copyright 1989, 1990 Intel Corporation 464462";
#endif /*lint*/

/*
 *	Generic Terminal Driver	(STREAMS version)
*/
#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/stream.h"
#include "sys/errno.h"
#include "sys/termio.h"
#include "sys/cmn_err.h"
#include "sys/stropts.h"
#include "sys/debug.h"
#include "sys/cred.h"
#include "sys/strtty.h"
#include "sys/file.h"
#include "sys/eucioctl.h"
#include "sys/iasy.h"
#include "sys/ddi.h"


extern void bcopy();
extern void splx();
extern void ttinit();
extern void wakeup();

extern struct strtty iasy_tty[];	/* tty structs for each device */
extern struct iasy_hw iasy_hw[];/* hardware infor per device */
extern int iasy_num;

#define TP_TO_Q(tp)			((tp)->t_rdqp)
#define Q_TO_TP(q)			((struct strtty *)q->q_ptr)
#define TP_TO_HW(tp)		(&iasy_hw[(tp)->t_dev])
#define HW_PROC(tp, func)	((*TP_TO_HW(tp)->proc)((tp), func))

int iasydevflag = 0; 	/* SVR4.0 requirement */
int iasy_cnt = 0;	 	/* /etc/crash requirement */

int iasyopen(), iasyclose(), iasyoput();
int iasyisrv(), iasyosrv();

#define IASY_HIWAT	512
#define IASY_LOWAT	256
#define IASY_BUFSZ	64	/* Chosen to be about CLSIZE */
struct module_info iasy_info = {
	'iasy', "iasy", 0, INFPSZ, IASY_HIWAT, IASY_LOWAT };
static struct qinit iasy_rint = {
	putq, iasyisrv, iasyopen, iasyclose, NULL, &iasy_info, NULL};
static struct qinit iasy_wint = {
	iasyoput, iasyosrv, iasyopen, iasyclose, NULL, &iasy_info, NULL};
struct streamtab iasyinfo = {
	&iasy_rint, &iasy_wint, NULL, NULL};

/*
 * Wakeup sleep function calls sleeping for a STREAMS buffer
 * to become available
 */
STATIC void
iasybufwake( tp)
register struct strtty *tp;
{
 	wakeup( (caddr_t)&tp->t_cc[3]);
}

/*
 *	Open an iasy line
*/
/* ARGSUSED */
iasyopen(q, devp, flag, sflag, crp)
queue_t *q;		/* Read queue pointer */
dev_t *devp;
int flag;
int sflag;
cred_t *crp;
{	register struct strtty *tp;
	register struct stroptions *sop;
	dev_t dev;
	int oldpri;
	mblk_t *mop;

	dev = getminor(*devp);
	if (dev >= iasy_num)
		return(ENXIO);
	if (iasy_hw[dev].proc == 0)
		return(ENXIO);	/* No hardware for this minor number */
	tp = &iasy_tty[dev];
	tp->t_dev = dev;
	tp->t_rdqp = q;
	q->q_ptr = (caddr_t) tp;
	WR(q)->q_ptr = (caddr_t) tp;

	oldpri = SPL();

	/* 
	 * Do the required things on first open 
	 */
	if ((tp->t_state & (ISOPEN | WOPEN)) == 0) {

		/*set process group on first tty open*/
		while ((mop = allocb(sizeof(struct stroptions),BPRI_MED)) 
											== NULL){
			if ( flag & (FNDELAY | FNONBLOCK)) {
				tp->t_rdqp = NULL;
				splx(oldpri);
				return( EAGAIN);
			}
			bufcall( (uint)sizeof( struct stroptions), BPRI_MED, 
						iasybufwake, tp);
			if ( sleep( (caddr_t)&tp->t_cc[3], TTIPRI | PCATCH)) {
				tp->t_rdqp = NULL;
				splx(oldpri);
				return( EINTR);
			}
		}

		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
		sop->so_hiwat = IASY_HIWAT;
		sop->so_lowat = IASY_LOWAT;
		(void) putnext(q, mop);
	
		/* Set water marks on write q */
		q = WR(q);
		strqset(q, QHIWAT,  0, IASY_HIWAT);
		strqset(q, QLOWAT,  0, IASY_LOWAT);
	}
	
	/* Init HW and SW state */
	if (HW_PROC(tp, T_CONNECT)) { /* T_CONNECT must compute CARR_ON */
		tp->t_rdqp = NULL;
		splx(oldpri);
		return(ENXIO);
	}

	if ((tp->t_state & (ISOPEN | WOPEN)) == 0) {
		tp->t_iflag = IGNPAR;
		tp->t_oflag = 0;
		tp->t_cflag = B9600|CS8|CREAD|HUPCL;
		tp->t_lflag = 0;

		/* allocate RX buffer */
		while ((tp->t_in.bu_bp = 
						allocb(IASY_BUFSZ, BPRI_MED)) == NULL){
			if ( flag & (FNDELAY | FNONBLOCK)) {
				tp->t_rdqp = NULL;
				splx (oldpri);
				return( EAGAIN);
			}
			bufcall( (uint)sizeof( struct stroptions), BPRI_MED, 
						iasybufwake, tp);
			if ( sleep( (caddr_t)&tp->t_cc[3], TTIPRI | PCATCH)) {
				tp->t_rdqp = NULL;
				splx (oldpri);
				return( EINTR);
			}
		}

		tp->t_in.bu_cnt = IASY_BUFSZ;
		tp->t_in.bu_ptr = tp->t_in.bu_bp->b_wptr;
		tp->t_out.bu_bp = 0;
		tp->t_out.bu_cnt = 0;
		tp->t_out.bu_ptr = 0;
		if (HW_PROC(tp, T_PARM)) {	/* Configure hardware */
			(void) HW_PROC(tp, T_DISCONNECT);
			if (tp->t_in.bu_bp)
				freeb(tp->t_in.bu_bp);
			tp->t_rdqp = NULL;
			splx(oldpri);
			return(ENXIO);
		}
	}

	if (tp->t_cflag & CLOCAL)
		tp->t_state |= CARR_ON;

	/* wait for carrier */
	if (!(flag & (FNDELAY | FNONBLOCK))) {
		while ((tp->t_state & CARR_ON) == 0) {
			tp->t_state |= WOPEN;
			if (sleep((caddr_t) &tp->t_rdqp, TTIPRI|PCATCH)) {
				if (!(tp->t_state & ISOPEN)) {
					tp->t_rdqp = NULL;
					if (tp->t_in.bu_bp)
						freeb(tp->t_in.bu_bp);
				}
				tp->t_state &= ~WOPEN;
				splx(oldpri);
				return(EINTR);
			}
		}
	}
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
	splx(oldpri);
	return(0);
}

/*
 *	Close an iasy line
*/
/* ARGSUSED */
iasyclose(q, flag, cred_p)
queue_t *q;		/* Read queue pointer */
int flag;
cred_t *cred_p;
{	register struct strtty *tp;
	register int	oldpri;

	tp = Q_TO_TP(q);
	oldpri = SPL();
	
	/* Drain queued output to the user's terminal. */
	while ((tp->t_state & CARR_ON) && ((tp->t_state & (BUSY|TIMEOUT|TTSTOP)) ||
				(WR(q)->q_first != NULL)) ) {
		tp->t_state |= TTIOW;
		if (sleep((caddr_t) &tp->t_oflag, TTOPRI|PCATCH)) {
			tp->t_state &= ~TTIOW;
			break;
		}
	}

	if (!(tp->t_state & ISOPEN)) {	/* See if it's closed already */
		splx(oldpri);
		return;
	}
	if (tp->t_cflag & HUPCL)
		(void) HW_PROC(tp, T_DISCONNECT);
	iasyflush(WR(q), FLUSHR);
	if (tp->t_in.bu_bp) {
		freeb((mblk_t *)tp->t_in.bu_bp);	
		tp->t_in.bu_bp  = 0;
		tp->t_in.bu_ptr = 0;
		tp->t_in.bu_cnt = 0;
	}
	if (tp->t_out.bu_bp) {
		freeb((mblk_t *)tp->t_out.bu_bp);	
		tp->t_out.bu_bp  = 0;
		tp->t_out.bu_ptr = 0;
		tp->t_out.bu_cnt = 0;
	}
	tp->t_state &= ~ISOPEN;
	tp->t_rdqp = NULL;
	q->q_ptr = WR(q)->q_ptr = NULL;
	splx(oldpri);
}

/*
 *	Resume output after a delay
*/
void
iasydelay(tp)
struct strtty *tp;
{	int s;

	s=SPL();
	tp->t_state &= ~TIMEOUT;
	(void) HW_PROC(tp, T_OUTPUT);
	splx(s);
}

/*
 * ioctl handler for output PUT procedure
*/
void
iasyputioc(q, bp)
queue_t *q;		/* Write queue pointer */
mblk_t *bp;		/* Ioctl message pointer */
{	struct strtty *tp;
	struct iocblk *iocbp;
	mblk_t *bp1;

	iocbp = (struct iocblk *)bp->b_rptr;
	tp = Q_TO_TP(q);

	switch (iocbp->ioc_cmd) {
	case TCSETSW:
	case TCSETSF:
	case TCSETAW:
	case TCSETAF:
	case TCSBRK: /* run these now, if possible */
		if (q->q_first != NULL || (tp->t_state & (BUSY|TIMEOUT|TTSTOP))) {
			(void) putq(q, bp);		/* queue ioctl behind output */
			break;
		}
		iasysrvioc(q, bp);			/* No output, do it now */
		break;

	case TCSETA:	/* immediate parm set */
	case TCSETS:
	case TCGETA:
	case TCGETS:	/* immediate parm retrieve */
		iasysrvioc (q, bp);			/* Do these anytime */
		break;

	case EUC_MSAVE:
	case EUC_MREST:
	case EUC_IXLOFF:
	case EUC_IXLON:
	case EUC_OXLOFF:
	case EUC_OXLON:
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		(void) putnext(RD(q), bp);
		break;

	default:
		if ((iocbp->ioc_cmd & 0xff00) != LDIOC) { /* An IOCTYPE ? */
			/*
			 *	Unknown ioctls are either intended for the hardware dependant
			 *	code or an upstream module that is not present.  Pass the
			 *	request to the HW dependant code to handle it.
			 */
			(*(TP_TO_HW(tp)->hwdep))(q, bp);
			return;
		}
		/* ignore LDIOC cmds */
		bp->b_datap->db_type = M_IOCACK;
		bp1 = unlinkb(bp);
		if (bp1)
			freeb(bp1);
		iocbp->ioc_count = 0;
		(void) putnext(RD(q), bp);
		break;
	}
}

/*
 *	A message has arrived for the output q
*/
iasyoput(q, bp)
queue_t *q;		/* Write queue pointer */
mblk_t *bp;
{	register mblk_t *bp1;
	register struct strtty *tp;
	int s;

	tp = Q_TO_TP(q);
	s = SPL();
	switch (bp->b_datap->db_type) {
	case M_DATA:
		if (!(tp->t_state & CARR_ON)) {
			freemsg(bp);	/* Output without carrier is lost */
			splx(s);
			return(0);
		}
		while (bp) {		/* Normalize the messages */
			bp->b_datap->db_type = M_DATA;
			bp1 = unlinkb(bp);
			bp->b_cont = NULL;
			if ((bp->b_wptr - bp->b_rptr) <= 0) {
				freeb(bp);
			} else {
				(void) putq(q, bp);
			}
			bp = bp1;
		}
		(void) HW_PROC(tp, T_OUTPUT);	/* Start output */
		break;

	case M_IOCTL:
		iasyputioc(q, bp);				/* Queue it or do it */
		(void) HW_PROC(tp, T_OUTPUT);	/* just in case */
		break;

	case M_FLUSH:
#if FLUSHRW != (FLUSHR|FLUSHW)
		cmn_err(CE_PANIC, "iasy: implementation assumption botched\n");
#endif
		switch (*(bp->b_rptr)) {

		case FLUSHRW:
			iasyflush(q, (FLUSHR|FLUSHW));
			freemsg(bp);	/* iasyflush has done FLUSHR */
			break;

		case FLUSHR:
			iasyflush(q, FLUSHR);
			freemsg(bp);	/* iasyflush has done FLUSHR */
			break;

		case FLUSHW:
			iasyflush(q, FLUSHW);
			freemsg(bp);
			break;

		default:
			freemsg(bp);
			break;
		}
		break;

	case M_START:
		(void) HW_PROC(tp, T_RESUME);
		freemsg(bp);
		break;

	case M_STOP:
		(void) HW_PROC(tp, T_SUSPEND);
		freemsg(bp);
		break;

	case M_BREAK:
		if (q->q_first != NULL || (tp->t_state & BUSY)) {
			(void) putq(q, bp);	/* Device busy, queue for later */
			break;
		}
		(void) HW_PROC(tp, T_BREAK); /* Do break now */
		freemsg(bp);
		break;

	case M_DELAY:
		tp->t_state |= TIMEOUT;
		(void) timeout(iasydelay, (caddr_t)tp, (int)*(bp->b_rptr));
		freemsg (bp);
		break;

	case M_STARTI:
		(void) HW_PROC(tp, T_UNBLOCK);
		freemsg(bp);
		break;

	case M_STOPI:
		(void) HW_PROC(tp, T_BLOCK);
		freemsg(bp);
		break;

	case M_IOCDATA:
		/* HW dep ioctl data has arrived */
		(*(TP_TO_HW(tp)->hwdep))(q, bp);
		break;

	default:
		freemsg(bp);
		break;
	}
	splx(s);
	return(0);
}

/*
 *	Return the next data block -- if none, return NULL
*/
mblk_t *
iasygetoblk(q)
struct queue *q;	/* Write queue pointer */
{	register struct strtty *tp;
	register int s;
	register mblk_t *bp;

	s = SPL();
	tp = Q_TO_TP(q);
	if (!tp) {		/* This can happen only if closed while no carrier */
		splx(s);
		return(0);
	}
	while (!(tp->t_state & BUSY) && ((bp = getq(q)) != NULL)) {
		/* wakeup close write queue drain */
		switch (bp->b_datap->db_type) {
		case M_DATA:
			if (tp->t_state & (TTSTOP | TIMEOUT)) {
				/* should never get here */
				cmn_err(CE_WARN, "iasygetoblk: dependent code timing botch\n");
				(void) putbq(q, bp);
				splx(s);
				return(0);
			}
			splx(s);
			return(bp);
		case M_IOCTL:
			iasysrvioc(q, bp);	/* Do ioctl, then return output */
			break;
		case M_BREAK:
			(void) HW_PROC(tp, T_BREAK);	/* Do break now */
			freemsg(bp);
			break;
		default:
			freemsg(bp);			/* Ignore junk mail */
			break;
		}
	} /*	part of while loop AMS */
	splx(s);
	return(0);
}

/*
 *	Routine to execute ioctl messages.
*/
iasysrvioc(q, bp)
queue_t *q;			/* Write queue pointer */
mblk_t *bp;			/* Ioctl message pointer */
{	struct strtty *tp;
	struct iocblk *iocbp;
	int arg, s;
	mblk_t *bpr;
	mblk_t *bp1;

	iocbp = (struct iocblk *)bp->b_rptr;
	tp = Q_TO_TP(q);
	switch (iocbp->ioc_cmd) {
	/* The output has drained now. */
	case TCSETAF:
		iasyflush(q, FLUSHR);
		/* FALLTHROUGH */
	case TCSETA:
	case TCSETAW: {
		register struct termio *cb;

		if (!bp->b_cont) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			(void) putnext(RD(q), bp);
			break;
		}
		cb = (struct termio *)bp->b_cont->b_rptr;
		tp->t_cflag = (tp->t_cflag & 0xffff0000 | cb->c_cflag);
		tp->t_iflag = (tp->t_iflag & 0xffff0000 | cb->c_iflag);
		s = SPL();
		if (HW_PROC(tp, T_PARM)) {
			splx(s);
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			(void) putnext(RD(q), bp);
			break;
		}
		splx(s);
		bp->b_datap->db_type = M_IOCACK;
		bp1 = unlinkb(bp);
		if (bp1)
			freeb(bp1);
		iocbp->ioc_count = 0;
		(void) putnext(RD(q), bp);
		break;

	}
	case TCSETSF:
		iasyflush(q, FLUSHR);
		/* FALLTHROUGH */
	case TCSETS:
	case TCSETSW:{
		register struct termios *cb;

		if (!bp->b_cont) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			(void) putnext(RD(q), bp);
			break;
		}
		cb = (struct termios *)bp->b_cont->b_rptr;

		tp->t_cflag = cb->c_cflag;
		tp->t_iflag = cb->c_iflag;
		s = SPL();
		if (HW_PROC(tp, T_PARM)) {
			splx(s);
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			(void) putnext(RD(q), bp);
			break;
		}
		splx(s);
		bp->b_datap->db_type = M_IOCACK;
		bp1 = unlinkb(bp);
		if (bp1)
			freeb(bp1);
		iocbp->ioc_count = 0;
		(void) putnext(RD(q), bp);
		break;

	}
	case TCGETA: {	/* immediate parm retrieve */
		register struct termio *cb;

		if (bp->b_cont)	/* Bad user supplied parameter */
			freemsg(bp->b_cont);

		if ((bpr = allocb(sizeof(struct termio), BPRI_MED)) == NULL) {
			ASSERT(bp->b_next == NULL);
			(void) putbq(q, bp);
			(void) bufcall((ushort)sizeof(struct termio),
							BPRI_MED, iasydelay, (long)tp);
			return;
		}
		bp->b_cont = bpr;
		cb = (struct termio *)bp->b_cont->b_rptr;

		cb->c_iflag = (ushort)tp->t_iflag;
		cb->c_cflag = (ushort)tp->t_cflag;

		bp->b_cont->b_wptr += sizeof(struct termio);
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = sizeof(struct termio);
		(void) putnext(RD(q), bp);
		break;

	}
	case TCGETS: {	/* immediate parm retrieve */
		register struct termios *cb;

		if (bp->b_cont)	/* Bad user supplied parameter */
			freemsg(bp->b_cont);

		if ((bpr = allocb(sizeof(struct termios), BPRI_MED)) == NULL) {
			ASSERT(bp->b_next == NULL);
			(void) putbq(q, bp);
			(void) bufcall((ushort)sizeof(struct termios),
							BPRI_MED, iasydelay, (long)tp);
			return;
		}
		bp->b_cont = bpr;
		cb = (struct termios *)bp->b_cont->b_rptr;

		cb->c_iflag = tp->t_iflag;
		cb->c_cflag = tp->t_cflag;

		bp->b_cont->b_wptr += sizeof(struct termios);
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = sizeof(struct termios);
		(void) putnext(RD(q), bp);
		break;

	}
	case TCSBRK:
		if (!bp->b_cont) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_count = 0;
			(void) putnext(RD(q), bp);
			break;
		}
		arg = *(int *)bp->b_cont->b_rptr;
		if (arg == 0) {
			s = SPL();
			(void) HW_PROC(tp, T_BREAK);
			splx(s);
		}
		bp->b_datap->db_type = M_IOCACK;
		bp1 = unlinkb(bp);
		if (bp1)
			freeb(bp1);
		iocbp->ioc_count = 0;
		(void) putnext(RD(q), bp);
		break;

	case EUC_MSAVE:	/* put these here just in case... */
	case EUC_MREST:
	case EUC_IXLOFF:
	case EUC_IXLON:
	case EUC_OXLOFF:
	case EUC_OXLON:
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		(void) putnext(RD(q), bp);
		break;

	default: /* unexpected ioctl type */
		if (canput(RD(q)->q_next) == 1) {
			bp->b_datap->db_type = M_IOCNAK;
			(void) putnext(RD(q), bp);
		} else {
			(void) putbq(q, bp);
		}
		break;
	}
	return;
}

/*
 *	Flush input and/or output queues
*/
iasyflush(q, cmd)
queue_t *q;			/* Write queue pointer */
register cmd;		/* FLUSHR, FLUSHW, or both */
{	struct strtty *tp;
	int s;

	s = SPL();
	tp = Q_TO_TP(q);
	if (cmd & FLUSHW) {
		flushq(q, FLUSHDATA);
		(void) HW_PROC(tp, T_WFLUSH);
	}
	if (cmd & FLUSHR) {
		q = RD(q);
		(void) HW_PROC(tp, T_RFLUSH);
		flushq(q, FLUSHDATA);
		(void) putctl1(q->q_next, M_FLUSH, FLUSHR);
	}
	splx(s);
}

/*
 * New service procedure.  Pass everything upstream.
 */
iasyisrv(q)
queue_t *q;		/* Read queue pointer */
{	register mblk_t *mp;
	register struct strtty *tp;
	int s;

	tp = Q_TO_TP(q);
	s = SPL();
	while ((mp = getq(q)) != NULL) {
		/*
		 * If we can't put, then put it back if it's not
		 * a priority message.  Priority messages go up
		 * whether the queue is "full" or not.  This should
		 * allow an interrupt in, even if the queue is hopelessly
		 * backed up.
		 */
		if (!canput(q->q_next)) {
			(void) putbq(q, mp);
			splx(s);
			return;
		}
		(void) putnext(q, mp);
	}
	if (tp->t_state & TBLOCK) {
		(void) HW_PROC(tp, T_UNBLOCK);
	}
	splx(s);
}

/* ARGSUSED */
iasyosrv(q)
queue_t *q;		/* Write queue pointer */
{
#ifdef lint
	void iasyctime();
	void iasyhwdep();

	if (iasyinfo.st_rdinit)	/* Bogus references to keep lint happy */
		iasyosrv(q);
	iasyhwdep(q, (mblk_t *)0);
	iasy_ctime((struct strtty *)0, 1);
#endif
	return;
}

/*
 *	Modify your interrupt thread to use this routine instead of l_input.  It
 *	takes the data from tp->t_in, ships it upstream to the line discipline,
 *	and allocates another buffer for tp->t_in.
*/
void
iasy_input(tp, cmd)
struct strtty *tp;		/* Device with input to report */
int cmd;				/* L_BUF or L_BREAK */
{	queue_t *q;
	mblk_t *bp;
	int cnt;

	q = TP_TO_Q(tp);
	if (!q)
		return;
	switch (cmd) {
	case L_BUF:
		cnt = IASY_BUFSZ - tp->t_in.bu_cnt;
		if (cnt && canput(q->q_next)) {
			bp = allocb(IASY_BUFSZ, BPRI_MED);
			if (bp) {	/* pass up old bp contents */
				tp->t_in.bu_bp->b_wptr += cnt;
				tp->t_in.bu_bp->b_datap->db_type = M_DATA;
				(void) putnext(q, tp->t_in.bu_bp);
				tp->t_in.bu_bp = bp;
			} /* else drop characters */
		} /* else drop characters */
		tp->t_in.bu_cnt = IASY_BUFSZ;	/* Reset to go again */
		tp->t_in.bu_ptr = tp->t_in.bu_bp->b_wptr;
		break;
	case L_BREAK:
		(void) putctl(q->q_next, M_BREAK);	/* signal "break detected" */
		break;
	default:
		cmn_err(CE_WARN, "iasy_input: unknown command\n");
	}
}

/*
 *	Modify your interrupt thread to use this routine instead of l_output.
 *	It retrieves the next output block from the stream and hooks it into
 *	tp->t_out.
*/
int
iasy_output(tp)
struct strtty *tp;		/* Device desiring to get more output */
{	queue_t *q;
	mblk_t *bp;

	if (tp->t_out.bu_bp) {
		freeb((mblk_t *)tp->t_out.bu_bp);	/* As stashed by previous call */
		tp->t_out.bu_bp = 0;
		tp->t_out.bu_cnt = 0;
	}
	q = TP_TO_Q(tp);
	if (!q)
		return(0);
	q = WR(q);
	bp = iasygetoblk(q);
	if (bp) {
		/*
		 *	Our put procedure insures each message consists of one
		 *	block.  Give the block to the user.
		*/
		tp->t_out.bu_ptr = bp->b_rptr;
		tp->t_out.bu_cnt = bp->b_wptr - bp->b_rptr;
		tp->t_out.bu_bp = bp;
		return(CPRES);
	}
	if (tp->t_state & TTIOW) {
		tp->t_state &= ~TTIOW;
		(void) wakeup((caddr_t)&tp->t_oflag);
	}
	return(0);
}

/*
 *	Register a terminal server.  This makes an interrupt thread
 *	available via the iasy major number.
*/
struct strtty *
iasy_register(fmin, count, proc, hwdep)
minor_t fmin; 		/* Starting minor number */
int count;			/* Number of minor numbers requested */
int  (*proc)();		/* proc routine */
void (*hwdep)();	/* Hardware dependant ioctl routine */
{	struct iasy_hw *hp;
	minor_t i;
	minor_t lmin;

	if (count == 0)
		return(iasy_tty);
	lmin = fmin + count - 1;
	/*
	 *	Scan for allocation problems
	*/
	hp = iasy_hw + fmin;
	for (i = fmin; i <= lmin; i++, hp++) {
		if (i >= iasy_num) {
			cmn_err(CE_WARN,
					"iasy_register: minor %d is out of range\n", i);
			return(0);
		}
		if (hp->proc) {
			cmn_err(CE_WARN,
					"iasy_register: minor %d conflict  0x%x vs 0x%x\n",
					i, hp->proc, proc);
			return(0);
		}
	}
	/*
	 *	Allocate the range of minor numbers
	*/
	hp = iasy_hw + fmin;
	for (i = fmin; i <= lmin; i++, hp++) {
		hp->proc = proc;
		hp->hwdep = hwdep;
	}
	if (iasy_cnt < lmin)
		iasy_cnt = lmin;
	return(iasy_tty);
}

/*
 *	Default Hardware dependant ioctl support (i.e. none).
 *	Use this routine as your hwdep() routine if you don't have any
 *	special ioctls to implement.
*/
/* ARGSUSED */
void
iasyhwdep(q, bp)
queue_t *q;	/* Write queue pointer */
mblk_t *bp;	/* This is an ioctl not understood by the DI code */
{	struct iocblk *ioc;

	ioc = (struct iocblk *)bp->b_rptr;
	switch (bp->b_datap->db_type) {
	case M_IOCTL:
		ioc->ioc_error = EINVAL;		/* NACK unknown ioctls */
		ioc->ioc_rval = -1;
		bp->b_datap->db_type = M_IOCNAK;
		(void) putnext(RD(q), bp);
		return;
	default:
		cmn_err(CE_PANIC, "iasyhwdep: illegal message type");
	}
}

/*
 *	Send a hangup upstream to indicate loss of the connection.
*/
void
iasy_hup(tp)
struct strtty *tp;
{	queue_t *q;

	q = TP_TO_Q(tp);
	if (!q)
		return;
	iasyflush(WR(q), FLUSHR|FLUSHW);
	(void) putctl(q->q_next, M_HANGUP);
}

/*
 *	Delay "count" character times to allow for devices which prematurely
 *	clear BUSY.
*/
void
iasy_ctime(tp, count)
struct strtty *tp;
int count;
{	register int	oldpri;
	static	int	rate[] = {
		HZ+1,	/* avoid divide-by-zero, as well as unnecessary delay */
		50,
		75,
		110,
		134,
		150,
		200,
		300,
		600,
		1200,
		1800,
		2400,
		4800,
		9600,
		19200,
		38400,
	};
	/*
	 *	Delay 11 bit times to allow uart to empty.
	 *	Add one to allow for truncation and one to
	 *	allow for partial clock tick.
	*/
	count *= 1 + 1 + 11*HZ/rate[tp->t_cflag&CBAUD];
	oldpri = SPL();
	tp->t_state |= TIMEOUT;
	(void) timeout(iasydelay, (caddr_t)tp, count);
	splx(oldpri);
}
