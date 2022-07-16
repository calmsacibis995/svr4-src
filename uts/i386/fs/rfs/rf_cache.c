/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_cache.c	1.3.1.5"

#include "sys/list.h"
#include "sys/types.h"
#include "sys/sysinfo.h"
#include "sys/fs/rf_acct.h"
#include "sys/errno.h"
#include "sys/param.h"
#include "sys/cmn_err.h"
#include "sys/vnode.h"
#include "sys/debug.h"
#include "sys/cred.h"
#include "sys/sysmacros.h"
#include "sys/rf_messg.h"
#include "sys/uio.h"
#include "sys/stream.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/hetero.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_comm.h"
#include "vm/page.h"
#include "vm/pvn.h"
#include "vm/seg_vn.h"
#include "vm/seg_map.h"
#include "vm/rm.h"
#include "sys/mman.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "sys/buf.h"
#include "rf_cache.h"
#include "rfcl_subr.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/kmem.h"
#include "sys/inline.h"
#include "sys/fs/rf_vfs.h"

/*
 * RFS-specific operations on pages and the page-cache
 */

extern int	rf_state;
long		rfc_time;	/* number of ticks to elapse after an update
				 * before re-enabling cache; -1 implies
				 * no caching */

/*
 * Internal subroutines.
 */
STATIC int	rfc_readfill();
STATIC int	rfc_pageget();
STATIC int	rfc_readrp1();
STATIC int	rfc_readin();
STATIC int	rfc_readend();
STATIC void	rfc_dismsout();
STATIC void	rfc_pagelock();

/*
 * Get pointers to two (possibly empty) lists of held pages for the
 * specified file, offset, and length.
 *
 * We call the smallest interval containing all non-resident pages in
 * the request the non-resident interval.  Pages preceeding the non-resident
 * interval are the resident prefix; those following the non-resident
 * interval are the resident suffix.  Any (but not all!) of these may be
 * empty.
 *
 * Params
 *	vp	caller must not hold lock
 *	offset	page-aligned file offset
 *	len	page-aligned IO length, must be nonzero
 *	*pfxp	updated with pointer to resident prefix, possibly
 *		NULL; if non-NULL, sorted by file offset.
 *	*sfxp	updated with pointer to resident suffix, possibly
 *		NULL; if non-NULL, sorted by file offset.
 *
 * Can sleep.
 * Returns error or nonzero errno; in error cases, out params are undefined.
 */
int
rfc_pagelist(vp, offset, len, pfxp, sfxp)
	vnode_t		*vp;
	register off_t	offset;
	size_t		len;
	register page_t	**pfxp;
	register page_t	**sfxp;
{
	register off_t	end = offset + len;
	register off_t	lnrpoff = -1;	/* last non-resident page offset */
	register struct	page *pp;

	ASSERT(!(offset % PAGESIZE));
	ASSERT(!(len % PAGESIZE));

	*pfxp = *sfxp = NULL;
	for ( ; offset < end; offset += PAGESIZE) {
		rfc_info.rfci_ptread++;

		/*
		 * First find and hold all resident pages in the requested
		 * range.
		 *
		 * Build a resident prefix in *pfxp until we find a nonresident
		 * page, then put any further resident pages into *sfxp.
		 */

		if ((pp = page_lookup(vp, offset)) != NULL) {

			/* page_lookup doesn't hold the page. */

			PAGE_HOLD(pp);
			ASSERT(pp->p_keepcnt > 0);
			rfc_pagelock(pp);
			page_sortadd(lnrpoff == -1 ? pfxp : sfxp, pp);

		} else {
			rfc_info.rfci_pmread++;
			lnrpoff = offset;
		}
	}
	/*
	 * Now *pfxp holds any resident suffix, *sfxp all following pages.
	 * If we did not find all pages resident, lnrpoff holds the offset
	 * of the last nonresident page we looked for.  Release all resident
	 * pages in *sfxp that have offsets less than lnrpoff, leaving the
	 * resident suffix in *sfxp.
	 */
	if (lnrpoff != -1) {
		while ((pp = *sfxp) != NULL && pp->p_offset < lnrpoff) {
			rfc_info.rfci_pmread++;
			page_sub(sfxp, pp);
			rfc_pageunlock(pp);
			ASSERT(pp->p_keepcnt > 0);
			PAGE_RELE(pp);
		}
	} else {
		/*
		 * Cache hit.  Everything requested (if anything) is in *pfxp.
		 */
		/* EMPTY */
		ASSERT(!*sfxp);
	}
	return 0;
}

/*
 * Lookup the page denoted by vp and off, and sort it into ppp.
 * Holds and locks the page.  Returns a pointer to the page found
 * for success, NULL for failure.
 */
