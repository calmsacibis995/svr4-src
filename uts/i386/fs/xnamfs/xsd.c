/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft 	*/
/*	Corporation and should be treated as Confidential.	*/

#ident	"@(#)kern-fs:xnamfs/xsd.c	1.3"

/*
 *  Shared Data functions.
 *
 *	Shared data segments are vnodes of the type VXNAM.  Thus, they
 *	live in the same name space as XENIX semaphores.  The per segment 
 *	data lives in the xnamnode for that segment; the per process data
 *	lives in the system table sdtab and in the process
 *	table (see fs/xnamnode.h and proc.h).
 *
 *	Note that in this implementation (386) the x_len element in the
 *	xnamnode structure for shared data is treated as a limit, not a
 *	size.
 */

/* XENIX Support */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/lock.h"
#include "sys/kmem.h"
#include "sys/mman.h"
#include "sys/sd.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/tuneable.h"
#include "sys/vm.h"
#include "sys/cmn_err.h"
#include "sys/var.h"
#include "sys/fstyp.h"
#include "sys/conf.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/fs/xnamnode.h"
#include "sys/debug.h"
#include "sys/uio.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_vn.h"
#include "vm/anon.h"


static char	firstsd = 1;
static struct	sd	*sdfreep;	/* points to free list of sdtab entries */

static struct xsd *xsd_freelist;
extern struct xsd xsd[]; 
extern int nxsd;


#define x_nextsd	x_sun.x_chain
#define x_amp		x_sun.xamp

STATIC int sdfrcm();
STATIC int shmfree();
STATIC void sdlvcm();
STATIC int xsd_alloc();

extern int lookupname();
extern void xnammark();

/* 
 * search for matching slot in sdtab.
 * by convention, addr == 0 means slot is empty.
 */
STATIC int
sdsrch(addr, psdp)
char *addr;
struct sd **psdp;
{
	register struct sd *sdp;

	/*
	 * Find sd entry in system wide table.
	 */
	for (sdp = u.u_procp->p_sdp; sdp != NULL; sdp = sdp->sd_link) 
		if ( addr == sdp->sd_addr ) {
			*psdp = sdp;
			return 0;
		}
	return EINVAL;
}

