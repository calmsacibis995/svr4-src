/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:xnamfs/xnamsubr.c	1.3"
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
#include "sys/fs/xnamnode.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/file.h"

STATIC dev_t xnamdev;

STATIC struct xnamnode *xnamfind();
STATIC void xnaminsert();

extern void xsdinit();
extern void xseminit();

extern struct vfsops xnam_vfsops;

/*
 * Return a special XENIX name vnode for the given sub-type.
 * If no xnamnode exists for this sub-type, create one and put it
 * in a table indexed by dev.  If the xnamnode for
 * this dev is already in the table return it (ref count is
 * incremented by xnamfind).  The xnamnode will be flushed from the
 * table when xnam_inactive calls xnamdelete.
 */
struct vnode *
xnamvp(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{
	register struct xnamnode *xp;
	register struct vnode *svp;
	dev_t dev = vp->v_rdev;
	vtype_t type = vp->v_type;
	struct vattr va;


	if (vp == NULL)
		return NULL;
	if ((xp = xnamfind(dev, vp)) == NULL) {
		xp = (struct xnamnode *)kmem_zalloc(sizeof(*xp), KM_SLEEP);
		XNAMTOV(xp)->v_op = &xnam_vnodeops;

		/*
		 * Init the times in the xnamnode to those in the vnode.
		 */
		va.va_mask = AT_TIMES;
		if (VOP_GETATTR(vp, &va, 0, cr) == 0) {
			xp->x_atime = va.va_atime.tv_sec;
			xp->x_mtime = va.va_mtime.tv_sec;
			xp->x_ctime = va.va_ctime.tv_sec;
			xp->x_fsid = va.va_fsid;
			xp->x_mode = va.va_mode;
			xp->x_uid = va.va_uid;
			xp->x_gid = va.va_gid;
		} else {
			xp->x_fsid = xnamdev;
		}
		xp->x_realvp = vp;
		VN_HOLD(vp);
		xp->x_dev = dev;
		svp = XNAMTOV(xp);
		svp->v_rdev = dev;
		svp->v_count = 1;
		svp->v_data = (caddr_t)xp;
		svp->v_type = type;
		svp->v_vfsp = vp->v_vfsp;
		xnaminsert(xp);
	}
	return XNAMTOV(xp);
}

/*
 * xnamnode lookup stuff.
 * These routines maintain a table of xnamnodes indexed by dev so
 * that the xnamnode for a name file can be found if it already exists.
 */

struct xnamnode *xnamtable[XNAMTABLESIZE];

/*
 * Put a xnamnode in the table.
 */
STATIC void
xnaminsert(xp)
	struct xnamnode *xp;
{

	xp->x_next = xnamtable[xp->x_dev - 1];
	xnamtable[xp->x_dev - 1] = xp;
}

/*
 * Remove an xnamnode from the table.
 */
void
xnamdelete(xp)
	struct xnamnode *xp;
{
	struct xnamnode *xt;
	struct xnamnode *xtprev = NULL;

	xt = xnamtable[xp->x_dev - 1];
	while (xt != NULL) {
		if (xt == xp) {
			if (xtprev == NULL)
				xnamtable[xp->x_dev - 1] = xt->x_next;
			else
				xtprev->x_next = xt->x_next;
			break;
		}
		xtprev = xt;
		xt = xt->x_next;
	}
}

/*
 * Lookup an xnamnode by <dev, vp>.
 */
STATIC struct xnamnode *
xnamfind(dev, vp)
	dev_t dev;
	struct vnode *vp;
{
	register struct xnamnode *xt;
	register struct vnode *svp;

	xt = xnamtable[dev - 1];
	while (xt != NULL) {
		svp = XNAMTOV(xt);
		if (VN_CMP(xt->x_realvp, vp)
		  && (vp != NULL )) {
			VN_HOLD(svp);
			return xt;
		}
		xt = xt->x_next;
	}
	return NULL;
}

/*
 * Mark the accessed, updated, or changed times in an xnamnode
 * with the current time.
 */
void
xnammark(xp, flag)
	register struct xnamnode *xp;
	register int flag;
{
	time_t t = hrestime.tv_sec;

	xp->x_flag |= flag;
	if (flag & XNAMACC)
		xp->x_atime = t;
	if (flag & XNAMUPD)
		xp->x_mtime = t;
	if (flag & XNAMCHG)
		xp->x_ctime = t;
}

/* ARGSUSED */
void
xnaminit(vswp, fstype)
	register struct vfssw *vswp;
	int fstype;
{
	vswp->vsw_vfsops = &xnam_vfsops;
	xsdinit();
	xseminit();
}
