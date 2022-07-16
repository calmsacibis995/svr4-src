/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfs_vfsops.c	1.3.1.1"

/*	@(#)nfs_vfsops.c 2.91 88/10/17 SMI 	*/

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

#define	NFSCLIENT

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/pathname.h>
#include <sys/uio.h>
#include <sys/tiuser.h>
#include <sys/sysmacros.h>
#include <sys/kmem.h>
#include <netinet/in.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <nfs/nfs.h>
#include <nfs/nfs_clnt.h>
#include <nfs/rnode.h>
#include <nfs/mount.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>

#ifdef NFSDEBUG
extern int nfsdebug;
#endif

STATIC int nfsrootvp();
extern struct vnode *makenfsnode();
#define	MAPSIZE  (256/NBBY)
static char nfs_minmap[MAPSIZE]; /* Map for minor device allocation */

/*
 * nfs vfs operations.
 */
STATIC	int nfs_mount();
STATIC	int nfs_unmount();
STATIC	int nfs_root();
STATIC	int nfs_statvfs();
STATIC	int nfs_sync();
STATIC	int nfs_vget();
STATIC	int nfs_mountroot();
STATIC	int nfs_swapvp();
STATIC	int nfs_nosys();

struct vfsops nfs_vfsops = {
	nfs_mount,
	nfs_unmount,
	nfs_root,
	nfs_statvfs,
	nfs_sync,
	nfs_vget,
	nfs_mountroot,
	nfs_swapvp,
	nfs_nosys,	/* filler */
	nfs_nosys,
	nfs_nosys,
	nfs_nosys,
	nfs_nosys,
	nfs_nosys,
	nfs_nosys,
	nfs_nosys
};

/*
 * Initialize the vfs structure
 */

int nfsfstyp;

void
nfsinit(vswp, fstyp)
	struct vfssw *vswp;
	int fstyp;
{
	vswp->vsw_vfsops = &nfs_vfsops;
	nfsfstyp = fstyp;
}

/*
 * nfs mount vfsop
 * Set up mount info record and attach it to vfs struct.
 */
/*ARGSUSED*/
STATIC int
nfs_mount(vfsp, mvp, uap, cred)
	struct vfs *vfsp;
	struct vnode *mvp;
	struct mounta *uap;
	struct cred *cred;
{
	char *data = uap->dataptr;
	int datalen = uap->datalen;
	int error;
	struct vnode *rtvp = NULL;	/* the server's root */
	struct mntinfo *mi;		/* mount info, pointed at by vfs */
	fhandle_t fh;			/* root fhandle */
	struct nfs_args args;		/* nfs mount arguments */
	struct netbuf addr;		/* server's address */
	int hlen;			/* length of hostname */
	char shostname[HOSTNAMESZ];	/* server's hostname */
	int nlen;			/* length of netname */
	char netname[MAXNETNAMELEN+1];	/* server's netname */
	struct netbuf syncaddr;		/* AUTH_DES time sync addr */
	struct knetconfig *knconf;	/* transport knetconfig structure */

	if (!suser(cred))
		return (EPERM);
	/*
	 * For now, ignore remount option.
	 */
	if (vfsp->vfs_flag & VFS_REMOUNT)
		return (0);

	if (mvp->v_type != VDIR)
		return (ENOTDIR);

	/* make sure things are zeroed for errout: */
	bzero((caddr_t) &addr, sizeof (addr));
	bzero((caddr_t) &syncaddr, sizeof (syncaddr));

	/*
	 * get arguments
	 */
	if (datalen != sizeof (args))
		return (EINVAL);
	else
		if (copyin(data, (caddr_t)&args, sizeof (args)))
			return (EFAULT);

