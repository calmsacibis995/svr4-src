/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_comm.c	1.3.2.1"
/*
 * Communications routines (and some other subroutines) for RFS.
 */

#include "sys/list.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/vnode.h"
#include "sys/stream.h"
#include "sys/rf_messg.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_comm.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/rf_debug.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/inline.h"
#include "sys/cred.h"
#include "sys/hetero.h"
#include "rf_canon.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "rfcl_subr.h"
#include "sys/rf_adv.h"
#include "rf_serve.h"
#include "sys/uio.h"
#include "sys/sysinfo.h"
#include "sys/fs/rf_acct.h"
#include "sys/fs/rf_vfs.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/kmem.h"
#include "sys/fbuf.h"
#include "vm/page.h"
#include "vm/pvn.h"
#include "rf_cache.h"

/*
 * Structure passed into esballoc to have freeb queue up work for rf_daemon.
 *
 * These are allocated and freed by rffr_alloc() and RFFR_FREE(), respectively.
 *
 *	frtn.free_func == rfa_workenq
 *	frtn.free_arg == &work.rfaw_elt
 *	work.rfaw_func == func passed into rffr_alloc
 *	work.rfaw_farg == pointer passed into rffr_alloc
 *
 *	freeb will call rfa_workenq(&work.rfaw_elt)
 *	rf_daemon will call rfaw_func(&work)
 */
typedef struct rf_frtn {
	frtn_t		frtn;
	rfa_work_t	work;
} rf_frtn_t;

/* Convert address of rffr_work to address of enclosing structure */
#define WTOFR(wp)	\
	((rf_frtn_t *)((caddr_t)(wp) - (caddr_t)&((rf_frtn_t *)NULL)->work))

#define RFFR_FREE(rffrp)	kmem_free((rffrp), sizeof(rf_frtn_t))

/*
 * Arg structure passed by rfesb_fbread into rffr_alloc for rfesb_fbrelse, 
 * which we use with esballoc().  We go to this trouble because we don't
 * want the provider to sleep in fbrelse.
 *
 *	fbp == &fbuf returned by fbread, to be passed to fbrelse
 *	rw == seg arg for fbrelse
 */
typedef struct rf_fba {
	struct fbuf	*fbp;
	enum seg_rw	rw;
} rf_fba_t;

STATIC void		rdu_free();
STATIC void		rfesb_fbrelse();
STATIC void		rfesb_pagerele();
STATIC rf_frtn_t	*rffr_alloc();
STATIC void		rfesb_pvn_done();
STATIC void		rf_savesigs();

/* imports */
extern void	dst_clean();
extern void	delay();

/* Gift templates. */
rf_gift_t	rf_daemon_gift	= {DAEMON_RD, 0};
rf_gift_t	rf_null_gift;

rcvd_t		*mountrd;		/* rd for mounts */
rcvd_t		*sigrd;			/* rd for signals */
STATIC rcvd_t  	*reserved_rd;		/* For critical allocations */
STATIC sndd_t  	*reserved_sd;		/* For critical allocations */

STATIC int		sd_nfree;	/* free send descriptor count */
STATIC ls_elt_t 	sd_freelist;	/* list of free sndds */
STATIC int		rd_nfree;	/* number of free receive descriptors */
STATIC rd_user_t	*rdu_freelist;	/* free rd_user structures */
STATIC rcvd_t  		*rd_freelist;	/* free recv desc link list */

/*
 * Create a send descriptor.  Only if canfail, returns ENOMEM for failure,
 * NULLING *sdpp.
 * Returns 0 for success, updating *sdpp with pointer to initialized sd.
 * NOTE:  sndd_create creates reserved_sd, that call must use canfail.
 */
int
sndd_create(canfail, sdpp)
	int	canfail;
	sndd_t	**sdpp;
{
	register sndd_t		*retsndd;
	register ls_elt_t	*free_el = NULL;

	static sndd_t initsd = {
		NULL,	/* sd_hash.ls_next */
		NULL,	/* sd_hash.ls_prev */
		NULL,	/* sd_free.ls_next */
		NULL,	/* sd_free.ls_prev */
		-1,	/* sd_size */
		SDUSED, /* sd_stat */
		{0, 0},	/* sd_gift */
		-1	/* sd_mntid */
	};
	/*
	 * When canfail, client requests don't take sndd that servers may
	 * need.  Servers only fail when the are no free sndds.
	 */
	if (canfail && (!sd_nfree || !RF_SERVER() &&
	  sd_nfree <= maxserve - rfsr_nservers)) {
		cmn_err(CE_NOTE, "sndd_create: not enough sndds\n");
		*sdpp = NULL;
		return ENOMEM;
	}

	for ( ; ; ) {
		if ((RF_SERVER() || sd_nfree > maxserve - rfsr_nservers) &&
                  (free_el = LS_REMQUE(&sd_freelist)) != NULL) {
			retsndd = FREETOSD(free_el);
			ASSERT(!(retsndd->sd_stat & SDUSED));
			sd_nfree--;
			break;
		}
		ASSERT(!canfail);
		if (!(reserved_sd->sd_stat & SDLOCKED)) {
			retsndd = reserved_sd;
			break;
		} else {
			reserved_sd->sd_stat |= SDWANT;
			(void)sleep((caddr_t)&reserved_sd, PZERO);
		}
	}
	if (!LS_ISEMPTY(&retsndd->sd_hash)) {
		LS_REMOVE(&retsndd->sd_hash);
		if (SDTOV(retsndd)->v_pages != NULL) {
			rfc_pageabort(retsndd, (off_t)0, (off_t)0);
		}
	}
	retsndd[0] = initsd;
	if (retsndd == reserved_sd) {
		reserved_sd->sd_stat |= SDLOCKED;
	}
	LS_INIT(&retsndd->sd_free);
	LS_INIT(&retsndd->sd_hash);
	*sdpp = retsndd;
	return 0;
}

/*
 * Hash the denoted send descriptor to the associated rf_vfs for sndd caching.
 * Put on head of hash list, tail of free list.
 */
void
sndd_hash(sdp)
	register sndd_t		*sdp;
{
	ASSERT(!SDTOV(sdp)->v_count);
	ASSERT(sdp->sd_stat & SDUSED);
	ASSERT(LS_ISEMPTY(&sdp->sd_hash));
	ASSERT(LS_ISEMPTY(&sdp->sd_free));
	sdp->sd_stat &= ~SDUSED;
	sdp->sd_gift = rf_null_gift;
	dst_clean(sdp);
	LS_INS_AFTER(&VFTORF(SDTOV(sdp)->v_vfsp)->rfvfs_sdhash, &sdp->sd_hash);
	LS_INSQUE(&sd_freelist, &sdp->sd_free);
	sd_nfree++;
}

/*
 * Remove the denoted send descriptor from hash and free lists.
 */
void
sndd_unhash(sdp)
	sndd_t	*sdp;
{
	ASSERT(!SDTOV(sdp)->v_count);
	ASSERT(!(sdp->sd_stat & SDUSED));
	ASSERT(!LS_ISEMPTY(&sdp->sd_hash));
	ASSERT(!LS_ISEMPTY(&sdp->sd_free));
	LS_REMOVE(&sdp->sd_hash);
	LS_REMOVE(&sdp->sd_free);
	sdp->sd_stat |= SDUSED;
	--sd_nfree;
}

/*
 * Return send descriptor to the (head of the) freelist, mark it unused.
 * Set *sdpp NULL.
 */
