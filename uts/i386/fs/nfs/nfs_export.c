/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfs_export.c	1.3"

/*      @(#)nfs_export.c 1.11 88/02/08 SMI      */

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

#define NFSSERVER

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/tiuser.h>
#include <sys/kmem.h>
#include <sys/pathname.h>
#include <sys/debug.h>
#include <netinet/in.h>
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#include <rpc/auth_des.h>
#include <nfs/nfs.h>
#include <nfs/export.h>

STATIC void	freenames();
STATIC void exportfree();

#define eqfsid(fsid1, fsid2)	\
	(bcmp((char *)fsid1, (char *)fsid2, (int)sizeof(fsid_t)) == 0)

#define eqfid(fid1, fid2) \
	((fid1)->fid_len == (fid2)->fid_len && \
	bcmp((char *)(fid1)->fid_data, (char *)(fid2)->fid_data,  \
	(int)(fid1)->fid_len) == 0)

#define exportmatch(exi, fsid, fid) \
	(eqfsid(&(exi)->exi_fsid, fsid) && eqfid((exi)->exi_fid, fid))

#ifdef NFSDEBUG 
extern int nfsdebug; 
#endif

struct exportinfo *exported;	/* the list of exported filesystems */
struct exportfs_args {
	char	*dname;
	struct export	*uex;
};


/*
 * Exportfs system call
 */
exportfs(uap)
	register struct exportfs_args *uap;
{
	struct vnode *vp;
	struct export *kex;
	struct exportinfo **tail;
	struct exportinfo *exi;
	struct exportinfo *tmp;
	struct fid *fid;
	struct vfs *vfs;
	int mounted_ro;
	int error;

	if (! suser(u.u_cred))
		return (EPERM);

	/*
	 * Get the vfs id
	 */
	error = lookupname(uap->dname, UIO_USERSPACE, FOLLOW, 
		(struct vnode **) NULL, &vp);
	if (error)
		return (error);	
	error = VOP_FID(vp, &fid);
	vfs = vp->v_vfsp;
	mounted_ro = vp->v_vfsp->vfs_flag & VFS_RDONLY;
	VN_RELE(vp);
	if (error)
		return (error);	

	if (uap->uex == NULL) {
		error = unexport(&vfs->vfs_fsid, fid);
		freefid(fid);
		return (error);
	}
	exi = (struct exportinfo *) mem_alloc(sizeof(struct exportinfo));
	exi->exi_fsid  = vfs->vfs_fsid;
	exi->exi_fid = fid;
	kex = &exi->exi_export;

	/*
	 * Load in everything, and do sanity checking
	 */	
	if (copyin((caddr_t) uap->uex, (caddr_t) kex, 
		(u_int) sizeof(struct export))) {
		error = EFAULT;
		goto error_return;
	}
	if (kex->ex_flags & ~EX_ALL) {
		error = EINVAL;
		goto error_return;
	}
	if (!(kex->ex_flags & EX_RDONLY) && mounted_ro) {
		error = EROFS;
		goto error_return;
	}
	if (kex->ex_flags & EX_EXCEPTIONS) {
		error = loadaddrs(&kex->ex_roaddrs);
		if (error)
			goto error_return;
		error = loadaddrs(&kex->ex_rwaddrs);
		if (error)
			goto error_return;
	}
	switch (kex->ex_auth) {
	case AUTH_UNIX:
		error = loadaddrs(&kex->ex_unix.rootaddrs);
		break;
	case AUTH_DES:
		error = loadrootnames(kex);
		break;
	default:
		error = EINVAL;
	}
	if (error) {	
		goto error_return;
	}

	/*
	 * Commit the new information to the export list, making
	 * sure to delete the old entry for the fs, if one exists.
	 */
	tail = &exported;
	while (*tail != NULL) {
		if (exportmatch(*tail, &exi->exi_fsid, exi->exi_fid)) {
			tmp = *tail;
			*tail = (*tail)->exi_next;
			exportfree(tmp);
		} else {
			tail = &(*tail)->exi_next;
		}
	}
	exi->exi_next = NULL;
	*tail = exi;
	return (0);

error_return:	
	freefid(exi->exi_fid);
	mem_free((char *) exi, sizeof(struct exportinfo));

	return (error);
}


/*
 * Remove the exported directory from the export list
 */
