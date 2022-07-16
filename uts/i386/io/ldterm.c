/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:ldterm.c	1.3.2.2"
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
 * Standard Streams Terminal Line Discipline module.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/termio.h>
#include <sys/stream.h>
#include <sys/conf.h>
#include <sys/stropts.h>
#include <sys/strtty.h>
#include <sys/tty.h>
#include <sys/signal.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>
#include <sys/euc.h>
#include <sys/eucioctl.h>
#include <sys/emap.h>
#include <sys/ldterm.h>
#include <sys/systm.h>
#include <sys/cred.h>
#include <sys/ddi.h>

int ldtermopen(/*queue_t *q, dev_t *devp,int oflag, int sflag, cred_t *crp*/);
int ldtermclose(/*queue_t *q, int cflag, cred_t *crp*/);

int ldtermrput(/*queue_t *q, mblk_t *mp*/);
int ldtermrsrv(/*queue_t *q*/);
int ldtermwput(/*queue_t *q, mblk_t *mp*/);

#ifdef DEBUG
int ldterm_debug = 0;
#define	DEBUG1(a)	if (ldterm_debug == 1) printf a
#define	DEBUG2(a)	if (ldterm_debug >= 2) printf a /* allocations */
#define	DEBUG3(a)	if (ldterm_debug >= 3) printf a /* M_CTL Stuff */
#define	DEBUG4(a)	if (ldterm_debug >= 4) printf a /* M_READ Stuff */
#define	DEBUG5(a)	if (ldterm_debug >= 5) printf a
#define	DEBUG6(a)	if (ldterm_debug >= 6) printf a
#define	DEBUG7(a)	if (ldterm_debug >= 7) printf a
#else
#define DEBUG1(a)
#define DEBUG2(a)
#define DEBUG3(a)
#define DEBUG4(a)
#define DEBUG5(a)
#define DEBUG6(a)
#define DEBUG7(a)
#endif /* DEBUG */

/*
 * Since most of the buffering occurs either at the stream head or in
 * the "message currently being assembled" buffer, we have a relatively
 * small input queue, so that blockages above us get reflected fairly
 * quickly to the module below us.  We also have a small maximum packet
 * size, since you can put a message of that size on an empty queue no
 * matter how much bigger than the high water mark it is.
 */
static struct module_info ldtermmiinfo = {
	0x0bad,
	"ldterm",
	0,
	256,
	512,
	200
};

static struct qinit ldtermrinit = {
	ldtermrput,
	ldtermrsrv,
	ldtermopen,
	ldtermclose,
	NULL,
	&ldtermmiinfo
};

static struct module_info ldtermmoinfo = {
	0x0bad,
	"ldterm",
	0,
	INFPSZ,
	1,
	0
};

static struct qinit ldtermwinit = {
	ldtermwput,
	NULL,
	ldtermopen,
	ldtermclose,
	NULL,
	&ldtermmoinfo
};

struct streamtab ldtrinfo = {
	&ldtermrinit,
	&ldtermwinit,
	NULL,
	NULL
};

int ldtermopen_wakeup(/*caddr_t addr;*/);
mblk_t *ldterm_docanon(/*unsigned char c, mblk_t *bpt, int ebsize,
    ldtermstd_state_t *tp*/);
int ldterm_unget(/*ldtermstd_state_t *tp*/);
void ldterm_trim(/*ldtermstd_state_t *tp*/);
void ldterm_rubout(/*unsigned char c, queue_t *q, int ebsize,
    ldtermstd_state_t *tp*/);
int ldterm_tabcols(/*ldtermstd_state_t *tp*/);
void ldterm_erase(/*queue_t *q, int ebsize, ldtermstd_state_t *tp*/);
void ldterm_werase(/*queue_t *q, int ebsize, ldtermstd_state_t *tp*/);
void ldterm_kill(/*queue_t *q, int ebsize, ldtermstd_state_t *tp*/);
void ldterm_reprint(/*queue_t *q, int ebsize, ldtermstd_state_t *tp*/);
mblk_t *ldterm_dononcanon(/*mblk_t *bp, mblk_t *bpt, int ebsize,
    queue_t *q, ldtermstd_state_t *tp*/);
int ldterm_echo(/*unsigned char c, queue_t *q, int ebsize, ldtermstd_state_t *tp*/);
void ldterm_outchar(/*unsigned char c, queue_t *q, int bsize, ldtermstd_state_t *tp*/);
void ldterm_outstring(/*unsigned char *cp, int len, queue_t *q,
    int ebsize, ldtermstd_state_t *tp*/);
mblk_t *newmsg(/*ldtermstd_state_t *tp*/);
void ldterm_msg_upstream(/*queue_t *q, ldtermstd_state_t *tp*/);
mblk_t *ldterm_output_msg(/*queue_t *q, mblk_t *imp, ldtermstd_state_t *tp,
    int echoing*/);
void ldterm_flush_output(/*unsigned char c, queue_t *q, ldtermstd_state_t *tp*/);
void ldterm_dosig(/*queue_t *q, int sig*/);
void ldterm_do_ioctl(/*queue_t *q, mblk_t *mp*/);
int chgstropts(/*struct termios *oldmodep, struct ldtermstd_state_t *tp,
    queue_t *q*/);
void ldterm_ioctl_reply(/*queue_t *q, mblk_t *mp*/);
void ldterm_vmin_timeout(/*queue_t *q*/);
void ldterm_adjust_modes(/* struct ldterm_state_d *tp*/);
void ldterm_euc_erase(/* q, ebsize, tp*/);
void ldterm_eucwarn(/* tp */);
void cp_eucwioc(/* from, to, dir*/);
int ldterm_memwidth(/*c, w*/);
int ldterm_dispwidth(/*c, w, mode*/);
int ldterm_codeset(/*c*/);

int ldtrdevflag = 0; /* Indicates new interface for open and close for cunix */ 

static struct termios initmodes = {
	BRKINT|ICRNL|IXON|ISTRIP,	/* iflag */
	OPOST|ONLCR|TAB3,		/* oflag */
	0,				/* cflag */
	ISIG|ICANON|ECHO|ECHOK,	/* lflag , note IEXTEN is turned off, no extensions*/
	{	CINTR,
		CQUIT,
		CERASE,
		CKILL,
		CEOF,
		CEOL,
		CEOL2,
		CNSWTCH,
		CSTART,
		CSTOP,
		CSUSP,
		CNUL,
		CRPRNT,
		CFLUSH,
		CWERASE,
		CLNEXT,
		0			/* nonexistent STATUS */
	}
};

#define	LDTERM_CHANMAP(tp)	( (tp)->t_emap.t_mstate )

/*
 * Line discipline open.
 */
ldtermopen(q, devp, oflag, sflag, crp)
	register queue_t *q;
	dev_t *devp;
	int oflag;
	int sflag;
	cred_t *crp;
{
	register ldtermstd_state_t *tp;
	register mblk_t *bp, *qryp;
	register struct iocblk *qiocp;
	int s;
	register struct stroptions *strop;

	if (q->q_ptr != NULL)
		return (0);		/* already attached */

	while ((bp = allocb((int)sizeof (ldtermstd_state_t), BPRI_MED)) == NULL) {
		s = splstr();
		cmn_err(CE_WARN, "ldtermopen: open fails, can't allocate state structure - trying bufcall\n");
		(void) bufcall(1, BPRI_HI, ldtermopen_wakeup, (long)&q->q_ptr);
		if (sleep((caddr_t)&q->q_ptr, STIPRI|PCATCH)) {
			(void) splx(s);
			return (EINTR);
		}
		(void) splx(s);
	}
	bp->b_wptr += sizeof (ldtermstd_state_t);
	tp = (ldtermstd_state_t *)bp->b_rptr;
	tp->t_savbp = bp;

	tp->t_modes = initmodes;
	tp->t_amodes = initmodes;
	bzero ((caddr_t)&tp->t_dmodes, sizeof (struct termios));
	bzero ((caddr_t)&tp->t_emap, sizeof (struct emp_tty));

	tp->t_state = 0;

	tp->t_line = 0;
	tp->t_col = 0;

	tp->t_rocount = 0;
	tp->t_rocol = 0;

	tp->t_message = NULL;
	tp->t_endmsg = NULL;
	tp->t_msglen = 0;
	tp->t_rd_request = 0;
	tp->t_tid = 0;

	tp->t_echomp = NULL;

	q->q_ptr = (caddr_t)tp;
	WR(q)->q_ptr = (caddr_t)tp;
	/*
	 * The following for EUC:
	 */
	tp->t_codeset = tp->t_eucleft = tp->t_eucign = 0;
	bzero((caddr_t)&tp->eucwioc, EUCSIZE);	/* zero out eucioc structure */
	tp->eucwioc.eucw[0] = 1;	/* ASCII mem & screen width */
	tp->eucwioc.scrw[0] = 1;
	tp->t_maxeuc = 1;	/* the max length in memory bytes of an EUC character */
	tp->t_eucp = NULL;
	tp->t_eucp_mp = NULL;
	tp->t_eucwarn = 0;	/* no bad chars seen yet */


	/*
	 * Find out if the module below us does canonicalization; if so,
	 * we won't do it ourselves.
	 */

	while ( !(qryp = allocb(sizeof (struct iocblk), BPRI_HI))) {
		s = splstr();
		(void) bufcall(1, BPRI_HI, ldtermopen_wakeup, (long)&q->q_ptr);
		if (sleep((caddr_t)&q->q_ptr, STIPRI|PCATCH)) {
			/* Dump the state structure, then unlink it */
			freeb(tp->t_savbp);
			q->q_ptr = NULL;
			WR(q)->q_ptr = NULL;
			(void) splx(s);
			return (EINTR);
		}
		(void) splx(s);
	}
	/* Formulate an M_CTL message; The first block looks like 
	 * an iocblk. Set the command and datasize. The actual data
	 * will be in the b_cont field.
	 */

	qryp->b_wptr = qryp->b_rptr + sizeof (struct iocblk);
	qryp->b_datap->db_type = M_CTL;
	qiocp = (struct iocblk *) qryp->b_rptr;
	qiocp->ioc_count = 0;	/* count is valid for return only */
	qiocp->ioc_error = 0;
	qiocp->ioc_rval = 0;
	qiocp->ioc_cmd = MC_CANONQUERY;
	putnext (WR(q), qryp);

	/*
	 * Set the high-water and low-water marks on the stream head
	 * to values appropriate for a terminal.  Also set the "vmin" and
	 * "vtime" values to 1 and 0, turn on message-nondiscard mode (as we're
	 * in ICANON mode), and turn on "old-style NODELAY" mode.
	 */
	while ((bp = allocb((int)sizeof (struct stroptions), BPRI_MED)) == NULL) {
		s = splstr();
		(void) bufcall(sizeof (struct stroptions), BPRI_MED,
		    ldtermopen_wakeup, (long)&q->q_ptr);
		if (sleep((caddr_t)&q->q_ptr, STIPRI|PCATCH)) {
			/* Dump the state structure, then unlink it */
			freeb(tp->t_savbp);
			q->q_ptr = NULL;
			WR(q)->q_ptr = NULL;
			(void) splx(s);
			return (EINTR);
		}
		(void) splx(s);
	}
	strop = (struct stroptions *)bp->b_wptr;
	strop->so_flags =
	    SO_READOPT|SO_HIWAT|SO_LOWAT|SO_NDELON|SO_ISTTY|SO_STRHOLD;
	strop->so_readopt = RMSGN;
	strop->so_hiwat = 512;
	strop->so_lowat = 128;
	bp->b_wptr += sizeof (struct stroptions);
	bp->b_datap->db_type = M_SETOPTS;
	putnext(q, bp);

	return (0);	/* this can become a controlling TTY */
}

ldtermopen_wakeup(addr)
	long addr;
{
	wakeup((caddr_t) addr);
}

ldtermclose(q, cflag, crp)
	register queue_t *q;
	int cflag;
	cred_t *crp;
{
	ldtermstd_state_t *tp = (ldtermstd_state_t *)q->q_ptr;
	register mblk_t *bp;
	int s;
	register struct stroptions *strop;

/* 
 * we don't want the close to be interrupted by a timer or by any input.
 * TS_CLOSE and splstrs might be a overkill but is safer across different
 * architectures, where timer interrupts may be of a higher priority than
 * splstr.
 */
	s = splstr();
	tp->t_state |= TS_CLOSE;
	if (tp->t_message != NULL)
		freemsg(tp->t_message);

	/* if there is a pending timeout, clear it */
	if (tp->t_tid && (tp->t_state & TS_TACT)) {
		tp->t_state &= ~TS_TACT;
		tp->t_state &= ~TS_RTO;
		DEBUG4 (("ldtermclose: timer active untimeout called \n"));
		untimeout(tp->t_tid);
		tp->t_tid = 0;
	}
	/*
	 * Reset the high-water and low-water marks on the stream head (?),
	 * turn on byte-stream mode, and turn off "old-style NODELAY" mode.
	 */
	while ((bp = allocb((int)sizeof (struct stroptions), BPRI_MED)) == NULL) {
		(void) bufcall(sizeof (struct stroptions), BPRI_MED,
		    ldtermopen_wakeup, (long)&q->q_ptr);
		if (sleep((caddr_t)&q->q_ptr, STIPRI|PCATCH)) {
			splx(s);	
			return (EINTR);
		}
	}
	strop = (struct stroptions *)bp->b_wptr;
	strop->so_flags = SO_READOPT|SO_NDELOFF;
	strop->so_readopt = RNORM;
	bp->b_wptr += sizeof (struct stroptions);
	bp->b_datap->db_type = M_SETOPTS;
	putnext(q, bp);
	/*
	 * Restart output, since it's probably got nowhere to
	 * go anyway, and we're probably not going to see
	 * another ^Q for a while.
	 */
	if (tp->t_state & TS_TTSTOP) {
		tp->t_state &= ~TS_TTSTOP;
		(void) putctl(WR(q)->q_next, M_START);
	}
	/* 
 	 * The following for EUC:
	 */
	if (tp->t_eucp_mp)
		freemsg(tp->t_eucp_mp);
	tp->t_eucp_mp = NULL;
	tp->t_eucp = NULL;

	/* Dump the state structure, then unlink it */
	freeb(tp->t_savbp);
	tp->t_state &= ~TS_CLOSE;
	q->q_ptr = NULL;
	(void) splx(s);
}

/*
 * Put procedure for input from driver end of stream (read queue).
 */
