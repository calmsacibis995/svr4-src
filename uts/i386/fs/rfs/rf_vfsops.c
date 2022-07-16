/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_vfsops.c	1.3.1.1"
#include "sys/list.h"
#include "sys/types.h"
#include "sys/sysinfo.h"
#include "sys/time.h"
#include "sys/fs/rf_acct.h"
#include "sys/bitmap.h"
#include "sys/errno.h"
#include "sys/stream.h"
#include "sys/tss.h"
#include "sys/param.h"
#include "sys/signal.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/pathname.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_messg.h"
#include "sys/rf_comm.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/rf_debug.h"
#include "sys/vfs.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/fs/rf_vfs.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "sys/statvfs.h"
#include "sys/ustat.h"
#include "sys/inline.h"
#include "sys/rf_adv.h"
#include "rfcl_subr.h"
#include "du.h"
#include "sys/mount.h"
#include "sys/mode.h"
#include "sys/hetero.h"
#include "rf_canon.h"
#include "sys/buf.h"
#include "sys/uio.h"
#include "vm/page.h"
#include "rf_cache.h"
#include "sys/kmem.h"

#ifndef	DEBUG
extern long	dudebug;
#endif

/*
 * VFS ops and subroutines for the RFS client.
 */

/* imports */
extern int	suser();
extern int	copyinstr();
extern int	strcmp();
extern int	rf_state;	/* TO DO: in which header file? */
extern int	rf_nosys();

/*
 *  Don't let RFS initialize unless we have enough resources:
 *      SNDD: one for rf_daemon, one for mount, one for request, one for cache
 *      RCVD: mount, signals, rf_daemon, cache, one to do something
 *      MINGDP: at least one circuit
 */

#define MINSNDD	4
#define MINRCVD	5
#define MINGDP	1

rfc_info_t	rfc_info;	/* client cache accounting */
fsinfo_t	rfcl_fsinfo;	/* client RFS file sys activity */
fsinfo_t	rfsr_fsinfo;	/* server RFS file sys activity */
rf_srv_info_t	rf_srv_info;	/* server accounting other than file sys */

int		rf_availkmem;	/* persistent kmem left to use */

sndd_t		*sndd;
rcvd_t		*rcvd;
gdp_t		*gdp;
rd_user_t	*rd_user;
dev_t		rfdev;

STATIC rf_gift_t	rf_mount_gift = {MOUNT_RD, 0};	/* template */


int	rf_init();
STATIC int	rf_mount();
STATIC int	rf_unmount();
STATIC int	rf_root();
STATIC int	rf_statvfs();
STATIC int	rf_sync();

struct vfsops rf_vfsops = {
	rf_mount,
	rf_unmount,
	rf_root,
	rf_statvfs,
	rf_sync,
	rf_nosys,	/* vget */
	rf_nosys,	/* mountroot */
	rf_nosys,	/* swapvp */
	rf_nosys,	/* filler */
	rf_nosys,	/* filler */
	rf_nosys,	/* filler */
	rf_nosys,	/* filler */
	rf_nosys,	/* filler */
	rf_nosys,	/* filler */
	rf_nosys,	/* filler */
	rf_nosys	/* filler */
};

STATIC int	rf_mount_resp();

/*
 * list header for client remote mount structures, initially empty
 */
rf_head_t rf_head = {(rf_vfs_t*)&rf_head, (rf_vfs_t*)&rf_head};

/*
 * operations on objects of type rf_vfs_t
 */
STATIC int	getrfvfs();
STATIC void	putrfvfs();
rf_vfs_t	*findrfvfs();

STATIC int	rf_type;

int		rfsr_nservers;
int		rfsr_nidle;
int		rfsr_nmsg;

/* ARGSUSED */
void
#ifdef i386
rf_clock(pc, cs)
	caddr_t		pc;
	int		cs;
#else
rf_clock(pc, psw)
	caddr_t		pc;
	psw_t		psw;
#endif
{
	rf_srv_info.rfsi_nservers += rfsr_nservers;
	if (rfsr_nidle) {
		rf_srv_info.rfsi_srv_que += rfsr_nidle;
		rf_srv_info.rfsi_srv_occ++;
	}
	if (rfsr_nmsg) {
		rf_srv_info.rfsi_rcv_que += rfsr_nmsg;
		rf_srv_info.rfsi_rcv_occ++;
	}
}

