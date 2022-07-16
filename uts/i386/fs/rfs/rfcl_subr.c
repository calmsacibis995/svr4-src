/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rfcl_subr.c	1.3.2.5"

#include "sys/list.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/debug.h"
#include "sys/rf_messg.h"
#include "sys/sysmacros.h"
#include "sys/nserve.h"
#include "sys/stream.h"
#include "sys/rf_cirmgr.h"
#include "sys/errno.h"
#include "sys/fs/rf_vfs.h"
#include "sys/fs/rf_acct.h"
#include "sys/rf_sys.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_comm.h"
#include "sys/file.h"
#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/hetero.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "rfcl_subr.h"
#include "sys/uio.h"
#include "sys/kmem.h"
#include "sys/mount.h"
#include "sys/systm.h"
#include "sys/mode.h"
#include "sys/pathname.h"
#include "vm/page.h"
#include "vm/seg_map.h"
#include "vm/pvn.h"
#include "rf_cache.h"

/* imports */
extern int	rf_state;
extern void	bp_mapin();

union rq_arg	init_rq_arg;

STATIC int	rfcl_vn_init();
STATIC int	rfcl_ckgiftrp();
STATIC int	rfcl_read_pass();
STATIC int	rfcl_write_pass();
STATIC int	rfcl_esbwrmsg();
STATIC void	rfcl_dorele();

/*
 * Common routine for those ops that have no request data portion.
 * Uses RFCL_MAXTRIES, BPRI_LO, and R_ULIMIT, passes canfail to rfcl_msg
 * and rcvd_create.  Handles message setup and exchange(s).
 * (*bpp == NULL) iff error
 */
int
rfcl_op(chansdp, crp, opcode, rqargp, bpp, canfail)
	register sndd_t	*chansdp;
	cred_t		*crp;
	int		opcode;
	union rq_arg	*rqargp;
	mblk_t		**bpp;
	int		canfail;
{
	register int	error;
	int		nacked;
	register int	ntries;
	rcvd_t		*rdp;
	register int	vcver = QPTOGP(chansdp->sd_queue)->version;
	size_t		minreq = RF_MIN_REQ(vcver);

	/* create an rd on which to receive the response */
	if ((error = rcvd_create(canfail, RDSPECIFIC, &rdp)) != 0) {
		return error;
	}
	rdp->rd_sdp = chansdp;
	for (ntries = nacked = 1; ntries < RFCL_MAXTRIES && nacked; ntries++) {
		if ((error = rf_allocmsg(minreq, (size_t)0, BPRI_LO, canfail,
		  NULLCADDR, NULLFRP, bpp)) != 0) {
			break;
		}
		rfcl_reqsetup(*bpp, chansdp, crp, opcode, R_ULIMIT);
		RF_REQ(*bpp)->rq_arg = *rqargp;
		error = rfcl_xac(bpp, minreq, rdp, vcver, FALSE, &nacked);
	}
	rcvd_free(&rdp);
	return error;
}

/*
 * Sends the message contained in *bpp (of size size) on rdp->rd_sdp.
 * Receives a message on rdp into *bpp (note that this is reused).
 * Handles signal and mandatory lock information when appropriate.
 * Checks cache consistency when appropriate.  retrans should be true
 * iff caller is retransmitting a duped message.
 *
 * Returns 0 normally, otherwise an error.  *nackp is set to 1 if the
 * conditions for retrying are met.  (*bpp == NULL) iff error
 */

int
rfcl_xac(bpp, size, rdp, vcver, retrans, nackp)
	mblk_t			**bpp;
	size_t			size;
	rcvd_t			*rdp;
	int			vcver;
	register int		*nackp;	/* out param - nack indication */
{
	register mblk_t		*bp;
	register rf_response_t	*rp;
	register rf_common_t	*cop;
	register int		error;
	register sndd_t		*chansdp = rdp->rd_sdp;

	*nackp = 0;			/* only reset when appropriate */

	if (RF_SERVER()) {
		rf_freemsg(*bpp);
		*bpp = NULL;
		return EMULTIHOP;
	}
	error = rf_sndmsg(chansdp, *bpp, size, rdp, retrans);
	*bpp = NULL;
	if (error) {
		return error;
	}
	if ((error = rf_rcvmsg(rdp, bpp)) != 0) {
		return error;
	}
	bp = *bpp;
	rp = RF_RESP(bp);
	cop = RF_COM(bp);
	if ((*nackp =
	  rp->rp_errno == ENOMEM && cop->co_type == RF_NACK_MSG) == 0 &&
	  cop->co_opcode != RFCOPYOUT && cop->co_opcode != RFCOPYIN) {
		if (!rf_sigisempty(rp, vcver)) {
			rf_postrpsigs(rp, vcver, u.u_procp);
		}
		if (vcver >= RFS2DOT0 && SDTOV(chansdp)->v_type == VREG &&
		  !rp->rp_errno && (error = rfc_v2vcodeck(chansdp, bp)) != 0) {
			rf_freemsg(bp);
			*bpp = NULL;
		}
	}
	if (!error && *nackp) {
		if (rp->rp_cache & RP_MNDLCK) {

			/*
			 * somebody turned on locking behind our back;
			 * disable caching for the file.
			 */

			if (vcver == RFS1DOT0) {
				chansdp->sd_stat |= SDMNDLCK;
				/* CONSTCOND */
				rfc_disable(chansdp, 0);
			} else {
				gdp_j_accuse("rfcl_xac mandlock NACK",
				  QPTOGP(chansdp->sd_queue));
				error = EPROTO;
				*nackp = FALSE;
			}
		}
		rf_freemsg(bp);
		*bpp = NULL;
		if (!error) {
			error = ENOMEM;
		}
	}
	return error;
}

/*
 * Do common work of setting up request message headers.
 */
void
rfcl_reqsetup(bp, chansdp, crp, opcode, ulim)
	mblk_t		*bp;
	sndd_t		*chansdp;
	cred_t		*crp;
	int		opcode;
	ulong		ulim;
{
	register gdp_t		*gdp = QPTOGP(chansdp->sd_queue);
	register rf_request_t	*reqp = RF_REQ(bp);
	register rf_common_t	*cop = RF_COM(bp);

	cop->co_sysid = gdp->sysid;
	cop->co_opcode = opcode;
	cop->co_type = RF_REQ_MSG;
	cop->co_uid = crp->cr_uid;
	cop->co_gid = crp->cr_gid;
	cop->co_mntid = chansdp->sd_mntid;

	reqp->rq_ulimit = ulim >> ULIMSHIFT;

	/*
	 * Copy as many as the agreed maximum number of groups into
	 * the request, if the server supports multi-groups.
	 */

	if (gdp->version > RFS1DOT0) {
		register ushort gn;

		reqp->rq_ngroups = MIN((int)crp->cr_ngroups, gdp->ngroups_max);
		for (gn = 0; gn < (ushort)reqp->rq_ngroups; gn++) {
			reqp->rq_groups[gn] = crp->cr_groups[gn];
		}
	} else {

		/*
		 * Since a single sndd represents a given file, another process
		 * may have set SDMNDLCK in ours, thus each request contains an
		 * indication of MNDLCK status.
		 */

		if (chansdp->sd_stat & SDMNDLCK) {
			reqp->rq_flags |= RQ_MNDLCK;
		} else {
			reqp->rq_flags &= ~RQ_MNDLCK;
		}
	}
}