	/*
	 * A valid knetconfig structure is required.
	 */
	if (args.flags & NFSMNT_KNCONF) {
		/*
		 *	Allocate space for a knetconfig structure and
		 *	its strings and copy in from user-land.
		 */
		knconf = (struct knetconfig *)
		    kmem_alloc(sizeof (struct knetconfig), KM_SLEEP);
		if (copyin((caddr_t) args.knconf, (caddr_t) knconf,
			sizeof (struct knetconfig)) == -1) {
			kmem_free(knconf, sizeof (struct knetconfig));
			return (EFAULT);
		} else {
			size_t nmoved_tmp;
			char *p, *pf;

			pf = (char *) kmem_alloc(KNC_STRSIZE, KM_SLEEP);
			p = (char *) kmem_alloc(KNC_STRSIZE, KM_SLEEP);
			error = copyinstr((caddr_t) knconf->knc_protofmly, pf,
				KNC_STRSIZE, &nmoved_tmp);
			if (!error) {
				error = copyinstr((caddr_t) knconf->knc_proto,
				    p, KNC_STRSIZE, &nmoved_tmp);
				if (!error) {
					knconf->knc_protofmly = pf;
					knconf->knc_proto = p;
				} else {
					kmem_free(pf, KNC_STRSIZE);
					kmem_free(p, KNC_STRSIZE);
					kmem_free(knconf,
					    sizeof (struct knetconfig));
					return (error);
				}
			} else {
				kmem_free(pf, KNC_STRSIZE);
				kmem_free(p, KNC_STRSIZE);
				kmem_free(knconf, sizeof (struct knetconfig));
				return (error);
			}
		}
	} else
		return (EINVAL);

	/*
	 * Get server address
	 */
	if (copyin((caddr_t) args.addr, (caddr_t) &addr,
		sizeof (struct netbuf))) {
		addr.buf = (char *) NULL;
		error = EFAULT;
	} else {
		char *userbufptr = addr.buf;

		addr.buf = kmem_alloc(addr.len, KM_SLEEP);
		addr.maxlen = addr.len;
		if (copyin(userbufptr, addr.buf, addr.len)) {
			kmem_free(addr.buf, addr.len);
			addr.buf = (char *) NULL;
			error = EFAULT;
		}
	}
	if (error)
		goto errout;

	/*
	 * Get the root fhandle
	 */
	if (copyin((caddr_t)args.fh, (caddr_t)&fh, sizeof (fh))) {
		error = EFAULT;
		goto errout;
	}

	/*
	 * Get server's hostname
	 */
	if (args.flags & NFSMNT_HOSTNAME) {
		error = copyinstr(args.hostname, shostname,
			sizeof (shostname), (u_int *)&hlen);
		if (error) {
			goto errout;
		}
	} else
		(void) strncpy(shostname, "unknown-host", sizeof (shostname));


	if (args.flags & NFSMNT_SECURE) {
		/*
		 * If using AUTH_DES, get time sync netbuf ...
		 */
		if (args.syncaddr == (struct netbuf *) NULL)
			error = EINVAL;
		else {
			if (copyin((caddr_t) args.syncaddr, (caddr_t) &syncaddr,
				sizeof (struct netbuf))) {
				syncaddr.buf = (char *) NULL;
				error = EFAULT;
			} else {
				char *userbufptr = syncaddr.buf;

				syncaddr.buf = kmem_alloc(syncaddr.len,
				    KM_SLEEP);
				syncaddr.maxlen = syncaddr.len;
				if (copyin(userbufptr, syncaddr.buf,
					syncaddr.len)) {
					kmem_free(syncaddr.buf, syncaddr.len);
					syncaddr.buf = (char *) NULL;
					error = EFAULT;
				}
			}

			/*
			 * ... and server's netname
			 */
			if (!error)
				error = copyinstr(args.netname, netname,
					sizeof (netname), (u_int *) &nlen);
		}
	} else {
		nlen = -1;
	}
	if (error)
		goto errout;