STATIC int
sdatt_common(xp, flags, rvp)
register struct	xnamnode	*xp;
register int	flags; 
rval_t *rvp;
{
	register struct vnode *vp;
	register struct	sd *sdp, *sdq;
	ushort	first_attach;
	int npages;		/* size of the shared data segment in pages */
	register proc_t *pp = u.u_procp;
	register u_int size;
	struct segvn_crargs	crargs; /* segvn create arguments */
	char *addr;
	int error;

	vp = XNAMTOV(xp);

	ASSERT(xp->x_sd);
	/* initialize sdtab entries on first sd use */
	if ((firstsd) && (v.v_xsdsegs && v.v_xsdslots))	
	{
		/*
 		 * Set up the free list of sdtab entries. One entry is
		 * maintained per proc per shared data segment.  The entries
 		 * for one proc's shared data segments are linked together  
                 * through the table.
 		 */
								/* M002 */
		for (sdfreep = sdp = &sdtab[0];
			sdp < &sdtab[(v.v_xsdsegs * v.v_xsdslots) - 1]; sdp++)
			sdp->sd_link = sdp + 1;
		sdp->sd_link = NULL;
		firstsd = 0;
	}

	/* allocate free slot in sdtab. */
	if ((sdp = sdfreep) == NULL) {
		VN_RELE(vp); /* v_count was incremented by caller, and is now */
		  	  /* decremented on failure to attach              */
		return EMFILE;
	}
	sdfreep = sdfreep->sd_link;

	first_attach = xp->x_sd->x_flags & SDI_CLEAR;
		
	/* check permissions */
	if(error = VOP_ACCESS(vp, VREAD, 0, u.u_cred)) {
		goto err1;
	}
	if (flags & SD_WRITE)
		if(error = VOP_ACCESS(vp, VWRITE, 0, u.u_cred)) {
			goto err1;
		}

	/* ensure this sd segment not already attached to this proc */
	for (sdq = u.u_procp->p_sdp; sdq != NULL; sdq = sdq->sd_link)
	{
		if (sdq->sd_addr && (sdq->sd_xnamnode == xp))
		{
			error = EINVAL;
			goto err1;
		}
	}

	/* ensure that this process can expand */
	npages = btopr((unsigned long) xp->x_sd->x_len + 1);

	if (first_attach)
	{
		/* first attach done to this sd segment */

		/*
		 * This is a new shared data segment.
		 * Allocate a segment and update xnamnode info.
		 */
		if (anon_resv(xp->x_sd->x_len + 1) == 0) {
			error = ENOMEM;
			goto err1;
		}
		xp->x_sd->x_amp = (struct anon_map *)
		    kmem_zalloc(sizeof (struct anon_map), KM_SLEEP);
		xp->x_sd->x_amp->anon = (struct anon **)
		    kmem_zalloc(npages * sizeof (struct anon *), KM_SLEEP);
		xp->x_sd->x_amp->size = ptob(npages);
		xp->x_sd->x_amp->refcnt = 0; 
	}

	/* expand the process to include the sd area */

	size = xp->x_sd->x_amp->size;

	map_addr(&addr, size, (off_t)0, 1);
	if(addr == NULL) {
		error = ENOMEM;
		goto err1;
	}

	crargs = *(struct segvn_crargs *)zfod_argsp;    /* structure copy */
	crargs.offset = 0;
	crargs.type = MAP_SHARED;
	crargs.amp = xp->x_sd->x_amp;
	crargs.maxprot = PROT_ALL;
	crargs.prot = (flags & SD_WRITE ) ? PROT_ALL :
	    (PROT_ALL & ~PROT_WRITE);

	error = as_map(pp->p_as, addr, size, segvn_create, (caddr_t)&crargs);
	if(error) {
		goto err1;
	}
	if (first_attach) 
		xp->x_sd->x_flags &= ~SDI_CLEAR;

	sdp->sd_addr = addr;

	/* fill the per process data with correct info */
	sdp->sd_xnamnode = xp;
	sdp->sd_cpaddr = NULL;
	sdp->sd_flags = flags & (SD_WRITE);
	sdp->sd_link = u.u_procp->p_sdp;
	u.u_procp->p_sdp = sdp;

	/* set return value (start address of segment) */
	/* convert linear address to logical address */

	rvp->r_val1 = (int)sdp->sd_addr;

	return 0;

	/* common exit point for error returns */
err1:
	sdp->sd_link = sdfreep;		/* free the sdtab entry */
	sdfreep = sdp;
	VN_RELE(vp); /* v_count was incremented by caller, and is now */
		  /* decremented on failure to attach              */
	return error;
}


/* 
 *	Sdget system call
 */
struct sdgeta {
	char *path;
	int flags;
	unsigned limit;
	int mode;
};

