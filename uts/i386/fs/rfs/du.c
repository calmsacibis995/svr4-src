/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/du.c	1.3.1.2"

#include "sys/types.h"
#include "sys/sysinfo.h"
#include "sys/fs/rf_acct.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/uio.h"
#include "sys/file.h"
#include "sys/pathname.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/conf.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/sysmacros.h"
#include "sys/cmn_err.h"
#include "sys/stream.h"
#include "sys/rf_messg.h"
#include "sys/nserve.h"
#include "sys/list.h"
#include "sys/mode.h"
#include "sys/idtab.h"
#include "sys/rf_cirmgr.h"
#include "vm/seg.h"
#include "rf_admin.h"
#include "sys/rf_comm.h"
#include "sys/fs/rf_vfs.h"
#include "sys/debug.h"
#include "sys/rf_debug.h"
#include "sys/rf_adv.h"
#include "rf_serve.h"
#include "sys/kmem.h"
#include "sys/utime.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "sys/statvfs.h"
#include "sys/hetero.h"
#include "rf_canon.h"
#include "rfcl_subr.h"
#include "rf_auth.h"
#include "du.h"
#include "sys/inline.h"
#include "sys/buf.h"
#include "vm/page.h"
#include "rf_cache.h"

/*
 * RFS ops and subroutines specific to the 3.X system call protocol.
 *
 * Client system call counterparts have the prefix dusys_".
 * vnode op-counterparts have the prefix "du_".
 * Client subroutines have the prefix "ducl_".
 * Server ops and subroutines have the prefix "dusr_".
 */

/* imports */
extern void	xrele();
extern int	strlen();
extern int	strcmp();
extern int	suser();
extern int	filesearch();

STATIC int	ducl_namemsg();
STATIC int	ducl_resetpath();
STATIC int	dusys_exec();
STATIC int	dusys_mknod();
STATIC int	dusys_rmount();
STATIC int	dusys_rumount();
STATIC int	du_caccess();
STATIC int	dusys_access();
STATIC int	du_fcntl_resp();
STATIC int	dusys_stat();
STATIC int	dusys_link();
STATIC int	du_link_chdir();
STATIC int	dusys_mkdir();
STATIC int	dusys_copen();
STATIC int	dusys_copen_resp();
STATIC int	dusys_unlink();
STATIC int	dusys_rename();
STATIC int	dusys_rmdir();
STATIC int	dusys_chdirec();
STATIC int	dusys_chown();
STATIC int	dusys_chmod();
STATIC int	dusys_utime();
STATIC int	dusys_utime_pass();
STATIC int	dusys_statfs();
STATIC void	du_stat_to_vattr();
STATIC int	du_fs_to_vfs();
STATIC int	du_o_flock_to_flock();
STATIC int	dusr_vn_open();
STATIC int	dusr_vn_create();
STATIC int	dusr_vn_link();
STATIC int	dusr_vn_remove();
STATIC int	dusr_cstat();
STATIC int	dusr_cstatfs();
STATIC int	dusr_namesetattr();

/*
 * Operations supporting "stashing" of remote file data, making the
 * system call protocol appear to conform to the vnode/VFS interface.
 *
 * In the old RFS system call based-protocol (DU), name-based operations
 * do more than a pure lookup.
 * The fundamental idea is this:  du_lookup passes its arguments,
 * the current system call, and other information from the u_block
 * necessary to the operation in progress, to  operation-specific
 * functions.  Those operations then complete the system call but,
 * instead of returning the result to the upper level immediately,
 * allocate a dustash_t to hold the result, and stash it in
 * the send descriptor that is the private part of the vnode representing
 * the remote file.  Du_lookup returns that vnode to the upper level.
 * Operations invoked after lookup fetch information from the stashed
 * data, and return it to the upper level without a network access.
 * The last such operation for the current system call deallocates
 * the stash.
 */

struct exec_ids {
	uid_t ex_uid;
	gid_t ex_gid;
};

/*
 * The "next" pointer chains together structures stashed on the same
 * send descriptor The sequence number lets us keep sane if system
 * calls are interrupted; we can unambiguously bind stashes to instances
 * of system calls.
 *
 * We use a union rather than an array of tagged length to have fixed
 * size structures that can be quickly allocated and deallocated.
 */
typedef struct dustash {
	struct dustash *dst_nextp;
	pid_t dst_pid;
	union {
		/* per-operation data */
		struct stat dst_stat;
		struct statfs dst_statfs;
		struct exec_ids dst_exec;
		vnode_t *dst_vp;
	} dst_u;
} dustash_t;

#define DST_STAT	dst_u.dst_stat
#define DST_STATFS	dst_u.dst_statfs
#define DST_EXEC	dst_u.dst_exec
#define DST_VP		dst_u.dst_vp

STATIC dustash_t	*dst_alloc();
STATIC void		dst_free();
STATIC dustash_t	*dst_unlink();
void			dst_clean();

/*
 * Link denoted dustash to denoted sd.
 * These expressions must me free of side effects
 */
#define DST_LINK(sdp, dstp) {				\
	(dstp)->dst_nextp = (sdp)->sd_stashp;		\
	(sdp)->sd_stashp = (dstp);			\
}

/*
 * Fakevn is a vnode that doesn't refer to a file, but is filled in to look
 * like an RFS vnode.  Its reference count is initially 1 so it will persist.
 */
STATIC vnode_t fakevn =
{
	0,		/* v_flag */
	1,		/* v_count */
	NULL,		/* v_vfsmountedhere */
	&rf_vnodeops,	/* v_op */
	NULL,		/* v_vfsp */
	NULL,		/* v_stream */
	NULL,		/* v_pages */
	VDIR,		/* v_type */
	0,		/* v_rdev */
	NULL,		/* v_data */
	NULL		/* v_filocks */
			/* filler */
};
/* X286H is defined to 50 to catch sysi86 syscalls generated from 
   /usr/bin/x286emul.  Want to call du_exec to exectute x286 executable
   on the remote machine. */
#define X286H	50

/*
 * du_lookup is called for name-based system calls with old servers,
 * or with new servers for ops that haven't been converted to pure vnode
 * ops.
 *
 * This routine calls other routines, based on the system call in progress,
 * that not only traverse the remaining pathname, but also perform the
 * appropriate operation on the effected file.
 *
 * Because the upper level only expects rf_lookup to return a vnode pointer,
 * the routine called by du_lookup may have to provide for intermediate stashing
 * of results.  After completing the pathname translation, the upper level
 * will switch to a rf_vfs_t vnode operation handling routine.  That routine
 * will retrieve the stashed result to return to the upper level.
 */