page_t *
rfc_page_lookup(vp, off, ppp)
	vnode_t	*vp;
	off_t	off;
	page_t	**ppp;
{
	page_t	*pp;

	ASSERT(!(off & PAGEOFFSET));

	pp = page_lookup(vp, off); 
	if (pp) {
		PAGE_HOLD(pp);
		ASSERT(pp->p_keepcnt > 0);
		page_lock(pp); 
		page_sortadd(ppp, pp);
	}
	return pp;
}

/*
 * Move data from incoming streams messages into page cache, and
 * from page cache into read destination.  Handles resident prefix
 * and suffix pages.  Handles copysync messages, needed only for pre-SVR4
 * servers.
 *
 * If *move_errorp is not already set, sets it for non-fatal errors.
 * Interrogates and sets *rf_reqap->rfrq_argp->ctlp for reads that should
 * restart or bypass the cache. (*rf_reqap->rfrq_argp->ctlp should be
 * RFC_INCACHE, initially.)  If *rf_reqap->rfrq_argp->ctlp is set to
 * another value,  or *move_errorp, drops data from incoming messages.
 *
 * Does bookeeping in uio structure passed in request structure.  Also
 * updates page lists embedded in read/write arg.
 *
 * Params:
 *	rf_rwap		RFS read/write arg.
 *	bpp		*bpp contains the response, is NULLed here.
 *	chansdp		where the original request was sent.
 *	*move_errorp	in/out, to intermediate error.
 *
 * Returns non-zero errno for fatal errors that keep it from continuing.
 */
int
rfc_readmove(rf_rwap, bpp, chansdp, move_errorp)
	register rf_rwa_t	*rf_rwap;
	register mblk_t		**bpp;
	register sndd_t		*chansdp;
	register int		*move_errorp;
{
	register rf_response_t	*resp = RF_RESP(*bpp);
	register int		move_error = *move_errorp;
	register rfc_ctl_t	ctl;
	long			opcode = RF_COM(*bpp)->co_opcode;
	long			copysync = resp->rp_copyout.copysync;
	rf_message_t		*mp = RF_MSG(*bpp);
	queue_t			*qp = (queue_t *)mp->m_queue;
	rf_gift_t		gift; 
	int			error = 0;

	/*
	 * Until we get the first response we don't know that we acquired the
	 * server vnode lock before a writer snuck in.  So we have to check
	 * now to make sure we haven't received a disable message in the
	 * interim.  Subsequently we don't have to check, because we hold the
	 * server vnode lock.
	 *
	 * If a disable message comes in after we relinquish the server vnode
	 * lock, our read will complete without interference, because we have
	 * already read the data we need on the server, which is consistent
	 * with that in earlier responses.
	 */
	if (rf_rwap->rd_nextnrb == rf_rwap->rd_firstnrb && !move_error) {
		move_error = rfc_readrp1(chansdp, rf_rwap);
	}
	gift = mp->m_gift;
	ctl = *rf_rwap->rd_ctlp;	/* may be set by rfc_readrp1 */
	if (ctl == RFC_INCACHE && !move_error &&
	  (opcode == RFCOPYOUT || !resp->rp_nodata)) {
		/*
		 * Move data from the streams message into the page cache
		 * and to the read destination.
		 */
		move_error = rfc_readin(chansdp, rf_rwap, bpp);
	}
	rf_freemsg(*bpp);
	*bpp = NULL;
	if (opcode == RFCOPYOUT && copysync) {
		error = rfcl_copysync(rf_rwap->replysdp, qp, &gift);
	} else if (opcode == RFREAD && ctl == RFC_INCACHE &&
	  !move_error && !error) {
		move_error = rfc_readend(chansdp, rf_rwap);
	}
	*move_errorp = move_error;
	return error;
}

/*
 * Special processing for first response to cached read request.
 *
 * Sets *rf_rwap->rd_ctlp to RFC_RETRY if it decides to restart the read.
 * Move any resident prefix to read address.
 *
 * Updates rf_rwap->(ctlp|nextnrb|pfx|uiop->stuff)
 */
STATIC int
rfc_readrp1(sdp, rf_rwap)
	sndd_t		*sdp;
	rf_rwa_t	*rf_rwap;
{
	/*
	 * We cannot rely on vcode to abort the cache read,
	 * because our own VOP_SPACEs, e.g., can update the vcode, but
	 * leave cache valid.
	 *
	 * The checks on pfx and sfx cover the case where the cache got
	 * disabled and reenabled while we were out to lunch,
	 * because then any resident pages got aborted.  If there
	 * were no resident pages, and the cache has been reenabled,
	 * and the file has not been truncated or grown, then we
	 * continue to read through the cache, and that's okay,
	 * because we are getting a consistent snapshot from the
	 * server.
	 */
	if (!(sdp->sd_stat & SDCACHE) ||
	  rf_rwap->rd_pfx && rfc_aborted(rf_rwap->rd_pfx) ||
	  rf_rwap->rd_sfx && rfc_aborted(rf_rwap->rd_sfx) ||
	  sdp->sd_size != rf_rwap->rd_sdsize) {
		/*
		 * The invoking context will get rid of held pages and retry
		 * after we drop the rest of the arriving data.
		 */
		*rf_rwap->rd_ctlp = RFC_RETRY;
	} else if (rf_rwap->rd_pfx) {
		/*
		 * Before moving any resident prefix to the read address,
		 * set nextnrb to the first page beyond the prefix.
		 */
		rf_rwap->rd_nextnrb =
			rf_rwap->rd_pfx->p_prev->p_offset + PAGESIZE;
		return rfc_plmove(&rf_rwap->rd_pfx, rf_rwap->uiop,
				rf_rwap->rd_nextnrb);
	}
	return 0;
}