/*
 * *giftsdpp is assumed to point to a send descriptor whose sd_gift and
 * sd_queue are well-defined, whose associated vnode has its type and vfsp
 * defined, and that was allocated only to hold a new reference to a
 * remote file.	 *giftsdp must be unlocked.  bp must contain a well-defined
 * response, and vfsp be the vfs in which the file reference resides.
 *
 * Either completes the definition of *giftsdpp, if it is the
 * first sndd to refer to a particular file, or frees *giftsdp and
 * replaces it with an existing sdp referring to the file.
 *
 * In error cases, NULLs *giftsdpp and returns error.
 *
 * Global side-effects: updates sdfreelist.
 */
int
rfcl_findsndd(giftsdpp, crp, bp, vfsp)
	sndd_t			**giftsdpp;
	cred_t			*crp;
	register mblk_t		*bp;
	register vfs_t		*vfsp;
{
	/* TO DO:  get rid of linear search, put in hashing */

	register sndd_t		*giftsdp = *giftsdpp;
	register rf_response_t	*rp = RF_RESP(bp);
	register sndd_t		*sdp;			/* candidate */
	register sndd_t		*endsndd = sndd + nsndd;
	register vnode_t	*vp;
	rf_common_t		*cop = RF_COM(bp);
	int			vcver = QPTOGP(giftsdp->sd_queue)->version;
	int			error = 0;

	ASSERT(!(giftsdp->sd_stat & SDLOCKED));
	ASSERT(vfsp);

	/*
	 * SDINTER is used to prevent a match on this or other sndds that
	 * may be redundant.
	 */

	giftsdp->sd_stat |= SDINTER;
	SDTOV(giftsdp)->v_type = IFTOVT(cop->co_ftype);
	giftsdp->sd_mntid = cop->co_mntid;
	giftsdp->sd_size = cop->co_size;

	/*
         * make gift vnode well-defined; does VN_HOLD, increments rfvfs_refcnt
	 */

	if ((error = rfcl_vn_init(giftsdp, vfsp)) != 0) {
		sndd_free(giftsdpp);
		return error;
	}

	vp = SDTOV(giftsdp);
	sdp = sndd;
	while (sdp != endsndd) {
		if (sdp->sd_gift.gift_id == giftsdp->sd_gift.gift_id &&
		  SDTOV(sdp)->v_vfsp == vfsp &&
		  sdp->sd_stat & SDUSED && !(sdp->sd_stat & SDINTER)) {

			/* found the sd */

			if (sdp->sd_stat & SDLOCKED) {

				/* busy - try again */

				sdp->sd_stat |= SDWANT;
				(void)sleep((caddr_t)sdp, PSNDD);
				sdp = sndd;
				continue;

			} else if (sdp->sd_gift.gift_gen !=
			  giftsdp->sd_gift.gift_gen) {

				gdp_j_accuse("rfcl_findsndd bad file reference",
				  QPTOGP(giftsdp->sd_queue));
				sndd_free(giftsdpp);
				--VFTORF(vfsp)->rfvfs_refcnt;
				return EPROTO;

			} else {
				break;
			}
		}

		/* keep looking */

		sdp++;
	}
	if (sdp != endsndd) {

		/* Have an unlocked, busy, therefore valid sd in hand. */

		vp = SDTOV(sdp);
		ASSERT(vp->v_vfsp == vfsp);
		ASSERT(sdp->sd_queue == giftsdp->sd_queue);

		if (vp->v_type != SDTOV(giftsdp)->v_type) {
                        gdp_j_accuse("rfcl_findsndd unexpected v_type",
			  QPTOGP(sdp->sd_queue));
			sndd_free(giftsdpp);
                        --VFTORF(vfsp)->rfvfs_refcnt;
                        return EPROTO;
		}

		sdp->sd_size = cop->co_size;
		VN_HOLD(vp);

		/*
                 * Remember how many times the server gave us this gift.  We'll
                 * tell it at rf_inactive time how many of its vnode references
                 * we had.
                 */

                if (QPTOGP(sdp->sd_queue)->version == RFS1DOT0) {
                        SDTOV(giftsdp)->v_count = 0;
                        rfcl_sdrele(giftsdpp, crp, 1);
		} else {
			--VFTORF(vfsp)->rfvfs_refcnt;
                        sdp->sd_remcnt++;
                        sndd_free(giftsdpp);
		}
                *giftsdpp = sdp;

	} else if ((vcver >= RFS2DOT0 || VFTORF(vfsp)->rfvfs_flags & MCACHE) &&
	  (sdp = rfc_sdsearch(VFTORF(vfsp), rp, giftsdp->sd_queue, 
	   &giftsdp->sd_gift, cop->co_size)) != NULL) {

		/*
		 * sndd cache hit.
		 *
		 * For files residing on new servers, we search for cached
		 * sndds even if client caching is not enabled for the mount,
		 * because we cache pages for mapped files.
		 *
		 * rfc_sdsearch did a VN_HOLD, set up the cached sndd,
		 * and bumped rfvfs_refcnt.  Here we even things out by
		 * tossing the giftsdp and its vnode's reference to the rfvfs.
		 */

		sndd_free(giftsdpp);
		--VFTORF(vfsp)->rfvfs_refcnt;
		sdp->sd_size = cop->co_size;
		sdp->sd_remcnt = 1;
		*giftsdpp = sdp;
		vp = SDTOV(sdp);

	} else {

		/*
		 * We didn't find a current reference to the vnode.  This
		 * is not a regular file, or it is a regular file
		 * for which we had no cached reference.  Use the current
		 * gift, which already has a well-defined vnode.  Resetting
		 * SDINTER allows others to match this sndd.
		 */

		giftsdp->sd_stat &= ~SDINTER;
		sdp = giftsdp;
		sdp->sd_remcnt = 1;
		vp = SDTOV(sdp);
	}

	if (vcver < RFS2DOT0 || rp->rp_v2giftinfo.flags & RPG_NOMAP) {

		/*
		 * Pre-SVR4 servers can't handle page faults.
		 * We can't just turn a page fault into a read request,
		 * because the server will try to lock its inode,
		 * which can cause a deadly embrace.
		 */

		vp->v_flag |= VNOMAP;
	}

	/*
	 * If remote swap space were used, disconnects, for example, would
	 * spell disaster for the client.  A future, more robust kernel may
	 * be able to handle this.  Mounts on remote pathnames are also
	 * problematic with respect to recovery.  A general kernel mechanism 
	 * should be added to locate mounts whose original directory paths have
	 * changed or disappeared.  With that facility, remote mounts would
	 * be safe.
	 */
	vp->v_flag |= VNOSWAP | VNOMOUNT;

	if (VFTORF(vfsp)->rfvfs_flags & MCACHE &&
	  vp->v_type == VREG && 
	  (error = rfcl_ckgiftrp(sdp, bp)) != 0) {
		VN_RELE(vp);
		*giftsdpp = NULL;
	}
	return error;
}

/*
 * Free the gift send descriptor.  If there is a message block
 * containing a remote file reference, give up the reference.
 * Set *giftsdpp to NULL.
 */
void
rfcl_giftfree(bp, giftsdpp, crp)
	register mblk_t		*bp;
	sndd_t			**giftsdpp;
	cred_t			*crp;
{
	register sndd_t		*sdp = *giftsdpp;
	register rf_message_t	*msgp;
	register rf_common_t	*cop;

	ASSERT(LS_ISEMPTY(&sdp->sd_hash));
	if (bp && ((msgp = RF_MSG(bp))->m_stat & RF_GIFT)) {
		/*
		 * set up to give up reference
		 */
		cop = RF_COM(bp);
		sndd_set(sdp, msgp->m_queue, &msgp->m_gift);
		sdp->sd_mntid = cop->co_mntid;
		sdp->sd_size = cop->co_size;
		SDTOV(sdp)->v_count = 0;
		rfcl_sdrele(giftsdpp, crp, 1);
	} else {
		sndd_free(giftsdpp);
	}
}

