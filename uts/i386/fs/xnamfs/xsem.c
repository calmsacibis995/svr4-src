/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:xnamfs/xsem.c	1.3"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 *	@(#)xsem.c	1.7 87/06/22 
 */

/*
 * THIS FILE CONTAINS CODE WHICH IS DESIGNED TO BE
 * PORTABLE BETWEEN DIFFERENT MACHINE ARCHITECHTURES
 * AND CONFIGURATIONS. IT SHOULD NOT REQUIRE ANY
 * MODIFICATIONS WHEN ADAPTING XENIX TO NEW HARDWARE.
 */


/*
 *  XENIX Semaphores are xnamnodes corresponding to vnodes
 *  of the special file type VXNAM.
 *  Semaphore xnamnodes contain the current count (x_scount) of the
 *  semaphore and pointers to the head and tail of the list of waiters for
 *  the semaphore (x_headw and x_tailw).
 *  When a process must wait for the resource, it puts its file
 *  structure on the waiting list. By convention, the first element on the
 *  waiting list is the one that currently "owns" the semaphore; i.e., it is the
 *  the one using the resource governed by the semaphore.
 */

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/immu.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/proc.h"
#include "sys/fs/xnamnode.h"
#include "sys/var.h"
#include "sys/conf.h"
#include "sys/fstyp.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/rf_messg.h"
#include "sys/uio.h"
#include "sys/sysinfo.h"


#define SERROR  01              /* process controlling a sem terminated */

extern int getf();
extern int lookupname();
extern void xnammark();

STATIC int xsem_alloc();
STATIC int cwaitsem();

static struct xsem *xs_freelist;
extern struct xsem xsem[]; 	/* XENIX semaphores */
extern int nxsem;


/*
 * XENIX creatsem() system call.
 *
 * Creates an instance of a semaphore named sem_name.
 * Semaphores are files of 0 length - the file name space
 * is used to provide unique identifiers for semaphores.
 * Creatsem fails if a file named sem_name already exists and has been
 * opened by at least one process.
 */
struct creatsema {
	char *sem_name;   /* path name specifying the semaphore */
	int mode;       /* protection of file */
};

int
creatsem(uap, rvp)
	register struct creatsema *uap;
	rval_t *rvp;
{
	struct vnode *vp;
	register struct xnamnode *xp;
	struct file *fp;
	struct vattr vattr;
	register int error;
	int fd;

	if(error = lookupname(uap->sem_name, UIO_USERSPACE, FOLLOW, NULLVPP, &vp)) {
		if(error != ENOENT)
			return error;
		vattr.va_type = VXNAM;
		vattr.va_mode = (uap->mode & MODEMASK) & ~u.u_cmask;
		vattr.va_mask = AT_TYPE|AT_MODE;
		vattr.va_rdev = XNAM_SEM;
		vattr.va_mask |= AT_RDEV;
		if(error = vn_create(uap->sem_name, UIO_USERSPACE,
					&vattr, EXCL, 0, &vp, CRMKNOD))
			return error;
	} else {
		if((vp->v_type != VXNAM) || (vp->v_rdev != XNAM_SEM)) {
			VN_RELE(vp);
			return ENOTNAM;
		}
		if(error = VOP_ACCESS(vp, VREAD, 0, u.u_cred)) {
			VN_RELE(vp);
			return error;
		}
	}

	/* must be xnam type vnode */

	if(vp->v_op != &xnam_vnodeops) {
		VN_RELE(vp);
		return EINVAL;
	}

	if(vp->v_count != 1) {
		VN_RELE(vp);
		return EEXIST;
	}

	ASSERT(vp->v_rdev == XNAM_SEM);
	xp = VTOXNAM(vp);
	if(error = xsem_alloc(xp)) {
		VN_RELE(vp);
		return error;
	}

	xp->x_sem->x_scount = 1;
	xp->x_sem->x_headw = (xp->x_sem->x_tailw = (struct file *)NULL);
	xp->x_sem->x_eflag = 0;
	xnammark(xp, XNAMACC);

	if(error = falloc(vp, (u_int)uap->mode&FMASK, &fp, &fd)) {
		VN_RELE(vp);
		return error;
	}

	rvp->r_val1 = fd;
	return 0;
}

/*
 * XENIX opensem() system call.
 *
 * Opens a semaphore named sem_name and returns that semaphore's unique
 * identification number.
 * Opensem() fails if the semaphore doesn't exist, or if a process that 
 * controlled the sem terminated without relinquishing control leaving 
 * the resource governed by the sem in an inconsistent state (ENAVAIL).
 */