/*
 * Move data from the denoted response message into the page cache
 * and to the read address.
 *
 * Updates rf_rwap->(infix|nextnrb|uiop->stuff)
 */
STATIC int
rfc_readin(sdp, rf_rwap, inbpp)
	register sndd_t		*sdp;
	register rf_rwa_t	*rf_rwap;
	mblk_t			**inbpp;
{
	register mblk_t		*bp;
	register uio_t		*uiop = rf_rwap->uiop;
	off_t			oldnextnrb = rf_rwap->rd_nextnrb;
	off_t			newnextnrb = rf_rwap->rd_nextnrb +
				  RF_RESP(*inbpp)->rp_count;
	gdp_t			*gp = QPTOGP(sdp->sd_queue);
	int			error = 0;

	if ((bp = rf_dropbytes(*inbpp, RF_MIN_RESP(gp->version))) == NULL ||
	  newnextnrb > MIN(rf_rwap->rd_endnrb, sdp->sd_size)) {
		*inbpp = NULL;
		rfc_pageabort(sdp, (off_t)0, (off_t)0);
		gdp_discon("rfc_readin bad response", gp);
		error = EPROTO;
	} else if (!(error = rfc_readfill(sdp, bp, &rf_rwap->rd_infix,
	    &rf_rwap->rd_nextnrb)) &&
	  uiop->uio_offset >= oldnextnrb &&
	  uiop->uio_offset < newnextnrb &&
	  uiop->uio_resid) {

		/*
		 * Move data from the streams message to the read
		 * address.
		 */

		iovec_t	*iovp;
		int	niov;
		uio_t	ruio;

		ASSERT(newnextnrb == rf_rwap->rd_nextnrb);

		/*
		 * First, set up by dropping from the start of the message
		 * any data not requested by the VOP_READ.
		 */

		bp = rf_dropbytes(bp, (size_t)(uiop->uio_offset - oldnextnrb));
		ASSERT(bp);

		/* Some meaningless fields here, offset, fmode, limit, e.g. */

		ruio.uio_iov = NULL;
		ruio.uio_iovcnt = 0;
		ruio.uio_offset = 0;
		ruio.uio_segflg = UIO_SYSSPACE;
		ruio.uio_fmode = 0;
		ruio.uio_limit = 0;

		rf_iov_alloc(&ruio, bp);
		iovp = ruio.uio_iov;
		niov = ruio.uio_iovcnt;

		ruio.uio_resid =
		  MIN(uiop->uio_resid, (newnextnrb - uiop->uio_offset));

		error = uiomvuio(&ruio, uiop);

		RF_IOV_FREE(iovp, niov);
	}
	*inbpp = bp;
	return error;
}

/*
 * Special processing for last response to cached read request.
 *
 * Check for premature EOF, aborting pages on sdp and disable caching for
 * the file if necessary.  Otherwise, if last page in read interval is
 * partially filled, bzero the balance and make the page resident  Move
 * any resident suffix to read address.
 *
 * Assumes earlier checks guarantee data is not overrunning request.
 *
 * Updates rf_rwap->(infix|nextnrb|sfx|uiop->stuff)
 */
