/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:log.c	1.3"

/*
 * Streams log driver.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/stropts.h"
#include "sys/stream.h"
#include "sys/strstat.h"
#include "sys/log.h"
#include "sys/inline.h"
#include "sys/strlog.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/cred.h"
#include "sys/file.h"
#include "sys/ddi.h"
#include "sys/syslog.h"
#include "sys/cmn_err.h"

STATIC int logopen(), logclose(), logwput(), logrsrv();
STATIC int logtrc(), logerr(), logcons();
STATIC int logtrace(), shouldtrace();
void loginit();

STATIC struct module_info logm_info = {
	LOG_MID, LOG_NAME, LOG_MINPS, LOG_MAXPS, LOG_HIWAT, LOG_LOWAT
};

STATIC struct qinit logrinit = {
	NULL, logrsrv, logopen, logclose, NULL, &logm_info, NULL
};

STATIC struct qinit logwinit = {
	logwput, NULL, NULL, NULL, NULL, &logm_info, NULL
};

struct streamtab loginfo = { &logrinit, &logwinit, NULL, NULL };

STATIC int log_errseq, log_trcseq, log_conseq;	/* logger sequence numbers */

STATIC int numlogtrc;		/* number of processes reading trace log */
STATIC int numlogerr;		/* number of processes reading error log */
STATIC int numlogcons;		/* number of processes reading console log */

int logdevflag = 0;		/* new driver interface */
int conslogging = 0;		/* set when someone is logging console output */

int putbufrpos = 0;		/* next byte to read in system putbuf */
int putbufwpos = 0;		/* next byte to write in system putbuf */

extern char putbuf[];		/* system putchar circular buffer */
extern int putbufsz;		/* size of above */

/*
 * Initialization function - called during system initialization to 
 * initaialize global variables and data structures.
 */
void
loginit()
{
	register int i;

	numlogtrc = 0;
	numlogerr = 0;
	numlogcons = 0;
	log_errseq = 0;
	log_trcseq = 0;
	log_conseq = 0;
	for (i = 0; i < log_cnt; i++)
		log_log[i].log_state = 0;
}

/*
 * Log driver open routine.  Only two ways to get here.  Normal
 * access for loggers is through the clone minor.  Only one user
 * per clone minor.  Access to writing to the console log is
 * through the console minor.  Users can't read from this device.
 * Any number of users can have it open at one time.
 */
/* ARGSUSED */
STATIC int
logopen(q, devp, flag, sflag, cr)
	queue_t *q;
	dev_t *devp;
	int flag;
	int sflag;
	cred_t *cr;
{
	int i;
	struct log *lp;

	if (sflag)
		return(ENXIO);

	switch (getminor(*devp)) {

	case CONSWMIN:
		if (flag & FREAD)	/* you can only write to this minor */
			return(EINVAL);
		if (q->q_ptr)		/* already open */
			return(0);
		lp = &log_log[CONSWMIN];
		break;

	case CLONEMIN:
		/*
		 * Find an unused minor > CLONEMIN.
		 */
		i = CLONEMIN + 1;
		for (lp = &log_log[i]; i < log_cnt; i++, lp++) {
			if (!(lp->log_state & LOGOPEN))
				break;
		}
		if (i >= log_cnt)
			return(ENXIO);
		*devp = makedevice(getemajor(*devp), i);	/* clone it */
		break;

	default:
		return(ENXIO);
	}

	/*
	 * Finish device initialization.
	 */
	lp->log_state = LOGOPEN;
	lp->log_rdq = q;
	q->q_ptr = (caddr_t)lp;
	WR(q)->q_ptr = (caddr_t)lp;
	return(0);
}

/*
 * Log driver close routine.
 */
/* ARGSUSED */
STATIC int
logclose(q, flag, cr)
	queue_t *q;
	int flag;
	cred_t *cr;
{
	struct log *lp;

	ASSERT(q->q_ptr);

	lp = (struct log *)q->q_ptr;
	if (lp->log_state & LOGTRC) {
		freemsg(lp->log_tracemp);
		lp->log_tracemp = NULL;
		numlogtrc--;
	}
	if (lp->log_state & LOGERR)
		numlogerr--;
	if (lp->log_state & LOGCONS) {
		numlogcons--;
		if (numlogcons == 0)
			conslogging = 0;
	}
	lp->log_state = 0;
	lp->log_rdq = NULL;
	flushq(q, FLUSHALL);
	flushq(WR(q), FLUSHALL);
	q->q_ptr = NULL;
	WR(q)->q_ptr = NULL;
	return(0);
}