void
sndd_free(sdpp)
	sndd_t		**sdpp;
{
	register sndd_t	*sdp = *sdpp;

	if (sdp) {
		ASSERT(sdp->sd_stat & SDUSED);
		ASSERT(LS_ISEMPTY(&sdp->sd_hash));
		ASSERT(LS_ISEMPTY(&sdp->sd_free));
		if (sdp == reserved_sd && sdp->sd_stat & SDWANT) {
			wakeprocs((caddr_t)&reserved_sd, PRMPT);
		}
		sdp->sd_stat = SDUNUSED;
		sdp->sd_gift = rf_null_gift;
		*sdpp = NULL;
		if (sdp != reserved_sd) {
			if (SDTOV(sdp)->v_pages != NULL) {
				rfc_pageabort(sdp, (off_t)0, (off_t)0);
			}
			dst_clean(sdp);
			LS_INS_AFTER(&sd_freelist, &sdp->sd_free);
			sd_nfree++;
		}
	}
}

/*
 * Send a message.  Fill in the gift field in the message if there is a gift.
 *
 * Returns 0 for success, ENOLINK for failure.  bp is a dangling reference
 * after call to this function; message goes downstream if succesful, is
 * freed in failure cases.  retrans should be TRUE iff caller had duped
 * message, is now resending.
 */
int
rf_sndmsg(sd, bp, bytes, gift, retrans)
	register sndd_t		*sd;
	register mblk_t		*bp;
	size_t			bytes;
	rcvd_t			*gift;
	int			retrans;
{
	register rf_message_t	*msg;
	register rf_common_t	*cop;
	register gdp_t		*tgdp;
	queue_t			*rq;
	queue_t			*wq;

	rfc_info.rfci_snd_msg++;

	rq = sd->sd_queue;
	ASSERT(rq);
	tgdp = QPTOGP(rq);
	wq = WR(rq);

	/*
	 * Check on constate is because currently not all sndds can be
	 * enumerated, e.g., those used by rf_daemon and rf_tmo.
	 */

	if (sd->sd_stat & SDLINKDOWN || tgdp->constate != GDPCONNECT) {
		rf_freemsg(bp);
		return ENOLINK;
	}

	if (retrans) {
		if (tgdp->hetero == ALL_CONV) {

			/*
			 * Need to decanonize headers to update time, etc. below
			 */

#ifdef DEBUG
			ASSERT(rf_mcfcanon(bp));
			ASSERT(rf_rhfcanon(bp, tgdp));
#else
			(void)rf_mcfcanon(bp);
			(void)rf_rhfcanon(bp, tgdp);
#endif
		}
	}

	/*
	 * Message allocation earlier set up header block write
	 * pointers.  Here we set up other write pointers as a service
	 * to callers.  Note that, as we set up messages on the send
	 * end, if any data is not piggybacked with headers, none will
	 * be.  On the receive end, however, no such assumption can be
	 * made, because transport providers may munge the message
	 * arbitrarily.
	 */

	if (!bp->b_cont) {

		/* Data is piggybacked in header block */

		ASSERT(bp->b_datap->db_lim - bp->b_datap->db_base >= bytes);

		bp->b_wptr = bp->b_rptr + bytes;
	} else {
		register mblk_t		*databp;
		register mblk_t		*predatabp;
		register size_t		sz;
		size_t			bpsz;
		dblk_t			*dp;

		predatabp = bp;
		databp = bp->b_cont;
		for (sz = bytes - (bp->b_wptr - bp->b_rptr);
		  /* LINTED - stupid lint says bpsz is uninitialized */
		  sz; sz -= bpsz) {

			ASSERT(databp);	/* but may have extra mblks */
			dp = databp->b_datap;
			bpsz = MIN(sz, dp->db_lim - dp->db_base);
			databp->b_wptr = databp->b_rptr + bpsz;
			predatabp = databp;
			databp = predatabp->b_cont;
		}

		if (databp) {
			ASSERT(predatabp->b_cont == databp);
			freemsg(databp);
			predatabp->b_cont = NULL;
		}
	}

	cop = RF_COM(bp);
	if (tgdp->version >= RFS2DOT0 && cop->co_type == RF_REQ_MSG) {

		/* If 4.0 server, send time in every request. */

		rf_request_t *req = RF_REQ(bp);

		req->rq_sec = hrestime.tv_sec;
		req->rq_nsec = hrestime.tv_nsec;
	}

	/*
	 * Each message contains the sending process's pid, though
	 * it is unused in response messages.
	 */

	cop->co_pid = u.u_procp->p_pid;

	msg = RF_MSG(bp);
	msg->m_dest = sd->sd_gift;
	msg->m_stat |= RF_VER1;
	msg->m_size = bytes;
	if (gift) {
		ASSERT(gift->rd_stat != RDUNUSED);

		msg->m_stat |= RF_GIFT;
		msg->m_gift.gift_id = gift - rcvd;
		msg->m_gift.gift_gen = gift->rd_gen;

		/*
		 * Keep track of who gets gift.
		 * (Keep track of RDGENERAL RDs in make_gift.)
		 */

		if (gift->rd_qtype & RDSPECIFIC && gift != rf_daemon_rd) {
			/* TO DO:  union */
			gift->rd_user_list = (rd_user_t *)rq;
		}

		ASSERT(gift->rd_qtype == RDSPECIFIC ||
		  gift == mountrd || gift == sigrd ||
		  gift->rd_vp);
	}

	/*
	 * Invalidate the rfcl_lookup_cache for all non-RFINACTIVE REQUEST
	 * and for RFINACTIVE request messages for other vnodes.
	 */

	if (cop->co_type == RF_REQ_MSG &&
	  (cop->co_opcode != RFINACTIVE ||
	   SDTOV(sd) == rfcl_lookup_cache.lkc_vp) &&
	  rfcl_lookup_cache.lkc_crp) {
		crfree(rfcl_lookup_cache.lkc_crp);
		rfcl_lookup_cache.lkc_crp =  NULL;
	}

	if (tgdp->hetero == ALL_CONV) {
		rf_hdrtcanon(bp, tgdp->version);
	}

	while (!canput(wq)) {
		(void)sleep((caddr_t)tgdp, PZERO - 1);

		/*
		 * Check on constate is because currently not all sndds can be
		 * enumerated, e.g., those used by rf_daemon and rf_tmo.
		 */

		if (sd->sd_stat & SDLINKDOWN || tgdp->constate != GDPCONNECT) {
			rf_freemsg(bp);
			return ENOLINK;
		}
	}
#ifdef DEBUG
	msg->filler0 = tgdp->outseq++;
#else
	tgdp->outseq++;
#endif
	putq(wq, bp);
	return 0;
}

/*
 * Rcvmsg is called to wait for a response on the denoted rd.  It is
 * used by clients waiting after sending remote requests, and by servers
 * doing intermediate data movement.
 *
 * Returns 0, updating mblk out parameter for success.
 * For failure, returns ECOMM, setting out param to NULL.
 */