STATIC int
rfc_readend(sdp, rf_rwap)
	sndd_t		*sdp;
	rf_rwa_t	*rf_rwap;
{
	int		error = 0;

	if (rf_rwap->rd_sfx && rf_rwap->rd_nextnrb != rf_rwap->rd_endnrb ||
	  !rf_rwap->rd_sfx &&
	   (rf_rwap->rd_nextnrb < rf_rwap->rd_endnrb - PAGESIZE ||
	   sdp->sd_size <= rf_rwap->rd_endnrb &&
	    sdp->sd_size > rf_rwap->rd_nextnrb)) {

		/*
		 * Premature EOF occurs if any of the following are true.
		 *   -	There is a resident suffix, and we failed to fill
		 *	the last nonresident page.
		 *   -	There is not a resident suffix and we did not get
		 *	onto the last page in the nonresident interval.
		 *   -  There is not a resident suffix, and we got onto
		 *	the last page requested, but our idea of EOF lies
		 *	in that page also, and we did not get data to take
		 *	us to EOF.
		 */

		gdp_discon("rfc_readend:  unexpected EOF",
		  QPTOGP(sdp->sd_queue));
		rfc_pageabort(sdp, (off_t)0, (off_t)0);
		/* CONSTCOND */
		rfc_disable(sdp, (ulong)0);
		error = EPROTO;

	} else if (rf_rwap->rd_infix) {
		off_t	on = rf_rwap->rd_nextnrb & PAGEOFFSET;
		size_t	nleft = PAGESIZE - on;
		page_t	*pp = rf_rwap->rd_infix;

		/* These two assert no premature EOF. */

		ASSERT(on);
		ASSERT(pp->p_next == pp);

		/* Zap the balance of the page and make it resident. */

		bzero(rfc_pptokv(pp) + on, nleft);
		rf_rwap->rd_nextnrb += nleft;
		page_sub(&rf_rwap->rd_infix, pp);
		rfc_pageunlock(pp);
		ASSERT(pp->p_keepcnt > 0);
		PAGE_RELE(pp);
		ASSERT(!rf_rwap->rd_infix);

	} else if (rf_rwap->rd_sfx) {

		/*
		 * Server sent all data we wanted, and now we have
		 * a resident suffix to attend to.
		 */

		error = rfc_plmove(&rf_rwap->rd_sfx, rf_rwap->uiop,
				(off_t)MIN(rf_rwap->rd_sfx->p_prev->p_offset +
					PAGESIZE, sdp->sd_size));
	}
	return error;
}

/*
 * Move data from pages in *plp according to uiop.  Knows about crossing
 * non-contiguous page boundaries.  Pages are removed from *plp and
 * released as they are consumed.
 */
int
rfc_plmove(plp, uiop, end_data)
	register page_t	**plp;
	register uio_t	*uiop;
	register off_t	end_data;
{
	register int error = 0;
	register off_t end_pages = ptob(btopr(end_data));
	register page_t *pp = *plp;

	while (pp && pp->p_offset < end_pages) {
		/*
		 * epd == end of page data for this page.
		 */
		register off_t	epd = MIN(PAGESIZE, end_data - pp->p_offset);

		if (rfc_aborted(pp)) {
			return EPROTO;
		}
		if (uiop->uio_offset >= pp->p_offset &&
		  uiop->uio_offset < pp->p_offset + epd &&
		  uiop->uio_resid > 0 &&
		  !error) {
			/*
			 * Pages are generally discontiguous, so we move only a
			 * page at a time.
			 */
			register off_t on = uiop->uio_offset & PAGEOFFSET;

			error = uiomove(rfc_pptokv(pp) + on,
					MIN(uiop->uio_resid, epd - on),
					UIO_READ, uiop);
		}
		if (end_data >= pp->p_offset + PAGESIZE) {

			/* Whole page. */

			page_sub(plp, pp);
			rfc_pageunlock(pp);
			ASSERT(pp->p_keepcnt > 0);
			PAGE_RELE(pp);
			pp = *plp;
		} else {
			/*
			 * We're onto a partial page.
			 */
			break;
		}
	}
	return error;
}

/*
 * Acquire and fill pages for vp according to uiop->uio_offset and nbytes.
 * Aborts current page in error cases.
 */
int
rfc_writefill(vp, uiop, nbytes)
	register vnode_t	*vp;
	uio_t			*uiop;
	size_t			nbytes;
{
	register int		error = 0;

	ASSERT(nbytes == uiop->uio_resid ||
	  uiop->uio_resid > nbytes &&
	   !(uiop->uio_offset + nbytes & PAGEOFFSET));

	while (!error && nbytes) {
		register off_t	poff = uiop->uio_offset & PAGEMASK;
		register off_t	pon = uiop->uio_offset & PAGEOFFSET;
		register page_t	*pp = page_lookup(vp, poff);
		register long	nbin = MIN(PAGESIZE - pon, nbytes);
		int		newp = 0;

		rfc_info.rfci_ptwrite++;

		/*
		 * We don't fill partial, non-resident pages, to
		 * avoid consistency problems and having to bring
		 * back pages from the server.
		 */

		if (pp) {
			/* page_get does a hold; page_lookup does not. */

			PAGE_HOLD(pp);
			ASSERT(pp->p_keepcnt > 0);
		} else {
			rfc_info.rfci_pmwrite++;
			if (poff == uiop->uio_offset &&
			  nbytes >= PAGESIZE &&
			  (pp = page_get(PAGESIZE, 1)) != NULL) {
				/*
				 * Above, 1 == can sleep.  Below,
				 * 0 == don't lock.  (We roll our own.)
				 */
				newp = 1;
			}
		}
		if (pp) {
			rfc_pagelock(pp);
			error = uiomove(rfc_pptokv(pp) + pon, nbin,
			  UIO_WRITE, uiop);
			rfc_pageunlock(pp);
			ASSERT(pp->p_keepcnt > 0);
			if (newp) {
				rfc_pageabort(VTOSD(vp), poff, poff+1);
				if (!error) {
					page_hashin(pp, vp, poff, 0);
				}
				PAGE_RELE(pp);
			} else {
				PAGE_RELE(pp);
				if (error) {
					rfc_pageabort(VTOSD(vp),
					  (off_t)pp->p_offset, 
					  (off_t)pp->p_offset+1);
				}
			}
		} else {
			/* Skip the page we don't have. */
			 uioskip(uiop, nbin);
		}
		nbytes -= nbin;
	}
	return error;
}