sdget(uap, rvp)
	struct sdgeta *uap;
	rval_t *rvp;
{
	struct vnode *vp;
	register struct xnamnode *xp;
	struct vattr vattr;
	int error;


	if (uap->flags & SD_CREAT)
	{
		/* want to create a new sd segment */

		/* convert path to pointer to vnode */
		if (error = lookupname(uap->path, UIO_USERSPACE, FOLLOW, NULLVPP, &vp)) {
			if (error != ENOENT) 
				goto sdget_err2;
			vattr.va_type = VXNAM;
			vattr.va_mode = (uap->mode & MODEMASK) & ~u.u_cmask;
			vattr.va_mask = AT_TYPE|AT_MODE;
			vattr.va_rdev = XNAM_SD;
			vattr.va_mask |= AT_RDEV;
			if(error = vn_create(uap->path, UIO_USERSPACE,
					&vattr, EXCL, 0, &vp, CRMKNOD)) 
				goto sdget_err2;
			if (vp->v_op != &xnam_vnodeops) {
				error = EINVAL;
				goto sdget_err1;
			}

			xp = VTOXNAM(vp);
			xnammark(xp, XNAMCHG);
		}
		else	
		{
			/* if file is not correct type or is active, */
			/* exit with error			     */
			if ((vp->v_type != VXNAM) || 
			    (vp->v_rdev != XNAM_SD))
			{
				error = ENOTNAM;
				goto sdget_err1;
			}

			/*
		 	 * Don't allow sdget unless
		 	 * vp is associated with an xnamnode.
		 	 */
			if (vp->v_op != &xnam_vnodeops) {
				error = EINVAL;
				goto sdget_err1;
			}

			if (vp->v_count != 1)
			{
				if (BADVISE_PRE_SV)
				{
					/* ignore extraneous CREAT flag */
					uap->flags &= ~SD_CREAT;
					goto attach_only;
				}
				error = EEXIST;
				goto sdget_err1;
			}
			/* N.B.  The mode bits do not get set to uap->mode
			 * upon re-use of an existing inode.  This is
			 * an unfortunate, but known, "feature" of XENIX.
			 * We maintain that behavior here for XENIX 286 and
			 * XENIX 386 compatibility.  Sigh.
			 */ 

		}
		xp = VTOXNAM(vp);
		if (error = xsd_alloc(xp)) {
			goto sdget_err1;
		}
		xp->x_sd->x_amp = (struct anon_map *)0;
		xp->x_sd->x_len = uap->limit;
		xp->x_sd->x_snum = 0;
		xp->x_sd->x_flags = (uap->flags & (SD_UNLOCK|SD_NOWAIT))
					   | SDI_CLEAR;

	}
	else	/* just want to attach, not create */
	{
		if (error = lookupname(uap->path, UIO_USERSPACE, FOLLOW, NULLVPP, &vp)) 
			goto sdget_err2;
		
		if ((vp->v_type != VXNAM) ||
		    (vp->v_rdev != XNAM_SD))
		{
			error = ENOTNAM;
			goto sdget_err1;
		}

		/*
		 * Don't allow sdget unless
		 * vp is associated with an xnamnode.
		 */
		if (vp->v_op != &xnam_vnodeops) {
			error = EINVAL;
			goto sdget_err1;
		}

		if (vp->v_count == 1)
		{
			error = ENOENT;
			goto sdget_err1;
		}

		xp = VTOXNAM(vp);
		ASSERT(xp->x_sd);
	}

attach_only:					/* M008 */
	/* Have found or created the sd xnamnode.  Now try to attach. */
	return sdatt_common(xp, uap->flags, rvp);
sdget_err1:				/* common error exit */
	VN_RELE(vp);
sdget_err2:
	return error;
}


/*
 *	Sdfree system call
 */

struct sdfreea {
	char *address;
};

xsdfree(uap, rvp)
	struct sdfreea *uap;
	rval_t *rvp;
{
	struct sd *sdp;
	int error;


	/* find the slot in sdtab. */
	if (error = sdsrch(uap->address, &sdp))
		return error;

	ASSERT(sdp->sd_xnamnode->x_sd);
	rvp->r_val1 = 0;

	return sdfrcm(sdp);
}

STATIC int
sdfrcm(sdp)
	register struct sd *sdp;
{
	int error;
	/* force an sdleave if necessary. */
	if (sdp->sd_flags & SD_BTWN)
		sdlvcm(sdp);

    	error = shmfree(sdp);

	/* zero out the per proc stuff */
	sdp->sd_addr = NULL;

	/* copy XENIX 286 shared data to "real" shared data */
	if ((BADVISE_XSDSWTCH) && (u.u_procp->p_sdp != NULL))
		xsdswtch(0);

	/* free the sdtab entry */
	if (u.u_procp->p_sdp == sdp)
		u.u_procp->p_sdp = sdp->sd_link;
	else 
	{
		struct	sd *p;
		for (p = u.u_procp->p_sdp; p->sd_link != sdp; p = p->sd_link)
			;
		p->sd_link = sdp->sd_link;
	}
	sdp->sd_link = sdfreep;
	sdfreep = sdp;
	return error;
}

