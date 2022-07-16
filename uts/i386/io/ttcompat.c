/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ttcompat.c	1.3.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 * Module to intercept old V7, 4BSD and XENIX "ioctl" calls.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/termios.h>
#include <sys/ttold.h>
#include <sys/cmn_err.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/ttcompat.h>

STATIC int ttcompatopen(/*queue_t *q, int dev, int oflag, int sflag*/);
STATIC int ttcompatclose(/*queue_t *q*/);
STATIC int ttcompatrput(/*queue_t *q, mblk_t *mp*/);
STATIC int ttcompatwput(/*queue_t *q, mblk_t *mp*/);

static struct module_info ttycompatmiinfo = {
	0,
	"ttcompat",
	0,
	INFPSZ,
	2048,
	128
};

static struct qinit ttycompatrinit = {
	ttcompatrput,
	NULL,
	ttcompatopen,
	ttcompatclose,
	NULL,
	&ttycompatmiinfo
};

static struct module_info ttycompatmoinfo = {
	42,
	"ttcompat",
	0,
	INFPSZ,
	300,
	200
};

static struct qinit ttycompatwinit = {
	ttcompatwput,
	NULL,
	ttcompatopen,
	ttcompatclose,
	NULL,
	&ttycompatmoinfo
};

struct streamtab ttcoinfo = {
	&ttycompatrinit,
	&ttycompatwinit,
	NULL,
	NULL
};

STATIC void ttcompat_do_ioctl(/*queue_t *q, mblk_t *mp*/);
STATIC void ttcompat_ioctl_ack(/*queue_t *q, mblk_t *mp*/);
STATIC void ttcopyout(/*queue_t *q, mblk_t *mp*/);
STATIC void ttcompat_ioctl_nak(/*queue_t *q, mblk_t *mp*/);
STATIC void from_compat(/*compat_state_t *csp, struct termios *termiosp*/);
STATIC void to_compat(/*struct termios *termiosp, compat_state_t *csp*/);


/* XENIX functions supplied by a 3rd party package for internationlization*/
/* defined in stubs */

extern void emsetmap(/*compat_state_t *tp, mblk_t *mp*/);
extern void emgetmap(/*compat_state_t *tp, mblk_t *mp*/);
extern void emunmap(/*compat_state_t *tp*/);

/*
 * Open - get the current modes and translate them to the V7/4BSD equivalent.
 */
/*ARGSUSED*/
STATIC int
ttcompatopen(q, dev, oflag, sflag)
	queue_t *q;
	int dev, oflag, sflag;
{
	register ttcompat_state_t *tp;
	register mblk_t *bp;


	if (q->q_ptr != NULL)  {
		tp = (ttcompat_state_t *)q->q_ptr;
		/* fail open if TIOCEXCL was done and its not super user */
		if ((tp->t_new_lflags & XCLUDE) && (!suser(u.u_cred))) {
			u.u_error = EBUSY;
			return(OPENFAIL);
		}
		else
			return(0);		/* already attached */
	}

	if ((bp = allocb((int)sizeof (ttcompat_state_t), BPRI_MED)) == NULL) {
		printf("ttcompatopen: open fails, can't allocate state structure\n");
		return(OPENFAIL);
	}
	bp->b_wptr += sizeof (ttcompat_state_t);
	tp = (ttcompat_state_t *)bp->b_rptr;
	bzero(tp,sizeof(ttcompat_state_t));
	tp->t_savbp = bp;
	tp->t_iocpending = NULL;

	tp->t_curstate.t_flags = 0;
	tp->t_curstate.t_ispeed = B0;
	tp->t_curstate.t_ospeed = B0;
	tp->t_curstate.t_erase = '\0';
	tp->t_curstate.t_kill = '\0';
	tp->t_curstate.t_intrc = '\0';
	tp->t_curstate.t_quitc = '\0';
	tp->t_curstate.t_startc = '\0';
	tp->t_curstate.t_stopc = '\0';
	tp->t_curstate.t_eofc = '\0';
	tp->t_curstate.t_brkc = '\0';
	tp->t_curstate.t_suspc = '\0';
	tp->t_curstate.t_dsuspc = '\0';
	tp->t_curstate.t_rprntc = '\0';
	tp->t_curstate.t_flushc = '\0';
	tp->t_curstate.t_werasc = '\0';
	tp->t_curstate.t_lnextc = '\0';
	tp->t_curstate.t_xflags = 0;
	tp->t_curstate.t_lflag = 0;
	tp->t_curstate.t_iflag = 0;

	q->q_ptr = (caddr_t)tp;
	WR(q)->q_ptr = (caddr_t)tp;
	return(0);
}

STATIC int
ttcompatclose(q)
	register queue_t *q;
{
	register ttcompat_state_t *tp = (ttcompat_state_t *)q->q_ptr;
	register mblk_t *mp;

	/* Dump the state structure, then unlink it */
	if ((mp = tp->t_iocpending) != NULL)
		freemsg(mp);
	freeb(tp->t_savbp);
	q->q_ptr = NULL;
}

/*
 * Put procedure for input from driver end of stream (read queue).
 * Most messages just get passed to the next guy up; we intercept
 * "ioctl" replies, and if it's an "ioctl" whose reply we plan to do
 * something with, we do it.
 */
STATIC int
ttcompatrput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	switch (mp->b_datap->db_type) {

	case M_IOCACK:
		ttcompat_ioctl_ack(q, mp);
		break;

	case M_IOCNAK:
		ttcompat_ioctl_nak(q, mp);
		break;

	default:
		putnext(q, mp);
		break;
	}
}

/*
 * Line discipline output queue put procedure: speeds M_IOCTL
 * messages.
 */