/*
 * Check to see if cache disable messages have to be sent to clients.  Do
 * so if necessary.
 */
void
rfc_disable_msg(vrdp, vcode)
	register rcvd_t		*vrdp;
	register ulong		vcode;
{
	register rd_user_t	*rdup;
	register rd_user_t	*rdup_next;
	rcvd_t			*rdp;
	sndd_t			*sdp;
	int			rdgen;

	/*
	 * Remember mtime for reenabling cache later.
	 */
	vrdp->rd_mtime = lbolt;
	(void)sndd_create(FALSE, &sdp);
	(void)rcvd_create(FALSE, RDSPECIFIC, &rdp);
	rdgen = vrdp->rd_gen;

	/*
	 * rdgen tells us if rd has been deallocated, wouldn't work if
	 * rcvds were kmem_allocked.
	 */
restart:
	if (vrdp && vrdp->rd_gen == rdgen && vrdp->rd_vp) {


		/*
		 * We allow the list to change under us.  rugen tells
		 * us if rdup_next was deallocated, wouldn't work if
		 * rd_users were kmem_allocked.
		 */

		/* Skip rd_users already processed. */

	for (rdup = vrdp->rd_user_list;
		  rdup && rdup->ru_flag & RU_DONE;
	  rdup = rdup->ru_next) {
			;
		}

		/* Send messages on unprocessed rd_users. */

		/* LINTED - stupid lint says rdup_next is uninitialized */
		for ( ; rdup; rdup = rdup_next) {
			int	rugen;

			rdup_next = rdup->ru_next;
			if (rdup_next) {
				rugen = rdup_next->ru_gen;
			}
			rdup->ru_flag |= RU_DONE;
		if (rdup->ru_flag & RU_CACHE_ON &&
		  (!RF_SERVER() || !rdu_match(rdup, u.u_procp->p_sysid,
		   u.u_srchan->sd_mntid))) {
			rdup->ru_flag &= ~RU_CACHE_ON;
			rdup->ru_flag |= RU_CACHE_DISABLE;
				rfc_dismsout(rdup, RDTOV(vrdp), sdp, rdp,
				  vcode);
		}
			if (rdup_next && rdup_next->ru_gen != rugen) {
				/* rdup_next was deallocated. */
				goto restart;
	}
		}

		/* Entire list was processed.  Now clean up. */

		if (vrdp && vrdp->rd_gen == rdgen) {
			for (rdup = vrdp->rd_user_list;
			  rdup;
			  rdup = rdup->ru_next) {
				rdup->ru_flag &= ~RU_DONE;
			}
		}
	}
	rcvd_free(&rdp);
	sndd_free(&sdp);
}

/*
 * Send a cache disable message to the client represented by rdup,
 * for the object vp.  Sdp is the send descriptor used to send the
 * message, rdp the receive descriptor to get the response.
 */
STATIC void
rfc_dismsout(rdup, vp, sdp, rdp, vcode)
	register rd_user_t	*rdup;
	vnode_t			*vp;
	register sndd_t		*sdp;
	register rcvd_t		*rdp;
	ulong			vcode;
{
	register rf_request_t	*request;
	register rf_common_t	*cop;
	register queue_t	*qp = rdup->ru_queue;
	register gdp_t		*gp = QPTOGP(qp);
	register size_t		minreq = RF_MIN_REQ(gp->version);
	mblk_t			*bp;
	int			rugen = rdup->ru_gen;

	rfc_info.rfci_snd_dis++;
	ASSERT(sdp->sd_stat & SDUSED);
	if (gp->constate != GDPCONNECT) {
		return;
	}
	sndd_set(sdp, qp, &rf_daemon_gift);
	rdp->rd_sdp = sdp;
	(void)rf_allocmsg(minreq, (size_t)0, BPRI_MED, FALSE, NULLCADDR,
	  NULLFRP, &bp);
	if (rdup->ru_gen != rugen) {
		/* rdup was deallocated. */
		rf_freemsg(bp);
		return;
	}
	ASSERT(bp);
	request = RF_REQ(bp);
	cop = RF_COM(bp);
	request->rq_cachedis.fhandle = (long)vp;
	request->rq_cachedis.vcode = vcode;
	cop->co_opcode = RFCACHEDIS;
	cop->co_type = RF_REQ_MSG;
	cop->co_mntid = rdup->ru_srmntid;
	if (rf_sndmsg(sdp, bp, minreq, rdp, FALSE) == 0) {
		(void)rf_rcvmsg(rdp, &bp);
	}
	rdp->rd_sdp = NULL;
	rf_freemsg(bp);
}