/*
 * Copy data from response message sent by server into client space
 *
 * Returns 0 for success, nonzero errno for fatal error.
 * If *uio_errorp is not set, moves data to read address, and then
 * may set *uio_errorp if an error occurs.
 *
 * Takes care of copysync messages.  (reply_sdp may be NULL if no
 * copysync is needed.)
 *
 * Always frees incoming streams buffer, and nulls its reference.
 */
int
rfcl_readmove(inbpp, uiop, reply_sdp, uio_errorp)
	register mblk_t	**inbpp;
	register uio_t	*uiop;
	register sndd_t	*reply_sdp;
	register int	*uio_errorp;
{
	mblk_t		*bp = *inbpp;
	rf_response_t	*resp = RF_RESP(bp);
	int		error = 0;
	long		opcode = RF_COM(bp)->co_opcode;
	long		copysync = resp->rp_copyout.copysync;
	rf_message_t	*mp = RF_MSG(bp);
	queue_t		*qp = (queue_t *)mp->m_queue;
	rf_gift_t	gift;
	uio_t		ruio;
	iovec_t		*iovp;
	int		niov;

	gift = mp->m_gift;
	if (!*uio_errorp &&
	  (bp = rf_dropbytes(bp, RF_MIN_RESP(QPTOGP(qp)->version))) != NULL) {
		ruio.uio_offset = 0;
		ruio.uio_segflg = UIO_SYSSPACE;
		ruio.uio_fmode = 0;
		ruio.uio_limit = 0;

		rf_iov_alloc(&ruio, bp);
		iovp = ruio.uio_iov;
		niov = ruio.uio_iovcnt;

		*uio_errorp = uiomvuio(&ruio, uiop);
		RF_IOV_FREE(iovp, niov);
	}

	if (!bp && opcode == RFCOPYOUT) {
		gdp_discon("rfcl_readmove empty RFCOPYOUT message",
		  QPTOGP(qp));
		return EPROTO;
	}

	rf_freemsg(bp);
	*inbpp = bp = NULL;

	if (opcode == RFCOPYOUT && copysync) {
		/* Send a response so the server will not hang */
		error = rfcl_copysync(reply_sdp, qp, &gift);
	}
	return error;
}

/*
 * Called from client to send a signal to a server process.
 * Send an interrupt message on the send descriptor provided by our caller.
 * Does not wait for a reply because the reply is a natural; albeit
 * interrupted, return from the remote system call.
 */
void
rfcl_signal(sdp)
	register sndd_t		*sdp;
{
	register rf_common_t	*cop;
	mblk_t			*bp;
	register size_t		hdrsz =
				    RF_MIN_REQ(QPTOGP(sdp->sd_queue)->version);

	(void)rf_allocmsg(hdrsz, (size_t)0, BPRI_MED, FALSE, NULLCADDR, NULLFRP,
	  &bp);
	ASSERT(bp);
	cop = RF_COM(bp);
	cop->co_sysid = SDTOSYSID(sdp);
	cop->co_opcode = RFRSIGNAL;
	cop->co_type = RF_REQ_MSG;
	cop->co_mntid = sdp->sd_mntid;
	RF_MSG(bp)->m_stat |= RF_SIGNAL;
	(void)rf_sndmsg(sdp, bp, hdrsz, (rcvd_t *)NULL, FALSE);
}

/*
 * Return nonzero errno for fatal communications error, 0 otherwise.
 */
int
rfcl_copysync(sdp, qp, giftp)
	register sndd_t		*sdp;
	queue_t			*qp;
	rf_gift_t		*giftp;
{
	register size_t		respsize = RF_MIN_RESP(QPTOGP(qp)->version);
	mblk_t			*bp;
	register rf_common_t	*cop;

	(void)rf_allocmsg(respsize, (size_t)0, BPRI_MED, FALSE, NULLCADDR,
	  NULLFRP, &bp);
	ASSERT(bp);
	cop = RF_COM(bp);
	sndd_set(sdp, qp, giftp);
	cop->co_type = RF_RESP_MSG;
	cop->co_opcode = RFCOPYOUT;
	return rf_sndmsg(sdp, bp, respsize, (rcvd_t *)NULL, FALSE);
}

/*
 * Give up last reference to remote file.
 * Call only when denoted vnode has v_count == 0.  SDINTER may or may not
 * be set in sdp, SDUSED must be set. *sdpp set to NULL.  This routine is
 * called even when an RF_INACTIVE has been piggybacked on a close request.
 * Don't repeat the request now; only send it when goremote is non-zero.
 */
void
rfcl_sdrele(sdpp, crp, goremote)
	sndd_t			**sdpp;
	cred_t			*crp;
	int			goremote;
{
	register sndd_t		*sdp = *sdpp;
	register vnode_t	*vp = SDTOV(sdp);

	ASSERT(sdp->sd_stat & SDUSED);
	ASSERT(!vp->v_count);
	ASSERT(!(sdp->sd_stat & SDLOCKED));
	ASSERT(!(vp->v_flag & VROOT));

	/*
	 * We lock to prevent races with other processes doing lookups
	 * on the same vnode while we sleep here.  If they find this
	 * sndd SDUSED, they will give up their new references, using
	 * this one instead.  But because we're giving up all current
	 * references, that leaves them dangling.  We can't just mark the
	 * sndd SDUNUSED, because that interferes with page caching.
	 */

	if (vp->v_vfsp) {

		/*
		 * Ugh.  Under some circumstances, we can get into
		 * this function with a partly-defined sndd.
		 */

		--VFTORF(vp->v_vfsp)->rfvfs_refcnt;
	}

	if (goremote) {

                ASSERT(!(sdp->sd_stat & SDLOCKED));

		SDLOCK(sdp);

		if (!vp->v_pages) {

			/*
			 * Since there are no pages, set SDINTER to prevent
			 * another process in rf_findsndd from waiting on
			 * our lock.
			 */

			sdp->sd_stat |= SDINTER;
		}
		rfcl_dorele(sdp, crp);
	} else if (vp->v_pages) {

		/*
		 * Allow cacheing across last reference.  We don't
		 * require SDCACHE to be on, because it can't hurt us;
		 * when caching is reenabled, the pages will get
		 * aborted.  In the meantime, somebody playing fast
		 * and loose (because otherwise SDCACHE would likely
		 * be on) with mmap is free to hit the cache.
		 */

		sdp->sd_stat &= ~SDINTER;
		sndd_hash(sdp);
		*sdpp = NULL;
	} else {
		sndd_free(sdpp);
	}
}

/*
 * Add a new sndd to the vfs, initalizing and holding associated
 * vnode.
 */
STATIC int
rfcl_vn_init(giftsdp, vfsp)
	register sndd_t	*giftsdp;
	register vfs_t	*vfsp;
{
	register vnode_t *vp = SDTOV(giftsdp);

	if (vp->v_type <= VNON || vp->v_type >= VBAD) {
		gdp_j_accuse("rfcl_vn_init unexpected vtype",
		  QPTOGP(giftsdp->sd_queue));
		return EPROTO;
	}
	/* At this time, there is no meaningful definition
	 * of rdev across network, and it is not needed
	 * by os code, so we use 0.
	 * Macro zeroes flags, too.
	 */
	VN_INIT(vp, vfsp, vp->v_type, 0);	/* sets v_count to 1 */
	vp->v_vfsmountedhere = NULL;
	vp->v_op = &rf_vnodeops;
	vp->v_stream = NULL;
	vp->v_data = (caddr_t)giftsdp;
	vp->v_filocks = NULL;
	VFTORF(vfsp)->rfvfs_refcnt++;	/* remember SD */
	return 0;
}

/*
 * Finish initializing send descriptor.
 * Enable or disable cache for vp based on data in response.
 */