	/*
	 * Get root vnode.
	 */
	error = nfsrootvp(&rtvp, vfsp, knconf, &addr, &syncaddr, &fh, shostname,
		netname, nlen, args.flags);
	if (error) {
		kmem_free(knconf->knc_protofmly, KNC_STRSIZE);
		kmem_free(knconf->knc_proto, KNC_STRSIZE);
		kmem_free(knconf, sizeof (struct knetconfig));
		if (addr.buf)
			kmem_free(addr.buf, addr.len);
		if (syncaddr.buf)
			kmem_free(syncaddr.buf, syncaddr.len);
		return (error);
	}

	/*
	 * Set option fields in mount info record
	 */
	/* LINTED pointer alignment */
	mi = vtomi(rtvp);
	mi->mi_noac = ((args.flags & NFSMNT_NOAC) != 0);
	mi->mi_nocto = ((args.flags & NFSMNT_NOCTO) != 0);
	if (args.flags & NFSMNT_RETRANS) {
		mi->mi_retrans = args.retrans;
		if (args.retrans < 0) {
			error = EINVAL;
			goto errout;
		}
	}
	if (args.flags & NFSMNT_TIMEO) {
		/*
		 * With dynamic retransmission, the mi_timeo is used only
		 * as a hint for the first one. The deviation is stored in
		 * units of hz shifted left by two, or 5msec. Since timeo
		 * was in units of 100msec, multiply by 20 to convert.
		 * rtxcur is in unscaled ticks, so multiply by 5.
		 */
		mi->mi_timeo = args.timeo;
		mi->mi_timers[3].rt_deviate = (args.timeo*HZ*2)/5;
		mi->mi_timers[3].rt_rtxcur = args.timeo*HZ/10;
		if (args.timeo <= 0) {
			error = EINVAL;
			goto errout;
		}
	}
	if (args.flags & NFSMNT_GRPID) {
		mi->mi_grpid = 1;
	}
	if (args.flags & NFSMNT_RSIZE) {
		if (args.rsize <= 0) {
			error = EINVAL;
			goto errout;
		}
		mi->mi_tsize = MIN(mi->mi_tsize, args.rsize);
		mi->mi_curread = mi->mi_tsize;
	}
	if (args.flags & NFSMNT_WSIZE) {
		if (args.wsize <= 0) {
			error = EINVAL;
			goto errout;
		}
		mi->mi_stsize = MIN(mi->mi_stsize, args.wsize);
		mi->mi_curwrite = mi->mi_stsize;
	}
	if (args.flags & NFSMNT_ACREGMIN) {
		if (args.acregmin < 0) {
			mi->mi_acregmin = ACMINMAX;
		} else if (args.acregmin == 0) {
			error = EINVAL;
			printf("nfs_mount: acregmin == 0\n");
			goto errout;
		} else {
			mi->mi_acregmin = MIN(args.acregmin, ACMINMAX);
		}
	}
	if (args.flags & NFSMNT_ACREGMAX) {
		if (args.acregmax < 0) {
			mi->mi_acregmax = ACMAXMAX;
		} else if (args.acregmax < mi->mi_acregmin) {
			error = EINVAL;
			printf("nfs_mount: acregmax < acregmin\n");
			goto errout;
		} else {
			mi->mi_acregmax = MIN(args.acregmax, ACMAXMAX);
		}
	}
	if (args.flags & NFSMNT_ACDIRMIN) {
		if (args.acdirmin < 0) {
			mi->mi_acdirmin = ACMINMAX;
		} else if (args.acdirmin == 0) {
			error = EINVAL;
			printf("nfs_mount: acdirmin == 0\n");
			goto errout;
		} else {
			mi->mi_acdirmin = MIN(args.acdirmin, ACMINMAX);
		}
	}
	if (args.flags & NFSMNT_ACDIRMAX) {
		if (args.acdirmax < 0) {
			mi->mi_acdirmax = ACMAXMAX;
		} else if (args.acdirmax < mi->mi_acdirmin) {
			error = EINVAL;
			printf("nfs_mount: acdirmax < acdirmin\n");
			goto errout;
		} else {
			mi->mi_acdirmax = MIN(args.acdirmax, ACMAXMAX);
		}
	}

#ifdef NFSDEBUG
	printf("nfs_mount: hard %d timeo %d retries %d wsize %d rsize %d\n",
	    mi->mi_hard, mi->mi_timeo, mi->mi_retrans, mi->mi_stsize,
	    mi->mi_tsize);
	printf("           regmin %d regmax %d dirmin %d dirmax %d\n",
	    mi->mi_acregmin, mi->mi_acregmax, mi->mi_acdirmin, mi->mi_acdirmax);
#endif

errout:
	if (error) {
		if (rtvp) {
			VN_RELE(rtvp);
		}
		kmem_free(knconf->knc_protofmly, KNC_STRSIZE);
		kmem_free(knconf->knc_proto, KNC_STRSIZE);
		kmem_free(knconf, sizeof (struct knetconfig));
		if (addr.buf)
			kmem_free(addr.buf, addr.len);
		if (syncaddr.buf)
			kmem_free(syncaddr.buf, syncaddr.len);
	}
	return (error);
}