struct opensema {
	caddr_t sem_name;       /* path name specifying the semaphore */
};

int
opensem(uap, rvp)
	register struct opensema *uap;
	rval_t *rvp;
{
	struct vnode *vp;
	register struct xnamnode *xp;
	struct file *fp;
	int error, fd;

	if(error = lookupname(uap->sem_name, UIO_USERSPACE, FOLLOW, NULLVPP, &vp))
		return error;

	if(vp->v_type != VXNAM || vp->v_rdev != XNAM_SEM) {
		VN_RELE(vp);
		return ENOTNAM;
	}

	/* must be associated with an xnamnode */ 
	if(vp->v_op != &xnam_vnodeops) {
		VN_RELE(vp);
		return EINVAL;
	}

	xp = VTOXNAM(vp);
	if(error = xsem_alloc(xp)) {	/* be sure x_sem allocated */
		VN_RELE(vp);
		return error;
	}

	if(xp->x_sem->x_eflag == SERROR) {
		VN_RELE(vp);
		return ENAVAIL;
	}

	if(error = VOP_ACCESS(vp, VREAD, 0, u.u_cred)) {
		VN_RELE(vp);
		return error;
	}

	if(error = falloc(vp, 0, &fp, &fd)) {
		VN_RELE(vp);
		return error;
	}

	rvp->r_val1 = fd;
	return 0;
}

/*  XENIX sigsem() system call. */
struct sigsema {
	int fdes;     /* sem # to signal (special file desc.) */
};

/*ARGSUSED*/
int
sigsem(uap, rvp)
	struct sigsema *uap;
	rval_t *rvp;
{
	register struct vnode *vp;
	register struct xnamnode *xp;
	struct file *fp;
	int error;


	if (error = getf(uap->fdes, &fp))
		return error;

	vp = fp->f_vnode;

	/* must be sem type vnode */
	if((vp->v_type != VXNAM) || (vp->v_rdev != XNAM_SEM))
		return ENOTNAM;

	if(vp->v_op != &xnam_vnodeops)
		return EINVAL;

	/* ensure sem owner is signalling */
	xp = VTOXNAM(vp);
	if (xp->x_sem->x_headw != fp) {
		error = ENAVAIL;
		goto out;
	}                               /* remove self from head of list as owner */

	xp->x_sem->x_headw = fp->f_un.f_slnk;
					/* signal 1st waiting process, if there is one */
	if((xp->x_sem->x_scount)++ < 0)
		wakeprocs((caddr_t)xp->x_sem->x_headw, PRMPT);
	xnammark(xp, XNAMACC);

out:
	return error;
}

struct waitsema {
	int fdes;
};

/*ARGSUSED*/
int
waitsem(uap, rval)
	struct waitsema *uap;
	rval_t *rval;
{
	return cwaitsem(uap->fdes, 0);
}
	
/*ARGSUSED*/
int
nbwaitsem(uap, rval)
	struct waitsema *uap;
	rval_t *rval;
{
	return cwaitsem(uap->fdes, 1);
}
	
/*  common code for waitsem and nbwaitsem */
STATIC int
cwaitsem(fdes, nowait)
	int fdes, nowait;
{
	register struct vnode *vp;
	register struct xnamnode *xp;
	struct file *fp;
	struct file *wp;
	int error;


	if (error = getf(fdes, &fp)) 
		return error;

	vp = fp->f_vnode;
	/* must be sem type vnode */
	if(vp->v_type != VXNAM || vp->v_rdev != XNAM_SEM)
		return ENOTNAM;
	if(vp->v_op != &xnam_vnodeops) 
	/* must be associated with an xnamnode */
		return EINVAL;
	xp = VTOXNAM(vp);
	if (xp->x_sem->x_eflag == SERROR)
		return ENAVAIL;
	/* 
	 * ensure that this process neither owns the semaphore nor waits for it
	 * (i.e, prevent 2 waitsem calls without intervening sigsem by same proc
	 * using the same file descriptor).
 	 */
	for (wp = xp->x_sem->x_headw; wp != NULL; wp = wp->f_un.f_slnk) {
		if (wp == fp)
			/* instead of pending */
	    		return EINVAL;
	}
	if (nowait && xp->x_sem->x_scount <= 0)
		return ENAVAIL;
	xnammark(xp, XNAMACC);

	fp->f_un.f_slnk = (struct file *)NULL;
	if(--(xp->x_sem->x_scount) < 0) {   /* sem busy, must wait */
		/* insert at tail of waiter list */
		xp->x_sem->x_tailw->f_un.f_slnk = fp;
		xp->x_sem->x_tailw = fp;
		while(fp != xp->x_sem->x_headw) {
			sleep((caddr_t)fp, PSLEP);
			/* check if process controlling the sem has */
			/* ceased to use it before giving up control*/
			if(xp->x_sem->x_eflag == SERROR)
				return ENAVAIL;
		}
	} else  /* insert on list as sem "owner" */
		xp->x_sem->x_headw = (xp->x_sem->x_tailw = fp);
	return 0;
}