/*
 * Predicate returns true iff remote file denoted by vnode resides on
 * a pre-SVR4 server.
 */
/*
 * Client calls here to sync remote and local notions of system time.
 */
int
rf_stime(crp)
	register cred_t		*crp;
{
	register struct gdp	*gp = gdp;
	register struct gdp	*endgdp = gdp + maxgdp;
	register int		error = 0;
	sndd_t			*sdp;
	union rq_arg		rqarg;

	if (rf_state != RF_UP) {
		return 0;
	}
	if ((error = sndd_create(FALSE, &sdp)) != 0) {
		return error;
	}
	rqarg = init_rq_arg;
	rqarg.rqsynctime.time = hrestime.tv_sec;
	for ( ; gp < endgdp; gp++) {
		register queue_t	*qp = gp->queue;

		if (qp && (gp->constate == GDPCONNECT) &&
		  gp->version < RFS2DOT0) {
			mblk_t		*bp;
			register int	tmperror;	/* to save error */

			/*
			 * Minimal sndd should be adequate for
			 * communication and recovery.
			 */
			sndd_set(sdp, qp, &rf_daemon_gift);
			if ((tmperror = rfcl_op(sdp, crp, RFSYNCTIME,
			  &rqarg, &bp, FALSE)) == 0) {
				if ((tmperror = RF_RESP(bp)->rp_errno) == 0) {
					gp->timeskew_sec =
					  RF_RESP(bp)->rp_synctime.time -
					  hrestime.tv_sec;
				} else {
					error = tmperror;
				}
				rf_freemsg(bp);
				bp = NULL;
			} else {
				error = tmperror;
			}
		}
	}
	sndd_free(&sdp);
	return error;
}

/*
 * Complete one time initialization of RFS.
 *
 * The allocations here aren't counted against rf_maxkmem because each
 * of them are separately tuneable.
 */
int
rf_init(vfswp, fstyp)
	register vfssw_t	*vfswp;
	int			fstyp;
{
	size_t			snddsz = nsndd * sizeof(sndd_t);
	size_t			rcvdsz = nrcvd * sizeof(rcvd_t);
	size_t			gdpsz = maxgdp * sizeof(gdp_t);
	size_t			rd_usersz = nrduser * sizeof(rd_user_t);
	int			dev;
	int			error = 0;

	if ((dev = getudev()) == -1) {
		cmn_err(CE_WARN, "rf_init: can't get unique device");
		error = ENOMEM;
		goto out;
	}
	rfdev = makedevice(dev, 0);

	rfcl_fsinfo.fsivop_other++;

	/*
	 * Kludge for SVID-compliance is preferable to allocating the
	 * structure in generic code.
	 */
	rfsi_servep = &rf_srv_info.rfsi_serve;
	rf_availkmem = rf_maxkmem;
	if (nsndd < MINSNDD || nrcvd < MINRCVD ||
	  maxgdp < MINGDP || nrduser < 0) {
		sndd = NULL;
		error = EINVAL;
		goto out;
	}
	if ((sndd = (sndd_t *)kmem_zalloc(snddsz, KM_SLEEP)) != NULL) {
		if ((rcvd = (rcvd_t *)kmem_zalloc(rcvdsz, KM_SLEEP)) != NULL) {
			if ((gdp = (gdp_t *)kmem_zalloc(gdpsz, KM_SLEEP))
			  != NULL) {
				if ((rd_user = (rd_user_t *)
				  kmem_zalloc(rd_usersz, KM_SLEEP)) != NULL) {
					goto out;
				}
				kmem_free((caddr_t)gdp, gdpsz);
				gdp = NULL;
			}
			kmem_free((caddr_t)rcvd, rcvdsz);
			rcvd = NULL;
		}
		kmem_free((caddr_t)sndd, snddsz);
		sndd = NULL;
	}
	error = ENOMEM;
out:
	vfswp->vsw_vfsops = &rf_vfsops;
	strcpy(vfswp->vsw_name, "rfs");
	rf_type = fstyp;
	return error;
}

/*
 * Return 0 for success, nonzero errno for failure.
 */