STATIC int
shmfree(sdp)
	struct sd *sdp;
{
	register struct anon_map	*amp;
	register struct as	*as;
	register struct	xnamnode	*xp = sdp->sd_xnamnode;
	register struct	vnode	*vp = XNAMTOV(xp);
	register proc_t *pp = u.u_procp;
	register char *addr;
	register uint ampsize = 0;

	ASSERT(xp->x_sd);

	/*
	 *  Find the segment corresponding to a sd entry.
	 */
	
	addr = sdp->sd_addr;
	as = pp->p_as;
	amp = xp->x_sd->x_amp;
	ASSERT(amp != NULL);

	/*
	 * if this is the last process, the segment will be
	 * freed in the next detach
	 */
	if (vp->v_count == 1) {
		ampsize = amp->size;
		xnammark(xp, XNAMACC | XNAMCHG);
	}

	/*
	 * Detach segment from process
	 */
	if(as_unmap(as, addr, xp->x_sd->x_len + 1) != 0) {
		VN_RELE(vp);
		return EINVAL;
	}
	VN_RELE(vp);

	/* If no attachments */
	if(ampsize)
		anon_unresv(ampsize);
	return 0;
}


/*
 *	Sdenter system call
 */
struct sdentera {
	char *	addr;
	int	flags;
};

/*ARGSUSED*/
sdenter(uap, rvp)
	struct sdentera *uap;
	rval_t *rvp;
{
	struct sd *sdp;
	register short 	*flg;
	int error;


	/* find the right slot */
	if (error = sdsrch(uap->addr, &sdp))
		return error;

	ASSERT(sdp->sd_xnamnode->x_sd);
	flg = &(sdp->sd_xnamnode)->x_sd->x_flags;

	/* disallow entering to write if are attached readonly */
	if ( (uap->flags & SD_WRITE) && !(sdp->sd_flags & SD_WRITE )) 
		return EINVAL;

	if (*flg & SDI_LOCKED)
		if (uap->flags & SD_NOWAIT)
		{
			return ENAVAIL;
		} else {
			while (*flg & SDI_LOCKED)
			{
				*flg |= SDI_NTFY;
				sleep((caddr_t)sdp->sd_xnamnode, PSLEP);
			}
		}
	if ((*flg & SD_UNLOCK) == 0)
		*flg |= SDI_LOCKED;
	sdp->sd_flags |= SD_BTWN;
	return 0;

}


/*
 *	Sdleave system call
 */
struct sdleavea {
	char *addr;
};

/*ARGSUSED*/
int 
sdleave(uap, rvp)
	struct sdleavea *uap;
	rval_t *rvp;
{
	struct sd *sdp;
	int error;

	/* find the slot. */
	if (error = sdsrch(uap->addr, &sdp))
		return error;
	ASSERT(sdp->sd_xnamnode->x_sd);
	sdlvcm(sdp);

	return 0;
}

/*
 * common code for sdleave, xsdfree
 */
STATIC void
sdlvcm(sdp)
	register struct sd *sdp;
{
	register short *flg;

	/* unlock segment if of type LOCKED */
	flg = &((sdp->sd_xnamnode)->x_sd->x_flags);
	if ( (*flg & SD_UNLOCK) == 0 )
		*flg &= ~(SDI_LOCKED);

	/* reset BETWEEN */
	sdp->sd_flags &= ~(SD_BTWN);

	/* increment snum */
	sdp->sd_xnamnode->x_sd->x_snum++;
	sdp->sd_xnamnode->x_sd->x_snum &= 0x7fff;	/* for 286 compatibility */
	/* wakeup if necessary */
	if (*flg & SDI_NTFY)
	{
		*flg &= ~(SDI_NTFY);
		wakeprocs((caddr_t)sdp->sd_xnamnode, PRMPT);
	}
}


/*
 *	Sdgetv system call
 */
struct sdgetva {
	char *addr;
};

sdgetv(uap, rvp)
	struct sdgetva *uap;
	rval_t *rvp;
{
	struct sd *sdp;
	int error;


	if (error = sdsrch(uap->addr, &sdp))
		return error;

	ASSERT(sdp->sd_xnamnode->x_sd);
	rvp->r_val1 = (sdp->sd_xnamnode)->x_sd->x_snum;
	return 0;
}