STATIC int
rfcl_ckgiftrp(sdp, bp)
	register sndd_t		*sdp;
	mblk_t			*bp;
{
	register rf_response_t	*rp = RF_RESP(bp);
	register rf_common_t	*cop = RF_COM(bp);
	register vnode_t	*vp = SDTOV(sdp);

	ASSERT(vp->v_type == VREG);
	ASSERT(VFTORF(vp->v_vfsp)->rfvfs_flags & MCACHE);

	if (QPTOGP(sdp->sd_queue)->version < RFS2DOT0) {
		struct a {
			char	*fname;
			int	mode;
			int	crtmode;
		} *uap = (struct a *)u.u_ap;

		if ((sdp->sd_stat & SDCACHE ||
		   rp->rp_cache & DU_CACHE_ENABLE) &&
		  (cop->co_opcode == RFCREATE ||
		   cop->co_opcode == RFCREATE && uap->mode & FTRUNC) &&
		  sdp->sd_vcode != rp->rp_rval) {
			rfc_pageabort(sdp, (off_t)0, (off_t)0);
			rfc_disable(sdp, (ulong)rp->rp_rval);
			sdp->sd_size = 0;
		}
		if (rp->rp_cache & DU_CACHE_ENABLE) {
			sdp->sd_size = (size_t)cop->co_size;
			rfc_enable(sdp, (ulong)rp->rp_rval, rp->rp_fhandle);
		}
		return 0;
	}

	/* More checks for SVR4. */

	if (vp->v_count == 1) {
		register long	fhandle = rp->rp_fhandle;
		register ulong	vcode = rp->rp_v2vcode;

		/*
		 * Since this is a first reference, normal consistency
		 * checks with the server don't apply.
		 */

		sdp->sd_size = (size_t)cop->co_size;
		if (!vcode) {

			/* Server thinks we shouldn't cache. */

			/* CONSTCOND */
			rfc_disable(sdp, (ulong)0);
		} else if (sdp->sd_vcode <= vcode) {
			if (!vp->v_pages ||
			  sdp->sd_stat & SDCACHE &&
			    vcode == sdp->sd_vcode &&
			    fhandle == sdp->sd_fhandle) {

				/*
				 * Server thinks we should cache, and we have
				 * no stale data.
				 */

				rfc_enable(sdp, vcode, fhandle);
			} else {

				/*
				 * Server thinks we should cache, we have
				 * pages on the vnode, and either the SDCACHE
				 * bit is off or the vcode or fhandle doesn't
				 * match.  We reenable caching by dumping the
				 * stale pages and then enabling.
				 */

				rfc_pageabort(sdp, (off_t)0, (off_t)0);
				rfc_enable(sdp, vcode, fhandle);
			}
		} else if (sdp->sd_stat & SDCACHE) {

			/*
			 * sdp->sd_vcode > vcode
			 *
			 * Therefore this message was preceeded by a disable
			 * message that actually originated later on the
			 * server.  It's hard to see how the cache bit would
			 * be on in the sndd, but we cover the case.
			 */

			/* CONSTCOND */
			rfc_disable(sdp, (ulong)0);
		}
		return 0;
	} else {
		return rfc_v2vcodeck(sdp, bp);
	}
}

/*
 * Manage a generic read request.  Send the request, handle retries and all
 * other responses.  Propagates any error back to caller.
 */
int
rfcl_read_op(chansdp, crp, op, rqargp, rf_rwap)
	sndd_t		*chansdp;
	cred_t		*crp;
	int		op;		/* RFREAD, RFGETPAGE, etc. */
	union rq_arg	*rqargp;	/* partially filled */
	rf_rwa_t	*rf_rwap;
{
	uio_t		*uiop = rf_rwap->uiop;
	register int	error;
	int		nacked;
	int		ntries;
	mblk_t		*bp = NULL;
	rcvd_t		*rdp = NULL;
	register int	vcver = QPTOGP(chansdp->sd_queue)->version;

	rf_rwap->replysdp = NULL;
	/*
	 * Get a send descriptor to handle intermediate data movement.
	 */
	if ((error = sndd_create(TRUE, &rf_rwap->replysdp)) != 0) {
		goto out;
	}
	/*
	 * create an rd on which to receive the response
	 */
	if ((error = rcvd_create(TRUE, RDSPECIFIC, &rdp)) != 0) {
		goto out;
	}
	rdp->rd_sdp = chansdp;
	/*
	 * We set base to keep old servers happy.  SVR4 servers ignore it, and
	 * SVR4 clients ignore the changes that SVR3 servers make to it.
	 */
	rqargp->rqxfer.base = (long)uiop->uio_iov->iov_base;
	rqargp->rqxfer.fmode = uiop->uio_fmode;
	for (ntries = nacked = 1; ntries < RFCL_MAXTRIES && nacked; ntries++) {
		if ((error = rf_allocmsg(RF_MIN_REQ(vcver), (size_t)0, BPRI_LO,
		  TRUE, NULLCADDR, NULLFRP, &bp)) != 0) {
			break;
		}
		rfcl_reqsetup(bp, chansdp, crp, op, (ulong)uiop->uio_limit);
		RF_REQ(bp)->rq_arg = *rqargp;
		error = rfcl_xac(&bp, RF_MIN_REQ(vcver), rdp, vcver, FALSE,
		  &nacked);
	}
	if (!error && (error = RF_RESP(bp)->rp_errno) == 0) {
		error = rfcl_read_pass(rdp, op, &bp, rf_rwap);
	}
out:
	sndd_free(&rf_rwap->replysdp);
	rcvd_free(&rdp);
	rf_freemsg(bp);
	return error;
}

/*
 * This routine is reached when the first valid read response is received.
 * It accepts further messages when they are expected, and handles data
 * movement using rfcl_readmove and rfc_readmove, which take care of
 * copysync responses.
 * rfcl_read_pass returns 0 for success or a non-zero errno for failure.
 */
STATIC int
rfcl_read_pass(rdp, rqop, bpp, rf_rwap)
	register rcvd_t		*rdp;
	register int		rqop;
	register mblk_t		**bpp;
	register rf_rwa_t	*rf_rwap;
{
	register sndd_t		*chansdp = rdp->rd_sdp;
	register vnode_t	*vp = SDTOV(chansdp);
	int			vcver = QPTOGP(chansdp->sd_queue)->version;
	register int		error = 0;
	int			move_error = 0;	/* saved error */

	for (;;) {
		register rf_response_t	*resp = RF_RESP(*bpp);
		register long		rpopcode = RF_COM(*bpp)->co_opcode;

		/*
		 * Even if we get an intermediate error, or if
		 * rfc_readmove sets *rf_rwap->rd_ctlp, we have to
		 * keep looping to satisfy the protocol.
		 */
		if (rpopcode == rqop) {
			if (!rf_sigisempty(resp, vcver)) {
				rf_postrpsigs(resp, vcver, u.u_procp);
			}
			if ((error = resp->rp_errno) != 0) {
				break;
			}
			if (vp->v_type == VREG) {
				if (vcver >= RFS2DOT0) {
					/* always sent */
					if ((error = rfc_v2vcodeck(chansdp,
					  *bpp)) != 0) {
						break;
					}
				} else {
					/*
					 * We update sd_size here to stay
					 * current with the server.
					 */
					chansdp->sd_size = resp->rp_rdwr.isize;
					if (resp->rp_cache & DU_CACHE_ENABLE &&
					  chansdp->sd_vcode && 
					  chansdp->sd_fhandle &&
					  !(chansdp->sd_stat & SDCACHE)) {
						/*
						 * Old servers are willing to 
						 * reenable caching but only
						 * give us a vcode and fhandle
						 * at open/close time.
						 */
						if (vp->v_pages) {
							rfc_pageabort(chansdp,
							  (off_t)0, (off_t)0);
						}
						rfc_enable(chansdp,
						  chansdp->sd_vcode,
						  chansdp->sd_fhandle);
					}
				}
			}
		} else if (rpopcode != RFCOPYOUT) {
			gdp_discon("rfcl_read_pass bad opcode",
			  QPTOGP(chansdp->sd_queue));
			error = EPROTO;
			break;
		}
		if (rf_rwap->cached) {
			/*
			 * rfc_readmove must be called even if last response
			 * has no data.
			 */
			if ((error = rfc_readmove(rf_rwap, bpp, chansdp,
			  &move_error)) != 0) {
				break;
			}
		} else if (rpopcode == rqop && resp->rp_nodata) {
			/*
			 * nodata is only defined for the final response; check
			 * it to avoid calling rfcl_readmove to move 0 bytes.
			 */
			break;
		} else if ((error = rfcl_readmove(bpp, rf_rwap->uiop,
		  rf_rwap->replysdp, &move_error)) != 0) {
			break;
		}
		if (rpopcode != RFCOPYOUT ||
		  (error = rf_rcvmsg(rdp, bpp)) != 0) {
			break;
		}
	}
	return error ? error : move_error;
}