int
rf_rcvmsg(rcvdp, bufpp)
	rcvd_t		*rcvdp;
	mblk_t		**bufpp;
{
	k_sigset_t	sig;
	sigqueue_t	*sigqueue = NULL;
	char		cursig = 0;
	sigqueue_t	*curinfo = NULL;
	int		sigsent = FALSE;
	proc_t		*p = u.u_procp;
	int		error = 0;

	sigemptyset(&sig);
	*bufpp = NULL;

	for (;;) {

		/*
		 * Break out only when a message has arrived or we detect that
		 * our link is down.  Check both of these conditions after
		 * each return from sleep.
		 */

		if (!RCVDEMP(rcvdp)) {

			/*
			 * Found something on the rd.  Even if link is down,
			 * do this first to strip copyin responses, e.g.
			 */

			rfc_info.rfci_rcv_msg++;
			break;
		}

		if (rcvdp->rd_stat & RDLINKDOWN) {
			error = ECOMM;
			break;
		}

		if (sleep((caddr_t)&rcvdp->rd_qslp, 
		  PREMOTE | PCATCH | PNOSTOP)) {

			/*
			 * We've got a signal.  Hide it in preparation for
			 * going back to sleep.
			 *
			 * We send the signal to the server to let the
			 * remote side decide what is interruptible
			 * and what isn't.
			 */

			rf_savesigs(&cursig, &curinfo, &sigqueue, &sig);

			if (!RF_SERVER() && !sigsent && RDTOSD(rcvdp)) {

				/*
				 * We haven't signalled the server yet, so
				 * send the signal.  If the signal message
				 * is not sent because the link is down, our
				 * rcvd will be marked LINKDOWN.
				 *
				 * The above check on RDTOSD(rcvdp) prevents 
				 * calls to rfcl_signal() for fumount and cache
				 * disable messages.
				 */

				rfcl_signal(RDTOSD(rcvdp));
				sigsent = TRUE;
			}
		}

	}

	if (!error) {
		*bufpp = rf_dequeue(rcvdp);
		ASSERT(*bufpp);
	}

	if (p->p_cursig) {
		rf_savesigs(&cursig, &curinfo, &sigqueue, &sig);
	}

	if (cursig) {

		/*
		 * Restore current signal context to be handled in return from
		 * trap.
		 */

		p->p_cursig = cursig;
		p->p_curinfo = curinfo;
	}

	if (!sigisempty(&sig)) {

		/*
		 * Preserve time-ordering by prepending saved pending context
		 * to new.
		 */

		p->p_sigqueue =
		  sigprepend(&p->p_sig, p->p_sigqueue, &sig, sigqueue);
	}

	return error;
}

STATIC void
rf_savesigs(cursigp, curinfop, sigqp, ksigp)
	char		*cursigp;
	sigqueue_t	**curinfop;
	sigqueue_t	**sigqp;
	k_sigset_t	*ksigp;
{
	char		cursig = *cursigp;
	sigqueue_t	*curinfo = *curinfop;
	sigqueue_t	*sigqueue = *sigqp;
	proc_t		*p = u.u_procp;

	if (!cursig) {

		/*
		 * Remember the first current signal context
		 * to restore later.
		 */

		cursig = p->p_cursig;
		curinfo = p->p_curinfo;
	} else {

		/*
		 * Put subsequent current signal contexts into
		 * pending signal context.
		 */

		sigaddset(&p->p_sig, cursig);
		if (p->p_curinfo) {
			p->p_curinfo->sq_next = p->p_sigqueue;
			p->p_sigqueue = p->p_curinfo;
		}
	}

	/*
	 * Clear current signal context.  Save and clear
	 * pending signal context.
	 */

	p->p_cursig = 0;
	p->p_curinfo = NULL;
	if (!sigisempty(&p->p_sig)) {

		/*
		 * Preserve time-ordering by appending new
		 * pending context to that already saved.
		 */

		sigqueue = sigappend(ksigp, sigqueue,
		   &p->p_sig, p->p_sigqueue);
		sigemptyset(&p->p_sig);
		p->p_sigqueue = NULL;
	}

	*cursigp = cursig;
	*curinfop = curinfo;
	*sigqp = sigqueue;
}

/*
 * Make datasz bytes contiguous in message denoted by bp.  Message
 * must contain hdrsz bytes of headers.  Returns errno.
 * NOTE: can sleep.
 */
int
rf_pullupmsg(bp, hdrsz, datasz)
	register mblk_t		*bp;
	register size_t		hdrsz;
	register size_t		datasz;
{
	register mblk_t		*pullupbp;
	register mblk_t		*dup = NULL;
	register rf_message_t	*msg = RF_MSG(bp);

	if (msg->m_size != hdrsz + datasz) {

		/*
		 * Circuit manager guarantees that m_size == gdp_msgsize(bp).
		 */

		return EPROTO;
	}

	if (!bp->b_cont) {

		/*
		 * Circuit manager checks and above check guarantee all
		 * data follows headers
		 */

		ASSERT(bp->b_wptr - bp->b_rptr == msg->m_size);
		return 0;
	}

	if (bp->b_wptr - bp->b_rptr == hdrsz) {

		/* All data is in continuation block(s). */

		pullupbp = bp->b_cont;
	} else {

		/* Some data is in first block. */

		ASSERT(bp->b_wptr - bp->b_rptr > hdrsz); /* circuit manager */

		while ((dup = dupb(bp)) == NULL) {
			delay(10);
		}

		/*
		 * Drop the data out of bp and put it into dup.  Then
		 * interpose dup between bp and b_cont.
		 */

		dup->b_rptr += hdrsz;
		bp->b_wptr = dup->b_rptr;
		dup->b_cont = bp->b_cont;
		bp->b_cont = dup;

		pullupbp = dup;
	}
	while (!pullupmsg(pullupbp, (int)datasz)) {

		/*
		 * We know all data is present.  Wait for data buffer to
		 * become available.
		 */

		if (!bufcall(datasz, BPRI_MED, (int(*)())setrun,
		  (long)u.u_procp)) {
			delay(100);
		} else {
			(void)sleep((caddr_t)&(u.u_procp->p_flag), PZERO);
		}
	}
	return 0;
}

/*
 * Allocate message blocks with hdrsz bytes for RFS headers, datasz
 * bytes for data.  (Either hdrsz or datasz can be 0).  Headers are
 * guaranteed to be contiguous.  If base is non-NULL, then frtnp must
 * also be non-NULL, and datasz nonzero, and we will esballoc a message
 * block to cover the denoted space, passing in frtnp.  If data space
 * is not externally supplied, it will either piggybacked in header
 * buffer (if any) after headers, or reachable through b_cont.  If data
 * space is externally supplied, it will be reachable through header
 * buffer (if any) b_cont.
 *
 * Returns errno for failure.
 */
#define PIGGYBACKSZ	PAGESIZE
int
rf_allocmsg(hdrsz, datasz, bpri, canfail, base, frp, bpp)
	size_t	hdrsz;
	size_t	datasz;
	uint	bpri;
	int	canfail;
	caddr_t	base;
	frtn_t	*frp;
	mblk_t	**bpp;
{
	mblk_t	*hmp = NULL;
	mblk_t	*dmp;
	size_t	totalsz;
	int	error = 0;

	ASSERT(hdrsz || datasz);
	ASSERT(!base == !frp);
	ASSERT(!base || base && datasz);

	*bpp = NULL;
	totalsz = hdrsz + datasz;

	if (!base && totalsz <= PIGGYBACKSZ) {
		if ((error = rf_allocb(totalsz, bpri, canfail, NULLCADDR,
		  NULLFRP, &hmp)) != 0)	{
			return error;
		}
		*bpp = hmp;
		if (hdrsz) {
			hmp->b_wptr = hmp->b_rptr + hdrsz;
			bzero((caddr_t)hmp->b_rptr, hdrsz);
		}
		return 0;
	}

	if (hdrsz) {
		if ((error = rf_allocb(hdrsz, bpri, canfail, NULLCADDR,
		  NULLFRP, &hmp)) != 0)	{
			return error;
		}
		*bpp = hmp;
		hmp->b_wptr = hmp->b_rptr + hdrsz;
		bzero((caddr_t)hmp->b_rptr, hdrsz);
	}

	if (datasz) {
		if ((error = rf_allocb(datasz, bpri, canfail, base, frp, &dmp))
		  != 0) {
			if (hmp) {
				*bpp = NULL;
				rf_freeb(hmp);
			}
			return error;
		}
		if (hmp) {
			hmp->b_cont = dmp;
		} else {
			*bpp = dmp;
		}
	}
	return 0;
}
 