STATIC int
rf_mount(vfsp, mntpt, uap, crp)
	vfs_t		*vfsp;		/* vfs to mount remote resource */
	vnode_t		*mntpt;		/* vnode of mount point dir */
	struct mounta	*uap;		/* user arguments */
	cred_t		*crp;
{
	int		nacked;
	register int	ntries;
	register int	vcver;
	register size_t	totalsz;
	size_t		datasz;
	size_t		headsz;
	rf_mountdata_t	rfmdata;
	char		*rscnm;			/* name w/out domain */
	char		name[MAXDNAME+1];	/* resource name */
	uint		namelen;
	int		error;			/* return value */
	mblk_t		*bp = NULL;
	rcvd_t		*rdp = NULL;
	rf_vfs_t	*rfvfsp = NULL;
	sndd_t		*chansdp = NULL;	/* channel to server */
	sndd_t		*giftsdp = NULL;	/* sd for resource */
	queue_t		*mountqp = NULL;
	int		flags;

	rfcl_fsinfo.fsivop_other++;
	if (!suser(crp)) {
		error = EPERM;
		goto out;
	}
	if (mntpt->v_type != VDIR) {
		error = ENOTDIR;
		goto out;
	}
	if (mntpt->v_count != 1 || mntpt->v_flag & VROOT) {
		error = EBUSY;
		goto out;
	}
	if (uap->datalen != sizeof(rf_mountdata_t)) {
		DUPRINT2(DB_MNT_ADV, "rf_mount: invalid datalen %d\n",
		  uap->datalen);
		error = EINVAL;
		goto out;
	}
	/*
	 * datap containes flags, circuit id, and machine name
	 */
	if (copyin(uap->dataptr, (caddr_t)&rfmdata, sizeof(rf_mountdata_t))) {
		error = EFAULT;
		goto out;
	}
	DUPRINT3(DB_MNT_ADV, "rf_mount: token.t_id=%x, t_uname=%s\n",
	  rfmdata.rfm_token.t_id, rfmdata.rfm_token.t_uname);
	if (rf_state != RF_UP) {
		error = ENONET;
		goto out;
	}
	if (error = gdp_get_circuit(-1, &rfmdata.rfm_token,
	  &mountqp, (gdpmisc_t*)NULL)) {
		DUPRINT3(DB_MNT_ADV,
		  "rf_mount fails: token.t_id=%x, t_uname=%s\n",
		  rfmdata.rfm_token.t_id, rfmdata.rfm_token.t_uname);
		/*
		 * WARNING - this is the ONLY time rf_mount is allowed to
		 * return ENOLINK, because of expectations of mount command.
		 */
		goto out;
	}
	/*
	 * Allocate send descriptor to send mount request across.
	 */
	if (error = sndd_create(TRUE, &chansdp)) {
		goto out;
	}
	sndd_set(chansdp, mountqp, &rf_mount_gift);
	/*
	 * Allocate send descriptor to hold reference to rf_rsrc.
	 */
	if (error = sndd_create(TRUE, &giftsdp)) {
		goto out;
	}
	/*
	 * Bring the advertised name into kernel space.
	 */
	if ((error = copyinstr(uap->spec, name, sizeof(name), &namelen)) != 0) {
		goto out;
	}
	if (namelen == 1 || (rscnm = rsc_nm(name)) == NULL) {
		error = EINVAL;
		goto out;
	}
	if (localrsrc(name) && !(dudebug & DB_LOOPBCK)) {

		/* Loopback mount is only allowed when DB_LOOPBCK is set */
		error = EINVAL;
		goto out;
	}
	if ((error = getrfvfs(name, &rfvfsp)) != 0) {
		goto out;
	}
	/*
	 * For ops other than mount, sd_mntid contains the
	 * server's srmount id for the resource in which the
	 * remote reference exists, and the server expects that
	 * value in every non-mount request.  Therefore rfcl_xac
	 * just loads it from the sd.  In the case of a mount,
	 * though, the protocol says that the mount index supplied
	 * to the server is the client side mount index of the
	 * requested resource.  To make life easy, we load that into
	 * the sd here, so rfcl_xac can find it transparently.
	 */
	chansdp->sd_mntid = rfvfsp->rfvfs_mntid;
	vfsp->vfs_data = (caddr_t)rfvfsp;
	vfsp->vfs_fstype = rf_type;
	vfsp->vfs_dev = rfdev;
	/*
	 * If the link goes down while a process is doing a mount, the
	 * cleanup part of rf_recovery can't use find the process's
	 * specific rd to marke is LINKDOWN, because data structures
	 * are only partially defined at that point.  For the same reason,
	 * fuser can't find and signal the process.  Therefore we set
	 * the rfvfs_mntproc flag for the time that the rf_head is in this
	 * semi-defined state, so cleanup can find and signal the process
	 * if the link drops.
	 */
	rfvfsp->rfvfs_vfsp = vfsp;
	rfvfsp->rfvfs_mntproc = u.u_procp;
	flags = MS_FLAGS_TO_RFS(uap->flags) | rfmdata.rfm_flags;
	if (rfc_time == -1) {
		flags &= ~MS_CACHE;
	}
	/* create an rd on which to receive the response */
	if ((error = rcvd_create(TRUE, RDSPECIFIC, &rdp)) != 0) {
		goto out;
	}
	rdp->rd_sdp = chansdp;
	vcver = QPTOGP(chansdp->sd_queue)->version;
	headsz = RF_MIN_REQ(vcver);
	datasz = C0SIZE(strlen(rscnm));
	totalsz = headsz + datasz;
	for (ntries = nacked = 1; ntries < RFCL_MAXTRIES && nacked; ntries++) {
		register rf_request_t	*reqp;

		if ((error = rf_allocmsg(headsz, datasz, BPRI_LO, TRUE,
		  NULLCADDR, NULLFRP, &bp)) != 0) {
			break;
		}
		rfcl_reqsetup(bp, chansdp, crp, RFMOUNT, R_ULIMIT);
		reqp = RF_REQ(bp);
		(void)rf_tcanon("c0", rscnm, rf_msgdata(bp, headsz));
		reqp->rq_srmount.mntflag = flags;
		/*
		 * Server ignores except for first mount on this circuit.
		 */
		reqp->rq_srmount.synctime = hrestime.tv_sec;
		error = rfcl_xac(&bp, totalsz, rdp, vcver, FALSE, &nacked);
	}
	if (!error) {
		/*
		 * rf_mount_resp may free and NULL giftsdp.
		 */
		error = rf_mount_resp(mntpt, bp, &giftsdp, vfsp,
		  QPTOGP(mountqp), crp);
	}
out:
	rcvd_free(&rdp);
	sndd_free(&chansdp);
	rf_freemsg(bp);
	bp = NULL;
	if (error) {
		sndd_free(&giftsdp);
		gdp_put_circuit(&mountqp);
		putrfvfs(&rfvfsp);
	}
	return error;
}

