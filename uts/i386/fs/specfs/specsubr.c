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

#ident	"@(#)kern-fs:specfs/specsubr.c	1.3.1.7"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/cred.h"
#include "sys/kmem.h"
#include "sys/sysmacros.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/fs/snode.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/open.h"
#include "sys/file.h"
#include "sys/user.h"
/* XENIX Support */
#include "sys/termios.h"
#include "sys/stream.h"
#include "sys/strsubr.h"
/* End XENIX Support */

STATIC int specfstype;
STATIC dev_t specdev;

STATIC struct snode *sfind();
STATIC struct vnode *commonvp();
int device_close();
STATIC void sinsert();

extern struct vfsops spec_vfsops;

/*
 * Return a shadow special vnode for the given dev.
 * If no snode exists for this dev create one and put it
 * in a table hashed by <dev,realvp>.  If the snode for
 * this dev is already in the table return it (ref count is
 * incremented by sfind).  The snode will be flushed from the
 * table when spec_inactive calls sdelete.
 *
 * The fsid is inherited from the real vnode so that clones
 * can be found.
 *
 */
struct vnode *
specvp(vp, dev, type, cr)
	struct vnode *vp;
	dev_t dev;
	vtype_t type;
	struct cred *cr;
{
	register struct snode *sp,*nsp;
	register struct vnode *svp;
	extern struct vnode *fifovp();
	extern struct vnode *xnamvp();
	struct vattr va;

	if (vp == NULL)
		return NULL;
	if (vp->v_type == VFIFO)
		return fifovp(vp, cr);

	if (vp->v_type == VXNAM)
		return xnamvp(vp, cr);

	ASSERT(vp->v_type == type);
	ASSERT(vp->v_rdev == dev);

	if ((sp = sfind(dev, type, vp)) == NULL) {
		sp = (struct snode *)kmem_zalloc(sizeof(*sp), KM_SLEEP);
		STOV(sp)->v_op = &spec_vnodeops;

		/*
		 * Init the times in the snode to those in the vnode.
		 */
		va.va_mask = AT_TIMES;
		if (VOP_GETATTR(vp, &va, 0, cr) == 0) {
			sp->s_atime = va.va_atime.tv_sec;
			sp->s_mtime = va.va_mtime.tv_sec;
			sp->s_ctime = va.va_ctime.tv_sec;
			sp->s_fsid = va.va_fsid;
		} else
			sp->s_fsid = specdev;
		sp->s_realvp = vp;
		VN_HOLD(vp);
		sp->s_dev = dev;
		svp = STOV(sp);
		svp->v_rdev = dev;
		svp->v_count = 1;
		svp->v_data = (caddr_t)sp;
		svp->v_type = type;
		svp->v_vfsp = vp->v_vfsp;
		if (type == VBLK || type == VCHR) {
			sp->s_commonvp = commonvp(dev, type);
			svp->v_stream = sp->s_commonvp->v_stream;
			if (vp->v_type == VBLK)
				sp->s_size = VTOS(sp->s_commonvp)->s_size;
		}

		if ((nsp = sfind(dev, type, vp)) == NULL)
			sinsert(sp);
		else {		/* Lost the race */
			if (sp->s_commonvp)
                                VN_RELE(sp->s_commonvp);
			kmem_free((caddr_t)sp, sizeof (*sp));
			sp = nsp;
		}
	}
	return STOV(sp);
}

/*
 * Return a special vnode for the given dev; no vnode is supplied
 * for it to shadow.  Always create a new snode and put it in the
 * table hashed by <dev,NULL>.  The snode will be flushed from the
 p table when spec_inactive() calls sdelete().
 */
struct vnode *
makespecvp(dev, type)
	register dev_t dev;
	register vtype_t type;
{
	register struct snode *sp;
	register struct vnode *svp;

	sp = (struct snode *)kmem_zalloc(sizeof(*sp), KM_SLEEP);
	svp = STOV(sp);
	svp->v_op = &spec_vnodeops;
	svp->v_type = type;
	svp->v_rdev = dev;
	svp->v_count = 1;
	svp->v_data = (caddr_t)sp;
	svp->v_vfsp = NULL;
	sp->s_atime = hrestime.tv_sec;
	sp->s_mtime = hrestime.tv_sec;
	sp->s_ctime = hrestime.tv_sec;
	sp->s_fsid = specdev;
	sp->s_commonvp = commonvp(dev, type);
	svp->v_stream = sp->s_commonvp->v_stream;
	sp->s_size = VTOS(sp->s_commonvp)->s_size;
	sp->s_realvp = NULL;
	sp->s_dev = dev;
	sinsert(sp);
	return svp;
}