void
rf_freemsg(bp)
	mblk_t	*bp;
{
	if (bp) {
		ASSERT(bp->b_next == bp->b_prev);
		ASSERT(bp->b_next == bp || bp->b_next == NULL);
		bp->b_next = bp->b_prev = NULL;
		freemsg(bp);
	}
}

/*
 * Data is either piggybacked with headers or begins in the next message
 * block.  Yields NULL if no data part.
 */
caddr_t
rf_msgdata(bp, hdrsz)
	register mblk_t	*bp;
	register size_t	hdrsz;
{
	register unchar	*endhdr;
	register unchar	*wptr;

	/* The write pointer may or may not have been updated to cover data. */

	endhdr = bp->b_rptr + hdrsz;
	wptr = bp->b_wptr;

	if (endhdr == wptr) {

		/* Write pointer covers only headers */

		if (wptr == bp->b_datap->db_lim) {
			register mblk_t	*contbp;

			/* Underlying storage is only big enough for headers */

			if ((contbp = bp->b_cont) != NULL) {
				return (caddr_t)contbp->b_rptr;
			} else {
				return NULL;
			}
		} else {

			/*
			 * Underlying storage has some data in it, but the
			 * data is not covered by the write pointer.
			 */

			ASSERT(wptr < bp->b_datap->db_lim);
			return (caddr_t)wptr;
		}
	} else {

		/* Write pointer covers headers and data. */

		ASSERT(endhdr < wptr);
		return (caddr_t)endhdr;
	}
}

/*
 * Convert *giftp to denoted rd, or NULL if *giftp is invalid.  vcver
 * is the RFS version number of the virtual circuit over which the
 * gift was given.
 */
rcvd_t	*
rf_gifttord(giftp, vcver)
	rf_gift_t	*giftp;
	int		vcver;
{
	long		id;
	long		gen;
	rcvd_t		*rd;

	id = giftp->gift_id;
	gen = giftp->gift_gen;

	if (id < 0 || id >= nrcvd) {
		return NULL;
	}

	/*
	 * It is tempting to depend on the implementation of the old
	 * protocol, where gens (connids in those days) are set
	 * but unused, but we really have to trust old hosts more
	 * than we would like.
	 */

	rd = rcvd + id;
	if (vcver >= RFS2DOT0 && rd->rd_gen != gen ||
	  rd->rd_stat == RDUNUSED) {
		return NULL;

	}
	return rd;
}

/*
 * Convert *giftp to a vp, or NULL if *giftp does not denote a receive
 * descriptor with a vp.  vcver is the RFS version number of the virtual
 * circuit over which the gift was given.
 */
vnode_t	*
rf_gifttovp(giftp, vcver)
	rf_gift_t	*giftp;
	int		vcver;
{
	rcvd_t		*rd;

	/*
	 * We don't worry about distinguished rds (e.g. DAEMON_RD) and their
	 * vps, because those vps are always NULL.
	 */

	return (rd = rf_gifttord(giftp, vcver)) == NULL ? NULL : rd->rd_vp;
}

/*
 * Allocate and intialize iovecs and uio_iovcnt, uio_iov, and uio_resid
 * describing data in message starting in bp.  Assumes bp contains only
 * data, no headers.  rf_maxkmem isn't checked because the allocation here
 * is transitory.
 *
 * Caller must remember uio_iovec and uio_iovcnt, and RF_IOV_FREE the iovec.
 */
void
rf_iov_alloc(uiop, bp)
	register uio_t		*uiop;
	register mblk_t		*bp;
{
	register iovec_t	*iovp;
	register int		niov;
	register mblk_t		*databp;

	uiop->uio_resid = 0;

	niov = 0;
	for (databp = bp ; databp; databp = databp->b_cont) {
		niov++;
	}

	iovp = (iovec_t *)kmem_alloc(niov * sizeof(iovec_t), KM_SLEEP);
	uiop->uio_iov = iovp;
	uiop->uio_iovcnt = niov;

	for (databp = bp; databp; databp = databp->b_cont, iovp++) {
		iovp->iov_base = (caddr_t)databp->b_rptr;
		uiop->uio_resid +=
		  (iovp->iov_len = databp->b_wptr - databp->b_rptr);
		ASSERT(iovp <= uiop->uio_iov + (niov - 1) * sizeof(iovec_t));
	}
	return;
}

/*
 * Drop nbytes out of bp.  Return balance or NULL if nbytes >= content of bp.
 */
mblk_t *
rf_dropbytes(bp, nbytes)
	register mblk_t	*bp;
	register size_t	nbytes;
{
	register size_t	bsize;
	register mblk_t	*contbp;

	while (nbytes && bp) {
		bsize =	bp->b_wptr - bp->b_rptr;
		if (bsize > nbytes) {
			bp->b_rptr += nbytes;
			break;
		} else {
			contbp = bp->b_cont;
			bp->b_cont = NULL;
			rf_freeb(bp);
			bp = contbp;
			nbytes -= bsize;
		}
	}
	return bp;
}

/*
 * Return reference to an initialized rf_frtn structure. Can sleep.
 *	work.rfaw_func == func
 *	work.rfaw_farg == farg
 * rf_maxkmem isn't checked because the allocation here is transitory.
 */
rf_frtn_t *
rffr_alloc(func, farg)
	void			(*func)();
	caddr_t			farg;
{
	register rf_frtn_t	*frp;

	frp = (rf_frtn_t *)kmem_alloc(sizeof(rf_frtn_t), KM_SLEEP);
	LS_INIT(&frp->work.rfaw_elt);
	frp->work.rfaw_func = func;
	frp->work.rfaw_farg = farg;
	frp->frtn.free_func = (void (*)())rfa_workenq;
	frp->frtn.free_arg = (caddr_t)&frp->work.rfaw_elt;
	return frp;
}

/*
 * Return a pointer to an esballoc-ed streams message block holding
 * a reference to len bytes of locked kernel virtual addresses
 * for file offset off.  The message is set up to free the pages
 * when it is freed.  If hdrsz is 0, no header part will be allocated.
 *
 * off + len is not allowed to cross a MAXBSIZE boundary.  Can sleep.
 */
int
rfesb_fbread(vp, off, len, rw, hdrsz, pri, canfail, bpp)
	vnode_t		*vp;
	off_t		off;
	size_t		len;
	enum seg_rw	rw;
	size_t		hdrsz;
	uint		pri;
	int		canfail;
	mblk_t		**bpp;
{
	/* We optimize for the single-page case, avoiding fbread overhead. */

	int		onepage;
	off_t		poff;
	struct fbuf	*fbp = (struct fbuf *)NULL;
	rf_frtn_t	*rfrp;
	int		error;
	rf_fba_t	*rfbap;
	page_t		*pp;
	caddr_t		addr;

	*bpp = (mblk_t *)NULL;
	poff = off & PAGEMASK;
	onepage = poff + PAGESIZE >= off + len;

	if (onepage && (pp = page_lookup(vp, poff)) != NULL) {

		/*
		 * Allocate a structure to allow the page to be released
		 * later.
		 */

		PAGE_HOLD(pp);
		ASSERT(pp->p_keepcnt > 0);
		rfrp = rffr_alloc(rfesb_pagerele, (caddr_t)pp);
		addr = rfc_pptokv(pp) + (off & PAGEOFFSET);

	} else if ((error = fbread(vp, off, len, rw, &fbp)) == 0) {

		/*
		 * We've now got nbytes locked down starting at fb_addr.
		 * Allocate a structure to allow the page(s) to be
		 * fbrelsed later.
		 * rf_maxkmem isn't checked because the allocation here 
		 * is transitory.
		 */

		rfrp = rffr_alloc(rfesb_fbrelse, kmem_alloc(sizeof(rf_fba_t),
		  KM_SLEEP));
		rfbap = (rf_fba_t *)rfrp->work.rfaw_farg;
 		ASSERT(fbp);
		rfbap->fbp = fbp;
		rfbap->rw = rw;
		addr = fbp->fb_addr;
	} else {
 		ASSERT(!fbp);
		return error;
	}

	error = rf_allocmsg(hdrsz, len, pri, canfail, addr, &rfrp->frtn, bpp);

	ASSERT(!error != !*bpp);

	if (error) {
		if (onepage) {
			ASSERT(pp->p_keepcnt > 0);
			PAGE_RELE(pp); 
		} else {
			fbrelse(rfbap->fbp, rfbap->rw);
			kmem_free(rfrp->work.rfaw_farg, sizeof(rf_fba_t));
		}
		RFFR_FREE(rfrp);
	}
	return error;
}