int
du_lookup(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* is containing directory needed? */
	vnode_t 	*rdirvp;	/* root vnode for this process */
	cred_t 		*crp;		/* credentials structure */
{
	ASSERT(QPTOGP(VTOSD(dvp)->sd_queue)->version == RFS1DOT0);
	if (strlen(comp) + pnp->pn_pathlen + 1 > DU_DATASIZE) {
		pn_setlast(pnp);
		return ENOMEM;
	}
	/*
	 * call appropriate routine based on syscall in progress
	 */
	switch (u.u_syscall) {
	case DUSACCESS:
		return dusys_access(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUCHDIR:
	case DUCHROOT:
		return dusys_chdirec(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUCHMOD:
		return dusys_chmod(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUCHOWN:
	case DULCHOWN:
		return dusys_chown(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUCOREDUMP:
	case RFCREATE:
	case RFOPEN:
	case DUSYSACCT:
		return dusys_copen(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUEXEC:
	case DUEXECE:
	case X286H:
		return dusys_exec(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DULINK:
		return dusys_link(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case RFMKDIR:
		return dusys_mkdir(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUMKNOD:
	case DUXMKNOD:
		return dusys_mknod(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUMOUNT:
		return dusys_rmount(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case RFRMDIR:
		return dusys_rmdir(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUSTATFS:
	case DUSTATVFS:
		return dusys_statfs(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DULSTAT:
	case DUSTAT:
	case DULXSTAT:
	case DUXSTAT:
		return dusys_stat(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUUMOUNT:
		return dusys_rumount(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUUNLINK:
		return dusys_unlink(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DUUTIME:
		return dusys_utime(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	case DURENAME:
		return dusys_rename(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	default:
		pn_setlast(pnp);
		return EREMOTE;
	}
}

/* ARGSUSED */
STATIC int
dusys_access(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* is containing directory needed? */
	vnode_t 	*rdirvp;	/* root vnode for this process */
	cred_t 		*crp;		/* credentials structure */
{
	struct a {
		char *fname;
		int fmode;
	}	*uap = (struct a *)u.u_ap;
	/*
	 * we must switch real and effective u and g ids here, since
	 * the upper level hasn't supplied us its duped cred structure.
	 */
	cred_t	*tmpcrp;
	int	error;

	tmpcrp = crdup(u.u_cred);
	tmpcrp->cr_uid = u.u_cred->cr_ruid;
	tmpcrp->cr_gid = u.u_cred->cr_rgid;
	tmpcrp->cr_ruid = u.u_cred->cr_uid;
	tmpcrp->cr_rgid = u.u_cred->cr_gid;
	error = du_caccess(dvp, comp, vpp, pnp, rdirvp, tmpcrp, uap->fmode);
	crfree(tmpcrp);
	return error;
}

/*
 * Handle the chdir and chroot system calls by going remote from
 * du_lookup, and completing the system call on the server before
 * returning.  In the case of an error, zap pathname structure.
 *
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_chdirec(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t			*dvp;		/* current directory */
	char			*comp;		/* current component of name */
	vnode_t			**vpp;		/* return vnode pointer */
	pathname_t		*pnp;		/* pathname structure */
	int			flags;		/* not used */
	vnode_t			*rdirvp;	/* root for this process */
	cred_t			*crp;		/* credentials structure */
{
	mblk_t			*bp = NULL;
	sndd_t			*chansdp = VTOSD(dvp);
	sndd_t			*giftsdp;
	register int		error;

	if ((error = sndd_create(TRUE, &giftsdp)) != 0) {
		return error;
	}
	error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUCHDIR,
  	  &init_rq_arg, &bp);
	if (!error && (error = RF_RESP(bp)->rp_errno) == 0) {
		/*
		 * Either RFDOTDOT, RFPATHREVAL or the system call
		 * completed succesfully. Update pnp and vpp for du_lookup
		 * to return. Get new reference to result vnode.
		 */
		if (RF_COM(bp)->co_opcode == DUCHDIR) {
			register rf_message_t *msg = RF_MSG(bp);

			if (!(msg->m_stat & RF_GIFT)) {
				gdp_j_accuse("dusys_chdirec: no file reference",
				  QPTOGP((queue_t *)msg->m_queue));
				error = EPROTO;
				goto out;
			}
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Set up for rfcl_findsndd or del_sndd
			 */
			sndd_set(giftsdp, msg->m_queue, &msg->m_gift);
			if (!(error = rfcl_findsndd(&giftsdp, crp,
			  bp, dvp->v_vfsp))) {
				*vpp = SDTOV(giftsdp);
			} else {
				*vpp = NULLVP;
			}
		} else {
			sndd_free(&giftsdp);
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		}
		rf_freemsg(bp);
		return error;
	}
out:
	/* Error condition from ducl_namemsg or response message */
	rfcl_giftfree(bp, &giftsdp, crp);
	rf_freemsg(bp);
	pn_setlast(pnp);
	return error;
}

/*
 * Handle the exec and exece system calls by going remote from
 * du_lookup, and completing the system call on the server before
 * returning.
 *
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_exec(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t			*dvp;		/* current directory */
	char			*comp;		/* current component of name */
	vnode_t			**vpp;		/* return vnode pointer */
	pathname_t		*pnp;		/* pathname structure */
	int			flags;		/* not used */
	vnode_t			*rdirvp;	/* root for this process */
	cred_t			*crp;		/* credentials structure */
{
	mblk_t			*bp = NULL;
	sndd_t			*chansdp = VTOSD(dvp);
	sndd_t			*giftsdp;
	register int		error;

	if ((error = sndd_create(TRUE, &giftsdp)) != 0) {
		return error;
	}
	error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUEXEC,
  	  &init_rq_arg, &bp);
	if (!error && (error = RF_RESP(bp)->rp_errno) == 0) {
		/*
		 * Either RFDOTDOT, RFPATHREVAL or the system call
		 * completed succesfully. Update pnp and vpp for du_lookup
		 * to return. Get new reference to result vnode.
		 */
		if (RF_COM(bp)->co_opcode == DUEXEC) {
			register rf_message_t *msg = RF_MSG(bp);

			if (!(msg->m_stat & RF_GIFT)) {
				gdp_j_accuse("dusys_exec: no file reference",
				  QPTOGP((queue_t *)msg->m_queue));
				error = EPROTO;
				goto out;
			}
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Set up for rfcl_findsndd or del_sndd
			 */
			sndd_set(giftsdp, msg->m_queue, &msg->m_gift);
			if ((error = rfcl_findsndd(&giftsdp, crp,
			  bp, dvp->v_vfsp)) == 0) {
				register dustash_t	*dstp = dst_alloc();
				rf_common_t		*cop = RF_COM(bp);

				dstp->dst_pid = u.u_procp->p_pid;
				dstp->DST_EXEC.ex_uid = (uid_t)cop->co_uid;
				dstp->DST_EXEC.ex_gid = (gid_t)cop->co_gid;
				DST_LINK(giftsdp, dstp);
				*vpp = SDTOV(giftsdp);
			} else {
				*vpp = NULLVP;
			}
		} else {
			sndd_free(&giftsdp);
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		}
		rf_freemsg(bp);
		return error;
	}
out:
	/* Error condition from ducl_namemsg or response message */
	rfcl_giftfree(bp, &giftsdp, crp);
	rf_freemsg(bp);
	pn_setlast(pnp);
	return error;
}

/*
 * Handle the mknod system call by going remote from
 * du_lookup, and completing the system call on the server before
 * returning. In the case of an error, zap pathname  structure.
 *
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_mknod(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t			*dvp;		/* current directory */
	char			*comp;		/* current component of name */
	vnode_t			**vpp;		/* return vnode pointer */
	pathname_t		*pnp;		/* pathname structure */
	int			flags;		/* not used */
	vnode_t			*rdirvp;	/* root for this process */
	cred_t			*crp;		/* credentials structure */
{
	mblk_t			*bp = NULL;
	sndd_t			*chansdp = VTOSD(dvp);
	register int		error;
	union rq_arg		rqarg;

	rqarg = init_rq_arg;
	if (u.u_syscall == DUMKNOD) {
		register struct a {
			char	*path;
			mode_t	mode;
			dev_t	dev;
		} *uargp = (struct a *)u.u_ap;

		rqarg.rqmknod.dev = (long)uargp->dev;
		rqarg.rqmknod.fmode = (long)uargp->mode;
	} else {				/* DUXMKNOD */
		register struct a {
			int	ver;		/* version # of this syscall */
			char	*path;
			mode_t	mode;
			dev_t	dev;
		} *uargp = (struct a *)u.u_ap;
		dev_t	rdev;

		if (S_ISBLK(uargp->mode) || S_ISCHR(uargp->mode)) {
			 if ((rdev = cmpdev(uargp->dev)) == NODEV) {
				return EINVAL;
			}
		} else {
			rdev = 0;
		}
		rqarg.rqmknod.dev = rdev;
		rqarg.rqmknod.fmode = (long)uargp->mode;
	}
	rqarg.rqmknod.cmask = (long)u.u_cmask;
	error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUMKNOD,
	  &rqarg, &bp);
	if (!error && !(error = RF_RESP(bp)->rp_errno)) {
		/* Either dusys_mknod completed successfully or
		 * the lookup continues.
		 * Update pnp and vpp for du_lookup to return.
		 * Give back a fakevnode which will get released later
		 * in the upper level.
		 */
		if (RF_COM(bp)->co_opcode == DUMKNOD) {
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Upper level expects back a reference,
			 * will do a VN_RELE
			 */
			VN_HOLD(&fakevn);
			*vpp = &fakevn;
		} else {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		}
		rf_freemsg(bp);
		return error;
	}
	/* Error condition from ducl_namemsg or response message */
	rf_freemsg(bp);
	pn_setlast(pnp);
	return error;
}

/*
 * dusys_rmount is called from du_lookup when the system call in progress
 * is mount.  The current pathname component names a remote
 * file, so the balance of the pathname must be parsed on the server.
 * Irrespective of other errors, if the pathname ends on the server
 * (does not cross back to the client), the server will return an
 * error.
 *
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_rmount(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* not used */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	int		error = 0;
	sndd_t		*chansdp = VTOSD(dvp);
	mblk_t		*bp = NULL;

	/*
	 * The current syscall is mount, but old servers expect
	 * DURMOUNT instead.
	 */
	if (!(error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DURMOUNT,
	  &init_rq_arg, &bp))) {
		ASSERT(bp);
		if (!(error = RF_RESP(bp)->rp_errno)) {
			/*
			 * Either RFDOTDOT or the system call completed
			 * succesfully. In the former case, update *vpp,
			 * kicking the refernce count.  The later case is
			 * an error, since it implies the server is letting
			 * us mount something on a remote directory; we really
			 * don't know what will happen, because a gift has
			 * been dropped somewhere.
			 */
			if (RF_COM(bp)->co_opcode != DURMOUNT) {
				error = ducl_resetpath(bp, pnp, dvp, vpp);
			} else {
				pn_setlast(pnp);
				gdp_discon("dusys_rmount mount on remote name",
				  QPTOGP(chansdp->sd_queue));
				error = EPROTO;
			}
		} else {
			pn_setlast(pnp);
		}
	} else {
		ASSERT(!bp);
		pn_setlast(pnp);
	}
	return error;
}

/*
 * Handle a umount system call with a pathname that goes remote.
 * It is an error if the pathname does not cross back over.
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_rumount(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int		flags;		/* not used */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	/*
	 * We do an access sys call on the pathname.  Note that this is,
	 * strictly speaking, incompatible, because rumount never had to
	 * have search permissions on the server.  Any lookup-based
	 * implementation would suffer the same drawback.  We use the
	 * saccess code as a convenience, however, unlike the access
	 * system call, we use effective ids.
	 */
	int		error = 0;
	char		dotdot[MAXNAMELEN];

	/* 0 arg is access mode for existence */
	if (!(error = du_caccess(dvp, comp, vpp, pnp, rdirvp, crp, 0)) &&
	  ((error = pn_peekcomponent(pnp, dotdot)) ||
	   (error = (strcmp(dotdot, "..") ? EREMOTE : 0)))) {

		/*
		 * Access succeeded remotely, but we either got
		 * an error in pn_peekcomponent, or we failed to
		 * DOTDOT back.  In the latter case, we are trying
		 * to unmount a remote pathname.
		 */

		VN_RELE(*vpp);
		*vpp = NULL;
	}
	return error;
}

/*
 * Send a stat request to server.  If no error, stash result on send
 * descriptor of current vnode.  Otherwise, zap pathname structure.
 *
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_stat(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* containing dir needed? */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	int		error = 0;
	sndd_t		*chansdp = VTOSD(dvp);
	gdp_t		*gp = QPTOGP(chansdp->sd_queue);
	mblk_t		*bp = NULL;
	size_t		datasz;
	union rq_arg	rqarg;
	struct stat	sb;		/* server wants an address */

	rqarg = init_rq_arg;
	rqarg.rqstat_op.buf = (long)&sb;
	datasz = gp->hetero == NO_CONV ? sizeof(struct stat) :
	  sizeof(struct stat) + STAT_XP;
	if ((error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUSTAT,
	  &rqarg, &bp)) == 0 && (error = RF_RESP(bp)->rp_errno) == 0) {
		register dustash_t	*dstp;
		caddr_t			rpdata;

		/*
		 * Either RFDOTDOT or valid stat data was received.
		 * Update pnp and vpp for du_lookup to return, and
		 * stash stat info for later request from upper level
		 * if any is received.
		 */

		if (RF_COM(bp)->co_opcode != DUSTAT) {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
			goto out;
		}
		
		if (RF_PULLUP(bp, RFV1_MINRESP, datasz)) {
			gdp_j_accuse("dusys_stat bad data", gp);
			error = EPROTO;
			goto out;
		}
		dstp = dst_alloc();
		rpdata = rf_msgdata(bp, RFV1_MINRESP);
		dstp->dst_pid = u.u_procp->p_pid;
		if (gp->hetero != NO_CONV) {
			if (!rf_fcanon(STAT_FMT, rpdata, rpdata +
			  datasz, (caddr_t)&dstp->DST_STAT)) {
				gdp_j_accuse("dusys_stat bad data", gp);
				error = EPROTO;
				goto out;
			}
		} else {
			dstp->DST_STAT = *(struct stat *)rpdata;
		}
		hibyte(dstp->DST_STAT.st_dev) = ~(gp - gdp);

		/*
		 * Stash under current vnode when don't have
		 * a new gift.
		 */

		DST_LINK(chansdp, dstp);
		pnp->pn_path += pnp->pn_pathlen;
		pnp->pn_pathlen = 0;

		/*
		 * Upper level expects back a reference, will
		 * VN_RELE
		 */

		VN_HOLD(dvp);
		*vpp = dvp;
	}
out:
	if (error) {
		pn_setlast(pnp);
	}
	rf_freemsg(bp);
	return error;
}

/*
 * Faking VOP_LINK is complicated for the system call protocol.
 *
 * In the normal case, it involves two or three calls to du_lookup,
 * hence here.
 *
 *	1.  Get a reference to the existing file.
 *	2.  Get a reference to the target directory.
 *	3.  Get a reference to the target name in the target directory.
 *
 * We accomplish the first lookup by the DULINK operations which, in
 * the original system call protocol, is a pure lookup on non-directories.
 *
 * The second and third calls are distinguished by the LOOKUP_DIR flag.
 *
 * If LOOKUP_DIR is set and the remaining pathname is non-NULL, we get
 * a reference to the target directory by faking a DUCHDIR message,
 * which is a pure lookup on directories on the original system call
 * protocol.
 *
 * If LOOKUP_DIR is set and the remaining pathname is NULL, we return
 * ENOENT, which will let the link pass, because vn_link wants the
 * target name to be a new one.  This saves a network message, but
 * relies on the server to fail the link later if the target name exists.
 * Alternatively, we could have used DULINK again, and failed on acquiring
 * a reference, or on getting an error other than ENOENT, but relying on
 * a particular error from another machine seems no less fragile.
 *
 * Assuming that the upper level gets references to both the "to" file and
 * the containing directory for the "from" file, and that these are both
 * RFS vnodes, it will issue the rf_link operation which is handled by
 * rf_link().  In other cases, the upper level will perform VN_RELE's on
 * any acquired vnodes, thus maintaining consistency.
 */
STATIC int
dusys_link(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t			*dvp;		/* current directory */
	char			*comp;		/* current component of name */
	vnode_t			**vpp;		/* return vnode pointer */
	pathname_t		*pnp;		/* pathname structure */
	int			flags;		/* not used */
	vnode_t			*rdirvp;	/* root for this process */
	cred_t			*crp;		/* credentials structure */
{
	mblk_t			*bp = NULL;
	sndd_t			*chansdp = VTOSD(dvp);
	sndd_t			*giftsdp;
	register int		error;

	if (flags & LOOKUP_DIR) {
		return du_link_chdir(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	}
	if ((error = sndd_create(TRUE, &giftsdp)) != 0) {
		return error;
	}

	if ((error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DULINK,
  	  &init_rq_arg, &bp)) == 0 && (error = RF_RESP(bp)->rp_errno) == 0) {
		/*
		 * Either RFDOTDOT, or the system call completed succesfully.
		 * Update pnp and vpp for du_lookup to return.  Get new
		 * reference to result vnode.
		 */
		if (RF_COM(bp)->co_opcode == DULINK) {
			register rf_message_t *msg = RF_MSG(bp);

			if (!(msg->m_stat & RF_GIFT)) {
				gdp_j_accuse("dulink: no file reference",
				  QPTOGP((queue_t *)msg->m_queue));
				error = EPROTO;
				goto out;
			}
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Set up for rfcl_findsndd or del_sndd
			 */
			sndd_set(giftsdp, msg->m_queue, &msg->m_gift);
			if (!(error = rfcl_findsndd(&giftsdp, crp,
			  bp, dvp->v_vfsp))) {
				*vpp = SDTOV(giftsdp);
			} else {
				*vpp = NULLVP;
			}
		} else {
			sndd_free(&giftsdp);
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		}
		rf_freemsg(bp);
		return error;
	}
out:
	/* Error condition from ducl_namemsg or response message */
	rfcl_giftfree(bp, &giftsdp, crp);
	rf_freemsg(bp);
	pn_setlast(pnp);
	return error;
}

/*
 * Send mkdir to server.  In the case of an error, zap pathname structure.
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_mkdir(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* not used */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	int		error = 0;
	sndd_t		*chansdp = VTOSD(dvp);
	mblk_t		*bp = NULL;
	union rq_arg	rqarg;
	struct a {
		char *pathp;
		int mode;
	};

	rqarg = init_rq_arg;
	rqarg.rqmkdir.fmode = ((struct a *)u.u_ap)->mode;
	rqarg.rqmkdir.cmask = u.u_cmask;

	if (!(error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, RFMKDIR,
	  &rqarg, &bp)) && !(error = RF_RESP(bp)->rp_errno)) {
		/*
		 * Either RFDOTDOT or the system call completed
		 * succesfully.
		 */
		if (RF_COM(bp)->co_opcode != RFMKDIR) {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		} else {
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * The upper level expects a reference, will release it.
			 */
			VN_HOLD(&fakevn);
			*vpp = &fakevn;
		}
	}
	rf_freemsg(bp);
	if (error) {
		pn_setlast(pnp);
	}
	return error;
}

/*
 * Open and creat system calls.  Lie to lookuppn and its callers,
 * because system call has completed remotely.
 *
 * If no error and no signal, return.
 * In the case of an error or signal, zap pathname structure and
 * post the signal against the current process
 *
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_copen(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* not used */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	int		error = 0;
	sndd_t		*chansdp = VTOSD(dvp);
	sndd_t		*giftsdp = NULL;
	union rq_arg	rqarg;
	mblk_t		*bp = NULL;
	int		rqop;

	rqarg = init_rq_arg;
	rqarg.rqopen.cmask = u.u_cmask;
	/*
	 * Upper level doesn't tell lookuppn; scrounge in u_block for modes.
	 */
	switch (u.u_syscall) {
	case DUSYSACCT:
	case RFOPEN:
		{
			register struct a {
				char	*fname;
				int	mode;
				int	crtmode;
			} *uap = (struct a *)u.u_ap;

			rqop = RFOPEN;
			rqarg.rqopen.fmode = uap->mode;
			rqarg.rqopen.crtmode = uap->crtmode;
			break;
		}
	case DUCOREDUMP:
		rqop = RFCREATE;
		rqarg.rqopen.fmode = 0666;	/* keep in sync with sig.c */
		rqarg.rqopen.crtmode = 0;
		break;
	default:
		ASSERT(u.u_syscall == RFCREATE);
		{
			register struct a {
				char	*fname;
				int	cmode;
			} *uap = (struct a *)u.u_ap;

			rqop = RFCREATE;
			rqarg.rqopen.fmode = uap->cmode; /* protocol quirk */
			rqarg.rqopen.crtmode = 0;
			break;
		}
	}
	if ((error = sndd_create(TRUE, &giftsdp)) == 0 &&
	  (error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, rqop, &rqarg,
	  &bp)) == 0) {
		error = dusys_copen_resp(bp, pnp, dvp, vpp, crp, rqop, giftsdp,
		  (int)rqarg.rqopen.fmode);
		rf_freemsg(bp);
	} else {
		sndd_free(&giftsdp);
	}
	if (error) {
		pn_setlast(pnp);
	}
	return error;
}

/*
 * Send unlink sys call to server.  In the case of an error, zap pathname
 * structure.
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_unlink(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* not used */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	int		error = 0;
	sndd_t		*chansdp = VTOSD(dvp);
	mblk_t		*bp = NULL;

	if (!(error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUUNLINK,
	  &init_rq_arg, &bp)) && !(error = RF_RESP(bp)->rp_errno)) {
		/*
		 * Either RFDOTDOT or the system call completed
		 * succesfully.
		 */
		if (RF_COM(bp)->co_opcode != DUUNLINK) {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		} else {
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Upper level expects a reference, will release it.
			 */
			VN_HOLD(&fakevn);
			*vpp = &fakevn;
		}
	}
	rf_freemsg(bp);
	if (error) {
		pn_setlast(pnp);
	}
	return error;
}

/*
 * The usual policy is not to try to fake new system calls for old servers.
 * rename() is an exception because so many things use it.
 */

/*
 * Faking VOP_RENAME is complicated for the system call protocol.
 *
 * In the normal case, it involves four calls to du_lookup, hence to
 * dusys_rename.
 *
 *	1.  Get a reference to the existing file's parent directory.
 *	2.  Get a reference to the existing file.
 *	3.  Get a reference to the target directory.
 *	4.  Get a reference to the target name in the target directory.
 *
 * We accomplish 1 and 3 by faking a DUCHDIR, which is a pure lookup
 * on directories, 2 and 4 by faking a DULINK, which is a pure lookup
 * on nondirectories.  We can tell which to do by looking at the remaining
 * pathname; if non-NULL, the parent is wanted.  If NULL, the file is
 * wanted.
 *
 * After all references are acquired, a call to rf_rename will switch
 * out to du_rename, which fakes the rename operation by composing a
 * dusys_link, a dusys_unlink, an rf_link, and a dusys_unlink.
 */
/* ARGSUSED */
STATIC int
dusys_rename(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t			*dvp;		/* current directory */
	char			*comp;		/* current component of name */
	vnode_t			**vpp;		/* return vnode pointer */
	register pathname_t	*pnp;		/* pathname structure */
	register int		flags;		/* not used */
	vnode_t			*rdirvp;	/* root for this process */
	cred_t			*crp;		/* credentials structure */
{
	register int		error;

	if (*pnp->pn_path == '\0') {

		/*
		 * This is a lookup on a non-directory file.  Turning
		 * off LOOKUP_DIR will coerce dusys_link to get a reference
		 * to the file.
		 */

		if ((error = dusys_link(dvp, comp, vpp, pnp,
		  flags & ~LOOKUP_DIR, rdirvp, crp)) == ENOENT) {

			/*
			 * Zap the pathname so a lookup that is really
			 * interested only in the directory will pass.
			 */

			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
		} else if (!error && (*vpp)->v_type == VDIR) {

			/*
			 * In general, to link directories is unsafe, because
			 * ".." needs to be reconstructed, but we are not
			 * prepared to make that effort.  (Note that this
			 * would succeed on the server only if the current
			 * iuid maps into root, anyway.)
			 */

			error = EISDIR;
			VN_RELE(*vpp);
		}
	} else {

		/* This is a lookup on a directory. */

		ASSERT(flags & LOOKUP_DIR);
		error = du_link_chdir(dvp, comp, vpp, pnp, flags, rdirvp, crp);
	}
	return error;
}

/*
 * Send rmdir to server.  In the case of an error, zap pathname structure.
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_rmdir(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* not used */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	int		error = 0;
	mblk_t		*bp = NULL;

	if (!(error = ducl_namemsg(VTOSD(dvp), rdirvp, pnp, comp, crp, RFRMDIR,
	  &init_rq_arg, &bp)) && !(error = RF_RESP(bp)->rp_errno)) {
		/*
		 * Either RFDOTDOT or the system call completed
		 * succesfully.
		 */
		if (RF_COM(bp)->co_opcode != RFRMDIR) {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		} else {
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Upper level expects a reference, will release it.
			 */
			VN_HOLD(&fakevn);
			*vpp = &fakevn;
		}
	}
	rf_freemsg(bp);
	if (error) {
		pn_setlast(pnp);
	}
	return error;
}

/*
 * We handle the chown system call by sending it to the server,
 * and trivially returning 0 for success when the du_setattr vnode
 * op is subsequently invoked.  In the case of a chown on the current
 * directory with a null pathname specified, the remote root directory,
 * or the root of the remote resource, there will be no final du_lookup
 * preceding the du_setattr.  For those cases, dudirchown calls dusys_chown
 * with the pathname ".", in order to perform the syscall.
 *
 * Returns 0 for success, nonzero errno for failure.  In the case of an
 * error, zap pathname structure.
 */
/* ARGSUSED */
STATIC int
dusys_chown(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 	*dvp;		/* current directory */
	char 		*comp;		/* current component of name */
	vnode_t 	**vpp;		/* return vnode pointer */
	pathname_t 	*pnp;		/* pathname structure */
	int 		flags;		/* not used */
	vnode_t 	*rdirvp;	/* root for this process */
	cred_t 		*crp;		/* credentials structure */
{
	int		error = 0;
	sndd_t		*chansdp = VTOSD(dvp);
	mblk_t		*bp = NULL;
	union rq_arg	rqarg;
	struct a {
		char *fname;
		uid_t uid;
		uid_t gid;
	} *uap = (struct a *)u.u_ap;

	rqarg = init_rq_arg;
	rqarg.rqchown.uid = uap->uid;
	rqarg.rqchown.gid = uap->gid;
	if (!(error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUCHOWN,
	  &rqarg, &bp)) && !(error = RF_RESP(bp)->rp_errno)) {
		/*
		 * Either RFDOTDOT or the system call completed succesfully.
		 */
		if (RF_COM(bp)->co_opcode != DUCHOWN) {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		} else {
			register dustash_t *dstp = dst_alloc();

			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Upper level expects back a reference, will release it
			 */
			VN_HOLD(dvp);
			*vpp = dvp;
			/*
			 * Link a NULL stash to our send descriptor so that when
			 * the expected du_setattr is issued, we can know that
			 * the chown has already completed successfully.
			 */
			dstp->dst_pid = u.u_procp->p_pid;
			DST_LINK(chansdp, dstp);
		}
	}
	rf_freemsg(bp);
	if (error) {
		pn_setlast(pnp);
	}
	return error;
}

/*
 * We handle the chmod system call by sending it to the server,
 * and trivially returning 0 for success when the du_setattr vnode
 * op is subsequently invoked.  In the case of a chmod on the current
 * directory with a null pathname specified, the remote root directory,
 * or the root of the remote resource, there will be no final du_lookup
 * preceding the du_setattr.  For those cases, dudirchmod calls dusys_chmod
 * with the pathname ".", in order to perform the syscall.
 *
 * Returns 0 for success, nonzero errno for failure.  In the case of an
 * error zap pathname structure.
 */
/* ARGSUSED */
STATIC int
dusys_chmod(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 		*dvp;		/* current directory */
	char 			*comp;		/* current component of name */
	vnode_t 		**vpp;		/* return vnode pointer */
	pathname_t 		*pnp;		/* pathname structure */
	int 			flags;		/* not used */
	vnode_t 		*rdirvp;	/* root for this process */
	cred_t 			*crp;		/* credentials structure */
{
	int			error = 0;
	sndd_t			*chansdp = VTOSD(dvp);
	union rq_arg		rqarg;
	mblk_t			*bp = NULL;
	struct a {
		char *fname;
		int fmode;
	};

	rqarg = init_rq_arg;
	rqarg.rqmode_op.fmode =  (long)((struct a *)u.u_ap)->fmode;
	if (!(error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUCHMOD,
	  &rqarg, &bp)) && !(error = RF_RESP(bp)->rp_errno)) {
		/*
		 * Either RFDOTDOT or the system call completed
		 * succesfully.
		 */
		if (RF_COM(bp)->co_opcode != DUCHMOD) {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		} else {
			dustash_t *dstp = dst_alloc();
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Upper level expects back a reference, will release it
			 */
			VN_HOLD(dvp);
			*vpp = dvp;
			/*
			 * Allocating a NULL stash gives du_setattr a means
			 * of knowing whether the chmod has already completed.
			 */
			dstp->dst_pid = u.u_procp->p_pid;
			DST_LINK(chansdp, dstp);
		}
	}
	rf_freemsg(bp);
	if (error) {
		pn_setlast(pnp);
	}
	return error;
}

typedef struct utimbuf utimbuf_t;

/*
 * Send utime to server with utimbuf pointer from u_area.
 * In the case of an error, zap pathname structure.
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_utime(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t 		*dvp;		/* current directory */
	char 			*comp;		/* current component of name */
	vnode_t 		**vpp;		/* return vnode pointer */
	pathname_t 		*pnp;		/* pathname structure */
	int 			flags;		/* not used */
	vnode_t 		*rdirvp;	/* root for this process */
	cred_t 			*crp;		/* credentials structure */
{
	uio_t			uio;
	iovec_t			iov;
	register int		error;
	int			nacked;
	register int		ntries;
	register int		complen = strlen(comp);
	register size_t		datasz = complen + pnp->pn_pathlen + 1;
	register size_t		rqsize = RFV1_MINREQ + datasz;
	mblk_t			*bp = NULL;
	rcvd_t			*rdp = NULL;
	sndd_t			*replysdp = NULL;
	register sndd_t		*chansdp = VTOSD(dvp);
	struct a {
		char *pathp;
		utimbuf_t *times;
	};


	/* create an sndd for any copyin. */

	if ((error = sndd_create(TRUE, &replysdp)) != 0) {
		goto out;
	}
	/*
	 * create an rd on which to receive the response
	 */
	if ((error = rcvd_create(TRUE, RDSPECIFIC, &rdp)) != 0) {
		goto out;
	}
	rdp->rd_sdp = chansdp;

	/* set up uio, iov to direct the data movement */
	uio.uio_offset = 0;
	uio.uio_segflg = UIO_USERSPACE;
	uio.uio_resid = sizeof(utimbuf_t);
	uio.uio_iovcnt = 1;
	uio.uio_iov = &iov;
	iov.iov_base = (caddr_t)((struct a *)u.u_ap)->times;
	iov.iov_len = sizeof(utimbuf_t);
	for (ntries = nacked = 1; ntries < RFCL_MAXTRIES && nacked; ntries++) {
		register rf_request_t	*reqp;
		register char		*datap;

		if ((error = rf_allocmsg(RFV1_MINREQ, datasz, BPRI_LO, TRUE,
		  NULLCADDR, NULLFRP, &bp)) != 0) {
			break;
		}
		rfcl_reqsetup(bp, chansdp, crp, DUUTIME, R_ULIMIT);
		reqp = RF_REQ(bp);
		reqp->rq_utime.buf = (long)iov.iov_base;
		if (rdirvp && ISRFSVP(rdirvp) &&
		  rdirvp->v_vfsp == dvp->v_vfsp) {
			reqp->rq_rrdir_id = VTOSD(rdirvp)->sd_gift.gift_id;
		} else {
			reqp->rq_rrdir_id = 0;
		}
		datap = rf_msgdata(bp, RFV1_MINREQ);
		(void)strcpy(datap, comp);
		(void)strcpy(datap + complen, pnp->pn_path);
		error = rfcl_xac(&bp, rqsize, rdp, RFS1DOT0, FALSE, &nacked);
	}
	if (!error && (error = RF_RESP(bp)->rp_errno) == 0 &&
	  (error = dusys_utime_pass(pnp, &bp, rdp, replysdp, &uio, dvp, vpp))
	  == 0) {
		register struct dustash *dstp = dst_alloc();

		dstp->dst_pid = u.u_procp->p_pid;
		DST_LINK(chansdp, dstp);
	}
out:
	rf_freemsg(bp);
	rcvd_free(&rdp);
	sndd_free(&replysdp);
	return error;
}

/*
 * Send a DUSTATFS message to the server.
 * If no error, stash the statfs structure from the response on the send
 * descriptor of the current vnode.  In the normal case, the upper level
 * will then issue a rf_statvfs switch op.  At that point, the stashed result
 * will be transcribed into the referenced statvfs structure.
 *
 * In the case of an error, zap pathname structure.
 * Returns 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
STATIC int
dusys_statfs(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t			*dvp;		/* current directory */
	char 			*comp;		/* current component of name */
	vnode_t 		**vpp;		/* return vnode pointer */
	pathname_t 		*pnp;		/* pathname structure */
	int 			flags;		/* not used */
	vnode_t 		*rdirvp;	/* root for this process */
	cred_t 			*crp;		/* credentials structure */
{
	int			error = 0;
	sndd_t			*chansdp = VTOSD(dvp);
	register gdp_t		*gp = QPTOGP(chansdp->sd_queue);
	mblk_t			*bp = NULL;
	union rq_arg		rqarg;
	size_t			datasz;
	register struct a {
		char *pathp;
		void *sbp;			/* ptr to statfs or statvfs */
	}			*uargp = (struct a *)u.u_ap;

	datasz = gp->hetero == NO_CONV ? sizeof(struct statfs) :
	  sizeof(struct statfs) + STATFS_XP;
	rqarg = init_rq_arg;
	rqarg.rqstatfs_op.buf = (long)&uargp->sbp;
	rqarg.rqstatfs_op.len = sizeof(struct statfs);
	rqarg.rqstatfs_op.fstyp = 0;
	if ((error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUSTATFS,
	  &rqarg, &bp)) == 0 &&
	 (error = RF_RESP(bp)->rp_errno) == 0) {
		/*
		 * Either RFDOTDOT or the system call completed succesfully.
		 */
		if (RF_COM(bp)->co_opcode != DUSTATFS) {
			error = ducl_resetpath(bp, pnp, dvp, vpp);
		} else if (RF_PULLUP(bp, RFV1_MINRESP, datasz)) {
			gdp_j_accuse("dusys_statfs bad data", gp);
			error = EPROTO;
		} else {
			register dustash_t	*dstp = dst_alloc();
			register caddr_t	rpdata =
						  rf_msgdata(bp, RFV1_MINRESP);

			if (gp->hetero != NO_CONV) {
				if (!rf_fcanon(STATFS_FMT, rpdata, rpdata +
				  datasz, (caddr_t)&dstp->DST_STATFS)) {
					gdp_j_accuse("dusys_statfs bad data",
					  gp);
					error = EPROTO;
					goto out;
				}
			} else {
				dstp->DST_STATFS = *(struct statfs *)rpdata;
			}
			dstp->dst_pid = u.u_procp->p_pid;
			/*
			 * We stash under current gift when we don't
			 * have a new one
			 */
			DST_LINK(chansdp, dstp);
			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			/*
			 * Upper level expects back a reference, will
			 * do a VN_RELE
			 */
			VN_HOLD(dvp);
			*vpp = dvp;
		}
	}
out:
	if (error) {
		*vpp = NULLVP;
		pn_setlast(pnp);
	}
	rf_freemsg(bp);
	return error;
}

/*
 * The fchdir system call is defined to be number 120 in os/sysent.c.
 * We #define it here to make it easier for future readers
 * of the code. This value may have to be updated if the sysent table
 * changes.
 */
#define FCHDIR 120

/*
 * VOP_ACCESS for files residing on pre-SVR4 RFS servers.
 * Return 0 for success, nonzero errno for failure.
 */
/* ARGSUSED */
int
du_access(vp, mode, flags, crp)
	vnode_t	*vp;
	int	mode;
	int	flags;
	cred_t	*crp;
{
	int	error = 0;	/* Assume we've already faked the syscall */
	int	scall = u.u_syscall;

	if (flags & ATTR_EXEC || scall == FCHDIR) {
		return ENOSYS;
	}
	if ((vp->v_flag & VROOT) &&
	  (scall == DUSACCESS || scall == DUCHDIR || scall == DUCHROOT)) {
		/*
		 * Haven't been remote yet.
		 */
		if (mode & VWRITE && vp->v_vfsp->vfs_flag & VFS_RDONLY) {
			error = EROFS;
		} else {
			/*
			 * Fake an access system call with the current
			 * directory vnode and the pathname "."
			 */
			pathname_t	pn;
			vnode_t		*sinkvp;	/* for fake lookup */

			if (!(error = pn_get("", UIO_SYSSPACE, &pn))) {
				if (!(error = du_caccess(vp, ".", &sinkvp, &pn,
				  NULLVP, crp, mode))) {
					VN_RELE(sinkvp);
				}
				pn_free(&pn);
			}
		}
	}
	return error;
}

/*
 * Retrieve stashed vnode, except for mknod system call, in which case there
 * is no new reference.
 * Return 0 for success, nonzero errno for failure.
 * NOTE:
 * We are not filling in the vattr structure, because
 * nobody above uses it.  This is consistent with
 * our implementation policy.
 */
/* ARGSUSED */
int
du_create(dvp, nm, vap, ex, mode, vpp, crp)
	vnode_t *dvp;
	char *nm;
	vattr_t *vap;
	vcexcl_t ex;
	int mode;
	vnode_t **vpp;
	cred_t *crp;
{
	register struct dustash *dstp;

	/* We don't have to worry about not having done a
	 * previous lookup here.  If we're trying to create
	 * a remote mount point, the parent directory is
	 * a local directory, and we're in that file system
	 * type's vn_create.
	 */
	if (u.u_syscall == DUMKNOD || u.u_syscall == DUXMKNOD) {
		VN_HOLD(dvp);
		*vpp = dvp;	/* both are released by upper level */
		return 0;
	}
	/* We know that our stuff is stashed under dvp
	 */
	dstp = dst_unlink(VTOSD(dvp), u.u_procp->p_pid);
	ASSERT(dstp);
	*vpp = dstp->DST_VP;
	dst_free(dstp);
	return 0;
}

int
du_fcntl(op, vp, cmd, arg, flag, offset, crp)
	int			op;
	vnode_t			*vp;
	int			cmd;
	caddr_t			arg;
	int			flag;
	off_t			offset;
	cred_t			*crp;
{
	register int		ntries;
	register int		error = 0;
	int			nacked;
	mblk_t			*bp = NULL;
	rcvd_t			*rdp;
	register sndd_t		*chansdp = VTOSD(vp);
	register gdp_t		*gp = QPTOGP(chansdp->sd_queue);
	register size_t		datasz = 0;
	register size_t		rqsz;
	flock_t			*flp = (flock_t *)arg;
	int			canon = gp->hetero != NO_CONV;
	o_flock_t		oflock;
	o_flock_t		*oflp;
	int			sendflock = 0;

	switch (cmd) {
	case	F_O_GETLK:
	case	F_SETLK:
	case	F_SETLKW:
	case	F_FREESP:
		sendflock = 1;
		datasz = sizeof(o_flock_t);
		if (canon) {
			datasz += OFLOCK_XP;
		}
	}
	rqsz = RFV1_MINREQ + datasz;

	/* create an rd on which to receive the response */
	if ((error = rcvd_create(TRUE, RDSPECIFIC, &rdp)) != 0) {
		return error;
	}
	rdp->rd_sdp = chansdp;

	if (cmd == F_GETLK) {
		cmd = F_O_GETLK;
	}
	for (ntries = nacked = 1; ntries < RFCL_MAXTRIES && nacked; ntries++) {
		register rf_request_t	*reqp;
		register caddr_t	rqdatap;

		if ((error = rf_allocmsg(RFV1_MINREQ, datasz, BPRI_LO, TRUE,
		  NULLCADDR, NULLFRP, &bp)) != 0) {
			break;
		}
		rfcl_reqsetup(bp, chansdp, crp, RFFCNTL, R_ULIMIT);
		reqp = RF_REQ(bp);
		reqp->rq_fcntl.cmd = (long)cmd;
		reqp->rq_fcntl.fcntl = (long)arg;
		reqp->rq_fcntl.offset = (long)offset;
		reqp->rq_fcntl.fflag = (long)flag;
		reqp->rq_fcntl.fflag &= ~DUFRPREWRITE;
		/*
		 * Send record locking data with request.
		 * F_FREESP uses the same structure, so let it go along
		 * for the ride.
		 */
		if (sendflock) {
			reqp->rq_fcntl.fflag |= DUFRPREWRITE;
			rqdatap = rf_msgdata(bp, RFV1_MINREQ);
			if (canon) {
				oflp = &oflock;
			} else {
				oflp = (o_flock_t *)rqdatap;
			}
			oflp->l_type = flp->l_type;
			oflp->l_whence = flp->l_whence;
			oflp->l_start = flp->l_start;
			oflp->l_len = flp->l_len;

			/* l_sysid and l_pid are output only, used with GETLK */

			oflp->l_sysid = 0;
			oflp->l_pid = 0;

			if (canon) {
				reqp->rq_xfer.prewrite = rf_tcanon(O_FLOCK_FMT,
				  (caddr_t)oflp, rqdatap);
			} else {
				reqp->rq_xfer.prewrite = sizeof(o_flock_t);
			}
		}
		error = rfcl_xac(&bp, rqsz, rdp, RFS1DOT0, FALSE, &nacked);
	}
	if (!error) {
		error = du_fcntl_resp(op, cmd, rdp, bp, arg);
	}
	rcvd_free(&rdp);
	return error;
}

/*
 * Update denoted structure with attribute information for remote file
 * denoted by node pointer.  This routine disregards the va_mask field
 * and always returns all of the information from a stat structure.
 * Returns 0 for success, nonzero errno for failure.
 * A signal returned from server is assumed to be evidence of failure,
 * and is posted against current process, irrespective of any other
 * explicit errors.
 */
/* ARGSUSED */
int
du_getattr(vp, vap, flags, crp)
	vnode_t		*vp;
	vattr_t		*vap;
	int		flags;
	cred_t		*crp;
{
	register sndd_t	*sdp = VTOSD(vp);
	register gdp_t	*gp = QPTOGP(sdp->sd_queue);
	dustash_t	*dstp;
	register int	error;
	mblk_t		*bp = NULL;
	size_t		datasz;
	struct stat	sbuf;	/* compatability bone thrown to server */
	union rq_arg	rqarg;
	struct exec_ids	exec_ids;

	rqarg = init_rq_arg;
	if (flags & ATTR_EXEC) {
		dstp = dst_unlink(sdp, u.u_procp->p_pid);
		ASSERT(dstp != NULL);
		/* there is a possibility dst_unlink returns NULL */
		if (dstp)	
			exec_ids = dstp->DST_EXEC;
		dst_free(dstp);
	} else if (u.u_syscall == RFOPEN || u.u_syscall == RFCREATE) {
		/*
		 * Concealing the fact that the system call is already done,
		 * we tell the upper level, which is checking for mandatory
		 * locks, the there are none in force on the file.
		 */
		vap->va_mode = 0;
		return 0;
	} else if ((u.u_syscall == DUSTAT || u.u_syscall == DULSTAT ||
	  u.u_syscall == DUXSTAT || u.u_syscall == DULXSTAT) &&
	  (dstp = dst_unlink(sdp, u.u_procp->p_pid)) != NULL) {
		du_stat_to_vattr(&dstp->DST_STAT, vap);
		dst_free(dstp);
		return 0;
	}

	datasz = gp->hetero == NO_CONV ? sizeof(struct stat) :
	  sizeof(struct stat) + STAT_XP;
	rqarg.rqstat_op.buf = (long)&sbuf;	/* woof */
	if ((error = rfcl_op(sdp, crp, DUFSTAT, &rqarg, &bp, TRUE)) == 0 &&
	  (error = RF_RESP(bp)->rp_errno) == 0) {
		caddr_t		rpdata;

		if (RF_PULLUP(bp, RFV1_MINRESP, datasz)) {
			gdp_j_accuse("du_getattr bad data", gp);
			error = EPROTO;
			goto out;
		}

		rpdata = rf_msgdata(bp, RFV1_MINRESP);

		if (gp->hetero != NO_CONV &&
		  !rf_fcanon(STAT_FMT, rpdata, rpdata + datasz, rpdata)) {
			gdp_j_accuse("du_getattr bad data", gp);
			error = EPROTO;
			goto out;
		}

		/* By convention, major dev for remote file is set to ones
		 * complement of gdp index.
		 * NOTE:
		 * This convention violates the invariant that identical dev
		 * and node numbers imply file identity.  Because there is
		 * a single virtual circuit between client and server, multiple
		 * server file systems can have the same dev on the client side.
		 */

		hibyte(((struct stat *)rpdata)->st_dev) = ~(gp - gdp);
		du_stat_to_vattr(((struct stat *)rpdata), vap);
		if (flags & ATTR_EXEC) {
			gdp_t		*gp = QPTOGP(sdp->sd_queue);

			/* shut out unauthorized setuid programs */

			vap->va_uid = gluid(gp, exec_ids.ex_uid);
			vap->va_gid = glgid(gp, exec_ids.ex_gid);
		}
	}
out:
	rf_freemsg(bp);
	return error;
}

/* Trivially returns 0 to denote successful completion
 * of dusys_mkdir called from du_lookup.
 */
/* ARGSUSED */
int
du_mkdir(vp, nm, vap, vpp, crp)
	vnode_t *vp;
	char *nm;
	vattr_t *vap;
	vnode_t **vpp;
	cred_t *crp;
{
	VN_HOLD(vp);
	*vpp = vp;			/* vn_mkdir expects vpp to be set */
	return 0;			/* for success */
}

/*
 * If call already went remote, returns 0, unless upper level is
 * misbehaving, because op has already succeeded on server.
 * Otherwise, *vpp is the root of an advertised resource, so op hasn't
 */
/* ARGSUSED */
int
du_open(vpp, f, crp)
	vnode_t		**vpp;
	int		f;
	cred_t		*crp;
{
	int		error;
	pathname_t	pn;
	vnode_t		*resultvp;

	f &= ~FCREAT;			/* paranoia */
	if ((*vpp)->v_flag & VROOT) {
		if ((error = pn_get("", UIO_SYSSPACE, &pn)) != 0) {
			return error;
		}
		if ((error = dusys_copen(*vpp, ".", &resultvp, &pn, 0, NULLVP,
		  crp)) == 0) {
			dustash_t *dstp;

			/*
			 * dusys_copen will stash in some circumstances, but
			 * we never need it here.
			 */
			if ((dstp = dst_unlink(VTOSD(resultvp),
			  u.u_procp->p_pid)) != NULL) {
				dst_free(dstp);
			}
			VN_RELE(*vpp);
		}
		pn_free(&pn);
	}
	return 0;
}

/* ARGSUSED */
int
du_remove(dvp, nmp, crp)
	vnode_t *dvp;		/* parent directory */
	char *nmp;		/* name of entry to remove */
	cred_t *crp;
{
	return 0;			/* the unlink was already done */
}

int
du_rename(fdvp, fnm, tdvp, tnm, crp)
	vnode_t		*fdvp;
	char		*fnm;
	vnode_t		*tdvp;
	char		*tnm;
	cred_t		*crp;
{
	/*
	 * We have to reaquire a reference to the existing file, because
	 * it doesn't come through the interface.
	 */

	pathname_t		pathname;
	vnode_t			*fvp;
	vnode_t			*tvp;
	register vnode_t	*rdir = u.u_rdir ? u.u_rdir : rootdir; /* ugh */
	int			error;

	int		rf_link();

	if (error = pn_get(fnm, UIO_SYSSPACE, &pathname)) {
		return error;
	}

	ASSERT(pn_peekchar(&pathname) != '/');

	pathname.pn_path += pathname.pn_pathlen;
	pathname.pn_pathlen = 0;
	if (strlen(fnm) + 1 > DU_DATASIZE) {
		error = ENOMEM;
		goto freepn;
	}

	if ((error =
	  dusys_link(fdvp, fnm, &fvp, &pathname, 0, rdir, crp)) != 0) {
		goto freepn;
	}
	VN_RELE(fvp);		/* get rid of redundant reference */

	if (fvp->v_type == VDIR) {
		error = ENOSYS;
		goto freepn;
	}

	if (strlen(tnm) + 1 > DU_DATASIZE) {
		error = ENOMEM;
		goto freepn;
	}

	/* Unlink the target file. */

	if ((error =
	  dusys_unlink(tdvp, tnm, &tvp, &pathname, 0, rdir, crp)) != 0 &&
	  error != ENOENT) {
		goto freepn;
	} else if (!error) {
		VN_RELE(tvp);	/* dusys_unlink fakes a lookup */
	}

	/* Link the "from" file to the target. */

	if (error = rf_link(tdvp, fvp, tnm, crp)) {
		goto freepn;
	}

	/* Unlink the "from" file. */

	pathname.pn_path += pathname.pn_pathlen;
	pathname.pn_pathlen = 0;
	if (strlen(fnm) + 1 > DU_DATASIZE) {
		error = ENOMEM;
		goto freepn;
	}

	if ((error =
	  dusys_unlink(fdvp, fnm, &fvp, &pathname, 0, rdir, crp)) == 0) {
		VN_RELE(fvp);	/* dusys_unlink fakes a lookup */
	}

freepn:
	pn_free(&pathname);
	return error;
}

/* ARGSUSED */
int
du_rmdir(dvp, nm, cdvp, crp)
	vnode_t	*dvp;
	char	*nm;
	vnode_t	*cdvp;
	cred_t	*crp;
{
	return 0;
}

/*
 * Assuming code self-consistency, if there is a stash, then the op has already
 * completed.  Free it and return.  Otherwise, either the upper level ignored
 * an earlier error, (we assume not), or there wasn't a preceding du_lookup.
 * This could be the case if vp is the mount point, or is the user's current
 * or root directory.  Depending on va_mask, go remote now with a "." pathname.
 */
/* ARGSUSED */
int
du_setattr(vp, vap, crp, flags)
	vnode_t		*vp;
	vattr_t		*vap;
	cred_t		*crp;
	int		flags;
{
	dustash_t	*dstp;
	register long	va_mask = vap->va_mask;
	register int	error;
	pathname_t	pn;
	vnode_t		*resultvp = NULLVP;

	if ((dstp = dst_unlink(VTOSD(vp), u.u_procp->p_pid)) != NULL) {
		dst_free(dstp);
		return 0;
	}
	if ((error = pn_get("", UIO_SYSSPACE, &pn)) != 0) {
		return error;
	}
	/* The following depend on the settings in vncalls.c and vnode.c */
	switch ((int)va_mask) {
	case AT_MODE:			/* chmod */
		if (u.u_syscall == DUCHMOD) {
			ASSERT(vp->v_type == VDIR);
			error = dusys_chmod(vp, ".", &resultvp, &pn, 0,
			  NULLVP, crp);
		} else {
			error = ENOSYS;
		}
		break;
	case AT_UID | AT_GID:		/* chown */
		if (u.u_syscall == DUCHOWN) {
			ASSERT(vp->v_type == VDIR);
			error = dusys_chown(vp, ".", &resultvp, &pn, 0,
			  NULLVP, crp);
		} else {
			error = ENOSYS;
		}
		break;
	case AT_ATIME | AT_MTIME:	/* utime */
		ASSERT(vp->v_type == VDIR);
		error = dusys_utime(vp, ".", &resultvp, &pn, 0, NULLVP, crp);
		break;
	case AT_SIZE:
	case AT_TYPE | AT_MODE:
		/* Concealing the fact that the system call is already done,
		 * we tell the upper level, which we know truncated the file,
		 * that we are updating its attributes.
		 */
		error = 0;
		break;
	default:		/* Other masks are unknown to old servers */
		error = ENOSYS;
		break;
	}
	if ((dstp = dst_unlink(VTOSD(vp), u.u_procp->p_pid)) != NULL) {
		dst_free(dstp);
	}
	if (resultvp) {
		VN_RELE(resultvp);
	}
	pn_free(&pn);
	return error;
}

/*
 * For 3.x servers, send the DUFSTATFS opcode instead of RFSTATVFS.
 * Returned statfs is converted to statvfs as expected by the interface.
 */
int
du_fstatfs(vfsp, stvfsp)
	vfs_t			*vfsp;
	statvfs_t		*stvfsp;
{
	vnode_t			*vp = VFTORF(vfsp)->rfvfs_rootvp;
	dustash_t		*dstp;
	register int		error;
	mblk_t			*bp = NULL;
	union rq_arg		rqarg;
	sndd_t			*sdp = VTOSD(vp);
	register gdp_t		*gp = QPTOGP(sdp->sd_queue);
	size_t			datasz;

	rqarg = init_rq_arg;
	/*
	 * Stashed struct statfs might be associated with the send descriptor
	 * by prior lookup going through du_statfs().
	 */
	if ((dstp = dst_unlink(sdp, u.u_procp->p_pid)) != NULL) {
		error = du_fs_to_vfs(&dstp->DST_STATFS, stvfsp, vfsp);
		dst_free(dstp);		/* done with this stash */
		if (error) {
			gdp_j_accuse("du_fstatfs bad server data", gp);
			error = EPROTO;
		}
		return error;
	}

	VN_HOLD(vp);	/* so it won't disappear (impossible?) */
	datasz = gp->hetero == NO_CONV ? sizeof(struct statfs) :
	  sizeof(struct statfs) + STATFS_XP;
	rqarg.rqstatfs_op.buf = (long)stvfsp;	/* server wants an address */
	rqarg.rqstatfs_op.len = sizeof(struct statfs);
	rqarg.rqstatfs_op.fstyp = 0;
	if ((error = rfcl_op(sdp, u.u_cred, DUFSTATFS, &rqarg, &bp, TRUE))
	  == 0 && (error = RF_RESP(bp)->rp_errno) == 0) {
		if (RF_PULLUP(bp, RFV1_MINRESP, datasz)) {
			gdp_j_accuse("du_fstatfs bad server data", gp);
			error = EPROTO;
		} else {
			caddr_t	rpdata;

			rpdata = rf_msgdata(bp, RFV1_MINRESP);

			if (gp->hetero != NO_CONV  && !rf_fcanon(STATFS_FMT,
			  rpdata, rpdata + datasz, rpdata)) {
				gdp_j_accuse("du_fstatfs bad server data", gp);
				error = EPROTO;
			}
			if (!error && 
			  (error = du_fs_to_vfs((struct statfs *)rpdata, stvfsp,
			  vfsp))) {
				gdp_j_accuse("du_fstatfs bad server data", gp);
			}
		}
		rf_freemsg(bp);
	}
	VN_RELE(vp);
	return error;
}

/*
 * Common routine for dusys_access and du_access.
 */
STATIC int
du_caccess(dvp, comp, vpp, pnp, rdirvp, crp, fmode)
	vnode_t 		*dvp;		/* current directory */
	char 			*comp;		/* current component of name */
	vnode_t 		**vpp;		/* return vnode pointer */
	pathname_t 		*pnp;		/* pathname structure */
	vnode_t 		*rdirvp;	/* root for this process */
	cred_t 			*crp;		/* credentials structure */
	int			fmode;		/* requested mode */
{
	int			error = 0;
	sndd_t			*chansdp = VTOSD(dvp);
	mblk_t			*bp = NULL;
	union rq_arg		rqarg;

	rqarg = init_rq_arg;
	rqarg.rqmode_op.fmode = fmode;
	if (!(error = ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, DUSACCESS,
	  &rqarg, &bp))) {
		ASSERT(bp);
		if (!(error = RF_RESP(bp)->rp_errno)) {
			if (RF_COM(bp)->co_opcode == DUSACCESS) {
				pnp->pn_path += pnp->pn_pathlen;
				pnp->pn_pathlen = 0;
				/*
				 * Upper level expects back a reference,
				 * will do a VN_RELE
				 */
				VN_HOLD(dvp);
				*vpp = dvp;
			} else {
				error = ducl_resetpath(bp, pnp, dvp, vpp);
			}
		} else {
			pn_setlast(pnp);
		}
	} else {
		pn_setlast(pnp);
	}
	rf_freemsg(bp);
	return error;
}

/*
 * Handle a successful response from an old server.  Always frees bp and
 * returns any error encountered, including rp_errno from the bracketing
 * response.
 * 3.0 servers don't piggyback the flock data on the final response.  They
 * require an extra rf_rcvmsg when the first response is a RFCOPYOUT.
 */
STATIC int
du_fcntl_resp(op, cmd, rdp, bp, arg)
	int			op;
	int			cmd;
	register rcvd_t		*rdp;
	mblk_t			*bp;
	register caddr_t	arg;
{
	struct flock		*flp = (struct flock *)arg;
	rf_response_t		*rp;
	sndd_t			*chansdp = rdp->rd_sdp;
	register int		error = 0;
	int			copyerr = 0;

	if (!(RF_MSG(bp)->m_stat & RF_VER1) &&
	  RF_COM(bp)->co_opcode == RFCOPYOUT) {
		/* 3.0 servers don't piggyback lock on final response */
		copyerr = du_o_flock_to_flock(bp, flp);
		rf_freemsg(bp);
		if ((error = rf_rcvmsg(rdp, &bp)) != 0) {
			return error;
		}
	}
	rp = RF_RESP(bp);
	error = rp->rp_errno;
	if (!rf_sigisempty(rp, RFS1DOT0)) {
		rf_postrpsigs(rp, RFS1DOT0, u.u_procp);
	}
	if (!error && RF_MSG(bp)->m_stat & RF_VER1 &&
	  op == RFFRLOCK && cmd == F_O_GETLK) {

		/*
		 * Data response from 3.1 or 3.2 server.
		 * NOTE: in general, rp will be a
		 * dangling reference after here.
		 */
		error = du_o_flock_to_flock(bp, flp);
	} else if (!error && op != RFFRLOCK && SDTOV(chansdp)->v_type ==
	  VREG && rp->rp_rval == F_FREESP) {
		/*
		 * Update sd_size since the RFSV1DOT0 server gives
		 * it to us in this case.
		 */
		if (rp->rp_fcntl.isize < chansdp->sd_size) {
			/*
			 * We truncated the file.  Toss pages
			 * not strictly contained in the new size.
			 */
			rfc_pageabort(chansdp, (off_t)rp->rp_fcntl.isize,
			  (off_t)0);
		}
		chansdp->sd_size = rp->rp_fcntl.isize;
	}
	rf_freemsg(bp);
	return error ? error : copyerr;
}

/*
 * The first error-free response could be RFDOTDOT, DUUTIME, or
 * could be a RFCOPYIN response, in which case send the utimbuf now.  Anything
 * else here is unexpected.  If we get a RFCOPYIN, then expect a single DUUTIME
 * response to complete the interaction.  Update pnp and vpp for du_lookup to
 * return.
 */
STATIC int
dusys_utime_pass(pnp, bpp, rdp, replysdp, uiop, dvp, vpp)
	register pathname_t	*pnp;
	mblk_t			**bpp;
	rcvd_t			*rdp;
	sndd_t			*replysdp;
	uio_t			*uiop;
	vnode_t			*dvp;
	vnode_t			**vpp;
{
	register rf_response_t	*rp = RF_RESP(*bpp);
	register rf_common_t	*cop = RF_COM(*bpp);
	register int		error;
	int			uio_error = 0;	/* for uiomove error if any */

	if (cop->co_opcode == RFCOPYIN) {
		rf_rwa_t	rf_rwa;

		/*
		 * rfcl_writemove handles copyin response, 0 for no cacheing
		 */

		rf_rwa.uiop = uiop;
		rf_rwa.cached = FALSE;
		rf_rwa.wr_ioflag = 0;
		rf_rwa.wr_kern = FALSE;

		if ((error = rfcl_writemove(bpp, &rf_rwa, replysdp, rdp->rd_sdp,
		  &uio_error)) == 0) {
			if ((error = rf_rcvmsg(rdp, bpp)) == 0) {
				rp = RF_RESP(*bpp);
				cop = RF_COM(*bpp);
				error = rp->rp_errno;
				if (!rf_sigisempty(rp, RFS1DOT0)) {
					rf_postrpsigs(rp, RFS1DOT0, u.u_procp);
				}
			}
		}
		if (error || (error = uio_error) != 0) {
			pn_setlast(pnp);
			return error;
		}
	}
	if (cop->co_opcode != DUUTIME) {
		return ducl_resetpath(*bpp, pnp, dvp, vpp);
	} else if ((error = rp->rp_errno) != 0) {
		pn_setlast(pnp);
		return error;
	} else {
		/* the following is only reached when the uutime has occurred
		 * successfully on the remote system, there may have been a
		 * a single RFCOPYIN response sent.
		 */
		pnp->pn_path += pnp->pn_pathlen;
		pnp->pn_pathlen = 0;

		/* Upper level expects back a reference, will do a VN_RELE */
		VN_HOLD(dvp);
		*vpp = dvp;
		return 0;
	}
}

/*
 * pure lookup via chdir for links only, this is not generally useable.
 */
STATIC int
du_link_chdir(dvp, comp, vpp, pnp, flags, rdirvp, crp)
	vnode_t			*dvp;		/* current directory */
	char			*comp;		/* current component of name */
	vnode_t			**vpp;		/* return vnode pointer */
	register pathname_t	*pnp;		/* pathname structure */
	int			flags;		/* not used */
	vnode_t			*rdirvp;	/* root for this process */
	cred_t			*crp;		/* credentials structure */
{
	register int		error;

	/*
	 * This a a lookup on the target directory or file.
	 */
	if (*pnp->pn_path == '\0') {
		/*
		 * This is a lookup on the target file.  ENOENT
		 * will let the link proceed now, maybe to fail
		 * later on the server.
		 */
		error = ENOENT;		/* so lookuppn will succeed */
		pnp->pn_pathlen = 0;
	} else {
		/*
		 * Lookup on the target directory.
		 * Save the last part of the pathname and send the
		 * rest as a DUCHDIR message.  Restore the saved
		 * part on return from dusys_chdirec.
		 */
		register char	savechar;
		register char	*buf = pnp->pn_buf;
		register int	savelen = 0;
		register char	*savepath = pnp->pn_path +
					pnp->pn_pathlen;

		while (savepath > buf && *savepath != '/') {
			--savepath;
			++savelen;
		}
		savechar = *savepath;
		*savepath = '\0';
		pnp->pn_pathlen -= savelen;
		error = dusys_chdirec(dvp, comp, vpp, pnp, flags, rdirvp, crp);

		/* Reset the pathname before returning. */
		pnp->pn_pathlen += savelen;
		*savepath = savechar;
	}
	return error;
}

/*
 * Either RFDOTDOT, RFPATHREVAL, or the system call completed succesfully.
 * Update pnp and vpp for du_lookup to return.  Get new reference to result
 * vnode we return through vpp.  Lie through your teeth to the upper level
 * to make it think this is a pure lookup.
 *
 * Return 0 for success, nonzero errno for failure.
 */
STATIC int
dusys_copen_resp(bp, pnp, dvp, vpp, crp, rqop, giftsdp, fmode)
	mblk_t			*bp;
	register pathname_t	*pnp;
	vnode_t			*dvp;
	vnode_t			**vpp;
	cred_t			*crp;
	int			rqop;
	sndd_t			*giftsdp;
	register int		fmode;
{
	register rf_response_t	*rp = RF_RESP(bp);
	register rf_message_t	*msg = RF_MSG(bp);
	int			error = 0;
	vnode_t			*vp;
	register struct dustash	*dstp;

	if (rqop != (int)RF_COM(bp)->co_opcode) {
		sndd_free(&giftsdp);
		error = ducl_resetpath(bp, pnp, dvp, vpp);
	} else if ((error = rp->rp_errno) != 0) {
		rfcl_giftfree(bp, &giftsdp, crp);
	} else if (!(msg->m_stat & RF_GIFT)) {
		gdp_j_accuse("dusys_copen_resp: no file reference",
		  QPTOGP((queue_t *)msg->m_queue));
		sndd_free(&giftsdp);
		error = EPROTO;
	} else {
		sndd_set(giftsdp, msg->m_queue, &msg->m_gift);
		if ((error = rfcl_findsndd(&giftsdp, crp, bp, dvp->v_vfsp))
		  == 0) {

			/*
			 * We can update the out parameter consistently, and
			 * with impunity, with the correct result vp.  The
			 * current implementations of lookuppn and vn_create
			 * will discard the reference if they are interested
			 * in the parent, and not the child.  We don't have to
			 * worry about the vnode disappearing in that case,
			 * because we bump the count when stashing it.  If this
			 * is an open, not a create, the upper level will retain
			 * the reference, and we will have done the right thing,
			 * not inflating the count.
			 *
			 * Note that rfcl_findsndd does a VN_HOLD.
			 */

			vp = *vpp = SDTOV(giftsdp);

			giftsdp->sd_fhandle = rp->rp_fhandle;
			if (rp->rp_cache & RP_MNDLCK) {
				giftsdp->sd_stat |= SDMNDLCK;
			}

			pnp->pn_path += pnp->pn_pathlen;
			pnp->pn_pathlen = 0;
			if (rqop == RFCREATE || rqop == DUCOREDUMP ||
			  fmode & FCREAT) {

				/* Need to stash only for creates */

				VN_HOLD(vp);
				dstp = dst_alloc();
				dstp->DST_VP = vp;
				dstp->dst_pid = u.u_procp->p_pid;
				DST_LINK(VTOSD(dvp), dstp);
			}
		} else {
			*vpp = NULLVP;
		}
	}
	return error;
}

/*
 * Copy the member fields from the referenced statfs structure into the
 * referenced statvfs structure, zeroing other fields of the statvfs.
 * Returns 0 for success, errno for failure.
 */
STATIC int
du_fs_to_vfs(sfp, svp, vfsp)
	register struct statfs	*sfp;
	register struct statvfs	*svp;
	register vfs_t		*vfsp;
{
	register int		i;
	register char		*cp;
	register char		*cp2;
	register int		bmul;

	if (!sfp->f_bsize) {
		return EPROTO;
	}

	bzero((caddr_t)svp, sizeof(struct statvfs));
	svp->f_bsize = sfp->f_bsize;
	svp->f_frsize = !sfp->f_frsize ? sfp->f_bsize : sfp->f_frsize;

	/* statfs blocks/bfree is in terms of 512 byte blocks. */

	if (svp->f_frsize >= 512) {
		bmul = svp->f_frsize >> 9;
		svp->f_blocks = sfp->f_blocks / bmul;
		svp->f_bavail = svp->f_bfree = sfp->f_bfree / bmul;
	} else {
		bmul = 512 / svp->f_frsize;
		svp->f_blocks = sfp->f_blocks * bmul;
		svp->f_bavail = svp->f_bfree = sfp->f_bfree * bmul;
	}

	svp->f_files = sfp->f_files;
	svp->f_ffree = sfp->f_ffree;
	svp->f_favail = sfp->f_ffree;
	svp->f_fsid = vfsp->vfs_dev;

	/*
	 * We can't provide the base type because the old statfs structure
	 * supplies only an fstype number, which is meaningless on the
	 * client.
	 */

	strcpy((caddr_t)svp->f_basetype, "unknown");
	svp->f_flag = vf_to_stf(vfsp->vfs_flag);

	/* Fill f_name, f_pack from variable length strings in f_fstr. */

	cp = svp->f_fstr;
	cp2 = sfp->f_fname;
	for (i = 0; i < sizeof(sfp->f_fname); i++, cp2++) {
		if (*cp != '\0') {
			*cp2 = *cp++;
		} else {
			*cp2 = '\0';
		}
	}
	while (cp++ != '\0' && i < sizeof(svp->f_fstr) - 
	  sizeof(sfp->f_fpack)) {
		i++;
	}
	cp2 = sfp->f_fpack;
	for (i = 0; i < sizeof(sfp->f_fpack); i++, cp2++) {
		if (*cp != '\0') {
			*cp2 = *cp++;
		} else {
			*cp2 = '\0';
		}
	}

	/*
	 * Set namemax to 14, which is DIRSIZ in SVR3.x.  This isn't generally
	 * correct, but the protocol doesn't provide the information.
	 */

	svp->f_namemax = 14;

	return 0;
}

int
dusr_saccess(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	vnode_t			*vp;
	register int		fmode = RF_REQ(stp->sr_in_bp)->rq_mode_op.fmode;
	register int		error = 0;

	error = rfsr_lookupname(FOLLOW, stp, NULLVPP, &vp, ctrlp);
	SR_FREEMSG(stp);
	if (!error && *ctrlp == SR_NORMAL) {

		/*
		 * NOTE:  if client had not stolen the mode bits
		 * out of the u_block during namei, they
		 * would already be shifted.
		 */
		fmode = (fmode << 6) & (VREAD|VWRITE|VEXEC);
		if (fmode & VWRITE && stp->sr_srmp->srm_flags & SRM_RDONLY) {
			error = EROFS;
		} else if (fmode) {
			error = VOP_ACCESS(vp, fmode, 0, stp->sr_cred);
		}
		VN_RELE(vp);
	}
	return error;
}

int
dusr_chdirec(stp, ctrlp)
	register rfsr_state_t *stp;
	register rfsr_ctrl_t *ctrlp;
{
	vnode_t *vp;
	register int error = 0;

	rfsr_fsinfo.fsivop_other++;
	rfsr_fsinfo.fsivop_lookup++;

	if (RF_COM(stp->sr_in_bp)->co_opcode == DUCHROOT &&
	  !suser(stp->sr_cred)) {
		return EPERM;
	}

	error = rfsr_lookupname(FOLLOW, stp, NULLVPP, &vp, ctrlp);
	SR_FREEMSG(stp);
	if (error || *ctrlp != SR_NORMAL) {
		return error;
	}
	if (vp->v_type != VDIR) {
		error = ENOTDIR;
	} else {
		error = VOP_ACCESS(vp, VEXEC, 0, stp->sr_cred);
	}
	if (error) {
		VN_RELE(vp);
		return error;
	}
	ASSERT(!stp->sr_out_bp);
	stp->sr_out_bp = rfsr_rpalloc((size_t)0, stp->sr_vcver);
	return rfsr_gift_setup(stp, vp, u.u_srchan);
}

int
dusr_chmod(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	vattr_t			vattr;

	vattr.va_mask = AT_MODE;
	vattr.va_mode = RF_REQ(stp->sr_in_bp)->rq_mode_op.fmode & MODEMASK;

	return dusr_namesetattr(FOLLOW, &vattr, 0, stp, ctrlp);
}

/* In the vnode kernel, file system implementations take care of flushing
 * internal data structures to physical media, so this operation can succeed
 * trivially, keeping clients happy.
 */
/* ARGSUSED */
int
dusr_iupdate(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	SR_FREEMSG(stp);
	return 0;
}

int
dusr_chown(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);
	register struct gdp	*gdpp = stp->sr_gdpp;
	vattr_t vattr;

	vattr.va_mask = AT_UID|AT_GID;
	vattr.va_uid = gluid(gdpp, req->rq_chown.uid);
	vattr.va_gid = glgid(gdpp, req->rq_chown.gid);

	return dusr_namesetattr(FOLLOW, &vattr, 0, stp, ctrlp);
}

int
dusr_coredump(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	rfsr_fsinfo.fsivop_open++;
	rfsr_fsinfo.fsivop_create++;
	rfsr_fsinfo.fsivop_lookup++;

	return dusr_vn_open(FREAD|FWRITE|FCREAT,
	  0666 & MODEMASK & (int)~RF_REQ(stp->sr_in_bp)->rq_coredump.cmask,
	  stp, ctrlp);
}

int
dusr_creat(stp, ctrlp)
	register rfsr_state_t 	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);

	rfsr_fsinfo.fsivop_open++;
	rfsr_fsinfo.fsivop_create++;
	rfsr_fsinfo.fsivop_lookup++;

	return dusr_vn_open(FWRITE | FCREAT | FTRUNC,
	  (int)req->rq_create.fmode & MODEMASK & (int)~req->rq_create.cmask,
	  stp, ctrlp);
}

int
dusr_exec(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	vnode_t		*vp;
	pathname_t	pn;
	size_t		len;
	rf_response_t	*rsp;
	vattr_t		vattr;
	int		error = 0;
	int		pathsz = RF_MSG(stp->sr_in_bp)->m_size - RFV1_MINREQ;

	rfsr_fsinfo.fsivop_lookup++;
	rfsr_fsinfo.fsivop_open++;

	if (RF_PULLUP(stp->sr_in_bp, RFV1_MINREQ, (size_t)pathsz)) {
		return rfsr_j_accuse("dusr_exec bad data", stp);
	}
	rf_msgdata(stp->sr_in_bp, RFV1_MINREQ)[pathsz - 1] = '\0';

	if ((error =
	  pn_get(rf_msgdata(stp->sr_in_bp, RFV1_MINREQ), UIO_SYSSPACE, &pn))
	  != 0) {
		return error;
	}

	if ((error =
	  rfsr_lookuppn(&pn, FOLLOW, stp, NULLVPP, &vp, ctrlp)) != 0 ||
	  *ctrlp != SR_NORMAL) {
		pn_free(&pn);
		return error;
	}
	if (VOP_ACCESS(vp, VEXEC, 0, stp->sr_cred)) {
		VN_RELE(vp);
		return EACCES;
	}

	len = pn.pn_pathlen + 1;
	ASSERT(!stp->sr_out_bp);
	stp->sr_out_bp = rfsr_rpalloc(len, stp->sr_vcver);
	rsp = RF_RESP(stp->sr_out_bp);
	rsp->rp_v1giftinfo.mode = 0;
	vattr.va_mask = AT_MODE;
	if (error = VOP_GETATTR(vp, &vattr, 0, stp->sr_cred)) {
		VN_RELE(vp);
		return error;
	}
	if (error = VOP_OPEN(&vp, FREAD, stp->sr_cred)) {
		VN_RELE(vp);
		return error;
	}
	if (vattr.va_mode & VSUID) {
		rsp->rp_v1giftinfo.mode |= VSUID;
	}
	if (vattr.va_mode & VSGID) {
		rsp->rp_v1giftinfo.mode |= VSGID;
	}
	if (vattr.va_mode & VSVTX) {
		rsp->rp_v1giftinfo.mode |= VSVTX;
	}
	bcopy((caddr_t)pn.pn_path,
	  (caddr_t)rf_msgdata(stp->sr_out_bp, RFV1_MINRESP), len);
	pn_free(&pn);
	/*
	 * Do this last because it commits resources
	 */
	if (!(error = rfsr_gift_setup(stp, vp, u.u_srchan))) {
		rdu_open(stp->sr_gift, stp->sr_gdpp->sysid, 
		  u.u_srchan->sd_mntid, FREAD);
		return 0;
	}
	(void)VOP_CLOSE(vp, FREAD, 1, 0, stp->sr_cred);
	VN_RELE(vp);
	return error;
}

/*
 * System call protocol overloads DUFCNTL.
 */
int
dusr_fcntl(stp, ctrlp)
	register rfsr_state_t 	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_message_t 	*msg = RF_MSG(stp->sr_in_bp);
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);
	register vnode_t 	*rvp = stp->sr_rdp->rd_vp;
	register int 		cmd = req->rq_fcntl.cmd;
	register int 		arg = (int)req->rq_fcntl.fcntl;
	register long 		fflag = req->rq_fcntl.fflag;
	off_t 			offset = (off_t)req->rq_fcntl.offset;
	int			error = 0;
	register int		canon = stp->sr_gdpp->hetero != NO_CONV;
	vattr_t			vattr;

	/* 3.0 clients don't want response data piggybacked. */
	int			copyout = !(msg->m_stat & RF_VER1);
	flock_t			flock;		/* SVR4 flock */
	o_flock_t		oflock;		/* SVR3.x flock */

	rfsr_fsinfo.fsivop_other++;
	stp->sr_ret_val = cmd;
	if (cmd == F_FREESP) {
		vattr_t vattr;

		vattr.va_mask = AT_MODE;
		if (error = VOP_GETATTR(rvp, &vattr, 0, stp->sr_cred)) {
			SR_FREEMSG(stp);
			return error;
		}
		if (MANDLOCK(rvp, vattr.va_mode) && msg->m_stat & RF_VER1 &&
		  !(req->rq_flags & RQ_MNDLCK)) {
			/*
			 * Old clients go remote with inode locked;
			 * NACK will force unlock, avoid deadlock.
			 */
			SR_FREEMSG(stp);
			ASSERT(!stp->sr_out_bp);
			stp->sr_out_bp = rfsr_rpalloc((size_t)0, stp->sr_vcver);
			RF_RESP(stp->sr_out_bp)->rp_cache |= RP_MNDLCK;
			*ctrlp = SR_NACK_RESP;
			return ENOMEM;
		}
	}
	if (cmd == F_O_GETLK || cmd == F_SETLK || cmd == F_SETLKW ||
	  cmd == F_FREESP || cmd == F_ALLOCSP) {
		/*
		 * won't see an SVR4 F_GETLK here -
		 * SVR4 F_O_GETLK = SVR3.x F_GETLK.
		 *
		 * These cmds involve flock structures.
		 * Client is SVR3.x, so read in old flock
		 * structure assign it to an SVR4 flock
		 * and call appropriate flock routine.
		 */
		if (fflag & DUFRPREWRITE) {
			/*
			 * Lock data is prewritten.
			 */
			if ((error = rfsr_copyflock((caddr_t)&oflock, stp))
			  != 0) {
				return error;
			}
			fflag &= ~DUFRPREWRITE;
		} else if (rcopyin((caddr_t)arg, (caddr_t)&oflock,
		  sizeof(o_flock_t), 1)) {
			/*
			 * 3.0 machines don't prewrite locks.
			 * Note that using sizeof(o_flock_t) works
			 * only by chance in a heterogeneous environment,
			 * but the protocol won't let us send back a
			 * format string.
			 */
			SR_FREEMSG(stp);
			return EFAULT;		 /* historical */
		} else if (canon && !rf_fcanon(O_FLOCK_FMT, (caddr_t)&oflock,
		  (caddr_t)&oflock + sizeof(oflock), (caddr_t)&oflock)) {
			return rfsr_j_accuse("dusr_fcntl bad data", stp);
		}

		/*
		 * Since flock grew in SVR4 and this is a SVR3.x client,
		 * copy SVR3 flock to an SVR4 flock. This overlays o_flock
		 * on flock. This works because the input structure
		 * members for both are at the same offsets.
		 */

		bcopy((caddr_t)&oflock, (caddr_t)&flock, sizeof(o_flock_t));
		flock.l_sysid = 0;	/* set output fields for F_O_GETLK */
		flock.l_pid = 0;
	}
	SR_FREEMSG(stp);

	switch (cmd) {
	case F_SETFL:
		error =  VOP_SETFL(rvp, fflag, arg, stp->sr_cred);
		break;
	case F_O_GETLK:
	case F_SETLK:
	case F_SETLKW:

		error = VOP_FRLOCK(rvp, cmd, (int)&flock, fflag,
				offset, stp->sr_cred);
		/*
		 * Translation for backward compatibility.
		 */
		if (error == EAGAIN) {
			error = EACCES;
		}
		/*
		 * check for large values. l_sysid and
		 * l_pid were defined as integral short
		 * in 3.X.
		 */
		if (cmd == F_O_GETLK && flock.l_pid > SHRT_MAX ||
		  flock.l_sysid > SHRT_MAX) {
			/*
			 * Not the correct error code but 3.X servers
			 * don't understand EOVERFLOW.  Compatibility stinks!
			 */
			error = EACCES;
		}
		break;
	case F_ALLOCSP:
	case F_FREESP:
		if (rvp->v_type != VREG) {
			error = EINVAL;
		} else {
			error = VOP_SPACE(rvp, cmd, (int)&flock, fflag,
					offset, stp->sr_cred);
		}
		break;
	default:
		error = EINVAL;
		break;
	}
	if (!error) {
		if (cmd == F_O_GETLK) {
			rf_response_t	*rp;
			caddr_t		rpdata;
			o_flock_t	*oflp;

			ASSERT(!stp->sr_out_bp);
			stp->sr_out_bp = rfsr_rpalloc(canon ?
			  sizeof(o_flock_t) + OFLOCK_XP :
			  sizeof(o_flock_t), stp->sr_vcver);
			rp = RF_RESP(stp->sr_out_bp);
			rpdata = rf_msgdata(stp->sr_out_bp, RFV1_MINRESP);

			if (canon) {
				oflp = &oflock;
			} else {
				oflp = (o_flock_t *)rpdata;
			}
			oflp->l_type = flock.l_type;
			oflp->l_whence = flock.l_whence;
			oflp->l_start = flock.l_start;
			oflp->l_len = flock.l_len;
			oflp->l_sysid = (short)flock.l_sysid;
			oflp->l_pid = (short)flock.l_pid;
			if (canon) {
				rp->rp_count = rf_tcanon(O_FLOCK_FMT,
				  (caddr_t)oflp, rpdata);
			} else {
				oflp[0] = oflock;
			}
			if (copyout) {
				/*
				 * 3.0 clients can't handle data piggybacked
				 * on response.
				 */
				rp->rp_copyout.buf = (long)arg;
				rp->rp_errno = 0;
				rp->rp_copyout.copysync = 0;
				RF_COM(stp->sr_out_bp)->co_opcode = RFCOPYOUT;
				error = rf_sndmsg(u.u_srchan, stp->sr_out_bp,
				  RF_MIN_RESP(stp->sr_vcver) +
				  (size_t)rp->rp_count, (rcvd_t *)NULL, FALSE);
				ASSERT(!stp->sr_out_bp);
				stp->sr_out_bp =
				  rfsr_rpalloc((size_t)0, stp->sr_vcver);
			} else {
				rp->rp_fcntl.buf = (long)arg;
			}
		} else {
			ASSERT(!stp->sr_out_bp);
			stp->sr_out_bp =
			  rfsr_rpalloc((size_t)0, stp->sr_vcver);
		}

		vattr.va_mask = AT_SIZE;
		if ((error = VOP_GETATTR(rvp, &vattr, 0, stp->sr_cred)) == 0) {
			rf_response_t *rp = RF_RESP(stp->sr_out_bp);

			rp->rp_fcntl.isize = vattr.va_size;
			rp->rp_vcode = vattr.va_vcode;
		}
	}
	return error;
}

