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

#ident  "@(#)kern-vm:vm_swap.c	1.3.1.6"

/*
 * Virtual swap device
 *
 * The virtual swap device consists of the logical concatenation of one
 * or more physical swap areas.  It provides a logical array of anon
 * slots, each of which corresponds to a page of swap space.
 *
 * Each physical swap area has an associated anon array representing
 * its physical storage.  These anon arrays are logically concatenated
 * sequentially to form the overall swap device anon array.  Thus, the
 * offset of a given entry within this logical array is computed as the
 * sum of the sizes of each area preceding the entry plus the offset
 * within the area containing the entry.
 *
 * The anon array entries for unused swap slots within an area are
 * linked together into a free list.  Allocation proceeds by finding a
 * suitable area (attempting to balance use among all the areas) and
 * then returning the first free entry within the area.  Thus, there's
 * no linear relation between offset within the swap device and the
 * address (within its segment(s)) of the page that the slot backs;
 * instead, it's an arbitrary one-to-one mapping.
 *
 * Associated with each swap area is a swapinfo structure.  These
 * structures are linked into a linear list that determines the
 * ordering of swap areas in the logical swap device.  Each contains a
 * pointer to the corresponding anon array, the area's size, and its
 * associated vnode.
 *
 * We we delete a swap device, since it is too complicated to find
 * and relocate all the data structures that point to our anon slots,
 * we use the an_bap field to go indirect to the new backing anon slot.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/proc.h"
#include "sys/kmem.h"
#include "sys/vfs.h"
#include "sys/stat.h"
#include "sys/vnode.h"
#include "sys/mode.h"
#include "sys/file.h"
#include "sys/uio.h"
#include "sys/conf.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/tuneable.h"
#include "sys/inline.h"
#include "vm/bootconf.h"
#include "vm/trace.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/page.h"
#include "vm/seg_vn.h"
#include "vm/hat.h"
#include "vm/anon.h"
#include "sys/swap.h"
#include "vm/seg_map.h"

extern struct	vnode *common_specvp();
extern int	lookupname();
extern int	suser();
extern void	segmap_flush();

/*
 * To balance the load among multiple swap areas, we don't allow
 * more than swap_maxcontig allocations to be satisfied from a
 * single swap area before moving on to the next swap area.  This
 * effectively "interleaves" allocations among the many swap areas.
 */
STATIC	int swap_maxcontig = 1024 * 1024 / PAGESIZE;	/* 1MB of pages */

struct	swapinfo	*swapinfo;
STATIC	struct	swapinfo *silast;
STATIC	int	nswapfiles;
STATIC	int	sw_rdwr();
STATIC	void	swapinfo_free();
STATIC	int	swapadd();
STATIC	int	swapdel();
STATIC	void	delswap();
STATIC	void	undelswap();
void	swapconf();		/* configure first swap device */

/*
 * Allocate a single page from the virtual swap device.
 */
struct anon *
swap_alloc()
{
	register struct swapinfo *sip = silast;
	register struct anon *ap;

	do {
		if ((sip->si_flags & ST_INDEL) == 0) {
			ap = sip->si_free;
			if (ap) {
				sip->si_free = ap->un.an_next;
				sip->si_nfpgs--;
				if (++sip->si_allocs >= swap_maxcontig) {
					sip->si_allocs = 0;
					if ((silast = sip->si_next) == NULL)
						silast = swapinfo;
				}
#				ifdef   TRACE
				{
					struct vnode *vp;
					uint off;
					swap_xlate(ap, &vp, &off);
					trace3(TR_MP_SWAP, vp, off, ap);
				}
#				endif /*   TRACE */
				return (ap);
			}
			sip->si_allocs = 0;
		}
		if ((sip = sip->si_next) == NULL)
			sip = swapinfo;
	} while (sip != silast);
	return ((struct anon *)NULL);
}

/*
 * Free a swap page.
 */
void
swap_free(ap)
	register struct anon *ap;
{
	register struct swapinfo *sip = swapinfo;

	/*
	 * Find the swap area containing ap and then put
	 * ap at the head of that area's free list.
	 * If we are an indirect anon pointer
	 * then free up the back end too.
	 */
	if (ap->an_bap) {
		register struct anon *bap;

		bap = ap->an_bap;
		ASSERT(bap->an_bap == ap);
		ASSERT(bap->an_refcnt == 0);
		ap->an_bap = NULL;
		bap->an_bap = NULL;
		swap_free(bap);
	}

	do {
		if (sip->si_anon <= ap && ap <= sip->si_eanon) {
			ap->un.an_next = sip->si_free;
			sip->si_free = ap;
			sip->si_nfpgs++;

			/* If swap area is deleted and the
			 * front is now free we can free up 
			 * all the data structures now.
			 */
			if ((sip->si_flags & ST_DELETED)
			  && (sip->si_nfpgs == sip->si_npgs)) {
				swapinfo_free(sip);
			}
			return;
		}
	} while ((sip = sip->si_next) != NULL);
	cmn_err(CE_PANIC, "swap_free");
	/*NOTREACHED*/
}