/*
 * Unmount denoted vfs of type RFS.
 * Return 0 for success, nonzero errno for failure.
 */
STATIC int
rf_unmount(vfsp, crp)
	register vfs_t		*vfsp;
	struct cred		*crp;
{
	rf_vfs_t		*rfvfsp = VFTORF(vfsp);
	register vnode_t	*rootvp = rfvfsp->rfvfs_rootvp;
	sndd_t			*sdp = VTOSD(rootvp);
	register int		error;
	union rq_arg		rqarg;
	mblk_t			*bp = NULL;

	rqarg = init_rq_arg;
	rfcl_fsinfo.fsivop_other++;
	DUPRINT4(DB_MNT_ADV, "rf_unmount: vfsp %x, rfvfsp %x, resource %s\n",
	  vfsp, rfvfsp, rfvfsp->rfvfs_name);
	if (!suser(crp)) {
		return EPERM;
	}
	if (rf_state != RF_UP)  {	  /*  have to be on network  */
		return ENONET;
	}
	if (rfvfsp->rfvfs_refcnt > 1 || rootvp->v_count > 1) {
		/*
		 * More than one sd => more than one file referred to
		 * (i.e., there are remote references active in rf_head).
		 */
		return EBUSY;
	}
        if (QPTOGP(sdp->sd_queue)->version > RFS1DOT0) {
		rqarg.rqrele.vcount = sdp->sd_remcnt;
	}
	/*
	 * Actually do the unmount if the link is already down to the
	 * server (sd_stat & SDLINKDOWN), if it completed successfully on the
	 * server, or if the link to the server goes down (ECOMM or ENOLINK).
	 */
	if (((error = rfcl_op(sdp, crp, RFUMOUNT, &rqarg, &bp, FALSE)) == 0 &&
	   (error = RF_RESP(bp)->rp_errno) == 0) ||
	  error == ENOLINK ||
	  error == ECOMM) {
		ASSERT(rootvp->v_count == 1);
		rfc_mountinval(rfvfsp);
		QPTOGP(sdp->sd_queue)->mntcnt--;
		gdp_put_circuit(&sdp->sd_queue);
		sndd_free(&sdp);
		putrfvfs(&rfvfsp);
		error = 0;
	}
	rf_freemsg(bp);
	return error;
}