STATIC int
ttcompatwput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register ttcompat_state_t *tp;
	struct copyreq *cqp;
	struct copyresp *csp;
	register struct iocblk *iocbp;

	tp = (ttcompat_state_t *)q->q_ptr;

	/*
	 * Process some M_IOCTL messages here; pass everything else down.
	 */
	switch(mp->b_datap->db_type) {

	default:
		putnext(q, mp);
		return;
	
	case M_IOCTL:
		iocbp = (struct iocblk *)mp->b_rptr;

		switch (iocbp->ioc_cmd) {

		default:
	/* these are ioctls with no arguments or are known to stream head */
	/* process them right away */
			ttcompat_do_ioctl(tp, q, mp);
			return;
		case TIOCSETN:
		case TIOCSLTC:
		case TIOCSETC:
		case TIOCLBIS:
		case TIOCLBIC:
		case TIOCLSET:
			if (iocbp->ioc_count != TRANSPARENT) {
				putnext(q, mp);
				return;
			}
			mp->b_datap->db_type = M_COPYIN;
			cqp = (struct copyreq *)mp->b_rptr;
			cqp->cq_addr = (caddr_t) *(long *)mp->b_cont->b_rptr;
			switch (iocbp->ioc_cmd) {
				case TIOCSETN:
					cqp->cq_size = sizeof(struct sgttyb);
					break;
				case TIOCSLTC:
					cqp->cq_size = sizeof(struct ltchars);
					break;
				case TIOCSETC:
					cqp->cq_size = sizeof(struct ltchars);
					break;
				case TIOCLBIS:
					cqp->cq_size = sizeof(struct ltchars);
					break;
				case TIOCLBIC:
					cqp->cq_size = sizeof(struct ltchars);
					break;
				case TIOCLSET:
					cqp->cq_size = sizeof(struct ltchars);
					break;
				default:
					break;
			}
			cqp->cq_flag = 0;
			cqp->cq_private = NULL;
			mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
			tp->t_ioccmd = iocbp->ioc_cmd;
			tp->t_state |= TS_W_IN;
			qreply(q, mp);
			return;

		} /* switch ioc_cmd */
	case M_IOCDATA:
		csp = (struct copyresp *)mp->b_rptr;

		switch (csp->cp_cmd) {

		default:
			putnext(q, mp);
			return;
		case TIOCSETN:
		case TIOCSLTC:
		case TIOCSETC:
		case TIOCLBIS:
		case TIOCLBIC:
		case TIOCLSET:
			tp->t_state &= ~TS_W_IN;
			if (csp->cp_rval) {	/* failure */
				freemsg(mp);
			} else {	/* make it look like an ioctl */
				mp->b_datap->db_type = M_IOCTL;
				mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
				iocbp = (struct iocblk *)mp->b_rptr;
				iocbp->ioc_count = mp->b_cont->b_wptr - mp->b_cont->b_rptr;
				iocbp->ioc_error = 0;
				iocbp->ioc_rval = 0;
				ttcompat_do_ioctl((ttcompat_state_t *)q->q_ptr, q, mp);
			}
			return;

		case TIOCGLTC:
		case TIOCLGET:
		case TIOCGETC:
			tp->t_state &= ~TS_W_OUT;
			if (csp->cp_rval) {	/* failure */
				freemsg(mp);
			} else {
				iocbp = (struct iocblk *)mp->b_rptr;
				iocbp->ioc_count = 0;
				iocbp->ioc_error = 0;
				iocbp->ioc_rval = 0;
				mp->b_datap->db_type = M_IOCACK;
				qreply(q, mp);
			}
			return;

		} /* switch cp_cmd */
	} /* end message switch */
}

/*
 * Retry an "ioctl", now that "bufcall" claims we may be able to allocate
 * the buffer we need.
 */
STATIC int
ttcompat_reioctl(q)
	register queue_t *q;
{
	register ttcompat_state_t *tp;
	register mblk_t *mp;

	if ((tp = (ttcompat_state_t *)q->q_ptr) == NULL)
		return;
	if ((mp = tp->t_iocpending) != NULL) {
		tp->t_iocpending = NULL;	/* not pending any more */
		ttcompat_do_ioctl(tp, q, mp);
	}
}

/*
 * Handle old-style "ioctl" messages; pass the rest down unmolested.
 */