/*
 * Return the <vnode, offset> pair
 * corresponding to the given anon struct.
 */
void
swap_xlate(ap, vpp, offsetp)
	register struct anon *ap;
	register struct vnode **vpp;
	register uint *offsetp;
{
	register struct swapinfo *sip = swapinfo;

	/*
	 * Use the back pointer if we are indirect.
	 */
	if (ap->an_bap) {
		register struct anon *bap;

		bap = ap->an_bap;
		ASSERT(ap->an_refcnt > 0);
		ASSERT(bap->an_bap == ap);
		ASSERT(bap->an_refcnt == 0);
		ap = bap;
	}
	do {
		if (sip->si_anon <= ap && ap <= sip->si_eanon) {
			*offsetp = sip->si_soff + 
					((ap - sip->si_anon) << PAGESHIFT);
			*vpp = sip->si_vp;
			return;
		}
	} while ((sip = sip->si_next) != NULL);
	cmn_err(CE_PANIC, "swap_xlate");
	/*NOTREACHED*/
}

/*
 * Return the anon struct corresponding for the given
 * <vnode, offset> if it is part of the virtual swap device.
 * This is used by page_free and page_reclaim to tear down
 * (or setup ) the `hint' in the anon structure.  If we have
 * a refcnt then this ap is directly in use, so return it.
 * If there is a back pointer and a refcnt, return it.
 * Otherwise return NULL.
 * Our callers would not do anything unless we give them
 * an ap with a non zero refcnt.
 */
struct anon *
swap_anon(vp, offset)
	register struct vnode *vp;
	register uint offset;
{
	register struct swapinfo *sip = swapinfo;
	register struct anon *ap;
	register struct anon *bap;

	if (vp && sip) {
		do {
			if (vp == sip->si_vp && offset >= sip->si_soff 
			  && offset < sip->si_eoff) {
				ap = (sip->si_anon
					+ ((offset-sip->si_soff) >> PAGESHIFT));
				bap = ap->an_bap;
				if (ap->an_refcnt) {
					ASSERT( (bap == 0)
					      || ((bap->an_bap == ap)
					       && (bap->an_refcnt == 0)));
					return (ap);
				} else if (bap != NULL && bap->an_refcnt) {
					ASSERT(bap->an_bap == ap);
					return (bap);
				} else {
					return ((struct anon *)NULL);
				}
			}
		} while ((sip = sip->si_next) != NULL);
	}
	return ((struct anon *)NULL);
}

/* does the vp offset range overlap a swap device */

int
swap_in_range(vp, offset, len)
	register struct vnode *vp;
	register uint offset;
	uint len;
{
	register struct swapinfo *sip = swapinfo;
	register uint eoff;

	eoff = offset + len;
	ASSERT (eoff > offset);
	if (vp && sip) {
		do {
			if (vp != sip->si_vp || eoff <= sip->si_soff 
			  || offset >= sip->si_eoff || sip->si_flags&ST_DELETED)
				continue;
			return(1);
		} while ((sip = sip->si_next) != NULL);
	}
	return(0);
}
/*
 * swread and swwrite implement the /dev/drum device, an indirect,
 * user visible, device to allow reading of the (virtual) swap device.
 */

/*ARGSUSED*/
swread(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (sw_rdwr(uio, UIO_READ));
}

/*ARGSUSED*/
swwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	return (sw_rdwr(uio, UIO_WRITE));
}

/*
 * Common routine used to break up reads and writes to the
 * (virtual) swap device to the underlying vnode(s).  This is
 * used to implement the user visable /dev/drum interface.
 */
/*ARGSUSED*/
STATIC int
sw_rdwr(uio, rw)
	register struct uio *uio;
	enum uio_rw rw;
{
	register struct swapinfo *sip = swapinfo;
	int nbytes = uio->uio_resid;
	uint off = 0;
	uint size = 0;
	int error = 0;

	do {
		size = sip->si_eoff - sip->si_soff;
		if (uio->uio_offset >= off &&
		    uio->uio_offset < off + size)
			break;
		off += size;
	} while ((sip = sip->si_next) != NULL);

	if (sip) {
		uio->uio_offset -= off;
		do {
			size = sip->si_eoff - sip->si_soff;
			uio->uio_resid = MIN(size - uio->uio_offset, nbytes);
			nbytes -= uio->uio_resid;
			if (rw == UIO_READ)
				error = VOP_READ(sip->si_vp, uio, 0, u.u_cred);
			else
				error = VOP_WRITE(sip->si_vp, uio, 0, u.u_cred);
			uio->uio_offset = 0;
		} while (error == 0 && uio->uio_resid == 0 && nbytes > 0 &&
		    (sip = sip->si_next));
		uio->uio_resid = nbytes + uio->uio_resid;
	}

