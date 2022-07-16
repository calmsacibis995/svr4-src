/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:strsubr.c	1.3.1.5"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/tss.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
#include "sys/stropts.h"
#include "sys/strstat.h"
#include "sys/var.h"
#include "sys/poll.h"
#include "sys/termio.h"
#include "sys/ttold.h"
#include "sys/inline.h"
#include "sys/systm.h"
#include "sys/uio.h"
#include "sys/cmn_err.h"
#include "sys/sad.h"
#include "sys/priocntl.h"
#include "sys/hrtcntl.h"
#include "sys/procset.h"
#include "sys/events.h"
#include "sys/evsys.h"
#include "sys/session.h"
#include "sys/tuneable.h"
#include "sys/map.h"
#include "sys/kmem.h"
#include "sys/siginfo.h"

/* KPERF is for kernel perf. measurment tools */

#ifdef KPERF  
#include "sys/disp.h"
#endif /* KPERF */

/*
 * WARNING:
 * The variables and routines in this file are private, belonging
 * to the STREAMS subsystem.  These should not be used by modules
 * or drivers.  Compatibility will not be guaranteed. 
 */

#define ncopyin(A, B, C, D)	copyin(A, B, C)		/* temporary */
#define ncopyout(A, B, C, D)	copyout(A, B, C)	/* temporary */

/*
 * Id value used to distinguish between different multiplexor links.
 */
STATIC long lnk_id;

/*
 * Queue scheduling control variables.
 */
char qrunflag;			/* set iff queues are enabled */
char queueflag;			/* set iff inside queuerun() */
struct queue *qhead;		/* head of queues to run */
struct queue *qtail;		/*  last queue */
struct queue *scanqhead;	/* list for msg consolidation feature */
struct queue *scanqtail;	/*  last queue */
char strbcwait;			/* bufcall functions waiting */
char strbcflag;			/* bufcall functions ready to go */
struct bclist strbcalls;	/* list of waiting bufcalls */
unsigned char qbf[NBAND];	/* band flushing backenable flags */
int strscanflag;		/* true when strscan timeout is pending */

struct	strstat strst;		/* Streams statistics structure */

/* the following 2 free lists are now referenced elsewhere
 * via macros for improved performance, so they are nolonger STATIC.
 */
struct	msgb		*msgfreelist;
struct	mdbblock	*mdbfreelist;
STATIC struct seinfo *sefreelist;
STATIC struct mux_node *mux_nodes;	/* mux info for cycle checking */
#define SECACHE 10
STATIC struct seinfo Secache[SECACHE];	/* emergency cache of seinfo's	*/
					/* in case memory runs out	*/
STATIC struct seinfo *secachep;		/* cache list head		*/
struct strinfo Strinfo[NDYNAMIC];	/* dynamic resource info	*/
long Strcount;				/* count of streams resources	*/
					/* in bytes			*/
extern long strthresh;			/* threshold for stopping some	*/
					/* streams operations		*/

extern struct qinit strdata;
extern struct qinit stwdata;
extern ulong kmem_avail();
void strgiveback();

/*
 * Init routine run from main at boot time.  This contains some 
 * machine dependent code.  spl's probably are not necessary, but
 * they don't cost anything here and they can't hurt.
 */