STATIC void
ttcompat_do_ioctl(tp, q, mp)
	register ttcompat_state_t *tp;
	queue_t *q;
	mblk_t *mp;
{
	register struct iocblk *iocp;
	struct copyreq *cqp;
	int s;

	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {

	/*
	 * "get"-style calls that get translated data from the "termios"
	 * structure.  Save the existing code and pass it down as a TCGETS.
	 */
	case TIOCGETC:
	case TIOCLGET:
	case TIOCGLTC:
		if (iocp->ioc_count != TRANSPARENT) {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = 0;
			iocp->ioc_count = 0;
			iocp->ioc_rval = 0;
			qreply(q,mp);
			return;
		}
		cqp = (struct copyreq *)mp->b_rptr;
		cqp->cq_private = (mblk_t *)(*(long *)mp->b_cont->b_rptr);
		/* free the data buffer - it might not be sufficient */
		/* driver will allocate one for termios size */
		if(mp->b_cont)
			freeb(mp->b_cont);
		mp->b_cont = NULL;
		iocp->ioc_count = 0;
 		/* fall thru */
	case TIOCGETP:
		goto dogets;

#ifdef SUN
	/*
	 * "get"-style calls that get translated data from the "winsize"
	 * structure.  Save the existing code and pass it down as a TIOCGWINSZ.
	 */
	case TIOCGSIZE:
	case _O_TIOCGSIZE:	/* XXX */
		tp->t_ioccmd = TIOCGSIZE;	/* map old GSIZE into new */
		tp->t_iocid = iocp->ioc_id;
		tp->t_state |= TS_IOCWAIT;
		iocp->ioc_cmd = TIOCGWINSZ;	/* turn it into a TIOCGWINSZ */
		break;
#endif /* SUN */

	/*
	 * "set"-style calls that set translated data into a "termios"
	 * structure.  Set our idea of the new state from the value
	 * given to us.  We then have to get the current state, so we
	 * turn this guy into a TCGETS and pass it down.  When the
	 * ACK comes back, we modify the state we got back and shove it
	 * back down as the appropriate type of TCSETS.
	 */
	case TIOCSETP:
	case TIOCSETN:
		tp->t_new_sgttyb = *((struct sgttyb *)mp->b_cont->b_rptr);
		goto dogets;

	case TIOCSETC:
		tp->t_new_tchars = *((struct tchars *)mp->b_cont->b_rptr);
		goto dogets;

	case TIOCSLTC:
		tp->t_new_ltchars = *((struct ltchars *)mp->b_cont->b_rptr);
		goto dogets;

	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
		tp->t_new_lflags = *(int *)mp->b_cont->b_rptr;
		goto dogets;
#ifdef SUN
	case TIOCSETX:
		tp->t_new_xflags = *(int *)mp->b_cont->b_rptr;
		goto dogets;
#endif /* SUN */

	/*
	 * "set"-style call that sets a particular bit in a "termios"
	 * structure.  We then have to get the current state, so we
	 * turn this guy into a TCGETS and pass it down.  When the
	 * ACK comes back, we modify the state we got back and shove it
	 * back down as the appropriate type of TCSETS.
	 */
	case TIOCHPCL:
	dogets:
		tp->t_ioccmd = iocp->ioc_cmd;
		tp->t_iocid = iocp->ioc_id;
		tp->t_state |= TS_IOCWAIT;
		iocp->ioc_cmd = TCGETS;
		iocp->ioc_count = 0;	/* no data returned unless we say so */
		break;

#ifdef SUN
	case TIOCSWINSZ:
		/*
		 * Unfortunately, TIOCSWINSZ and the old TIOCSSIZE "ioctl"s
		 * share the same code.  If the upper 16 bits of the number
		 * of lines is non-zero, it was probably a TIOCSWINSZ,
		 * with both "ws_row" and "ws_col" non-zero.  Otherwise, it was
		 * probably an old TIOCSSIZE "ioctl"; we turn it into a
		 * new-style TIOCSSIZE "ioctl" so that nobody below us has to
		 * know about this stuff.
		 */
		if ((((struct ttysize *)mp->b_cont->b_rptr)->ts_lines&0xffff0000) == 0) {
			/*
			 * It's an old-style TIOCSSIZE; map it into a new-style
			 * one.
			 */
			iocp->ioc_cmd = TIOCSSIZE;
		}
		break;
#endif /* SUN */

	/*
	 * "set"-style call that sets DTR.  Pretend that it was a TIOCMBIS
	 * with TIOCM_DTR set.
	 */
	case TIOCSDTR: {
		register mblk_t *datap;

		if ((datap = allocb((int)sizeof (int), BPRI_HI)) == NULL)
			goto allocfailure;
		*(int *)datap->b_wptr = TIOCM_DTR;
		datap->b_wptr += (sizeof (int))/(sizeof *datap->b_wptr);
		iocp->ioc_cmd = TIOCMBIS;	/* turn it into a TIOCMBIS */
		if (mp->b_cont != NULL)
			freemsg(mp->b_cont);
		mp->b_cont = datap;	/* attach the data */
		break;
	}

	/*
	 * "set"-style call that clears DTR.  Pretend that it was a TIOCMBIC
	 * with TIOCM_DTR set.
	 */
	case TIOCCDTR: {
		register mblk_t *datap;

		if ((datap = allocb((int)sizeof (int), BPRI_HI)) == NULL)
			goto allocfailure;
		*(int *)datap->b_wptr = TIOCM_DTR;
		datap->b_wptr += (sizeof (int))/(sizeof *datap->b_wptr);
		iocp->ioc_cmd = TIOCMBIC;	/* turn it into a TIOCMBIC */
		if (mp->b_cont != NULL)
			freemsg(mp->b_cont);
		mp->b_cont = datap;	/* attach the data */
		break;
	}

	/*
	 * Translate into the S5 form of TCFLSH.
	 */
	case TIOCFLUSH: {
		register int flags = *(int *)mp->b_cont->b_rptr;

		switch (flags&(FREAD|FWRITE)) {

		case 0:
		case FREAD|FWRITE:
			flags = 2;	/* flush 'em both */
			break;

		case FREAD:
			flags = 0;	/* flush read */
			break;

		case FWRITE:
			flags = 1;	/* flush write */
			break;
		}
		iocp->ioc_cmd = TCFLSH;	/* turn it into a TCFLSH */
		*(int *)mp->b_cont->b_rptr = flags;	/* fiddle the arg */
		break;
	}

	/*
	 * Turn into a TCXONC.
	 */
	case TIOCSTOP: {
		register mblk_t *datap;

		if ((datap = allocb((int)sizeof (int), BPRI_HI)) == NULL)
			goto allocfailure;
		*(int *)datap->b_wptr = 0;	/* stop */
		datap->b_wptr += (sizeof (int))/(sizeof *datap->b_wptr);
		iocp->ioc_cmd = TCXONC;	/* turn it into a XONC */
		iocp->ioc_count = 0; /* set it to !TRANSPARENT */
		if (mp->b_cont != NULL)
			freemsg(mp->b_cont);
		mp->b_cont = datap;	/* attach the data */
		break;
	}

	case TIOCSTART: {
		register mblk_t *datap;

		if ((datap = allocb((int)sizeof (int), BPRI_HI)) == NULL)
			goto allocfailure;
		*(int *)datap->b_wptr = 1;	/* start */
		datap->b_wptr += (sizeof (int))/(sizeof *datap->b_wptr);
		iocp->ioc_cmd = TCXONC;	/* turn it into a XONC */
		iocp->ioc_count = 0; /* set it to !TRANSPARENT */
		if (mp->b_cont != NULL)
			freemsg(mp->b_cont);
		mp->b_cont = datap;	/* attach the data */
		break;
	}
	case TIOCGETD:
	case TIOCSETD:
	case DIOCSETP:
	case DIOCGETP:
	case LDOPEN:
	case LDCLOSE:
	case LDCHG:
	case LDSETT:
	case LDGETT:
	/* case FIORDCHK: handled in the stream head */
	 	/* all the ioctls just get acked */	
		/* set and get allowed for only line discipline zero */
		/* if its not line discipline zero, TIOCSETD is naked */ 

		if ((iocp->ioc_cmd == TIOCSETD) && (*mp->b_cont->b_rptr != 0))
			mp->b_datap->db_type = M_IOCNAK;
		else
			mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q,mp);
		return;
	case IOCTYPE:
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_count = 0;
		iocp->ioc_rval = TIOC;
		qreply(q,mp);
		return;
	case TIOCEXCL:
		/* check for binary value of XCLUDE flag ???? */
		tp->t_new_lflags |= XCLUDE;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q,mp);
		return;
	case TIOCNXCL:
		tp->t_new_lflags &= ~XCLUDE;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		qreply(q,mp);
		return;
	}

	/*
	 * We don't reply to most calls, we just pass them down,
	 * possibly after modifying the arguments.
	 */
	putnext(q, mp);
	return;