unexport(fsid, fid)
	fsid_t *fsid;
	struct fid *fid;
{
	struct exportinfo **tail;	
	struct exportinfo *exi;

	tail = &exported;
	while (*tail != NULL) {
		if (exportmatch(*tail, fsid, fid)) {
			exi = *tail;
			*tail = (*tail)->exi_next;
			exportfree(exi);
			return (0);
		} else {
			tail = &(*tail)->exi_next;
		}
	}
	return (EINVAL);
}

struct nfs_getfh_args {
	char	*fname;
	fhandle_t	*fhp;
};

/*
 * Get file handle system call.
 * Takes file name and returns a file handle for it.
 */
nfs_getfh(uap)
	register struct nfs_getfh_args *uap;
{
	fhandle_t fh;
	struct vnode *vp;
	struct vnode *dvp;
	struct exportinfo *exi;	
	int error;

	if (!suser(u.u_cred))
		return (EPERM);

	error = lookupname(uap->fname, UIO_USERSPACE, FOLLOW, 
			       &dvp, &vp);
	if (error == EINVAL) {
		/*
		 * if fname resolves to / we get EINVAL error
		 * since we wanted the parent vnode. Try again
		 * with NULL dvp.
		 */
		error = lookupname(uap->fname, UIO_USERSPACE,
			FOLLOW, (struct vnode **)NULL, &vp);
		dvp = NULL;
	}
	if (error == 0 && vp == NULL) {
		/*
		 * Last component of fname not found
		 */
		if (dvp) {
			VN_RELE(dvp);
		}
		error = ENOENT;
	}
	if (error)
		return (error);
	error = findexivp(&exi, dvp, vp);
	if (!error) {
		error = makefh(&fh, vp, exi);
		if (!error) {
			if (copyout((caddr_t)&fh, (caddr_t)uap->fhp, 
					sizeof(fh)))
				error = EFAULT;
		}
	}
	VN_RELE(vp);
	if (dvp != NULL) {
		VN_RELE(dvp);
	}
	return (error);
}

/*
 * Strategy: if vp is in the export list, then
 * return the associated file handle. Otherwise, ".."
 * once up the vp and try again, until the root of the
 * filesystem is reached.
 */
findexivp(exip, dvp, vp)
	struct exportinfo **exip;
	struct vnode *dvp;  /* parent of vnode want fhandle of */
	struct vnode *vp;   /* vnode we want fhandle of */
{
	struct fid *fid;
	int error;

	VN_HOLD(vp);
	if (dvp != NULL) {
		VN_HOLD(dvp);
	}
	for (;;) {
		error = VOP_FID(vp, &fid);
		if (error) {
			break;
		}
		*exip = findexport(&vp->v_vfsp->vfs_fsid, fid); 
		freefid(fid);
		if (*exip != NULL) {
			/*
			 * Found the export info
			 */
			error = 0;
			break;
		}

		/*
		 * We have just failed finding a matching export.
		 * If we're at the root of this filesystem, then
		 * it's time to stop (with failure).
		 */
		if (vp->v_flag & VROOT) {
			error = EINVAL;
			break;	
		}

		/*
		 * Now, do a ".." up vp. If dvp is supplied, use it,
	 	 * otherwise, look it up.
		 */
		if (dvp == NULL) {
			error = VOP_LOOKUP(vp, "..", &dvp,
					(struct pathname *)NULL, 0,
					(struct vnode *) 0,	/* XXX - unused? */
					u.u_cred);
			if (error) {
				break;
			}
		}
		VN_RELE(vp);
		vp = dvp;
		dvp = NULL;
	}
	VN_RELE(vp);
	if (dvp != NULL) {
		VN_RELE(dvp);
	}
	return (error);
}

/*
 * Make an fhandle from a vnode
 */
makefh(fh, vp, exi)
	register fhandle_t *fh;
	struct vnode *vp;
	struct exportinfo *exi;
{
	struct fid *fidp;
	int error;