/* We continue to ship client side pointer around for compatability.
 */
/* ARGSUSED */
int
dusr_fstat(stp, ctrlp)
	register rfsr_state_t *stp;
	register rfsr_ctrl_t *ctrlp;
{
	struct stat *sbp = (struct stat *)RF_REQ(stp->sr_in_bp)->rq_stat_op.buf;
	register vnode_t *vp = stp->sr_rdp->rd_vp;

	SR_FREEMSG(stp);
	return dusr_cstat(vp, sbp, stp);
}

/* ARGSUSED */
int
dusr_fstatfs(stp, ctrlp)
	register rfsr_state_t *stp;
	register rfsr_ctrl_t *ctrlp;
{
	register rf_request_t *req = RF_REQ(stp->sr_in_bp);
	int len = req->rq_statfs_op.len;
	int fstyp= req->rq_statfs_op.fstyp;
	struct statfs *sb = (struct statfs *)req->rq_statfs_op.buf;

	SR_FREEMSG(stp);
	return dusr_cstatfs(stp, stp->sr_rdp->rd_vp->v_vfsp, len, fstyp, sb);
}

/*
 * System call protocol.  Lookup the existing name for a link operation.
 */
int
dusr_link(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	vnode_t			*vp;
	register int		error;

	rfsr_fsinfo.fsivop_lookup++;
	error = rfsr_lookupname(NO_FOLLOW, stp, NULLVPP, &vp, ctrlp);
	SR_FREEMSG(stp);
	if (!(error) && *ctrlp == SR_NORMAL) {
		ASSERT(!stp->sr_out_bp);
		stp->sr_out_bp = rfsr_rpalloc((size_t)0, stp->sr_vcver);
		error = rfsr_gift_setup(stp, vp, u.u_srchan);
	}
	return error;
}