ldtermrput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	register ldtermstd_state_t *tp;
	register unsigned char c;
	queue_t *wrq = WR(q);		/* write queue of ldterm mod */
	queue_t *nextq = q->q_next;	/* queue below us */
	mblk_t *bp;
	mblk_t *swb;
	struct iocblk *qryp;
	struct iocblk *swiocp;
	register unsigned char *readp;
	register unsigned char *writep;
	struct termios *emodes;		/* effective modes set by driver */

	tp = (ldtermstd_state_t *)q->q_ptr;

	switch (mp->b_datap->db_type) {

	default:
		putq(q, mp);
		return;

	/* 
	 *  Send these up unmolested
	 *
	 */
	case M_PCSIG:
	case M_SIG:

		putnext( q, mp);
		return;

	case M_BREAK:

		/*
		 * We look at the apparent modes here instead of the effective
		 * modes. Effective modes cannot be used if IGNBRK, BRINT
		 * and PARMRK have been negotiated to be handled by the
		 * driver. Since M_BREAK should be sent upstream only if
		 * break processing was not already done, it should be ok
		 * to use the apparent modes. 
		 */
		
		if (!(tp->t_amodes.c_iflag & IGNBRK )) {
			if (tp->t_amodes.c_iflag & BRKINT) {
				ldterm_dosig(q, SIGINT, '\0', M_PCSIG, FLUSHRW);
				freemsg (mp);
			} else if (tp->t_amodes.c_iflag & PARMRK) {
					/*
					 *  Send '\377','\0', '\0'.
					 */
					mp->b_datap->db_type = M_DATA;
					*mp->b_wptr++ = '\377';
					*mp->b_wptr++ = '\0';
					*mp->b_wptr++ = '\0';
					putnext( q, mp);
				} else {
					/*
					 * Act as if a '\0' came in.
					 */
					mp->b_datap->db_type = M_DATA;
					*mp->b_wptr++ = '\0';
					putnext( q, mp);
			}
		
		}else {
			freemsg(mp);
		}
		return;

	case M_CTL:
		DEBUG3(("ldtermrput: M_CTL received\n"));
		/* The M_CTL has been standardized to look like an M_IOCTL
		 * message.
		 */

		if ((mp->b_wptr - mp->b_rptr) != sizeof (struct iocblk)) {
			DEBUG3 (("Non standard M_CTL received by the ldterm module\n"));
			/* May be for someone else; pass it on */
			putnext (q, mp);
			return;
		}
		qryp = (struct iocblk *)mp->b_rptr;

		switch (qryp->ioc_cmd) {

		case MC_PART_CANON:

			DEBUG3(("ldtermrput: M_CTL Query Reply\n"));
			if (!mp->b_cont) {
				DEBUG3 (("No information in Query Message\n"));
				break;
			}
			if ((mp->b_cont->b_wptr - mp->b_cont->b_rptr) ==
			     sizeof (struct termios)) {
				DEBUG3(("ldtermrput: M_CTL GrandScheme\n"));
				/* elaborate turning off scheme */
				emodes = (struct termios *)mp->b_cont->b_rptr;
				bcopy ((caddr_t)emodes, (caddr_t)&tp->t_dmodes, sizeof (struct termios));
				ldterm_adjust_modes (tp);
				break;
			} else {
				DEBUG3 (("Incorrect query replysize\n"));
				break;
			}

		case MC_NO_CANON:
			tp->t_state |= TS_NOCANON;
			/*
			 * Note: this is very nasty.  It's not
			 * clear what the right thing to do
			 * with a partial message is; 
			 * We throw it out
			 */
			if (tp->t_message != NULL) {
				freemsg(tp->t_message);
				tp->t_message = NULL;
				tp->t_endmsg = NULL;
				tp->t_msglen = 0;
				tp->t_rocount = 0;
				tp->t_rocol = 0;
				if (tp->t_state & TS_MEUC) {
					ASSERT(tp->t_eucp_mp);
					tp->t_eucp = tp->t_eucp_mp->b_rptr;
					tp->t_codeset = 0;
					tp->t_eucleft = 0;
				}
			}
			break;

		case MC_DO_CANON:
			tp->t_state &= ~TS_NOCANON;
			break;
		default:
			DEBUG3 (("Unknown M_CTL Message\n"));
			break;
		}
		putnext(q, mp);	/* In case anyone else has to see it */
		return;
		
	case M_DATA:
		break;
	}
	drv_setparm(SYSRAWC, msgdsize(mp));

	/*
	 * Flow control: send "start input" message if blocked and
	 * our queue is below its low water mark.
	 */
	if ((tp->t_modes.c_iflag & IXOFF) && (tp->t_state & TS_TBLOCK)
	    && q->q_count <= TTXOLO) {
		tp->t_state &= ~TS_TBLOCK;
		(void) putctl(wrq->q_next, M_STARTI);
		DEBUG1 (("M_STARTI down\n"));
	}

	/*
	 * If somebody below us ("intelligent" communications board,
	 * pseudo-tty controlled by an editor) is doing
	 * canonicalization, don't scan it for special characters.
	 */
	if (tp->t_state & TS_NOCANON) {
		putq(q, mp);
		return;
	}

	bp = mp;

	do {
		readp = bp->b_rptr;
		writep = readp;
		/**tk_nin += bp->b_wptr - readp; **/
		if (tp->t_modes.c_iflag & (INLCR|IGNCR|ICRNL|IUCLC|IXON)
		    || tp->t_modes.c_lflag & (ISIG|ICANON)) {
			/*
			 * We're doing some sort of non-trivial processing
			 * of input; look at every character.
			 */
			while (readp < bp->b_wptr) {
				c = *readp++;

				if (tp->t_modes.c_iflag & ISTRIP)
					c &= 0177;

				/*
				 * First, check that this hasn't been escaped
				 * with the "literal next" character.
				 */
				if (tp->t_state & TS_PLNCH) {
					tp->t_state &= ~TS_PLNCH;
					tp->t_modes.c_lflag &= ~FLUSHO;
					*writep++ = c;
					continue;
				}

				/*
				 * Setting a special character to NUL disables
				 * it, so if this character is NUL, it should
				 * not be compared with any of the special
				 * characters.  It should, however, restart
				 * frozen output if IXON and IXANY are set.
				 */
				if (c == '\0') {
					if (tp->t_modes.c_iflag & IXON
					    && tp->t_state & TS_TTSTOP
					    && tp->t_modes.c_iflag & IXANY) {
						tp->t_state  &= ~TS_TTSTOP;
						(void) putctl(wrq->q_next,
						    M_START);
					}
					tp->t_modes.c_lflag &= ~FLUSHO;
					*writep++ = c;
					continue;
				}

				/*
				 * If stopped, start if you can; if running,
				 * stop if you must.
				 */
				if (tp->t_modes.c_iflag & IXON) {
					if (tp->t_state & TS_TTSTOP) {
						if (c == tp->t_modes.c_cc[VSTART]
						    || tp->t_modes.c_iflag & IXANY) {
							tp->t_state &= ~TS_TTSTOP;
							(void) putctl(wrq->q_next, M_START);
						}
					} else {
						if (c == tp->t_modes.c_cc[VSTOP]) {
							tp->t_state |= TS_TTSTOP;
							(void) putctl(wrq->q_next, M_STOP);
						}
					}
					if (c == tp->t_modes.c_cc[VSTOP]
					    || c == tp->t_modes.c_cc[VSTART])
						continue;
				}
					/*
					 * Check for "literal next" character
					 * and "flush output" character.
					 */
					if (tp->t_modes.c_lflag & (ISIG|ICANON)) {
						if ((tp->t_modes.c_lflag & IEXTEN) && c == tp->t_modes.c_cc[VLNEXT]) {
							/*
							 * Remember that we saw
							 * a "literal next"
							 * while scanning
							 * input, but leave it
							 * in the message so
							 * that the service
							 * routine can see it
							 * too.
							 */
							tp->t_state |= TS_PLNCH;
							tp->t_modes.c_lflag &= ~FLUSHO;
							*writep++ = c;
							continue;
						}
						if ((tp->t_modes.c_lflag & IEXTEN) && c == tp->t_modes.c_cc[VDISCARD]) {
							ldterm_flush_output(c, wrq, tp);
							continue;
						}
					}

				tp->t_modes.c_lflag &= ~FLUSHO;

				/*
				 * Check for signal-generating characters.
				 */
				if (tp->t_modes.c_lflag & ISIG) {
					if (c == tp->t_modes.c_cc[VINTR]) {
						ldterm_dosig(q, SIGINT, c, M_PCSIG, FLUSHRW);
						continue;
					}
					if (c == tp->t_modes.c_cc[VQUIT]) {
						ldterm_dosig(q, SIGQUIT, c, M_PCSIG, FLUSHRW);
						continue;
					}
					if (c == tp->t_modes.c_cc[VSWTCH]) {
						
						/* User typed Cntrl-Z - send an
						
M_CTL message to SXT driver */
						/* Cntrl-Z for SXT is checked first and then for job control */
						if ((swb = allocb ( sizeof( struct iocblk), BPRI_HI)) != NULL) {
							swb->b_wptr = swb->b_rptr + sizeof (struct iocblk);
							swb->b_datap->db_type = M_CTL;
							swiocp = (struct iocblk *) swb->b_rptr;
							swiocp->ioc_count = 0;
							swiocp->ioc_error = 0;
							swiocp->ioc_rval = 0;
							swiocp->ioc_cmd = SXTSWTCH;
							putnext (q, swb);
						}
						continue;
					}
					if (c == tp->t_modes.c_cc[VSUSP]) {
						ldterm_dosig(q, SIGTSTP, c, M_PCSIG, FLUSHR);
						continue;
					}
					if (c == tp->t_modes.c_cc[VDSUSP]) {
						ldterm_dosig(q, SIGTSTP, c, M_SIG, 0);
						continue;
					}
				}

				/*
				 * Throw away CR if IGNCR set, or turn
				 * it into NL if ICRNL set.
				 */
				if (c == '\r') {
					if (tp->t_modes.c_iflag & IGNCR)
						continue;
					if (tp->t_modes.c_iflag & ICRNL)
						c = '\n';
				} else {
					/*
					 * Turn NL into CR if INLCR set.
					 */
					if (c == '\n'
					    && tp->t_modes.c_iflag & INLCR)
						c = '\r';
				}

				/*
				 * Map upper case input to lower case if
				 * IUCLC flag set.
				 */
				if (tp->t_modes.c_iflag & IUCLC
				    && c >= 'A' && c <= 'Z')
					c += 'a' - 'A';

				/*
				 * Put the possibly-transformed character
				 * back in the message.
				 */
				*writep++ = c;
			}

			/*
			 * If we didn't copy some characters because
			 * we were ignoring them, fix the size of the
			 * data block by adjusting the write pointer.
			 * XXX This may result in a zero-length block;
			 * will this cause anybody gastric distress?
			 */
			bp->b_wptr -= (readp - writep);
		} else {
			/*
			 * We won't be doing anything other than possibly
			 * stripping the input.
			 */
			if (tp->t_modes.c_iflag & ISTRIP) {
				while (readp < bp->b_wptr)
					*writep++ = *readp++ & 0177;
			}
			tp->t_modes.c_lflag &= ~FLUSHO;
		}

	} while ((bp = bp->b_cont) != NULL);	/* next block, if any */

	/*
	 * Queue the message for service procedure.
	 */

	putq(q, mp);

	/*
	 * Flow control: send "stop input" message if our queue is
	 * approaching its high-water mark. The message will be dropped
	 * on the floor in the service procedure, if we cannot ship it
	 * up and we have had it upto our neck!
	 * 
	 * Set QWANTW to ensure that the read queue service
	 * procedure gets run when nextq empties up again, so that
	 * it can unstop the input.
	 */
	if ((tp->t_modes.c_iflag & IXOFF) && !(tp->t_state & TS_TBLOCK)
		    && q->q_count >= TTXOHI) {
		nextq->q_flag |= QWANTW;
		tp->t_state |= TS_TBLOCK;
		(void) putctl(wrq->q_next, M_STOPI);
DEBUG1 (("M_STOPI down\n"));
	}
}

/*
 * Line discipline input server processing.  Erase/kill and escape ('\')
 * processing, gathering into messages, upper/lower case input mapping.
 */
ldtermrsrv(q)
	register queue_t *q;
{
	register ldtermstd_state_t *tp;
	mblk_t *mp;
	register mblk_t *bp;
	register mblk_t *bpt;
	mblk_t *bcont;
	register unsigned char c;
	int ebsize, sx;

	tp = (ldtermstd_state_t *)q->q_ptr;

	if (tp->t_state & TS_RESCAN) {
		/*
		 * Canonicalization was turned on or off.
		 * Put the message being assembled back in the input queue,
		 * so that we rescan it.
		 */
		if (tp->t_message != NULL) {
			DEBUG5 (("RESCAN WAS SET; put back in q\n"));
			putbq(q, tp->t_message);
			tp->t_message = NULL;
			tp->t_endmsg = NULL;
			tp->t_msglen = 0;
		}
		tp->t_state &= ~TS_RESCAN;
	}

	bpt = NULL;

	while ((mp = getq(q)) != NULL) {
		if (mp->b_datap->db_type <= QPCTL && !canput(q->q_next)) {
		/* 
		 * Stream head is flow controlled. If echo is turned on,
		 * flush the read side or send a bell down the line
		 * to stop input and process the current message.
	 	 * Otherwise(putbq) the user will not see any response to
		 * to the typed input. Typically happens if there is no
		 * reader process. Note that you will loose the data
		 * in this case if the data is coming too fast. There
		 * is an assumption here that if ECHO is turned on its
		 * some user typing the data on a terminal and its not network.
		 */
		 
			if (tp->t_modes.c_lflag & ECHO) {
				if (tp->t_modes.c_iflag & IMAXBEL) {
					ldterm_outchar(CTRL('g'), WR(q), 4, tp);
				}
				else {
					(void) putctl1(q, M_FLUSH, FLUSHR);
				}
			}
			else {
				putbq(q, mp);
				goto out;	/* read side is blocked */
			}
		}
		switch (mp->b_datap->db_type) {

		default:
			putnext(q, mp);	/* pass it on */
			continue;

		case M_FLUSH:
			/*
			 * Flush everything we haven't looked at yet.
			 */
			flushq(q, FLUSHDATA);

			/*
			 * Flush everything we have looked at.
			 */
			freemsg(tp->t_message);
			tp->t_message = NULL;
			tp->t_endmsg = NULL;
			tp->t_msglen = 0;
			tp->t_rocount = 0;
			tp->t_rocol = 0;
			if (tp->t_state & TS_MEUC) {	/* EUC multi-byte */
				ASSERT(tp->t_eucp_mp);
				tp->t_eucp = tp->t_eucp_mp->b_rptr;
			}
			putnext(q, mp);		/* pass it on */
			continue;

		case M_HANGUP:
			/*
			 * Flush everything we haven't looked at yet.
			 */
			flushq(q, FLUSHDATA);

			/*
			 * Flush everything we have looked at.
			 */
			freemsg(tp->t_message);
			tp->t_message = NULL;
			tp->t_endmsg = NULL;
			tp->t_msglen = 0;
		/* should we set read request tp->t_rd_request to NULL? **/
			tp->t_rocount = 0;	/* if it hasn't been typed, */
			tp->t_rocol = 0;	/* it hasn't been echoed :-) */
			if (tp->t_state & TS_MEUC) {
				ASSERT(tp->t_eucp_mp);
				tp->t_eucp = tp->t_eucp_mp->b_rptr;
			}
			/*
			 * Restart output, since it's probably got 
			 * nowhere to go anyway, and we're probably not
			 * going to see another ^Q for a while.
			 */
			if (tp->t_state & TS_TTSTOP) {
				tp->t_state &= ~TS_TTSTOP;
				(void) putctl(WR(q)->q_next, M_START);
			}

			/*
			 * This message will travel up the read queue, flushing
			 * as it goes, get turned around at the stream head,
			 * and travel back down the write queue, flushing as
			 * it goes.
			 */
			(void) putctl1(q->q_next, M_FLUSH, FLUSHW);

			/*
			 * This message will travel down the write queue, flushing
			 * as it goes, get turned around at the driver,
			 * and travel back up the read queue, flushing as
			 * it goes.
			 */
			(void) putctl1(WR(q), M_FLUSH, FLUSHR);

			/*
			 * Now that that's done, we send a SIGCONT upstream,
			 * followed by the M_HANGUP.
			 */
			/** (void) putctl1(q->q_next, M_PCSIG, SIGCONT); **/
			(void) putnext(q, mp);
			continue;

		case M_IOCACK:

			/*
			 * Augment whatever information the driver is
			 * returning  with the information we supply.
			 */
			ldterm_ioctl_reply(q, mp);
			continue;

		case M_DATA:
			break;
		}

		/*
		 * This is an M_DATA message.
		 */

		/* do XENIX channel mapping first, if set */
		if (LDTERM_CHANMAP(tp)) {
			mblk_t *bp, *cmp, *omp;

			bp = mp;
			omp = (mblk_t *) NULL;
			while (bp) {
			   cmp = unlinkb(bp);
			   ebsize = bp->b_wptr - bp->b_rptr;

			   if (ebsize != 0) {
				ebsize = str_emmapin(bp,&tp->t_emap);
				if (tp->t_emap.t_merr && (tp->t_modes.c_lflag&ECHO)) {
					if (ebsize == 0) ebsize = 1;
					ldterm_outchar(CTRL('g'),WR(q),ebsize,tp);
				}
			   }
			   if (omp == NULL)
				omp = bp;
			   else {
				omp->b_cont = bp;
				omp = bp;
			   }
			   bp = cmp;
			}
		}

		/*
		 * If somebody below us ("intelligent" communications board,
		 * pseudo-tty controlled by an editor) is doing
		 * canonicalization, don't scan it for special characters.
		 */
		if (tp->t_state & TS_NOCANON) {
			putnext(q, mp);
			continue;
		}

		bp = mp;

		ebsize = bp->b_wptr - bp->b_rptr;
		if (ebsize > EBSIZE)
			ebsize = EBSIZE;
		/*
	 	 * ldterm_vmin_timeout may interrupt this code
		 * and  set t_endmsg to zero during noncanonical 
		 * processing.
		 */

		sx = splstr();
		if ((bpt = newmsg(tp)) != NULL) {
			do {
				bcont = bp->b_cont;
				if (CANON_MODE) {
					splx(sx);
					/* 
					 * update sysinfo canch character. 
					 * The value of canch may vary
					 * as compared to character tty
					 * implementation.  
				         */

					while (bp->b_rptr < bp->b_wptr) {
						c = *bp->b_rptr++;
						if ((bpt = ldterm_docanon(c, bpt,
						    ebsize, q, tp)) == NULL)
							break;
					}
					/*
					 * Release this block.
					 */
					freeb(bp);
				} else
					bpt = ldterm_dononcanon(bp, bpt, ebsize,
					    q, tp);
				if (bpt == NULL) {
					cmn_err(CE_WARN, "ldtermrsrv: out of blocks\n");
					freemsg(bcont);
					break;
				}
			} while ((bp = bcont) != NULL);
		}
		splx(sx);

		/*
		 * Send whatever we echoed downstream.
		 */
		if (tp->t_echomp != NULL) {
			putnext(WR(q), tp->t_echomp);
			tp->t_echomp = NULL;
		}
	}

out:
	/*
	 * Flow control: send start message if blocked and
	 * our queue is below its low water mark.
	 */
	if ((tp->t_modes.c_iflag & IXOFF) && (tp->t_state & TS_TBLOCK)
	    && q->q_count <= TTXOLO) {
		tp->t_state &= ~TS_TBLOCK;
		(void) putctl(WR(q), M_STARTI);
	}
}