/*
 * Write queue put procedure.  
 */
STATIC int
logwput(q, bp)
	queue_t *q;
	mblk_t *bp;
{
	register s;
	register struct iocblk *iocp;
	register struct log *lp;
	struct log_ctl *lcp;
	mblk_t *cbp, *pbp;
	int size;

	lp = (struct log *)q->q_ptr;
	switch (bp->b_datap->db_type) {
	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW) {
			flushq(q, FLUSHALL);
			*bp->b_rptr &= ~FLUSHW;
		}
		if (*bp->b_rptr & FLUSHR) {
			flushq(RD(q), FLUSHALL);
			qreply(q, bp);
		} else {
			freemsg(bp);
		}
		break;

	case M_IOCTL:
		if (lp == &log_log[CONSWMIN])	/* can not ioctl CONSWMIN */
			goto lognak;
		iocp = (struct iocblk *)bp->b_rptr;
		if (iocp->ioc_count == TRANSPARENT)
			goto lognak;
		switch (iocp->ioc_cmd) {

		case I_CONSLOG:
			if (lp->log_state & LOGCONS) {
				iocp->ioc_error = EBUSY;
				goto lognak;
			}
			numlogcons++;
			lp->log_state |= LOGCONS;
			if (putbufrpos == putbufwpos) {
				conslogging = 1;
				goto logack;
			}
			if (!(cbp = allocb(sizeof(struct log_ctl), BPRI_HI))) {
				iocp->ioc_error = EAGAIN;
				lp->log_state &= ~LOGCONS;
				numlogcons--;
				goto lognak;
			}
			s = splhi();
			if (putbufwpos > putbufrpos)
				size = putbufwpos - putbufrpos;
			else
				size = putbufsz - putbufrpos + putbufwpos;
			if (!(pbp = allocb(size, BPRI_HI))) {
				splx(s);
				freeb(cbp);
				iocp->ioc_error = EAGAIN;
				lp->log_state &= ~LOGCONS;
				numlogcons--;
				goto lognak;
			}
			cbp->b_datap->db_type = M_PROTO;
			cbp->b_cont = pbp;
			cbp->b_wptr += sizeof(struct log_ctl);
			lcp = (struct log_ctl *)cbp->b_rptr;
			lcp->mid = LOG_MID;
			lcp->sid = lp - log_log;
			(void) drv_getparm(LBOLT, &lcp->ltime);
			(void) drv_getparm(TIME, &lcp->ttime);
			lcp->level = 0;
			lcp->flags = SL_CONSOLE;
			lcp->seq_no = log_conseq;
			lcp->pri = LOG_KERN|LOG_INFO; /* XXX */
			while (putbufrpos != putbufwpos) {
				*pbp->b_wptr++ = putbuf[putbufrpos++];
				if (putbufrpos >= putbufsz)
					putbufrpos = 0;
			}
			splx(s);
			conslogging = 1;
			s = CLONEMIN + 1;
			for (lp = &log_log[s]; s < log_cnt; s++, lp++)
				if (lp->log_state & LOGCONS)
					(void) sendmsg(lp, cbp);
			freemsg(cbp);
			goto logack;

		case I_TRCLOG:
			if (!(lp->log_state & LOGTRC) && bp->b_cont) {
				lp->log_tracemp = bp->b_cont;
				bp->b_cont = NULL;
				numlogtrc++;
				lp->log_state |= LOGTRC;
				goto logack;
			}
			iocp->ioc_error = EBUSY;
			goto lognak;

		case I_ERRLOG:
			if (!(lp->log_state & LOGERR)) {
				numlogerr++;
				lp->log_state |= LOGERR;
logack:
				iocp->ioc_count = 0;
				bp->b_datap->db_type = M_IOCACK;
				qreply(q,bp);
				break;
			}
			iocp->ioc_error = EBUSY;
			goto lognak;

		default:
lognak:
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			break;
		}
		break;

	case M_PROTO:
		if (((bp->b_wptr - bp->b_rptr) != sizeof(struct log_ctl)) ||
		    !bp->b_cont) {
			freemsg(bp);
			break;
		}
		lcp = (struct log_ctl *)bp->b_rptr;
		if (lcp->flags & SL_ERROR) {
			if (numlogerr == 0) {
				lcp->flags &= ~SL_ERROR;
			} else {
				log_errseq++;
			}
		}
		if (lcp->flags & SL_TRACE) {
			if ((numlogtrc == 0) || !shouldtrace(LOG_MID,
			    (struct log *)(q->q_ptr) - log_log, lcp->level)) {
				lcp->flags &= ~SL_TRACE;
			} else {
				log_trcseq++;
			}
		}
		if (!(lcp->flags & (SL_ERROR|SL_TRACE|SL_CONSOLE))) {
			freemsg(bp);
			break;
		}
		(void) drv_getparm(LBOLT, &lcp->ltime);
		(void) drv_getparm(TIME, &lcp->ttime);
		lcp->mid = LOG_MID;
		lcp->sid = (struct log *)q->q_ptr - log_log;
		if (lcp->flags & SL_TRACE)
			(void) logtrc(bp);
		if (lcp->flags & SL_ERROR)
			(void) logerr(bp);
		if (lcp->flags & SL_CONSOLE) {
			log_conseq++;
			if ((lcp->pri & LOG_FACMASK) == LOG_KERN)
				lcp->pri |= LOG_USER;
			(void) logcons(bp);
		}
		freemsg(bp);
		break;

	case M_DATA:
		if (lp != &log_log[CONSWMIN]) {
			bp->b_datap->db_type = M_ERROR;
			if (bp->b_cont) {
				freemsg(bp->b_cont);
				bp->b_cont = NULL;
			}
			bp->b_rptr = bp->b_datap->db_base;
			bp->b_wptr = bp->b_rptr + sizeof(char);
			*bp->b_rptr = EIO;
			qreply(q, bp);
			break;
		}

		/*
		 * allocate message block for proto
		 */
		if (!(cbp = allocb(sizeof(struct log_ctl), BPRI_HI))) {
			freemsg(bp);
			break;
		}
		cbp->b_datap->db_type = M_PROTO;
		cbp->b_cont = bp;
		cbp->b_wptr += sizeof(struct log_ctl);
		lcp = (struct log_ctl *)cbp->b_rptr;
		lcp->mid = LOG_MID;
		lcp->sid = CONSWMIN;
		(void) drv_getparm(LBOLT, &lcp->ltime);
		(void) drv_getparm(TIME, &lcp->ttime);
		lcp->level = 0;
		lcp->flags = SL_CONSOLE;
		lcp->pri = LOG_USER|LOG_INFO;
		log_conseq++;
		(void) logcons(cbp);
		freemsg(cbp);
		break;

	default:
		freemsg(bp);
		break;
	}
	return(0);
}