	return (error);
}

/*
 * See if name is one of our swap files
 * even though lookupname failed.
 * This can be used by swapdel to delete
 * swap resources on remote machines
 * where the link has gone down.
 */
STATIC struct vnode *
swapdel_byname(name, lowblk)
	char 	*name;	/* pathname to delete. */
	register uint	lowblk;	/* Low block number of area to delete. */
{
	register struct swapinfo **sipp, *osip;
	register uint	soff;

	/*
	 * Find the swap file entry for the file to
	 * be deleted. Skip any entries that are in 
	 * transition.  
	 */

	soff = ctob(btoc(lowblk << SCTRSHFT)); /* must be page aligned */

	for (sipp = &swapinfo; (osip = *sipp) != NULL; sipp = &osip->si_next) {
		if ((strcmp(osip->si_pname, name) == 0) 
		  && (osip->si_soff == soff) && (osip->si_flags == 0)) {
			VN_HOLD(osip->si_svp);
			return osip->si_svp;
		}
	}
	
	return NULL;
}


/*
 * New system call to manipulate swap files.
 */

struct swapcmda	{
	int	sc_dummy;	/* uadmin cmd argument	*/
	int	sc_cmd;		/* command code for swapctl	*/
	void	*sc_arg;	/* argument pointer for swaptcl	*/
};

int
swapctl(uap, rvp)
	register struct swapcmda *uap;
	rval_t *rvp;
{
	register struct swapinfo *sip;
	register error = 0;

	struct swapent st, *ust;
	struct swapres sr;
	struct vnode *vp;
	register int cnt = 0;
	int length;
	char *swapname;

	switch (uap->sc_cmd) {
	case SC_GETNSWP:

		rvp->r_val1 = nswapfiles;
		return(0);

	case SC_LIST:
		if (copyin((caddr_t)uap->sc_arg, 
		  (caddr_t)&length, sizeof(int)) != 0)
			return(EFAULT);

		/*
		 * Return an error if we don't have enough space
		 * for the whole table.  We also have to check
		 * during the loop, since we can add entries
		 * on the fly.
		 */
		if (length < nswapfiles)
			return(ENOMEM);

		ust = (swapent_t *)((swaptbl_t *)uap->sc_arg)->swt_ent;

		sip = swapinfo;
		do {
			if (sip->si_flags & ST_DELETED)
				continue;
			if (length-- == 0)	/* no place for more entries */
				break;
			if (copyin((caddr_t)ust, (caddr_t)&st, sizeof(swapent_t)) != 0)
				return(EFAULT);
			st.ste_flags = sip->si_flags;
			st.ste_length = (sip->si_eoff-sip->si_soff) >> SCTRSHFT;
			st.ste_start = sip->si_soff >> SCTRSHFT;
			st.ste_pages = sip->si_npgs;
			st.ste_free = sip->si_nfpgs;

			if (copyout((caddr_t)&st, (caddr_t)ust, sizeof(swapent_t)) != 0) {
				rvp->r_val1 = cnt;
				return (EFAULT);
			}
			if (sip->si_pname)
				if (copyout(sip->si_pname, st.ste_path, strlen(sip->si_pname)) != 0) {
					rvp->r_val1 = cnt;
					return (EFAULT);
				}
			ust++;
			cnt++;
		} while ((sip = sip->si_next) != NULL);
		rvp->r_val1 = cnt;
		if (sip)
			return (ENOMEM);
		return 0;

	case SC_ADD:
	case SC_REMOVE:
		break;
	default:
		return(EINVAL);
	}
	if (!suser(u.u_cred))
		return(EPERM);

	if (copyin((caddr_t)uap->sc_arg, (caddr_t)&sr, sizeof(swapres_t)))
		return(EFAULT);

	/* Allocate the space to read in pathname */
	if ((swapname = (char *)kmem_alloc(MAXPATHLEN, KM_NOSLEEP)) == NULL)
		return(ENOMEM);

	error = copyinstr(sr.sr_name, swapname, MAXPATHLEN, 0);
	if (error)
		goto out;

	error = lookupname(swapname, UIO_SYSSPACE, FOLLOW, NULLVPP, &vp);
	if (error) {
		if (uap->sc_cmd == SC_ADD)
			goto out;
		/* see if we match by name */
		vp = swapdel_byname(swapname, (uint)sr.sr_start);
		if (vp == NULL)
			goto out;
	}

	if (vp->v_flag & (VNOMAP | VNOSWAP)) {
		VN_RELE(vp);
		error = ENOSYS;
		goto out;
	}

	switch (vp->v_type) {
	case VBLK: 
		break;

	case VREG:
		if (vp->v_vfsp->vfs_flag & VFS_RDONLY) {
			error = EROFS;
		} else 
			error = VOP_ACCESS(vp, VREAD|VWRITE, 0, u.u_cred);
		break;

	case VDIR:
		error = EISDIR;
		break;
	default:
		error = ENOSYS;
		break;
	}
	if (error == 0) {
		if (uap->sc_cmd == SC_REMOVE)
			error = swapdel(vp, sr.sr_start);
		else
			error = swapadd(vp, sr.sr_start, sr.sr_length, swapname);
	}
	VN_RELE(vp);
out:
	kmem_free(swapname, MAXPATHLEN);
	return(error);

}