allocfailure:
	/*
	 * We needed to allocate something to handle this "ioctl", but
	 * couldn't; save this "ioctl" and arrange to get called back when
	 * it's more likely that we can get what we need.
	 * If there's already one being saved, throw it out, since it
	 * must have timed out.
	 */
	s = splstr();
	if (tp->t_iocpending != NULL)
		freemsg(tp->t_iocpending);
	tp->t_iocpending = mp;	/* hold this ioctl */
	/* (void) */ splx(s);
	(void) bufcall((u_int)sizeof (struct iocblk), BPRI_HI,
	    ttcompat_reioctl, (long)q);
}

/*
 * Called when an M_IOCACK message is seen on the read queue; if this
 * is the response we were waiting for, we either:
 *    modify the data going up (if the "ioctl" read data); since in all
 *    cases, the old-style returned information is smaller than or the same
 *    size as the new-style returned information, we just overwrite the old
 *    stuff with the new stuff (beware of changing structure sizes, in case
 *    you invalidate this)
 * or
 *    take this data, modify it appropriately, and send it back down (if
 *    the "ioctl" wrote data).
 * In either case, we cancel the "wait"; the final response to a "write"
 * ioctl goes back up to the user.
 * If this wasn't the response we were waiting for, just pass it up.
 */
STATIC void
ttcompat_ioctl_ack(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	register ttcompat_state_t *tp;
	register struct iocblk *iocp;
	register mblk_t *datap;

	tp = (ttcompat_state_t *)q->q_ptr;
	iocp = (struct iocblk *)mp->b_rptr;

	if (!(tp->t_state&TS_IOCWAIT) || iocp->ioc_id != tp->t_iocid) {
		/*
		 * This isn't the reply we're looking for.  Move along.
		 */
		putnext(q, mp);
		return;
	}

	datap = mp->b_cont;	/* mblk containing data going up */

	switch (tp->t_ioccmd) {

	case TIOCGETP: {
		register struct sgttyb *cb;

		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		datap->b_wptr = datap->b_datap->db_base;
			/* recycle the reply's buffer */
		cb = (struct sgttyb *)datap->b_wptr;
		cb->sg_ispeed = tp->t_curstate.t_ispeed;
		cb->sg_ospeed = tp->t_curstate.t_ospeed;
		cb->sg_erase = tp->t_curstate.t_erase;
		cb->sg_kill = tp->t_curstate.t_kill;
		cb->sg_flags = tp->t_curstate.t_flags;
		datap->b_wptr += (sizeof (struct sgttyb))/(sizeof *datap->b_wptr);
		iocp->ioc_count = sizeof (struct sgttyb);

		/* you are lucky - strema head knows how to copy you out */ 

		tp->t_state &= ~TS_IOCWAIT;	/* we got what we wanted */
		iocp->ioc_rval = 0;
		iocp->ioc_cmd =  tp->t_ioccmd;
		putnext(q, mp);  
		return;
	}

	case TIOCGETC:
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		datap->b_wptr = datap->b_datap->db_base;
			/* recycle the reply's buffer */
		bcopy((caddr_t)&tp->t_curstate.t_intrc, (caddr_t)datap->b_wptr,
		    sizeof (struct tchars));
		datap->b_wptr += (sizeof (struct tchars))/(sizeof *datap->b_wptr);
		break;

	case TIOCGLTC:
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		datap->b_wptr = datap->b_datap->db_base;
			/* recycle the reply's buffer */
		bcopy((caddr_t)&tp->t_curstate.t_suspc, (caddr_t)datap->b_wptr,
		    sizeof (struct ltchars));
		datap->b_wptr += (sizeof (struct ltchars))/(sizeof *datap->b_wptr);
		break;

	case TIOCLGET:
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		datap->b_wptr = datap->b_datap->db_base;
			/* recycle the reply's buffer */
		*(int *)datap->b_wptr = ((unsigned) tp->t_curstate.t_flags) >> 16;
		datap->b_wptr += (sizeof (int))/(sizeof *datap->b_wptr);
#if 0
		iocp->ioc_count = sizeof (int);
		tp->t_state &= ~TS_IOCWAIT;	/* we got what we wanted */
		iocp->ioc_rval = 0;
		iocp->ioc_cmd =  tp->t_ioccmd;
		putnext(q, mp);  
		return;
#endif
		break;

#ifdef SUN
	case TIOCGETX:
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		datap->b_wptr = datap->b_datap->db_base;
			/* recycle the reply's buffer */
		*(int *)datap->b_wptr = tp->t_curstate.t_xflags;
		datap->b_wptr += (sizeof (int))/(sizeof *datap->b_wptr);
		iocp->ioc_count = sizeof (int);
		break;
	case TIOCGSIZE: {
		struct winsize winsize;
		register struct ttysize *cb;

		winsize = *(struct winsize *)datap->b_rptr;
		datap->b_wptr = datap->b_datap->db_base;
			/* recycle the reply's buffer */
		cb = (struct ttysize *)datap->b_wptr;
		cb->ts_lines = winsize.ws_row;
		cb->ts_cols = winsize.ws_col;
		datap->b_wptr += (sizeof (struct ttysize))/(sizeof *datap->b_wptr);
		iocp->ioc_count = sizeof (struct ttysize);
		break;
	}
#endif /* SUN */

	case TIOCSETP:
	case TIOCSETN:
		/*
		 * Get the current state from the GETS data, and
		 * update it.
		 */
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		tp->t_curstate.t_erase = tp->t_new_sgttyb.sg_erase;
		tp->t_curstate.t_kill = tp->t_new_sgttyb.sg_kill;
		tp->t_curstate.t_ispeed = tp->t_new_sgttyb.sg_ispeed;
		tp->t_curstate.t_ospeed = tp->t_new_sgttyb.sg_ospeed;
		tp->t_curstate.t_flags =
		    (tp->t_curstate.t_flags & 0xffff0000) | (tp->t_new_sgttyb.sg_flags & 0xffff);

		/*
		 * Replace the data that came up with the updated data.
		 */
		from_compat(&tp->t_curstate, (struct termios *)datap->b_rptr);

		/*
		 * Send it back down as a TCSETS or TCSETSF.
		 */
		iocp->ioc_cmd = (tp->t_ioccmd == TIOCSETP) ? TCSETSF : TCSETS;
		goto senddown;

	case TIOCSETC:
		/*
		 * Get the current state from the GETS data, and
		 * update it.
		 */
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		bcopy((caddr_t)&tp->t_new_tchars,
		    (caddr_t)&tp->t_curstate.t_intrc, sizeof (struct tchars));

		/*
		 * Replace the data that came up with the updated data.
		 */
		from_compat(&tp->t_curstate, (struct termios *)datap->b_rptr);

		/*
		 * Send it back down as a TCSETS.
		 */
		iocp->ioc_cmd = TCSETS;
		goto senddown;

	case TIOCSLTC:
		/*
		 * Get the current state from the GETS data, and
		 * update it.
		 */
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		bcopy((caddr_t)&tp->t_new_ltchars,
		    (caddr_t)&tp->t_curstate.t_suspc, sizeof (struct ltchars));

		/*
		 * Replace the data that came up with the updated data.
		 */
		from_compat(&tp->t_curstate, (struct termios *)datap->b_rptr);

		/*
		 * Send it back down as a TCSETS.
		 */
		iocp->ioc_cmd = TCSETS;
		goto senddown;

	case TIOCLBIS:
		/*
		 * Get the current state from the GETS data, and
		 * update it.
		 */
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		tp->t_curstate.t_flags |= (tp->t_new_lflags << 16);

		/*
		 * Replace the data that came up with the updated data.
		 */
		from_compat(&tp->t_curstate, (struct termios *)datap->b_rptr);

		/*
		 * Send it back down as a TCSETS.
		 */
		iocp->ioc_cmd = TCSETS;
		goto senddown;

	case TIOCLBIC:
		/*
		 * Get the current state from the GETS data, and
		 * update it.
		 */
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		tp->t_curstate.t_flags &= ~(tp->t_new_lflags << 16);

		/*
		 * Replace the data that came up with the updated data.
		 */
		from_compat(&tp->t_curstate, (struct termios *)datap->b_rptr);

		/*
		 * Send it back down as a TCSETS.
		 */
		iocp->ioc_cmd = TCSETS;
		goto senddown;

	case TIOCLSET:
		/*
		 * Get the current state from the GETS data, and
		 * update it.
		 */
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		tp->t_curstate.t_flags &= 0xffff;
		tp->t_curstate.t_flags |= (tp->t_new_lflags << 16);

		/*
		 * Replace the data that came up with the updated data.
		 */
		from_compat(&tp->t_curstate, (struct termios *)datap->b_rptr);

		/*
		 * Send it back down as a TCSETS.
		 */
		iocp->ioc_cmd = TCSETS;
		goto senddown;
#ifdef SUN
	case TIOCSETX:
		/*
		 * Get the current state from the GETS data, and
		 * update it.
		 */
		to_compat((struct termios *)datap->b_rptr, &tp->t_curstate);
		tp->t_curstate.t_xflags = tp->t_new_xflags;

		/*
		 * Replace the data that came up with the updated data.
		 */
		from_compat(&tp->t_curstate, (struct termios *)datap->b_rptr);

		/*
		 * Send it back down as a TCSETS.
		 */
		iocp->ioc_cmd = TCSETS;
		goto senddown;
#endif /* SUN */

	case TIOCHPCL:
		/*
		 * Replace the data that came up with the updated data.
		 */
		((struct termios *)datap->b_rptr)->c_cflag |= HUPCL;

		/*
		 * Send it back down as a TCSETS.
		 */
		iocp->ioc_cmd = TCSETS;
		goto senddown;

	default:
		cmn_err(CE_NOTE,"ttcompat: unexpected ioctl acknowledgment");
		putnext(q, mp);
		return;
	}

	/*
	 * All the calls that return something return 0.
	 */
	tp->t_state &= ~TS_IOCWAIT;	/* we got what we wanted */
	iocp->ioc_rval = 0;
/*	putnext(q, mp);  */

	/* copy out the data - ioctl transparency */
	iocp->ioc_cmd =  tp->t_ioccmd;
	ttcopyout(q,mp);
	return;

senddown:
	/*
	 * Send a "get state" reply back down, with suitably-modified
	 * state, as a "set state" "ioctl".
	 */
	tp->t_state &= ~TS_IOCWAIT;
	mp->b_datap->db_type = M_IOCTL;
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	putnext(WR(q), mp);
}
/* Called from ttcompatrput M_IOCACK processing. */
/* Copies out the data using M_COPYOUT messages */