mblk_t *
ldterm_docanon(c, bpt, ebsize, q, tp)
	register unsigned char c;
	register mblk_t *bpt;
	int ebsize;
	queue_t *q;
	register ldtermstd_state_t *tp;
{
	register queue_t *wrq = WR(q);
	int i;

	/*
	 * If the previous character was the "literal next" character, treat
	 * this character as regular input.
	 */
	if (tp->t_state & TS_SLNCH)
		goto escaped;

	/*
	 * Setting a special character to NUL disables it, so if this
	 * character is NUL, it should not be compared with any of the
	 * special characters.
	 */
	if (c == '\0') {
		tp->t_state &= ~TS_QUOT;
		goto escaped;
	}

	/*
	 * If this character is the literal next character, echo it as '^',
	 * backspace over it, and record that fact.
	 */
	if ((tp->t_modes.c_lflag & IEXTEN) && c == tp->t_modes.c_cc[VLNEXT]) {
		if (tp->t_modes.c_lflag & ECHO)
			ldterm_outstring((unsigned char *)"^\b", 2, wrq, ebsize,
			    tp);
		tp->t_state |= TS_SLNCH;
		goto out;
	}

	/*
	 * Check for the editing characters.  EUC: we can't use "codeset"
	 * because that may change around on us.  Just look at the value
	 * of the end byte in the canonical buffer (it's in t_endmsg->wptr-1).
	 * That provides a clue to what we're doing; if that's got the
	 * high bit set, then we're in business - do an EUC character erase.
	 */
	if (c == tp->t_modes.c_cc[VERASE]) {
		if (tp->t_state & TS_QUOT) {
			/*
			 * Get rid of the backslash, and put the erase
			 * character in its place.
			 */
			ldterm_erase(wrq, ebsize, tp);
			bpt = tp->t_endmsg;
			goto escaped;
		} else {
			if ((tp->t_state & TS_MEUC) &&
			     NOTASCII(*(tp->t_endmsg->b_wptr - 1)))
				ldterm_euc_erase(wrq, ebsize, tp);
			else
				ldterm_erase(wrq, ebsize, tp);
			bpt = tp->t_endmsg;
			goto out;
		}
	}

	if ((tp->t_modes.c_lflag & IEXTEN) && c == tp->t_modes.c_cc[VWERASE]) {
		/*
		 * Do "ASCII word" or "EUC token" erase.
		 */
		if (tp->t_state & TS_MEUC)
			ldterm_tokerase(wrq, ebsize, tp);
		else
			ldterm_werase(wrq, ebsize, tp);
		bpt = tp->t_endmsg;
		goto out;
	}

	if (c == tp->t_modes.c_cc[VKILL]) {
		if (tp->t_state & TS_QUOT) {
			/*
			 * Get rid of the backslash, and put the kill character
			 * in its place.
			 */
			ldterm_erase(wrq, ebsize, tp);
			bpt = tp->t_endmsg;
			goto escaped;
		} else {
			ldterm_kill(wrq, ebsize, tp);
			bpt = tp->t_endmsg;
			goto out;
		}
	}

	if ((tp->t_modes.c_lflag & IEXTEN) && c == tp->t_modes.c_cc[VREPRINT]) {
		ldterm_reprint(wrq, ebsize, tp);
		goto out;
	}

	/*
	 * If the preceding character was a backslash:
	 *     if the current character is an EOF, get rid of the backslash
	 *     and treat the EOF as data;
	 *     if we're in XCASE mode and the current character is part
	 *     of a backslash-X escape sequence, process it;
	 *     otherwise, just treat the current character normally.
	 */
	if (tp->t_state & TS_QUOT) {
		tp->t_state &= ~TS_QUOT;
		if (c == tp->t_modes.c_cc[VEOF]) {
			/*
			 * EOF character.
			 * Since it's escaped, get rid of the backslash and put
			 * the EOF character in its place.
			 */
			ldterm_erase(wrq, ebsize, tp);
			bpt = tp->t_endmsg;
		} else {
			/*
			 * If we're in XCASE mode, and the current character
			 * is part of a backslash-X sequence, get rid of the
			 * backslash and replace the current character with
			 * what that sequence maps to.
			 */
			if ((tp->t_modes.c_lflag & XCASE)
			    && imaptab[c] != '\0') {
				ldterm_erase(wrq, ebsize, tp);
				bpt = tp->t_endmsg;
				c = imaptab[c];
			}
		}
	} else {
		/*
		 * Previous character wasn't backslash; check whether this
		 * was the EOF character.
		 */
		if (c == tp->t_modes.c_cc[VEOF]) {
			/*
			 * EOF character.
			 * Don't echo it unless ECHOCTL is set, don't stuff it
			 * in the current line, but send the line up the
			 * stream.
			 */
			if ((tp->t_modes.c_lflag & ECHOCTL)
			    && (tp->t_modes.c_lflag & IEXTEN)
			    && (tp->t_modes.c_lflag & ECHO)) {
				i = ldterm_echo(c, wrq, ebsize, tp);
				while (i > 0) {
					ldterm_outchar('\b', wrq, ebsize, tp);
					i--;
				}
			}
			bpt->b_datap->db_type = M_DATA;
			ldterm_msg_upstream(q, tp);
			bpt = newmsg(tp);
			goto out;
		}
	}

escaped:
	/*
	 * First, make sure we can fit one WHOLE EUC char in the buffer.  This is
	 * one place where we have overhead even if not in multi-byte mode; the
	 * overhead is subtracting tp->t_maxeuc from MAX_CANON before checking.
	 *
	 * Allows 256 bytes in the buffer before throwing awaying the
         * the overflow of characters.
	 */
	if (tp->t_msglen > ((MAX_CANON + 1) - (int)tp->t_maxeuc)) {
		/*
		 * Byte will cause line to overflow, or the next EUC
		 * won't fit:
		 * Ring the bell or discard all input, and don't save the
		 * byte away.
		 */
		if (tp->t_modes.c_iflag & IMAXBEL) {
			ldterm_outchar(CTRL('g'), wrq, ebsize, tp);
			goto out;
		} else {
			/* MAX_CANON processing.
			 * free everything in the current line and 
			 * start with the current character as the
			 * first character.
			 */
			DEBUG7(("ldterm_docanon: MAX_CANON processing\n"));
			freemsg(tp->t_message);
			tp->t_message = NULL;
			tp->t_endmsg = NULL;
			tp->t_msglen = 0;
			tp->t_rocount = 0;	/* if it hasn't been typed, */
			tp->t_rocol = 0;	/* it hasn't been echoed :-) */
			if (tp->t_state & TS_MEUC) {
				ASSERT(tp->t_eucp_mp);
				tp->t_eucp = tp->t_eucp_mp->b_rptr;
			}
			tp->t_state &= ~TS_SLNCH;
			bpt = newmsg(tp);
		}
	}

	/*
	 * Add the character to the current line.
	 */
	if (bpt->b_wptr >= bpt->b_datap->db_lim) {
		/*
		 * No more room in this mblk; save this one away, and
		 * allocate a new one.
		 */
		bpt->b_datap->db_type = M_DATA;
		if ((bpt = allocb(IBSIZE, BPRI_MED)) == NULL)
			goto out;

		/*
		 * Chain the new one to the end of the old one, and
		 * mark it as the last block in the current line.
		 */
		tp->t_endmsg->b_cont = bpt;
		tp->t_endmsg = bpt;
	}
	*bpt->b_wptr++ = c;
	tp->t_msglen++; /* message length in BYTES */

	/*
	 * In multi-byte mode, we have to keep track of where we are.
	 * The first bytes of EUC chars get the full count for the
	 * whole character.  We don't do any column calculations here,
	 * but we need the information for when we do.
	 * We could come across cases where we are getting garbage on the
	 * line, but we're in multi-byte mode.  In that case, we may see
	 * ASCII come in the middle of what should have been an EUC char-
	 * acter.  Call ldterm_eucwarn...eventually, a warning message will
	 * be printed about it.
	 */
	if (tp->t_state & TS_MEUC) {
		if (tp->t_eucleft) {	/* if in a multi-byte char already */
			--tp->t_eucleft;
			*tp->t_eucp++ = 0;	/* is a subsequent byte */
			if (ISASCII(c))
				ldterm_eucwarn(tp);
		}
		else {	/* is the first byte of an EUC, or is ASCII */
			if (ISASCII(c)) {
				*tp->t_eucp++ = ldterm_dispwidth(c, &tp->eucwioc, tp->t_modes.c_lflag & ECHOCTL);
				tp->t_codeset = 0;
			}
			else {
				*tp->t_eucp = ldterm_dispwidth(c, &tp->eucwioc, tp->t_modes.c_lflag & ECHOCTL);
				tp->t_eucleft = ldterm_memwidth(c, &tp->eucwioc) - 1;
				++(tp->t_eucp);
				tp->t_codeset = ldterm_codeset(c, &tp->eucwioc);
			}
		}
	}

/* 
 * EOL2/XCASE should be conditioned with IEXTEN to be truly POSIX conformant.
 * This is going to cause problems for pre-SVR4.0 programs that don't
 * know about IEXTEN. Hence EOL2/IEXTEN is not conditioned with IEXTEN.
 */
	if (!(tp->t_state & TS_SLNCH)
	    && (c == '\n' || (c != '\0' && (c == tp->t_modes.c_cc[VEOL]
	   || (c == tp->t_modes.c_cc[VEOL2]))))) {
	   /* || ((tp->t_modes.c_lflag & IEXTEN) && c == tp->t_modes.c_cc[VEOL2]))))) { */
		/*
		 * It's a line-termination character; send the line
		 * up the stream.
		 */
		bpt->b_datap->db_type = M_DATA;
		ldterm_msg_upstream(q, tp);
		if (tp->t_state & TS_MEUC) {
			ASSERT(tp->t_eucp_mp);
			tp->t_eucp = tp->t_eucp_mp->b_rptr;
		}
		if ((bpt = newmsg(tp)) == NULL)
			goto out;
	} else {
		/*
		 * Character was escaped with LNEXT.
		 */
		if (tp->t_rocount++ == 0)
			tp->t_rocol = tp->t_col;
		tp->t_state &= ~(TS_SLNCH|TS_QUOT);
		if (c == '\\')
			tp->t_state |= TS_QUOT;
	}

	/*
	 * Echo it.
	 */
	if (tp->t_state & TS_ERASE) {
		tp->t_state &= ~TS_ERASE;
		if (tp->t_modes.c_lflag & ECHO)
			ldterm_outchar('/', wrq, ebsize, tp);
	}

	if (tp->t_modes.c_lflag & ECHO)
		(void) ldterm_echo(c, wrq, ebsize, tp);
	else {
		/*
		 * Echo NL when ECHO turned off, if ECHONL flag is set.
		 */
		if (c == '\n' && (tp->t_modes.c_lflag & ECHONL))
			ldterm_outchar(c, wrq, ebsize, tp);
	}

out:

	return (bpt);
}

ldterm_unget(tp)
	register ldtermstd_state_t *tp;
{
	register mblk_t *bpt;

	if ((bpt = tp->t_endmsg) == NULL)
		return(-1);	/* no buffers */
	if (bpt->b_rptr == bpt->b_wptr)
		return(-1);	/* zero-length record */
	tp->t_msglen--;	/* one fewer character */
	return(*--bpt->b_wptr);
}

void
ldterm_trim(tp)
	register ldtermstd_state_t *tp;
{
	register mblk_t *bpt;
	register mblk_t *bp;

	ASSERT(tp->t_endmsg);
	bpt = tp->t_endmsg;

	if (bpt->b_rptr == bpt->b_wptr) {
		/*
		 * This mblk is now empty.
		 * Find the previous mblk; throw this one away, unless
		 * it's the first one.
		 */
		bp = tp->t_message;
		if (bp != bpt) {
			while (bp->b_cont != bpt) {
				ASSERT(bp->b_cont);
				bp = bp->b_cont;
			}
			bp->b_cont = NULL;
			freeb(bpt);
			tp->t_endmsg = bp;	/* point to that mblk */
		}
	}
}