/*
 * Update *vpp with root vnode of mounted remote rf_rsrc.
 * Return 0 for success, nonzero errno for failure.
 */
STATIC int
rf_root(vfsp, vpp)
	register vfs_t *vfsp;
	register vnode_t **vpp;
{
	register vnode_t *vp;

	rfcl_fsinfo.fsivop_other++;
	ASSERT(ISRFSVFSP(vfsp));
	*vpp = vp = VFTORF(vfsp)->rfvfs_rootvp;
	VN_HOLD(vp);
	return 0;
}

/*
 * 4.x servers support statvfs requests, so we send the RFSTATVFS opcode
 * across.
 * 3.x servers don't support statvfs requests, but do yield some of the
 * same information through the (f)statfs request.  When this vfs switch
 * operation is invoked, there are two possibilities.
 *
 *	1.  A preceding du_lookup succeeded in doing the statfs.  This is the
 *	    case if there is a stash on the current sndd.
 *
 *	2.  Otherwise, send a statfs request now.
 *
 * If error-free, fill in the referenced statvfs with the statfs and
 * additional information available through the vfs structure.
 */

STATIC int
rf_statvfs(vfsp, stvfsp)
	vfs_t		*vfsp;
	statvfs_t	*stvfsp;
{
	vnode_t		*vp = VFTORF(vfsp)->rfvfs_rootvp;
	sndd_t		*chansdp = VTOSD(vp);
	gdp_t		*gp = QPTOGP(chansdp->sd_queue);
	int		error;
	mblk_t		*bp = NULL;
	size_t		datasz;
	size_t		rpsz;

	rfcl_fsinfo.fsivop_other++;
	if (gp->version < RFS2DOT0) {
		int du_fstatfs();

		return du_fstatfs(vfsp, stvfsp);
	}
	datasz = gp->hetero == NO_CONV ? sizeof(statvfs_t) :
	  sizeof(statvfs_t) + STATVFS_XP;
	rpsz = RF_MIN_RESP(gp->version);
	if ((error = rfcl_op(chansdp, u.u_cred, RFSTATVFS, &init_rq_arg, &bp,
	  TRUE)) == 0 && (error = RF_RESP(bp)->rp_errno) == 0) {

		if (RF_PULLUP(bp, rpsz, datasz)) {
			gdp_j_accuse("rf_statvfs bad data from server", gp);
			error = EPROTO;
		} else {
			caddr_t	rpdata = rf_msgdata(bp, rpsz);

			if (gp->hetero != NO_CONV &&
			  !rf_fcanon(STATVFS_FMT, rpdata, rpdata + datasz,
			   (caddr_t)stvfsp)) {
				gdp_j_accuse("rf_statvfs bad data from server",
				  gp);
				error = EPROTO;
			}else if(gp->hetero == NO_CONV) { 
				*stvfsp = *(statvfs_t *)rpdata;
			}
			if (!error) {
				/*
				 * NOSUID is strictly an attibute of the client
				 * mounting, so disregard the server's value.
				 * Turn off other flags not understood locally
				 * and take the union of client and server
				 * RDONLY and NOTRUNC values.
				 */
				stvfsp->f_flag &= ST_RDONLY | ST_NOTRUNC;
				stvfsp->f_flag |= vf_to_stf(vfsp->vfs_flag);
			}
		}
	}
	rf_freemsg(bp);
	return error;
}