STATIC void
ttcopyout(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	struct copyreq *cqp;
	register ttcompat_state_t *tp;

	tp = (ttcompat_state_t *)q->q_ptr;

	mp->b_datap->db_type = M_COPYOUT;
	cqp = (struct copyreq *)mp->b_rptr;
	cqp->cq_addr = (caddr_t)cqp->cq_private;
	switch (tp->t_ioccmd) {
		case TIOCGLTC:
			cqp->cq_size = sizeof(struct ltchars);
			break;
		case TIOCGETC:
			cqp->cq_size = sizeof(struct tchars);
			break;
		case TIOCLGET:
			cqp->cq_size = sizeof(int);
			break;
		default:
			cmn_err(CE_CONT,"TTCOMPAT: Unknown ioctl to copyout\n");
			break;
		}
	cqp->cq_flag = 0;
	cqp->cq_private = NULL;
	tp->t_state |= TS_W_OUT;
	putnext(q, mp);
	return;
}


/*
 * Called when an M_IOCNAK message is seen on the read queue; if this is
 * the response we were waiting for, cancel the wait.  Pass the reply up;
 * if we were waiting for this response, we can't complete the "ioctl" and
 * the NAK will tell that to the guy above us.
 * If this wasn't the response we were waiting for, just pass it up.
 */