/*
 * Manage a generic write request.  Send the request, handle retries and all
 * other responses.  Propogates any error back to caller.
 */
int
rfcl_write_op(chansdp, crp, op, rf_rwap)
	sndd_t		*chansdp;
	cred_t		*crp;
	int		op;
	rf_rwa_t	*rf_rwap;
{
	uio_t		*uiop = rf_rwap->uiop;
	register int	error;
	int		nacked;
	int		ntries;
	mblk_t		*bp;
	mblk_t		*dupbp;
	rcvd_t		*rdp;
	gdp_t		*gp;
	register int	vcver;
	size_t		hdrsz;
	size_t		prewrite;
	off_t		offset;
	int		resid;
	caddr_t		base;
	off_t		opoff;

	bp = NULL;
	dupbp = NULL;
	rdp = NULL;
	gp = QPTOGP(chansdp->sd_queue);
	vcver = gp->version;
	hdrsz = RF_MIN_REQ(vcver);
	prewrite = MIN(uiop->uio_resid, gp->datasz);
	rf_rwap->replysdp = NULL;

	/* Save these before updating. */

	offset = uiop->uio_offset;
	resid = uiop->uio_resid;
	base = uiop->uio_iov->iov_base;

	if (rf_rwap->wr_kern) {

		if (prewrite < uiop->uio_resid) {

			/*
			 * page-align to increase write cache hits and avoid
			 * blocking on page locks in RFCOPYIN responses.
			 */

			prewrite -= uiop->uio_offset + prewrite & PAGEOFFSET;
		}

		opoff = uiop->uio_offset;
	}

	/* Get a send descriptor to handle intermediate data movement. */

	if ((error = sndd_create(TRUE, &rf_rwap->replysdp)) != 0) {
		goto out;
	}

	/* Get an rd on which to receive the response */

	if ((error = rcvd_create(TRUE, RDSPECIFIC, &rdp)) != 0) {
		goto out;
	}

	rdp->rd_sdp = chansdp;

	if (rf_rwap->cached) {
		size_t	precache;

		/*
		 * Fill pages on which we're doing I/O, using the cloned
		 * uio structure allocated just for that purpose.  If the
		 * write fails, we'll clean up later.  We don't worry about
		 * the transient inconsistency, because we hold the
		 * shared lock.  (mmappers beware.)
		 *
		 * We could use either uiop or rf_wap->cwruio here, because
		 * they are identical at this point.
		 *
		 * Make sure to fill through end of data touched by this
		 * message.  Otherwise, we'd have to hold the page locked
		 * until next message, and fbread would sleep forever on
		 * the page.
		 */

		precache = ptob(btopr(rf_rwap->cwruio.uio_offset + prewrite)) -
		  rf_rwap->cwruio.uio_offset;
		precache = MIN(precache, rf_rwap->cwruio.uio_resid);

		error = rfc_writefill(SDTOV(chansdp), &rf_rwap->cwruio,
		  precache);

		if (error) {
			goto out;
		}
	}

	for (ntries = nacked = 1; ntries < RFCL_MAXTRIES && nacked; ntries++) {
		rf_request_t	*reqp;

		if (ntries == 1) {
			if (rf_rwap->wr_kern && prewrite) {
				error = rfcl_esbwrmsg(chansdp, rf_rwap, hdrsz,
				  prewrite, BPRI_LO, TRUE, &bp);
				if (error) {
					break;
				}
			} else {
				error = rf_allocmsg(hdrsz, prewrite, BPRI_LO,
				  TRUE, NULLCADDR, NULLFRP, &bp);
				if (error ||
				  (error = uiomove(rf_msgdata(bp, hdrsz),
				   (long)prewrite, UIO_WRITE, uiop)) != 0) {
					break;
				}
			}

			/* set up op specific fields in req message */
			reqp = RF_REQ(bp);
			reqp->rq_xfer.offset = offset;
			reqp->rq_xfer.count = resid;
			reqp->rq_xfer.base = (long)base;
			reqp->rq_xfer.fmode = uiop->uio_fmode;
			reqp->rq_xfer.prewrite = prewrite;

			if ((dupbp = dupmsg(bp)) == NULL) {
				error = EAGAIN;
				break;
			}

		} else if ((bp = dupmsg(dupbp)) == NULL) {
			error = EAGAIN;
			break;
		}

		/*
		 * Repeat this per retry because MANDLOCK status may change.
		 */
		rfcl_reqsetup(bp, chansdp, crp, op, (ulong)uiop->uio_limit);

		error = rfcl_xac(&bp, hdrsz + prewrite, rdp, vcver,
		  ntries == 1 ? FALSE : TRUE, &nacked);

	}

	rf_freemsg(dupbp);
	dupbp = NULL;

	if (rf_rwap->cached && !(chansdp->sd_stat & SDCACHE)) {

		/*
		 * This can bother us only on the first write response,
		 * because thereafter we hold the server vnode lock.
		 */

		chansdp->sd_crwlock.writer = FALSE;
		if (chansdp->sd_crwlock.want) {
		  	chansdp->sd_crwlock.want = FALSE;
			wakeprocs((caddr_t)&chansdp->sd_crwlock, PRMPT);
		}
		rf_rwap->cached = FALSE;
		rf_rwap->wr_kern = FALSE;
		if (opoff  == rf_rwap->cwruio.uio_offset)
			rfc_pageabort(chansdp, opoff, rf_rwap->cwruio.uio_offset+1);
		else
			rfc_pageabort(chansdp, opoff, rf_rwap->cwruio.uio_offset);
	} else if (rf_rwap->wr_kern &&
	  (error || (error = RF_RESP(bp)->rp_errno))) {

		/*
		 * Here we handle error occurring in the initial transmission.
		 * rfcl_write_pass must abort pages in case of later errors.
		 */

		if (opoff  == uiop->uio_offset)
			rfc_pageabort(chansdp, opoff, uiop->uio_offset+1);
		else
			rfc_pageabort(chansdp, opoff, uiop->uio_offset);
	}

	if (!error && (error = RF_RESP(bp)->rp_errno) == 0) {
		error = rfcl_write_pass(rdp, op, &bp, rf_rwap);
	}
out:
	sndd_free(&rf_rwap->replysdp);
	rcvd_free(&rdp);
	rf_freemsg(bp);
	return error;
}