STATIC int
rf_sync(vfsp, flag, crp)
	register vfs_t	*vfsp;
	short		flag;
	cred_t		*crp;
{
	register sndd_t	*sdp = sndd;
	register sndd_t	*sd_toofar = sdp + nsndd;

	rfcl_fsinfo.fsivop_other++;
	if (rf_state != RF_UP) {
		return 0;
	}
	/*
	 * Asynchronously flush all dirty pages for specified rf_vfs objects
	 * (or all, if vfsp == NULL).  Attributes are not updated locally,
	 * so we don't have a SYNC_ATTR case.
	 */
	if (flag != SYNC_ATTR) {
		for ( ; sdp < sd_toofar; sdp++) {

			/*
			 * When vfsp == NULL, check for v_vfsp != NULL because
			 * sndds can be used for other than references to 
			 * remote files.
			 */
			if (sdp->sd_stat & SDUSED) {
				register vnode_t	*vp = SDTOV(sdp);

				if (!vfsp && vp->v_vfsp ||
				  vfsp && vp->v_vfsp == vfsp) {
					(void)VOP_PUTPAGE(vp, (uint)0, (uint)0,
					  B_ASYNC, crp);
				}
			}
		}
	}
	return 0;
}

/*
 * Process response to succesfully completed rf_mount.
 * Return 0 for success, nonzero errno for failure.
 * Does type-dependent updating of vfs, rf_head, allocated vnode,
 * gift sd, etc.
 *
 * Note:  mntpt unused only in the presence of unnecessary restriction
 * against mounting nondirectories.
 */
/* ARGSUSED */
STATIC int
rf_mount_resp(mntpt, bp, giftsdpp, vfsp, gp, crp)
	vnode_t		*mntpt;
	mblk_t		*bp;
	register sndd_t	**giftsdpp;
	vfs_t		*vfsp;
	gdp_t		*gp;
	cred_t		*crp;
{
	register rf_response_t	*rp = RF_RESP(bp);
	register rf_message_t	*msg = RF_MSG(bp);
	register rf_vfs_t	*rfvfsp = VFTORF(vfsp);
	vnode_t			*vp;
	register int		error;

	/*
	 * Clean up the gift and return non-zero when rp_errno,
	 * misbehaving server or a fumount message beat us back from server.
	 * Other ops detect this last condition in rf_sndmsg or rf_rcvmsg
	 * because their data structures are already hooked into the VFS, can
	 * be found by rf_recovery/fumount.
	 */
	if ((error = rp->rp_errno) != 0 || !(msg->m_stat & RF_GIFT) ||
	  rfvfsp->rfvfs_flags & MFUMOUNT) {
		rfcl_giftfree(bp, giftsdpp, crp);
		return error ? error : ENOLINK;
	}
	rfvfsp->rfvfs_refcnt = 0;	/* incremented by rfcl_findsndd */
	sndd_set(*giftsdpp, (queue_t *)msg->m_queue, &msg->m_gift);
     	if (!(error = rfcl_findsndd(giftsdpp, crp, bp, vfsp))) {

		/*
		 * With current implementation, rfcl_findsndd will actually
		 * just return the vnode attached to current giftsd,
		 * because there is no previous reference to the rf_rsrc.
		 * (Even if it were a subdirectory of another mounted
		 * resource, and we had a reference to that subdirectory,
		 * the server allocates a new rcvd for the first mount.)
		 * We make the call primarily to get the vnode filled in;
		 * the following assignment is redundant, then, but harmless.
		 */

		vp = SDTOV(*giftsdpp);
		vp->v_flag |= VROOT;

		/*
		 * Finish defining upper level VFS.
		 *
		 * fsid == <sysid, sd_mntid>
		 *
		 * Defining sysid is redundant after the first mount on
		 * a circuit, but so what?
		 *
		 * Even talking to old servers, large RFS messages are
		 * cheaper than the equivalent number of small ones.
		 */

		vfsp->vfs_bsize = MAXBSIZE;
		hibyte(gp->sysid) = lobyte(loword(RF_COM(bp)->co_sysid));
		DUPRINT2(DB_MNT_ADV, "rmount: set sysid to %d\n", gp->sysid);
		vfsp->vfs_fsid.val[0] = gp->sysid;
		vfsp->vfs_fsid.val[1] = (*giftsdpp)->sd_mntid;
		/*
		 * Finish defining lower level DUFS
		 */
		if (!(msg->m_stat & RF_VER1)) {
		    rfvfsp->rfvfs_flags &= ~MCACHE;
		} else {
		    rfvfsp->rfvfs_flags |= (rp->rp_rval & MCACHE);
		}
		rfvfsp->rfvfs_rootvp = vp;
		rfvfsp->rfvfs_mntproc = NULL;
		gp->mntcnt++;
	}
	return error;
}