/*
 * Closesem() is called from closef() to cleanup in case a terminating process
 * is the current "owner" of a semaphore protected resource or is waiting
 * on a semaphore.
 */

void
closesem(fp, vp)
	struct file *fp;
	struct vnode *vp;
{
	struct file *lfp;
	struct file *tfp;
	register struct xnamnode *xp;

	if(vp->v_rdev != XNAM_SEM)
		return;
	xp = VTOXNAM(vp);
	lfp = (struct file *)NULL;
	for(tfp=xp->x_sem->x_headw; tfp != NULL; lfp=tfp, tfp=tfp->f_un.f_slnk) {
		if(tfp == fp) {
	   		if (lfp == NULL) {     /* process is owner of semaphore */
				xp->x_sem->x_eflag = SERROR;
				xp->x_sem->x_scount++;
				/* cause all processes waiting to error return */
				for(tfp = tfp->f_un.f_slnk; tfp != NULL; tfp = tfp->f_un.f_slnk) {
					xp->x_sem->x_scount++;
					wakeprocs((caddr_t) tfp, PRMPT);
				}
				xnammark(xp, XNAMCHG);
				xp->x_sem->x_headw = (struct file *)NULL;
				return;
	    		}
			/* remove self from waiting list */
			xp->x_sem->x_scount++;
			lfp->f_un.f_slnk = tfp->f_un.f_slnk;
			if (tfp->f_un.f_slnk == NULL)
				xp->x_sem->x_tailw = lfp;
	    		xnammark(xp, XNAMCHG);
		}
	}
}


/* Xsemfork() handles inheritance of XENIX semaphores over forks.
 * Each child is given its own copy of a file structure referencing
 * the semaphore.
 */
xsemfork()
{
	struct file *fp;
	struct file *ofp;
	register int i;
	int error, fd;

	if(nxsem == 0)
		return 0;
	for(i = 0; i < u.u_nofiles; i++) {
		if(getf(i, &ofp) == 0 && ofp->f_vnode->v_type == VXNAM &&
			 ofp->f_vnode->v_rdev == XNAM_SEM) {
			/* if an open semaphore was inherited */
			/* replace the inherited file structure */
			/* with a new one not shared with parent */
			setf(i, NULLFP);
			
			if(error = falloc(ofp->f_vnode, 0, &fp, &fd))
				return error;
			ofp->f_count--;
			ASSERT(ofp->f_count);
			fp->f_vnode->v_count++;
		}
	}
	return(0);
}


/* initialize XENIX semaphore free list */
void
xseminit()
{
	register struct xsem *pxsem;

	if (nxsem <= 0)		
		return; 		/* XENIX sems not configured in */

	/* last one in list has headw set to NULL */
	for(xs_freelist=pxsem = &xsem[0]; pxsem < &xsem[nxsem-1]; pxsem++)
		pxsem->x_headw = (struct file *)(pxsem+1);
}

/* allocate a new XENIX semaphore struct */
STATIC int
xsem_alloc(xp)
	struct xnamnode *xp;
{
	register struct xsem *pxsem;

	if(xp->x_sem)
		return 0;	/* already allocated */

	if((pxsem = xs_freelist) == NULL) {
		cmn_err(CE_NOTE, "XENIX semaphore table overflow\n");
		return ENFILE;
	}
	xs_freelist = (struct xsem *)pxsem->x_headw;
	xp->x_sem = pxsem;
	return 0;
}

/* put XENIX semphore struct back on freelist; called from xnam_inactive() */
void
unxsem_alloc(xp)
	struct xnamnode *xp;
{
	xp->x_sem->x_headw = (struct file *)xs_freelist;
	xs_freelist = xp->x_sem;
	xp->x_sem = NULL;
}