	error = VOP_FID(vp, &fidp);
	if (error || fidp == NULL) {
		/*
		 * Should be something other than EREMOTE
		 */
		return (EREMOTE);
	}
	if (fidp->fid_len + exi->exi_fid->fid_len + sizeof(fsid_t) 
		> NFS_FHSIZE) {
		freefid(fidp);
		return (EREMOTE);
	}
	bzero((caddr_t) fh, sizeof(*fh));
	fh->fh_fsid.val[0] = vp->v_vfsp->vfs_fsid.val[0];
	fh->fh_fsid.val[1] = vp->v_vfsp->vfs_fsid.val[1];
	fh->fh_len = fidp->fid_len;
	bcopy(fidp->fid_data, fh->fh_data, fidp->fid_len);
	fh->fh_xlen = exi->exi_fid->fid_len;
	bcopy(exi->exi_fid->fid_data, fh->fh_xdata, fh->fh_xlen);
#ifdef NFSDEBUG
/*
	printf("makefh: vp %x fsid %x %x len %d data %d %d\n",
		vp, fh->fh_fsid.val[0], fh->fh_fsid.val[1], fh->fh_len,
		*(int *)fh->fh_data, *(int *)&fh->fh_data[sizeof(int)]);
*/
#endif
	freefid(fidp);
	return (0);
}

/*
 * Find the export structure associated with the given filesystem
 */
struct exportinfo *
findexport(fsid, fid)
	fsid_t *fsid;	
	struct fid *fid;
{
	struct exportinfo *exi;

	for (exi = exported; exi != NULL; exi = exi->exi_next) {
		if (exportmatch(exi, fsid, fid)) {
			return (exi);
		}
	}
	return (NULL);
}

/*
 * Load from user space a list of exception addresses and masks
 */
loadaddrs(addrs)
	struct exaddrlist *addrs;
{
	char *tmp;
	int allocsize;
	register int i;
	struct netbuf *uaddrs;
	struct netbuf *umasks;

	if (addrs->naddrs > EXMAXADDRS)
		return (EINVAL);
	if (addrs->naddrs == 0)
		return (0);

	allocsize = addrs->naddrs * sizeof(struct netbuf);
	uaddrs = addrs->addrvec;
	umasks = addrs->addrmask;

	addrs->addrvec = (struct netbuf *) mem_alloc(allocsize);
	if (copyin((caddr_t)uaddrs, (caddr_t)addrs->addrvec, (u_int)allocsize)) {
		mem_free((char *)addrs->addrvec, allocsize);
		return (EFAULT);
	}

	addrs->addrmask = (struct netbuf *) mem_alloc(allocsize);
	if (copyin((caddr_t)umasks, (caddr_t)addrs->addrmask, (u_int)allocsize)) {
		mem_free((char *)addrs->addrmask, allocsize);
		mem_free((char *)addrs->addrvec, allocsize);
		return (EFAULT);
	}

	for (i = 0; i < addrs->naddrs; i++) {
		tmp = (char *) mem_alloc(addrs->addrvec[i].len);
		if (copyin(addrs->addrvec[i].buf, tmp, (u_int) addrs->addrvec[i].len)) {
			register int j;

			for (j = 0; j < i; j++)
				mem_free((char *) addrs->addrvec[j].buf, addrs->addrvec[j].len);
			mem_free(tmp, addrs->addrvec[i].len);
			mem_free((char *)addrs->addrmask, allocsize);
			mem_free((char *)addrs->addrvec, allocsize);
			return (EFAULT);
		}
		else
			addrs->addrvec[i].buf = tmp;
	}

	for (i = 0; i < addrs->naddrs; i++) {
		tmp = (char *) mem_alloc(addrs->addrmask[i].len);
		if (copyin(addrs->addrmask[i].buf, tmp, (u_int) addrs->addrmask[i].len)) {
			register int j;

			for (j = 0; j < i; j++)
				mem_free((char *) addrs->addrmask[j].buf, addrs->addrmask[j].len);
			mem_free(tmp, addrs->addrmask[i].len);
			for (j = 0; j < addrs->naddrs; j++)
				mem_free((char *) addrs->addrvec[j].buf, addrs->addrvec[j].len);
			mem_free((char *)addrs->addrmask, allocsize);
			mem_free((char *)addrs->addrvec, allocsize);
			return (EFAULT);
		}
		else
			addrs->addrmask[i].buf = tmp;
	}
	return (0);
}

/*
 * Load from user space the root user names into kernel space
 * (AUTH_DES only)
 */
loadrootnames(kex)
	struct export *kex;
{
	int error;
	char *exnames[EXMAXROOTNAMES];
	int i;
	u_int len;
	char netname[MAXNETNAMELEN+1];
	u_int allocsize;