/* Destroys *wp. */
STATIC void
rfesb_fbrelse(wp)
	rfa_work_t 		*wp;
{
	register rf_fba_t	*rfbap = (rf_fba_t *)wp->rfaw_farg;

	fbrelse(rfbap->fbp, rfbap->rw);
	kmem_free((caddr_t)rfbap, sizeof(rf_fba_t));
	RFFR_FREE(WTOFR(wp));
}

/* Destroys *wp. */
STATIC void
rfesb_pagerele(wp)
	rfa_work_t 	*wp;
{
	ASSERT(((page_t *)wp->rfaw_farg)->p_keepcnt > 0);
	PAGE_RELE((page_t *)wp->rfaw_farg);
	RFFR_FREE(WTOFR(wp));
}

/*
 * Return a pointer to an esballoc-ed streams message block covering
 * the addresses [bufp->b_un.b_addr + off .. bufp->b_un.b_addr + off + len).
 * The message is set up to free the pages when it is freed.  If hdrsz is
 * 0, no header part will be allocated.
 */

int
rfesb_pageio_setup(bufp, hdrsz, off, len, pri, canfail, bpp)
	buf_t	*bufp;
	size_t	hdrsz;
	off_t	off;
	size_t	len;
	uint	pri;
	int	canfail;
	mblk_t	**bpp;
{
	rf_frtn_t	*rfrp;
	int		error;

	ASSERT(bufp->b_flags & (B_ASYNC | B_REMAPPED | B_WRITE));
	rfrp = rffr_alloc(rfesb_pvn_done, (caddr_t)bufp);
	error = rf_allocmsg(hdrsz, len, pri, canfail, bufp->b_un.b_addr + off,
	  &rfrp->frtn, bpp);
	ASSERT(!error != !*bpp);
	if (error) {
		RFFR_FREE(rfrp);
	}
	return error;
}

STATIC void
rfesb_pvn_done(wp)
	rfa_work_t	*wp;
{
	ASSERT(((buf_t *)wp->rfaw_farg)->b_flags &
	  (B_ASYNC | B_REMAPPED | B_WRITE));
	pvn_done((buf_t *)wp->rfaw_farg);
	RFFR_FREE(WTOFR(wp));
}

/*
 * Allocate a message block of size bytes.  If base is non-NULL, so
 * must be frp, and we then use esballoc to cover the denoted storage.
 * Returns errno for failure.
 */
int
rf_allocb(size, bpri, canfail, base, frp, bpp)
	size_t	size;
	uint	bpri;
	int	canfail;
	caddr_t	base;
	frtn_t	*frp;
	mblk_t	**bpp;
{
	mblk_t	*bp = NULL;
	int	pri = canfail ?  (PREMOTE | PCATCH | PNOSTOP) : PZERO;
	int	error = 0;
	int	strid;

	ASSERT(!base == !frp);

	for (;;) {
		if (!base) {
			if ((bp = allocb((int)size, bpri)) != NULL) {
				break;
			}
			if (!(strid = bufcall(size, bpri, (int(*)())setrun,
			  (long)u.u_procp))) {
				if (canfail) {
					error = ENOMEM;
					break;
				}
				delay(100);
				continue;
			}
		} else {
			if ((bp = esballoc((unchar *)base, size, bpri, frp))
			  != NULL) {
				break;
			}
			if (!(strid = esbbcall(bpri, setrun, (long)u.u_procp))) {
				if (canfail) {
					error = ENOMEM;
					break;
				}
				delay(100);
				continue;
			}
		}
		if (sleep((caddr_t)&(u.u_procp->p_flag), pri)) {
			ASSERT(canfail);
			unbufcall(strid);
			error = EINTR;
			break;
		}
	}
	*bpp = bp;
	return error;
}

void
rf_freeb(bp)
	mblk_t	*bp;
{
	if (bp) {
		ASSERT(bp->b_next == bp->b_prev);
		ASSERT(bp->b_next == bp || bp->b_next == NULL);
		bp->b_next = bp->b_prev = NULL;
		freeb(bp);
	}
}

/* Create a receive descriptor, putting value in *rdpp.
 * Usually returns 0.  Only if canfail and no more rcvds, returns ENOMEM,
 * NULLing *rdpp.
 * NOTE:  rcvd_create creates reserved_rd.  That call must use canfail.
 */
int
rcvd_create(canfail, type, rdpp)
	register int canfail;
	register int type;
	register rcvd_t **rdpp;
{
	register rcvd_t *rdp;	/*  return value of the function  */

	while (!(rdp = rd_freelist)) {
		if (canfail) {
			*rdpp = NULL;
			cmn_err(CE_NOTE, "rcvd_create: not enough rcvds\n");
			return ENOMEM;
		} else if (!(reserved_rd->rd_stat & RDLOCKED)) {
			rdp = reserved_rd;
			break;
		} else {
			reserved_rd->rd_stat |= RDWANT;
			sleep((caddr_t)&reserved_rd, PZERO);
		}
	}
	*rdpp = rdp;
	rdp->rd_stat = RDUSED;
	if (rdp != reserved_rd) {
		rd_freelist = rdp->rd_next;
		rdp->rd_next = NULL;
		rd_nfree--;
	} else {
		rdp->rd_stat |= RDLOCKED;
	}
	rdp->rd_qcnt = 0;
	rdp->rd_vp = NULL;
	rdp->rd_refcnt = 1;
	rdp->rd_qtype = (char)type;
	rdp->rd_user_list = NULL;
	LS_INIT(&rdp->rd_rcvdq);
	rdp->rd_qslp = 0;
	rdp->rd_mtime = 0;
	if (type == RDSPECIFIC) {
		rdp->rd_un.rdun_sdp = NULL;
	} else {
		rdp->rd_un.rdun_vp = NULL;
	}
	return 0;
}

/*
 * Give up nrefs references to the denoted receive descriptor, freeing it and
 * NULLing *rdpp when it becomes unused.
 *
 * mntid is non-negative iff there is an rduser structure associated
 * with the reference.  In that case, sysid disambiguates references,
 * is ignored otherwise.
 *
 * NOTE:  callers giving up references to the attached vnode should
 * do so AFTER the call here.
 */
void
rcvd_delete(rdpp, sysid, mntid, nrefs, crp)
	rcvd_t			**rdpp;
	register sysid_t	sysid;
 	long			mntid;
	unsigned                nrefs;
	register cred_t		*crp;
{
	register rcvd_t		*rdp = *rdpp;

	ASSERT(rdp);
	ASSERT(rdp->rd_stat != RDUNUSED);

	if (mntid >= 0) {
		/*
		 * Otherwise no rduser structure associated with
		 * the reference.
		 */
		rdu_del(rdp, sysid, mntid, nrefs, crp);
	}
	if (!(rdp->rd_refcnt -= nrefs)) {
		rcvd_free(rdpp);
	}
}