/*
 * rfcl_write_pass is reached with the first valid, non-error, non-flow control
 * response to a RFWRITE request.  This routine expects 0 or more RFCOPYIN
 * responses followed by a single RFWRITE response. It uses the rfcl_writemove
 * routine with the supplied uio structure to manage data copying.
 *
 * Returns 0 for success or a nonzero errno for failure.
 */
STATIC int
rfcl_write_pass(rdp, rqop, bpp, rf_rwap)
	register rcvd_t		*rdp;
	register int		rqop;
	register mblk_t		**bpp;
	register rf_rwa_t	*rf_rwap;
{
	register uio_t		*uiop = rf_rwap->uiop;
	register sndd_t 	*replysdp = rf_rwap->replysdp;
	register rf_response_t	*resp = RF_RESP(*bpp);
	register rf_common_t	*cop = RF_COM(*bpp);
	register sndd_t		*req_sdp = rdp->rd_sdp;
	register int		error = 0;
	int			uio_error = 0;
	int			vcver = QPTOGP(req_sdp->sd_queue)->version;
	off_t			opoff;

	if (rf_rwap->wr_kern) {

		/*
		 * Even in cached case, this and cwruio.uio_offset will
		 * be past the start of the same page.
		 */

		opoff = uiop->uio_offset;
	}

	/*
	 * As long as we keep receiving sane RFCOPYIN responses, send
	 * responses to the server.
	 */

	while (cop->co_opcode == RFCOPYIN) {
		if (rf_rwap->wr_kern) {
			opoff = uiop->uio_offset;
		}

		if (error = rfcl_writemove(bpp, rf_rwap, replysdp, req_sdp,
		  &uio_error)) {
			goto out;
		}

		if (error = rf_rcvmsg(rdp, bpp)) {
			rfc_pageabort(req_sdp, (off_t)0, (off_t)0);
			goto out;
		}
		resp = RF_RESP(*bpp);
		cop = RF_COM(*bpp);
	}

	if (cop->co_opcode != rqop) {
		rfc_pageabort(req_sdp, (off_t)0, (off_t)0);
		gdp_discon("rfcl_write_pass bad opcode",
		  QPTOGP(req_sdp->sd_queue));
		error = EPROTO;
		goto out;
	}

	if ((error = resp->rp_errno) != 0) {
		if (!rf_sigisempty(resp, vcver)) {
			rf_postrpsigs(resp, vcver, u.u_procp);
		}
		goto out;
	}
	if (SDTOV(req_sdp)->v_type == VREG && vcver >= RFS2DOT0 &&
	  (error = rfc_v2vcodeck(req_sdp, *bpp))) {
		goto out;
	}

	/* resid count in rval */

	uiop->uio_offset -= resp->rp_rval;
	uiop->uio_resid = resp->rp_rval;

	/* partial write case */
	if (uiop->uio_resid)
		rfc_pageabort(req_sdp, (off_t)0, (off_t)0);

	if (vcver == RFS1DOT0) {
		if (rf_rwap->wr_ioflag & IO_APPEND) {

			/*
			 * We use a computed size for append writes, because
			 * rp_rdwr.isize is the STARTING write offset for them.
			 */

			req_sdp->sd_size = uiop->uio_offset;
		} else {
			req_sdp->sd_size = resp->rp_rdwr.isize;
		}
	}
out:
	if (!error) {
		error = uio_error;
	}
	if (error && rf_rwap->wr_kern) {
		if (opoff == uiop->uio_offset)
			rfc_pageabort(req_sdp, opoff, uiop->uio_offset+1);
		else
			rfc_pageabort(req_sdp, opoff, uiop->uio_offset);
	}
	return error;
}

/*
 * In response to copyin message from server, move data from client
 * to server, and possibly into page cache.
 *
 * Returns 0 for success, errno for fatal error.  Updates *uio_errorp
 * for errors that must be preserved while data movement continues
 * to avoid hanging server.
 *
 * Always frees incoming streams buffer and nulls its reference.
 */
int
rfcl_writemove(in_bpp, rf_rwap, reply_sdp, req_sdp, uio_errorp)
	register mblk_t		**in_bpp;	/* holds incoming message*/
	register rf_rwa_t	*rf_rwap;	/* to direct data movement */
	register sndd_t		*reply_sdp;	/* channel to send the data */
	register sndd_t		*req_sdp;	/* request channel */
	register int		*uio_errorp;
{
	register rf_message_t	*msgp = RF_MSG(*in_bpp);
	register size_t		count = RF_RESP(*in_bpp)->rp_count;
	register int		vcver = QPTOGP(req_sdp->sd_queue)->version;
	register size_t		datasz = QPTOGP(req_sdp->sd_queue)->datasz;
	register size_t		nwritten;
	register size_t		hdrsz = RF_MIN_RESP(vcver);
	uio_t			*uiop = rf_rwap->uiop;
	uio_t			*cwruiop = &rf_rwap->cwruio;
	off_t			opoff;
	int			uio_error = *uio_errorp;
	int			error = 0;

	sndd_set(reply_sdp, msgp->m_queue, &msgp->m_gift);
	rf_freemsg(*in_bpp);
	*in_bpp = NULL;

	if (count > uiop->uio_resid) {
		gdp_discon("rfcl_writemove bad RCOPYIN",
		  QPTOGP(req_sdp->sd_queue));
		return  EPROTO;
	}

	if (rf_rwap->wr_kern) {
		opoff = uiop->uio_offset;
	}

	if (rf_rwap->cached && !uio_error &&
	  uiop->uio_offset + count > cwruiop->uio_offset) {
		size_t	cachesz;

		/* Fill through last byte touched by this message. */

		cachesz = ptob(btopr(cwruiop->uio_offset + count)) -
		  cwruiop->uio_offset;
		cachesz = MIN(cachesz, cwruiop->uio_resid);

		uio_error = rfc_writefill(SDTOV(req_sdp), cwruiop, cachesz);
	}


	/*
	 * Data is likely resident if this is a cached write.  Otherwise,
	 * we're told explicitly by segflg.  If either, avoid copying by
	 * using esballoc.
	 *
	 * We don't look for pages under other circumstances because they
	 * may not be up to date with user buffers.
	 */

	for (nwritten = MIN(count, datasz);
	  count;
	  count -= nwritten, nwritten = MIN(count, datasz)) {

		mblk_t			*wbp;
		register rf_response_t	*nresp;
		register rf_common_t	*cop;
		size_t			msgsz;

		if (!uio_error) {
			if (rf_rwap->wr_kern) {
				uio_error = rfcl_esbwrmsg(req_sdp, rf_rwap,
				  hdrsz, nwritten, BPRI_MED, FALSE, &wbp);
				ASSERT(!uio_error != !wbp);
				if (uio_error) {
					(void)rf_allocmsg(hdrsz, (size_t)0,
					  BPRI_MED, FALSE, NULLCADDR, NULLFRP,
					  &wbp);
					ASSERT(wbp);
				}
			} else {
				(void)rf_allocmsg(hdrsz, nwritten, BPRI_MED,
				  FALSE, NULLCADDR, NULLFRP, &wbp);
				ASSERT(wbp);
				uio_error = uiomove(rf_msgdata(wbp, hdrsz),
				  (long)nwritten, UIO_WRITE, uiop);
			}
		} else {
			(void)rf_allocmsg(hdrsz, (size_t)0, BPRI_MED,
			  FALSE, NULLCADDR, NULLFRP, &wbp);
			ASSERT(wbp);
		}

		cop = RF_COM(wbp);
		nresp = RF_RESP(wbp);
		cop->co_type = RF_RESP_MSG;
		cop->co_opcode = RFCOPYIN;
		msgsz = hdrsz;

		if (*uio_errorp || (*uio_errorp = uio_error) != 0) {
			nresp->rp_count = 0;
			nresp->rp_errno = *uio_errorp;
		} else {
			msgsz += nwritten;
			nresp->rp_count = nwritten;
			nresp->rp_errno = 0;
		}

		if ((error = rf_sndmsg(reply_sdp, wbp, msgsz, (rcvd_t *)NULL,
		  FALSE)) != 0 ||
		  *uio_errorp) {
			break;
		}
	}

	if (rf_rwap->wr_kern && (error || uio_error)) {

		/*
		 * We don't fill the cache once an error has occurred (see
		 * above), but an error occurred on this call, so undo the
		 * cache.
		 */

		if (opoff == uiop->uio_offset)
			rfc_pageabort(req_sdp, opoff, uiop->uio_offset+1);
		else
			rfc_pageabort(req_sdp, opoff, uiop->uio_offset);
	}

	return error;
}