/*
 * System call protocol
 * The pathname in the message is relative to the current directory,
 * and not necessarily a simple name.
 */
int
dusr_link1(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	vnode_t		*from_vp;
	int		error = 0;
	int		pathsz = RF_MSG(stp->sr_in_bp)->m_size - RFV1_MINREQ;

	rfsr_fsinfo.fsivop_lookup++;
	rfsr_fsinfo.fsivop_lookup++;
	rfsr_fsinfo.fsivop_other++;

	if (RF_PULLUP(stp->sr_in_bp, RFV1_MINREQ, (size_t)pathsz)) {
		return rfsr_j_accuse("dusr_link1 bad request", stp);
	}
	rf_msgdata(stp->sr_in_bp, RFV1_MINREQ)[pathsz - 1] = '\0';

	if ((from_vp = rf_gifttovp(&RF_REQ(stp->sr_in_bp)->rq_link.from,
	  RFS1DOT0)) == NULL) {
		/*
		 * The existing file is not on this server, but we can't
		 * just fail the operation outright, because the target
		 * pathname  still might cross back to the client.  So
		 * we do a lookup and return EXDEV if the pathname
		 * ends on this server.
		 */
		error =
		  rfsr_lookupname(NO_FOLLOW, stp, NULLVPP, NULLVPP, ctrlp);
		if (*ctrlp == SR_NORMAL && (!error || error == ENOENT)) {
			error = EXDEV;
		}
		return error;
	}

	/*
	 * dusr_vn_link is prepared to deal with pathnames crossing
	 * back to the client.
	 */

	return dusr_vn_link(from_vp, rf_msgdata(stp->sr_in_bp, RFV1_MINREQ),
	  stp, ctrlp);
}