/*
 * Rubout one character from the current line being built for tp
 * as cleanly as possible.  q is the write queue for tp.
 * Most of this can't be applied to multi-byte processing.  We do our
 * own thing for that... See the "ldterm_eucerase" routine.  We never
 * call ldterm_rubout on a multi-byte or multi-column character.
 */
void
ldterm_rubout(c, q, ebsize, tp)
	register unsigned char c;
	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register int tabcols;
	static unsigned char crtrubout[] = "\b \b\b \b";
#define	RUBOUT1	&crtrubout[3]	/* rub out one position */
#define	RUBOUT2	&crtrubout[0]	/* rub out two positions */

	if (!(tp->t_modes.c_lflag & ECHO))
		return;
	if (tp->t_modes.c_lflag & ECHOE) {
		/*
		 * "CRT rubout"; try erasing it from the screen.
		 */
		if (tp->t_rocount == 0) {
			/*
			 * After the character being erased was echoed,
			 * some data was written to the terminal; we
			 * can't erase it cleanly, so we just reprint the
			 * whole line as if the user had typed the
			 * reprint character.
			 */
			ldterm_reprint(q, ebsize, tp);
			return;
		} else {
			/*
			 * XXX what about escaped characters?
			 */
			switch (typetab[c]) {

			case ORDINARY:
				if ((tp->t_modes.c_lflag & XCASE)
				    && omaptab[c])
					ldterm_outstring(RUBOUT1, 3, q, ebsize,
					    tp);
				ldterm_outstring(RUBOUT1, 3, q, ebsize, tp);
				break;

			case VTAB:
			case BACKSPACE:
			case CONTROL:
			case RETURN:
			case NEWLINE:
				if ((tp->t_modes.c_lflag & ECHOCTL)
				    && (tp->t_modes.c_lflag & IEXTEN))
					ldterm_outstring(RUBOUT2, 6, q, ebsize,
					    tp);
				break;

			case TAB:
				if (tp->t_rocount < tp->t_msglen) {
					/*
					 * While the tab being erased was
					 * expanded, some data was written
					 * to the terminal; we can't erase it
					 * cleanly, so we just reprint the
					 * whole line as if the user had typed
					 * the reprint character.
					 */
					ldterm_reprint(q, ebsize, tp);
					return;
				}
				tabcols = ldterm_tabcols(tp);
				while (--tabcols >= 0)
					ldterm_outchar('\b', q, ebsize, tp);
				break;
			}
		}
	} else if ((tp->t_modes.c_lflag & ECHOPRT)
		    && (tp->t_modes.c_lflag & IEXTEN)) {
		/*
		 * "Printing rubout"; echo it between \ and /.
		 */
		if (!(tp->t_state & TS_ERASE)) {
			ldterm_outchar('\\', q, ebsize, tp);
			tp->t_state |= TS_ERASE;
		}
		(void) ldterm_echo(c, q, ebsize, tp);
	} else
		(void) ldterm_echo(tp->t_modes.c_cc[VERASE], q, ebsize, tp);
	tp->t_rocount--;	/* we "unechoed" this character */
}

/*
 * Find the number of characters the tab we just deleted took up by
 * zipping through the current line and recomputing the column number.
 */
ldterm_tabcols(tp)
	register ldtermstd_state_t *tp;
{
	register int col;
	register mblk_t *bp;
	register unsigned char *readp, *endp;
	register unsigned char c;

	col = tp->t_rocol;
	/*
	 * If we're doing multi-byte stuff, zip through the list of
	 * widths to figure out where we are (we've kept track).
	 */
	if (tp->t_state & TS_MEUC) {
		ASSERT(tp->t_eucp_mp);
		readp = tp->t_eucp_mp->b_rptr;
		endp = tp->t_eucp;
		while (readp < endp) {
			switch (*readp) {
			case EUC_TWIDTH:	/* it's a tab */
				col |= 07;	/* bump up */
				col++;
				break;
			case EUC_BSWIDTH:	/* backspace */
				if (col)
					col--;
				break;
			case EUC_NLWIDTH:	/* newline */
				if ((tp->t_modes.c_lflag & ECHOCTL)
				    && (tp->t_modes.c_lflag & IEXTEN))
					col += 2;
				else if (tp->t_modes.c_oflag & ONLRET)
					col = 0;
				break;
			case EUC_CRWIDTH:	/* return */
				col = 0;
				break;
			default:
				col += *readp;
			}
			++readp;
		}
		goto eucout;	/* finished! */
	}
	bp = tp->t_message;
	do {
		readp = bp->b_rptr;
		while (readp < bp->b_wptr) {
			c = *readp++;
			if ((tp->t_modes.c_lflag & ECHOCTL)
			    && (tp->t_modes.c_lflag & IEXTEN)) {
				if (c <= 037 && c != '\t' && c != '\n'
				    || c == 0177) {
/* XXX is this right?  shouldn't ctl chars take 2 columns? */
					col++;
					continue;
				}
			}

			/*
			 * Column position calculated here.
			 */
			switch (typetab[c]) {

			/* Ordinary characters; advance by one. */
			case ORDINARY:
				col++;
				break;

			/* Non-printing characters; nothing happens. */
			case CONTROL:
				break;

			/* Backspace */
			case BACKSPACE:
				if (col != 0)
					col--;
				break;

			/* Newline; column depends on flags. */
			case NEWLINE:
				if (tp->t_modes.c_oflag & ONLRET)
					col = 0;
				break;

			/* tab */
			case TAB:
				col |= 07;
				col++;
				break;

			/* vertical motion */
			case VTAB:
				break;

			/* carriage return */
			case RETURN:
				col = 0;
				break;
			}
		}
	} while ((bp = bp->b_cont) != NULL);	/* next block, if any */

	/*
	 * "col" is now the column number before the tab.
	 * "tp->t_col" is still the column number after the tab,
	 * since we haven't erased the tab yet.
	 * Thus "tp->t_col - col" is the number of positions the tab
	 * moved.
	 */
eucout:
	col = tp->t_col - col;
	if (col > 8)
		col = 8;		/* overflow screw */
	return (col);
}

/*
 * Erase a single character; We ONLY ONLY deal with ASCII or single-column
 * single-byte EUC.  For multi-byte characters, see "ldterm_euc_erase".
 */
void
ldterm_erase(q, ebsize, tp)
	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register int c;

	if ((c = ldterm_unget(tp)) != -1) {
		ldterm_rubout((unsigned char)c, q, ebsize, tp);
		ldterm_trim(tp);
		if (tp->t_state & TS_MEUC)
			--tp->t_eucp;
	}
}

/*
 * Erase an entire word, single-byte EUC only please.
 */
void
ldterm_werase(q, ebsize, tp)
	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register int c;

	/*
	 * Erase trailing white space, if any.
	 */
	while ((c = ldterm_unget(tp)) == ' ' || c == '\t') {
		ldterm_rubout((unsigned char)c, q, ebsize, tp);
		ldterm_trim(tp);
	}

	/*
	 * Erase non-white-space characters, if any.
	 */
	while (c != -1 && c != ' ' && c != '\t') {
		ldterm_rubout((unsigned char)c, q, ebsize, tp);
		ldterm_trim(tp);
		c = ldterm_unget(tp);
	}
	if (c != -1) {
		/*
		 * We removed one too many characters; put the last one
		 * back.
		 */
		tp->t_endmsg->b_wptr++;	/* put 'c' back */
		tp->t_msglen++;
	}
}
/*
 * ldterm_tokerase - This is EUC equivalent of "word erase".  "Word erase"
 * only makes sense in languages which space between words, and it's
 * presumptuous for us to attempt "word erase" when we don't know anything
 * about what's really going on.  It makes no sense for many languages, as
 * the criteria for defining words and tokens may be completely different.
 * 
 * In the TS_MEUC case (which is how we got here), we define a token to be
 * space- or tab-delimited, and erase one of them.  It helps to have this
 * for command lines, but it's otherwise useless for text editing
 * applications; you need more sophistication than we can provide here.
 */

ldterm_tokerase(q, ebsize, tp)

	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register int c, i;
	register unchar *ip;

	/*
	 * ip points to the width of the actual bytes.  t_eucp points
	 * one byte beyond, where the next thing will be inserted.
	 */
	ip = tp->t_eucp - 1;
	/*
	 * Erase trailing white space, if any.
	 */
	while ((c = ldterm_unget(tp)) == ' ' || c == '\t') {
		ldterm_rubout((unsigned char)c, q, ebsize, tp);
		ldterm_trim(tp);
		tp->t_eucp--;
		--ip;
	}

	/*
	 * Erase non-white-space characters, if any.  The outer loop
	 * bops through each byte in the buffer.  EUC is removed, as is
	 * ASCII, one byte at a time. The inner loop (for) is only executed
	 * for first bytes of EUC.  The inner loop erases the number of
	 * columns required for the EUC char.  We check for ASCII first, and
	 * ldterm_rubout knows about ASCII.  We DON'T check for special values
	 * such as EUC_TWIDTH and friends.
	 */
	while (c != -1 && c != ' ' && c != '\t') {
		if (ISASCII(c))
			ldterm_rubout((unsigned char)c, q, ebsize, tp);
		else if (*ip) {
			/*
			 * erase for number of columns required for this EUC
			 * character.  Hopefully, matches ldterm_dispwidth!
			 */
			for (i = 0; i < (int) *ip; i++)
				ldterm_rubout(' ', q, ebsize, tp);
		}
		ldterm_trim(tp);
		tp->t_eucp--;
		--ip;
		c = ldterm_unget(tp);
	}
	if (c != -1) {
		/*
		 * We removed one too many characters; put the last one
		 * back.
		 */
		tp->t_endmsg->b_wptr++;	/* put 'c' back */
		tp->t_msglen++;
	}
}

/*
 * Kill an entire line, erasing each character one-by-one (if ECHOKE
 * is set) or just echoing the kill character, followed by a newline
 * (if ECHOK is set).  Multi-byte processing is included here.
 */

static void
ldterm_kill(q, ebsize, tp)
	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register int c, i;
	register unchar *ip;

	if ((tp->t_modes.c_lflag & ECHOKE)
	    && (tp->t_modes.c_lflag & IEXTEN)
	    && (tp->t_msglen == tp->t_rocount)) {
		if (tp->t_state & TS_MEUC) {
			ip = tp->t_eucp - 1;
			/*
			 * This loop similar to "tokerase" above.
			 */
			while ((c = ldterm_unget(tp)) != (-1)) {
				if (ISASCII(c))
					ldterm_rubout((unsigned char)c, q, ebsize, tp);
				else if (*ip) {
					for (i = 0; i < (int) *ip; i++)
						ldterm_rubout(' ', q, ebsize, tp);
				}
				ldterm_trim(tp);
				tp->t_eucp--;
				--ip;
			}
		}
		else {
			while ((c = ldterm_unget(tp)) != -1) {
				ldterm_rubout((unsigned char)c, q, ebsize, tp);
				ldterm_trim(tp);
			}
		}
	} else {
		(void) ldterm_echo(tp->t_modes.c_cc[VKILL], q, ebsize, tp);
		if (tp->t_modes.c_lflag & ECHOK)
			(void) ldterm_echo('\n', q, ebsize, tp);
		while (ldterm_unget(tp) != -1)
			ldterm_trim(tp);
		tp->t_rocount = 0;
		if (tp->t_state & TS_MEUC)
			tp->t_eucp = tp->t_eucp_mp->b_rptr;
	}
	tp->t_state &= ~(TS_QUOT|TS_ERASE|TS_SLNCH);
}

/*
 * Reprint the current input line.
 * We assume c_cc has already been checked.
 * XXX just the current line, not the whole queue?
 * What about DEFECHO mode?
 */
void
ldterm_reprint(q, ebsize, tp)
	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register mblk_t *bp;
	register unsigned char *readp;

	if (tp->t_modes.c_cc[VREPRINT] != (unsigned char)0)
		(void) ldterm_echo(tp->t_modes.c_cc[VREPRINT], q, ebsize, tp);
	ldterm_outchar('\n', q, ebsize, tp);

	bp = tp->t_message;
	do {
		readp = bp->b_rptr;
		while (readp < bp->b_wptr)
			(void) ldterm_echo(*readp++, q, ebsize, tp);
	} while ((bp = bp->b_cont) != NULL);	/* next block, if any */

	tp->t_state &= ~TS_ERASE;
	tp->t_rocount = tp->t_msglen;	/* we reechoed the entire line */
	tp->t_rocol = 0;
}

/*
 * Non canonical processing.
 * Called at splstr level from  ldtermrsrv.
 *
 */
mblk_t *
ldterm_dononcanon(bp, bpt, ebsize, q, tp)
	register mblk_t *bp;
	register mblk_t *bpt;
	int ebsize;
	queue_t *q;
	register ldtermstd_state_t *tp;
{
	queue_t *wrq = WR(q);
	register unsigned char *rptr;
	register int bytes_in_bp;
	register int roomleft;
	register int bytes_to_move;
	register unsigned char c;
	int free_flag = 0;
	int sx;


DEBUG4 (("ldterm, VMIN = %d, VTIME = %d\n",V_MIN, V_TIME));
	bytes_in_bp = bp->b_wptr - bp->b_rptr;
	rptr = bp->b_rptr;
	while (bytes_in_bp != 0) {
		roomleft = bpt->b_datap->db_lim - bpt->b_wptr;
		if (roomleft == 0) {
			/*
			 * No more room in this mblk; save this one
			 * away, and allocate a new one.
			 */
			if ((bpt = allocb(IBSIZE, BPRI_MED)) == NULL) {
				freeb(bp);
				DEBUG4 (("ldterm_do_noncanon: allcob failed\n"));
				return (bpt);
			}

			/*
			 * Chain the new one to the end of the old
			 * one, and mark it as the last block in the
			 * current lump.
			 */
			tp->t_endmsg->b_cont = bpt;
			tp->t_endmsg = bpt;
			roomleft = IBSIZE;
		}
DEBUG5(("roomleft=%d, bytes_in_bp=%d, tp->t_rd_request=%d\n",roomleft, bytes_in_bp, tp->t_rd_request));
		/* if there is a read pending before this data got here
		 * move bytes according to the minimum of room left in
		 * this buffer, bytes in the message and byte count
		 * requested in the read. If there is no read pending,
		 * move the minimum of the first two 
		 */
		if (tp->t_rd_request == 0) 
			bytes_to_move = MIN(roomleft, bytes_in_bp);
		else
			bytes_to_move = MIN(MIN(roomleft, bytes_in_bp),tp->t_rd_request);
DEBUG5(("Bytes to move = %d\n", bytes_to_move));
		if (bytes_to_move ==0)
			break;
		bcopy((caddr_t)rptr, (caddr_t)bpt->b_wptr,
		    (unsigned int)bytes_to_move);
		bpt->b_wptr += bytes_to_move;
		rptr += bytes_to_move;
		tp->t_msglen += bytes_to_move;
		bytes_in_bp -= bytes_to_move;
	}
	/*
	 * Echo the data in this message.
	 */
	if (tp->t_modes.c_lflag & ECHO) {
		rptr = bp->b_rptr;
		while (rptr < bp->b_wptr)
			(void) ldterm_echo(*rptr++, wrq, ebsize, tp);
	} else {
		if (tp->t_modes.c_lflag & ECHONL) {
			/*
			 * Echo NL, even though ECHO is not set.
			 */
			rptr = bp->b_rptr;
			while (rptr < bp->b_wptr) {
				c = *rptr++;
				if (c == '\n')
					ldterm_outchar(c, wrq, ebsize, tp);
			}
		}
	}

	if (bytes_in_bp == 0) {
		DEBUG4 (("bytes_in_bp is zero\n"));
		freeb(bp);
	} else
		free_flag = 1; /* for debugging olny */

	/* Sending data upstream is dictated by VMIN/VTIME. If vmin
	 * is satisfied, *AND* read request is pending at stream head,
	 * then send data up and cancel any timeouts in progress.
	 * VTIME has to be handled intelligently to eliminate
	 * unsetting callout table entries and software interrupts.
	 * when the first char arrives, if VTIME is set, timer is started.
	 * Subsequent characters only clears RTO flag (does not untimeout).
	 */

	tp->t_state &= ~TS_RTO;
DEBUG4 (("Unsetting TS_RTO, msglen = %d\n", tp->t_msglen));
	if (tp->t_msglen >= V_MIN) {
		DEBUG4 (("VMIN Ready\n"));
		ldterm_msg_upstream (q, tp);
	}

	else if (V_TIME) {
		DEBUG4 (("ldterm_dononcanon VTIME  set\n"));
		if (!(tp->t_state & TS_TACT)) {
			DEBUG4 (("ldterm_dononcanon ldterm_vmin_timeout called\n"));
			ldterm_vmin_timeout (q);
		}
	}

	if (free_flag)
		DEBUG4 (("CAUTION message block not freed\n"));

	return (newmsg(tp));
}