/*
 * Add a new swap file.
 */
STATIC int
swapadd(vp, lowblk, nblks, swapname)
	struct vnode *vp;
	uint	lowblk;
	uint	nblks;
	char	*swapname;
{
	register struct anon *ap, *ap2;
	register struct swapinfo **sipp, *nsip;
	register struct vnode *cvp;
	struct vattr vattr;
	register uint pages;
	uint soff, eoff, off;
	int error;
	int dummy;

	u.u_error = 0;		/* XXX - 386 SCSI open bug fix */
	if (error = VOP_OPEN(&vp, FREAD|FWRITE, u.u_cred)) {
		return error;
	}
	cvp = common_specvp(vp);

	/* 
	 * Get partition size. Return error if empty partition,
	 * or if request does not fit within the partition.
	 * If this is the first swap device, we can reduce
	 * the size of the swap area to match what is 
	 * available.  This can happen if the system was built
	 * on a machine with a different size swap partition.
	 */
	vattr.va_mask = AT_SIZE;
	if (error = VOP_GETATTR(cvp, &vattr, ATTR_COMM, u.u_cred))
		goto out;

	soff = lowblk << SCTRSHFT; 
	if ((vattr.va_size == 0) || (soff >= vattr.va_size)) {
		error = EINVAL;
		goto out;
	}

	eoff = soff + (nblks << SCTRSHFT);
	if (eoff > vattr.va_size) {
		uint endblk;
		if (swapinfo != NULL) {
			error = EINVAL;
			goto out;
		}
		eoff = vattr.va_size;
		endblk = eoff >> SCTRSHFT;
		cmn_err(CE_WARN,
		 "swapadd: configured values for swplo (%d) and nswap (%d) are too large for",
		  lowblk, nblks);
		cmn_err(CE_CONT,
		 "partition size (%d). nswap reduced to %d blocks\n",
		  endblk, endblk - lowblk);
	}

	/*
	 * The starting and ending offsets must be page aligned.
	 * Round soff up to next page boundary, round eoff
	 * down to previous page boundary.
	 */
	soff = ctob(btoc(soff)); 
	eoff = ctob(btoct(eoff));
	if (soff >= eoff) {
		error = EINVAL;
		goto out;
	}

	pages = btoc(eoff - soff);

	for (sipp = &swapinfo; (nsip = *sipp) != NULL; sipp = &nsip->si_next) {
		/* See if this swapfile aleady exists in one form
		 * or another. It can be in many stages of existance,
		 * being added by another process, in use, being
		 * deleted, or been deleted (but the data structures
		 * are still around.).
		 */
		if (nsip->si_flags & ST_DELETED) {
			if (nsip->si_soff == soff && nsip->si_npgs == pages
			  && (strcmp(nsip->si_pname, swapname) == 0)) {

				/* We are adding a device that
				 * we were trying to delete. 
				 * We are done with the delete,
				 * but haven't free'd up all the
				 * data structures yet, so we need
				 * to undo the deletion, turn the
				 * vnode swap flag back on,
				 * and keep the device open.
				 * Add our pages back into the
				 * total available for swap.
				 */
				VN_HOLD(vp);
				nsip->si_svp = vp;
				nsip->si_vp = cvp;
				nsip->si_flags &= ~ST_DELETED;
				undelswap(nsip);
				nsip->si_flags &= ~ST_INDEL;
				anoninfo.ani_max += nsip->si_npgs;
				anoninfo.ani_free += nsip->si_npgs;
				availsmem += nsip->si_npgs;
				cvp->v_flag |= VISSWAP;
				nswapfiles++;
				return (0);
			}
			continue;
		}
		if (nsip->si_vp == cvp) {
			if (nsip->si_soff == soff && nsip->si_npgs == pages
			  && (nsip->si_flags & ST_DOINGDEL)) {

				/* We are adding a device that
				 * we were trying to delete. 
				 * We are still in the middle
				 * of the deletion, so just turn off
				 * the flag so the deltion will stop.
				 * Swapdel will add our pages back 
				 * into the total available for swap.
				 */
				nsip->si_flags &= ~ST_DOINGDEL;
				goto out;
			}
			/* disallow overlapping swap files */
			if ((soff < nsip->si_eoff) && (eoff > nsip->si_soff)) {
				error = EEXIST;
				goto out;
			}
		}
	}

	VN_HOLD(vp);

	/*
	 * Force blocks to be allocated for the entire file.
	 */
	error = VOP_ALLOCSTORE(vp, soff, eoff - soff, u.u_cred);
	if (error == ENOSYS) {
		for (off = soff; off < eoff; off += PAGESIZE) {
			error = vn_rdwr(UIO_WRITE, vp, (char *)&dummy, 1, off,
				UIO_SYSSPACE, RLIM_INFINITY, u.u_cred, &dummy);
			if (error)
				break;
		}
	}
	if (error) {
		VN_RELE(vp);
		goto out;
	}

	nsip = (struct swapinfo *)kmem_zalloc(sizeof (struct swapinfo), KM_SLEEP);
	nsip->si_vp = cvp;
	nsip->si_svp = vp;	/* snode */
	nsip->si_flags = ST_INDEL;

	/*
	 * Add this swap device to end of list.
	 * We may have slept and list may have changed.
	 */
	for (sipp = &swapinfo; *sipp != NULL; sipp = &(*sipp)->si_next) ;
	*sipp = nsip;
	if (silast == NULL)	/* first swap device */
		silast = nsip;

	cvp->v_flag |= VISSWAP;

	nsip->si_soff = soff;
	nsip->si_eoff = eoff;
	nsip->si_anon = (struct anon *)kmem_zalloc(pages*sizeof (struct anon), KM_SLEEP);
	nsip->si_eanon = nsip->si_anon + (pages - 1);
	nsip->si_pname = (char *) kmem_zalloc((strlen(swapname) + 1), KM_SLEEP);
	bcopy(swapname, nsip->si_pname, strlen(swapname));

	/*
	 * ap2 now points to the first usable slot in the swap area.
	 * Set up free list links so that the head of the list is at
	 * the front of the usable portion of the array.
	 */
	ap = nsip->si_eanon;
	ap2 = nsip->si_anon;
	while (--ap >= ap2)
		ap->un.an_next = ap + 1;
	nsip->si_free = ap + 1;
	nsip->si_npgs = pages;
	nsip->si_nfpgs = pages;

	anoninfo.ani_free += pages;
	anoninfo.ani_max += pages;
	availsmem += pages;
	nswapfiles++;
	nsip->si_flags &= ~ST_INDEL;

	return 0;

out:
	(void) VOP_CLOSE(vp, FREAD|FWRITE, 1, (off_t) 0, u.u_cred);

	return error;
}