int
dusr_mkdir(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);
	vnode_t			*vp;
	vattr_t			vattr;
	int			error;

	rfsr_fsinfo.fsivop_create++;
	rfsr_fsinfo.fsivop_lookup++;
	rfsr_fsinfo.fsivop_lookup++;
	vattr.va_mask = AT_TYPE | AT_MODE;
	vattr.va_type = VDIR;
	vattr.va_mode = req->rq_mkdir.fmode & PERMMASK & ~req->rq_mkdir.cmask;

	error = dusr_vn_create(&vattr, EXCL, 0, &vp, CRMKDIR, stp, ctrlp);
	SR_FREEMSG(stp);
	if (!error && *ctrlp == SR_NORMAL) {
		VN_RELE(vp);
	}
	return error;
}

/* ARGSUSED */
int
dusr_mknod(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);
	register int		fmode = (int)req->rq_mknod.fmode;
	vnode_t			*vp;
	vattr_t			vattr;
	register int		error = 0;

	/*
	 * Zero type is equivalent to a regular file
	 */
	if (!(fmode & S_IFMT)) {
		fmode |= S_IFREG;
	}
	/* Must be the super-user unless making a FIFO node.
	 */
	if (fmode & S_IFMT != S_IFIFO && !suser(stp->sr_cred)) {
		return EPERM;
	}
	/*
	 * Set up desired attributes and dusr_vn_create the file.
	 */
	vattr.va_mask = AT_MODE | AT_TYPE;
	vattr.va_type = IFTOVT(fmode);
	vattr.va_mode = fmode & MODEMASK & ~req->rq_mknod.cmask;
	if (vattr.va_type == VCHR || vattr.va_type == VBLK) {
		vattr.va_rdev = (dev_t)req->rq_mknod.dev;
	}
	error = dusr_vn_create(&vattr, EXCL, 0, &vp, CRMKNOD, stp, ctrlp);
	SR_FREEMSG(stp);
	if (!error && *ctrlp == SR_NORMAL) {
		VN_RELE(vp);
	}
	return error;
}