STATIC void
ttcompat_ioctl_nak(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register ttcompat_state_t *tp;
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	tp = (ttcompat_state_t *)q->q_ptr;

	if (tp->t_state&TS_IOCWAIT && iocp->ioc_id == tp->t_iocid)
		tp->t_state &= ~TS_IOCWAIT;	/* this call isn't going through */
	putnext(q, mp);
}

#define	FROM_COMPAT_CHAR(to, from) if ((to = from) == 0377) to = 0

STATIC void
from_compat(csp, termiosp)
	register compat_state_t *csp;
	register struct termios *termiosp;
{
	termiosp->c_iflag = 0;
	termiosp->c_oflag &= (ONLRET|ONOCR);

#ifdef SUN
	termiosp->c_cflag = (termiosp->c_cflag & (CRTSCTS|LOBLK|HUPCL))|(csp->t_ospeed & CBAUD)|CREAD;
#else
	termiosp->c_cflag = (termiosp->c_cflag & (LOBLK|HUPCL))|(csp->t_ospeed & CBAUD)|CREAD;
#endif /* SUN */

	if (csp->t_ospeed != csp->t_ispeed) {
		termiosp->c_cflag |= (csp->t_ispeed&CBAUD)<<IBSHIFT;
		if (csp->t_ispeed == 0)
			termiosp->c_cflag &= ~CBAUD;	/* hang up if ispeed is 0 */
	}
	if ((csp->t_ispeed & CBAUD) == B110 || csp->t_xflags & STOPB)
		termiosp->c_cflag |= CSTOPB;
	termiosp->c_lflag = csp->t_lflag;
	FROM_COMPAT_CHAR(termiosp->c_cc[VERASE], csp->t_erase);
	FROM_COMPAT_CHAR(termiosp->c_cc[VKILL], csp->t_kill);
	FROM_COMPAT_CHAR(termiosp->c_cc[VINTR], csp->t_intrc);
	FROM_COMPAT_CHAR(termiosp->c_cc[VQUIT], csp->t_quitc);
	FROM_COMPAT_CHAR(termiosp->c_cc[VSTART], csp->t_startc);
	FROM_COMPAT_CHAR(termiosp->c_cc[VSTOP], csp->t_stopc);
	termiosp->c_cc[VEOL2] = 0;
	FROM_COMPAT_CHAR(termiosp->c_cc[VSUSP], csp->t_suspc);
	FROM_COMPAT_CHAR(termiosp->c_cc[VDSUSP], csp->t_dsuspc);	/* is this useful? */
	FROM_COMPAT_CHAR(termiosp->c_cc[VREPRINT], csp->t_rprntc);
	FROM_COMPAT_CHAR(termiosp->c_cc[VDISCARD], csp->t_flushc);
	FROM_COMPAT_CHAR(termiosp->c_cc[VWERASE], csp->t_werasc);
	FROM_COMPAT_CHAR(termiosp->c_cc[VLNEXT], csp->t_lnextc);
	if (csp->t_flags & O_TANDEM)
		termiosp->c_iflag |= IXOFF;
	if (csp->t_flags & O_LCASE) {
		termiosp->c_iflag |= IUCLC;
		termiosp->c_oflag |= OLCUC;
		termiosp->c_lflag |= XCASE;
	}
	if (csp->t_flags & O_ECHO)
		termiosp->c_lflag |= ECHO;
	if (csp->t_flags & O_CRMOD) {
		termiosp->c_iflag |= ICRNL;
		termiosp->c_oflag |= ONLCR;
		switch (csp->t_flags & O_CRDELAY) {

		case O_CR1:
			termiosp->c_oflag |= CR2;
			break;

		case O_CR2:
			termiosp->c_oflag |= CR3;
			break;
		}
	} else {
		if ((csp->t_flags & O_NLDELAY) == O_NL1)
			termiosp->c_oflag |= ONLRET|CR1;	/* tty37 */
	}
	if ((csp->t_flags & O_NLDELAY) == O_NL2)
		termiosp->c_oflag |= NL1;
	if (csp->t_flags & O_RAW) {
		termiosp->c_cflag |= CS8;
		termiosp->c_iflag &= ~(ICRNL|IUCLC);
		termiosp->c_lflag &= ~XCASE;
	} else {
		termiosp->c_iflag |= csp->t_iflag;
		if (csp->t_intrc >= 0)
			termiosp->c_iflag |= BRKINT;
		if (termiosp->c_cc[VSTOP] != 0 && termiosp->c_cc[VSTART] != 0)
			termiosp->c_iflag |= IXON;
		if (csp->t_flags & O_LITOUT)
			termiosp->c_cflag |= CS8;
		else {
			if (csp->t_flags & O_PASS8)
				termiosp->c_cflag |= CS8;
				/* XXX - what about 8 bits plus parity? */
			else {
				switch (csp->t_flags & (O_EVENP|O_ODDP)) {

				case 0:
					termiosp->c_iflag |= ISTRIP;
					termiosp->c_cflag |= CS8;
					break;

				case O_EVENP:
					termiosp->c_iflag |= INPCK|ISTRIP;
					termiosp->c_cflag |= CS7|PARENB;
					break;

				case O_ODDP:
					termiosp->c_iflag |= INPCK|ISTRIP;
					termiosp->c_cflag |= CS7|PARENB|PARODD;
					break;

				case O_EVENP|O_ODDP:
					termiosp->c_iflag |= ISTRIP;
					termiosp->c_cflag |= CS7|PARENB;
					break;
				}
			}
			if (!(csp->t_xflags & NOPOST))
				termiosp->c_oflag |= OPOST;
		}
		if (!(csp->t_xflags & NOISIG))
			termiosp->c_lflag |= ISIG;
		if (!(csp->t_flags & O_CBREAK))
			termiosp->c_lflag |= ICANON;
		if (csp->t_flags & O_CTLECH)
			termiosp->c_lflag |= ECHOCTL;
	}
	switch (csp->t_flags & O_TBDELAY) {

	case O_TAB1:
		termiosp->c_oflag |= TAB1;
		break;

	case O_TAB2:
		termiosp->c_oflag |= TAB2;
		break;

	case O_XTABS:
		termiosp->c_oflag |= TAB3;
		break;
	}
	if (csp->t_flags & O_VTDELAY)
		termiosp->c_oflag |= FFDLY;
	if (csp->t_flags & O_BSDELAY)
		termiosp->c_oflag |= BSDLY;
	if (csp->t_flags & O_PRTERA)
		termiosp->c_lflag |= ECHOPRT;
	if (csp->t_flags & O_CRTERA)
		termiosp->c_lflag |= ECHOE;
	if (csp->t_flags & O_TOSTOP)
		termiosp->c_lflag |= TOSTOP;
	if (csp->t_flags & O_FLUSHO)
		termiosp->c_lflag |= FLUSHO;
	if (csp->t_flags & O_NOHANG)
		termiosp->c_cflag |= CLOCAL;
#ifdef SUN
	if (csp->t_flags & O_MDMBUF)
		???;
#endif /* SUN */
	if (csp->t_flags & O_CRTKIL)
		termiosp->c_lflag |= ECHOKE;
	if (csp->t_flags & O_PENDIN)
		termiosp->c_lflag |= PENDIN;
	if (!(csp->t_flags & O_DECCTQ))
		termiosp->c_iflag |= IXANY;
	if (csp->t_flags & O_NOFLSH)
		termiosp->c_lflag |= NOFLSH;
	if (termiosp->c_lflag & ICANON) {
		FROM_COMPAT_CHAR(termiosp->c_cc[VEOF], csp->t_eofc);
		FROM_COMPAT_CHAR(termiosp->c_cc[VEOL], csp->t_brkc);
	} else {
		termiosp->c_cc[VMIN] = 1;
		termiosp->c_cc[VTIME] = 0;
	}
}