/*
 * Delete a swap file.
 */
STATIC int
swapdel(vp, lowblk)
	register struct vnode *vp;
	register int	lowblk;	/* Low block number of area to delete. */
{
	register struct swapinfo **sipp, *osip, **psipp = NULL;
	register struct vnode *cvp;
	register int	ok = 0;
	register uint	soff;

	/*
	 * Find the swap file entry for the file to
	 * be deleted. Skip any entries that are in 
	 * transition.  Also, make sure that we don't
	 * delete the last swap file.  
	 */

	cvp = common_specvp(vp);

	soff = ctob(btoc(lowblk << SCTRSHFT)); /* must be page aligned */

	for (sipp = &swapinfo; (osip = *sipp) != NULL; sipp = &osip->si_next) {
		if ((osip->si_vp == cvp)
		  && (osip->si_soff == soff) && (osip->si_flags == 0)) {
			psipp = sipp;
		} else if ((osip->si_flags & ST_INDEL) == 0)
			ok++;
	}
	
	/*
	 * If the file was not found, error.
	 */

	if (psipp == NULL)
		return EINVAL;

	/*
	 * If we're trying to delete the last swap file, error.
	 */
	
	if (!ok)
		return ENOMEM;
	
	osip = *psipp;

	/*
	 * Do not delete if we will be low on swap pages.
	 */

	if ((anoninfo.ani_max - anoninfo.ani_resv)
	    < tune.t_minasmem + osip->si_npgs) {
		cmn_err(CE_WARN, "swapdel - too few free pages");
		return ENOMEM;
	}

	/*
	 * Set the delete flag.  We will not
	 * allocate any more pages from this device.
	 * ST_DOINGDEL limits us to one process
	 * doing a swapdel per swap file.
	 */
	osip->si_flags |= ST_INDEL|ST_DOINGDEL;
	anoninfo.ani_max -= osip->si_npgs;
	anoninfo.ani_free -= osip->si_npgs;
	availsmem -= osip->si_npgs;

	/* 
	 * try to remove pages - this may sleep, and
	 * swapadd may add this file back and turn off ST_DOINGDEL.
	 * In that case, we stop trying to delete it.
	 */

	if (osip->si_nfpgs < osip->si_npgs)
		delswap(osip);

	ASSERT (osip->si_nfpgs <= osip->si_npgs);

	/* 
	 * If we are not still trying to delete 
	 * this swap device, because we have
	 * swapadd'ed it again, then we are done.
	 */

	if ((osip->si_flags & ST_DOINGDEL) == 0) {
		undelswap(osip);
		osip->si_flags &= ~ST_INDEL;
		anoninfo.ani_max += osip->si_npgs;
		anoninfo.ani_free += osip->si_npgs;
		availsmem += osip->si_npgs;
		return (0);
	}
	osip->si_flags &= ~ST_DOINGDEL;

	/*
	 * If we have finally freed all the pages,
	 * we can delete all the data structures.
	 * If the front is not yet free, the data 
	 * structures will be free'd by swap_free.
	 */
	vp = osip->si_svp;
	if (osip->si_nfpgs == osip->si_npgs)
		swapinfo_free(osip);
	else
		osip->si_flags |= ST_DELETED;

	(void) VOP_CLOSE(vp, FREAD|FWRITE, 1, (off_t) 0, u.u_cred);
	cvp->v_flag &= ~VISSWAP;
	VN_RELE(vp);

	nswapfiles--;

	return 0;
}