int
dusr_open(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);

	rfsr_fsinfo.fsivop_open++;
	rfsr_fsinfo.fsivop_lookup++;

	return dusr_vn_open((int)(req->rq_open.fmode - FOPEN),
	  (int)req->rq_open.crtmode & MODEMASK & (int)~req->rq_open.cmask,
	  stp, ctrlp);
}

int
dusr_rmdir(stp, ctrlp)
	register rfsr_state_t *stp;
	register rfsr_ctrl_t *ctrlp;
{
	rfsr_fsinfo.fsivop_other++;
	rfsr_fsinfo.fsivop_lookup++;

	return dusr_vn_remove(RMDIRECTORY, stp, ctrlp);
}

int
dusr_rmount(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	vnode_t			*vp;
	register int		error;

	rfsr_fsinfo.fsivop_lookup++;
	error = rfsr_lookupname(FOLLOW, stp, NULLVPP, &vp, ctrlp);
	SR_FREEMSG(stp);
	if (!error && *ctrlp == SR_NORMAL && vp) {
		error = EREMOTE;
		VN_RELE(vp);
	}
	return error;
}

/*
 * Seeks on RFS remote files are documented to have funny semantics, just
 * setting the file pointer back on the client.
 * For the same reason, there is no RFS server VOP_SEEK.
 */
/* ARGSUSED */
int
dusr_seek(stp, ctrlp)
	register rfsr_state_t *stp;
	register rfsr_ctrl_t *ctrlp;
{
	vattr_t vattr;
	int error = 0;

	vattr.va_mask = AT_SIZE;
	if (error = VOP_GETATTR(stp->sr_rdp->rd_vp, &vattr, 0, stp->sr_cred)) {
		stp->sr_ret_val = -1;
	} else {
		stp->sr_ret_val = vattr.va_size;
	}
	return error;
}

/*
 * We continue to send stat buf pointer back to client for compatibility
 */
int
dusr_stat(stp, ctrlp)
	register rfsr_state_t 	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register struct stat	*sb;
	vnode_t			*vp;
	register int		error;

	sb = (struct stat *)RF_REQ(stp->sr_in_bp)->rq_stat_op.buf;
	error = rfsr_lookupname(FOLLOW, stp, NULLVPP, &vp, ctrlp);
	SR_FREEMSG(stp);
	if (!error && *ctrlp == SR_NORMAL) {
		error = dusr_cstat(vp, sb, stp);
		VN_RELE(vp);
	}
	return error;
}

/*
 * System call protcol.
 * We continue to send stat buf pointer back to client for compatibility
 */
