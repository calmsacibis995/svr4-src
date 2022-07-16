/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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

#ident  "@(#)kern-fs:specfs/specvnops.c	1.3.3.13"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/file.h"
#include "sys/immu.h"
#include "sys/kmem.h"
#include "sys/mman.h"
#include "sys/open.h"
#include "sys/swap.h"
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/poll.h"
#include "sys/stream.h"
#include "sys/strsubr.h"

#include "sys/proc.h"
#include "sys/user.h"
#include "sys/session.h"

#include "sys/fs/snode.h"

#include "sys/vmparam.h"
#include "vm/seg.h"
#include "vm/seg_map.h"
#include "vm/page.h"
#include "vm/pvn.h"
#include "vm/seg_dev.h"
#include "vm/seg_vn.h"

#include "fs/fs_subr.h"

extern int nodev();

STATIC int spec_open();
STATIC int spec_close();
STATIC int spec_read();
STATIC int spec_write();
STATIC int spec_ioctl();
STATIC int spec_getattr();
STATIC int spec_setattr();
STATIC int spec_access();
STATIC int spec_fsync();
STATIC void spec_inactive();
STATIC int spec_fid();
STATIC int spec_seek();
STATIC int spec_frlock();
STATIC int spec_realvp();
STATIC int spec_getpage();
extern int spec_putpage();
STATIC int spec_map();
STATIC int spec_addmap();
STATIC int spec_delmap();
STATIC int spec_poll();
STATIC int spec_allocstore();

extern int spec_segmap();

struct vnodeops spec_vnodeops = {
	spec_open,
	spec_close,
	spec_read,
	spec_write,
	spec_ioctl,
	fs_setfl,
	spec_getattr,
	spec_setattr,
	spec_access,
	fs_nosys,	/* lookup */
	fs_nosys,	/* create */
	fs_nosys,	/* remove */
	fs_nosys,	/* link */
	fs_nosys,	/* rename */
	fs_nosys,	/* mkdir */
	fs_nosys,	/* rmdir */
	fs_nosys,	/* readdir */
	fs_nosys,	/* symlink */
	fs_nosys,	/* readlink */
	spec_fsync,
	spec_inactive,
	spec_fid,
	fs_rwlock,
	fs_rwunlock,
	spec_seek,
	fs_cmp,
	spec_frlock,
	fs_nosys,	/* space */
	spec_realvp,
	spec_getpage,
	spec_putpage,
	spec_map,
	spec_addmap,
	spec_delmap,
	spec_poll,
	fs_nosys,	/* dump */
	fs_pathconf,
	spec_allocstore,
	fs_nosys,	/* filler */
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
};

/*
 * Allow handler of special files to initialize and validate before
 * actual I/O.
 */
STATIC int
spec_open(vpp, flag, cr)
	struct vnode **vpp;
	int flag;
	struct cred *cr;
{
	register unsigned int maj;
	register dev_t dev;
	dev_t newdev;
	o_pid_t *oldttyp;
	struct vnode *nvp;
	struct vnode *vp = *vpp;
	struct vnode *cvp = VTOS(vp)->s_commonvp;
	int error = 0;
	label_t	saveq;

	flag &= ~FCREAT;		/* paranoia */
	saveq = u.u_qsav;
	/*
	 * Catch half-opens.
	 */
	if (setjmp(&u.u_qsav)) {
		(void) spec_close(vp, flag & FMASK, 1, (off_t) 0, cr);
		u.u_qsav = saveq;
		return EINTR;
	}
	VTOS(vp)->s_count++;	/* one more open reference */
	VTOS(cvp)->s_count++;	/* one more open reference */
	dev = vp->v_rdev;
	newdev = dev;

	ASSERT(vp->v_type == VCHR || vp->v_type == VBLK);