/*
 * Send a log message up a given log stream.
 */
STATIC int
logrsrv(q)
	queue_t *q;
{
	mblk_t *mp;
	register s;

	s = splstr();
	while (mp = getq(q)) {
		if (!canput(q->q_next)) {
			putbq(q, mp);
			break;
		}
		putnext(q, mp);
	}
	splx(s);
	return(0);
}

/*
 * Kernel logger interface function.  Attempts to construct a log
 * message and send it up the logger stream.  Delivery will not be
 * done if message blocks cannot be allocated or if the logger
 * is not registered (exception is console logger).
 *
 * Returns 0 is a message is not seen by a reader, either because
 * nobody was reading or an allocation failed.  Returns 1 otherwise.
 */

/*PRINTFLIKE5*/
int
#ifdef __STDC__
strlog(short mid, short sid, char level, u_short flags, char *fmt, ...)
#else
strlog(mid, sid, level, flags, fmt, arg)
	short mid, sid;
	char level;
	u_short flags;
	char *fmt;
	unsigned arg;
#endif
{
	register char *dst, *src;
	register int i;
	VA_LIST argp;
	struct log_ctl *lcp;
	int nlog;
	mblk_t *dbp, *cbp;

	ASSERT(flags & (SL_ERROR|SL_TRACE|SL_CONSOLE));
	if (flags & SL_ERROR) {
		if (numlogerr == 0)
			flags &= ~SL_ERROR;
		else
			log_errseq++;
	}
	if (flags & SL_TRACE) {
		if ((numlogtrc == 0) || !shouldtrace(mid, sid, level))
			flags &= ~SL_TRACE;
		else
			log_trcseq++;
	}
	if (!(flags & (SL_ERROR|SL_TRACE|SL_CONSOLE))) {
		return(0);
	}

	/*
	 * allocate message blocks for log text, log header, and 
	 * proto control field.
	 */
	if (!(dbp = allocb(LOGMSGSZ, BPRI_HI)))
		return(0);
	if (!(cbp = allocb(sizeof(struct log_ctl), BPRI_HI))) {
		freeb(dbp);
		return(0);
	}

	/*
	 * copy log text into text message block.  This consists of a 
	 * format string and NLOGARGS integer arguments.
	 */
	dst = (char *)dbp->b_wptr;
	src = fmt;
	logstrcpy(dst, src);

	/*
	 * dst now points to the null byte at the end of the format string.  
	 * Move the wptr to the first int boundary after dst.
	 */
	dbp->b_wptr = (unsigned char *)logadjust(dst);

	ASSERT((int)(dbp->b_datap->db_lim-dbp->b_wptr) >= 
	  NLOGARGS*sizeof(int));

	VA_START(argp, fmt);

	for (i = 0; i < NLOGARGS; i++) {
		*((int *)dbp->b_wptr) = VA_ARG(argp, int);
		dbp->b_wptr += sizeof(int);
	}

	/*
	 * set up proto header
	 */
	cbp->b_datap->db_type = M_PROTO;
	cbp->b_cont = dbp;
	cbp->b_wptr += sizeof(struct log_ctl);
	lcp = (struct log_ctl *)cbp->b_rptr;
	lcp->mid = mid;
	lcp->sid = sid;
	(void) drv_getparm(LBOLT, &lcp->ltime);
	(void) drv_getparm(TIME, &lcp->ttime);
	lcp->level = level;
	lcp->flags = flags;

	nlog = 0;
	i = 0;
	if (lcp->flags & SL_TRACE) {
		nlog++;
		lcp->pri = LOG_KERN|LOG_DEBUG;
		i += logtrc(cbp);
	}
	if (lcp->flags & SL_ERROR) {
		nlog++;
		lcp->pri = LOG_KERN|LOG_ERR;
		i += logerr(cbp);
	}
	if (lcp->flags & SL_CONSOLE) {
		nlog++;
		log_conseq++;
		if (lcp->flags & SL_FATAL)
			lcp->pri = LOG_KERN|LOG_CRIT;
		else if (lcp->flags & SL_ERROR)
			lcp->pri = LOG_KERN|LOG_ERR;
		else if (lcp->flags & SL_WARN)
			lcp->pri = LOG_KERN|LOG_WARNING;
		else if (lcp->flags & SL_NOTE)
			lcp->pri = LOG_KERN|LOG_NOTICE;
		else if (lcp->flags & SL_TRACE)
			lcp->pri = LOG_KERN|LOG_DEBUG;
		else
			lcp->pri = LOG_KERN|LOG_INFO;
		i+= logcons(cbp);
	}
	freemsg(cbp);
	return((i == nlog) ? 1 : 0);
}