/*
 * Return receive descriptor to the freelist.
 * NULL *rdpp;
 */
void
rcvd_free(rdpp)
	rcvd_t		**rdpp;
{
	register rcvd_t	*rdp = *rdpp;
	mblk_t		*straybp;

	if (rdp) {
		ASSERT(rdp->rd_stat != RDUNUSED);
		rdp->rd_gen++;

		/* Take care of races with rf_deliver */

		while ((straybp = rf_dequeue(rdp)) != NULL) {
			rfd_stray(straybp);
		}

		if (rdp->rd_qtype == RDGENERAL && rdp->rd_vp) {
			VN_RELE(rdp->rd_vp);
			rdp->rd_vp = NULL;
		}

		if (rdp == reserved_rd) {
			if (rdp->rd_stat & RDWANT) {
				wakeprocs((caddr_t)&reserved_rd, PRMPT);
			}
		} else {
			rd_nfree++;
			rdp->rd_next = rd_freelist;
			rd_freelist = rdp;
		}
		rdp->rd_stat = RDUNUSED;
		rdp->rd_user_list = NULL;
		*rdpp = NULL;
	}
}

/*
 * Post signals in rp received on virtual circuit version vcver against p.
 */
void
rf_postrpsigs(rp, vcver, p)
	register rf_response_t	*rp;
	int			vcver;
	register proc_t		*p;
{
	k_sigset_t		ksig;
	register int		sig;

	rf_getrpsigs(rp, vcver, &ksig);
	for (sig = 1; sigismember(&fillset, sig); sig++) {
		if (sigismember(&ksig, sig)) {
			psignal(p, sig);
			sigdelset(&ksig, sig);
		}
	}
}

/*
 * Delete sig and associated data structures from context of p.
 */
void
rf_delsig(p, sig)
	struct proc	*p;
	int		sig;
{
	if (p->p_cursig == sig) {
		p->p_cursig = 0;
		if (p->p_curinfo) {
			ASSERT(p->p_curinfo->sq_info.si_signo == sig);
			kmem_free((caddr_t)p->p_curinfo, sizeof(*p->p_curinfo));
			p->p_curinfo = NULL;
		}
	}
	sigdelset(&p->p_sig, sig);
	sigdelq(p, sig);
}

#ifdef DEBUG
int
rdu_match(rdup, sysid, mntid)
	register rd_user_t	*rdup;
	sysid_t			sysid;
	long			mntid;
{
	sysid_t			rusysid;
	long			rumntid;
	queue_t			*ruqp;
	gdp_t			*rugdpp;

	ASSERT(rdup);
	ruqp = rdup->ru_queue;
	rugdpp = QPTOGP(ruqp);
	rusysid = rugdpp->sysid;
	rumntid = rdup->ru_srmntid;
	return rusysid == sysid && rumntid == mntid;
}
#endif

/* rd denotes a gift of the resource in the srmount structure identified
 * by the sysid and mntid.
 * Return an rd_user structure to keep track of it, or NULL if unable to
 * allocate an rd_user structure.
 *
 * NOTE:  can't sleep.  See srmnt().
 */
rd_user_t *
rdu_get(rd, sysid, mntid, qp)
	register rcvd_t *rd;
	sysid_t sysid;
	register long mntid;
	register queue_t *qp;
{
	register rd_user_t *rduptr;

	/* If we've already given this rd to the same client, use the
	 * same rd_user structure, incrementing the reference count.
	 * Otherwise point rd at new structure and initialize it.
	 */
	if ((rduptr = rdu_find(rd, sysid, mntid, (rd_user_t **)NULL))
	  != NULL) {
		/* got one in use */
		rduptr->ru_vcount++;
		ASSERT(rduptr->ru_queue == qp);
		return rduptr;
	} else if ((rduptr = rdu_freelist) == NULL) {
		cmn_err(CE_WARN, "rdu_get:  Out of rd_user space\n");
		return NULL;
	} else {
		rduptr->ru_srmntid = mntid;
		rduptr->ru_vcount = 1;
		rduptr->ru_queue = qp;
		rduptr->ru_frwcnt = rduptr->ru_frcnt = rduptr->ru_fwcnt = 0;
		rduptr->ru_flag = 0;
		rdu_freelist = rdu_freelist->ru_next;
		rduptr->ru_next = NULL;
		if (!rd->rd_user_list) {
		rd->rd_user_list = rduptr;
		} else {
			rd_user_t *pred;

			for (pred = rd->rd_user_list;
			  pred->ru_next;
			  pred = pred->ru_next) {
				;
			}
			pred->ru_next = rduptr;
		}
		return rduptr;
	}
}

/*
 * Free  rd_user structure, NULL *rdupp.
 */
STATIC void
rdu_free(rdupp)
	rd_user_t		**rdupp;
{
	register rd_user_t	*rdup = *rdupp;

	rdup->ru_gen++;
	rdup->ru_next = rdu_freelist;
	rdu_freelist = rdup;
	*rdupp = NULL;
}

/*
 * rdu_find() returns a pointer to the rduser structure corresponding
 * to the given rd, sysid and mntid, or NULL if no such structure
 * was found. If prev_rdupp is not NULL, the previous rduser structure
 * (in the list) is returned in that parameter.
 */
rd_user_t *
rdu_find(rd, sysid, mntid, prev_rdupp)
	register rcvd_t	*rd;
	register sysid_t sysid;
	register long mntid;
	rd_user_t **prev_rdupp;
{
	rd_user_t *match_rdup = rd->rd_user_list;
	rd_user_t *pred_rdup = NULL;

	while (match_rdup) {
		if (rdu_match(match_rdup, sysid, mntid)) {
			break;
		}
		pred_rdup = match_rdup;
		match_rdup = match_rdup->ru_next;
	}
	if (match_rdup && prev_rdupp) {
		*prev_rdupp = pred_rdup;
	}
	return match_rdup;
}

/*
 * rdu_open() increments file, read and write counts for the given rd,
 * sysid, mntid, and fmode.
 */
void
rdu_open(rd, sysid, mntid, fmode)
	register rcvd_t		*rd;
	register sysid_t	sysid;
	register long		mntid;
	register int		fmode;
{
	rd_user_t		*rdup;

	if (!(rdup = rdu_find(rd, sysid, mntid, (rd_user_t **)NULL))) {
		/* error? */
		return;
	}
	if ((fmode & (FREAD | FWRITE)) == (FREAD | FWRITE)) {
		rdup->ru_frwcnt++;
	} else if (fmode & FREAD) {
		rdup->ru_frcnt++;
	} else {
		rdup->ru_fwcnt++;
	}
}

/*
 * A client is giving up nrefs references to this RD in srmount structure
 * denoted by sysid and mntid;  decrement count in rd_user struct by nrefs.
 * If it goes to zero, free it.
 */