int
dusr_statfs(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);
	vnode_t			*vp;
	int			len = req->rq_statfs_op.len;
	int			fstyp = req->rq_statfs_op.fstyp;
	struct statfs		*sb = (struct statfs *)req->rq_statfs_op.buf;
	register int		error;

	error = rfsr_lookupname(FOLLOW, stp, NULLVPP, &vp, ctrlp);
	SR_FREEMSG(stp);
	if (!error && *ctrlp == SR_NORMAL) {
		error = dusr_cstatfs(stp, vp->v_vfsp, len, fstyp, sb);
		VN_RELE(vp);
	}
	return error;
}

int
dusr_unlink(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	return dusr_vn_remove(RMFILE, stp, ctrlp);
}

int
dusr_utime(stp, ctrlp)
	register rfsr_state_t	*stp;
	register rfsr_ctrl_t	*ctrlp;
{
	register rf_request_t	*req = RF_REQ(stp->sr_in_bp);
	register gdp_t		*gdpp = stp->sr_gdpp;
	time_t			stv[2];
	vattr_t			vattr;
	int			flags = 0;

	/*
	 * The client sends the path in the data part of the request.  If this
	 * utime request involves use of a client side time_t, that must be
	 * retrieved with an RFCOPYIN exchange.
	 */
	if (req->rq_utime.buf) {

		/* Remote is setting time explicitly. */

		if (rcopyin((caddr_t)req->rq_utime.buf, (caddr_t)stv,
		  sizeof(stv), 1)) {

			/*
			 * Note that using sizeof(stv) works
			 * only by chance in a heterogeneous environment,
			 * but the protocol won't let us send back a
			 * format string.
			 */

			return EFAULT;	/* historical */
		} else {
			if (gdpp->hetero != NO_CONV &&
			  !rf_fcanon("ll", (caddr_t)stv,
			  (caddr_t)stv + sizeof(stv), (caddr_t)stv)) {
				return rfsr_j_accuse("dusr_utime bad data",
				  stp);
			}
			stv[0] -= gdpp->timeskew_sec;
			stv[1] -= gdpp->timeskew_sec;
		}
		flags = ATTR_UTIME;
	} else {
		/* Use local time
		 */
		stv[0] = hrestime.tv_sec;
		stv[1] = hrestime.tv_sec;
	}
	vattr.va_mask = AT_ATIME|AT_MTIME;
	vattr.va_atime.tv_sec = stv[0];
	vattr.va_atime.tv_nsec = 0;
	vattr.va_mtime.tv_sec = stv[1];
	vattr.va_mtime.tv_nsec = 0;

	return dusr_namesetattr(FOLLOW, &vattr, flags, stp, ctrlp);
}

/*
 * Open/create a vnode.
 * For succesful cases, updates stp->sr_gift and returns 0.
 * Otherwise returns a nonzero errno.
 * Sets stp->sr_mandatory if appropriate.
 *
 * In all cases, frees incoming message.
 */
STATIC int
dusr_vn_open(filemode, createmode, stp, sr_ctrlp)
	register int	filemode;
	int		createmode;
	rfsr_state_t	*stp;
	rfsr_ctrl_t	*sr_ctrlp;
{
	vnode_t		*vp;
	register int	mode  = 0;
	register int	error = 0;
	vattr_t		vattr;

	if (!(filemode & (FREAD|FWRITE))) {
		SR_FREEMSG(stp);
		return EINVAL;
	}
	if (filemode & FREAD) {
		mode |= VREAD;
	}
	if (filemode & (FWRITE | FTRUNC)) {
		mode |= VWRITE;
	}
	if (filemode & FCREAT) {
		vcexcl_t excl;

		vattr.va_mask = AT_TYPE | AT_MODE;
		vattr.va_type = VREG;
		vattr.va_mode = (ushort)createmode;
		if (filemode & FTRUNC) {
			vattr.va_mask |= AT_SIZE;
			vattr.va_size = 0;
		}
		if (filemode & FEXCL) {
			excl = EXCL;
		} else {
			excl = NONEXCL;
		}
		filemode &= ~(FTRUNC | FEXCL);
		error = dusr_vn_create(&vattr, excl, mode, &vp,
		  CRCREAT, stp, sr_ctrlp);
		SR_FREEMSG(stp);
		if (error || *sr_ctrlp != SR_NORMAL) {
			return error;
		}
	} else {
		/*
		 * Plain open; just look up the name for now.
		 */
		error = rfsr_lookupname(FOLLOW, stp, NULLVPP, &vp, sr_ctrlp);
		SR_FREEMSG(stp);
		if (error || *sr_ctrlp != SR_NORMAL) {
			return error;
		}
		/*
		 * Can't write directories, active texts, or
		 * read-only file systems.  Can't truncate files
		 * on which mandatory locking is in effect.
		 */
		if (filemode & (FWRITE | FTRUNC)) {
			if (vp->v_type == VDIR) {
				VN_RELE(vp);
				return EISDIR;
			}
			if (stp->sr_srmp->srm_flags & SRM_RDONLY) {
				VN_RELE(vp);
				return EACCES;
			}
			if (vp->v_vfsp->vfs_flag & VFS_RDONLY) {
				VN_RELE(vp);
				return EROFS;
			}
			/*
			 * Can't truncate files on which mandatory locking
			 * is in effect.
			 */
			vattr.va_mask = AT_MODE;
			if ((filemode & FTRUNC)
			  && vp->v_filocks
			  && !(error = VOP_GETATTR(vp, &vattr, 0, stp->sr_cred))
			  && MANDLOCK(vp, vattr.va_mode)) {
				VN_RELE(vp);
				return EAGAIN;
			}
		}
		/*
		 * check permissions
		 */
		if (error = VOP_ACCESS(vp, mode, 0, stp->sr_cred)) {
			VN_RELE(vp);
			return error;
		}
	}
	/*
	 * Do opening protocol for creat or open.
	 */
	if (error = VOP_OPEN(&vp, filemode, stp->sr_cred)) {
		VN_RELE(vp);
		return error;
	}
	/*
	 * truncate if required
	 */
	if (filemode & FTRUNC) {
		vattr.va_mask = AT_SIZE;
		vattr.va_size = 0;
		error = VOP_SETATTR(vp, &vattr, 0, stp->sr_cred);
	}
	if (!error) {
		/*
		 * Do these last to avoid committing resources.
		 */
		sndd_t	*srchan = u.u_srchan;

		VN_HOLD(vp);
		ASSERT(!stp->sr_out_bp);
		stp->sr_out_bp = rfsr_rpalloc((size_t)0, stp->sr_vcver);
		if (!(error = rfsr_gift_setup(stp, vp, srchan))) {
			rdu_open(stp->sr_gift, stp->sr_gdpp->sysid,
			  srchan->sd_mntid, filemode);
			VN_RELE(vp);
		}
	}
	if (error) {
		/*
		 * Errors with an open file collected here.
		 * Don't overwrite error
		 */
		(void)VOP_CLOSE(vp, filemode, 1, 0, stp->sr_cred);
		VN_RELE(vp);
	}
	return error;
}

/*
 * create a vnode
 */
STATIC int
dusr_vn_create(vap, excl, mode, vpp, why, stp, sr_ctrlp)
	vattr_t		*vap;
	vcexcl_t	excl;
	int		mode;
	vnode_t		**vpp;
	create_t	why;
	rfsr_state_t	*stp;
	rfsr_ctrl_t	*sr_ctrlp;
{
	vnode_t		*dvp;	/* ptr to parent dir vnode */
	struct pathname	pn;
	register int	error;
	int		pathsz = RF_MSG(stp->sr_in_bp)->m_size - RFV1_MINREQ;

	/*
	 * Lookup directory.
	 * If new object is a file, call lower level to create it.
	 * Note that it is up to the lower level to enforce exclusive
	 * creation, if the file is already there.
	 * This allows the lower level to do whatever
	 * locking or protocol that is needed to prevent races.
	 * If the new object is directory call lower level to make
	 * the new directory, with "." and "..".
	 */
	dvp = NULL;
	*vpp = NULL;

	if (RF_PULLUP(stp->sr_in_bp, RFV1_MINREQ, (size_t)pathsz)) {
		return rfsr_j_accuse("dusr_vn_create bad request", stp);
	}

	rf_msgdata(stp->sr_in_bp, RFV1_MINREQ)[pathsz - 1] = '\0';
	if ((error =
	  pn_get(rf_msgdata(stp->sr_in_bp, RFV1_MINREQ), UIO_SYSSPACE, &pn))
	  != 0) {
		return error;
	}

	/*
	 * lookup will find the parent directory for the vnode.
	 * When it is done the pn holds the name of the entry
	 * in the directory.
	 * If this is a non-exclusive create we also find the node itself.
	 */

	if (excl == EXCL) {
		error = rfsr_lookuppn(&pn, NO_FOLLOW, stp, &dvp,
		  NULLVPP, sr_ctrlp);
	} else {
		error = rfsr_lookuppn(&pn, FOLLOW, stp, &dvp, vpp, sr_ctrlp);
	}
	if (error || *sr_ctrlp != SR_NORMAL) {
		pn_free(&pn);
		return error;
	}
	/*
	 * Make sure filesystem is writeable
	 */
	if (stp->sr_srmp->srm_flags & SRM_RDONLY) {
		if (*vpp) {
			VN_RELE(*vpp);
		}
		error = EACCES;
	} else if (dvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		if (*vpp) {
			VN_RELE(*vpp);
		}
		error = EROFS;
	} else if (excl == NONEXCL && *vpp != NULLVP) {
		register struct vnode *vp = *vpp;

		/*
		 * File already exists.	 If a mandatory lock has been
		 * applied, return EAGAIN.
		 */
		if (vp->v_filocks != NULL) {
			struct vattr vattr;

			vattr.va_mask = AT_MODE;
			if (error = VOP_GETATTR(vp, &vattr, 0, u.u_cred)) {
				VN_RELE(vp);
				goto out;
			}
			if (MANDLOCK(vp, vattr.va_mode)) {
				error = EAGAIN;
				VN_RELE(vp);
				goto out;
			}
		}

		/*
		 * If the file is the root of a VFS, we've crossed a
		 * mount point and the "containing" directory that we
		 * acquired above (dvp) is irrelevant because it's in
		 * a different file system.  We apply VOP_CREATE to the
		 * target itself instead of to the containing directory
		 * and supply a null path name to indicate (conventionally)
		 * the node itself as the "component" of interest.
		 *
		 * The intercession of the file system is necessary to
		 * ensure that the appropriate permission checks are
		 * done.
		 */
		if (vp->v_flag & VROOT) {
			ASSERT(why != CRMKDIR);
			error = VOP_CREATE(vp, "", vap, excl, mode,
			  vpp, stp->sr_cred);
			/*
			 * If the create succeeded, it will have created
			 * a new reference to the vnode.  Give up the
			 * original reference.
			 */
			VN_RELE(vp);
			goto out;
		}

		/*
		 * The file is already there.
		 * We throw the vnode away to let VOP_CREATE truncate the
		 * file in a non-racy manner.
		 */
		VN_RELE(*vpp);
	}
	if (!error) {
		if (why != CRMKNOD) {
			vap->va_mode &= ~VSVTX;
		}
		/*
		 * Call mkdir() if specified, otherwise create().
		 */
		if (why == CRMKDIR) {
			error = VOP_MKDIR(dvp, pn.pn_path, vap, vpp,
				stp->sr_cred);
		} else {
			error = VOP_CREATE(dvp, pn.pn_path, vap, excl, mode,
				vpp, stp->sr_cred);
		}
	}
out:
	pn_free(&pn);
	VN_RELE(dvp);
	return error;
}

/*
 * dusr_vn_link() is the server-specific version of the vn_link()
 * routine.  dusr_vn_link() takes a pointer to a vnode corresponding
 * to the first pathname and a pointer to the second pathname as
 * arguments, unlike the standard vn_link() which takes two pathnames.
 * It also uses the server-specific version of the lookup routine.
 */
STATIC int
dusr_vn_link(fvp, to, stp, sr_ctrlp)
	vnode_t		*fvp;		/* from vnode ptr */
	char		*to;
	rfsr_state_t	*stp;
	rfsr_ctrl_t	*sr_ctrlp;
{
	vnode_t		*tdvp;		/* to directory vnode ptr */
	struct pathname	pn;
	register int	error;

	tdvp = NULLVP;
	if (error = pn_get(to, UIO_SYSSPACE, &pn)) {
		return error;
	}

	error = rfsr_lookuppn(&pn, NO_FOLLOW, stp, &tdvp, NULLVPP, sr_ctrlp);
	if (error || *sr_ctrlp != SR_NORMAL) {
		goto out;
	}
	/*
	 * Make sure both source vnode and target directory vnode are
	 * in the same vfs and that it is writeable.
	 */
	if (fvp->v_vfsp != tdvp->v_vfsp) {
		error = EXDEV;
		goto out;
	}
	if (tdvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
		goto out;
	}
	if (stp->sr_srmp->srm_flags & SRM_RDONLY) {
		error = EACCES;
		goto out;
	}
	/*
	 * do the link
	 */
	error = VOP_LINK(tdvp, fvp, pn.pn_path, stp->sr_cred);
out:
	pn_free(&pn);
	/*
	 * Client will issue a DUIPUT of the remote reference to fvp
	 */
	if (tdvp) {
		VN_RELE(tdvp);
	}
	return error;
}

/*
 * remove a file or directory.
 */
STATIC int
dusr_vn_remove(dirflag, stp, sr_ctrlp)
	rm_t		dirflag;
	rfsr_state_t	*stp;
	rfsr_ctrl_t	*sr_ctrlp;
{
	vnode_t		*vp;		/* entry vnode */
	vnode_t		*dvp;		/* ptr to parent dir vnode */
	vfs_t		*vfsp;		/* ptr to vfs containing vp */
	struct pathname	pn;		/* name of entry */
	vtype_t		vtype;
	register int	error;
	int		pathsz = RF_MSG(stp->sr_in_bp)->m_size - RFV1_MINREQ;

	vp = NULL;

	if (RF_PULLUP(stp->sr_in_bp, RFV1_MINREQ, (size_t)pathsz)) {
		return rfsr_j_accuse("dusr_vn_remove bad request", stp);
	}

	rf_msgdata(stp->sr_in_bp, RFV1_MINREQ)[pathsz - 1] = '\0';
	if ((error =
	  pn_get(rf_msgdata(stp->sr_in_bp, RFV1_MINREQ), UIO_SYSSPACE, &pn))
	  != 0) {
		return error;
	}

	error = rfsr_lookuppn(&pn, NO_FOLLOW, stp, &dvp, &vp, sr_ctrlp);
	if (error || *sr_ctrlp != SR_NORMAL) {
		pn_free(&pn);
		return error;
	}

	/*
	 * make sure there is an entry
	 */
	if (!vp) {
		error = ENOENT;
		goto out;
	}
	vfsp = vp->v_vfsp;

	/*
	 * make sure filesystem is writeable
	 */
	if (stp->sr_srmp->srm_flags & SRM_RDONLY) {
		error = EACCES;
		goto out;
	}
	/*
	 * don't unlink the root of a mounted filesystem.
	 */
	if (vp->v_flag & VROOT) {
		if (!(vfsp->vfs_flag & VFS_UNLINKABLE)) {
			error = EBUSY;
			goto out;
		} else {
			vnode_t *coveredvp = vfsp->vfs_vnodecovered;
			VN_HOLD(coveredvp);
			VN_RELE(vp);
			if ((error = dounmount(vfsp, stp->sr_cred)) != 0) {
				goto out;
			}
			vp = coveredvp;
			vfsp = vp->v_vfsp;
		}
	}
	if (vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
		goto out;
	}
	/*
	 * release vnode before removing
	 */
	vtype = vp->v_type;
	VN_RELE(vp);
	vp = NULL;
	if (dirflag == RMDIRECTORY) {
		/*
		 * Caller is using mkdir(2), which can only be applied to
		 * directories.
		 */
		if (vtype != VDIR) {
			error = ENOTDIR;
		} else {
			error = VOP_RMDIR(dvp, pn.pn_path, stp->sr_rdp->rd_vp,
			  stp->sr_cred);
		}
	} else {
		/*
		 * Unlink(2) can be applied to anything.
		 */
		error = VOP_REMOVE(dvp, pn.pn_path, stp->sr_cred);
	}
out:
	pn_free(&pn);
	if (vp) {
		VN_RELE(vp);
	}
	VN_RELE(dvp);
	return error;
}