/*
 * Find a special vnode that refers to the given device
 * of the given type.  Never return a "common" vnode.
 * Return NULL if a special vnode does not exist.
 * HOLD the vnode before returning it.
 */
struct vnode *
specfind(dev, type)
	dev_t dev;
	vtype_t type;
{
	register struct snode *st;
	register struct vnode *nvp;

	st = stable[STABLEHASH(dev)];
	while (st != NULL) {
		if (st->s_dev == dev) {
			nvp = STOV(st);
			if (nvp->v_type == type && st->s_commonvp != nvp) {
				VN_HOLD(nvp);
				return nvp;
			}
		}
		st = st->s_next;
	}
	return NULL;
}

/*
 * Check to see if a device is still in use, given the snode.  The
 * device may still be in use if someone has it open either through
 * the same snode or a different snode, or if someone has it mapped.
 * If it is still open, return 1, otherwise return 0.
 */
int
stillreferenced(dev, type)
	dev_t dev;
	vtype_t type;
{
	register struct snode *sp, *csp;
	register struct vnode *vp;

	if ((vp = specfind(dev, type)) == NULL)
		return 0;
	VN_RELE(vp);
	sp = VTOS(vp);
	csp = VTOS(sp->s_commonvp);
	if (csp->s_count > 0)		/* another snode exists */
		return 1;
	if (csp->s_mapcnt > 0)		/* mappings to device exist */
		return 1;
	return 0;
}

/*
 * Check to see if there is an snode in the table referring to a given device
 * other than the one the vnode provided is associated with.  If so, return
 * it.
 */
struct vnode *
other_specvp(vp)
	register struct vnode *vp;
{
	struct snode *sp;
	register dev_t dev;
	register struct snode *st;
	register struct vnode *nvp;

	sp = VTOS(vp);
	dev = sp->s_dev;
	st = stable[STABLEHASH(dev)];
	while (st != NULL) {
		if (st->s_dev == dev && (nvp = STOV(st)) != vp
		  && nvp->v_type == vp->v_type)
			return nvp;
		st = st->s_next;
	}
	return NULL;
}

/*
 * Given a device vnode, return the common
 * vnode associated with it.
 */
struct vnode *
common_specvp(vp)
	register struct vnode *vp;
{
	register struct snode *sp;

	if ((vp->v_type != VBLK) && (vp->v_type != VCHR) || 
	  vp->v_op != &spec_vnodeops)
		return vp;
	sp = VTOS(vp);
	return sp->s_commonvp;
}

/*
 * Returns a special vnode for the given dev.  The vnode is the
 * one which is "common" to all the snodes which represent the
 * same device.  For use ONLY by SPECFS.
 */

STATIC
struct vnode *
commonvp(dev, type)
	dev_t dev;
	vtype_t type;
{
	register struct snode *sp, *nsp;
	register struct vnode *svp;

	if ((sp = sfind(dev, type, NULL)) == NULL) {
		sp = (struct snode *)kmem_zalloc(sizeof(*sp), KM_SLEEP);
		STOV(sp)->v_op = &spec_vnodeops;
		sp->s_realvp = NULL;
		sp->s_dev = dev;
		sp->s_fsid = specdev;
		svp = STOV(sp);
		svp->v_rdev = dev;
		svp->v_count = 1;
		svp->v_data = (caddr_t)sp;
		svp->v_type = type;
		svp->v_vfsp = NULL;
		sp->s_commonvp = STOV(sp); /* points to itself */
		if (type == VBLK && getmajor(dev) < bdevcnt) {
			int (*size)();

			size = bdevsw[getmajor(dev)].d_size;
			if (size != nulldev)
				sp->s_size = dtob((*size)(dev));
			else 
				sp->s_size = UNKNOWN_SIZE;
		}
                if ((nsp = sfind(dev, type, NULL)) == NULL)
                        sinsert(sp);
		else {
                      	kmem_free((caddr_t)sp, sizeof (*sp));
                        sp = nsp;
		}
	}
	return STOV(sp);
}

/*
 * Snode lookup stuff.
 * These routines maintain a table of snodes hashed by dev so
 * that the snode for an dev can be found if it already exists.
 */

struct snode *stable[STABLESIZE];

/*
 * Put a snode in the table.
 */
STATIC void
sinsert(sp)
	struct snode *sp;
{

	sp->s_next = stable[STABLEHASH(sp->s_dev)];
	stable[STABLEHASH(sp->s_dev)] = sp;
	return;
}

/*
 * Remove an snode from the hash table.
 * The realvp is not released here because spec_inactive() still
 * needs it to do a spec_fsync().
 */