/*
 * Free the cached sndds for the denoted vfs, disabling client caching
 * for each sndd.
 */
void
rfc_sdabort(rfvfsp)
	register rf_vfs_t	*rfvfsp;
{
	register ls_elt_t	*vfhash = &rfvfsp->rfvfs_sdhash;

	while (!LS_ISEMPTY(vfhash)) {
		sndd_t	*sdp = HASHTOSD(vfhash->ls_next);

		sndd_unhash(sdp);
		/* CONSTCOND */
		rfc_disable(sdp, (ulong)0);
		sndd_free(&sdp);
	}
}

/*
 * Look for a cached send descriptor matching the reference in the response
 * denoted by rp.  Return the send descriptor with sd_queue, sd_stat, and
 * sd_gift initialized, or NULL for failure.
 */
sndd_t *
rfc_sdsearch(rfvfsp, rp, qp, giftp, size)
	rf_vfs_t		*rfvfsp;
	register rf_response_t	*rp;
	register queue_t	*qp;
	rf_gift_t		*giftp;
	off_t			size;
{
	register ls_elt_t	*vfhash = &rfvfsp->rfvfs_sdhash;
	register ls_elt_t	*sdhash = vfhash->ls_next;
	register long 		fhandle = rp->rp_fhandle;
	register ulong		vcode;
	sndd_t			*sdp;

	if (QPTOGP(qp)->version < RFS2DOT0) {
		ASSERT(rfvfsp->rfvfs_flags & MCACHE);
		if (rp->rp_cache & DU_CACHE_ENABLE) {
			vcode = rp->rp_rval;
		} else {
			return NULL;
		}
	} else {
		vcode = rp->rp_v2vcode;
	}
	while (sdhash != vfhash &&
	  (sdp = HASHTOSD(sdhash))->sd_fhandle != fhandle) {
		sdhash = sdhash->ls_next;
	}
	if (sdhash != vfhash) {
		/*
		 * Matched on fhandle; cache hit or stale cache entry.
		 */
		sndd_unhash(sdp);
		if (sdp->sd_vcode == vcode && sdp->sd_size == size) {

			/* cache hit; define sd */

			sdp->sd_stat |= SDUSED;
			sndd_set(sdp, qp, giftp);
			rfvfsp->rfvfs_refcnt++;	/* remember SD */
			VN_HOLD(SDTOV(sdp));
		} else {
			/* stale cache entry */
			/* CONSTCOND */
			rfc_disable(sdp, (ulong)0);
			sndd_free(&sdp);
			sdp = NULL;
		}
	} else {
		sdp = NULL;
	}
	return sdp;
}

/*
 * Check the denoted send descriptor which MUST be for a regular file,
 * for consistency with the caching information in the denoted response
 * message, which MUST be SVR4 or later.
 * Re-enable or disable cache as appropriate; may sleep.
 *
 * Return zero for success, nonzero errno for error cases.
 */
int
rfc_v2vcodeck(sdp, bp)
	register sndd_t		*sdp;
	register mblk_t		*bp;
{
	register rf_response_t	*rp = RF_RESP(bp);
	register gdp_t		*gdp = QPTOGP(sdp->sd_queue);
	int			error = 0;
	size_t			size;
	int			isgift;

	ASSERT(SDTOV(sdp)->v_type == VREG);
	ASSERT(gdp->version >= RFS2DOT0);

	/*
	 * If there is a gift in the message, co_size applies to the gift,
	 * not to the current sdp, and will be handled elsewhere.
	 */

	isgift = RF_MSG(bp)->m_stat & RF_GIFT;
	if (!isgift) {
		size = RF_COM(bp)->co_size;
	}

	if (rp->rp_v2vcode) {
		register ulong	vcode = rp->rp_v2vcode;
		register long	fhandle = rp->rp_fhandle;

		if (sdp->sd_stat & SDCACHE) {
			if (sdp->sd_fhandle != fhandle) {

				rfc_pageabort(sdp, (off_t)0, (off_t)0);

				/* CONSTCOND */
				rfc_disable(sdp, (ulong)0);
				gdp_discon("rfc_v2vcodeck bad cache handshake",
				   gdp);
				error = EPROTO;
			} else {

				/*
				 * Keep cache attributes up to date.  Changes
				 * can happen without a disable message when
				 * this client updates the file.
				 */

				if (!isgift && size < sdp->sd_size) {

					/*
					 * We truncated the file.  Toss pages
					 * not strictly contained in the new
					 * size.
					 */

					rfc_pageabort(sdp, (off_t)size,
					  (off_t)0);
				}
				rfc_enable(sdp, vcode, fhandle);
			}
		} else if (sdp->sd_vcode <= vcode) {

			/*
			 * Enable or renable the cache.  Reenabling is defined
			 * to be throwing out pages of which client caching
			 * has no knowledge, followed by enabling the cache.
			 */

			if (SDTOV(sdp)->v_pages) {
				rfc_pageabort(sdp, (off_t)0, (off_t)0);
			}
			rfc_enable(sdp, vcode, fhandle);
		}

		/* The missing branch here is
		 *
		 *	!(sdp->sd_stat & SDCACHE) && sdp->sd_vcode > vcode
		 *
		 * The following sequence would have occcurred:
		 *
		 *	1.  Server decided enabling cache was okay.
		 *	2.  Server slept before sending that message,
		 *	    which is the one we have here.
		 *	3.  Server sent us disable message with newer
		 *	    vcode.
		 *	4.  Server sent us this message with older
		 *	    vcode.
		 *
		 * Therefore we do nothing, leaving the cache disabled.
		 */

	} else if (sdp->sd_stat & SDCACHE) {

		/*
		 * We consider this an "implicit disable message,"
		 * e.g., as a result of our sending a VOP_FRLOCK.
		 */

		/* CONSTCOND */
		rfc_disable(sdp, (ulong)0);

	}
	if (!isgift) {
		sdp->sd_size = size;
	}
	return error;
}