/*
 * We are finished with this swap resource and
 * can not delete all the data structures.
 */
STATIC void
swapinfo_free(sip)
	register struct swapinfo *sip;
{
	register struct swapinfo **sipp;
	register uint	pages;

	sipp = &swapinfo; 
	ASSERT(*sipp != NULL);

	do {
		if (*sipp == sip)
			break;
		sipp = &(*sipp)->si_next;
	} while (*sipp != NULL);

	if (*sipp == NULL)
		cmn_err(CE_PANIC, "swap device vp %x off %x gone",
			sip->si_vp, sip->si_soff);

	if (silast == sip)
		if ((silast = sip->si_next) == NULL)
			silast = swapinfo;
	*sipp = sip->si_next;

	/* si_npgs may not be true number of anon entries allocated */
	pages = btoc(sip->si_eoff - sip->si_soff);
	kmem_free(sip->si_anon, pages*sizeof(struct anon));
	kmem_free(sip->si_pname, strlen(sip->si_pname) + 1);
	kmem_free((caddr_t)sip, sizeof(*sip));
}

#define delswap_done(sip) ((sip->si_nfpgs == sip->si_npgs) || ((sip->si_flags & ST_DOINGDEL) == 0))

/*
 * Free up swap space on the swap device being deleted.
 * We make two passes thru the anon slots.  On the first
 * pass we only handle pages that are already in memory,
 * so we don't force any of these pages out if we have
 * to read some pages in.
 */

STATIC void
delswap(sip)
	struct swapinfo *sip;
{
	register struct anon *new, *old;
	register page_t *opp;
	register struct anon *eanon;
	register struct anon *cur;
	struct vnode *nvp, *ovp;
	u_int noff, ooff;
	int i, pass, dummy, resid;