#define	TO_COMPAT_CHAR(to, from) if ((to = from) == 0) to = 0377

STATIC void
to_compat(termiosp, csp)
	register struct termios *termiosp;
	register compat_state_t *csp;
{	
	csp->t_lflag = 0;
	csp->t_lflag |= termiosp->c_lflag & ECHOK;
	csp->t_lflag |= termiosp->c_lflag & IEXTEN;
	csp->t_iflag = 0;
	csp->t_iflag |= termiosp->c_iflag & IMAXBEL;
	csp->t_iflag |= termiosp->c_iflag & IGNPAR;
	csp->t_xflags &= (NOISIG|NOPOST);
	csp->t_ospeed = termiosp->c_cflag & CBAUD;
	if ((csp->t_ispeed = ((termiosp->c_cflag & CIBAUD)>>IBSHIFT)) == 0)
		csp->t_ispeed = csp->t_ospeed;
	if (termiosp->c_cflag & CSTOPB && (csp->t_ispeed & CBAUD) != B110)
		csp->t_xflags |= STOPB;
	TO_COMPAT_CHAR(csp->t_erase, termiosp->c_cc[VERASE]);
	TO_COMPAT_CHAR(csp->t_kill, termiosp->c_cc[VKILL]);
	TO_COMPAT_CHAR(csp->t_intrc, termiosp->c_cc[VINTR]);
	TO_COMPAT_CHAR(csp->t_quitc, termiosp->c_cc[VQUIT]);
	TO_COMPAT_CHAR(csp->t_startc, termiosp->c_cc[VSTART]);
	TO_COMPAT_CHAR(csp->t_stopc, termiosp->c_cc[VSTOP]);
	TO_COMPAT_CHAR(csp->t_suspc, termiosp->c_cc[VSUSP]);
	TO_COMPAT_CHAR(csp->t_dsuspc, termiosp->c_cc[VDSUSP]);
	TO_COMPAT_CHAR(csp->t_rprntc, termiosp->c_cc[VREPRINT]);
	TO_COMPAT_CHAR(csp->t_flushc, termiosp->c_cc[VDISCARD]);
	TO_COMPAT_CHAR(csp->t_werasc, termiosp->c_cc[VWERASE]);
	TO_COMPAT_CHAR(csp->t_lnextc, termiosp->c_cc[VLNEXT]);
	csp->t_flags &= (O_CTLECH|O_LITOUT|O_PASS8|O_ODDP|O_EVENP);
	if (termiosp->c_iflag & IXOFF)
		csp->t_flags |= O_TANDEM;
	if (!(termiosp->c_iflag & (IMAXBEL|BRKINT|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR|IGNCR|ICRNL|IUCLC|IXON))
	    && !(termiosp->c_oflag & OPOST)
	    && (termiosp->c_cflag & (CSIZE|PARENB)) == CS8
	    && !(termiosp->c_lflag & (ISIG|ICANON|XCASE)))
		csp->t_flags |= O_RAW;
	else {
		if (!(termiosp->c_iflag & IXON)) {
			csp->t_startc = 0377;
			csp->t_stopc = 0377;
		}
		if ((termiosp->c_cflag & (CSIZE|PARENB)) == CS8
		    && !(termiosp->c_oflag & OPOST))
			csp->t_flags |= O_LITOUT;
		else {
			csp->t_flags &= ~O_LITOUT;
			if ((termiosp->c_cflag & (CSIZE|PARENB)) == CS8) {
				if (!(termiosp->c_iflag & ISTRIP))
					csp->t_flags |= O_PASS8;
			} else {
				csp->t_flags &= ~(O_ODDP|O_EVENP|O_PASS8);
				if (termiosp->c_cflag & PARODD)
					csp->t_flags |= O_ODDP;
				else if (termiosp->c_iflag & INPCK)
					csp->t_flags |= O_EVENP;
				else
					csp->t_flags |= O_ODDP|O_EVENP;
			}
			if (!(termiosp->c_oflag & OPOST))
				csp->t_xflags |= NOPOST;
			else
				csp->t_xflags &= ~NOPOST;
		}
		if (!(termiosp->c_lflag & ISIG))
			csp->t_xflags |= NOISIG;
		else
			csp->t_xflags &= ~NOISIG;
		if (!(termiosp->c_lflag & ICANON))
			csp->t_flags |= O_CBREAK;
		if (termiosp->c_lflag & ECHOCTL)
			csp->t_flags |= O_CTLECH;
		else
			csp->t_flags &= ~O_CTLECH;
	}
	if (termiosp->c_oflag & OLCUC)
		csp->t_flags |= O_LCASE;
	if (termiosp->c_lflag&ECHO)
		csp->t_flags |= O_ECHO;
	if (termiosp->c_oflag & ONLCR) {
		csp->t_flags |= O_CRMOD;
		switch (termiosp->c_oflag & CRDLY) {

		case CR2:
			csp->t_flags |= O_CR1;
			break;

		case CR3:
			csp->t_flags |= O_CR2;
			break;
		}
	} else {
		if ((termiosp->c_oflag & CR1)
		    && (termiosp->c_oflag & ONLRET))
			csp->t_flags |= O_NL1;	/* tty37 */
	}
	if (termiosp->c_oflag & ONLRET)
	if (termiosp->c_oflag & NL1)
		csp->t_flags |= O_NL2;
	switch (termiosp->c_oflag & TABDLY) {

	case TAB1:
		csp->t_flags |= O_TAB1;
		break;

	case TAB2:
		csp->t_flags |= O_TAB2;
		break;

	case XTABS:
		csp->t_flags |= O_XTABS;
		break;
	}
	if (termiosp->c_oflag & FFDLY)
		csp->t_flags |= O_VTDELAY;
	if (termiosp->c_oflag & BSDLY)
		csp->t_flags |= O_BSDELAY;
	if (termiosp->c_lflag & ECHOPRT)
		csp->t_flags |= O_PRTERA;
	if (termiosp->c_lflag & ECHOE)
		csp->t_flags |= (O_CRTERA|O_CRTBS);
	if (termiosp->c_lflag & TOSTOP)
		csp->t_flags |= O_TOSTOP;
	if (termiosp->c_lflag & FLUSHO)
		csp->t_flags |= O_FLUSHO;
	if (termiosp->c_cflag & CLOCAL)
		csp->t_flags |= O_NOHANG;
#ifdef SUN
	if (?)
		csp->t_flags |= O_MDMBUF;
#endif /* SUN */
	if (termiosp->c_lflag & ECHOKE)
		csp->t_flags |= O_CRTKIL;
	if (termiosp->c_lflag & PENDIN)
		csp->t_flags |= O_PENDIN;
	if (!(termiosp->c_iflag & IXANY))
		csp->t_flags |= O_DECCTQ;
	if (termiosp->c_lflag & NOFLSH)
		csp->t_flags |= O_NOFLSH;
	if (termiosp->c_lflag & ICANON) {
		TO_COMPAT_CHAR(csp->t_eofc, termiosp->c_cc[VEOF]);
		TO_COMPAT_CHAR(csp->t_brkc, termiosp->c_cc[VEOL]);
	} else {
		termiosp->c_cc[VMIN] = 1;
		termiosp->c_cc[VTIME] = 0;
	}
}