void
sdelete(sp)
	struct snode *sp;
{
	struct snode *st;
	struct snode *stprev = NULL;

	st = stable[STABLEHASH(sp->s_dev)];
	while (st != NULL) {
		if (st == sp) {
			if (stprev == NULL)
				stable[STABLEHASH(sp->s_dev)] = st->s_next;
			else
				stprev->s_next = st->s_next;
			break;
		}
		stprev = st;
		st = st->s_next;
	}
	return;
}

/*
 * Lookup an snode by <dev, type, vp>.
 * ONLY looks for snodes with non-NULL s_realvp members and
 * common snodes (with s_commonvp poining to its vnode).
 */
STATIC struct snode *
sfind(dev, type, vp)
	dev_t dev;
	vtype_t type;
	struct vnode *vp;
{
	register struct snode *st;
	register struct vnode *svp;

	st = stable[STABLEHASH(dev)];
	while (st != NULL) {
		svp = STOV(st);
		if (st->s_dev == dev && svp->v_type == type
		  && VN_CMP(st->s_realvp, vp)
		  && (vp != NULL || st->s_commonvp == svp)) {
			VN_HOLD(svp);
			return st;
		}
		st = st->s_next;
	}
	return NULL;
}

/*
 * Mark the accessed, updated, or changed times in an snode
 * with the current time.
 */
void
smark(sp, flag)
	register struct snode *sp;
	register int flag;
{
	time_t t = hrestime.tv_sec;

	sp->s_flag |= flag;
	if (flag & SACC)
		sp->s_atime = t;
	if (flag & SUPD)
		sp->s_mtime = t;
	if (flag & SCHG)
		sp->s_ctime = t;
	return;
}

/*
 * Compute the blocksize for a given block device.  If the device is
 * currently mounted, use the block size of the file system, otherwise
 * (for compatibility with such things as old tapes) use 512.  Note that
 * SVR3 mistakenly used 1024 in the latter case.
 */
int
bdevbsize(dev)
	register dev_t dev;
{
	register struct vfs *vfsp;

	if ((vfsp = vfs_devsearch(dev)) == NULL)
		return 512;
	return vfsp->vfs_bsize;
}

void
specinit(vswp, fstype)
	struct vfssw *vswp;
	int fstype;
{
	dev_t dev;

	/*
	 * Associate vfs and vnode operations.
	 */
	vswp->vsw_vfsops = &spec_vfsops;
	specfstype = fstype;
	if ((dev = getudev()) == -1)
		dev = 0;
	specdev = makedevice(dev, 0);
	return;
}

int
device_close(vp, flag, cr)
	struct vnode *vp;
	int flag;
	struct cred *cr;
{
	struct snode *sp = VTOS(vp);
	dev_t dev = sp->s_dev;
	enum vtype type = vp->v_type;
	register int error;

	switch (type) {

	case VCHR:
		if (cdevsw[getmajor(dev)].d_str) {
			error = strclose(sp->s_commonvp, flag, cr);
			vp->v_stream = NULL;
		} else {
			error = (*cdevsw[getmajor(dev)].d_close)
			  (dev, flag, OTYP_CHR, cr);
		}
		break;

	case VBLK:
		/*
		 * On last close a block device we must
		 * invalidate any in-core blocks so that we
		 * can, for example, change floppy disks.
		 */
		(void) spec_putpage(sp->s_commonvp, 0, 0, B_INVAL, (struct cred *) 0);
		bflush(dev);
		binval(dev);

                error = (*bdevsw[getmajor(dev)].d_close) 
		 (dev, flag, OTYP_BLK, cr);
		break;
	}

	return error;
}

struct vnode *
makectty(ovp)
	register vnode_t *ovp;
{
	register vnode_t *vp;

	if (vp = makespecvp(ovp->v_rdev, VCHR)) {
		VTOS(vp)->s_count++;
		VTOS(VTOS(vp)->s_commonvp)->s_count++;
	}

	return vp;
}

/* XENIX Support */
/*
 * XENIX rdchk() support.
 */
int
spec_rdchk(vp, cr, rvalp)
	struct vnode *vp;
	struct cred *cr;
	int *rvalp;
{
	dev_t dev;
	int error;

	if (vp->v_type != VCHR || vp->v_op != &spec_vnodeops)
		return ENOTTY;
	dev = VTOS(vp)->s_dev;
	if (cdevsw[getmajor(dev)].d_str)
		error = strioctl(vp, FIORDCHK, 0, 0, K_TO_K, cr, rvalp);
	else
		error =
		  (*cdevsw[getmajor(dev)].d_ioctl)(dev, FIORDCHK, 0, 0, cr, rvalp);
	return error;
}
/* End XENIX Support */