/*
 * Echo a typed byte to the terminal.  Returns the number of bytes printed.
 * Bytes of EUC characters drop through the ECHOCTL stuff and are just
 * output as themselves.
 */
ldterm_echo(c, q, ebsize, tp)
	register unsigned char c;
	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register int i;

	if (!(tp->t_modes.c_lflag & ECHO))
		return (0);
	i = 0;

/*
 * Echo control characters (c <= 37) only if the ECHOCTRL
 * flag is set as ^X. 
 */

	if ((tp->t_modes.c_lflag & ECHOCTL)
	    && (tp->t_modes.c_lflag & IEXTEN)) {
		if (c <= 037 && c != '\t' && c != '\n') {
			ldterm_outchar('^', q, ebsize, tp);
			i++;
			if (tp->t_modes.c_oflag & OLCUC)
				c += 'a' - 1;
			else
				c += 'A' - 1;
		} else if (c == 0177) {
			ldterm_outchar('^', q, ebsize, tp);
			i++;
			c = '?';
		}
		ldterm_outchar(c, q, ebsize, tp);
		return (i + 1);
		/* echo only special control character and the Bell */
	} else {
		 int cset;
		 cset = tp->t_modes.c_cflag & CSIZE;
		 if ((c > 037 && c != 0177) || cset <= CS6 || c == '\t' || c == '\n' 
		  || c == '\r' || c == '\b' || c == 007 || c == tp->t_modes.c_cc[VKILL]) {
			ldterm_outchar(c, q, ebsize, tp);
			return (i + 1);
		 }
	}
}

/*
 * Put a character on the output queue.
 */
void
ldterm_outchar(c, q, bsize, tp)
	register unsigned char c;
	register queue_t *q;
	int bsize;
	register ldtermstd_state_t *tp;
{
	register mblk_t *curbp;

	/*
	 * Don't even look at the characters unless we
	 * have something useful to do with them.
	 */
	if ( LDTERM_CHANMAP(tp) || (tp->t_modes.c_oflag & OPOST)
	    || ((tp->t_modes.c_lflag & XCASE)
	      && (tp->t_modes.c_lflag & ICANON))) {
		register mblk_t *mp;

		if ((mp = allocb(4, BPRI_HI)) == NULL) {
			cmn_err(CE_WARN,"ldterm: (ldterm_outchar) out of blocks\n");
			return;
		}
		*mp->b_wptr++ = c;
		tp->t_echomp = ldterm_output_msg(q, mp, tp->t_echomp, tp, bsize,
		    1);
	} else {
		if ((curbp = tp->t_echomp) != NULL) {
			while (curbp->b_cont != NULL)
				curbp = curbp->b_cont;
			if (curbp->b_datap->db_lim == curbp->b_wptr) {
				register mblk_t *newbp;

				if ((newbp = allocb(bsize, BPRI_HI)) == NULL) {
					cmn_err(CE_WARN,"ldterm: (ldterm_outchar) out of blocks\n");
					return;
				}
				curbp->b_cont = newbp;
				curbp = newbp;
			}
		} else {
			if ((curbp = allocb(bsize, BPRI_HI)) == NULL) {
				cmn_err(CE_WARN,"ldterm: (ldterm_outchar) out of blocks\n");
				return;
			}
			tp->t_echomp = curbp;
		}
		*curbp->b_wptr++ = c;
	}
}

/*
 * Copy a string, of length len, to the output queue.
 */
void
ldterm_outstring(cp, len, q, bsize, tp)
	register unsigned char *cp;
	register int len;
	register queue_t *q;
	int bsize;
	register ldtermstd_state_t *tp;
{

	while (len > 0) {
		ldterm_outchar(*cp++, q, bsize, tp);
		len--;
	}
}

mblk_t *
newmsg(tp)
	register ldtermstd_state_t *tp;
{
	register mblk_t *bp;

	/*
	 * If no current message, allocate a block
	 * for it.
	 */
	if ((bp = tp->t_endmsg) == NULL) {
		if ((bp = allocb(IBSIZE, BPRI_MED)) == NULL) {
			cmn_err(CE_WARN,"ldterm: (ldtermrsrv) out of blocks\n");
			return (bp);
		}
		tp->t_message = bp;
		tp->t_endmsg = bp;
	}
	return (bp);
}

void
ldterm_msg_upstream(q, tp)
	register queue_t *q;
	register ldtermstd_state_t *tp;
{
	int s;
	register mblk_t *bp;
	
	s = splstr();
	bp = tp->t_message;
	if (bp)
		putnext(q, tp->t_message);
	/* 
	 * update sysinfo canch character. 
	 */

	if (CANON_MODE) {
		drv_setparm(SYSCANC, msgdsize(bp));
	}
	tp->t_message = NULL;
	tp->t_endmsg = NULL;
	tp->t_msglen = 0;
	tp->t_rocount = 0;
	tp->t_rd_request = 0;
	if (tp->t_state & TS_MEUC) {
		ASSERT(tp->t_eucp_mp);
		tp->t_eucp = tp->t_eucp_mp->b_rptr;
		/* can't reset everything, as we may have other input */
	}
	splx(s);
}
/*
 * Line discipline output queue put procedure.
 */
ldtermwput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register ldtermstd_state_t *tp;
	mblk_t *bpt;
	int s, runout_flag = 0;

	tp = (ldtermstd_state_t *)q->q_ptr;

	switch (mp->b_datap->db_type) {

	case M_FLUSH:
		/*
		 * This is coming from above, so we only handle the write
		 * queue here.  If FLUSHR is set, it will get turned around
		 * at the driver, and the read procedure will see it
		 * eventually.
		 */
		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);
		putnext(q, mp);
		break;

	case M_IOCDATA:
		ldterm_do_iocdata(q,mp);
		break;

	case M_IOCTL:
		ldterm_do_ioctl(q, mp);
		break;

	case M_READ:
		DEBUG1 (("ldtermwput:M_READ RECEIVED\n"));
		/* Stream head needs data to satisfy timed read.
		 * Has meaning only if ICANON flag is off indicating
		 * raw mode 
		 */

		DEBUG4 (("M_READ: RAW_MODE = %d, COUNT = %d, VMIN = %d, VTIME = %d\n",
			RAW_MODE, *(unsigned int *)mp->b_rptr, V_MIN, V_TIME));

		tp->t_rd_request = *(unsigned int *)mp->b_rptr;

		if (RAW_MODE) {
			s=splstr();

			tp->t_state &= ~TS_RTO;

			if ((bpt = newmsg(tp)) != NULL)  {
				if (V_MIN == 0) {
					if (V_TIME == 0) /* vmin = 0,vtime = 0 */
						ldterm_msg_upstream (RD(q), tp);
					else {	/* vmin = 0, vtime > 0 */
						if (tp->t_msglen)
							ldterm_msg_upstream (RD(q), tp);
						else { /* start timer only is there is not one active */
							if (!(tp->t_state & TS_TACT)) {
								DEBUG4 (("M_READ VTIME  set and timer not active\n"));
								ldterm_vmin_timeout (RD (q));
							}
						}
					}
				} else { 		/* vmin > 0 */
					if (tp->t_msglen >= V_MIN)
						ldterm_msg_upstream (RD(q), tp);
					/* if msglen = 0 and vmin has any value
					 * timeout should be started only after
					 * atleast one char has arrived.
					 */
				}
			} else  /* should do bufcall, really! */
				cmn_err(CE_WARN,"ldterm: (ldtermwput) out of blocks\n");
		(void) splx(s);
		}
		/*
		 * pass M_READ down
	         */
		putnext (q, mp);
		break;

	case M_DATA:
		if ((tp->t_modes.c_lflag & FLUSHO)
		    && (tp->t_modes.c_lflag & IEXTEN)) {
			freemsg(mp);	/* drop on floor */
			break;
		}
		tp->t_rocount = 0;

		/*
		 * Don't even look at the characters unless we
		 * have something useful to do with them.
		 */
		if (LDTERM_CHANMAP(tp) || (tp->t_modes.c_oflag & OPOST)
		    || ((tp->t_modes.c_lflag & XCASE)
		      && (tp->t_modes.c_lflag & ICANON))) {
			if ((mp = ldterm_output_msg(q, mp, (mblk_t *)NULL,
				tp, OBSIZE, 0)) == NULL)
				break;
		}
		/* Update sysinfo outch */
			drv_setparm(SYSOUTC, msgdsize(mp));
		putnext(q, mp);
		break;

	default:
		putnext(q, mp);	/* pass it through unmolested */
		break;
	}
}
/*
 * Perform output processing on a message, accumulating the output
 * characters in a new message.
 */
mblk_t *
ldterm_output_msg(q, imp, omp, tp, bsize, echoing)
	register queue_t *q;
	mblk_t *imp;		/* head of input message we're examining */
	mblk_t *omp;		/* head of output message we're constructing */
	register ldtermstd_state_t *tp;
	int bsize;
	int echoing;