STATIC int
nfsrootvp(rtvpp, vfsp, kp, addr, syncaddr, fh, shostname, netname, nlen, flags)
	struct vnode **rtvpp;		/* where to return root vp */
	register struct vfs *vfsp;	/* vfs of fs, if NULL make one */
	struct knetconfig *kp;		/* transport knetconfig structure */
	struct netbuf *addr;		/* server address */
	struct netbuf *syncaddr;	/* AUTH_DES time sync address */
	fhandle_t *fh;			/* swap file fhandle */
	char *shostname;		/* server's hostname */
	char *netname;			/* server's netname */
	int nlen;			/* length of netname, -1 if none */
	int flags;			/* mount flags */
{
	register struct vnode *rtvp = NULL;	/* the server's root */
	register struct mntinfo *mi = NULL;	/* mount info */
						/* pointed at by vfs */
	struct vattr va;		/* root vnode attributes */
	struct nfsfattr na;		/* root vnode attributes in nfs form */
	struct statvfs sb;		/* server's file system stats */
	register int error;
	/* struct ifaddr *ifa; */

	/*
	 * Create a mount record and link it to the vfs struct.
	 */
	mi = (struct mntinfo *)kmem_zalloc(sizeof (*mi), KM_SLEEP);
	mi->mi_hard = ((flags & NFSMNT_SOFT) == 0);
	mi->mi_int = ((flags & NFSMNT_INT) != 0);
	mi->mi_addr = *addr;
	if (flags & NFSMNT_SECURE)
		mi->mi_syncaddr = *syncaddr;
	mi->mi_knetconfig = kp;
	mi->mi_retrans = NFS_RETRIES;
	mi->mi_timeo = NFS_TIMEO;
	mi->mi_mntno = vfs_getnum(nfs_minmap, MAPSIZE);
	bcopy(shostname, mi->mi_hostname, HOSTNAMESZ);
	mi->mi_acregmin = ACREGMIN;
	mi->mi_acregmax = ACREGMAX;
	mi->mi_acdirmin = ACDIRMIN;
	mi->mi_acdirmax = ACDIRMAX;
	mi->mi_netnamelen = nlen;
	if (nlen >= 0) {
		mi->mi_netname = (char *)kmem_alloc((u_int)nlen, KM_SLEEP);
		bcopy(netname, mi->mi_netname, (u_int)nlen);
	}
	mi->mi_authflavor = (flags & NFSMNT_SECURE) ?  AUTH_DES : AUTH_UNIX;
	mi->mi_rpctimesync = (flags & NFSMNT_RPCTIMESYNC) ? 1 : 0;

	mi->mi_dynamic = 0;
	/*
	 * Make a vfs struct for nfs.  We do this here instead of below
	 * because rtvp needs a vfs before we can do a getattr on it.
	 */
	vfsp->vfs_fsid.val[0] = mi->mi_mntno;
	vfsp->vfs_fsid.val[1] = nfsfstyp;
	vfsp->vfs_data = (caddr_t)mi;
	vfsp->vfs_fstype = nfsfstyp;

	/*
	 * Make the root vnode, use it to get attributes,
	 * then remake it with the attributes.
	 */
	rtvp = makenfsnode(fh, (struct nfsfattr *)0, vfsp);
	if ((rtvp->v_flag & VROOT) != 0) {
		error = EINVAL;
		goto bad;
	}
	rtvp->v_flag |= VROOT;
	error = VOP_GETATTR(rtvp, &va, 0, u.u_cred);
	if (error)
		goto bad;
	VN_RELE(rtvp);
	vattr_to_nattr(&va, &na);
	rtvp = makenfsnode(fh, &na, vfsp);
	rtvp->v_flag |= VROOT;
	mi->mi_rootvp = rtvp;

	/*
	 * Get server's filesystem stats.  Use these to set transfer
	 * sizes, filesystem block size, and read-only.
	 */
	error = VFS_STATVFS(vfsp, &sb);
	if (error)
		goto bad;
	mi->mi_tsize = min(NFS_MAXDATA, (u_int)nfstsize());
	mi->mi_curread = mi->mi_tsize;

	/*
	 * Set filesystem block size to maximum data transfer size
	 */
	mi->mi_bsize = NFS_MAXDATA;
	vfsp->vfs_bsize = mi->mi_bsize;

	/*
	 * Need credentials in the rtvp so do_bio can find them.
	 */
	crhold(u.u_cred);
	/* LINTED pointer alignment */
	vtor(rtvp)->r_cred = u.u_cred;

	*rtvpp = rtvp;
	return (0);
bad:
	if (mi) {
		if (mi->mi_netnamelen >= 0) {
			kmem_free((caddr_t)mi->mi_netname,
				(u_int)mi->mi_netnamelen);
		}
		kmem_free((caddr_t)mi, sizeof (*mi));
	}
	if (rtvp) {
		VN_RELE(rtvp);
	}
	*rtvpp = NULL;
	return (error);
}