void
rdu_del(rdp, sysid, mntid, nrefs, crp)
	register rcvd_t		*rdp;
	register sysid_t	sysid;
	register long		mntid;
	register unsigned	nrefs;
	register cred_t		*crp;
{
	rd_user_t		*rdup;
	rd_user_t		*pred_rdup;
	vnode_t			*vp = rdp->rd_vp;
	int                     rugen;
	char                    node[MAXDNAME];

	if (!(rdup = rdu_find(rdp, sysid, mntid, &pred_rdup))) {
		/* error? */
		return;
	}
	if ((rdup->ru_vcount -= nrefs) > 0) {
		return;
	}

        /*
         * Undo any outstanding opens.
	 *
	 * Check rugen to catch races with nasty clients.  Remember
	 * client name for squawking.  (It could be gone later.)
	 */

	rugen = rdup->ru_gen;
	(void)strcpy(node, QPTOGP(rdup->ru_queue)->token.t_uname);

	while (rdup->ru_frwcnt) {
		rdup->ru_frwcnt--;
		VOP_CLOSE(vp, FWRITE | FREAD, 1, 0, crp);
		if (rugen != rdup->ru_gen) {
			goto err;
		}
	}
	while (rdup->ru_frcnt) {
		rdup->ru_frcnt--;
		VOP_CLOSE(vp, FREAD, 1, 0, crp);
		if (rugen != rdup->ru_gen) {
			goto err;
		}
	}
	while (rdup->ru_fwcnt) {
		rdup->ru_fwcnt--;
		VOP_CLOSE(vp, FWRITE, 1, 0, crp);
		if (rugen != rdup->ru_gen) {
			goto err;
		}
	}

        /* Remove any remaining mappings. */

	if(rfm_lock(rdup, node)) {
		goto err;
	}
	if (!LS_ISEMPTY(&rdup->ru_mapdlist) &&
	  rfm_empty(rdup, vp, node) == EPROTO) {
		goto err;
	}
        rfm_unlock(rdup);

	if (rugen == rdup->ru_gen) {

		/* last reference - get rid of rd_user */

		if (rdup == rdp->rd_user_list) {
			rdp->rd_user_list = rdup->ru_next;
		} else {
			pred_rdup->ru_next = rdup->ru_next;
		}
		rdu_free(&rdup);
		return;
	}
err:
	/* We'd like to disconnect, but where's the circuit now? */

	cmn_err(CE_NOTE, "rdu_del: bad file reference from %s", node);
}

/* Returns 1 if modes are allowed, 0 otherwise. */
int
rdu_modecheck(rdup, mode)
	register rd_user_t      *rdup;
	register int            mode;
{
	if (!rdup->ru_frcnt && !rdup->ru_fwcnt && !rdup->ru_frwcnt) {
		return 0;
	}
	if (mode & FREAD && !rdup->ru_frcnt && !rdup->ru_frwcnt) {
		return 0;
	}
	if (mode & FWRITE && !rdup->ru_fwcnt && !rdup->ru_frwcnt) {
		return 0;
	}
	return 1;
}

/*
 * In SVR4.0, vattr and rfattr have the same members though (for how long?) the,
 * rfattr is all longs.
 */
void
vtorf_attr(rap, vap)
	rf_attr_t	*rap;
	vattr_t		*vap;
{
	rap->rfa_mask = vap->va_mask;
	rap->rfa_type = vap->va_type;
	rap->rfa_mode = vap->va_mode;
	rap->rfa_uid = vap->va_uid;
	rap->rfa_gid = vap->va_gid;
	rap->rfa_fsid = vap->va_fsid;
	rap->rfa_nodeid = vap->va_nodeid;
	rap->rfa_nlink = vap->va_nlink;
	rap->rfa_size = vap->va_size;
	rap->rfa_atime = vap->va_atime;
	rap->rfa_mtime = vap->va_mtime;
	rap->rfa_ctime = vap->va_ctime;
	rap->rfa_rdev = vap->va_rdev;
	rap->rfa_blksize = vap->va_blksize;
	rap->rfa_nblocks = vap->va_nblocks;
	rap->rfa_filler[0] = vap->va_filler[0];
	rap->rfa_filler[1] = vap->va_filler[1];
	rap->rfa_filler[2] = vap->va_filler[2];
	rap->rfa_filler[3] = vap->va_filler[3];
	rap->rfa_filler[4] = vap->va_filler[4];
	rap->rfa_filler[5] = vap->va_filler[5];
	rap->rfa_filler[6] = vap->va_filler[6];
	rap->rfa_filler[7] = vap->va_filler[7];
}

void
rftov_attr(vap, rap)
	vattr_t		*vap;
	rf_attr_t	*rap;
{
	vap->va_mask = rap->rfa_mask;
	vap->va_type = rap->rfa_type;
	vap->va_mode = rap->rfa_mode;
	vap->va_uid = rap->rfa_uid;
	vap->va_gid = rap->rfa_gid;
	vap->va_fsid = rap->rfa_fsid;
	vap->va_nodeid = rap->rfa_nodeid;
	vap->va_nlink = rap->rfa_nlink;
	vap->va_size = rap->rfa_size;
	vap->va_atime = rap->rfa_atime;
	vap->va_mtime = rap->rfa_mtime;
	vap->va_ctime = rap->rfa_ctime;
	vap->va_rdev = rap->rfa_rdev;
	vap->va_blksize = rap->rfa_blksize;
	vap->va_nblocks = rap->rfa_nblocks;
	vap->va_vcode = 0;
	vap->va_filler[0] = rap->rfa_filler[0];
	vap->va_filler[1] = rap->rfa_filler[1];
	vap->va_filler[2] = rap->rfa_filler[2];
	vap->va_filler[3] = rap->rfa_filler[3];
	vap->va_filler[4] = rap->rfa_filler[4];
	vap->va_filler[5] = rap->rfa_filler[5];
	vap->va_filler[6] = rap->rfa_filler[6];
	vap->va_filler[7] = rap->rfa_filler[7];
}

/*
 * Given a vnode pointer, yield a pointer to a receive descriptor representing
 * a remote reference to that vnode, or NULL if there is no such rd.
 */
rcvd_t *
vtord(vp)
	register vnode_t	*vp;
{
	/*
	 * TO DO:  We MUST sort rcvds and avoid the linear search.
	 */
	register rcvd_t		*rdp = rcvd;
	register rcvd_t		*endrdp = rcvd + nrcvd;

	while (rdp < endrdp) {
		if (rdp->rd_stat & RDUSED && rdp->rd_qtype & RDGENERAL &&
		  rdp->rd_vp == vp) {
			return rdp;
		} else {
			rdp++;
		}
	}
	return NULL;
}

/*
 * Dequeue an mblk from the denoted rcvd.  If the rcvd is RDGENERAL and is then
 * empty, remove it from the server work list.
 * Returns the mblk from the queue, possibly NULL.
 */
mblk_t *
rf_dequeue(rd)
	register rcvd_t	*rd;
{
	mblk_t		*result;

	ASSERT(!rd->rd_qslp++);

	result = (mblk_t *)LS_REMQUE(&rd->rd_rcvdq);
	rd->rd_qcnt--;
	if (RCVDEMP(rd) && rd->rd_qtype & RDGENERAL) {
		/*
		 * If rcvd is empty, remove it from servers' work list.
		 */
		rfsr_rmmsg(rd);
	}

	ASSERT(!--rd->rd_qslp);
	return result;
}

/* Initialize the communications data structures;
 * mostly list setup, also some well-known RD & SD setup
 */