/*
 * Service interface for rf_getapage/rf_pushpages.  Sets up a request
 * message, handles data movement and response messages, updates
 * b_resid, b_error, b_flags.  Assumes it is called with vnode locked.
 * Args
 *	bp
 *		initialized buffer header, including associated page list.
 *	crp
 *		pointer to credentials to initialize request message, or
 *		NULL if credentials are unknown.
 *	*residp
 *		updated with residual count
 *
 * Returns zero for success nonzero errno for failure.
 */
int
rfcl_strategy(bp, crp, residp)
	register struct buf	*bp;
	cred_t			*crp;
	int			*residp;
{
	register sndd_t		*chansdp = VTOSD(bp->b_vp);
	size_t			iovsize = 0;
	caddr_t			iovp;
	int			error;
	rf_rwa_t		rf_rwa;
	uio_t			uio;
	iovec_t			iovec;
	union rq_arg		rqarg;

	rqarg = init_rq_arg;
	if (!crp) {
		crp = u.u_cred;	/* feh */
	}

	/* TO DO:  is handling of fmode okay?*/

	iovec.iov_base = (caddr_t)bp->b_un.b_addr;
	iovec.iov_len = bp->b_bcount;
	uio.uio_iov = &iovec;
	uio.uio_iovcnt = 1;
	uio.uio_offset = bp->b_pages->p_offset;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_resid = bp->b_bcount;
	uio.uio_limit = R_ULIMIT;
	uio.uio_fmode = 0;	/* unneeded */
	rf_rwa.uiop = &uio;
	rf_rwa.cached = FALSE;	/* we're dealing with mapped files */
	if (bp->b_flags & B_READ) {

		/*
		 * rfcl_read_op sends a superset of what the server needs, so we
		 * use it.
		 */

		rqarg.rqxfer.offset = uio.uio_offset;
		rqarg.rqxfer.count = uio.uio_resid;

		error = rfcl_read_op(chansdp, crp, RFGETPAGE, &rqarg, &rf_rwa);

		if (error && uio.uio_resid) {
			/*
			 * Didn't get it all because we hit EOF;
			 * zero the balance.
			 */
			bzero(bp->b_un.b_addr + (bp->b_bcount - uio.uio_resid),
			  (size_t)uio.uio_resid);
		}

		bp->b_error = error;

		/*
		 * pvn_done removes translations to pages, so they are
		 * NOT ADDRESSABLE after this call.  Also removes pages
		 * from b_pages, clears p_intrans and p_pagein, does
		 * PAGE_RELE and unlocks pages.
		 */
	
		pvn_done(bp);

		/*
		 * Throw away the buffer header; calling here assumes
		 * synchronous IO.  Otherwise, the bp would have been
		 * tossed by pvn_done.
		 */

		pageio_done(bp);

		rfcl_fsinfo.fsireadch += bp->b_bcount - uio.uio_resid;
	} else {
		unsigned	bcount = bp->b_bcount;

		rf_rwa.wr_ioflag = 0;
		rf_rwa.wr_kern = TRUE;
		rf_rwa.wr_bufp = bp;

		bp->b_flags |= B_ASYNC;

		error = rfcl_uioclone(&uio, &rf_rwa, &iovsize);

		if (!error) {
			iovp = (caddr_t)rf_rwa.cwruio.uio_iov;

			error = rfcl_write_op(chansdp, crp, RFPUTPAGE, &rf_rwa);

			bp = NULL;	/* bp now a dangling reference */

			if (rf_rwa.wr_bufp) {

				ASSERT(error);	/* B_ASYNC */

				rf_rwa.wr_bufp->b_flags |= B_ERROR;
				pvn_done(rf_rwa.wr_bufp);
			}

			if (iovsize > sizeof(iovec_t)) {
				kmem_free(iovp, iovsize);
			}
			rfcl_fsinfo.fsiwritech += bcount - uio.uio_resid;
		} else {
			bp->b_flags |= B_ERROR;
			pvn_done(bp);
		}
	}
	*residp = uio.uio_resid;

	return error;
}

/*
 * Clone *uiop into rf_rwap->cwruio, providing iovec copies, too.  If
 * uiop->uio_iovcnet == 1, we use rf_rwap->wr_iovec and set *iovszp to
 * sizeof(iovec_t).  Otherwise, kmem_zalloc a vector of the right size
 * and set *iovszp to that size.
 *
 * Don't bother with rf_maxkmem here, the allocation is temporary.
 * Caller is responsible for freeing allocated iovecs.
 */
int
rfcl_uioclone(uiop, rf_rwap, iovszp)
	register uio_t		*uiop;
	register rf_rwa_t	*rf_rwap;
	size_t			*iovszp;
{
	register size_t		iovsize;

	rf_rwap->cwruio = uiop[0];
	if (rf_rwap->cwruio.uio_iovcnt == 1) {
		rf_rwap->wr_iovec = uiop->uio_iov[0];
		rf_rwap->cwruio.uio_iov = &rf_rwap->wr_iovec;
		iovsize = sizeof(iovec_t);
	} else {
		iovsize = rf_rwap->cwruio.uio_iovcnt * sizeof(iovec_t);
		if ((rf_rwap->cwruio.uio_iov =
		  (iovec_t *)kmem_zalloc(iovsize, KM_SLEEP)) == NULL) {
			return ENOMEM;
		}
		bcopy((caddr_t)uiop->uio_iov,
			(caddr_t)rf_rwap->cwruio.uio_iov, iovsize);
	}
	*iovszp = iovsize;
	return 0;
}

/*
 * Build an RFS message containing
 *	0 < resid <= rf_rwap->uiop->uio_resid <= gdp datasz
 * bytes of write data, using resident pages as externally supplied
 * STREAMS data buffers where possible.
 *
 * NOTE:  pri and canfail apply to message allocation;  even though
 * !canfail, rfcl_esbwrmsg may fail due to I/O error, and will then
 * NULL *bpp.
 */