{
	register mblk_t *ibp;	/* block we're examining from input message */
	register mblk_t *ipbp;	/* block before ibp in input message */
	register mblk_t *obp;	/* block we're filling in output message */
	mblk_t **contpp;	/* where to stuff pointer to newly-allocated block */
	register unsigned char c;
	register int count, ctype;
	register int bytes_left;

	mblk_t *bp;	/* block to stuff an M_DELAY message in */


	/*
	 * Allocate a new block into which to put bytes.
	 * If we can't, we just drop the rest of the message on the
	 * floor.
	 */
#define	NEW_BLOCK()	{ \
			if ((obp = allocb(bsize, BPRI_MED)) == NULL) \
				goto outofbufs; \
			*contpp = obp; \
			contpp = &obp->b_cont; \
			bytes_left = obp->b_datap->db_lim - obp->b_wptr; \
			}

	ibp = imp;
	ipbp = (mblk_t *) NULL;

	/*
	 * When we allocate the first block of a message, we should stuff
	 * the pointer to it in "omp".  All subsequent blocks should
	 * have the pointer to them stuffed into the "b_cont" field of the
	 * previous block.  "contpp" points to the place where we should
	 * stuff the pointer.
	 *
	 * If we already have a message we're filling in, continue doing
	 * so.
	 */
	if ((obp = omp) != NULL) {
		for (; obp->b_cont != NULL; obp = obp->b_cont)
			;
		contpp = &obp->b_cont;
		bytes_left = obp->b_datap->db_lim - obp->b_wptr;
	} else {
		contpp = &omp;
		omp = NULL;
		bytes_left = 0;
	}

	do {
		unsigned int bsize;

		bsize = ibp->b_wptr - ibp->b_rptr;

		/* if channel mapping, map ibp */
		if (bsize && LDTERM_CHANMAP(tp)) {
		   mblk_t *mapbp,*oibp;
		   unchar c;
		   extern emcp_t str_emmapout();
		   emcp_t emp;	/* this is a unchar * to mapped characters */
		   int i;

#define MAXEMSIZE 10	/* give us a little extra room */
		   if ((mapbp = allocb(bsize+MAXEMSIZE,BPRI_HI)) == NULL) {
			cmn_err(CE_NOTE,"ldterm_output_msg: allocb failed. Unable to perform channel mapping");
			goto nomap;
		   }

		   while (ibp->b_rptr != ibp->b_wptr) {
			i = 0;
			if ((mapbp->b_wptr - mapbp->b_rptr) > bsize)
				goto insertmapbp;
			c = *ibp->b_rptr++;
			emp = str_emmapout(&tp->t_emap,c,&i);
			if (i == 0) continue;
			if (i > (mapbp->b_datap->db_lim - mapbp->b_wptr)) {
				/* attempt to copy message */
				mblk_t *cmp;

				cmp = allocb(i+mapbp->b_wptr - mapbp->b_rptr,BPRI_HI);
				if (cmp == NULL)
					goto insertmapbp; /* punt extra chars */
				while (mapbp->b_rptr != mapbp->b_wptr)
					*cmp->b_wptr++ = *mapbp->b_rptr++;
				freemsg(mapbp);
				mapbp = cmp;
			}
			while (i>0) {
				*mapbp->b_wptr++ = *emp++;
				i--;
			}
		   }
		   /* successfully mapped entire ibp */
		   mapbp->b_cont = ibp->b_cont; 
		   freeb(ibp);
		   ibp = mapbp;
		   goto attach_ibp_to_ipbp;

insertmapbp: /* we did not do all of ibp, so save what's left for next loop */
		   mapbp->b_cont = ibp;
		   ibp = mapbp;

attach_ibp_to_ipbp:
		   if (ipbp == NULL) { /* ibp is first block of message */
			imp = ibp;
			ipbp = imp;
		   }
		   else {
			ipbp->b_cont = ibp;
			ipbp = ibp;
		   }
		}
nomap:
		while (ibp->b_rptr < ibp->b_wptr) {
			/*
			 * Make sure there's room for one more
			 * character.  At most, we'll need "t_maxeuc"
			 * bytes.
			 */
			if ((bytes_left < (int) tp->t_maxeuc))
				NEW_BLOCK();

			/*
			 * If doing XCASE processing (not very likely,
			 * in this day and age), look at each character
			 * individually.
			 */
			if ((tp->t_modes.c_lflag & XCASE)
			    && (tp->t_modes.c_lflag & ICANON)) {
				c = *ibp->b_rptr++;

				/*
				 * If character is mapped on output, put out
				 * a backslash followed by what it is
				 * mapped to.
				 */
				if (omaptab[c] != 0
				    && (!echoing || c != '\\')) {
					tp->t_col++;	/* backslash is an ordinary character */
					*obp->b_wptr++ = '\\';
					bytes_left--;
					if (bytes_left == 0)
						NEW_BLOCK();
					c = omaptab[c];
				}

				/*
				 * If no other output processing is required,
				 * push the character into the block and
				 * get another.
				 */
				if (!(tp->t_modes.c_oflag & OPOST)) {
					tp->t_col++;
					*obp->b_wptr++ = c;
					bytes_left--;
					continue;
				}

				/*
				 * OPOST output flag is set.
				 * Map lower case to upper case if OLCUC flag
				 * is set.
				 */
				if ((tp->t_modes.c_oflag & OLCUC)
				    && c >= 'a' && c <= 'z')
					c -= 'a' - 'A';
			} else {
/*
 * Copy all the ORDINARY characters, possibly mapping upper case to lower
 * case.  We use "movtuc", STOPPING when we can't move some character.
 * For multi-byte or multi-column EUC, we can't depend on the regular tables.
 * Rather than just drop through to the "big switch" for all characters, it
 * _might_ be faster to let "movtuc" move a bunch of characters.  Chances
 * are, even in multi-byte mode we'll have lots of ASCII going through.
 * We check the flag once, and call movtuc with the appropriate table as
 * an argument.
 */
				register int bytes_to_move;
				register int bytes_moved;

				bytes_to_move = ibp->b_wptr - ibp->b_rptr;
				if (bytes_to_move > bytes_left)
					bytes_to_move = bytes_left;
				if (tp->t_state & TS_MEUC) {
					bytes_moved = movtuc(bytes_to_move,
					 ibp->b_rptr, obp->b_wptr,
					 (tp->t_modes.c_oflag & OLCUC ? elcuctab : enotrantab));
				}
				else {
					bytes_moved = movtuc(bytes_to_move,
					 ibp->b_rptr, obp->b_wptr,
					 (tp->t_modes.c_oflag & OLCUC ? lcuctab : notrantab));
				}
/*
 * We're save to just do this column calculation, because if TS_MEUC is set,
 * we used the proper EUC tables, and won't have copied any EUC bytes.
 */
				tp->t_col += bytes_moved;
				ibp->b_rptr += bytes_moved;
				obp->b_wptr += bytes_moved;
				bytes_left -= bytes_moved;
				if (ibp->b_rptr >= ibp->b_wptr)
					continue;	/* moved all of block */
				if (bytes_left == 0)
					NEW_BLOCK();
				c = *ibp->b_rptr++;	/* stopper */
			}
/*
 * If the driver has requested, don't process output flags.  However, if
 * we're in multi-byte mode, we HAVE to look at EVERYTHING going out to
 * maintain column position properly. Therefore IF the driver says don't
 * AND we're not doing multi-byte, then don't do it.  Otherwise, do it.
 * 
 * NOTE:  Hardware USUALLY doesn't expand tabs properly for multi-byte
 * situations anyway; that's a known problem with the 3B2 "PORTS" board
 * firmware, and any other hardware that doesn't ACTUALLY know about the
 * current EUC mapping that WE are using at this very moment.  The problem
 * is that memory width is INDEPENDENT of screen width - no relation - so
 * WE know how wide the characters are, but an off-the-host board probably
 * doesn't.  So, until we're SURE that the hardware below us can correctly
 * expand tabs in a multi-byte/multi-column EUC situation, we do it
 * ourselves.
 */
			/*
			 * Map <CR>to<NL> on output if OCRNL flag set.
			 * ONLCR processing is not done if OCRNL is set.

			 */
			if (c == '\r' && (tp->t_modes.c_oflag & OCRNL)) {
				c = '\n';
				ctype = typetab[c];
				goto jocrnl;
			}
			ctype = typetab[c];

			/*
			 * Map <NL> to <CR><NL> on output if ONLCR
			 * flag is set.
			 */
			if (c == '\n' && (tp->t_modes.c_oflag & ONLCR)) {
				if (!(tp->t_state & TS_TTCR)) {
					tp->t_state |= TS_TTCR;
					c = '\r';
					ctype = typetab['\r'];
					--ibp->b_rptr;
				} else
					tp->t_state &= ~TS_TTCR;
			}

/*
* Delay values and column position calculated here.  For EUC chars in
* multi-byte mode, we use "t_eucign" to help calculate columns.  When
* we see the first byte of an EUC, we set t_eucign to the number of
* bytes that will FOLLOW it, and we add the screen width of the WHOLE
* EUC character to the column position.  In particular, we can't count
* SS2 or SS3 as printing characters.  Remember, folks, the screen width
* and memory width are independent - no relation.
* We could have dropped through for ASCII, but we want to catch any
* bad characters (i.e., t_eucign set and an ASCII char received) and
* possibly report the garbage situation.
*/
			jocrnl:

			count = 0;
			switch (ctype) {

			case T_SS2:
			case T_SS3:
			case ORDINARY:
				if (tp->t_state & TS_MEUC) {
					if (NOTASCII(c)) {
						*obp->b_wptr++ = c;
						bytes_left--;
						/* In middle of EUC? */
						if (tp->t_eucign) {
							--tp->t_eucign;
							break;
						}
						else {
							tp->t_col +=
							   ldterm_dispwidth(c,
							     &tp->eucwioc,
							     tp->t_modes.c_lflag & ECHOCTL);
							tp->t_eucign =
							    ldterm_memwidth(c, &tp->eucwioc) - 1;
						}
					}
					else {
						if (tp->t_eucign) {
							tp->t_eucign = 0;
							ldterm_eucwarn(tp);
						}
						tp->t_col++;
						*obp->b_wptr++ = c;
						bytes_left--;
					}
				}
				else {	/* ho hum, ASCII mode... */
					tp->t_col++;
					*obp->b_wptr++ = c;
					bytes_left--;
				}
				break;

/*
* If we're doing ECHOCTL, we've already mapped the thing during the process
* of canonising.  Don't bother here, as it's not one that we did.
*/
			case CONTROL:
				*obp->b_wptr++ = c;
				bytes_left--;
				break;

/*
* This is probably a backspace received, not one that we're
* echoing.  Let it go as a single-column backspace.
*/
			case BACKSPACE:
				if (tp->t_col)
					tp->t_col--;
				if (tp->t_modes.c_oflag & BSDLY) {
					if (tp->t_modes.c_oflag & OFILL)
						count = 2;
					else
						count = 3;
				}
				*obp->b_wptr++ = c;
				bytes_left--;
				break;

			case NEWLINE:
				if (tp->t_modes.c_oflag & ONLRET)
					goto cr;
				if ((tp->t_modes.c_oflag & NLDLY) == NL1)
					count = 2;
				*obp->b_wptr++ = c;
				bytes_left--;
				break;

			case TAB:
/*
* Map '\t' to spaces if XTABS flag is set.  The calculation of "t_eucign"
* has probably insured that column will be correct, as we bumped t_col by the
* DISP width, not the memory width.
*/
				if ((tp->t_modes.c_oflag & TABDLY) == XTABS) {
					for (;;) {
						*obp->b_wptr++ = ' ';
						bytes_left--;
						tp->t_col++;
						if ((tp->t_col & 07) == 0)
							break;	/* every 8th */
						/*
						 * If we don't have room to
						 * fully expand this tab in
						 * this block, back up to
						 * continue expanding it
						 * into the next block.
						 */
						if (obp->b_wptr >= obp->b_datap->db_lim) {
							ibp->b_rptr--;
							break;
						}
					}
				} else {
					tp->t_col |= 07;
					tp->t_col++;
					if (tp->t_modes.c_oflag & OFILL) {
						if (tp->t_modes.c_oflag & TABDLY)
							count = 2;
					} else {
						switch (tp->t_modes.c_oflag & TABDLY) {
						case TAB2:
							count = 6;
							break;

						case TAB1:
							count = 1 + (tp->t_col | ~07);
							if (count < 5)
								count = 0;
							break;
						}
					}
					*obp->b_wptr++ = c;
					bytes_left--;
				}
				break;

			case VTAB:
				if ((tp->t_modes.c_oflag & VTDLY)
				    && !(tp->t_modes.c_oflag & OFILL))
					count = 127;
				*obp->b_wptr++ = c;
				bytes_left--;
				break;

			case RETURN:
				/*
				 * Ignore <CR> in column 0 if ONOCR flag set.
				 */
				if (tp->t_col == 0
				    && (tp->t_modes.c_oflag & ONOCR))
					break;

			cr:
				switch (tp->t_modes.c_oflag & CRDLY) {

				case CR1:
					if (tp->t_modes.c_oflag & OFILL)
						count = 2;
					else
						count = tp->t_col % 2;
					break;

				case CR2:
					if (tp->t_modes.c_oflag & OFILL)
						count = 4;
					else
						count = 6;
					break;

				case CR3:
					if (tp->t_modes.c_oflag & OFILL)
						count = 0;
					else
						count = 9;
					break;
				}
				tp->t_col = 0;
				*obp->b_wptr++ = c;
				bytes_left--;
				break;
			}

			if (count != 0) {
				if (tp->t_modes.c_oflag & OFILL) {
					do {
						if (bytes_left == 0)
							NEW_BLOCK();
						if (tp->t_modes.c_oflag & OFDEL)
							*obp->b_wptr++ = CDEL;
						else
							*obp->b_wptr++ = CNUL;
						bytes_left--;
					} while (--count != 0);
				} else {
					if ((tp->t_modes.c_lflag & FLUSHO)
					    && (tp->t_modes.c_lflag & IEXTEN)) {
						freemsg(omp);	/* drop on floor */
					} else {
					/* Update sysinfo outch */
						drv_setparm(SYSOUTC, msgdsize(omp));
						putnext(q, omp);
						/*
						 * Send M_DELAY downstream
						 */
						if (( bp = allocb( 1, BPRI_MED)) != NULL) {
							bp->b_datap->db_type = M_DELAY;
							*bp->b_wptr++ = count;
							(*q->q_next->q_qinfo->qi_putp)( q->q_next, bp);
						}
					}
					bytes_left = 0;
					/*
					 * We have to start a new message;
					 * the delay introduces a break
					 * between messages.
					 */
					omp = NULL;
					contpp = &omp;
				}
			}
		}
	} while ((ibp = ibp->b_cont) != NULL);	/* next block, if any */

outofbufs:
	freemsg(imp);
	return (omp);
#undef NEW_BLOCK
}

#if !defined(vax) && !defined(sun)
movtuc(size, from, origto, table)
	register int size;
	register unsigned char *from;
	unsigned char *origto;
	register unsigned char *table;
{
	register unsigned char *to = origto;
	register unsigned char c;

	while (size != 0 && (c = table[*from++]) != 0) {
		*to++ = c;
		size--;
	}
	return (to - origto);
}
#endif

void
ldterm_flush_output(c, q, tp)
	unsigned char c;
	register queue_t *q;
	register ldtermstd_state_t *tp;
{

	/* Already conditioned with IEXTEN during VDISCARD processing*/
	if (tp->t_modes.c_lflag & FLUSHO)
		tp->t_modes.c_lflag &= ~FLUSHO;
	else {
		flushq(q, FLUSHDATA);	/* flush our write queue */
		(void) putctl1(q->q_next, M_FLUSH, FLUSHW);	/* flush the ones below us */
		if ((tp->t_echomp = allocb(EBSIZE, BPRI_HI)) != NULL) {
			(void) ldterm_echo(c, q, 1, tp);
			if (tp->t_msglen != 0)
				ldterm_reprint(q, EBSIZE, tp);
			if (tp->t_echomp != NULL) {
				putnext(q, tp->t_echomp);
				tp->t_echomp = NULL;
			}
		}
		tp->t_modes.c_lflag |= FLUSHO;
	}
}

/*
 * Signal generated by the reader: M_PCSIG and M_FLUSH messages sent.
 */
void
ldterm_dosig(q, sig, c, mtype, mode)
	register queue_t *q;
	int sig;
	unsigned char c;
{
	register ldtermstd_state_t *tp = (ldtermstd_state_t *)q->q_ptr;

	/*
	 * c == \0 is brk case; need to flush on BRKINT even
         * if noflsh is set.
         */
	if ((!(tp->t_modes.c_lflag & NOFLSH)) || (c == '\0')) {
		if (mode) {
			/*
			 * Flush read or write side
			 * Restart the input or output
			 */

			if (mode & FLUSHR) {
				flushq(q, FLUSHDATA);
				(void) putctl1(WR(q)->q_next, M_FLUSH, FLUSHR);
				if (tp->t_state & TS_TBLOCK) {
					(void) putctl(WR(q)->q_next, M_STARTI);
					tp->t_state &= ~TS_TBLOCK;
				}
			}
			if (mode & FLUSHW) {
				flushq(WR(q), FLUSHDATA);
				(void) putctl1(q->q_next, M_FLUSH, FLUSHW);
				if (tp->t_state & TS_TTSTOP) {
					(void) putctl(WR(q)->q_next, M_START);
					tp->t_state &= ~TS_TTSTOP;
				}
			}
		}
	}
	tp->t_state &= ~TS_QUOT;
	(void) putctl1(q->q_next, mtype, sig);

	if (c != '\0') {
		if ((tp->t_echomp = allocb(4, BPRI_HI)) != NULL) {
			(void) ldterm_echo(c, WR(q), 4, tp);
			putnext(WR(q), tp->t_echomp);
			tp->t_echomp = NULL;
		}
	}
	return;
}

void
ldterm_iocack(qp, mp, iocp, rval)
queue_t *qp;
mblk_t *mp;
struct iocblk *iocp;
int rval;
{
	mblk_t	*tmp;

	mp->b_datap->db_type = M_IOCACK;
	iocp->ioc_rval = rval;
	iocp->ioc_count = iocp->ioc_error = 0;
	if ((tmp = unlinkb(mp)) != (mblk_t *)NULL)
		freemsg(tmp);
	qreply(qp,mp);
}

void
ldterm_iocnack(qp, mp, iocp, error, rval)
queue_t *qp;
mblk_t *mp;
struct iocblk *iocp;
int error;
int rval;
{
	mp->b_datap->db_type = M_IOCACK;
	iocp->ioc_rval = rval;
	iocp->ioc_error = error;
	qreply(qp,mp);
}

ldterm_do_iocdata(q,mp)
	queue_t *q;
	register mblk_t *mp;
{
	register ldtermstd_state_t *tp;
	register struct copyresp *crsp;
	register struct iocblk *iocp;

	crsp = (struct copyresp *)mp->b_rptr;
	tp = (ldtermstd_state_t *)q->q_ptr;
	iocp = (struct iocblk *)mp->b_rptr;

	switch (iocp->ioc_cmd) {
	default:
		putnext(q,mp); /* not for us */
		return;

	case LDGMAP: 
		if (crsp->cp_rval) { /* already nak'ked for us */
			freemsg(mp);
			return;
		}

		ldterm_iocack(q,mp,iocp,0);
		return;

	case LDSMAP: {
		int error;
		if (crsp->cp_rval) { /* already nak'ked for us */
			freemsg(mp);
			return;
		}
		if (pullupmsg(mp->b_cont,E_TABSZ) == 0) {
			ldterm_iocnack(q,mp,iocp,EINVAL,-1);
			return;
		}
		error = str_emsetmap(q,mp->b_cont,&tp->t_emap);
		if (error) {
			ldterm_iocnack(q,mp,iocp,error,-1);
			return;
		}
		ldterm_iocack(q,mp,iocp,0);
		return;
	}
   	}
	return;
}