/*
 * vfs operations
 */
STATIC int
nfs_unmount(vfsp, cr)
	struct vfs *vfsp;
	struct cred *cr;
{
	/* LINTED pointer alignment */
	struct mntinfo *mi = (struct mntinfo *)vfsp->vfs_data;

	if (!suser(cr))
		return (EPERM);

#ifdef NFSDEBUG
	printf("nfs_unmount(%x) mi = %x\n", vfsp, mi);
#endif
	rflush(vfsp);
	rinval(vfsp);

	if (mi->mi_refct != 1 || mi->mi_rootvp->v_count != 1) {
		return (EBUSY);
	}
	/*
	 *	Release root vnode -- but first manually remove its identity
	 *	and pages, since VN_RELE() might decide to hang onto them
	 */
	/* LINTED pointer alignment */
	rp_rmhash(vtor(mi->mi_rootvp));
	/* LINTED pointer alignment */
	rinactive(vtor(mi->mi_rootvp));
	VN_RELE(mi->mi_rootvp);
	vfs_putnum(nfs_minmap, mi->mi_mntno);
	if (mi->mi_netnamelen >= 0) {
		kmem_free((caddr_t)mi->mi_netname, (u_int)mi->mi_netnamelen);
	}
	kmem_free(mi->mi_addr.buf, mi->mi_addr.len);
	if (mi->mi_authflavor == AUTH_DES)
		kmem_free(mi->mi_syncaddr.buf, mi->mi_syncaddr.len);
	kmem_free(mi->mi_knetconfig->knc_protofmly, KNC_STRSIZE);
	kmem_free(mi->mi_knetconfig->knc_proto, KNC_STRSIZE);
	kmem_free(mi->mi_knetconfig, sizeof (struct knetconfig));
	kmem_free((caddr_t)mi, sizeof (*mi));
	return (0);
}

/*
 * find root of nfs
 */