	switch (vp->v_type) {

	case VCHR:
		if ((maj = getmajor(dev)) >= cdevcnt) {
			error = ENXIO;
			break;
		}
		oldttyp = u.u_ttyp; 	/* used only by old tty drivers */
		if (cdevsw[maj].d_str) {
			if ((error = stropen(cvp, &newdev, flag, cr)) == 0) {
				struct stdata *stp = cvp->v_stream;
				if (dev != newdev) {
					/*
					 * Clone open.
					 */
					vp->v_stream = NULL;
					cvp->v_stream = NULL;
					if ((nvp = makespecvp(newdev, VCHR))
					  == NULL) {
						vp->v_stream = stp;
						cvp->v_stream = stp;
						strclose(vp, flag, cr);
						error = ENOMEM;
						break;
					}
					/*
					 * STREAMS clones inherit fsid and
					 * stream.
					 */
					VTOS(nvp)->s_fsid = VTOS(vp)->s_fsid;
					VTOS(vp)->s_count--;
					VTOS(cvp)->s_count--;
					nvp->v_vfsp = vp->v_vfsp;
					nvp->v_stream = stp;
					cvp = VTOS(nvp)->s_commonvp;
					cvp->v_stream = stp;
					stp->sd_vnode = cvp;
					stp->sd_strtab =
					  cdevsw[getmajor(newdev)].d_str;
					VN_RELE(vp);
					VTOS(nvp)->s_count++;
					VTOS(cvp)->s_count++;
					*vpp = nvp;
				} else {
					/*
					 * Normal open.
					 */
					vp->v_stream = stp;
				}
        			if (oldttyp == NULL && u.u_ttyp != NULL) {
					/* 
					 * pre SVR4 driver has allocated the
					 * stream as a controlling terminal -
					 * check against SVR4 criteria and
					 * deallocate it if it fails
					 */
					if ((flag&FNOCTTY) 
					  || !strctty(u.u_procp, stp)) {
						*u.u_ttyp = 0;
						u.u_ttyp = NULL;
					}
				} else if (stp->sd_flag & STRISTTY) {
					/*
					 * this is a post SVR4 tty driver -
					 * try to allocate it as a
					 * controlling terminal
					 */	
					if (!(flag&FNOCTTY))
						(void)strctty(u.u_procp, stp);
				}
			}
		} else {
			if ((error = (*cdevsw[maj].d_open)
			  (&newdev, flag, OTYP_CHR, cr)) == 0
			    && dev != newdev) {
				/*
				 * Clone open.
				 */
				if ((nvp = makespecvp(newdev, VCHR)) == NULL) {
					(*cdevsw[getmajor(newdev)].d_close)
					  (newdev, flag, OTYP_CHR, cr);
					error = ENOMEM;
					break;
				}

				/*
				 * Character clones inherit fsid.
				 */
				VTOS(nvp)->s_fsid = VTOS(vp)->s_fsid;
				VTOS(vp)->s_count--;
				VTOS(cvp)->s_count--;
				nvp->v_vfsp = vp->v_vfsp;
				cvp = VTOS(nvp)->s_commonvp;
				VN_RELE(vp);
				VTOS(nvp)->s_count++;
				VTOS(cvp)->s_count++;
				*vpp = nvp;
			}
			if (oldttyp == NULL && u.u_ttyp != NULL) {
                                register proc_t *pp = u.u_procp;
                                register sess_t *sp = pp->p_sessp;
				if ((flag&FNOCTTY) || pp->p_pid != sp->s_sid) {
                                        *u.u_ttyp = 0;
                                        u.u_ttyp = NULL;
                                } else {
					extern vnode_t *makectty();
                                        alloctty(pp, 
					  makectty(VTOS(*vpp)->s_commonvp));
                                        u.u_ttyd = (o_dev_t)cmpdev(sp->s_dev);
                                }
                        }
		}
		break;

	case VBLK:
		newdev = dev;
		if ((maj = getmajor(dev)) >= bdevcnt) {
			error = ENXIO;
			break;
		}
		if ((error = (*bdevsw[maj].d_open)(&newdev, flag,
                    OTYP_BLK, cr)) == 0 && dev != newdev) {
			/*
			 * Clone open.
			 */
			if ((nvp = makespecvp(newdev, VBLK)) == NULL) {
				(*bdevsw[getmajor(newdev)].d_close)
				  (newdev, flag, OTYP_BLK, cr);
				error = ENOMEM;
				break;
			}

			/*
			 * Block clones inherit fsid.
			 */
			VTOS(nvp)->s_fsid = VTOS(vp)->s_fsid;
			VTOS(vp)->s_count--;
			VTOS(cvp)->s_count--;
			nvp->v_vfsp = vp->v_vfsp;
			cvp = VTOS(nvp)->s_commonvp;
			VN_RELE(vp);
			VTOS(nvp)->s_count++;
			VTOS(cvp)->s_count++;
			*vpp = nvp;
		}
		break;

	default:
		cmn_err(CE_PANIC, "spec_open: type not VCHR or VBLK\n");
		break;
	}
done:
	if (error != 0) {
		VTOS(*vpp)->s_count--;	/* one less open reference */
		VTOS(cvp)->s_count--;   /* one less open reference */
	}
	u.u_qsav = saveq;
	return error;
}

/* ARGSUSED */
STATIC int
spec_close(vp, flag, count, offset, cr)
	struct vnode *vp;
	int flag;
	int count;
	off_t offset;
	struct cred *cr;
{
	register struct snode *sp;
	enum vtype type;
	dev_t dev;
	register struct vnode *cvp = VTOS(vp)->s_commonvp;
	int error = 0;
	label_t	saveq;

	extern	void    raioclose();

	cleanlocks(vp, u.u_procp->p_epid, u.u_procp->p_sysid);
	if (vp->v_stream)
		strclean(vp);
	ASSERT(count >= 1);
	if (count > 1)
		return 0;

	saveq = u.u_qsav;
	if (setjmp(&u.u_qsav)) {	/* catch half-closes */
		u.u_qsav = saveq;
		return EINTR;
	}

	sp = VTOS(vp);
	sp->s_count--;		/* one fewer open reference */
	VTOS(cvp)->s_count--;	/* one fewer open reference */

	dev = sp->s_dev;
	type = vp->v_type;

	ASSERT(type == VCHR || type == VBLK);

	if ((type == VCHR) && u.u_procp->p_raiocnt) 
		(void)raioclose(dev);
	/*
	 * Only call the close routine when the last open reference through
	 * any [s,v]node goes away.
	 */
	if (!stillreferenced(dev, type))
		error = device_close(vp, flag, cr);
	u.u_qsav = saveq;
	return error;
}

/* ARGSUSED */
STATIC int
spec_read(vp, uiop, ioflag, cr)
	register struct vnode *vp;
	register struct uio *uiop;
	int ioflag;
	struct cred *cr;
{
	int error;
	struct snode *sp = VTOS(vp);
	register dev_t dev = sp->s_dev;
	register unsigned on, n;
	unsigned long bdevsize;
	off_t off;
	struct vnode *blkvp;

	if (uiop->uio_resid == 0)
		return 0;