/*
 * For all pages on the denoted vnode such that
 *
 *	ptob(btop(startoff)) <= p_offset < ptob(btopr(endoff))
 *
 * If the page is not held, destroy its vnode association and abort it.
 * Otherwise, mark the page gone; it will be aborted when the
 * last references to them go away.  endoff == 0 implies EOF.
 *
 */
void
rfc_pageabort(sdp, startoff, endoff)
	sndd_t			*sdp;
	register off_t		startoff;
	register off_t		endoff;
{
	register vnode_t	*vp = SDTOV(sdp);
	register page_t		*pp;
	register page_t		*nextpp, *spp;
	register int		s;

        if ((spp = (page_t *)kmem_zalloc(sizeof(page_t), KM_SLEEP)) == NULL)
		cmn_err(CE_PANIC, "rfc_pageabort: cannot allocate marker pages");

	s = splvm();
	pp = vp->v_pages;
	if (pp == NULL) {
		splx(s);
		kmem_free(spp, sizeof(page_t));
		return;
	}

	spp->p_vnode = (struct vnode *)&rfc_pageabort;
#ifdef DEBUG
	spp->p_offset = (u_int)u.u_procp;
#endif

	/*
	 * Insert a start marker at the front of the v_pages list,
	 * so we'll know when we've traversed the entire list.
	 */
	spp->p_vpnext = pp;
	spp->p_vpprev = pp->p_vpprev;
	pp->p_vpprev = spp;
	spp->p_vpprev->p_vpnext = spp;
	vp->v_pages = spp;

	startoff = ptob(btop(startoff));
	if (endoff) {
		endoff = ptob(btopr(endoff));
	} else if (startoff == 0) {
		/*
		 * Optimize the usual case.
		 */
		do {
			nextpp = pp->p_vpnext;
			if (spp == nextpp) {
				nextpp = NULL;
			}
			if (pp->p_vnode != vp)
				continue;
			if (pp->p_free) {
				page_reclaim(pp);
			}
			page_abort(pp);
		} while ((pp = nextpp) != NULL);
		goto out;
	}

	do {
		nextpp = pp->p_vpnext;
		if (spp == nextpp) {
			nextpp = NULL;
		}
		if (pp->p_vnode != vp)
			continue;
		if (pp->p_offset >= startoff &&
		    (pp->p_offset < endoff || !endoff)) {
			if (pp->p_free) {
				page_reclaim(pp);
			}
			page_abort(pp);
		}
	} while ((pp = nextpp) != NULL);
out:
	if (spp->p_vpnext == spp)
		vp->v_pages = NULL;
	else {
		spp->p_vpnext->p_vpprev = spp->p_vpprev;
		spp->p_vpprev->p_vpnext = spp->p_vpnext;
		if (vp->v_pages == spp)
			vp->v_pages = spp->p_vpnext;
	}
	kmem_free(spp, sizeof(page_t));

	splx(s);
	return;
}

/*
 * Release all pages on the denoted pagelist.  NULL the pagelist.
 */
void
rfc_plrele(plp)
	register page_t	**plp;
{
	register page_t	*pp;

	while ((pp = *plp) != NULL) {
		page_sub(plp, pp);
		rfc_pageunlock(pp);
		ASSERT(pp->p_keepcnt > 0);
		PAGE_RELE(pp);
	}
	*plp = NULL;
}

STATIC void
rfc_pagelock(pp)
	register page_t	*pp;
{
	page_lock(pp);
	pp->p_intrans = 1;
	pp->p_pagein = 1;
	pp->p_nio = 1;
}

/*
 * This routine knows it's called after a page is filled, so it clears the mod
 * bit.
 */