/*
 * Server-specific common code for stat, fstat operations.
 */
STATIC int
dusr_cstat(vp, ubp, stp)
	register vnode_t	*vp;
	struct stat		*ubp;	/* client address - compatability */
	register rfsr_state_t	*stp;
{
	register struct stat	*sbp;
	register rf_response_t	*rp;
	int			error;
	vattr_t			vattr;
	register size_t		datasz;
	int			canon = stp->sr_gdpp->hetero != NO_CONV;
	char			*data;
	struct stat		stat;

	datasz = (stp->sr_gdpp->hetero != NO_CONV) ?
			sizeof(struct stat) + STAT_XP :
			 sizeof(struct stat);
	ASSERT(!stp->sr_out_bp);
	stp->sr_out_bp = rfsr_rpalloc(datasz, stp->sr_vcver);
	rp = RF_RESP(stp->sr_out_bp);
	data = rf_msgdata(stp->sr_out_bp, RFV1_MINRESP);
	if (canon) {
		sbp = &stat;
	} else {
		sbp = (struct stat *)data;
	}
	vattr.va_mask = AT_STAT;
	if (!(error = VOP_GETATTR(vp, &vattr, 0, stp->sr_cred)) &&
	  !(error = rfsr_vattr_map(stp, &vattr))) {
		/*
		 * Massage the struct vattr into a struct stat
		 * Fail (with a 3.2 vintage errno) if important values
		 * won't fit in the old field types.
		 */
		if (vattr.va_uid > USHRT_MAX || vattr.va_gid > USHRT_MAX ||
		  vattr.va_nodeid > USHRT_MAX || vattr.va_nlink > SHRT_MAX ||
		  cmpdev(vattr.va_rdev) == NODEV) {
			return EPROTO;
		}
		sbp->st_mode = VTTOIF(vattr.va_type) | vattr.va_mode;
		sbp->st_uid = (o_uid_t)vattr.va_uid;
		sbp->st_gid = (o_gid_t)vattr.va_gid;
		sbp->st_dev = (o_dev_t)cmpdev(vattr.va_fsid);
		sbp->st_ino = (o_ino_t)vattr.va_nodeid;
		sbp->st_nlink = (o_nlink_t)vattr.va_nlink;
		sbp->st_size = vattr.va_size;
		sbp->st_atime = vattr.va_atime.tv_sec;
		sbp->st_mtime = vattr.va_mtime.tv_sec;
		sbp->st_ctime = vattr.va_ctime.tv_sec;
		sbp->st_rdev = (o_dev_t)cmpdev(vattr.va_rdev);
		rp->rp_copyout.buf = (long)ubp;
		if (canon) {
			rp->rp_count = rf_tcanon(STAT_FMT, (caddr_t)sbp, data);
		}
	} else {
		/* reset rp_count and rp_nodata since there was a failure */
		rp->rp_count = 0;
		rp->rp_nodata = 1;
	}
	return error;
}

/*
 * Server-specific common code for statfs, fstatfs operations.
 * For success, fills in a response message with stafs info and
 * client statfs struct pointer, stuff it into state vector, returns 0.
 * For failure, returns nonzero errno.
 */
STATIC int
dusr_cstatfs(stp, vfsp, len, fstyp, csbp)
	register rfsr_state_t	*stp;
	register vfs_t		*vfsp;
	register int		len;
	register int		fstyp;
	register struct statfs	*csbp;	/* pass client pointer back */
{
	int			error;
	register rf_response_t	*rp;
	register size_t		datasz;
	char			*data;
	statvfs_t		statvfs;
	int			canon = stp->sr_gdpp->hetero != NO_CONV;

	/*
	 * fstype !=0 (unmounted filesys) unsupported.
	 */
	if (len < 0 || len > sizeof(struct statfs) || fstyp) {
		return EINVAL;
	}
	datasz = canon ? sizeof(struct statfs) + STATFS_XP :
	  sizeof(struct statfs);
	ASSERT(!stp->sr_out_bp);
	stp->sr_out_bp = rfsr_rpalloc(datasz, stp->sr_vcver);
	rp = RF_RESP(stp->sr_out_bp);
	data = rf_msgdata(stp->sr_out_bp, RFV1_MINRESP);
	/*
	 * Use VFS_STATVFS op to simulate obsolescent fstatfs sys call.
	 */
	if (!(error = VFS_STATVFS(vfsp, &statvfs))) {
		register vfssw_t	*vswp;
		register struct statfs	*sbp;
		struct statfs		statfs;
		register char		*cp, *cp2;
		register int		i;

		if (canon) {
			sbp = &statfs;
		} else {
			sbp = (struct statfs *)data;
		}
		sbp->f_bsize = statvfs.f_bsize;
		sbp->f_frsize = (statvfs.f_frsize == statvfs.f_bsize) ?
		  0 : statvfs.f_frsize;
		sbp->f_blocks = statvfs.f_blocks * (statvfs.f_frsize >> 9);
		sbp->f_bfree = statvfs.f_bfree * (statvfs.f_frsize >> 9);
		sbp->f_files = statvfs.f_files;
		sbp->f_ffree = statvfs.f_ffree;

		/*
		 * Fill f_name, f_pack from variable length strings in f_fstr.
		 */
		cp = statvfs.f_fstr;
		cp2 = sbp->f_fname;
		for (i = 0; i < sizeof(sbp->f_fname); i++, cp2++) {
			if (*cp != '\0') {
				*cp2 = *cp++;
 			} else {
				*cp2 = '\0';
			}
		}
		while (cp++ != '\0' && i < sizeof(statvfs.f_fstr) - 
		  sizeof(sbp->f_fpack)) {
			i++;
		}
		cp2 = sbp->f_fpack;
		for (i = 0; i < sizeof(sbp->f_fpack); i++, cp2++) {
			if (*cp != '\0') {
				*cp2 = *cp++;
			} else {
				*cp2 = '\0';
			}
		}
		if ((vswp = vfs_getvfssw(statvfs.f_basetype)) == NULL) {
			sbp->f_fstyp = 0;
		} else {
			sbp->f_fstyp = vswp - vfssw;
		}
		rp->rp_copyout.buf = (long)csbp;
		if (canon) {
			rp->rp_count = rf_tcanon(STATFS_FMT, (caddr_t)sbp,
			  data);
		}
	} else {
		/* reset these to reflect the failure */
		rp->rp_count = 0;
		rp->rp_nodata = 1;
	}
	return error;
}

/*
 * Server-specific utility routine for modifying attributes of named files.
 * Caller sets up attribute structure.
 * It frees the incoming message in all cases.
 * May change *sr_ctrlp, and always returns an error status.
 */
STATIC int
dusr_namesetattr(followlink, vap, flags, stp, sr_ctrlp)
	enum symfollow	followlink;
	vattr_t		*vap;
	int		flags;
	rfsr_state_t	*stp;
	rfsr_ctrl_t	*sr_ctrlp;
{
	vnode_t		*vp;
	register int	error;

	error = rfsr_lookupname(followlink, stp, NULLVPP, &vp, sr_ctrlp);
	SR_FREEMSG(stp);
	if (error || *sr_ctrlp != SR_NORMAL) {
		return error;
	}
	if (stp->sr_srmp->srm_flags & SRM_RDONLY) {
		error = EACCES;
	} else if (vp->v_vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
	} else {
		error = VOP_SETATTR(vp, vap, flags, stp->sr_cred);
	}
	VN_RELE(vp);
	return error;
}

/*
 * Allocate and transmit a name-based sys call protocol message,
 * and receive the response.
 * *(bpp == NULL) iff error.
 */
STATIC int
ducl_namemsg(chansdp, rdirvp, pnp, comp, crp, opcode, rqargp, bpp)
	sndd_t		*chansdp;
	vnode_t		*rdirvp;
	pathname_t	*pnp;
	char		*comp;
	cred_t		*crp;
	int		opcode;
	union rq_arg	*rqargp;
	mblk_t		**bpp;
{
	size_t		rqsize;
	int		complen = strlen(comp);
	size_t		datasz = complen + pnp->pn_pathlen + 1;
	rcvd_t		*rdp;
	char		*data;
	int		ntries;
	int		nacked;
	int		error = 0;

	*bpp = NULL;
	rqsize = RFV1_MINREQ + datasz;
	if ((error = rcvd_create(TRUE, RDSPECIFIC, &rdp)) != 0) {
		return error;
	}
	rdp->rd_sdp = chansdp;
	for (ntries = nacked = 1; ntries < RFCL_MAXTRIES && nacked; ntries++) {
		rf_request_t	*rqp;

		if ((error = rf_allocmsg(RFV1_MINREQ, datasz, BPRI_LO, TRUE,
		 NULLCADDR, NULLFRP, bpp)) != 0) {
			break;
		}
		rfcl_reqsetup(*bpp, chansdp, crp, opcode, R_ULIMIT);
		rqp = RF_REQ(*bpp);
		rqp->rq_arg = *rqargp;
		data = rf_msgdata(*bpp, RFV1_MINREQ);
		if (rdirvp && ISRFSVP(rdirvp) &&
		  rdirvp->v_vfsp == SDTOV(chansdp)->v_vfsp) {
			rqp->rq_rrdir_id = VTOSD(rdirvp)->sd_gift.gift_id;
		} else {
			rqp->rq_rrdir_id = 0;
		}
	 	(void)strcpy(data, comp);
		(void)strcpy(data + complen, pnp->pn_path);
		error = rfcl_xac(bpp, rqsize, rdp, RFS1DOT0, FALSE, &nacked);
	}
	rcvd_free(&rdp);
	return error;
}

/*
 * A utility for RFS1DOT0 connections.  Verifies that the response opcode
 * is RFDOTDOT and that the length of the response data is no more than
 * the original pathname.  If so, updates pnp and vpp appropriately and
 * returns 0.  Otherwise, issues a console warning, drops the circuit
 * to the offending server, and returns EPROTO.
 */
STATIC int
ducl_resetpath(bp, pnp, dvp, vpp)
	mblk_t			*bp;
	pathname_t		*pnp;
	vnode_t			*dvp;
	vnode_t			**vpp;
{
	register vnode_t	*vp;
	register int		datalen;
	int			error;


	if ((error = RF_PULLUP(bp, RFV1_MINRESP,
	  (size_t)RF_MSG(bp)->m_size - RFV1_MINRESP)) != 0) {
		gdp_j_accuse("ducl_resetpath bad response",
		  QPTOGP(VTOSD(dvp)->sd_queue));
		return EPROTO;
	}

	datalen = strlen(rf_msgdata(bp, RFV1_MINRESP));

	if (pnp->pn_pathlen < datalen || RF_COM(bp)->co_opcode != RFDOTDOT) {
		gdp_j_accuse("ducl_resetpath bad response",
		  QPTOGP(VTOSD(dvp)->sd_queue));
		return EPROTO;
	}
	pnp->pn_path += pnp->pn_pathlen - datalen;
	pnp->pn_pathlen = datalen;
	vp = ((rf_vfs_t *)(dvp->v_vfsp->vfs_data))->rfvfs_rootvp;
	VN_HOLD(vp);
	*vpp = vp;
	return error;
}

/*
 * Success:  returns a pointer to a dustash_t, its sequence number
 * set, nextp NULLed.
 * Failure:  panic
 *
 * Note:  callers have to remember sequence numbers for retrieval.
 * (rf_maxkmem isn't checked because this is a transient allocation.)
 */
STATIC dustash_t *
dst_alloc()
{
	register dustash_t *stashp;	/* result */

	if ((stashp = (dustash_t *)kmem_alloc(sizeof(dustash_t), KM_SLEEP))
	  == NULL) {
		cmn_err(CE_PANIC,
		  "dst_alloc: couldn't get space for dustash\n");
	} else {
		stashp->dst_nextp = NULL;
	}
	return stashp;
}

/* Return the denoted dustash to free list.
 */
STATIC void
dst_free(dstp)
	register dustash_t *dstp;
{
	kmem_free((caddr_t)dstp, sizeof(dustash_t));
}

/*
 * Unlink a dustash_t with dst_pid == pid from the denoted
 * send descriptor.
 * Returns a pointer to the dustash, or NULL if there is no
 * stash with dst_pid == pid.
 * NOTE:  Heavy use might argue for linking stashes on rings.
 */
STATIC dustash_t *
dst_unlink(sdp, pid)
	register sndd_t		*sdp;
	register pid_t		pid;
{
	register dustash_t	*dstp = sdp->sd_stashp;
	register dustash_t	*predp = NULL;

	while (dstp && dstp->dst_pid != pid) {
		predp = dstp;
		dstp = dstp->dst_nextp;
	}
	if (dstp) {
		if (predp) {
			predp->dst_nextp = dstp->dst_nextp;
		} else {
			sdp->sd_stashp = dstp->dst_nextp;
		}
		dstp->dst_nextp = NULL;
	}
	return dstp;
}


/*
 * Unlink and free any stashes on the denoted send descriptor.
 * Set sd_stashp to NULL.
 */
void
dst_clean(sdp)
	register sndd_t *sdp;
{
	register dustash_t *dstp = sdp->sd_stashp;
	register dustash_t *nextp;

	while (dstp) {
		nextp = dstp->dst_nextp;
		dst_free(dstp);
		dstp = nextp;
	}
	sdp->sd_stashp = NULL;
}

/* Copy stat info into vattr structure, disregard the va_mask field. */
/* TO DO:  good enough? */
STATIC void
du_stat_to_vattr(statp, vap)
	register struct stat	*statp;
	register vattr_t	*vap;
{
	vap->va_type = IFTOVT((int)statp->st_mode);
	vap->va_mode = statp->st_mode & MODEMASK;
	vap->va_uid = (uid_t)statp->st_uid;
	vap->va_gid = (gid_t)statp->st_gid;

	hiword(vap->va_fsid) = hibyte(statp->st_dev);
	loword(vap->va_fsid) = lobyte(statp->st_dev);

	/* Nasty machine-dependency:  sign-extend a character. */

	ASSERT(hiword(vap->va_fsid) & 0x80);

	hiword(vap->va_fsid) |= 0xff00;

	vap->va_nodeid = (ino_t)statp->st_ino;
	vap->va_nlink = (nlink_t)statp->st_nlink;
	vap->va_size = statp->st_size;
	vap->va_atime.tv_sec = statp->st_atime;
	vap->va_atime.tv_nsec = 0;
	vap->va_mtime.tv_sec = statp->st_mtime;
	vap->va_mtime.tv_nsec = 0;
	vap->va_ctime.tv_sec = statp->st_ctime;
	vap->va_ctime.tv_nsec = 0;
	vap->va_rdev = expdev(statp->st_rdev);
	vap->va_blksize = DU_DATASIZE;
	vap->va_nblocks = btod(statp->st_size);
	vap->va_vcode = 0;
}

/*
 * Massage o_flock structure embedded in bp into flock, decanonizing
 * if indicated by hetero.  bp must not have headers stripped from
 * RFS message.
 */
STATIC int
du_o_flock_to_flock(bp, flp)
	register mblk_t		*bp;
	register struct flock	*flp;
{
	size_t			datasz;
	register struct o_flock	*oflp;
	gdp_t			*gp;

	gp = QPTOGP((queue_t *)RF_MSG(bp)->m_queue);
	datasz = gp->hetero == NO_CONV ? sizeof(struct o_flock) :
	  sizeof(struct o_flock) + STATFS_XP;

	if (RF_PULLUP(bp, RFV1_MINRESP, datasz)) {
		gdp_j_accuse("du_o_flock_to_flock bad data", gp);
		return EPROTO;
	}

	oflp = (struct o_flock *)rf_msgdata(bp, RFV1_MINRESP);

	if (gp->hetero != NO_CONV &&
	  !rf_fcanon(O_FLOCK_FMT, (caddr_t)oflp, (caddr_t)oflp + datasz,
	  (caddr_t)oflp)) {
		gdp_j_accuse("du_o_flock_to_flock bad data", gp);
		return EPROTO;
	}

	flp->l_type = oflp->l_type;
	flp->l_whence = oflp->l_whence;
	flp->l_start = oflp->l_start;
	flp->l_len = oflp->l_len;
	flp->l_sysid = (long)oflp->l_sysid;
	flp->l_pid = (pid_t)oflp->l_pid;
	return 0;
}