	ASSERT(vp->v_type == VCHR || vp->v_type == VBLK);

	if (vp->v_type == VCHR) {
		smark(sp, SACC);
		if (cdevsw[getmajor(dev)].d_str)
			error = strread(vp, uiop, cr);
		else 
			error = (*cdevsw[getmajor(dev)].d_read)(dev, uiop, cr);
		return error;
	}

	/*
	 * Block device.
	 */
	error = 0;
	blkvp = sp->s_commonvp;
	bdevsize = sp->s_size;
	do {
		int diff;
		caddr_t base;

		off = uiop->uio_offset & MAXBMASK;
		on = uiop->uio_offset & MAXBOFFSET;
		n = MIN(MAXBSIZE - on, uiop->uio_resid);
		diff = bdevsize - uiop->uio_offset;

		if (diff <= 0) 
			break;
		if (diff < n)
			n = diff;

		base = segmap_getmap(segkmap, blkvp, off);

		if ((error = uiomove(base + on, n, UIO_READ, uiop)) == 0) {
			int flags = 0;
			/*
			 * If we read a whole block, we won't need this
			 * buffer again soon.  Don't mark it with
			 * SM_FREE, as that can lead to a deadlock
			 * if the block corresponds to a u-page.
			 * (The keep count never drops to zero, so
			 * waiting for "i/o to complete" never
			 * terminates; this points out a flaw in
			 * our locking strategy.)
			 */
			if (n + on == MAXBSIZE)
				flags = SM_DONTNEED;
			error = segmap_release(segkmap, base, flags);
		} else {
			(void) segmap_release(segkmap, base, 0);
			if (bdevsize == UNKNOWN_SIZE) {
				error = 0;
				break;
			}
		}
	} while (error == 0 && uiop->uio_resid > 0 && n != 0);

	return error;
}

/* ARGSUSED */
STATIC int
spec_write(vp, uiop, ioflag, cr)
	struct vnode *vp;
	register struct uio *uiop;
	int ioflag;
	struct cred *cr;
{
	int error;
	struct snode *sp = VTOS(vp);
	register dev_t dev = sp->s_dev;
	register unsigned n, on;
	unsigned long bdevsize;
	off_t off;
	struct vnode *blkvp;
	page_t *iolpl[MAXBSIZE/PAGESIZE + 2];
	page_t **ppp;

	ASSERT(vp->v_type == VCHR || vp->v_type == VBLK);

	if (vp->v_type == VCHR) {
		smark(sp, SUPD|SCHG);
		if (cdevsw[getmajor(dev)].d_str)
			error = strwrite(vp, uiop, cr);
		else
			error = (*cdevsw[getmajor(dev)].d_write)(dev, uiop, cr);
		return error;
	}

	if (uiop->uio_resid == 0)
		return 0;

	error = 0;
	blkvp = sp->s_commonvp;
	bdevsize = sp->s_size;
	do {
		int diff, pagecreate;
		caddr_t base;

		off = uiop->uio_offset & MAXBMASK;
		on = uiop->uio_offset & MAXBOFFSET;
		n = MIN(MAXBSIZE - on, uiop->uio_resid);
		diff = bdevsize - uiop->uio_offset;

		if (diff <= 0) {
			error = ENXIO;
			break;
		}
		if (diff < n)
			n = diff;

		/*
		 * as_iolock will determine if we are properly
		 * page-aligned to do the pagecreate case, and if so,
		 * will hold the "from" pages until after the uiomove
		 * to avoid deadlocking and to catch the case of
		 * writing a file to itself.
		 */
		n = as_iolock(uiop, iolpl, n, blkvp, bdevsize, &pagecreate);
		if (n == 0) {
			error = EFAULT;
			break;
		}

		base = segmap_getmap(segkmap, blkvp, off);

		if (pagecreate) {
			SNLOCK(sp);
			segmap_pagecreate(segkmap, base + on, (u_int)n, 0);
			SNUNLOCK(sp);
		}

		error = uiomove(base + on, n, UIO_WRITE, uiop);

		/* Now release any pages held by as_iolock. */
		for (ppp = iolpl; *ppp; ppp++ )
			PAGE_RELE(*ppp);

		if (pagecreate
		  && uiop->uio_offset < roundup(off + on + n, PAGESIZE)) {
			/*
			 * We created pages w/o initializing them completely,
			 * thus we need to zero the part that wasn't set up.
			 * This can happen if we write to the end of the device
			 * or if we had some sort of error during the uiomove.
			 */
			int nzero, nmoved;

			nmoved = uiop->uio_offset - (off + on);
			ASSERT(nmoved >= 0 && nmoved <= n);
			nzero = roundup(n, PAGESIZE) - nmoved;
			ASSERT(nzero > 0 && on + nmoved + nzero <= MAXBSIZE);
			(void) kzero(base + on + nmoved, (u_int)nzero);
		}

		if (error == 0) {
			int flags = 0;

			/*
			 * Force write back for synchronous write cases.
			 */
			if (ioflag & IO_SYNC)
				flags = SM_WRITE;
			else if (n + on == MAXBSIZE || IS_SWAPVP(vp)) {
				/*
				 * Have written a whole block.
				 * Start an asynchronous write and
				 * mark the buffer to indicate that
				 * it won't be needed again soon.
				 * Push swap files here, since it
				 * won't happen anywhere else.
				 */
				flags = SM_WRITE | SM_ASYNC | SM_DONTNEED;
			}
			smark(sp, SUPD|SCHG);
			error = segmap_release(segkmap, base, flags);
		} else
			(void) segmap_release(segkmap, base, SM_INVAL);

	} while (error == 0 && uiop->uio_resid > 0 && n != 0);