	for (pass = 0; pass < 2 ; pass++) {

	  cur = sip->si_anon;
	  eanon = sip->si_eanon;

	  for ( ; cur <= eanon; cur++ ) {

		if (delswap_done(sip))
			return;

		/* 
		 * If an_bap is not NULL, we can be an 
		 * anon slot that points to the backing
		 * anon slot, or the anon slot that is 
		 * being pointed at.
		 */
lookagain:
		if (cur->an_bap) {
			register struct anon *bap = cur->an_bap;

			ASSERT(bap->an_bap == cur);
			if (cur->an_refcnt) {
				/*
				 * This is the anon slot (front).
				 * It has already been relocated.
				 */
				ASSERT(bap->an_refcnt == 0);
				continue;
			} else {
				/*
				 * This is the object of indirection
				 * (the back). We relocate the front 
				 * instead of this entry, and free
				 * this up when we are done.
				 */
				ASSERT(bap->an_refcnt > 0);
				old = bap;
			}
		} else {
			if (cur->an_refcnt == NULL)
				continue;
			old = cur;
		}

		if (old->an_flag & ALOCKED) {
			while (old->an_flag & ALOCKED) {
				old->an_flag |= AWANT;
				(void) sleep((caddr_t)old, PINOD);
			}
			goto lookagain;
		} else
			old->an_flag |= ALOCKED;

		swap_xlate(old, &ovp, &ooff);
delagain:
		opp = page_lookup(ovp, ooff);

		if (opp == NULL) {
			if (pass == 0) {
				AUNLOCK(old);
				continue;
			}
		}
		/*
		 * If this page has been swapped out,
		 * then bring it back in.
		 */
		while (opp == NULL) {
			vn_rdwr(UIO_READ, ovp, &dummy, 1, ooff,
				UIO_SYSSPACE, 0, (long)0, 0, &resid);
			opp = page_lookup(ovp, ooff);
		}

		/*
		 * We must remove any pages for this vp
		 * from segmap since we will change 
		 * the page identity below.
		 */
		segmap_flush(segkmap, ovp);

		/*
		 * we may have slept, so it is time
		 * to check that we still need to handle
		 * this anon slot.
		 */
		if (old->an_refcnt == 0) {
			AUNLOCK(old);
			continue;
		}

		PAGE_HOLD(opp);
		page_lock(opp);

		ASSERT(opp->p_vnode == ovp && opp->p_offset == ooff);
		ASSERT(!opp->p_free && !opp->p_gone);

		new = swap_alloc();
		if (new == NULL) 
			cmn_err(CE_PANIC, "delswap: out of anon");

		swap_xlate(new, &nvp, &noff);

		page_hashout(opp);	/* destroy old name for page */

		/* enter new name for old page */
		while (page_enter(opp, nvp, noff)) {
			page_t *npp;

			npp = page_find(nvp, noff);
			if (npp != NULL) 
				page_abort(npp);
		}

		/* We've changed the page's identity;
		   we must re-initialize the p_dblist[] fields. */
		for (i = 0; i < PAGESIZE/NBPSCTR; i++)
			opp->p_dblist[i] = -1;

		/* set up indirection */
		old->an_bap = new;
		new->an_bap = old;
		if (cur != old) {
			/*
			 * "old" used to point to "cur".
			 * We have relocated old to point
			 * to new, and now we free up cur.
			 */
			ASSERT(cur->an_bap == old);
			ASSERT(cur->an_refcnt == 0);
			cur->an_bap = NULL;
			/* inline swap_free */
			cur->un.an_next = sip->si_free;
			sip->si_free = cur;
			sip->si_nfpgs++;
		}

		opp->p_mod = 1;			/* mark page as modified */

		page_unlock(opp);
		PAGE_RELE(opp);
		AUNLOCK(old);
		ASSERT(opp->p_free == 0);	/* page should not be free */

	  }
	}

}

#define undelswap_done(sip) (sip->si_nfpgs == sip->si_npgs) 

/*
 * Break the indirections set up so far. This is used
 * when we swapadd back a resource that we have deleted
 * but have not freed the data structures, and when we
 * stop a swapdel in the middle of deleting a resource.
 * We make two passes thru the anon slots.  On the first
 * pass we only handle pages that are already in memory,
 * so we don't force any of these pages out if we have
 * to read some pages in.
 */

STATIC void
undelswap(sip)
	struct swapinfo *sip;
{
	register struct anon *dir, *ind;
	register page_t *opp;
	register struct anon *eanon;
	struct vnode *nvp, *ovp;
	u_int noff, ooff;
	int i, pass, dummy, resid;
	
	nvp = sip->si_vp;

	for (pass = 0; pass < 2 ; pass++) {

	  dir = sip->si_anon;
	  eanon = sip->si_eanon;
	  noff = sip->si_soff;

	  for ( ; dir <= eanon; dir++, noff += PAGESIZE ) {

		if (undelswap_done(sip))
			return;

lookagain:
		if ((dir->an_refcnt == 0) || (dir->an_bap == NULL))
			continue;

		ind = dir->an_bap;
		ASSERT(ind->an_bap == dir);
		ASSERT(ind->an_refcnt == 0);

		if (dir->an_flag & ALOCKED) {
			while (dir->an_flag & ALOCKED) {
				dir->an_flag |= AWANT;
				(void) sleep((caddr_t)dir, PINOD);
			}
			goto lookagain;
		} else
			dir->an_flag |= ALOCKED;

		/* swap_xlate finds the indirect page */
		swap_xlate(dir, &ovp, &ooff);
undelagain:
		opp = page_lookup(ovp, ooff);

		if (opp == NULL) {
			if (pass == 0) {
				AUNLOCK(dir);
				continue;
			}
		}
		/*
		 * If this page has been swapped out,
		 * then bring it back in.
		 */
		while (opp == NULL) {
			vn_rdwr(UIO_READ, ovp, &dummy, 1, ooff,
				UIO_SYSSPACE, 0, (long)0, 0, &resid);
			opp = page_lookup(ovp, ooff);
		}

		/*
		 * We must remove any pages for this vp
		 * from segmap since we will change 
		 * the page identity below.
		 */
		segmap_flush(segkmap, ovp);

		/*
		 * we may have slept, so it is time
		 * to check that we still need to handle
		 * this anon slot.
		 */
		if (dir->an_refcnt == 0) {
			AUNLOCK(dir);
			continue;
		}

		PAGE_HOLD(opp);
		page_lock(opp);

		ASSERT(opp->p_vnode == ovp && opp->p_offset == ooff);
		ASSERT(!opp->p_free);

		page_hashout(opp);	/* destroy name for (indirect) page */

		/* enter new (original) name for direct page */
		while (page_enter(opp, nvp, noff)) {
			page_t *npp;

			npp = page_find(nvp, noff);
			if (npp != NULL) 
				page_abort(npp);
		}

		/* We've changed the page's identity;
		   we must re-initialize the p_dblist[] fields. */
		for (i = 0; i < PAGESIZE/NBPSCTR; i++)
			opp->p_dblist[i] = -1;

		ASSERT(ind->an_bap == dir);
		ASSERT(ind->an_refcnt == 0);
		/* break the indirection */
		dir->an_bap = NULL;
		ind->an_bap = NULL;
		swap_free(ind);

		opp->p_mod = 1;			/* mark page as modified */

		page_unlock(opp);
		PAGE_RELE(opp);
		AUNLOCK(dir);
		ASSERT(opp->p_free == 0);	/* page should not be free */

	  }
	}

}