STATIC int
nfs_root(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{

	/* LINTED pointer alignment */
	*vpp = ((struct mntinfo *)vfsp->vfs_data)->mi_rootvp;
	(*vpp)->v_count++;
#ifdef NFSDEBUG
	printf("nfs_root(0x%x) = %x\n", vfsp, *vpp);
#endif
	return (0);
}

/*
 * Get file system statistics.
 */
STATIC int
nfs_statvfs(vfsp, sbp)
	register struct vfs *vfsp;
	struct statvfs *sbp;
{
	struct nfsstatfs fs;
	struct mntinfo *mi;
	fhandle_t *fh;
	int error = 0;

	/* LINTED pointer alignment */
	mi = vftomi(vfsp);
	/* LINTED pointer alignment */
	fh = vtofh(mi->mi_rootvp);
#ifdef NFSDEBUG
	printf("nfs_statvfs vfs %x\n", vfsp);
#endif
	error = rfscall(mi, RFS_STATFS, 0, xdr_fhandle,
	    (caddr_t)fh, xdr_statfs, (caddr_t)&fs, u.u_cred);
	if (!error) {
		error = geterrno(fs.fs_status);
	}
	if (!error) {
		if (mi->mi_stsize) {
			mi->mi_stsize = MIN(mi->mi_stsize, fs.fs_tsize);
		} else {
			mi->mi_stsize = fs.fs_tsize;
			mi->mi_curwrite = mi->mi_stsize;
		}
		sbp->f_bsize = fs.fs_bsize;
		sbp->f_frsize = fs.fs_bsize;
		sbp->f_blocks = fs.fs_blocks;
		sbp->f_bfree = fs.fs_bfree;
		sbp->f_bavail = fs.fs_bavail;
		sbp->f_files = (u_long)-1;
		sbp->f_ffree = (u_long)-1;
		sbp->f_favail = (u_long)-1;
		/*
		 *	XXX - This is wrong, should be a real fsid
		 */
		bcopy((caddr_t)&vfsp->vfs_fsid, (caddr_t)&sbp->f_fsid,
		    sizeof (fsid_t));
		strncpy(sbp->f_basetype, vfssw[vfsp->vfs_fstype].vsw_name,
			FSTYPSZ);
		sbp->f_flag = vf_to_stf(vfsp->vfs_flag);
		sbp->f_namemax = (u_long)-1;
	}
#ifdef NFSDEBUG
	printf("nfs_statvfs returning %d\n", error);
#endif
	return (error);
}

/*
 * Flush dirty nfs files for file system vfsp.
 * If vfsp == NULL, all nfs files are flushed.
 */
/*ARGSUSED*/
STATIC int
nfs_sync(vfsp, flag, cr)
	struct vfs *vfsp;
	short flag;
	struct cred *cr;
{
	static int nfslock;

	if (nfslock == 0 && !(flag & SYNC_ATTR)) {
#ifdef NFSDEBUG
		printf("nfs_sync\n");
#endif
		nfslock++;
		rflush(vfsp);
		nfslock = 0;
	}
	return (0);
}

/*ARGSUSED*/
STATIC int
nfs_vget(vfsp, vpp, fidp)
	vfs_t	*vfsp;
	vnode_t	**vpp;
	fid_t	*fidp;
{
	cmn_err (CE_WARN, "nfs_vget called\n");
	return (ENOSYS);
}

/*ARGSUSED*/
STATIC int
nfs_mountroot(vfsp, why)
	vfs_t		*vfsp;
	whymountroot_t	why;
{
	cmn_err (CE_WARN, "nfs_mountroot called\n");
	return (ENOSYS);
}

/*ARGSUSED*/
STATIC int
nfs_swapvp(vfsp, vpp, nm)
	vfs_t	*vfsp;
	vnode_t	**vpp;
	char	*nm;
{
	cmn_err (CE_WARN, "nfs_swapvp called\n");
	return (ENOSYS);
}

STATIC int
nfs_nosys()
{
	return (ENOSYS);
}