/*
 * primitive operations on objects of type rf_vfs_t
 */

/*
 * bit map for tracking pseudo-indices.
 * Because RFS always assumed that a mount index would fit in a uchar,
 * and pervasively assumed an 8 bit byte, 256 is a big enough map for
 * compatability with 3.x machines.
 */
#define MAPSZ 256
STATIC ulong	rfvfs_map[BT_BITOUL(MAPSZ)];

/*
 * If name is a remote resource already mounted, or
 * space to mount another remote resource, returns and errno.
 * Otherwise returns 0 and updates *rfvfspp with a pointer to a free
 * rf_vfs_t with the index and name defined, and linked into the rf_vfs
 * chain, but with other members zeroed.
 *
 * We link the partially-defined rf_head into the chain because
 * client-side rf_recovery has to be able to find it in case a
 * link drops or a fumount message comes in before a mount
 * completes.  Operations other than mount get the bad news
 * in receive and send descriptor flags but, because mount is
 * a bootstrapping kind of operation, its sndds and rcvds are
 * not well-defined either, so fumount-rf_recovery can't find them.
 * It's safe to put the rf_head on the list because the only thing
 * client fumount does is match on index, then look at root vp.
 */
STATIC int
getrfvfs(name, rfvfspp)
	register char		*name;
	rf_vfs_t			**rfvfspp;
{
	register rf_vfs_t	*newrfvfs;
	register rf_vfs_t	*rfp = rf_head.rfh_next;
	register rf_vfs_t	*headp = (rf_vfs_t *)&rf_head;
	register index_t	inx = bt_availbit(rfvfs_map, MAPSZ);

	/*
	 * To avoid races, we allocate all resources before checking
	 * the list for duplicate names, so we don't sleep between
	 * having ascertained absence of duplicates, and putting
	 * the rf_head in the list.
	 * Don't bother with rf_maxkmem since the administrator is controlling
	 * these mounts.
	 */
	if (inx < 0) {
		return ENOMEM;
	}
	BT_SET(rfvfs_map, inx);
	if ((newrfvfs =
	    (rf_vfs_t *)kmem_zalloc(sizeof(rf_vfs_t), KM_SLEEP)) == NULL) {
		BT_CLEAR(rfvfs_map, inx);
		return ENOMEM;
	}
	while (rfp != headp) {
		/*
		 * Check for name duplications
		 * TO DO: sort?
		 */
		if (!strcmp(rfp->rfvfs_name, name)) {
			/* duplicate name */
			kmem_free((caddr_t)newrfvfs, sizeof(rf_vfs_t));
			BT_CLEAR(rfvfs_map, inx);
			return EBUSY;
		}
		rfp = rfp->rfvfs_next;
	}
	newrfvfs->rfvfs_mntid = inx;
	(void)strcpy(newrfvfs->rfvfs_name, name);
	LS_INIT(newrfvfs);
	LS_INIT(&newrfvfs->rfvfs_sdhash);
	LS_INS_AFTER(headp, newrfvfs);
	*rfvfspp = newrfvfs;
	return 0;
}

/*
 * Unlink denoted DUFST dependent file sys from chain and
 * return to free pool.
 * Assumes dependent data structures, e.g., name, already
 * cleaned up.
 * NULLs *rfsvpp.
 */
STATIC void
putrfvfs(rfvfspp)
	rf_vfs_t		**rfvfspp;
{
	register rf_vfs_t	*rfvfsp = *rfvfspp;

	if (rfvfsp) {
		BT_CLEAR(rfvfs_map, rfvfsp->rfvfs_mntid);
		LS_REMOVE(rfvfsp);
		kmem_free((caddr_t)rfvfsp, sizeof(rf_vfs_t));
		*rfvfspp = NULL;
	}
}

/*
 * Returns a pointer to the rf_head with denoted index, or NULL
 * if no such rf_head exists.
 */
rf_vfs_t *
findrfvfs(id)
	register long		id;
{
	register rf_vfs_t	*rfvfsp = rf_head.rfh_next;
	register rf_vfs_t	*headp = (rf_vfs_t *)&rf_head;

	if (BT_TEST(rfvfs_map, id)) {
		while (rfvfsp != headp) {
			if (rfvfsp->rfvfs_mntid == id) {
				return rfvfsp;
			}
			rfvfsp = rfvfsp->rfvfs_next;
		}
	}
	return NULL;
}