void
rfc_pageunlock(pp)
	register page_t	*pp;
{
	pp->p_intrans = 0;
	pp->p_pagein = 0;
	pp->p_nio = 0;
	pp->p_mod = 0;
	page_unlock(pp);
}

/*
 * Abort pages and free send descriptors associated with denoted rf_vfs.
 */
void
rfc_mountinval(rfvfsp)
	rf_vfs_t		*rfvfsp;
{
	register sndd_t		*sdp;
	register sndd_t 	*endsndd = sndd + nsndd;
	register vfs_t		*vfsp = RFTOVF(rfvfsp);

	rfc_sdabort(rfvfsp);
	for (sdp = sndd; sdp < endsndd; sdp ++) {
		if (sdp->sd_stat & SDUSED &&
		    SDTOV(sdp)->v_vfsp == vfsp) {
			rfc_disable(sdp, (ulong)0);
			rfc_pageabort(sdp, (off_t)0, (off_t)0);
		}
	}
}

/*
 * Move data from the (headerless!) RFS message bp into pages for
 * file SDTOV(sdp), rooted in *ppp, starting at file offset *offp.
 * Updates *offp. Partially filled pages are held, locked and in transit.
 * Filled pages are released, unlocked and resident.
 *
 * At first call here, *ppp must be NULL.  It is then used to hold pages
 * across calls, in case filling a page takes more than one call.
 *
 * If a page is not in *ppp and
 *	1.  the page is locked or in transit
 *	  or
 *	2.  the current offset is not on a page boundary,
 *	  or
 *	3.  the page is resident.
 * then we will not fill the page.  The first condition is to avoid sleeps
 * while streams messages are arriving.  The second is to avoid introducing
 * a data inconsistency.  The third is to avoid unnecessary copying.
 * Nonethess, in the last case we will try to reclaim the page, anticipating
 * a future cache hit.
 *
 * Releases pages from *ppp as they are filled.
 */
STATIC int
rfc_readfill(sdp, bp, ppp, offp)
	sndd_t		*sdp;
	mblk_t		*bp;
	page_t		**ppp;
	off_t		*offp;
{
	off_t		off = *offp;
	page_t		*pp = *ppp;
	int		error = 0;

	ASSERT(!pp ||
	  pp->p_intrans && pp->p_lock &&
	  pp->p_prev == pp && off > pp->p_offset &&
	  off < pp->p_offset + PAGESIZE);

	while (bp) {
		caddr_t data = (caddr_t)bp->b_rptr;
		size_t	datasz = (caddr_t)bp->b_wptr - data;

		while (datasz) {
			off_t	on = off & PAGEOFFSET;
			size_t	cpsize = MIN(datasz, PAGESIZE - on);

			if (!on) {
				ASSERT(!pp);
				if (error = rfc_pageget(sdp, off, &pp)) {
					goto out;
				}
			}

			/*
			 * pp could be NULL if rfc_pageget didn't get a page.
			 */

			if (pp) {
				bcopy(data, rfc_pptokv(pp) + on, cpsize);
			}

			/*
			 * Skip over data moved, or discard data if pp == NULL;
			 * resident.
			 */

			off += cpsize;
			data += cpsize;
			datasz -= cpsize;
			if (!(off & PAGEOFFSET) && pp) {

				/* Page is full. */

				rfc_pageunlock(pp);
				ASSERT(pp->p_keepcnt > 0);
				PAGE_RELE(pp);
				pp = NULL;
			}
		}
		bp = bp->b_cont;
	}
out:
	*offp = off;
	*ppp = pp;
	return error;
}

/*
 * Returns an empty, held, locked page for the specified file and offset.
 * Otherwise, if either of the following is true, returns NULL:
 *
 *	1.  The specified page is resident.  (If the page is also free,
 *	    we try to reclaim it, anticipating a future cache hit.)
 *	2.  A new page is not available.
 *
 * Does not sleep.
 *
 * If a page is returned, it will have been hashed and linked to
 * SDTOV(sdp), and rfc_pagelocked.
 */
STATIC int
rfc_pageget(sdp, offset, ppp)
	sndd_t			*sdp;
	off_t			offset;
	page_t			**ppp;
{
	register vnode_t	*vp = SDTOV(sdp);
	register page_t		*pp;
	int			error = 0;

	/*
	 * This routine is much like page_lookup, but without the sleeps.
	 * We do our best to avoid sleeping but, in fact, page_reclaim
	 * could, in some future mp implementation, sleep.
	 */

	if ((pp = page_find(vp, offset)) != NULL) {
		pp = NULL;
	} else if ((pp = page_get(PAGESIZE, 0)) != NULL) {

		/*
		 * Second arg to page_get above is "canwait". page_get 
		 * holds the page, so we don't have to do another.
		 */

		page_hashin(pp, vp, offset, 0);	/* 0 lock */
		rfc_pagelock(pp);
	}
	*ppp = pp;
	return error;
}