/*
 * Called when an M_IOCTL message is seen on the write queue; does whatever
 * we're supposed to do with it, and either replies immediately or passes it
 * to the next module down.
 */
void
ldterm_do_ioctl(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register ldtermstd_state_t *tp;
	register struct iocblk *iocp;
	register struct eucioc *euciocp;	/* needed for EUC ioctls */

	iocp = (struct iocblk *)mp->b_rptr;
	tp = (ldtermstd_state_t *)q->q_ptr;


	switch (iocp->ioc_cmd) {

	case LDSMAP: {
		struct copyreq *crqp;
		if (iocp->ioc_count != TRANSPARENT) {
			ldterm_iocnack(q,mp,iocp,EINVAL,-1);
			return;
		}
		crqp = (struct copyreq *) iocp;
		crqp->cq_addr = * (caddr_t *)mp->b_cont->b_rptr;
		crqp->cq_size = E_TABSZ;
		crqp->cq_private = (mblk_t *) NULL;
		crqp->cq_flag = 0;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		mp->b_datap->db_type = M_COPYIN;
		qreply(q,mp);
		return;
	}

	case LDGMAP: {
		mblk_t *datamp;
		struct copyreq *crqp;
		int error;

		if (iocp->ioc_count != TRANSPARENT) {
			ldterm_iocnack(q,mp,iocp,EINVAL,-1);
			return;
		}
		if ((datamp = allocb(E_TABSZ,BPRI_HI)) == (mblk_t *) NULL) {
			ldterm_iocnack(q,mp,iocp,ENOMEM,-1);
			return;
		}
		error = str_emgetmap(&tp->t_emap,datamp);
		if (error) {
			freemsg(datamp);
			ldterm_iocnack(q,mp,iocp,error,-1);
			return;
		}
		crqp = (struct copyreq *) iocp;
		crqp->cq_addr = * (caddr_t *)mp->b_cont->b_rptr;
		freeb(mp->b_cont);
		crqp->cq_size = E_TABSZ;
		crqp->cq_private = (mblk_t *) NULL;
		crqp->cq_flag = 0;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		mp->b_cont = datamp;
		mp->b_datap->db_type = M_COPYOUT;
		qreply(q,mp);
		return;
	}

	case LDNMAP: 
		if (iocp->ioc_count != TRANSPARENT) {
			ldterm_iocnack(q,mp,iocp,EINVAL,-1);
			return;
		}
		str_emunmap(q,&tp->t_emap);
		ldterm_iocack(q,mp,iocp,0);
		return;


	case TCSETS:
	case TCSETSW:
	case TCSETSF: {
		/*
		 * Set current parameters and special characters.
		 */
		register struct termios *cb;
		struct termios oldmodes;

		if (! mp->b_cont)	/* THIS CAN HAPPEN! */
			goto setgetfail;

		cb = (struct termios *)mp->b_cont->b_rptr;

		oldmodes = tp->t_amodes;
		tp->t_amodes = *cb;
		if ((tp->t_amodes.c_lflag & PENDIN)
		    && (tp->t_modes.c_lflag & IEXTEN)) {
			/*
			 * Yuk.  The C shell file completion code actually
			 * uses this "feature", so we have to support it.
			 */
			if (tp->t_message != NULL) {
				tp->t_state |= TS_RESCAN;
				qenable(RD(q));
			}
			tp->t_amodes.c_lflag &= ~PENDIN;
		}

		bcopy((caddr_t)tp->t_amodes.c_cc, (caddr_t)tp->t_modes.c_cc, NCCS);

		/* ldterm_adjust_modes does not deal with cflags */
		tp->t_modes.c_cflag = tp->t_amodes.c_cflag;

		ldterm_adjust_modes (tp);
		if (chgstropts(&oldmodes, tp, RD(q)) == (-1)) {
			iocp->ioc_error = EAGAIN;
			goto setgetfail2;
		}

		/*
		 * The driver may want to know about the following iflags:
		 * IGNBRK, BRKINT, IGNPAR, PARMRK, INPCK, IXON, IXANY.
		 */
		break;
	}

	case TCSETA:
	case TCSETAW:
	case TCSETAF: {
		/*
		 * Old-style "ioctl" to set current parameters and
		 * special characters.
		 * Don't clear out the unset portions, leave them as
		 * they are.
		 */
		register struct termio *cb;
		struct termios oldmodes;

		if (! mp->b_cont)	/* THIS CAN HAPPEN! */
			goto setgetfail;

		cb = (struct termio *)mp->b_cont->b_rptr;

		oldmodes = tp->t_amodes;
		tp->t_amodes.c_iflag =
		    (tp->t_amodes.c_iflag & 0xffff0000 | cb->c_iflag);
		tp->t_amodes.c_oflag =
		    (tp->t_amodes.c_oflag & 0xffff0000 | cb->c_oflag);
		tp->t_amodes.c_cflag =
		    (tp->t_amodes.c_cflag & 0xffff0000 | cb->c_cflag);
		tp->t_amodes.c_lflag =
		    (tp->t_amodes.c_lflag & 0xffff0000 | cb->c_lflag);

		bcopy((caddr_t)cb->c_cc, (caddr_t)tp->t_modes.c_cc, NCC);
		/* TCGETS returns amodes, so update that too */
		bcopy((caddr_t)cb->c_cc, (caddr_t)tp->t_amodes.c_cc, NCC);

		/* ldterm_adjust_modes does not deal with cflags */
		tp->t_modes.c_cflag = tp->t_amodes.c_cflag;

		ldterm_adjust_modes (tp);
		if (chgstropts(&oldmodes, tp, RD(q)) == (-1)) {
			iocp->ioc_error = EAGAIN;
			goto setgetfail2;
		}

		/*
		 * The driver may want to know about the following iflags:
		 * IGNBRK, BRKINT, IGNPAR, PARMRK, INPCK, IXON, IXANY.
		 */
		break;
	}

	case TCFLSH:
		/*
		 * Do the flush on the write queue immediately, and queue
		 * up any flush on the read queue for the service procedure
		 * to see.  Then turn it into the appropriate M_FLUSH message,
		 * so that the module below us doesn't have to know about
		 * TCFLSH.
		 */
		if (! mp->b_cont)
			goto setgetfail;
		ASSERT(mp->b_datap != NULL);
		if (*(int *)mp->b_cont->b_rptr == 0) {
			ASSERT(mp->b_datap != NULL);
			(void) putctl1(q->q_next, M_FLUSH, FLUSHR);
			(void) putctl1(RD(q), M_FLUSH, FLUSHR);
		} else if (*(int *)mp->b_cont->b_rptr == 1) {
			flushq(q, FLUSHDATA);
			ASSERT(mp->b_datap != NULL);
			(void) putctl1(q->q_next, M_FLUSH, FLUSHW);
			(void) putctl1(RD(q)->q_next, M_FLUSH, FLUSHW);
		} else if (*(int *)mp->b_cont->b_rptr == 2) {
			flushq(q, FLUSHDATA);
			ASSERT(mp->b_datap != NULL);
			(void) putctl1(q->q_next, M_FLUSH, FLUSHRW);
			(void) putctl1(RD(q), M_FLUSH, FLUSHRW);
		} else {
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			iocp->ioc_rval = (-1);
			iocp->ioc_count = 0;
			qreply(q, mp);
			return;
		}
		ASSERT(mp->b_datap != NULL);
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_rval = 0;
		iocp->ioc_count = 0;
		qreply(q, mp);
		return;

	case TCXONC:
		if (! mp->b_cont)
			goto setgetfail;

		switch (*(int *)mp->b_cont->b_rptr) {
		case 0:
			if (!(tp->t_state & TS_TTSTOP)) {
				(void) putctl(q->q_next, M_STOP);
				tp->t_state |= TS_TTSTOP;
			}
			break;

		case 1:
			if (tp->t_state & TS_TTSTOP) {
				(void) putctl(q->q_next, M_START);
				tp->t_state &= ~TS_TTSTOP;
			}
			break;

		case 2:
			if (!(tp->t_state & TS_TBLOCK)) {
				(void) putctl(q->q_next, M_STOPI);
				tp->t_state |= TS_TBLOCK;
			}
			break;

		case 3:
			if (tp->t_state & TS_TBLOCK) {
				(void) putctl(q->q_next, M_STARTI);
				tp->t_state &= ~TS_TBLOCK;
			}
			break;

		default:
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			iocp->ioc_rval = (-1);
			iocp->ioc_count = 0;
			qreply(q, mp);
			return;
		}
		ASSERT(mp->b_datap != NULL);
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_rval = 0;
		iocp->ioc_count = 0;
		qreply(q, mp);
		return;
/* 
 * TCSBRK is expected to be handled by the driver. The reason its
 * left for the driver is that when the argument to TCSBRK is zero
 * driver has to drain the data and sending a M_IOCACK from LDTERM
 * before the driver drains the data is going to cause problems.
 */

	/*
	 * The following are EUC related ioctls.  For EUC_WSET,
	 * we have to pass the information on, even though we ACK
	 * the call.  It's vital in the EUC environment that
	 * everybody downstream knows about the EUC codeset
	 * widths currently in use; we therefore pass down the
	 * information in an M_CTL message.  It will bottom out
	 * in the driver.
	 */
	case EUC_WSET: {

		register struct iocblk *riocp;	/* only needed for EUC_WSET */
		register mblk_t *dmp, *dmp_cont;
		register int i;

		/*
		 * If the user didn't supply any information, NAK it.
		 */
		if ((! mp->b_cont) ||
		    (! (euciocp = (struct eucioc *) mp->b_cont->b_rptr))) {
/*
 * protocol failure comes here - either there wasn't a buffer attached
 * when there should have been, or something else is amiss.
 */
setgetfail:
			iocp->ioc_error = EPROTO;
/*
 * Streams resource failures and others can come here, with ioc_error
 * already set (e.g., to ENOSR or whatever).
 */
setgetfail2:
			iocp->ioc_count = 0;
			iocp->ioc_rval = (-1);
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return;
		}
		/*
		 * Check here for reasonableness.  If anything will take
		 * more than EUC_MAXW columns or more than EUC_MAXW bytes
		 * following SS2 or SS3, then just reject it out of hand.
		 * It's not impossible for us to do it, it just isn't
		 * reasonable.  So far, in the world, we've seen the
		 * absolute max columns to be 2 and the max number of
		 * bytes to be 3.  This allows room for some expansion
		 * of that, but it probably won't even be necessary.
		 * At the moment, we return a "range" error.  If you
		 * really need to, you can push EUC_MAXW up to over 200;
		 * it doesn't make sense, though, with only a CANBSIZ sized
		 * input limit (usually 256)!
		 */
		for (i = 0; i < 4; i++) {
			if ((euciocp->eucw[i] > EUC_MAXW) ||
			    (euciocp->scrw[i] > EUC_MAXW)) {
				iocp->ioc_error = ERANGE;
				goto setgetfail2;
			}
		}
		/*
		 * Otherwise, save the information in tp, force codeset 0
		 * (ASCII) to be one byte, one column.
		 */
		cp_eucwioc(euciocp, &tp->eucwioc, EUCIN);
		tp->eucwioc.eucw[0] = tp->eucwioc.scrw[0] = 1;
		/*
		 * Now, check out whether we're doing multibyte processing.
		 * if we are, we need to allocate a block to hold the
		 * parallel array.
		 * By convention, we've been passed what amounts to a
		 * CSWIDTH definition.  We actually NEED the number of
		 * bytes for Codesets 2 & 3.  
		 */
		tp->t_maxeuc = 0;	/* reset to say we're NOT */
		tp->t_state &= ~TS_MEUC;
		/*
		 * We'll set TS_MEUC if we're doing multi-column OR multi-
		 * byte OR both.  It makes things easier...  NOTE:  If we
		 * fail to get the buffer we need to hold display widths,
		 * then DON'T let the TS_MEUC bit get set!
		 */
		for (i = 0; i < 4; i++) {
			if (tp->eucwioc.eucw[i] > tp->t_maxeuc)
				tp->t_maxeuc = tp->eucwioc.eucw[i];
			if (tp->eucwioc.scrw[i] > 1)
				tp->t_state |= TS_MEUC;
		}
		if ((tp->t_maxeuc > 1) || (tp->t_state & TS_MEUC)) {
			if (!tp->t_eucp_mp) {
				if (! (tp->t_eucp_mp = allocb(CANBSIZ, BPRI_HI))) {
					tp->t_maxeuc = 1;
					tp->t_state &= ~TS_MEUC;
/* cmn_err(CE_WARN, "Can't allocate eucp_mp\n"); */
					iocp->ioc_error = ENOSR;
					goto setgetfail2;
				}
			}
			/*
			 * here, if there's junk in the canonical buffer,
			 * then move the eucp pointer past it, so we don't
			 * run off the beginning.  This is a total botch,
			 * but will hopefully keep stuff from getting too
			 * messed up until the user flushes this line!
			 */
			if (tp->t_msglen) {
				tp->t_eucp = tp->t_eucp_mp->b_rptr;
				for (i = tp->t_msglen; i; i--)
					*tp->t_eucp++ = 1;
				tp->t_eucp = tp->t_eucp_mp->b_rptr + tp->t_msglen;
			}
			else
				tp->t_eucp = tp->t_eucp_mp->b_rptr;
			tp->t_state |= TS_MEUC;	/* doing multi-byte handling */
		}
		else if (tp->t_eucp_mp) {
			freemsg(tp->t_eucp_mp);
		}

		/*
		 * If we are able to allocate two blocks (the iocblk and
		 * the associated data), then pass it downstream, otherwise
		 * we'll need to NAK it, and drop whatever we WERE able to
		 * allocate.
		 */
		if (! (dmp = allocb(sizeof(struct iocblk), BPRI_HI))) {
			iocp->ioc_error = ENOSR;
			goto setgetfail2;
		}
		if (! (dmp_cont = allocb(EUCSIZE, BPRI_HI))) {
			freemsg(dmp);
			iocp->ioc_error = ENOSR;
			goto setgetfail2;
		}
		/*
		 * We got both buffers.  Copy out the EUC information
		 * (as we received it, not what we're using!) & pass it on.
		 */
		bcopy((caddr_t)mp->b_cont->b_rptr, (caddr_t)dmp_cont->b_wptr, EUCSIZE);
		dmp_cont->b_wptr += EUCSIZE;
		dmp->b_cont = dmp_cont;
		dmp->b_datap->db_type = M_CTL;
		dmp_cont->b_datap->db_type = M_DATA;
		riocp = (struct iocblk *) dmp->b_rptr;
		riocp->ioc_count = iocp->ioc_count;
		riocp->ioc_error = 0;
		riocp->ioc_rval = 0;
		riocp->ioc_cmd = EUC_WSET;
		putnext(q, dmp);
		/*
		 * Now ACK the ioctl.
		 */
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_rval = 0;
		qreply(q, mp);
		return;
	}

	case EUC_WGET:
		if (! mp->b_cont)
			goto setgetfail;	/* protocol error */
		euciocp = (struct eucioc *) mp->b_cont->b_rptr;
		cp_eucwioc(&tp->eucwioc, euciocp, EUCOUT);
		iocp->ioc_count = EUCSIZE;
		iocp->ioc_error = iocp->ioc_rval = 0;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_rval = 0;
		qreply(q, mp);
		return;

	}

	putnext(q, mp);
}