STATIC int	rf_ustat_sdget();

int
rf_ustat(dev, ustbuf)
	dev_t			dev;
	struct ustat		*ustbuf;
{
	/*
	 * This routine is necessarily heuristic; we don't really know we
	 * have a remote reference, since we're dealing with a dev.
	 * In the millenium, this system call won't be supported.
	 */

	short			sdev;
	register int		error;
	mblk_t			*bp = NULL;
	gdp_t			*gp;
	sndd_t			*chansdp;
	size_t			datasz;
	size_t			rpsz;
	union rq_arg		rqarg;

	rqarg = init_rq_arg;
	if (rf_state != RF_UP) {
		return EINVAL;
	}

	ASSERT((long)dev < 0);
#ifdef SHORT_DEVS
	sdev = dev;
#else
	hibyte(sdev) = (unchar)hiword(dev);
	lobyte(sdev) = (unchar)loword(dev);
#endif
	ASSERT(sdev < 0);

	if ((error = rf_ustat_sdget(sdev, &chansdp, &gp)) != 0) {
		return error;
	}
	rqarg.rqustat.buf = (long)ustbuf;
	rqarg.rqustat.dev = (long)lobyte(sdev);

	datasz = gp->hetero == NO_CONV ? sizeof(struct ustat) :
	  sizeof(struct ustat) + USTAT_XP;
	rpsz = RF_MIN_RESP(gp->version);
	if ((error = rfcl_op(chansdp, u.u_cred, RFUSTAT, &rqarg, &bp, TRUE)) ==
	  0 && (error = RF_RESP(bp)->rp_errno) == 0) {
		if (RF_PULLUP(bp, rpsz, datasz)) {
			gdp_j_accuse("rf_ustat bad data", gp);
			error = EPROTO;
		} else {
			caddr_t	rpdata = rf_msgdata(bp, rpsz);

			if (gp->hetero != NO_CONV &&
			  !rf_fcanon(USTAT_FMT, rpdata, rpdata + datasz,
			   rpdata)) {
				gdp_j_accuse("rf_ustat bad data", gp);
				error = EPROTO;
			}
			if (!error && copyout(rpdata, (caddr_t)ustbuf,
			  sizeof(struct ustat))) {
				error = EFAULT;
			}
		}
	}
	rf_freemsg(bp);
	sndd_free(&chansdp);
	return error;
}

/*
 * Assumes that sdev is the old, short, representation.
 */
STATIC int
rf_ustat_sdget(sdev, sdpp, gpp)
	short		sdev;
	sndd_t		**sdpp;
	register gdp_t	**gpp;
{
	sndd_t		*sdp;
	sndd_t		*searchsdp;
	sndd_t		*endsd = sndd + nsndd;
	short		index = (short)((~(hibyte(sdev))) & 0x00ff);

	if (index >= maxgdp || (*gpp = &gdp[index])->constate != GDPCONNECT) {
		return ENOENT;
	}
	if (sndd_create(TRUE, &sdp) != 0) {
		return ENOMEM;
	}
	sdp->sd_stat |= SDINTER;	/* This prevents a match on our sdp */
	/*
	 * Since we have a circuit to some server, we'll use the mntindx
	 * from some sndd in some resource in that channel to convince
	 * the serve to let us in.  This depends on details of the
	 * server implementation.
	 */
	searchsdp = sndd;
	while (searchsdp < endsd) {
		if (searchsdp->sd_stat & SDUSED &&
		  !(searchsdp->sd_stat & SDINTER) &&
		  QPTOGP(searchsdp->sd_queue) == *gpp) {
			if (searchsdp->sd_stat & SDLOCKED) {
				searchsdp->sd_stat |= SDWANT;
				(void)sleep((caddr_t)searchsdp, PSNDD);
				searchsdp = sndd;
				continue;
			} else {
				break;
			}
		}
		searchsdp++;
	}
	if (searchsdp == endsd) {
		sndd_free(&sdp);
		return ENOENT;
	} else {
		sndd_set(sdp, (*gpp)->queue, &rf_mount_gift);
		sdp->sd_mntid = searchsdp->sd_mntid;
		*sdpp = sdp;
		return 0;
	}
}