	return error;
}

STATIC int
spec_ioctl(vp, cmd, arg, mode, cr, rvalp)
	register struct vnode *vp;
	int cmd;
	int arg;
	int mode;
	struct cred *cr;
	int *rvalp;
{
	dev_t dev;
	int error;

	if (vp->v_type != VCHR)
		return ENOTTY;
	dev = VTOS(vp)->s_dev;
	if (cdevsw[getmajor(dev)].d_str)
		error = strioctl(vp, cmd, arg, mode, U_TO_K, cr, rvalp);
	else
		error = (*cdevsw[getmajor(dev)].d_ioctl)(dev, cmd, arg, 
			mode, cr, rvalp);
	return error;
}

/* ARGSUSED */
STATIC int
spec_getattr(vp, vap, flags, cr)
	register struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *cr;
{
	int error;
	register struct snode *sp;
	register struct vnode *realvp;

	if (flags & ATTR_COMM && vp->v_type == VBLK)
		vp = VTOS(vp)->s_commonvp;
	
	sp = VTOS(vp);
	if ((realvp = sp->s_realvp) == NULL) {
		/*
		 * No real vnode behind this one.  Fill in the fields
		 * from the snode.
		 *
		 * This code should be refined to return only the
		 * attributes asked for instead of all of them.
		 */
		vap->va_type = vp->v_type;
		vap->va_mode = 0;
		vap->va_uid = vap->va_gid = 0;
		vap->va_fsid = sp->s_fsid;
		vap->va_nodeid = (long)sp & 0xFFFF;	/* XXX -- must fix */
		vap->va_nlink = 0;
		vap->va_size = sp->s_size;
		vap->va_rdev = sp->s_dev;
		vap->va_blksize = MAXBSIZE;
		vap->va_nblocks = btod(vap->va_size);
	} else if (error = VOP_GETATTR(realvp, vap, flags, cr))
		return error;

	vap->va_atime.tv_sec = sp->s_atime;
	vap->va_atime.tv_nsec = 0;
	vap->va_mtime.tv_sec = sp->s_mtime;
	vap->va_mtime.tv_nsec = 0;
	vap->va_ctime.tv_sec = sp->s_ctime;
	vap->va_ctime.tv_nsec = 0;
	vap->va_vcode = 0;

	return 0;
}

STATIC int
spec_setattr(vp, vap, flags, cr)
	struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *cr;
{
	register struct snode *sp = VTOS(vp);
	register struct vnode *realvp;
	int error;

	if ((realvp = sp->s_realvp) == NULL)
		error = 0;	/* no real vnode to update */
	else
		error = VOP_SETATTR(realvp, vap, flags, cr);
	if (error == 0) {
		/*
		 * If times were changed, update snode.
		 */
		if (vap->va_mask & AT_ATIME)
			sp->s_atime = vap->va_atime.tv_sec;
		if (vap->va_mask & AT_MTIME) {
			sp->s_mtime = vap->va_mtime.tv_sec;
			sp->s_ctime = hrestime.tv_sec;
		}
	}
	return error;
}

STATIC int
spec_access(vp, mode, flags, cr)
	struct vnode *vp;
	int mode;
	int flags;
	struct cred *cr;
{
	register struct vnode *realvp;

	if ((realvp = VTOS(vp)->s_realvp) != NULL)
		return VOP_ACCESS(realvp, mode, flags, cr);
	else
		return 0;	/* Allow all access. */
}

/*
 * In order to sync out the snode times without multi-client problems,
 * make sure the times written out are never earlier than the times
 * already set in the vnode.
 */