	if (kex->ex_des.nnames > EXMAXROOTNAMES)
		return (EINVAL);
	if (kex->ex_des.nnames == 0)
		return (0);

	/*
	 * Get list of names from user space
	 */
	allocsize =  kex->ex_des.nnames * sizeof(char *);
	if (copyin((char *)kex->ex_des.rootnames, (char *)exnames, allocsize))
		return (EFAULT);
	kex->ex_des.rootnames = (char **) mem_alloc(allocsize);
	bzero((char *) kex->ex_des.rootnames, allocsize);

	/*
	 * And now copy each individual name
	 */
	for (i = 0; i < kex->ex_des.nnames; i++) {
		error = copyinstr(exnames[i], netname, sizeof(netname), &len);
		if (error) {
			goto freeup;
		}
		kex->ex_des.rootnames[i] = mem_alloc(len + 1);
		bcopy(netname, kex->ex_des.rootnames[i], len);
		kex->ex_des.rootnames[i][len] = 0;
	}
	return (0);

freeup:
	freenames(kex);
	return (error);
}

/*
 * Figure out everything we allocated in a root user name list in
 * order to free it up. (AUTH_DES only)
 */
STATIC void
freenames(ex)
	struct export *ex;
{
	int i;

	for (i = 0; i < ex->ex_des.nnames; i++) {
		if (ex->ex_des.rootnames[i] != NULL) {
			mem_free((char *) ex->ex_des.rootnames[i],
				strlen(ex->ex_des.rootnames[i]) + 1);
		}
	}	
	mem_free((char *) ex->ex_des.rootnames, ex->ex_des.nnames * sizeof(char *));
}


/*
 * Free an entire export list node
 */
STATIC void
exportfree(exi)
	struct exportinfo *exi;
{
	register int i;
	struct export *ex;

	ex = &exi->exi_export;
	switch (ex->ex_auth) {
	case AUTH_UNIX:
		for (i = 0; i < ex->ex_unix.rootaddrs.naddrs; i++) {
			mem_free(ex->ex_unix.rootaddrs.addrvec[i].buf, ex->ex_unix.rootaddrs.addrvec[i].len);
			mem_free(ex->ex_unix.rootaddrs.addrmask[i].buf, ex->ex_unix.rootaddrs.addrmask[i].len);
		}
		mem_free((char *)ex->ex_unix.rootaddrs.addrvec,
			 (ex->ex_unix.rootaddrs.naddrs *
			  sizeof(struct netbuf)));
		mem_free((char *)ex->ex_unix.rootaddrs.addrmask,
			 (ex->ex_unix.rootaddrs.naddrs *
			  sizeof(struct netbuf)));
		break;
	case AUTH_DES:
		freenames(ex);
		break;
	}
	if (ex->ex_flags & EX_EXCEPTIONS) {
		for (i = 0; i < ex->ex_roaddrs.naddrs; i++) {
			mem_free(ex->ex_roaddrs.addrvec[i].buf,
				 ex->ex_roaddrs.addrvec[i].len);
			mem_free(ex->ex_roaddrs.addrmask[i].buf,
				 ex->ex_roaddrs.addrmask[i].len);
		}
		mem_free((char *)ex->ex_roaddrs.addrvec,
			 ex->ex_roaddrs.naddrs * sizeof(struct netbuf));
		mem_free((char *)ex->ex_roaddrs.addrmask,
			 ex->ex_roaddrs.naddrs * sizeof(struct netbuf));

		for (i = 0; i < ex->ex_rwaddrs.naddrs; i++) {
			mem_free(ex->ex_rwaddrs.addrvec[i].buf,
				 ex->ex_rwaddrs.addrvec[i].len);
			mem_free(ex->ex_rwaddrs.addrmask[i].buf,
				 ex->ex_rwaddrs.addrmask[i].len);
		}
		mem_free((char *)ex->ex_rwaddrs.addrvec,
			 ex->ex_rwaddrs.naddrs * sizeof(struct netbuf));
		mem_free((char *)ex->ex_rwaddrs.addrmask,
			 ex->ex_rwaddrs.naddrs * sizeof(struct netbuf));
	}
	freefid(exi->exi_fid);
	mem_free(exi, sizeof(struct exportinfo));
}