/*
 * Send an M_SETOPTS message upstream if any mode changes are being made
 * that affect the stream head options.
 * Also send an M_FLUSH message upstream if canonical mode is being turned on;
 * a pile of non-canonical input will look very confusing when we switch to
 * "message-nondiscard" mode - it will appear to be bunches of characters
 * separated by EOFs.
 * returns -1 if allocb fails, else returns 0.
 */
int
chgstropts(oldmodep, tp, q)
	register struct termios *oldmodep;
	register ldtermstd_state_t *tp;
	register queue_t *q;
{
	struct stroptions optbuf;
	register mblk_t *bp;
	int s;

	optbuf.so_flags = 0;
	if ((oldmodep->c_lflag ^ tp->t_modes.c_lflag) & ICANON) {
		/*
		 * Canonical mode is changing state; switch the stream head
		 * to message-nondiscard or byte-stream mode.  Also, rerun
		 * the service procedure so it can change its mind about
		 * whether to send data upstream or not.
		 */
		if (tp->t_modes.c_lflag & ICANON) {
			DEBUG4 (("CHANGING TO CANON MODE\n"));
			(void) putctl1(q->q_next, M_FLUSH, FLUSHR);
			optbuf.so_flags = SO_READOPT|SO_MREADOFF;
			optbuf.so_readopt = RMSGN;

			/* if there is a pending raw mode timeout, clear it */

			s = splstr();
			if (tp->t_tid && (tp->t_state & TS_TACT)) {
				tp->t_state &= ~TS_TACT;
				tp->t_state &= ~TS_RTO;
				DEBUG4 (("chgstropts: timer active untimeout called \n"));
				untimeout(tp->t_tid);
				tp->t_tid = 0;
			}
			(void) splx(s);
		} else {
			DEBUG4 (("CHANGING TO RAW MODE\n"));
			optbuf.so_flags = SO_READOPT|SO_MREADON;
			optbuf.so_readopt = RNORM;
		}
	}

	if ((oldmodep->c_lflag ^ tp->t_modes.c_lflag) & TOSTOP) {
		/*
		 * The "stop on background write" bit is changing.
		 */
		if (tp->t_modes.c_lflag & TOSTOP)
			optbuf.so_flags |= SO_TOSTOP;
		else
			optbuf.so_flags |= SO_TONSTOP;
	}

	if (optbuf.so_flags != 0) {
		if ((bp = allocb(sizeof (struct stroptions), BPRI_HI)) == NULL) {
			return(-1);
		}
		*(struct stroptions *)bp->b_wptr = optbuf;
		bp->b_wptr += sizeof (struct stroptions);
		bp->b_datap->db_type = M_SETOPTS;
		DEBUG4 (("M_SETOPTS to stream head\n"));
		putnext(q, bp);
	}
	return(0);
}


/*
 * Called when an M_IOCACK message is seen on the read queue; modifies
 * the data being returned, if necessary, and passes the reply up.
 */
void
ldterm_ioctl_reply(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register ldtermstd_state_t *tp;
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	tp = (ldtermstd_state_t *)q->q_ptr;

	switch (iocp->ioc_cmd) {

	case TCGETS: {
		/*
		 * Get current parameters and return them to stream head
		 * eventually.
		 */
		register struct termios *cb =
		    (struct termios *)mp->b_cont->b_rptr;

		/* cflag has cflags sent upstream by the driver */
		register unsigned long cflag = cb->c_cflag;

		*cb = tp->t_amodes;
		if (cflag != 0)
			cb->c_cflag = cflag;	/* set by driver */
		break;
	}

	case TCGETA: {
		/*
		 * Old-style "ioctl" to get current parameters and
		 * return them to stream head eventually.
		 */
		register struct termio *cb =
		    (struct termio *)mp->b_cont->b_rptr;

		cb->c_iflag = tp->t_amodes.c_iflag;	/* all except the */
		cb->c_oflag = tp->t_amodes.c_oflag;	/* cb->c_cflag */
		cb->c_lflag = tp->t_amodes.c_lflag;

		if (cb->c_cflag == 0)	/* not set by driver */
			cb->c_cflag = tp->t_amodes.c_cflag;

		cb->c_line = 0;
		bcopy((caddr_t)tp->t_amodes.c_cc, (caddr_t)cb->c_cc, NCC);
		break;
	}
	}
	putnext(q, mp);
}

/*
 * Routine called by a driver when a "canput" fails because the queue
 * upstream is full (driver without a service procedure) or when the driver
 * queues have reached MAX_INPUT level (driver with a service procedure).
 * Drivers can send STOP/START characters to stop input at its respective
 * high and low water marks before reaching MAX_INPUT level.
 * If IMAXBEL mode is set, echo a bell; otherwise,
 * flush the queue in question.
 */
void
ldterm_qfull(q)
	queue_t *q;
{
	register queue_t *qnext;
	register ldtermstd_state_t *tp;

	if ((qnext = q->q_next) != NULL) {
		if (qnext->q_qinfo == &ldtermrinit
		    && (tp = (ldtermstd_state_t *)qnext->q_ptr) != NULL
		    && tp->t_modes.c_iflag & IMAXBEL) {
			if ((tp->t_echomp = allocb(4, BPRI_HI)) != NULL) {
				ldterm_outchar(CTRL('g'), WR(qnext), 4, tp);
				putnext(WR(qnext), tp->t_echomp);
				tp->t_echomp = NULL;
			}
		} else {
			(void) putctl1(qnext, M_FLUSH, FLUSHR);
		}
	}
}
void
ldterm_vmin_timeout (q)
	register queue_t *q;
{
	
	register ldtermstd_state_t *tp;
	int s;

	DEBUG4(("ldterm_vmin_timeout:\n"));
	/* 
	 * LDTERM  may be popped when a function is pending in callout table.
	 */
	if (!q) {
		DEBUG4( ("ldterm_vmin_timeout returning with a bad q pointer\n"));
	 	return;
	}

	tp = (ldtermstd_state_t *)q->q_ptr;


	if (!tp) {
		DEBUG4( ("ldterm_vmin_timeout returning with a bad tp pointer\n"));
	 	return;
	}

	tp->t_state &= ~TS_TACT;

	if (CANON_MODE || (tp->t_state & TS_CLOSE)) {
		DEBUG4 (("OOPS: CANON MODE or CLOSE interrupted\n"));
		tp->t_state &= ~TS_RTO;
		return;
	}
	/* if VMIN has any value, the timer is to be started after one
	 * char has been received only (if need be). Hence t_msglen
	 * should never be equal to zero here if VMIN > 0.
	 * However, this can happens in some situations like flush
	 * followed by input before the timer expires etc.
	 */


	if ((tp->t_msglen == 0) && V_MIN) {
		DEBUG4 (("OOPS: Timer messed up\n"));
		tp->t_state &= ~TS_RTO;
		return;				
	}
	if (!(tp->t_state & TS_RTO )) {
		tp->t_state |= TS_RTO | TS_TACT;
		DEBUG4( ("calling timeout from ldterm_vmin_timeout\n"));
		tp->t_tid = timeout (ldterm_vmin_timeout, (caddr_t)q, (int)(V_TIME * (HZ / 10)));
		return;
	}
	/* Send RAW blocks off here */
	tp->t_state &= ~TS_RTO;
	ldterm_msg_upstream (q, tp);
	DEBUG4 (("VMIN READY\n"));
}

/* Routine to adjust termios flags to be processed by the line discipline.
 * Driver below sends a termios structure, with the flags the driver
 * intends to process. XOR'ing the driver sent termios structure with
 * current termios structure with the default values (or set by ioctls
 * from userland), we come up with a new termios structrue, the flags
 * of which will be used by the line discipline in processing input and
 * output. On return from this routine, we will have the following fields
 * set in tp structure -->
 * tp->t_modes:	modes the line discipline will process
 * tp->t_amodes: modes the user process thinks the line discipline is 
 * 		 processing
 */

void
ldterm_adjust_modes (tp)

	register ldtermstd_state_t *tp;

{

	DEBUG6 (("original iflag = %o\n", tp->t_modes.c_iflag)); 
	tp->t_modes.c_iflag = tp->t_amodes.c_iflag & ~(tp->t_dmodes.c_iflag);
	tp->t_modes.c_oflag = tp->t_amodes.c_oflag & ~(tp->t_dmodes.c_oflag);
	tp->t_modes.c_lflag = tp->t_amodes.c_lflag & ~(tp->t_dmodes.c_lflag);
	DEBUG6 (("driver iflag = %o\n", tp->t_dmodes.c_iflag)); 
	DEBUG6 (("apparent iflag = %o\n", tp->t_amodes.c_iflag)); 
	DEBUG6 (("effective iflag = %o\n", tp->t_modes.c_iflag)); 

	/* No negotiation of clfags  c_cc array special characters */
	/* Copy from amodes to modes already done by TCSETA/TCSETS code*/
}

/*
 * Erase one EUC SUPPLEMENTARY character.  If TS_MEUC is set AND this is
 * an EUC character (NOT! an ASCII character!), then this should be called
 * instead of ldterm_erase.  "ldterm_erase" will handle ASCII nicely, thank you.
 *
 * We'd better be pointing to the last byte.  If we aren't, it will get
 * screwed up.
 */
void
ldterm_euc_erase(q, ebsize, tp)

	register queue_t *q;
	int ebsize;
	register ldtermstd_state_t *tp;
{
	register int c, i, ung;
	register unchar *p, *bottom;

	if (tp->t_eucleft) {
		/* XXX Ick.  We're in the middle of an EUC! */
		/* What to do now? */
		ldterm_eucwarn(tp);
		return;	/* ignore it??? */
	}
	bottom = tp->t_eucp_mp->b_rptr;
	p = tp->t_eucp - 1;	/* previous byte */
	ung = 1;	/* number of bytes to un-get from buffer */
	/*
	 * go through the buffer until we find the beginning of the
	 * multi-byte char.
	 */
	while ((*p == 0) && (p > bottom)) {
		p--;
		++ung;
	}
	/*
	 * Now, "ung" is the number of bytes to unget from the buffer and
	 * "*p" is the disp width of it.
	 * Fool "ldterm_rubout" into thinking we're rubbing out ASCII
	 * characters.  Do that for the display width of the character.
	 */
	for (i = 0; i < ung; i++) {	/* remove from buf */
		if ((c = ldterm_unget(tp)) != (-1))
			ldterm_trim(tp);
	}
	for (i = 0; i < (int) *p; i++)	/* remove from screen */
		ldterm_rubout(' ', q, ebsize, tp);
	/*
	 * Adjust the parallel array pointer.  Zero out the contents of
	 * parallel array for this position, just to make sure...
	 */
	tp->t_eucp = p;
	*p = 0;
}

/*
 * This is kind of a safety valve.  Whenever we see a bad sequence come
 * up, we call eucwarn.  It just tallies the junk until a threshold is
 * reached.  Then it prints ONE message on the console and not any more.
 * Hopefully, we can catch garbage; maybe it will be useful to somebody.
 */
void
ldterm_eucwarn(tp)

	register ldtermstd_state_t *tp;
{
	++tp->t_eucwarn;
#ifdef DEBUG
	if ((tp->t_eucwarn > EUC_WARNCNT) && !(tp->t_state & TS_WARNED)) {
		cmn_err(CE_WARN,"ldterm: tty at addr %x in multi-byte mode --\n",tp);
		cmn_err(CE_WARN, "Over %d bad EUC characters this session\n", EUC_WARNCNT);
		tp->t_state |= TS_WARNED;
	}
#endif
}

/*
 * Copy an "eucioc_t" structure.  We use the structure with incremented
 * values for Codesets 2 & 3.  The specification in eucioctl is that
 * the sames values as the CSWIDTH definition at user level are passed to us.
 * When we copy it "in" to ourselves, we do the increment.  That allows us
 * to avoid treating each character set separately for "t_eucleft" purposes.
 * When we copy it "out" to return it to the user, we decrement the values
 * so the user gets what it expects, and it matches CSWIDTH in the environment
 * (if things are consistent!).
 */
void
cp_eucwioc(from, to, dir)

	register eucioc_t *from, *to;
	int dir;
{
	bcopy((caddr_t)from, (caddr_t)to, EUCSIZE);
	if (dir == EUCOUT) {	/* copying out to user */
		if (to->eucw[2])
			--to->eucw[2];
		if (to->eucw[3])
			--to->eucw[3];
	}
	else {			/* copying in */
		if (to->eucw[2])
			++to->eucw[2];
		if (to->eucw[3])
			++to->eucw[3];
	}
}

/*
 * ldtermemwidth - Take the first byte of an EUC (or an ASCII char) and
 * return its memory width.  The routine could have been implemented to
 * use only the codeset number, but that would require the caller to
 * have that value available.  Perhaps the user doesn't want to make
 * the extra call or keep the value of codeset around.  Therefore, we
 * use the actual character with which they're concerned.  This should
 * never be called with anything but the first byte of an EUC, otherwise
 * it will return a garbage value.
 */

int
ldterm_memwidth(c, w)

	register unchar c;
	register eucioc_t *w;

{
	if (ISASCII(c))
		return 1;
	switch (c) {
		case SS2: return(w->eucw[2]);
		case SS3: return(w->eucw[3]);
		default:  return(w->eucw[1]);
	}
}

/*
 * ldterm_dispwidth - Take the first byte of an EUC (or ASCII) and return
 * the display width.  Since this is intended mostly for multi-byte
 * handling, it returns EUC_TWIDTH for tabs so they can be
 * differentiated from EUC characters (assumption: EUC require fewer
 * than 255 columns).  Also, if it's a backspace and !flag, it returns
 * EUC_BSWIDTH.  Newline & CR also depend on flag.  This routine SHOULD
 * be cleaner than this, but we have the situation where we may or
 * may not be counting control characters as having a column width.
 * Therefore, the computation of ASCII is pretty messy.  The caller will
 * be storing the value, and then switching on it when it's used.  We
 * really should define the EUC_TWIDTH and other constants in a header
 * so that the routine could be used in other modules in the kernel.
 */

int
ldterm_dispwidth(c, w, mode)

	register unchar c;
	register eucioc_t *w;
	int mode;	/* the t_state variable ; we need ECHOCTL */
{
	if (ISASCII(c)) {
		if (c <= '\037') {
			switch (c) {
			case '\t':	return EUC_TWIDTH;
			case '\b':	return(mode ? 2 : EUC_BSWIDTH);
			case '\n':	return(mode ? 2 : EUC_NLWIDTH);
			case '\r':	return(mode ? 2 : EUC_CRWIDTH);
			default:	return(mode ? 2 : 0);
			}
		}
		return 1;
	}
	switch (c) {
		case SS2: return(w->scrw[2]);
		case SS3: return(w->scrw[3]);
		default:  return(w->scrw[1]);
	}
}

/*
 * Take the first byte of an EUC, or an ASCII char.  Return its codeset.
 * If it's NOT the first byte of an EUC, then the return value may be
 * garbage, as it's probably not SS2 or SS3, and therefore must be in
 * codeset 1.  Another bizarre catch here is the fact that we don't
 * do anything about the "C1" control codes.  In real life, we should;
 * but nobody's come up with a good way of treating them.
 */

int
ldterm_codeset(c)

	register unchar c;
{
	if (ISASCII(c))
		return 0;
	switch (c) {
		case SS2: return(2);
		case SS3: return(3);
		default:  return(1);
	}
}