STATIC int
spec_fsync(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{
	register struct snode *sp = VTOS(vp);
	register struct vnode *realvp;
	struct vattr va, vatmp;

	/*
	 * If times didn't change, don't flush anything.
	 */
	if ((sp->s_flag & (SACC|SUPD|SCHG)) == 0 && vp->v_type != VBLK)
		return 0;

	sp->s_flag &= ~(SACC|SUPD|SCHG);

	if (vp->v_type == VBLK && sp->s_commonvp != vp
	&& sp->s_commonvp->v_pages != NULL && !IS_SWAPVP(sp->s_commonvp))
		(void) VOP_PUTPAGE(sp->s_commonvp, 0, 0, 0, (struct cred *) 0);

	/*
	 * If no real vnode to update, don't flush anything.
	 */
	if ((realvp = sp->s_realvp) == NULL)
		return 0;

	vatmp.va_mask = AT_ATIME|AT_MTIME;
	if (VOP_GETATTR(realvp, &vatmp, 0, cr) == 0) {
		if (vatmp.va_atime.tv_sec > sp->s_atime)
			va.va_atime = vatmp.va_atime;
		else {
			va.va_atime.tv_sec = sp->s_atime;
			va.va_atime.tv_nsec = 0;
		}
		if (vatmp.va_mtime.tv_sec > sp->s_mtime)
			va.va_mtime = vatmp.va_mtime;
		else {
			va.va_mtime.tv_sec = sp->s_mtime;
			va.va_mtime.tv_nsec = 0;
		}
		va.va_mask = AT_ATIME|AT_MTIME;
		(void) VOP_SETATTR(realvp, &va, 0, cr);
	}
	(void) VOP_FSYNC(realvp, cr);
	return 0;
}

STATIC void
spec_inactive(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{
	register struct snode *sp = VTOS(vp);
	register struct vnode *cvp;

	/*
	 * Must sdelete() first to prevent a race when spec_fsync() sleeps.
	 */
	sdelete(sp);

	if (sp->s_realvp)
		(void) spec_fsync(vp, cr);

	if (vp->v_type == VBLK && vp->v_pages != NULL)
		VOP_PUTPAGE(sp->s_commonvp, 0, 0, B_INVAL, (struct cred *) 0);

	if (sp->s_realvp) {
		VN_RELE(sp->s_realvp);
		sp->s_realvp = NULL;
	}

	cvp = sp->s_commonvp;
	if (cvp && VN_CMP(cvp, vp) == 0)
		VN_RELE(cvp);

	kmem_free((caddr_t)sp, sizeof (*sp));
}

STATIC int
spec_fid(vp, fidpp)
	struct vnode *vp;
	struct fid **fidpp;
{
	register struct vnode *realvp;

	if ((realvp = VTOS(vp)->s_realvp) != NULL)
		return VOP_FID(realvp, fidpp);
	else
		return EINVAL;
}

/* ARGSUSED */
STATIC int
spec_seek(vp, ooff, noffp)
	struct vnode *vp;
	off_t ooff;
	off_t *noffp;
{
	return 0;
}

/* ARGSUSED */
STATIC int
spec_frlock(vp, cmd, bfp, flag, offset, cr)
	register struct vnode *vp;
	int cmd;
	struct flock *bfp;
	int flag;
	off_t offset;
	struct cred *cr;
{

	register struct snode *csp = VTOS(VTOS(vp)->s_commonvp);

	/*
	 * If file is being mapped, disallow frlock.
	 */
	if (csp->s_mapcnt > 0)
		return EAGAIN;

	return fs_frlock(vp, cmd, bfp, flag, offset, cr);
}

/* ARGSUSED */
STATIC int
spec_realvp(vp, vpp)
	register struct vnode *vp;
	register struct vnode **vpp;
{
	register struct snode *sp = VTOS(vp);
	struct vnode *rvp;

	vp = sp->s_realvp;
	if (vp && VOP_REALVP(vp, &rvp) == 0)
		vp = rvp;
	*vpp = vp;
	return 0;
}

/*
 * klustsize should be a multiple of PAGESIZE and <= MAXPHYS.
 *
 * XXX -- until drivers understand pageio it's not safe to kluster
 * more than a page at a time.
 */

#define	KLUSTSIZE	PAGESIZE

#if 0
#define	KLUSTSIZE	(56 * 1024)
#endif

int klustsize = KLUSTSIZE;
int spec_ra = 1;
int spec_lostpage;	/* number of times we lost original page */

/* ARGSUSED */
STATIC int
spec_getapage(vp, off, protp, pl, plsz, seg, addr, rw, cr)
	register struct vnode *vp;
	u_int off;
	u_int *protp;
	struct page *pl[];
	u_int plsz;
	struct seg *seg;
	addr_t addr;
	enum seg_rw rw;
	struct cred *cr;
{
	register struct snode *sp;
	struct buf *bp, *bp2;
	struct page *pp, *pp2, **ppp, *pagefound;
	u_int io_off, io_len;
	u_int blksz, blkoff;
	int dora, err;
	u_int xlen;
	int adj_klustsize;

	sp = VTOS(vp);

reread:
	err = 0;
	bp = NULL;
	bp2 = NULL;
	dora = 0;

	if (spec_ra && sp->s_nextr == off)
		dora = 1;
	else
		dora = 0;
	if (sp->s_size == UNKNOWN_SIZE) {
		dora = 0;
		adj_klustsize = PAGESIZE;
	} else
		adj_klustsize = klustsize;

	if ((pagefound = page_find(vp, off)) == NULL) {
		/*
		 * Need to really do disk I/O to get the page.
		 */
		blkoff = (off / adj_klustsize) * adj_klustsize;
		if (sp->s_size == UNKNOWN_SIZE) {
			blksz = PAGESIZE;
		} else {
			if (blkoff + adj_klustsize <= sp->s_size)
				blksz = adj_klustsize;
			else
				blksz = sp->s_size - blkoff;
		}

		pp = pvn_kluster(vp, off, seg, addr, &io_off, &io_len,
		    blkoff, blksz, 0);

		if (pp == NULL)
			cmn_err(CE_PANIC, "spec_getapage pvn_kluster");

		if (pl != NULL) {
			int sz;

			if (plsz >= io_len) {
				/*
				 * Everything fits, set up to load
				 * up and hold all the pages.
				 */
				pp2 = pp;
				sz = io_len;
			} else {
				/*
				 * Set up to load plsz worth
				 * starting at the needed page.
				 */
				for (pp2 = pp; pp2->p_offset != off;
				  pp2 = pp2->p_next)
					ASSERT(pp2->p_next->p_offset !=
					  pp->p_offset);
				sz = plsz;
			}

			ppp = pl;
			do {
				PAGE_HOLD(pp2);
				*ppp++ = pp2;
				pp2 = pp2->p_next;
				sz -= PAGESIZE;
			} while (sz > 0);
			*ppp = NULL;		/* terminate list */
		}

		bp = pageio_setup(pp, io_len, vp,
		  pl == NULL ? (B_ASYNC|B_READ) : B_READ);

		bp->b_dev = cmpdev(vp->v_rdev);
		bp->b_edev = vp->v_rdev;
		bp->b_blkno = btodt(io_off);

		/*
		 * Zero part of page which we are not
		 * going to be reading from disk now.
		 */
		xlen = io_len & PAGEOFFSET;
		if (xlen != 0)
			pagezero(pp->p_prev, xlen, PAGESIZE - xlen);

		(*bdevsw[getmajor(vp->v_rdev)].d_strategy)(bp);
		vminfo.v_pgin++;
		vminfo.v_pgpgin += btopr(io_len);
		sp->s_nextr = io_off + io_len;
	}

	if (dora) {
		u_int off2;
		addr_t addr2;

		off2 = ((off / klustsize) + 1) * klustsize;
		addr2 = addr + (off2 - off);

		/*
		 * If addr is now in a different seg or we are past
		 * EOF then don't bother trying with read-ahead.
		 */
		if (addr2 >= seg->s_base + seg->s_size || off2 >= sp->s_size)
			pp2 = NULL;
		else {
			if (off2 + klustsize <= sp->s_size)
				blksz = klustsize;
			else
				blksz = sp->s_size - off2;

			pp2 = pvn_kluster(vp, off2, seg, addr2, &io_off,
			  &io_len, off2, blksz, 1);
		}

		if (pp2 != NULL) {
			bp2 = pageio_setup(pp2, io_len, vp, B_READ|B_ASYNC);

			bp2->b_dev = cmpdev(vp->v_rdev);
			bp2->b_edev = vp->v_rdev;
			bp2->b_blkno = btodt(io_off);
			/*
			 * Zero part of page which we are not
			 * going to be reading from disk now.
			 */
			xlen = io_len & PAGEOFFSET;
			if (xlen != 0)
				pagezero(pp2->p_prev, xlen, PAGESIZE - xlen);

			(*bdevsw[getmajor(vp->v_rdev)].d_strategy)(bp2);
			vminfo.v_pgin++;
			vminfo.v_pgpgin += btopr(io_len);
		}
	}

	if (bp != NULL && pl != NULL) {
		err = biowait(bp);
		pageio_done(bp);
	} else if (pagefound != NULL) {
		int s;

		/*
		 * We need to be careful here because if the page was
		 * previously on the free list, we might have already
		 * lost it at interrupt level.
		 */
		s = splvm();
		if (pagefound->p_vnode == vp && pagefound->p_offset == off) {
			/*
			 * If the page is still in transit or if
			 * it is on the free list, call page_lookup
			 * to try and wait for / reclaim the page.
			 */
			if (pagefound->p_intrans || pagefound->p_free)
				pagefound = page_lookup(vp, off);
		}
		(void) splx(s);
		if (pagefound == NULL || pagefound->p_offset != off
		  || pagefound->p_vnode != vp || pagefound->p_gone) {
			spec_lostpage++;
			goto reread;
		}
		if (pl != NULL) {
			PAGE_HOLD(pagefound);
			pl[0] = pagefound;
			pl[1] = NULL;
			sp->s_nextr = off + PAGESIZE;
		}
	}

	if (err && pl != NULL) {
		for (ppp = pl; *ppp != NULL; *ppp++ = NULL)
			PAGE_RELE(*ppp);
	}

	return err;
}

/*
 * Return all the pages from [off..off+len) in block device.
 */
STATIC int
spec_getpage(vp, off, len, protp, pl, plsz, seg, addr, rw, cr)
	struct vnode *vp;
	u_int off, len;
	u_int *protp;
	struct page *pl[];
	u_int plsz;
	struct seg *seg;
	addr_t addr;
	enum seg_rw rw;
	struct cred *cr;
{
	struct snode *sp = VTOS(vp);
	int err;

	ASSERT(vp->v_type == VBLK && sp->s_commonvp == vp);

	if (vp->v_flag & VNOMAP)
		return ENOSYS;

	if (off + len > sp->s_size + PAGEOFFSET)
		return EFAULT;	/* beyond EOF */

	if (protp != NULL)
		*protp = PROT_ALL;

	SNLOCK(sp);
	if (len <= PAGESIZE)
		err = spec_getapage(vp, off, protp, pl, plsz, seg, addr,
		  rw, cr);
	else
		err = pvn_getpages(spec_getapage, vp, off, len, protp, pl,
		  plsz, seg, addr, rw, cr);
	SNUNLOCK(sp);

	return err;
}

/*
 * Flags are composed of {B_ASYNC, B_INVAL, B_FREE, B_DONTNEED}.
 */
STATIC int
spec_wrtblk(vp, pp, off, len, flags)
	register struct vnode *vp;
	struct page *pp;
	uint off;
	uint len;
	int flags;
{
	register struct buf *bp;
	int error;

	if ((bp = pageio_setup(pp, len, vp, B_WRITE | flags)) == NULL) {
		pvn_fail(pp, B_WRITE | flags);
		return ENOMEM;
	}

	bp->b_dev = cmpdev(vp->v_rdev);
	bp->b_edev = vp->v_rdev;
	bp->b_blkno = btodt(off);

	(*bdevsw[getmajor(vp->v_rdev)].d_strategy)(bp);

	/*
	 * If async, assume that pvn_done will handle the pages when
	 * I/O is done.
	 */
	if (flags & B_ASYNC)
		return 0;

	error = biowait(bp);
	pageio_done(bp);

	return error;

}

/*
 * Flags are composed of {B_ASYNC, B_INVAL, B_DIRTY B_FREE, B_DONTNEED}.
 * If len == 0, do from off to EOF.
 *
 * The normal cases should be len == 0 & off == 0 (entire vp list),
 * len == MAXBSIZE (from segmap_release actions), and len == PAGESIZE
 * (from pageout).
 */
/* ARGSUSED */
int
spec_putpage(vp, off, len, flags, cr)
	register struct vnode *vp;
	u_int off;
	u_int len;
	int flags;
	struct cred *cr;
{
	register struct snode *sp = VTOS(vp);
	register struct page *pp;
	struct page *dirty, *io_list;
	register u_int io_off, io_len;
	int vpcount;
	int err = 0;
	int adj_klustsize;

	if (vp->v_flag & VNOMAP)
		return ENOSYS;

	if (vp->v_pages == NULL || off >= sp->s_size)
		return 0;

	ASSERT(vp->v_type == VBLK && sp->s_commonvp == vp);

	vpcount = vp->v_count;
	VN_HOLD(vp);

	if (sp->s_size == UNKNOWN_SIZE)
		adj_klustsize = PAGESIZE;
	else
		adj_klustsize = klustsize;

	if (len == 0) {
		/*
		 * Search the entire vp list for pages >= off.
		 */
		pvn_vplist_dirty(vp, off, flags);
		goto out;
	} else {
		u_int fsize, eoff, offlo, offhi;

		/*
		 * Do a range from [off...off + len) via page_find.
		 * We set limits so that we kluster to klustsize boundaries.
		 */
		fsize = (sp->s_size + PAGEOFFSET) & PAGEMASK;
		eoff = MIN(off + len, fsize);
		offlo = (off / adj_klustsize) * adj_klustsize;
		offhi = roundup(eoff, adj_klustsize);
		dirty = pvn_range_dirty(vp, off, eoff, offlo, offhi, flags);
	}
	/*
	 * Now pp will have the list of kept dirty pages marked for
	 * write-back.  It will also handle invalidation and freeing
	 * of pages that are not dirty.  All the pages on the list
	 * returned must still be dealt with here.
	 */

	/*
	 * Handle all the dirty pages not yet dealt with.
	 */
	while ((pp = dirty) != NULL) {
		/*
		 * Pull off a contiguous chunk
		 */
		page_sub(&dirty, pp);
		io_list = pp;
		io_off = pp->p_offset;
		io_len = PAGESIZE;
#if 0 /* XXX */
		while (dirty != NULL && dirty->p_offset == io_off + io_len) {
			pp = dirty;
			page_sub(&dirty, pp);
			page_sortadd(&io_list, pp);
			io_len += PAGESIZE;
			if (io_len >= adj_klustsize - PAGEOFFSET)
				break;
		}
#endif
		/*
		 * Check for page length rounding problems
		 */
		if (io_off + io_len > sp->s_size) {
			ASSERT((io_off + io_len) - sp->s_size < PAGESIZE);
			io_len = sp->s_size - io_off;
		}
		if (err = spec_wrtblk(vp, io_list, io_off, io_len, flags))
			break;
	}

	if (err && dirty != NULL)
		pvn_fail(dirty, B_WRITE | flags);

out:
	/*
	 * Instead of using VN_RELE here we are careful to
	 * call the inactive routine only if the vnode
	 * reference count is now zero but wasn't zero
	 * coming into putpage.  This is to prevent calling
	 * the inactive routine on a vnode that is already
	 * considered to be in the "inactive" state.
	 * XXX -- inactive is a relative term here (sigh).
	 */
	if (--vp->v_count == 0 && vpcount > 0)
		spec_inactive(vp, cr);

	return err;
}

STATIC int
spec_poll(vp, events, anyyet, reventsp, phpp)
	vnode_t *vp;
	short events;
	int anyyet;
	short *reventsp;
	struct pollhead **phpp;
{
	register dev_t dev;
	int error;

	if (vp->v_type == VBLK)
		error = fs_poll(vp, events, anyyet, reventsp, phpp);
	else {
		ASSERT(vp->v_type == VCHR);
		dev = vp->v_rdev;
		if (cdevsw[getmajor(dev)].d_str) {
			ASSERT(vp->v_stream != NULL);
			error = strpoll(vp->v_stream, events, anyyet,
			  reventsp, phpp);
		} else if (cdevsw[getmajor(dev)].d_poll)
			error = (*cdevsw[getmajor(dev)].d_poll)
			  (dev, events, anyyet, reventsp, phpp);
		else
			error = fs_poll(vp, events, anyyet, reventsp, phpp);
	}
	return error;
}

/*
 * This routine is called through the cdevsw[] table to handle
 * traditional mmap'able devices that support a d_mmap function.
 */
/* ARGSUSED */
int
spec_segmap(dev, off, as, addrp, len, prot, maxprot, flags, cred)
	dev_t dev;
	u_int off;
	struct as *as;
	addr_t *addrp;
	u_int len;
	u_int prot, maxprot;
	u_int flags;
	struct cred *cred;
{
	struct segdev_crargs dev_a;
	int (*mapfunc)();
	register int i;

	if ((mapfunc = cdevsw[getmajor(dev)].d_mmap) == nodev)
		return ENODEV;

	/*
	 * Character devices that support the d_mmap
	 * interface can only be mmap'ed shared.
	 */
	if ((flags & MAP_TYPE) != MAP_SHARED)
		return EINVAL;

	/*
	 * Check to ensure that the entire range is
	 * legal and we are not trying to map in
	 * more than the device will let us.
	 */
	for (i = 0; i < len; i += PAGESIZE) {
		if ((*mapfunc)(dev, off + i, maxprot) == (int)NOPAGE)
			return ENXIO;
	}

	if ((flags & MAP_FIXED) == 0) {
		/*
		 * Pick an address w/o worrying about
		 * any vac alignment contraints.
		 */
		map_addr(addrp, len, (off_t)off, 0);
		if (*addrp == NULL)
			return ENOMEM;
	} else {
		/*
		 * User-specified address; blow away any previous mappings.
		 */
		(void) as_unmap(as, *addrp, len);
	}

	dev_a.mapfunc = mapfunc;
	dev_a.dev = dev;
	dev_a.offset = off;
	dev_a.prot = (u_char)prot;
	dev_a.maxprot = (u_char)maxprot;

	return as_map(as, *addrp, len, segdev_create, (caddr_t)&dev_a);
}

STATIC int
spec_map(vp, off, as, addrp, len, prot, maxprot, flags, cred)
	struct vnode *vp;
	u_int off;
	struct as *as;
	addr_t *addrp;
	u_int len;
	u_int prot, maxprot;
	u_int flags;
	struct cred *cred;
{
	register int error = 0;
	struct vnode *cvp = VTOS(vp)->s_commonvp;

	ASSERT(cvp != NULL);
	if (vp->v_flag & VNOMAP)
		return ENOSYS;

	/*
	 * If file is locked, fail mapping attempt.
	 */
	if (vp->v_filocks != NULL)
		return EAGAIN;

	if (vp->v_type == VCHR) {
		int (*segmap)();
		dev_t dev = vp->v_rdev;

		/*
		 * Character device: let the device driver
		 * pick the appropriate segment driver.
		 */
		segmap = cdevsw[getmajor(dev)].d_segmap;
		if (segmap == nodev) {
			if (cdevsw[getmajor(dev)].d_mmap == nodev)
				return ENODEV;

			/*
			 * For cdevsw[] entries that specify a d_mmap
			 * function but don't have a d_segmap function,
			 * we default to spec_segmap for compatibility.
			 */
			segmap = spec_segmap;
		}

		return (*segmap)(dev, off, as, addrp, len, prot, maxprot,
		    flags, cred);
	} else if (vp->v_type == VBLK) {
		struct segvn_crargs vn_a;
		struct vnode *cvp;

		/*
		 * Block device, use the underlying commonvp name for pages.
		 */
		cvp = VTOS(vp)->s_commonvp;
		ASSERT(cvp != NULL);

		if ((int)off < 0 || (int)(off + len) < 0)
			return EINVAL;

		/*
		 * Don't allow a mapping beyond the last page of the device.
		 */
		if (off + len > ((VTOS(cvp)->s_size + PAGEOFFSET) & PAGEMASK))
			return ENXIO;
	
		if ((flags & MAP_FIXED) == 0) {
			map_addr(addrp, len, (off_t)off, 1);
			if (*addrp == NULL)
				return ENOMEM;
		} else {
			/*
			 * User-specified address; blow away any
			 * previous mappings.
			 */
			(void) as_unmap(as, *addrp, len);
		}

		vn_a.vp = cvp;
		vn_a.offset = off;
		vn_a.type = flags & MAP_TYPE;
		vn_a.prot = (u_char)prot;
		vn_a.maxprot = (u_char)maxprot;
		vn_a.cred = cred;
		vn_a.amp = NULL;

		error = as_map(as, *addrp, len, segvn_create, (caddr_t)&vn_a);
	} else
		return ENODEV;

	return error;
}

STATIC int
spec_addmap(vp, off, as, addr, len, prot, maxprot, flags, cred)
	struct vnode *vp;
	u_int off;
	struct as *as;
	addr_t addr;
	u_int len;
	u_int prot, maxprot;
	u_int flags;
	struct cred *cred;
{
	register int error = 0;
	struct vnode *cvp = VTOS(vp)->s_commonvp;

	ASSERT(cvp != NULL);
	if (vp->v_flag & VNOMAP)
		return ENOSYS;
	VTOS(cvp)->s_mapcnt += btopr(len);
	return 0;
}

STATIC int
spec_delmap(vp, off, as, addr, len, prot, maxprot, flags, cred)
	struct vnode *vp;
	u_int off;
	struct as *as;
	addr_t addr;
	u_int len;
	u_int prot, maxprot;
	u_int flags;
	struct cred *cred;
{
	struct snode *csp;

	if (vp->v_flag & VNOMAP)
		return ENOSYS;

	csp = VTOS(VTOS(vp)->s_commonvp);

	csp->s_mapcnt -= btopr(len);

	if (csp->s_mapcnt < 0)
		cmn_err(CE_PANIC, "spec_unmap: fewer than 0 mappings");

	/*
	 * Call the close routine when the last reference of any
	 * kind through any [s,v]node goes away.
	 */
	if (!stillreferenced((dev_t)csp->s_dev, vp->v_type))
		/* XXX - want real file flags here. */
		(void) device_close(vp, 0, cred);
	return 0;
}

STATIC int
spec_allocstore(vp, off, len, cred)
	struct vnode	*vp;
	u_int		off;
	u_int		len;
	struct cred	*cred;
{
	/* Specfs needs no blocks allocated, so succeed silently */
	return 0;
}