/*
 * Check mid, sid, and level against list of values requested by
 * processes reading trace messages.
 */
STATIC int
logtrace(lp, mid, sid, level)
	struct log *lp;
	register short mid, sid;
	register char level;
{
	register struct trace_ids *tid;
	register int i;
	int ntid;

	ASSERT(lp->log_tracemp);
	tid = (struct trace_ids *)lp->log_tracemp->b_rptr;
	ntid = (long)(lp->log_tracemp->b_wptr - lp->log_tracemp->b_rptr) / 
	    sizeof(struct trace_ids);
	for (i = 0; i < ntid; tid++, i++) {
		if ((tid->ti_level < level) && (tid->ti_level >= 0))
			continue;
		if ((tid->ti_mid != mid) && (tid->ti_mid >= 0))
			continue;
		if ((tid->ti_sid != sid) && (tid->ti_sid >= 0))
			continue;
		return(1);
	}
	return(0);
}

/*
 * Returns 1 if someone wants to see the trace message for the
 * given module id, sub-id, and level.  Returns 0 otherwise.
 */
STATIC int
shouldtrace(mid, sid, level)
	register short mid, sid;
	register char level;
{
	register struct log *lp;
	register int i;

	i = CLONEMIN + 1;
	for (lp = &log_log[i]; i < log_cnt; i++, lp++)
		if ((lp->log_state & LOGTRC) && logtrace(lp, mid, sid, level))
			return(1);
	return(0);
}