/*
 * Obsolete sys3b function: manipulate swap files.
 * This only works on block devices.
 */

int
swapfunc(si)
	register swpi_t	*si;
{
	register error = 0;
	register struct swapinfo *sip;
	register char *ustp;
	swpt_t st;
	struct vnode *vp;
	char *swapname;

	switch (si->si_cmd) {
	case SI_LIST:

		ustp = si->si_buf;
		sip = swapinfo;
		do {
			/* Don't report this device if it has been deleted
			 * but the data structures have not been freed.
			 */
			if (sip->si_flags & ST_DELETED)
				continue;
			st.st_dev = (o_dev_t)cmpdev(sip->si_vp->v_rdev);
			st.st_flags = sip->si_flags;
			st.st_swplo = sip->si_soff >> SCTRSHFT;
			st.st_npgs = sip->si_npgs;
			st.st_nfpgs = sip->si_nfpgs;
			st.st_ucnt = 1;
			st.pad1 = st.pad2 = st.pad3 = 0;
			if (copyout((caddr_t)&st, ustp, sizeof(swpt_t)) != 0) {
				error = EFAULT;
				break;
			}
			ustp += sizeof(swpt_t);
		} while ((sip = sip->si_next) != NULL);
		return error;

	case SI_ADD:
	case SI_DEL:
		if (!suser(u.u_cred)) {
			return EPERM;
		}

		if ((swapname = (char *)kmem_alloc(MAXPATHLEN, 
		    KM_NOSLEEP)) == NULL) {
			return ENOMEM;
		}
		error = copyinstr(si->si_buf, swapname, MAXPATHLEN, 0);
		if (error)
			break;

		error = lookupname(swapname, UIO_SYSSPACE, FOLLOW, NULLVPP, &vp);
		if (error)
			break;

		if (vp->v_type == VBLK) {
			if (si->si_cmd == SI_DEL)
				error = swapdel(vp, si->si_swplo);
			else
				error = swapadd(vp, si->si_swplo, si->si_nblks,
						swapname);
		} else 
			error = ENOTBLK;
		VN_RELE(vp);
		break;
	default:
		return(EINVAL);
	}

	kmem_free(swapname, MAXPATHLEN);
	return error;
}


/*
 * Debugging routine only.
 */
void
sxlate(ap)
	register struct anon *ap;
{
	register struct swapinfo *sip = swapinfo;

	if (ap->an_bap)
		ap = ap->an_bap;
	do {
		if (sip->si_anon <= ap && ap <= sip->si_eanon) {
			cmn_err(CE_CONT, "vp=%x offset=%x\n",
				sip->si_vp,
				sip->si_soff + 
					((ap - sip->si_anon) << PAGESHIFT));
			return;
		}
	} while ((sip = sip->si_next) != NULL);
	cmn_err(CE_CONT, "%x not an anon pointer\n", ap);
	return;
}


/*
 * Add the initial swap device.
 */
void
swapconf()
{
	register int error;

	if (error = lookupname(swapfile.bo_name, UIO_SYSSPACE,
	  FOLLOW, NULLVPP, &swapfile.bo_vp))
		cmn_err(CE_PANIC, "swapconf lookupname %s failed - error %d\n",
			swapfile.bo_name, error);

	/*
	 * bo_offset, and bo_size have been 
	 * filled in by cunix.
	 */
	(void)strcpy(swapfile.bo_fstype, rootfstype);
	swapfile.bo_flags |= BO_VALID;

	if (swapadd(swapfile.bo_vp, swapfile.bo_offset,
		 swapfile.bo_size, swapfile.bo_name))
		cmn_err(CE_PANIC, "swapconf - swapadd failed");
}