STATIC int
rfcl_esbwrmsg(sdp, rf_rwap, hdrsz, resid, pri, canfail, bpp)
	sndd_t		*sdp;
	rf_rwa_t	*rf_rwap;
	size_t		hdrsz;
	size_t		resid;
	uint		pri;
	int		canfail;
	mblk_t		**bpp;
{
	uio_t		*uiop = rf_rwap->uiop;
	vnode_t		*vp = SDTOV(sdp);
	mblk_t		*bp = NULL;
	mblk_t		*lastbp = NULL;
	mblk_t		*nextbp = NULL;
	int		error = 0;
	off_t		poff;		/* page-aligned IO offset */
	off_t		pend;		/* page-aligned IO end */
	off_t		opoff;
	page_t		*pl;		/* IO list */
	buf_t		*bufp;		/* for IO list */
	int		resident;	/* current state */
	int		nextresident;	/* next state */
	size_t		pchunk;		/* page-aligned message block size */
	size_t		iochunk;	/* actual message block size */

	ASSERT(resid <= uiop->uio_resid);
	ASSERT(resid <= QPTOGP(sdp->sd_queue)->datasz);

	*bpp = NULL;

	if (rf_rwap->wr_bufp) {

		/* rf_putpage has the pages ready for us. */

		ASSERT(rf_rwap->wr_bufp->b_bcount <=
		  QPTOGP(sdp->sd_queue)->datasz);
		ASSERT(rf_rwap->wr_bufp->b_flags &
		  (B_REMAPPED | B_ASYNC | B_WRITE));
		ASSERT(!(uiop->uio_offset & PAGEOFFSET));
		ASSERT(!(rf_rwap->wr_bufp->b_bcount & PAGEOFFSET));

		error = rfesb_pageio_setup(rf_rwap->wr_bufp, hdrsz,
		  (off_t)0, rf_rwap->wr_bufp->b_bcount, pri, canfail, &bp);

		if (error) {

			/*
			 * B_ASYNC provokes pvn_done to call pageio_done which,
			 * seeing B_REMAPPED, will call bp_mapout.
			 */

			rf_rwap->wr_bufp->b_flags |= B_ERROR;
			pvn_done(rf_rwap->wr_bufp);
		}
		rf_rwap->wr_bufp = NULL;
		*bpp = bp;
		return error;
	}

	/*
	 * Find intervals of resident pages.  Allocate a separate
	 * message block for each contiguous chunk of pages, a separate
	 * one for each nonresident chunk.  Hook the chain of blocks
	 * into a single RFS message.
	 *
	 * In error cases, we dispose of current IO list, depend on caller
	 * to abort pages that are already in streams message.
	 *
	 * Notice extra trip through loop to scavenge last page.
	 */

	poff = uiop->uio_offset & PAGEMASK;
	pend = ptob(btopr(uiop->uio_offset + resid));
	pl = NULL;
	bufp = NULL;

	error = rf_allocmsg(hdrsz, (size_t)0, pri, canfail, NULLCADDR, NULLFRP,
	 &bp);
	if (error) {
		return error;
	}
	lastbp = bp;

	nextresident = resident = rfc_page_lookup(vp, poff, &pl) != NULL;
	opoff = poff;
	poff += PAGESIZE;
	for ( ; poff <= pend; poff += PAGESIZE) {
		off_t	pon;

		ASSERT(!bufp);
		ASSERT(!pl == !resident);
		ASSERT(!lastbp->b_cont);

		if (poff < pend) {
			nextresident = rfc_page_lookup(vp, poff, &pl) != NULL;
		}

		if (nextresident == resident && poff < pend) {
			continue;
		}

		/* End of chunk. */

		pon = uiop->uio_offset & PAGEOFFSET;
		pchunk = poff - opoff;
		iochunk = MIN(pchunk - pon, resid);

		if (resident) {

			/* end resident chunk */

			bufp = pageio_setup(pl, pchunk, vp, B_WRITE | B_ASYNC);
			if (!bufp) {
				error = ENOMEM;
				break;
			}

			bp_mapin(bufp);

			bufp->b_blkno = 0;
			bufp->b_dev = 0;
			bufp->b_edev = 0;

			error = rfesb_pageio_setup(bufp, (size_t)0,
			  uiop->uio_offset & PAGEOFFSET, iochunk,
			  pri, canfail, &nextbp);
			if (!error) {
				bufp = NULL;
				pl = NULL;
			}
		} else {

			/* end nonresident chunk */

			error = rf_allocmsg((size_t)0, iochunk, pri, canfail,
			  NULLCADDR, NULLFRP, &nextbp);
		}

		if (error) {
			break;
		}

		lastbp = lastbp->b_cont = nextbp;
		nextbp = NULL;

		if (resident) {
			uioskip(uiop, iochunk);
		} else {
			error = uiomove((caddr_t)lastbp->b_rptr, (long)iochunk,
			  UIO_WRITE, uiop);
		}

		if (error) {
			break;
		}

		resid -= iochunk;
		resident = nextresident;	/* state change */

		/*
		 * opoff becomes the offset of the first page in the new chunk.
		 */

		opoff = poff;

	} /* for */

	if (error) {
		if (bufp) {

			/*
			 * Turn B_WRITE off and B_READ on so pageout doesn't
			 * try to write this later.  B_ASYNC provokes pvn_done
			 * to call pageio_done which, seeing B_REMAPPED, will
			 * call bp_mapout.
			 */

			ASSERT((bufp->b_flags & (B_REMAPPED|B_ASYNC|B_WRITE)) ==
			  (B_REMAPPED|B_ASYNC|B_WRITE));

			bufp->b_flags &= ~B_WRITE;
			bufp->b_flags |= (B_ERROR | B_READ);
			pvn_done(bufp);
		} else if (pl) {
			pvn_fail(pl, 0);
		}

		rf_freemsg(bp);
		bp = NULL;
	}

	*bpp = bp;
	return error;
}

/*
 * Give up the vnode reference in rep->sdp.  sdp must be locked.
 */
STATIC void
rfcl_dorele(sdp, crp)
	sndd_t		*sdp;
	cred_t		*crp;
{
	int		error = 0;
	vnode_t		*vp = SDTOV(sdp);
	mblk_t		*bp = NULL;
	rcvd_t		*rdp = NULL;
	int		nacked;
	int		vcver = QPTOGP(sdp->sd_queue)->version;
	union rq_arg	rqarg;

	rqarg = init_rq_arg;
	ASSERT(sdp->sd_stat & SDUSED);
	ASSERT(!vp->v_count);
	ASSERT(sdp->sd_stat & SDLOCKED);
	ASSERT(!(vp->v_flag & VROOT));

	(void)rcvd_create(FALSE, RDSPECIFIC, &rdp);
	ASSERT(rdp);
	rdp->rd_sdp = sdp;

	if (vcver > RFS1DOT0) {
		rqarg.rqrele.vcount = sdp->sd_remcnt;
		(void)rf_allocmsg(RF_MIN_REQ(vcver), (size_t)0, BPRI_LO,
		  FALSE, NULLCADDR, NULLFRP, &bp);
		ASSERT(bp);

		rfcl_reqsetup(bp, sdp, crp, RFINACTIVE, R_ULIMIT);
		RF_REQ(bp)->rq_arg = rqarg;
		(void)rf_sndmsg(sdp, bp, RF_MIN_REQ(vcver), rdp, FALSE);
	} else {
		do {
			(void)rf_allocmsg(RF_MIN_REQ(vcver), (size_t)0, BPRI_LO,
			  FALSE, NULLCADDR, NULLFRP, &bp);
			ASSERT(bp);

			rfcl_reqsetup(bp, sdp, crp, RFINACTIVE, R_ULIMIT);
			RF_REQ(bp)->rq_arg = rqarg;

			if ((error = rfcl_xac(&bp, RF_MIN_REQ(vcver), rdp,
			  vcver, FALSE, &nacked)) == 0) {
				rf_freemsg(bp);
				bp = NULL;
			}

		} while (nacked);
	}

	sdp->sd_stat &= ~SDINTER;

	SDUNLOCK(sdp);

	rcvd_free(&rdp);
	if (!error && vp->v_pages) {

		/*
		 * Allow cacheing across last reference.  We don't require
		 * SDCACHE to be on, because it can't hurt us; when caching
		 * is reenabled, the pages will get aborted.  In
		 * the meantime, somebody playing fast and loose (because
		 * otherwise SDCACHE would likely be on) with mmap is free
		 * to hit the cache.
		 */

		sndd_hash(sdp);
	} else {
		sndd_free(&sdp);
	}
}