/*
 * Send a log message to a reader.  Returns 1 if the
 * message was sent and 0 otherwise.
 */
STATIC int
sendmsg(lp, mp)
	register struct log *lp;
	mblk_t *mp;
{
	register int s;
	register mblk_t *bp2, *mp2;

	if (bp2 = copyb(mp)) {
		if (mp2 = dupb(mp->b_cont)) {
			bp2->b_cont = mp2;
			s = splhi();
			while (lp->log_rdq->q_flag & QFULL)
				freemsg(getq(lp->log_rdq));
			putq(lp->log_rdq, bp2);
			splx(s);
			return(1);
		} else {
			freeb(bp2);
		}
	}
	return(0);
}

/*
 * Log a trace message.  Returns 1 if everyone sees the message
 * and 0 otherwise.
 */
STATIC int
logtrc(mp)
	register mblk_t *mp;
{
	register int i;
	register struct log *lp;
	struct log_ctl *lcp;
	int nlog = 0;
	int didsee = 0;

	lcp = (struct log_ctl *)mp->b_rptr;
	lcp->seq_no = log_trcseq;
	i = CLONEMIN + 1;
	for (lp = &log_log[i]; i < log_cnt; i++, lp++)
		if (lp->log_state & LOGTRC) {
			nlog++;
			didsee += sendmsg(lp, mp);
		}

	return((nlog == didsee) ? 1 : 0);
}

/*
 * Log an error message.  Returns 1 if everyone sees the message
 * and 0 otherwise.
 */
STATIC int
logerr(mp)
	register mblk_t *mp;
{
	register int i;
	register struct log *lp;
	struct log_ctl *lcp;
	int nlog = 0;
	int didsee = 0;

	lcp = (struct log_ctl *)mp->b_rptr;
	lcp->seq_no = log_errseq;
	i = CLONEMIN + 1;
	for (lp = &log_log[i]; i < log_cnt; i++, lp++)
		if (lp->log_state & LOGERR) {
			nlog++;
			didsee += sendmsg(lp, mp);
		}

	return((nlog == didsee) ? 1 : 0);
}

/*
 * Log a console message.  Returns 1 if everyone sees the message
 * and 0 otherwise.
 */
STATIC int
logcons(mp)
	register mblk_t *mp;
{
	register int s, i;
	register unsigned char *cp;
	register struct log *lp;
	mblk_t *bp;
	struct log_ctl *lcp;
	int nlog = 0;
	int didsee = 0;

	/*
	 * Assumption that only short messages get logged to the
	 * console.  That's why we don't use bcopy().
	 */
	bp = mp;
	s = splhi();
	for (mp = mp->b_cont; mp; mp = mp->b_cont) {
		cp = mp->b_rptr;
		while (cp < mp->b_wptr) {
			if (++putbufwpos >= putbufsz)
				putbufwpos = 0;
			putbuf[putbufwpos] = *cp++;
		}
	}

	lcp = (struct log_ctl *)bp->b_rptr;
	lcp->seq_no = log_conseq;
	i = CLONEMIN + 1;
	for (lp = &log_log[i]; i < log_cnt; i++, lp++)
		if (lp->log_state & LOGCONS) {
			nlog++;
			didsee += sendmsg(lp, bp);
		}

	if (didsee)
		putbufrpos = putbufwpos;
	splx(s);

	return((nlog == didsee) ? 1 : 0);
}