/*
 *	Sdwaitv system call
 */
struct sdwaitva {
	char *addr;
	int	num;
};

sdwaitv(uap, rvp)
	struct sdwaitva *uap;
	rval_t *rvp;
{
	struct sd *sdp;
	int error;

	if (error = sdsrch(uap->addr, &sdp))
		return error;

	ASSERT(sdp->sd_xnamnode->x_sd);
	while ((sdp->sd_xnamnode)->x_sd->x_snum == uap->num)
	{
		sdp->sd_xnamnode->x_sd->x_flags |= SDI_NTFY;
		sleep((caddr_t)sdp->sd_xnamnode, PSLEP);
	}

	rvp->r_val1 = (sdp->sd_xnamnode)->x_sd->x_snum;
	return 0;
}

/*
 *	Xsdexit - called to free shared data on proc exit.
 */
void
xsdexit()
{
	while (u.u_procp->p_sdp != NULL) 
		(void)sdfrcm(u.u_procp->p_sdp);
}

/*
 *	Xsdfork - called to adjust vnode reference counts during fork
 *		and to create duplicate sdtab entries for the child.
 */
xsdfork(cp,pp)
	register struct proc *cp, *pp;
{
	register struct sd *sdpp, *sdcp;
	int error;

	for (sdpp = pp->p_sdp; sdpp != NULL; sdpp = sdpp->sd_link) {
		if (sdpp->sd_addr != NULL) {
			/* allocate free slot in sdtab. */
	
			if ((sdcp = sdfreep) == NULL) {
				error = EMFILE;
				goto failed;
			}
			sdfreep = sdfreep->sd_link;
			/* fill the per process data with right stuff. */
			sdcp->sd_xnamnode = sdpp->sd_xnamnode;
			sdcp->sd_addr = sdpp->sd_addr;
			sdcp->sd_cpaddr = sdpp->sd_cpaddr;
			sdcp->sd_flags = sdpp->sd_flags;
			sdcp->sd_link = cp->p_sdp;		
			cp->p_sdp = sdcp;
		
			/* Increment reference count of vnode */
			sdpp->sd_xnamnode->x_vnode.v_count++;
			ASSERT(sdpp->sd_xnamnode->x_sd);
		}

	}
	
	return 0;

failed:
	if (cp->p_sdp != NULL) {
		for (sdcp=cp->p_sdp; sdcp!=NULL; sdpp=sdcp, sdcp = sdcp->sd_link){
			sdcp->sd_addr = NULL;
			sdcp->sd_xnamnode->x_vnode.v_count--;
	    	}
		sdpp->sd_link = sdfreep;
		sdfreep = cp->p_sdp;
		cp->p_sdp = NULL;
	}
	return error;
}



/* initialize XENIX shared data free list */
void
xsdinit()
{
	register struct xsd *pxsd;

	if (nxsd <= 0)		/* XENIX sd not configured in */
		return;
	
	/* last one in list has next set to NULL */
	for(xsd_freelist=pxsd = &xsd[0]; pxsd < &xsd[nxsd-1]; pxsd++)
		pxsd->x_nextsd = pxsd + 1;
	pxsd->x_nextsd = NULL;
}

/* allocate a new XENIX shared data struct */
STATIC int
xsd_alloc(xp)
	struct xnamnode *xp;
{
	register struct xsd *pxsd;

	if(xp->x_sd)
		return 1;	/* already allocated */

	if((pxsd = xsd_freelist) == NULL) {
		cmn_err(CE_NOTE, "XENIX shared data table overflow\n");
		return ENFILE;
	}
	xsd_freelist = (struct xsd *)pxsd->x_nextsd;
	xp->x_sd = pxsd;
	return 0;
}

/* put XENIX shared data struct back on freelist; 
   called from xnam_inactive() */
void
unxsd_alloc(xp)
	struct xnamnode *xp;
{
	xp->x_sd->x_nextsd = xsd_freelist;
	xsd_freelist = xp->x_sd;
	xp->x_sd = NULL;
}
/* End XENIX Support */