void
strinit()
{
	register i;
	register int s;

	s = splstr();
	timeout(strgiveback, 0, 300 * HZ);

	/*
	 * Set up the freelists.
	 */
	mdbfreelist = NULL;
	msgfreelist = NULL;

	/*
	 * set up seinfo cache (if memory runs out, we need a few of
	 * these so things have a chance to recover)
	 */
	sefreelist = NULL;
	for (i = 0; i < SECACHE; ++i) {
		Secache[i].s_next = secachep;
		secachep = &Secache[i];
	}

	/*
	 * Set up mux_node structures.
	 */
	if ((mux_nodes =
	    (struct mux_node *)kmem_alloc((sizeof(struct mux_node) * cdevcnt),
	    KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC, "Could not allocate space for mux_nodes\n");
	for (i = 0; i < cdevcnt; i++) {
		mux_nodes[i].mn_imaj = i;
		mux_nodes[i].mn_indegree = 0;
		mux_nodes[i].mn_originp = NULL;
		mux_nodes[i].mn_startp = NULL;
		mux_nodes[i].mn_outp = NULL;
		mux_nodes[i].mn_flags = 0;
	}

	splx(s);
	return;
}


/*
 * see about giving back streams resources
 */

void
strgiveback()
{
	static int mdbavguse = 0;
	static int msgavguse = 0;
	register int mdbfree;
	register int msgfree;
	register int n, j;
	register struct msgb *msgp;
	register struct mdbblock *mdbp;

	mdbavguse = (mdbavguse + strst.mdbblock.use) >> 1;
	msgavguse = (msgavguse + strst.msgblock.use) >> 1;
	mdbfree	= Strinfo[DYN_MDBBLOCK].sd_cnt - strst.mdbblock.use;
	msgfree = Strinfo[DYN_MSGBLOCK].sd_cnt - strst.msgblock.use;
	if (mdbfree > mdbavguse)  {
		/* give back 1/8 of free list (easy to calculate) */
		n = mdbfree >> 3;
		for (j = 0; j < n; ++j) {
			mdbp = mdbfreelist;
			mdbfreelist = 
		  	   (struct mdbblock *)( mdbp->msgblk.m_mblock.b_next);
			kmem_free(mdbp, sizeof(struct mdbblock));
			Strinfo[DYN_MDBBLOCK].sd_cnt--;
			Strcount -= sizeof(struct mdbblock);
		}
	}
	if (msgfree > msgavguse) {
		/* give back 1/8 of free list (easy to calculate) */
		n = msgfree >> 3;
		for (j = 0; j < n; ++j) {
			msgp = msgfreelist;
			msgfreelist = msgp->b_next;
			kmem_free(msgp, sizeof(struct mbinfo));
			Strinfo[DYN_MSGBLOCK].sd_cnt--;
			Strcount -= sizeof(struct mbinfo);
		}
	}
	timeout(strgiveback, 0, 60 * HZ);
}




/*
 * Send SIGPOLL signal to all processes registered on the given signal
 * list that want a signal for the specified event.  Always called at
 * splstr().
 */
void
strsendsig(siglist, event, data)
	register struct strevent *siglist;
	register event;
	long data;
{
	struct strevent *sep;
	k_siginfo_t info;

	info.si_signo = SIGPOLL;
	info.si_errno = 0;
	for (sep = siglist; sep; sep = sep->se_next) {
		if (sep->se_events & event) {
			switch (event) {
			case S_INPUT:
				info.si_code = POLL_IN;
				info.si_band = data;
				goto addq;

			case S_OUTPUT:
				info.si_code = POLL_OUT;
				info.si_band = data;
				goto addq;

			case S_HIPRI:
				info.si_code = POLL_PRI;
				info.si_band = 0;
				goto addq;

			case S_MSG:
				info.si_code = POLL_MSG;
				info.si_band = data;
				goto addq;

			case S_ERROR:
				info.si_code = POLL_ERR;
				info.si_band = 0;
				info.si_errno = (int)data;
				goto addq;

			case S_HANGUP:
				info.si_code = POLL_HUP;
				info.si_band = 0;
addq:
				sigaddq(sep->se_procp, &info, KM_NOSLEEP);

			case S_RDBAND:
				if (sep->se_events & S_BANDURG) {
					psignal(sep->se_procp, SIGURG);
					break;
				}
				/* Fall through */

			case S_RDNORM:
			case S_WRBAND:
				psignal(sep->se_procp, SIGPOLL);
				break;

			default:
				cmn_err(CE_PANIC,
				    "strsendsig: unknown event %x\n", event);
			}
		}
	}
}

/*
 * Post the given ET_STREAM type event to the event queues
 * on sd_eventlist.  The caller must splstr() before calling
 * this routine.
 */
void
strevpost(stp, event, data)
	struct stdata *stp;
	register int event;
	long data;
{
	register struct strevent *sep;
	extern ev_stream_post();

	for (sep = stp->sd_eventlist; sep; sep = sep->se_next) {
		if (sep->se_kmask & event) {
			if ((event & (S_INPUT|S_OUTPUT|S_MSG)) &&
			    (sep->se_kband != (unchar)data))
				continue;
			(void) ev_stream_post(sep->se_vp, &sep->se_kecb,
			    sep->se_pid, sep->se_hostid, sep->se_uid, event, data);
		}
	}
}

/*
 * Post the given ET_DRIVER type event to the given event
 * queue.  mp is an M_EVENT/M_PCEVENT message block pointer.
 */
void
strdrpost(mp)
	mblk_t *mp;
{
	evdr_t drev;
	register struct str_evmsg *ep;

	ASSERT((mp->b_datap->db_type == M_EVENT) || (mp->b_datap->db_type = M_PCEVENT));
	ep = (struct str_evmsg *)mp->b_rptr;
	drev.evdr_flags = ep->sv_flags;
	drev.evdr_eid = ep->sv_eid;
	drev.evdr_pri = ep->sv_evpri;
	drev.evdr_hostid = ep->sv_hostid;
	drev.evdr_pid = ep->sv_pid;
	drev.evdr_uid = ep->sv_uid;
	if (mp->b_cont) {
		drev.evdr_flags &= ~EF_QUICKD;
		drev.evdr_datasize = mp->b_cont->b_wptr - mp->b_cont->b_rptr;
		drev.evdr_data = (char *)mp->b_cont->b_rptr;
	} else {
		drev.evdr_flags |= EF_QUICKD;
		drev.evdr_datasize = EV_MAXQD;
		drev.evdr_data = (char *)ep->sv_event;
	}
	ev_dr_post(ep->sv_vp, &drev);
}

/*
 * Attach a stream device or module.
 * qp is a read queue; the new queue goes in so its next
 * read ptr is the argument, and the write queue corresponding
 * to the argument points to this queue.  Return 0 on success,
 * or a non-zero errno on failure.
 */
int
qattach(qp, devp, flag, table, idx, crp)
	register queue_t *qp;
	dev_t *devp;
	int flag;
	int table;
	int idx;
	cred_t *crp;
{
	register queue_t *rq;
	register s;
	struct streamtab *qinfop;
	dev_t odev;
	int sflg;
	int error = 0;

	if ((rq = allocq()) == NULL)
		return (ENXIO);
	odev = *devp;
	if (table == CDEVSW) {
		if (*cdevsw[idx].d_flag & D_OLD) {
			rq->q_flag |= QOLD;
			WR(rq)->q_flag |= QOLD;
		}
		qinfop = cdevsw[idx].d_str;
		sflg = 0;
	} else {
		ASSERT(table == FMODSW);
		if (*fmodsw[idx].f_flag & D_OLD) {
			rq->q_flag |= QOLD;
			WR(rq)->q_flag |= QOLD;
		}
		qinfop = fmodsw[idx].f_str;
		sflg = MODOPEN;
	}

	s = splstr();
	rq->q_next = qp;
	WR(rq)->q_next = WR(qp)->q_next;
	if (WR(qp)->q_next)
		OTHERQ(WR(qp)->q_next)->q_next = rq;
	WR(qp)->q_next = WR(rq);
	setq(rq, qinfop->st_rdinit, qinfop->st_wrinit);
 	rq->q_flag |= QWANTR;
	WR(rq)->q_flag |= QWANTR;

	/*
	 * Open the attached module or driver.
	 * The open may sleep, but it must always return here.  Therefore
	 * all sleeps must set PCATCH or ignore all signals to avoid a 
	 * longjmp if a signal arrives.
	 */
	if (rq->q_flag & QOLD) {
		extern void gen_setup_idinfo();

		gen_setup_idinfo(crp);
		if ((cmpdev(odev) == NODEV) || ((*rq->q_qinfo->qi_qopen)(rq, cmpdev(odev), flag, sflg) == OPENFAIL)) {
			qdetach(rq, 0, 0, crp);
			splx(s);
			if ((error = u.u_error) == 0)		/* XXX */
				error = ENXIO;
			return (error);
		}
	} else {
		if (error = (*rq->q_qinfo->qi_qopen)(rq, devp, flag, sflg, crp)) {
			qdetach(rq, 0, 0, crp);
			splx(s);
			return (error);
		}
		idx = getmajor(*devp);
		if ((table == CDEVSW) && (idx != getmajor(odev))) {
			if (idx >= cdevcnt) {
				qdetach(rq, 0, 0, crp);
				splx(s);
				return (ENXIO);
			}
			qinfop = cdevsw[idx].d_str;
			setq(rq, qinfop->st_rdinit, qinfop->st_wrinit);
		}
	}
	splx(s);
	return (0);
}

/*
 * Detach a stream module or device.
 * If clmode == 1 then the module or driver was opened and its
 * close routine must be called.  If clmode == 0, the module
 * or driver was never opened or the <open failed, and so its close
 * should not be called.
 */
void
qdetach(qp, clmode, flag, crp)
	register queue_t *qp;
	int clmode;
	int flag;
	cred_t *crp;
{
	register s;
	register queue_t *q;
	register queue_t *prev = NULL;

	if (clmode) {
		if (qready())
			runqueues();
		s = splstr();
		if (qp->q_flag & QOLD) {
			extern void gen_setup_idinfo();
			gen_setup_idinfo(crp);
			(*qp->q_qinfo->qi_qclose)(qp, flag);
		} else {
			(*qp->q_qinfo->qi_qclose)(qp, flag, crp);
		}

		/*
		 * Check if queues are still enabled, and remove from 
		 * runlist if necessary.
		 */
		if ((qp->q_flag | WR(qp)->q_flag) & QENAB) {
			for (q = qhead; q; q = q->q_link)  {
				if (q == qp || q == WR(qp)) {
					if (prev)
						prev->q_link = q->q_link;
					else
						qhead = q->q_link;
					if (q == qtail)
						qtail = prev;
				} else
					prev = q;
			}
		}
		flushq(qp, FLUSHALL);
		flushq(WR(qp), FLUSHALL);
	} else 
		s = splstr();

	if (WR(qp)->q_next)
		backq(qp)->q_next = qp->q_next;
	if (qp->q_next)
		backq(WR(qp))->q_next = WR(qp)->q_next;
	freeq(qp);
	splx(s);
}

/*
 * This function is placed in the callout table when messages have been
 * held in strwrite, in case the hope of being able to add more data is
 * unrealized.
 */
void
strscan()  {
	setqsched();
	strscanflag = 0;
}

/*
 * This function is placed in the callout table to wake up a process
 * waiting to close a stream that has not completely drained.
 */
void
strtime(stp)
	struct stdata *stp;
{
	if (stp->sd_flag & STRTIME) {
		wakeprocs((caddr_t)stp->sd_wrq, PRMPT);
		stp->sd_flag &= ~STRTIME;
	}
}

/*
 * This function is placed in the callout table to wake up all
 * processes waiting to send an ioctl down a particular stream,
 * as well as the process whose ioctl is still outstanding.  The
 * process placing this function in the callout table will remove
 * it if he gets control of the ioctl mechanism for the stream -
 * this should only run if there is a failure.  This wakes up
 * the same processes as str3time below.
 */
void
str2time(stp)
	struct stdata *stp;
{
	if (stp->sd_flag & STR2TIME) {
		wakeprocs((caddr_t)&stp->sd_iocwait, PRMPT);
		stp->sd_flag &= ~STR2TIME;
	}
}

/*
 * This function is placed in the callout table to wake up the
 * process that has an outstanding ioctl waiting acknowledgement
 * on a stream, as well as any processes waiting to send their
 * own ioctl messages.  It should be removed from the callout table
 * when the acknowledgement arrives.  If this function runs, it
 * is the result of a failure.  This wakes up the same processes
 * as str2time above.
 */
void
str3time(stp)
	struct stdata *stp;
{
	if (stp->sd_flag & STR3TIME) {
		wakeprocs((caddr_t)stp, PRMPT);
		stp->sd_flag &= ~STR3TIME;
	}
}

/*
 * Put ioctl data from user land to ioctl buffers.  Return non-zero
 * errno for failure, 1 for success.
 */
int
putiocd(bp, ebp, arg, copymode, flag, fmt)
	register mblk_t *bp;
	mblk_t *ebp;
	caddr_t arg;
	int copymode;
	int flag;
	char *fmt;
{
	register mblk_t *tmp;
	register int count, n;
	mblk_t *obp = bp;
	int error = 0;

	if (bp->b_datap->db_type == M_IOCTL)
		count = ((struct iocblk *)bp->b_rptr)->ioc_count;
	else {
		ASSERT(bp->b_datap->db_type == M_COPYIN);
		count = ((struct copyreq *)bp->b_rptr)->cq_size;
	}
	/*
	 * strdoioctl validates ioc_count, so if this assert fails it
	 * cannot be due to user error.
	 */
	ASSERT(count >= 0);

	while (count) {
		n = MIN(MAXIOCBSZ, count);
		if (flag == SE_SLEEP) {
			while (!(tmp = allocb(n, BPRI_HI))) {
				if (error = strwaitbuf(n, BPRI_HI))
					return (error);
			}
		} else if (!(tmp = allocb(n, BPRI_HI)))
			return (EAGAIN);
		error = strcopyin((char *)arg, tmp->b_wptr, n, fmt, copymode);
		if (error) {
			freeb(tmp);
			return (error);
		}
		if (fmt && (count > MAXIOCBSZ) && (copymode == U_TO_K))
			adjfmtp(&fmt, tmp, n);
		arg += n;
		tmp->b_datap->db_type = M_DATA;
		tmp->b_wptr += n;
		count -= n;
		bp = (bp->b_cont = tmp);
	}

	/*
	 * If ebp was supplied, place it between the
	 * M_IOCTL block and the (optional) M_DATA blocks.
	 */
	if (ebp) {
		ebp->b_cont = obp->b_cont;
		obp->b_cont = ebp;
	}
	return (0);
}

/*
 * Copy ioctl data to user-land.  Return non-zero errno on failure,
 * 0 for success.
 */
int
getiocd(bp, arg, copymode, fmt)
	register mblk_t *bp;
	caddr_t arg;
	int copymode;
	char *fmt;
{
	register int count, n;
	int error;

	if (bp->b_datap->db_type == M_IOCACK)
		count = ((struct iocblk *)bp->b_rptr)->ioc_count;
	else {
		ASSERT(bp->b_datap->db_type == M_COPYOUT);
		count = ((struct copyreq *)bp->b_rptr)->cq_size;
	}
	ASSERT(count >= 0);

	for (bp = bp->b_cont; bp && count; count -= n, bp = bp->b_cont, arg += n) {
		n = MIN(count, bp->b_wptr - bp->b_rptr);
		error = strcopyout(bp->b_rptr, arg, n, fmt, copymode);
		if (error)
			return (error);
		if (fmt && bp->b_cont && (copymode == U_TO_K))
			adjfmtp(&fmt, bp, n);
	}
	ASSERT(count == 0);
	return (0);
}

/* 
 * Allocate a linkinfo table entry given the write queue of the
 * bottom module of the top stream and the write queue of the
 * stream head of the bottom stream.
 *
 * linkinfo table entries are freed by nulling the li_lblk.l_qbot field.
 */
struct linkinfo *
alloclink(qup, qdown, fpdown)
	queue_t *qup, *qdown;
	file_t *fpdown;
{
	register struct linkinfo *linkp, *lp;
	register s;

	s = splstr();
	if ((linkp = (struct linkinfo *) kmem_zalloc(sizeof(struct linkinfo), SE_NOSLP)) == NULL) {
		strst.linkblk.fail++;
		splx(s);
		return(NULL);
	}
	linkp->li_lblk.l_qtop = qup;
	linkp->li_lblk.l_qbot = qdown;

	/*
	 * Assign link id, being careful to watch out
	 * for wrap-around leading to id clashes.  Unlike
	 * ioctl ids, link ids can be long-lived.
	 */
	do {
		linkp->li_lblk.l_index = ++lnk_id;
		if (lnk_id == 0)
			linkp->li_lblk.l_index = ++lnk_id;
		for (lp = Strinfo[DYN_LINKBLK].sd_head; lp; lp = lp->li_next)
			if (lp->li_lblk.l_index == lnk_id)
				break;
	} while (lp);

	linkp->li_fpdown = fpdown;
	BUMPUP(strst.linkblk);
	Strinfo[DYN_LINKBLK].sd_cnt++;
	Strcount += sizeof(struct linkinfo);
	linkp->li_next = (struct linkinfo *) Strinfo[DYN_LINKBLK].sd_head;
	if (linkp->li_next)
		linkp->li_next->li_prev = linkp;
	linkp->li_prev = NULL;
	Strinfo[DYN_LINKBLK].sd_head = (void *) linkp;
	splx(s);
	return(linkp);
}

/*
 * Free a linkinfo entry.
 */
void
lbfree(linkp)
	register struct linkinfo *linkp;
{
	register s;

	s = splstr();
	if (linkp->li_prev == NULL) {
		if (linkp->li_next)
			linkp->li_next->li_prev = NULL;
		Strinfo[DYN_LINKBLK].sd_head = (void *) linkp->li_next;
	}
	else {
		if (linkp->li_next)
			linkp->li_next->li_prev = linkp->li_prev;
		linkp->li_prev->li_next = linkp->li_next;
	}
	kmem_free(linkp, sizeof(struct linkinfo));
	Strinfo[DYN_LINKBLK].sd_cnt--;
	Strcount -= sizeof(struct linkinfo);
	strst.linkblk.use--;
	splx(s);
	return;
}

/*
 * Check for a potential linking cycle.
 * Return 1 if a link will result in a cycle,
 * and 0 otherwise.
 */
int
linkcycle(upstp, lostp)
	stdata_t *upstp;	/* upper stream head */
	stdata_t *lostp;	/* lower stream head */
{
	register struct mux_node *np;
	register struct mux_edge *ep;
	register int i;
	long lomaj;
	long upmaj;
	/*
	 * if the lower stream is a pipe/FIFO, return, since link
	 * cycles can not happen on pipes/FIFOs
	 */
	if (lostp->sd_vnode->v_type == VFIFO)
		return(0);

	for (i = 0; i < cdevcnt; i++) {
		np = &mux_nodes[i];
		MUX_CLEAR(np);
	}
	lomaj = getmajor(lostp->sd_vnode->v_rdev);
	upmaj = getmajor(upstp->sd_vnode->v_rdev);
	np = &mux_nodes[lomaj];
	for ( ; ; ) {
		if (!MUX_DIDVISIT(np)) {
			if (np->mn_imaj == upmaj)
				return (1);
			if (np->mn_outp == NULL) {
				MUX_VISIT(np);
				if (np->mn_originp == NULL)
					return (0);
				np = np->mn_originp;
				continue;
			}
			MUX_VISIT(np);
			np->mn_startp = np->mn_outp;
		} else {
			if (np->mn_startp == NULL) {
				if (np->mn_originp == NULL)
					return (0);
				else {
					np = np->mn_originp;
					continue;
				}
			}
			ep = np->mn_startp;
			np->mn_startp = ep->me_nextp;
			ep->me_nodep->mn_originp = np;
			np = ep->me_nodep;
		}
	}
}

/* 
 * Find linkinfo table entry corresponding to the parameters.
 */
struct linkinfo *
findlinks(stp, index, type)
	stdata_t *stp;
	int index;
	int type;
{
	register struct linkinfo *linkp;
	register struct mux_edge *mep;
	struct mux_node *mnp;
	queue_t *qup;

	if ((type & LINKTYPEMASK) == LINKNORMAL) {
		qup = getendq(stp->sd_wrq);
		for (linkp = (struct linkinfo *) Strinfo[DYN_LINKBLK].sd_head; linkp; linkp = linkp->li_next) {
			if ((qup == linkp->li_lblk.l_qtop) &&
		    	    (!index || (index == linkp->li_lblk.l_index)))
				return (linkp);
		}
	} else {
		ASSERT((type & LINKTYPEMASK) == LINKPERSIST);
		mnp = &mux_nodes[getmajor(stp->sd_vnode->v_rdev)];
		mep = mnp->mn_outp;
		while (mep) {
			if ((index == 0) || (index == mep->me_muxid))
				break;
			mep = mep->me_nextp;
		}
		if (!mep)
			return (NULL);
		for (linkp = (struct linkinfo *) Strinfo[DYN_LINKBLK].sd_head; linkp; linkp = linkp->li_next) {
			if ( (!linkp->li_lblk.l_qtop) &&
			    (mep->me_muxid == linkp->li_lblk.l_index))
				return (linkp);
		}
	}
	return (NULL);
}

/* 
 * Given a queue ptr, follow the chain of q_next pointers until you reach the
 * last queue on the chain and return it.
 */
queue_t *
getendq(q)
	register queue_t *q;
{
	ASSERT( q!= NULL);
	while (SAMESTR(q))
		q = q->q_next;
	return (q);
}

/*
 * Unlink a multiplexor link.  Stp is the controlling stream for the
 * link, fpdown is the file pointer for the lower stream, and
 * linkp points to the link's entry in the linkinfo table.
 */
int
munlink(stp, linkp, flag, crp, rvalp)
	struct stdata *stp;
	struct linkinfo *linkp;
	int flag;
	cred_t *crp;
	int *rvalp;
{
	register int s;
	struct strioctl strioc;
	struct stdata *stpdown;
	queue_t *rq;
	int error = 0;

	if ((flag & LINKTYPEMASK) == LINKNORMAL)
		strioc.ic_cmd = I_UNLINK;
	else
		strioc.ic_cmd = I_PUNLINK;
	strioc.ic_timout = 0;
	strioc.ic_len = sizeof(struct linkblk);
	strioc.ic_dp = (char *)&linkp->li_lblk;
	
	error = strdoioctl(stp, &strioc, NULL, K_TO_K, STRLINK, crp, rvalp);

	/*
	 * If there was an error and this is not called via strclose, 
	 * return to the user.  Otherwise, pretend there was no error 
	 * and close the link.  
	 */
	if (error) {
		if (flag & LINKCLOSE) {
			cmn_err(CE_CONT, "KERNEL: munlink: could not perform unlink ioctl, closing anyway\n");
			s = splstr();
			stp->sd_flag &= ~(STRDERR|STWRERR); /* allows strdoioctl() to work */
			splx(s);
		} else
			return (error);
	}

	stpdown = linkp->li_fpdown->f_vnode->v_stream;
	s = splstr();
	stpdown->sd_flag &= ~STPLEX;
	mux_rmvedge(stp, linkp->li_lblk.l_index);
	splx(s);
	rq = RD(stpdown->sd_wrq);
	setq(rq, &strdata, &stwdata);
	rq->q_ptr = WR(rq)->q_ptr = (caddr_t)stpdown;
	closef(linkp->li_fpdown);
	lbfree(linkp);
	return (0);
}

/*
 * Unlink all multiplexor links for which stp is the controlling stream.
 * Return 0, or a non-zero errno on failure.
 */
int
munlinkall(stp, flag, crp, rvalp)
	struct stdata *stp;
	int flag;
	cred_t *crp;
	int *rvalp;
{
	struct linkinfo *linkp;
	queue_t *qup;
	int error = 0;

	while (linkp = findlinks(stp, 0, flag)) {
		if (error = munlink(stp, linkp, flag, crp, rvalp))
			return (error);
	}
	return (0);
}

/*
 * A multiplexor link has been made.  Add an
 * edge to the directed graph.  Returns 0 on success
 * and an error number on failure.
 */
int
mux_addedge(upstp, lostp, muxid)
	stdata_t *upstp;	/* the upper stream */
	stdata_t *lostp;	/* the lower stream */
	int muxid;		/* index for link */
{
	register struct mux_node *np;
	register struct mux_edge *ep;
	long upmaj;
	long lomaj;

	upmaj = getmajor(upstp->sd_vnode->v_rdev);
	lomaj = getmajor(lostp->sd_vnode->v_rdev);
	np = &mux_nodes[upmaj];
	if (np->mn_outp) {
		ep = np->mn_outp;
		while (ep->me_nextp)
			ep = ep->me_nextp;
		if ((ep->me_nextp = (struct mux_edge *)kmem_alloc(sizeof(struct mux_edge), 0)) == NULL) {
			cmn_err(CE_WARN, "Can not allocate memory for mux_edge\n");
			return (EAGAIN);
		}
		ep = ep->me_nextp;
	} else {
		if ((np->mn_outp = (struct mux_edge *)kmem_alloc(sizeof(struct mux_edge), 0)) == NULL) {
			cmn_err(CE_WARN, "Can not allocate memory for mux_edge\n");
			return (EAGAIN);
		}
		ep = np->mn_outp;
	}
	ep->me_nextp = NULL;
	ep->me_muxid = muxid;
	ep->me_nodep = &mux_nodes[lomaj];
	return (0);
}

/*
 * A multiplexor link has been removed.  Remove the
 * edge in the directed graph.
 */
void
mux_rmvedge(upstp, muxid)
	stdata_t *upstp;	/* the upper stream */
	int muxid;		/* the link id */
{
	register struct mux_node *np;
	register struct mux_edge *ep;
	register struct mux_edge *pep = NULL;
	long upmaj;

	upmaj = getmajor(upstp->sd_vnode->v_rdev);
	np = &mux_nodes[upmaj];
	ASSERT (np->mn_outp != NULL);
	ep = np->mn_outp;
	while (ep) {
		if (ep->me_muxid == muxid) {
			if (pep)
				pep->me_nextp = ep->me_nextp;
			else
				np->mn_outp = ep->me_nextp;
			kmem_free(ep, sizeof(struct mux_edge));
			return;
		}
		pep = ep;
		ep = ep->me_nextp;
	}
	ASSERT(0);	/* should not reach here */
}

/*
 * Set the interface values for a pair of queues (qinit structure,
 * packet sizes, water marks).
 */
void
setq(rq, rinit, winit)
	queue_t *rq;
	struct qinit *rinit, *winit;
{
	register queue_t  *wq;
	register int s;

	wq = WR(rq);
	s = splstr();
	rq->q_qinfo = rinit;
	rq->q_hiwat = rinit->qi_minfo->mi_hiwat;
	rq->q_lowat = rinit->qi_minfo->mi_lowat;
	rq->q_minpsz = rinit->qi_minfo->mi_minpsz;
	rq->q_maxpsz = rinit->qi_minfo->mi_maxpsz;
	wq->q_qinfo = winit;
	wq->q_hiwat = winit->qi_minfo->mi_hiwat;
	wq->q_lowat = winit->qi_minfo->mi_lowat;
	wq->q_minpsz = winit->qi_minfo->mi_minpsz;
	wq->q_maxpsz = winit->qi_minfo->mi_maxpsz;
	splx(s);
}

/*
 * Make a protocol message given control and data buffers.
 */
int
strmakemsg(mctl, count, uiop, stp, flag, mpp)
	register struct strbuf *mctl;
	int count;
	struct uio *uiop;
	struct stdata *stp;
	long flag;
	mblk_t **mpp;
{
	register mblk_t *mp = NULL;
	register mblk_t *bp;
	caddr_t base;
	int pri;
	int msgtype;
	int wroff = (int)stp->sd_wroff;
	int offlg = 0;
	int error = 0;

	*mpp = NULL;
	if (flag & RS_HIPRI)
		pri = BPRI_MED;
	else
		pri = BPRI_LO;

	/*
	 * Create control part of message, if any.
	 */
	if ((mctl != NULL) && (mctl->len >= 0)) {
		register int ctlcount;
		int allocsz;

		if (flag & RS_HIPRI) 
			msgtype = M_PCPROTO;
		else 
			msgtype = M_PROTO;

		ctlcount = mctl->len;
		base = mctl->buf;

		/*
		 * Give modules a better chance to reuse M_PROTO/M_PCPROTO
		 * blocks by increasing the size to something more usable.
		 */
		allocsz = MAX(ctlcount, 64);

		/*
		 * Range checking has already been done; simply try
		 * to allocate a message block for the ctl part.
		 */
		while (!(bp = allocb(allocsz, pri))) {
			if (uiop->uio_fmode  & (FNDELAY|FNONBLOCK))
				return (EAGAIN);
			if (error = strwaitbuf(allocsz, pri))
				return (error);
		}

		bp->b_datap->db_type = msgtype;
		if (copyin(base, (caddr_t) bp->b_wptr, ctlcount)) {
			freeb(bp);
			return (EFAULT);
		}

		/*
		 * We could have slept copying in user pages.
		 * Recheck the stream head state (the other end
		 * of a pipe could have gone away).
		 */
		if (stp->sd_flag & (STRHUP|STRDERR|STWRERR|STPLEX)) {
			error = ((stp->sd_flag & STPLEX) ? EINVAL :
			    (stp->sd_werror ? stp->sd_werror : stp->sd_rerror));
			freeb(bp);
			return (error);
		}
		bp->b_wptr += ctlcount;
		mp = bp;
	}

	/*
	 * Create data part of message, if any.
	 */
	if (count >= 0) {
		register int size;

		size = count + (offlg ? 0 : wroff);
		while ((bp = allocb(size, pri)) == NULL) {
			if (uiop->uio_fmode  & (FNDELAY|FNONBLOCK))
				return (EAGAIN);
			if (error = strwaitbuf(size, pri)) {
				freemsg(mp);
				return (error);
			}
		}
		if (wroff && !offlg++ &&
		    (wroff < bp->b_datap->db_lim - bp->b_wptr)) {
			bp->b_rptr += wroff;
			bp->b_wptr += wroff;
		}
		if ((size = MIN(count, bp->b_datap->db_lim - bp->b_wptr)) &&
		    (error = uiomove((caddr_t)bp->b_wptr, size, UIO_WRITE, uiop))) {
			freeb(bp);
			freemsg(mp);
			return (error);
		}

		/*
		 * We could have slept copying in user pages.
		 * Recheck the stream head state (the other end
		 * of a pipe could have gone away).
		 */
		if (stp->sd_flag & (STRHUP|STRDERR|STWRERR|STPLEX)) {
			error = ((stp->sd_flag & STPLEX) ? EINVAL :
			    (stp->sd_werror ? stp->sd_werror : stp->sd_rerror));
			freeb(bp);
			freemsg(mp);
			return (error);
		}
		bp->b_wptr += size;
		count -= size;
		if (!mp)
			mp = bp;
		else
			linkb(mp, bp);

	}
	*mpp = mp;
	return (0);
}

/*
 * Wait for a buffer to become available.  Return non-zero errno
 * if not able to wait, 0 if buffer is probably there.
 */
int
strwaitbuf(size, pri)
	int size;
	int pri;
{
	register int s;

	s = splstr();
	if (!bufcall(size, pri, setrun, u.u_procp)) {
		splx(s);
		return (ENOSR);
	}
	if (sleep((caddr_t)&(u.u_procp->p_flag), STOPRI|PCATCH)) {
		strunbcall(size, u.u_procp);
		splx(s);
		return (EINTR);
	}
	strunbcall(size, u.u_procp);
	splx(s);
	return (0);
}

/*
 * Remove a setrun for the given process from the bufcall list for
 * the given buffer size.  'size' can be -1 for externally-supplied buffers.
 */
void
strunbcall(size, p)
	int size;
	struct proc *p;
{
	register int s;
	register struct strevent *sep, *prevsep;
	
	s = splstr();
	sep = strbcalls.bc_head;
	prevsep = NULL;
	while (sep) {
		if ((sep->se_arg == (long)p) &&
		    (sep->se_func == setrun) &&
		    (sep->se_size == size)) {
			if (prevsep == NULL)
				strbcalls.bc_head = sep->se_next;
			else
				prevsep->se_next = sep->se_next;
			if (sep == strbcalls.bc_tail)
				strbcalls.bc_tail = prevsep;
			splx(s);
			sefree(sep);
			return;
		}
		prevsep = sep;
		sep = sep->se_next;
	}
	splx(s);
}

/*
 * This function waits for a read or write event to happen on a stream.
 */
int
strwaitq(stp, flag, count, fmode, done)
	register struct stdata *stp;
	int flag;
	off_t count;
	int fmode;
	int *done;
{
	register int s;
	int slpflg, slppri, errs;
	int error = 0;
	caddr_t slpadr;
	mblk_t *mp;
	long *rd_count;
	int bid;

	if (fmode & (FNDELAY|FNONBLOCK)) {
		if (!(flag & NOINTR))
			error = EAGAIN;
		*done = 1;
		return (error);
	}

	if ((flag & READWAIT) || (flag & GETWAIT)) {
		slpflg = RSLEEP;
		slpadr = (caddr_t)RD(stp->sd_wrq);
		slppri = STIPRI;
		errs = STRDERR|STPLEX;

		/*
		 * If any module downstream has requested read notification
		 * by setting SNDMREAD flag using M_SETOPTS, send a message
		 * down stream.
		 */
		if ((flag & READWAIT) && (stp->sd_flag & SNDMREAD)) {
			while ((mp = allocb(sizeof(long), BPRI_MED)) == NULL) {
				s = splstr();
				bid = bufcall(sizeof(long), BPRI_MED, strqbuf,
				    stp);
				if (bid == 0) {
					splx(s);
					*done = 1;
					return (EAGAIN);
				} else {
					stp->sd_flag |= RDBUFWAIT;
					if (sleep (slpadr, slppri| PCATCH)) {
						if (stp->sd_flag & RDBUFWAIT) {
							/* interrupted sleep */
							stp->sd_flag &= ~RDBUFWAIT;
							unbufcall(bid);
							splx(s);
							*done = 1;
							return (EINTR);
						}
					}
					splx(s);
				}
			}
			mp->b_datap->db_type = M_READ;
			rd_count = (long *)mp->b_wptr;
			*rd_count = count;
			mp->b_wptr += sizeof(long);
			/*
			 * Send the number of bytes requested by the
			 * read as the argument to M_READ.
			 */
			putnext(stp->sd_wrq, mp);
			if (qready())
				runqueues();
			/*
			 * If any data arrived due to inline processing
			 * of putnext(), don't sleep.
			 */
			mp = RD(stp->sd_wrq)->q_first;
			while (mp) {
				if (!(mp->b_flag & MSGNOGET))
					break;
				mp = mp->b_next;
			}
			if (mp != NULL) {
				*done = 0;
				return (error);
			}
		}
	} else {
		slpflg = WSLEEP;
		slpadr = (caddr_t)stp->sd_wrq;
		slppri = STOPRI;
		errs = STWRERR|STRHUP|STPLEX;
	}
	
	s = splstr();
	stp->sd_flag |= slpflg;
	if (sleep(slpadr, slppri|PCATCH)) {
		stp->sd_flag &= ~slpflg;
		splx(s);
		wakeprocs(slpadr, PRMPT);
		if (!(flag & NOINTR))
			error = EINTR;
		*done = 1;
		return (error);
	}
	splx(s);
	if (stp->sd_flag & errs) {
		*done = 1;
		return ((stp->sd_flag & STPLEX) ? EINVAL :
		    ((errs & STWRERR) ? stp->sd_werror : stp->sd_rerror));
	}
	*done = 0;
	return (error);
}

int
str2num(str)		/* string to number, updating pointer */
	register char **str;
{
	register int n;

	n = 0;
	for (; **str >= '0' && **str <= '9'; (*str)++)
		n = 10 * n + **str - '0';
	return (n);
}

/*
 * Update canon format pointer with "bytes" worth of data.
 */
void
adjfmtp(str, bp, bytes)
	register char **str;
	register mblk_t *bp;
	int bytes;
{
	register caddr_t addr;
	register caddr_t lim;
	long num;

	addr = (caddr_t)bp->b_rptr;
	lim = addr + bytes;
	while (addr < lim) {
		switch (*(*str)++) {
		case 's':			/* short */
			addr = SALIGN(addr);
			addr = SNEXT(addr);
			break;
		case 'i':			/* integer */
			addr = IALIGN(addr);
			addr = INEXT(addr);
			break;
		case 'l':			/* long */
			addr = LALIGN(addr);
			addr = LNEXT(addr);
			break;
		case 'b':			/* byte */
			addr++;
			break;
		case 'c':			/* character */
			if ((num = str2num(str)) == 0) {
				while (*addr++)
					;
			} else
				addr += num;
			break;
		case 0:
			return;
		default:
			break;
		}
	}
}

void
strqbuf(stp)
	register struct stdata *stp;
{
	register int s;
	 
	if (stp->sd_flag & RDBUFWAIT) {
		s = splstr();
		stp->sd_flag &= ~RDBUFWAIT;
		splx(s);
		wakeprocs((caddr_t)RD(stp->sd_wrq), PRMPT);
	}
}

/*
 * Perform job control discipline access checks.
 * Return 0 for success and the errno for failure.
 */

#define cantsend(pp,sig) \
	(sigismember(&pp->p_ignore,sig) || sigismember(&pp->p_hold,sig))

int
straccess(stp, mode)
	struct stdata *stp;
	enum jcaccess mode;
{
	register proc_t *pp;
	register sess_t *sp;

	pp = u.u_procp;
	sp = pp->p_sessp;

	for (;;) {

		/* 
		 * if this is not the calling process's controlling terminal
		 * or this is a FIFO or the calling process is
		 * already in the foreground then allow access
		 */

		if (sp->s_dev != stp->sd_vnode->v_rdev 
		  || stp->sd_vnode->v_type == VFIFO
		  || pp->p_pgidp == stp->sd_pgidp)
			return 0;

		/*
		 * check to see if controlling terminal has been deallocated
		 */

		if (sp->s_vp == NULL) {
			if (cantsend(pp,SIGHUP))
				return EIO;
			pgsignal(pp->p_pgidp, SIGHUP);
		} 

		else if (mode == JCGETP)
			return 0;

		else if (mode == JCREAD) {
			if (cantsend(pp,SIGTTIN) || pp->p_detached)
				return EIO;
			pgsignal(pp->p_pgidp,SIGTTIN);
		} 

		else {  /* mode == JCWRITE or JCSETP */
			if (mode == JCWRITE && !(stp->sd_flag & STRTOSTOP)
			  || cantsend(pp,SIGTTOU))
				return 0;
			if (pp->p_detached)
				return EIO;
			pgsignal(pp->p_pgidp, SIGTTOU);
		}

		(void) sleep((caddr_t)&lbolt, STIPRI);
	}
}

/*
 * Return size of message of block type (bp->b_datap->db_type)
 * If fromhead is non-zero, then start at beginning of message.
 * If fromhead is zero, then start at end of message.
 */
int
xmsgsize(mp, fromhead)
	mblk_t *mp;
	int fromhead;
{
	register mblk_t *bp;
	register unsigned char type;
	register count;
	register int s;
	mblk_t *endbp;
	
	s = splstr();
	if (fromhead) {
		bp = mp;
		type = bp->b_datap->db_type;
		count = 0;
		for (; bp; bp = bp->b_cont) {
			if (type != bp->b_datap->db_type)
				break;
			ASSERT(bp->b_wptr >= bp->b_rptr);
			count += bp->b_wptr - bp->b_rptr;
		}
	} else {
		for (bp = mp; bp->b_cont; bp = bp->b_cont)
			;
		type = bp->b_datap->db_type;
		ASSERT(bp->b_wptr >= bp->b_rptr);
		count = bp->b_wptr - bp->b_rptr;
		endbp = bp;
		while (bp != mp) {
			for (bp = mp; bp->b_cont != endbp; bp = bp->b_cont)
				;
			if (type != bp->b_datap->db_type)
				break;
			ASSERT(bp->b_wptr >= bp->b_rptr);
			count += bp->b_wptr - bp->b_rptr;
			endbp = bp;
		}
	}
	splx(s);
	return(count);
}

/*
 * Allocate a stream head.
 */
struct stdata *
shalloc(qp)
	queue_t *qp;
{
	register s;
	register stdata_t *stp;
	register struct shinfo *shp;

	s = splstr();
	if ((shp = (struct shinfo *) kmem_zalloc(sizeof(struct shinfo), SE_SLEEP)) == NULL) {
		strst.stream.fail++;
		splx(s);
		return(NULL);
	}
	stp = (stdata_t *) shp;
	stp->sd_wrq = WR(qp);
	BUMPUP(strst.stream);
	Strinfo[DYN_STREAM].sd_cnt++;
	Strcount += sizeof(struct shinfo);
	shp->sh_next = (struct shinfo *) Strinfo[DYN_STREAM].sd_head;
	if (shp->sh_next)
		shp->sh_next->sh_prev = shp;
	shp->sh_prev = NULL;
	Strinfo[DYN_STREAM].sd_head = (void *) shp;
	splx(s);
	return(stp);
}

/*
 * Free a stream head.
 */
void
shfree(stp)
	register struct stdata *stp;
{
	register s;
	register struct shinfo *shp;

	s = splstr();
	shp = (struct shinfo *) stp;
	if (shp->sh_prev == NULL) {
		if (shp->sh_next)
			shp->sh_next->sh_prev = NULL;
		Strinfo[DYN_STREAM].sd_head = (void *) shp->sh_next;
	}
	else {
		if (shp->sh_next)
			shp->sh_next->sh_prev = shp->sh_prev;
		shp->sh_prev->sh_next = shp->sh_next;
	}
	kmem_free(shp, sizeof(struct shinfo));
	Strinfo[DYN_STREAM].sd_cnt--;
	Strcount -= sizeof(struct shinfo);
	strst.stream.use--;
	splx(s);
	return;
}




/*
 * Allocate a message block.
 *
 * Should be protected by appropriate spl's from the calling code.
 */
struct msgb *
xmsgalloc()
{
	register struct msgb *mp;

	if ((mp	= (struct msgb *) kmem_zalloc(sizeof(struct mbinfo), SE_NOSLP))
	 	== NULL) {
		strst.msgblock.fail++;
		return NULL;
	}
	BUMPUP(strst.msgblock);
	Strinfo[DYN_MSGBLOCK].sd_cnt++;
	Strcount += sizeof(struct mbinfo);
	_INSERT_MSG_INUSE((struct mbinfo *)mp);
	return((mblk_t *)mp);
}





/*
 * Allocate a message/data/buffer triplet.
 *
 * Should be protected by appropriate spl's from the calling code.
 */
struct mdbblock *
xmdballoc()
{
	register struct mdbblock *mp;

	if ((mp	= (struct mdbblock *) kmem_zalloc(sizeof(struct mdbblock), SE_NOSLP))
	 	== NULL) {
		strst.mdbblock.fail++;
		return NULL;
	}

	BUMPUP(strst.mdbblock);
	Strinfo[DYN_MDBBLOCK].sd_cnt++;
	Strcount += sizeof(struct mdbblock);
	_INSERT_MDB_INUSE(mp);
	return(mp);
}


/*
 * Allocate a pair of queues
 */
queue_t *
allocq()
{
	register s;
	register queue_t *qp;
	register struct queinfo *qip;

	s = splstr();
	/* allocate a queinfo struct (which contains two queues) */
	if ((qip = (struct queinfo *) kmem_zalloc(sizeof(struct queinfo), SE_SLEEP)) == NULL) {
		strst.queue.fail++;
		splx(s);
		return(NULL);
	}
	qp = (queue_t *) qip;
	qp->q_qinfo = NULL;
	qp->q_first = NULL;
	qp->q_last = NULL;
	qp->q_next = NULL;
	qp->q_ptr = NULL;
	qp->q_count = 0;
	qp->q_flag = QUSE | QREADR;
	qp->q_minpsz = 0;
	qp->q_maxpsz = 0;
	qp->q_hiwat = 0;
	qp->q_lowat = 0;
#ifdef _STYPES
	qp->q_eq = &qip->qu_requeue;
#endif /* _STYPES */
	qp->q_link = NULL;
	qp->q_bandp = NULL;
	qp->q_nband = 0;
	WR(qp)->q_qinfo = NULL;
	WR(qp)->q_first = NULL;
	WR(qp)->q_last = NULL;
	WR(qp)->q_next = NULL;
	WR(qp)->q_ptr = NULL;
	WR(qp)->q_count = 0;
	WR(qp)->q_flag = QUSE;
	WR(qp)->q_minpsz = 0;
	WR(qp)->q_maxpsz = 0;
	WR(qp)->q_hiwat = 0;
	WR(qp)->q_lowat = 0;
#ifdef _STYPES
	WR(qp)->q_eq = &qip->qu_wequeue;
#endif /* _STYPES */
	WR(qp)->q_link = NULL;
	WR(qp)->q_bandp = NULL;
	WR(qp)->q_nband = 0;
	/* for accounting purposes, count as 2 */
	BUMPUP(strst.queue);
	BUMPUP(strst.queue);
	Strinfo[DYN_QUEUE].sd_cnt += 2;
	Strcount += sizeof(struct queinfo);
	qip->qu_next = (struct queinfo *) Strinfo[DYN_QUEUE].sd_head;
	if (qip->qu_next)
		qip->qu_next->qu_prev = qip;
	qip->qu_prev = NULL;
	Strinfo[DYN_QUEUE].sd_head = (void *) qip;
	splx(s);
	return(qp);
}

/*
 * Free a pair of queues.
 */
void
freeq(qp)
	register queue_t *qp;
{
	register int s;
	register struct queinfo *qip;
	register qband_t *qbp;
	register qband_t *nqbp;

	s = splstr();
	qbp = qp->q_bandp;
	while (qbp) {
		nqbp = qbp->qb_next;
		freeband(qbp);
		qbp = nqbp;
	}
	qbp = WR(qp)->q_bandp;
	while (qbp) {
		nqbp = qbp->qb_next;
		freeband(qbp);
		qbp = nqbp;
	}
	qip = (struct queinfo *) qp;
	if (qip->qu_prev == NULL) {
		if (qip->qu_next)
			qip->qu_next->qu_prev = NULL;
		Strinfo[DYN_QUEUE].sd_head = (void *) qip->qu_next;
	}
	else {
		if (qip->qu_next)
			qip->qu_next->qu_prev = qip->qu_prev;
		qip->qu_prev->qu_next = qip->qu_next;
	}
	kmem_free(qip, sizeof(struct queinfo));
	Strcount -= sizeof(struct queinfo);
	/* for accounting purposes, count individually */
	strst.queue.use -= 2;
	Strinfo[DYN_QUEUE].sd_cnt -= 2;
	splx(s);
	return;
}

/*
 * Allocate a qband structure.
 */
qband_t *
allocband()
{
	register s;
	register struct qbinfo *qbip;

	s = splstr();
	if ((qbip = (struct qbinfo *) kmem_zalloc(sizeof(struct qbinfo), SE_NOSLP)) == NULL) {
		splx(s);
		return(NULL);
	}
	Strinfo[DYN_QBAND].sd_cnt++;
	Strcount += sizeof(struct qbinfo);
	qbip->qbi_next = (struct qbinfo *) Strinfo[DYN_QBAND].sd_head;
	if (qbip->qbi_next)
		qbip->qbi_next->qbi_prev = qbip;
	qbip->qbi_prev = NULL;
	Strinfo[DYN_QBAND].sd_head = (void *) qbip;
	splx(s);
	return((qband_t *) qbip);
}

/*
 * Free a qband structure.
 */
void
freeband(qbp)
	register qband_t *qbp;
{
	register s;
	register struct qbinfo *qbip;

	s = splstr();
	qbip = (struct qbinfo *) qbp;
	if (qbip->qbi_prev == NULL) {
		if (qbip->qbi_next)
			qbip->qbi_next->qbi_prev = NULL;
		Strinfo[DYN_QBAND].sd_head = (void *) qbip->qbi_next;
	}
	else {
		if (qbip->qbi_next)
			qbip->qbi_next->qbi_prev = qbip->qbi_prev;
		qbip->qbi_prev->qbi_next = qbip->qbi_next;
	}
	kmem_free(qbip, sizeof(struct qbinfo));
	Strcount -= sizeof(struct qbinfo);
	Strinfo[DYN_QBAND].sd_cnt--;
	splx(s);
	return;
}

/*
 * Allocate a stream event cell.
 */
struct strevent *
sealloc(slpflag)
	int slpflag;
{
	register s;
	register struct seinfo *sep;

	s = splstr();
	if (sefreelist) {
		sep = sefreelist;
		sefreelist = sep->s_next;
		sep->s_strevent.se_procp = NULL;
		sep->s_strevent.se_events = 0;
		sep->s_strevent.se_next = NULL;
	}
	else if (sep = (struct seinfo *) kmem_zalloc(sizeof(struct seinfo), slpflag)) {
		Strinfo[DYN_STREVENT].sd_cnt++;
		Strcount += sizeof(struct seinfo);
	}
	else {
		if (slpflag == SE_NOSLP) {
			/* use the cache for these */
			if (secachep) {
				sep = secachep;
				secachep = secachep->s_next;
				sep->s_strevent.se_procp = NULL;
				sep->s_strevent.se_events = 0;
				sep->s_strevent.se_next = NULL;
			}
		}
		if (sep == NULL) {
			/* failed anyhow */
			strst.strevent.fail++;
			splx(s);
			return(NULL);
		}
	}
	BUMPUP(strst.strevent);
	sep->s_next = (struct seinfo *) Strinfo[DYN_STREVENT].sd_head;
	if (sep->s_next)
		sep->s_next->s_prev = sep;
	sep->s_prev = NULL;
	Strinfo[DYN_STREVENT].sd_head = (void *) sep;
	splx(s);
	return((struct strevent *) sep);
}

/*
 * Free a stream event cell
 */
void
sefree(sep)
	struct strevent *sep;
{
	register s;
	register struct seinfo *seip;

	s = splstr();
	seip = (struct seinfo *) sep;
	if (seip->s_prev == NULL) {
		if (seip->s_next)
			seip->s_next->s_prev = NULL;
		Strinfo[DYN_STREVENT].sd_head = (void *) seip->s_next;
	}
	else {
		if (seip->s_next)
			seip->s_next->s_prev = seip->s_prev;
		seip->s_prev->s_next = seip->s_next;
	}
	if (seip >= &Secache[0] && seip <= &Secache[SECACHE]) {
		/* it's from the cache */
		seip->s_next = secachep;
		secachep = seip;
	}
	else {
		seip->s_next = sefreelist;
		sefreelist = seip;
	}
	strst.strevent.use--;
	splx(s);
	return;
}


/*
 * Check linked list of strhead write queues with held messages;
 * put downstream those that have been held long enough.
 *
 * Run the service procedures of each enabled queue
 *	-- must not be reentered
 *
 * Called by service mechanism (processor dependent) if there
 * are queues to run.  The mechanism is reset.
 */
void
queuerun()
{
	register queue_t *q;
	register int s;
	register int count;
	register struct strevent *sep;
	register qband_t *qbp;
	register int nevent;
	mblk_t *mp;

	s = splstr();

	while ((q = scanqhead) && ((struct stdata *)q->q_ptr)->sd_rtime <= lbolt)  {
		if ((scanqhead = q->q_link) == NULL)
			scanqtail = NULL;

		q->q_flag &= ~QHLIST;

		if (mp = q->q_first)  {
			q->q_first = NULL;
			if (((struct stdata *)q->q_ptr)->sd_flag & (STRHUP|STWRERR))  {
				freemsg(mp);
			} else  {
				spl1();
				putnext(q, mp);
				splstr();
			}
		}
	}

	do {
		if (strbcflag) {
			strbcwait = strbcflag = 0;

			/*
			 * count how many events are on the list
			 * now so we can check to avoid looping
			 * in low memory situations
			 */
			nevent = 0;
			for (sep = strbcalls.bc_head; sep; sep = sep->se_next)
				nevent++;
			/*
			 * get estimate of available memory from kmem_avail().
			 * awake all bufcall functions waiting for
			 * memory whose request could be satisfied 
			 * by 'count' memory and let 'em fight for it.
			 */
			count = kmem_avail();
			while ( (sep = strbcalls.bc_head) && nevent ) {
				--nevent;
				if ( sep->se_size <= count ) {
					strbcalls.bc_head = sep->se_next;
					(*sep->se_func)(sep->se_arg);
					sefree(sep);
				}
				else {
					/*
					 * too big, try again later - note
					 * that nevent was decremented above
					 * so we won't retry this one on this
					 * iteration of the loop
					 */
					strbcalls.bc_head = sep->se_next;
					sep->se_next = NULL;
					strbcalls.bc_tail->se_next = sep;
					strbcalls.bc_tail = sep;
				}
			}
			if (strbcalls.bc_head)
				/*
				 * still some bufcalls we couldn't do
				 * let kmem_free know
				 */
				strbcwait = 1;
			else
				strbcalls.bc_tail = NULL;
		}

		while (q = qhead) {
			if (!(qhead = q->q_link))
				qtail = NULL;
			q->q_flag &= ~QENAB;
			if (q->q_qinfo->qi_srvp) {
				spl1();

#ifdef KPERF
				if(kpftraceflg)
					kperf_write(KPT_INTR,
					    q->q_qinfo->qi_srvp, curproc);
#endif /* KPERF */

				(*q->q_qinfo->qi_srvp)(q);

#ifdef KPERF
				if(kpftraceflg)
					kperf_write(KPT_INT_KRET,
					    q->q_qinfo->qi_srvp, curproc);
#endif /* KPERF */
				splstr();
				q->q_flag &= ~QBACK;
				for (qbp = q->q_bandp; qbp; qbp = qbp->qb_next)
					qbp->qb_flag &= ~QB_BACK;
			}
		}
	} while (strbcflag);

	if (scanqhead && !strscanflag)  {
		strscanflag++;
		timeout(strscan, 0, STRSCANP);
	}

	qrunflag = 0;
	splx(s);
}

/*
 * Function to kick off queue scheduling for those system calls
 * that cause queues to be enabled (read, recv, write, send, ioctl).
 */
void
runqueues()
{
	register s;

	s = splhi();
	if (qrunflag && !queueflag) {
		queueflag = 1;
		splx(s);
		queuerun();
		queueflag = 0;
		return;
	}
	splx(s);
}

/* 
 * Find module
 * 
 * return index into fmodsw
 * or -1 if not found
 */
int
findmod(name)
	register char *name;
{
	register int i, j;

	for (i = 0; i < fmodcnt; i++)
		for (j = 0; j < FMNAMESZ + 1; j++) {
			if (fmodsw[i].f_name[j] != name[j]) 
				break;
			if (name[j] == '\0')
				return(i);
		}
	return(-1);
}


/*
 * Set the QBACK or QB_BACK flag in the given queue for
 * the given priority band.
 */
void
setqback(q, pri)
	register queue_t *q;
	register unsigned char pri;
{
	register int i;
	qband_t *qbp;
	qband_t **qbpp;

	if (pri != 0) {
		if (pri > q->q_nband) {
			qbpp = &q->q_bandp;
			while (*qbpp)
				qbpp = &(*qbpp)->qb_next;
			while (pri > q->q_nband) {
				if ((*qbpp = allocband()) == NULL) {
					cmn_err(CE_WARN,
					    "setqback: can't allocate qband\n");
					return;
				}
				(*qbpp)->qb_hiwat = q->q_hiwat;
				(*qbpp)->qb_lowat = q->q_lowat;
				q->q_nband++;
				qbpp = &(*qbpp)->qb_next;
			}
		}
		qbp = q->q_bandp;
		i = pri;
		while (--i)
			qbp = qbp->qb_next;
		qbp->qb_flag |= QB_BACK;
	} else {
		q->q_flag |= QBACK;
	}
}

int
strcopyin(from, to, len, fmt, copyflag)
	caddr_t from;
	caddr_t to;
	unsigned int len;
	char *fmt;
	int copyflag;
{
	int error = 0;

	if (copyflag == U_TO_K) {
		if (ncopyin(from, to, len, fmt))
			return (EFAULT);
	} else {
		ASSERT(copyflag == K_TO_K);
		bcopy(from, to, len);
	}
	return (0);
}

int
strcopyout(from, to, len, fmt, copyflag)
	caddr_t from;
	caddr_t to;
	unsigned int len;
	char *fmt;
	int copyflag;
{
	int error = 0;

	if (copyflag == U_TO_K) {
		if (ncopyout(from, to, len, fmt))
			return (EFAULT);
	} else {
		ASSERT(copyflag == K_TO_K);
		bcopy(from, to, len);
	}
	return (0);
}

void
strsignal(stp, sig, band)
	struct stdata *stp;
	int sig;
	long band;
{
	register int s;

	switch (sig) {
	case SIGPOLL:				
		s = splstr();
		if (stp->sd_sigflags & S_MSG) 
			strsendsig(stp->sd_siglist, S_MSG, band);
		if (stp->sd_eventflags & S_MSG) 
			strevpost(stp, S_MSG, band);
		splx(s);
		break;

	default:
		if (stp->sd_pgidp)
			pgsignal(stp->sd_pgidp, sig);
		break;
	}
}

strhup(stp)
	struct stdata *stp;
{
	int s;

	s = splstr();
	if (stp->sd_sigflags & S_HANGUP) 
		strsendsig(stp->sd_siglist, S_HANGUP, 0L);
	pollwakeup(&stp->sd_pollist, POLLHUP);
	if (stp->sd_eventflags & S_HANGUP)
		strevpost(stp, S_HANGUP, 0L);
	splx(s);
}

stralloctty(sp, stp)
	register sess_t *sp;
	register struct stdata *stp;
{
	stp->sd_sidp = sp->s_sidp;
	stp->sd_pgidp = sp->s_sidp;
	PID_HOLD(stp->sd_pgidp);
	PID_HOLD(stp->sd_sidp);
}

strfreectty(stp)
	register struct stdata *stp;
{
	pgsignal(stp->sd_pgidp, SIGHUP);
	PID_RELE(stp->sd_pgidp);
	PID_RELE(stp->sd_sidp);
	stp->sd_pgidp = NULL;
	stp->sd_sidp = NULL;
	if (! (stp->sd_flag & STRHUP) )
		strhup(stp);
}

/*
 * Unlink "all" persistant links.
 */
void
strpunlink(crp)
	struct cred *crp;
{
	struct stdata *stp;
	struct shinfo  *shp;
	int rval;
	/*
	 * for each allocated stream head, call munlinkall()
	 * with flag of LINKPERSIST to unlink any/all persistant
	 * links for the device.
	 */

	shp = (struct shinfo *) Strinfo[DYN_STREAM].sd_head;
	while(shp)
	{
		stp = (struct stdata *) shp;
		(void) munlinkall(stp, LINKIOCTL|LINKPERSIST, crp, &rval);
		shp = shp->sh_next;
	}
}

int
strctty(pp, stp)
	register proc_t *pp;
	register struct stdata *stp;
{
	register int s;
	register sess_t *sp = pp->p_sessp;
	extern vnode_t *makectty();

	if ((stp->sd_flag & (STRHUP|STRDERR|STWRERR|STPLEX)) == 0
	  && stp->sd_sidp == NULL		/* not allocated as ctty */
	  && sp->s_sidp == pp->p_pidp		/* session leader */
	  && sp->s_vp == NULL) {		/* without ctty */
	  	ASSERT(stp->sd_pgidp == NULL);
		alloctty(pp, makectty(stp->sd_vnode));
		stralloctty(sp, stp);
		s = splstr();
		stp->sd_flag |= STRISTTY;	/* just to be sure */
		splx(s);
		return (1);
	}
	return (0);
}