int
rf_comminit()
{
	extern rcvd_t *sigrd;
	register sndd_t *tmp;
	register rcvd_t *rd;
	rcvd_t		*tmprdp;
	register int	nrd;

	if (!sndd || !rcvd || !gdp || !rd_user) {
		return ENOMEM;
	}
	if (nsndd <= maxserve) {
		maxserve = nsndd - 1;
		cmn_err(CE_WARN,
		  "maxserve changed to %d - not enough send descriptors\n",
		  maxserve);
	}
	for (nrd = 0; nrd < nrduser - 1; nrd++) {
		LS_INIT(&rd_user[nrd].ru_mapdlist);
		rd_user[nrd].ru_next = &rd_user[nrd+1];
	}
	if (nrduser >= 1) {
		rd_user[nrduser - 1].ru_next = NULL;
		rdu_freelist = rd_user;
	} else {
		rdu_freelist = NULL;
	}
	if (maxserve < minserve) {
		minserve = maxserve;
		cmn_err(CE_WARN,
		  "minserve changed to %d  (maxserve)\n", minserve);
	}

	LS_INIT(&sd_freelist);
	{
		register sndd_t *endsndd = sndd + nsndd;

		for (tmp = sndd; tmp  < endsndd; tmp++) {
			tmp->sd_stat = SDUNUSED;
			LS_INIT(&tmp->sd_free);
			LS_INIT(&tmp->sd_hash);
			LS_INS_AFTER(&sd_freelist, &tmp->sd_free);
		}
	}
	sd_nfree = nsndd;

	{
		register rcvd_t *endrcvd = rcvd + nrcvd;

		for (rd = rcvd; rd < endrcvd; rd++) {
			rd->rd_stat = RDUNUSED;
			rd->rd_gen = 0;
			rd->rd_next = rd + 1;
		}
	}
	rcvd[nrcvd - 1].rd_next = NULL;
	rd_freelist = rcvd;
	rd_nfree = nrcvd;

	/* create well-known RDs & SDs */

	(void)rcvd_create(TRUE, RDGENERAL, &mountrd);
	(void)rcvd_create(TRUE, RDGENERAL, &sigrd); /* signals */
	(void)rcvd_create(TRUE, RDSPECIFIC, &rf_daemon_rd);  /* rf_daemon */

	/*
         * Use a tmp so that rcvd_create correctly updates rd_freelist and
	 * rd_nfree.
	 */

	(void)rcvd_create(TRUE, RDSPECIFIC, &tmprdp);
	reserved_rd = tmprdp;
	(void)sndd_create(TRUE, &reserved_sd);
	return 0;
}


/* De-initialize communications data structures. */
void
rf_commdinit()
{
 	/*
         * the following are used so that the reserved descriptors can be
	 * entirely freed.
	 */
	rcvd_t *tmp_res_rd = reserved_rd;
	sndd_t *tmp_res_sd = reserved_sd;

	DUPRINT1(DB_RFSTART, "rf_commdinit \n");
	rcvd_free(&mountrd);
	rcvd_free(&sigrd);
	rcvd_free(&rf_daemon_rd);
	reserved_rd->rd_stat = RDUSED;
	reserved_rd = NULL;
	rcvd_free(&tmp_res_rd);
	reserved_sd->sd_stat = SDUSED;
	reserved_sd = NULL;
	sndd_free(&tmp_res_sd);

	if (rfcl_lookup_cache.lkc_crp) {
		crfree(rfcl_lookup_cache.lkc_crp);
		rfcl_lookup_cache.lkc_crp = NULL;
	}
#ifdef DEBUG
	{
		register sndd_t *endsndd = sndd + nsndd;
		register sndd_t	*sdp;

		for (sdp = sndd; sdp  < endsndd; sdp++) {
			if (SDTOV(sdp)->v_pages) {
				cmn_err(CE_NOTE, "rf_commdinit pages on sd %x",
				  sdp);
			}
		}
	}
#endif
}

/*
 * Put arriving message in right rcvd queue.
 */
void
rf_deliver(bp)
	register mblk_t		*bp;
{
	register rf_message_t	*msgp = RF_MSG(bp);
	register rcvd_t		*rd;

	extern rcvd_t		*sigrd;
	extern int		rf_daemon_flag;

	if ((rd = rf_gifttord(&msgp->m_dest, QPTOGP(msgp->m_queue)->version)) ==
	   NULL ||
	  rd->rd_stat == RDUNUSED) {
		rfd_stray(bp);
		return;
	}
	if (msgp->m_stat & RF_SIGNAL) {
		rd = sigrd;
		ASSERT(rd->rd_stat != RDUNUSED);
	}

	ASSERT(!rd->rd_qslp++);

	LS_INIT(bp);
	LS_INSQUE(&rd->rd_rcvdq, bp);
	rd->rd_qcnt++;
	if (rd->rd_qtype & RDSPECIFIC) {
		wakeprocs((caddr_t)&rd->rd_qslp, PRMPT);
	} else {
		proc_t	*found = rfsr_idle_procp;

		/* ASSERT(rfsr_nidle == rfsr_listcount(rfsr_idle_procp)); */
		ASSERT(!rfsr_idle_lock++);

		/*
		 * put message where it can be found and dispatch server
		 */
		rfsr_addmsg(rd);
		if (found) {
			/*
			 * Remove the server from the idle list here to
			 * avoid overestimating the number of idle
			 * servers due to rapidly arriving messages.
			 * Server will put itself on the active list
			 * when it starts work.
			 *
			 * Servers neither on the active or idle lists are
			 * those looking for work, but not sleeping.
			 */
			rfsr_idle_procp = found->p_rlink;
			found->p_rlink = NULL;
			rfsr_nidle--;
			wakeprocs((caddr_t)&found->p_srwchan, PRMPT);
		} else if (rfsr_nservers < maxserve) {
			rf_daemon_flag |= RFDSERVE;
			wakeprocs((caddr_t)&rf_daemon_rd->rd_qslp, PRMPT);
		}

		ASSERT(!--rfsr_idle_lock);
		/* ASSERT(rfsr_nidle == rfsr_listcount(rfsr_idle_procp)); */
	}

	ASSERT(!--rd->rd_qslp);
}

#ifdef DEBUG
/* Starting at sdp, print n sndds */
void
sndd_print(sdp, n)
	register sndd_t	*sdp;
	unsigned	n;
{
	register sndd_t	*end = sdp + n - 1;

	if (sdp < sndd ||
	  ((caddr_t)sdp - (caddr_t)sndd) % sizeof(sndd_t) ||
	  sdp >= end) {
		cmn_err(CE_CONT, "bad args\n");
		return;
	}
	for (; sdp < end; sdp++) {
		cmn_err(CE_CONT, "\nhash\t\t\t\tfree\n");
		cmn_err(CE_CONT, "0x%x\t0x%x\t0x%x\t0x%x\n",
		  sdp->sd_hash.ls_next, sdp->sd_hash.ls_prev,
		  sdp->sd_free.ls_next, sdp->sd_free.ls_prev);
		cmn_err(CE_CONT, "size\t\tstat\t\tgift\n");
		cmn_err(CE_CONT, "0x%x\t0x%x\t0x%x\t0x%x\n",
		  sdp->sd_size, sdp->sd_stat,
		  sdp->sd_gift.gift_id, sdp->sd_gift.gift_gen);
		cmn_err(CE_CONT, "mntid\t\tsrvproc\t\tqueue\t\tfhandle\n");
		cmn_err(CE_CONT, "0x%x\t0x%x\t0x%x\t0x%x\n",
		  sdp->sd_mntid, sdp->sd_srvproc,
		  sdp->sd_queue,   sdp->sd_fhandle);
		cmn_err(CE_CONT,"vcode\t\tmapcnt\t\tnextr\t\tstashp\n");
		cmn_err(CE_CONT, "0x%x\t0x%x\t0x%x\n",
		  sdp->sd_vcode, sdp->sd_nextr, sdp->sd_stashp);
		cmn_err(CE_CONT, "remcnt\t\tcrwlock\t\tv_count\n");
		cmn_err(CE_CONT, "0x%x\t0x%x\t0x%x\t0x%x\n",
		  sdp->sd_remcnt, sdp->sd_crwlock, sdp->sd_vn.v_count);
	}
}
#endif
