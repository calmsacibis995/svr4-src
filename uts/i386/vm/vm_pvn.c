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

#ident	"@(#)kern-vm:vm_pvn.c	1.3.1.9"

/*
 * VM - paged vnode.
 *
 * This file supplies vm support for the vnode operations that deal with pages.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/inline.h"
#include "sys/time.h"
#include "sys/buf.h"
#include "sys/vnode.h"
#include "sys/uio.h"
#include "sys/vmmeter.h"
#include "sys/mman.h"
#include "sys/vfs.h"
#include "sys/cred.h"
#include "sys/kmem.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/sysinfo.h"
#include "sys/vmsystm.h"
#include "vm/trace.h"

#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/rm.h"
#include "vm/pvn.h"
#include "vm/page.h"
#include "vm/seg_map.h"
#include "sys/proc.h"		/* XXX - needed for PREEMPT() */
#include "sys/disp.h"		/* XXX - needed for PREEMPT() */

extern void	bp_mapout();
extern int	kzero();
extern void	pageio_done();

STATIC int pvn_nofodklust = 0;
STATIC int pvn_range_noklust = 0;

/*
 * Find the largest contiguous block which contains `addr' for file offset
 * `offset' in it while living within the file system block sizes (`vp_off'
 * and `vp_len') and the address space limits for which no pages currently
 * exist and which map to consecutive file offsets.
 */
page_t *
pvn_kluster(vp, off, seg, addr, offp, lenp, vp_off, vp_len, isra)
	struct vnode *vp;
	register u_int off;
	register struct seg *seg;
	register addr_t addr;
	u_int *offp, *lenp;
	u_int vp_off, vp_len;
	int isra;
{
	register int delta, delta2;
	register page_t *pp;
	page_t *plist = NULL;
	addr_t straddr;
	int bytesavail;
	u_int vp_end;

	ASSERT(off >= vp_off && off < vp_off + vp_len);

	/*
	 * We only want to do klustering/read ahead if there
	 * is more than minfree pages currently available.
	 */
	if (freemem - minfree > 0)
		bytesavail = ptob(freemem - minfree);
	else
		bytesavail = 0;

	if (bytesavail == 0) {
		if (isra)
			return ((page_t *)NULL);	/* ra case - give up */
		else
			bytesavail = PAGESIZE;		/* just pretending */
	}

	if (bytesavail < vp_len) {
		/*
		 * Don't have enough free memory for the
		 * max request, try sizing down vp request.
		 */
		delta = off - vp_off;
		vp_len -= delta;
		vp_off += delta;
		if (bytesavail < vp_len) {
			/*
			 * Still not enough memory, just settle for
			 * bytesavail which is at least PAGESIZE.
			 */
			vp_len = bytesavail;
		}
	}

	vp_end = vp_off + vp_len;
	ASSERT(off >= vp_off && off < vp_end);

	if (page_exists(vp, off))
		return ((page_t *)NULL);		/* already have page */

	if (vp_len <= PAGESIZE || pvn_nofodklust) {
		straddr = addr;
		*offp = off;
		*lenp = MIN(vp_len, PAGESIZE);
	} else {
		/* scan forward from front */
		for (delta = PAGESIZE; off + delta < vp_end;
		    delta += PAGESIZE) {
			/*
			 * Call back to the segment driver to verify that
			 * the klustering/read ahead operation makes sense.
			 */
			if ((*seg->s_ops->kluster)(seg, addr, delta))
				break;		/* page not file extension */
			if (page_exists(vp, off + delta))
				break;		/* already have page */
		}
		delta2 = delta;

		/* scan back from front */
		for (delta = 0; off + delta > vp_off; delta -= PAGESIZE) {
			if (page_exists(vp, off + delta - PAGESIZE))
				break;		/* already have the page */
			/*
			 * Call back to the segment driver to verify that
			 * the klustering/read ahead operation makes sense.
			 */
			if ((*seg->s_ops->kluster)(seg, addr, delta - PAGESIZE))
				break;		/* page not eligible */
		}

		straddr = addr + delta;
		*offp = off = off + delta;
		*lenp = delta2 - delta;
		ASSERT(off >= vp_off);

		if ((vp_off + vp_len) < (off + *lenp)) {
			ASSERT(vp_end > off);
			*lenp = vp_end - off;
		}
	}

	/*
	 * Allocate pages for <vp, off> at <seg, addr> for delta bytes.
	 * Note that for the non-read ahead case we might not have the
	 * memory available right now so that rm_allocpage operation could
	 * sleep and someone else might race to this same spot if the
	 * vnode object was not locked before this routine was called.
	 */
	delta2 = *lenp;
	delta = roundup(delta2, PAGESIZE);
	/* `pp' list kept */
	pp = rm_allocpage(seg, straddr, (u_int)delta, P_CANWAIT);

	plist = pp;
	do {
		pp->p_intrans = 1;
		pp->p_pagein = 1;

#ifdef TRACE
		{
			addr_t taddr = straddr + (off - *offp);

			trace3(TR_SEG_KLUSTER, seg, taddr, isra);
			trace6(TR_SEG_ALLOCPAGE, seg, taddr, TRC_SEG_UNK,
			    vp, off, pp);
		}
#endif /* TRACE */
		if (page_enter(pp, vp, off)) {		/* `pp' locked if ok */
			/*
			 * Oops - somebody beat us to the punch
			 * and has entered the page before us.
			 * To recover, we use pvn_fail to free up
			 * all the pages we have already allocated
			 * and we return NULL so that whole operation
			 * is attempted over again.  This should never
			 * happen if the caller of pvn_kluster does
			 * vnode locking to prevent multiple processes
			 * from creating the same pages as the same time.
			 */
			pvn_fail(plist, B_READ);
			return ((page_t *)NULL);
		}
		off += PAGESIZE;
	} while ((pp = pp->p_next) != plist);

	return (plist);
}

/*
 * Entry point to be use by page r/w subr's and other such routines which
 * want to report an error and abort a list of pages setup for pageio
 * which do not do though the normal pvn_done processing.
 */
void
pvn_fail(plist, flags)
	page_t *plist;
	int flags;
{
	static struct buf abort_buf;
	struct buf *bp;
	page_t *pp;
	int len;
	int s;

	len = 0;
	pp = plist;
	do {
		len += PAGESIZE;
	} while ((pp = pp->p_next) != plist);

	bp = &abort_buf;
	s = splimp();
	while (bp->b_pages != NULL) {
		(void) sleep((caddr_t)&bp->b_pages, PSWP+2);
	}
	(void) splx(s);
	/* ~B_PAGEIO is a flag to pvn_done not to pageio_done the bp */
	bp->b_flags = B_KERNBUF | B_ERROR | B_ASYNC | (flags & ~B_PAGEIO);
	bp->b_pages = plist;
	bp->b_bcount = len;
	pvn_done(bp);			/* let pvn_done do all the work */
	if (bp->b_pages != NULL) {
		/* XXX - this should never happen, should it be a panic? */
		cmn_err(CE_NOTE, "pvn_fail: page list not empty! - plist %x\n",
			plist);
		bp->b_pages = NULL;
	}
	wakeprocs((caddr_t)&bp->b_pages, PRMPT);
}

/*
 * Routine to be called when pageio's complete.
 * Can only be called from process context, not
 * from interrupt level.
 */
void
pvn_done(bp)
	register struct buf *bp;
{
	register page_t *pp;
	register int bytes;

	/*
	 * Release any I/O mappings to the pages described by the
	 * buffer that are finished before processing the completed I/O.
	 */
	if (bp->b_flags & B_REMAPPED)
		bp_mapout(bp);

	/*
	 * Handle each page in the I/O operation.
	 */
	for (bytes = 0; bytes < bp->b_bcount; bytes += PAGESIZE) {
		struct vnode *vp;
		u_int off;
		register int s;

		pp = bp->b_pages;
		if (pp->p_nio > 1) {
			/*
			 * There were multiple IO requests outstanding
			 * for this particular page.  This can happen
			 * when the file system block size is smaller
			 * than PAGESIZE.  Since there are more IO
			 * requests still outstanding, we don't process
			 * the page given on the buffer now.
			 */
			if (bp->b_flags & B_ERROR) {
				if ((bp->b_flags & B_READ) ||
				    pp->p_keepcnt == 1) {
					trace3(TR_PG_PVN_DONE, pp, pp->p_vnode,
						pp->p_offset);
					page_abort(pp);	/* assumes no waiting */
				} else {
					pp->p_mod = 1;
				}
			}
			pp->p_nio--;
			break;
			/* real page locked for the other io operations */
		}

		pp->p_nio = 0;
		page_sub(&bp->b_pages, pp);

		vp = pp->p_vnode;
		off = pp->p_offset;
		pp->p_intrans = 0;
		pp->p_pagein = 0;

		PAGE_RELE(pp);
		/*
		 * Verify the page identity before checking to see
		 * if the page was freed by PAGE_RELE().  This must
		 * be protected by splvm() to prevent the page from
		 * being ripped away at interrupt level.
		 */
		s = splvm();
		if (pp->p_vnode != vp || pp->p_offset != off || pp->p_free) {
			(void) splx(s);
			continue;
		}
		(void) splx(s);

		/*
		 * Check to see if the page has an error.
		 */
		if ((bp->b_flags & (B_ERROR|B_READ)) == (B_ERROR|B_READ)) {
			page_abort(pp);
			continue;
		}

		/*
		 * Check if we are to be doing invalidation.
		 * XXX - Failed writes with B_INVAL set are
		 * not handled appropriately.
		 */
		if ((bp->b_flags & B_INVAL) != 0) {
			page_abort(pp);
			continue;
		}

		if ((bp->b_flags & (B_ERROR | B_READ)) == B_ERROR) {
			/*
			 * Write operation failed.  Abort the page
			 * if we can.  Otherwise, set the mod bit again
			 * and catch it later.
			 */
			if (pp->p_keepcnt == 0)
				page_abort(pp);
			else
				pp->p_mod = 1;
		}

		if (bp->b_flags & B_FREE) {
			cnt.v_pgpgout++;
			vminfo.v_pgpgout++;
			if (pp->p_keepcnt == 0
			    && pp->p_lckcnt == 0 && pp->p_cowcnt == 0) {
				/*
				 * Check if someone has reclaimed the
				 * page.  If no ref or mod, no one is
				 * using it so we can free it.
				 * The rest of the system is careful
				 * to use hat_ghostunload to unload
				 * translations set up for IO w/o
				 * affecting ref and mod bits.
				 */
				if (pp->p_mod == 0 && pp->p_mapping)
					hat_pagesync(pp);
				if (!pp->p_ref && !pp->p_mod) {
					if (pp->p_mapping)
						hat_pageunload(pp);
					page_free(pp,
					    (int)(bp->b_flags & B_DONTNEED));
					cnt.v_dfree++;
					vminfo.v_dfree++;
				} else {
					page_unlock(pp);
					cnt.v_pgrec++;
					vminfo.v_pgrec++;
				}
			} else {
				page_unlock(pp);
			}
			continue;
		}

		page_unlock(pp);		/* a read or write */
	}

	/*
	 * Count pageout operations if applicable.  Release the
	 * buf struct associated with the operation if async & pageio.
	 */
	if (bp->b_flags & B_FREE) {
		cnt.v_pgout++;
		vminfo.v_pgout++;
	}
	if ((bp->b_flags & (B_ASYNC | B_PAGEIO)) == (B_ASYNC | B_PAGEIO))
		pageio_done(bp);
}
/*
 * Flags are composed of {B_ASYNC, B_INVAL, B_FREE, B_DONTNEED, B_DELWRI}
 * B_DELWRI indicates that this page is part of a kluster operation and
 * is only to be considered if it doesn't involve any waiting here.
 * Returns non-zero if page added to dirty list.
 *
 * NOTE:  The caller must ensure that the page is not on the free list.
 */
STATIC int
pvn_getdirty(pp, dirty, flags)
	register page_t *pp, **dirty;
	int flags;
{
	struct vnode *vp;
	u_int offset;

	ASSERT(pp->p_free == 0);
	vp = pp->p_vnode;
	offset = pp->p_offset;

	/*
	 * If trying to invalidate a logically locked page, forget it.
	 */
	if ((flags & B_INVAL|B_FREE) && (pp->p_lckcnt != 0 || pp->p_cowcnt != 0))
		return (0);

	if ((flags & B_DELWRI) != 0 && (pp->p_keepcnt != 0 || pp->p_lock)) {
		/*
		 * This is a klustering case that would
		 * cause us to block, just give up.
		 */
		return (0);
	}

	if (pp->p_intrans && (flags & (B_INVAL | B_ASYNC)) == B_ASYNC) {
		/*
		 * Don't bother waiting for an intrans page if we are not
		 * doing invalidation and this is an async operation
		 * (the page will be correct when the current io completes).
		 */
		return (0);
	}

	/*
	 * If i/o is in progress on the page or we have to
	 * invalidate or free the page, wait for the page keep
	 * count to go to zero.
	 */
	if (pp->p_intrans || (flags & (B_INVAL | B_FREE)) != 0) {
		if (pp->p_keepcnt != 0) {
			page_wait(pp);
			/*
			 * Re-verify page identity since it could have
			 * changed while we were sleeping.
			 */
			if (pp->p_vnode != vp || pp->p_offset != offset) {
				/*
				 * Lost the page - nothing to do?
				 */
				return (0);
			}
			/*
			 * The page has not lost its identity and hence
			 * should not be on the free list.
			 */
			ASSERT(pp->p_free == 0);
		}
	}

	page_lock(pp);

	/* Isn't this check still needed? */
	if (pp->p_vnode != vp || pp->p_offset != offset || pp->p_gone) {
		/*
		 * Lost the page - nothing to do?
		 */
		page_unlock(pp);
		return (0);
	}

	/*
	 * If the page has mappings and it is not the case that the
	 * page is already marked dirty and we are going to unload
	 * the page below because we are going to free/invalidate
	 * it, then we sync current mod bits from the hat layer now.
	 */
	if (pp->p_mapping && !(pp->p_mod && (flags & (B_FREE | B_INVAL)) != 0))
		hat_pagesync(pp);

	if (pp->p_mod == 0) {
		if ((flags & (B_INVAL | B_FREE)) != 0) {
			if (pp->p_mapping)
				hat_pageunload(pp);
			if ((flags & B_INVAL) != 0) {
				if (pp->p_free)
					page_reclaim(pp);
				page_abort(pp);
				return (0);
			}
			if (pp->p_free == 0) {
				if ((flags & B_FREE) != 0) {
					page_free(pp, (flags & B_DONTNEED));
					return (0);
				}
			}
		}
		page_unlock(pp);
		return (0);
	}

	/*
	 * Page is dirty, get it ready for the write back
	 * and add page to the dirty list.  First unload
	 * the page if we are going to free/invalidate it.
	 */
	if (pp->p_mapping && (flags & (B_FREE | B_INVAL)) != 0)
		hat_pageunload(pp);
	pp->p_mod = 0;
	pp->p_ref = 0;
	trace3(TR_PG_PVN_GETDIRTY, pp, pp->p_vnode, pp->p_offset);
	ASSERT(pp->p_free == 0);
	ASSERT(pp->p_intrans == 0);
	pp->p_intrans = 1;
	/*
	 * XXX - The `p_pagein' bit is set for asynchronous or
	 * synchronous invalidates to prevent other processes
	 * from accessing the page in the window after the i/o is
	 * complete but before the page is aborted. If this is not
	 * done, updates to the page before it is aborted will be lost.
	 */
	pp->p_pagein = (flags & B_INVAL) ? 1 : 0;
	PAGE_HOLD(pp);
	page_sortadd(dirty, pp);
	return (1);
}

/*
 * Run down the vplist and handle all pages whose offset is >= off.
 * For each dirty page, call VOP_PUTPAGE().
 * For B_INVAL&~B_ASYNC, keep trying until no pages are left.
 * For compatibility reasons, this routine still returns type (page_t *),
 * but it will always be NULL, since the actual work is done here now.
 *
 * Assumptions:
 *	Exclusion locks for v_pages processing done at higher level
 *	No locks in VOP_PUTPAGE recursion.
 *	Flags are {B_ASYNC, B_INVAL, B_FREE, B_DONTNEED}
 *
 * This routine should only be called with the vp locked by the file
 * system code.
 * Though the processing is safe for normal page hashin/hashout,
 * the lock insures that only one pvn_vptrunc or pvn_vplist_dirty
 * has marker pairs running around the list.  Analysis of the multiple
 * marker set case is too hard on our tired brains.
 * The hat_map() marker behaves like a normal page (unlike the marker
 * pair rules), and so causes no problems.
 */

#define ADVANCE_FROM_MARKER(pp, mpp) { \
		if (pp == mpp->p_vpnext) \
			pp = pp->p_vpnext; \
		else \
			pp = mpp->p_vpnext; \
		mpp->p_vpnext->p_vpprev = mpp->p_vpprev; \
		mpp->p_vpprev->p_vpnext = mpp->p_vpnext; \
	}

page_t *
pvn_vplist_dirty(vp, off, flags)
	register struct vnode *vp;
	u_int off;
	register int flags;
{
	register page_t *pp;
	register page_t *nextpp;
	register int s;
	register page_t *spp, *mpp;
	u_int offset;

	if (vp->v_type == VCHR || vp->v_pages == NULL) {
		return NULL;
	}

	if ((spp = (page_t *)kmem_zalloc(2 * sizeof(page_t), KM_SLEEP)) == NULL)
		cmn_err(CE_PANIC, "pvn_vplist_dirty: cannot allocate marker pages");
	s = splvm();
	if ((pp = vp->v_pages) == NULL) {
		splx(s);
		kmem_free(spp, 2 * sizeof(page_t));
		return NULL;
	}
	mpp = spp + 1;
	spp->p_vnode = (struct vnode *)&pvn_vplist_dirty;
	mpp->p_vnode = (struct vnode *)&pvn_vplist_dirty;
#ifdef DEBUG
	spp->p_offset = (u_int)u.u_procp;
	mpp->p_offset = (u_int)u.u_procp;
#endif
	/*
	 * Insert a start marker at the front of the v_pages list,
	 * so we'll know when we've traversed the entire list.
	 */
	spp->p_vpnext = pp;
	spp->p_vpprev = pp->p_vpprev;
	pp->p_vpprev = spp;
	spp->p_vpprev->p_vpnext = spp;
	vp->v_pages = spp;

	while (pp != spp) {
		if (pp->p_vnode != vp) {
			pp = pp->p_vpnext;
			continue;
		}

		/* Process the current page */
		if (pp->p_offset < off || pp->p_gone) {
			pp = pp->p_vpnext;
			continue;
		}
		offset = pp->p_offset;

		/*
		 * If trying to invalidate a logically locked page, forget it.
		 */
		if ((flags & B_INVAL|B_FREE)
		    && (pp->p_lckcnt != 0 || pp->p_cowcnt != 0)) {
			pp = pp->p_vpnext;
			continue;
		}

		if ((flags & B_DELWRI) != 0 && (pp->p_keepcnt != 0 || pp->p_lock)) {
			/*
			 * This is a klustering case that would
			 * cause us to block, just give up.
			 */
			pp = pp->p_vpnext;
			continue;
		}

		if (pp->p_intrans && (flags & (B_INVAL | B_ASYNC)) == B_ASYNC) {
			/*
			 * Don't bother waiting for an intrans page if we are not
			 * doing invalidation and this is an async operation
			 * (the page will be correct when the current io completes).
			 */
			pp = pp->p_vpnext;
			continue;
		}

		/*
		 * put marker in front of current page
		 * do it now to avoid several marker adds
		 */
		mpp->p_vpnext = pp;
		mpp->p_vpprev = pp->p_vpprev;
		pp->p_vpprev = mpp;
		mpp->p_vpprev->p_vpnext = mpp;

intransck:
		/*
		 * If i/o is in progress on the page or we have to
		 * invalidate or free the page, wait for the page keep
		 * count to go to zero.
		 */

		if (pp->p_intrans || (flags & (B_INVAL | B_FREE)) != 0) {
			if (pp->p_keepcnt != 0) {
				splx(s);
				page_wait(pp);
				s = splvm();
				/*
				 * Re-verify page identity since it could have
				 * changed while we were sleeping.
				 */
				if (pp->p_vnode != vp || pp->p_offset != offset
				    || pp->p_gone) {
					/*
					 * Lost the page - nothing to do?
					 */
					ADVANCE_FROM_MARKER(pp, mpp);
					continue;
				}
				/*
				 * The page has not lost its identity and hence
				 * should not be on the free list.
				 */
				ASSERT(pp->p_free == 0);
			}
		}

		if (pp->p_lock) {
			while (pp->p_lock) {
				page_cv_wait(pp);
				if (pp->p_vnode != vp || pp->p_offset != offset
				    || pp->p_gone) {
					/*
					 * Lost the page - nothing to do?
					 */
					ADVANCE_FROM_MARKER(pp, mpp);
					goto contin;	/* continue 2 levels */
				}
			}
		}
		pp->p_lock = 1;
		ASSERT(pp->p_intrans == 0);	/*locked during IO */
		if (pp->p_mapping &&
		    !(pp->p_mod && (flags & (B_FREE | B_INVAL)) != 0))
			/* should be hat_getmod(pp) */
			hat_pagesync(pp);

		if (pp->p_mod == 0) {
			if ((flags & (B_INVAL | B_FREE)) != 0) {
				if (pp->p_mapping)
					hat_pageunload(pp);
				if ((flags & B_INVAL) != 0) {
					if (pp->p_free)
						page_reclaim(pp);
					page_abort(pp);
					ADVANCE_FROM_MARKER(pp, mpp);
					continue;
				}
				if (pp->p_free == 0) {
					page_free(pp, (flags & B_DONTNEED));
					ADVANCE_FROM_MARKER(pp, mpp);
					continue;
				}
			}
			page_unlock(pp);
			ADVANCE_FROM_MARKER(pp, mpp);
			continue;
		}
		page_unlock(pp);
		(void) splx(s);
		VOP_PUTPAGE(pp->p_vnode, pp->p_offset,
		    PAGESIZE, flags, (struct cred *)0);
		PREEMPT();
		s = splvm();
		if (pp == mpp->p_vpnext && !pp->p_gone && pp->p_vnode == vp
			    && pp->p_offset != offset) {
#ifdef DEBUG
printf("pvn_vplist_dirty:Removing the marker page -- the identity has changed\n");
#endif
			mpp->p_vpnext->p_vpprev = mpp->p_vpprev;
			mpp->p_vpprev->p_vpnext = mpp->p_vpnext;
			continue;	/* it is a new identity */
		}
		ADVANCE_FROM_MARKER(pp, mpp);
contin:
		;
	}

	/* remove spp and kmem_free markers */
	if (spp->p_vpnext == spp)
		vp->v_pages = NULL;
	else {
		spp->p_vpnext->p_vpprev = spp->p_vpprev;
		spp->p_vpprev->p_vpnext = spp->p_vpnext;
		if (vp->v_pages == spp)
			vp->v_pages = spp->p_vpnext;
	}
	kmem_free(spp, 2 * sizeof(page_t));
	(void) splx(s);
	return NULL;
}

/*
 * Used when we need to find a page but don't care about free pages.
 */
static page_t *
pvn_pagefind(vp, off)
	register struct vnode *vp;
	register u_int off;
{
	register page_t *pp;
	register int s;

	s = splvm();
	pp = page_exists(vp, off);
	if (pp != NULL && pp->p_free)
		pp = NULL;
	(void) splx(s);
	return (pp);
}

/*
 * Use page_find's and handle all pages for this vnode whose offset
 * is >= off and < eoff.  This routine will also do klustering up
 * to offlo and offhi up until a page which is not found.  We assume
 * that offlo <= off and offhi >= eoff.
 *
 * Returns a list of dirty kept pages all ready to be written back.
 */
page_t *
pvn_range_dirty(vp, off, eoff, offlo, offhi, flags)
	register struct vnode *vp;
	u_int off, eoff;
	u_int offlo, offhi;
	int flags;
{
	page_t *dirty = NULL;
	register page_t *pp;
	register u_int o;
	register page_t *(*pfind)();

	ASSERT(offlo <= off && offhi >= eoff);

	off &= PAGEMASK;
	eoff = (eoff + PAGEOFFSET) & PAGEMASK;

	/*
	 * If we are not invalidating pages, use the routine,
	 * pvn_pagefind(), to prevent reclaiming them from the
	 * free list.
	 */
	if ((flags & B_INVAL) == 0)
		pfind = pvn_pagefind;
	else
		pfind = page_find;

	/* first do all the pages from [off..eoff) */
	for (o = off; o < eoff; o += PAGESIZE) {
		pp = (*pfind)(vp, o);
		if (pp != NULL) {
			(void) pvn_getdirty(pp, &dirty, flags);
		}
	}

	if (pvn_range_noklust)
		return (dirty);

	/* now scan backwards looking for pages to kluster */
	for (o = off - PAGESIZE; (int)o >= 0 && o >= offlo; o -= PAGESIZE) {
		pp = (*pfind)(vp, o);
		if (pp == NULL)
			break;		/* page not found */
		if (pvn_getdirty(pp, &dirty, flags | B_DELWRI) == 0)
			break;		/* page not added to dirty list */
	}

	/* now scan forwards looking for pages to kluster */
	for (o = eoff; o < offhi; o += PAGESIZE) {
		pp = (*pfind)(vp, o);
		if (pp == NULL)
			break;		/* page not found */
		if (pvn_getdirty(pp, &dirty, flags | B_DELWRI) == 0)
			break;		/* page not added to dirty list */
	}

	return (dirty);
}

/*
 * Take care of invalidating all the pages for vnode vp going to size
 * vplen.  This includes zero'ing out zbytes worth of file beyond vplen.
 * This routine should only be called with the vp locked by the file
 * system code.
 * Though the processing is safe for normal page hashin/hashout,
 * the lock insures that only one pvn_vptrunc or pvn_vplist_dirty
 * has marker pairs running around the list.  Analysis of the multiple
 * marker set case is too hard on our tired brains.
 * The hat_map() marker behaves like a normal page (unlike the marker
 * pair rules), and so causes no problems.
 */
void
pvn_vptrunc(vp, vplen, zbytes)
	register struct vnode *vp;
	register u_int vplen;
	u_int zbytes;
{
	register page_t *pp;
	register page_t *spp;
	register page_t *mpp;
	u_int offset;
	register int s;

	if (vp->v_pages == NULL || vp->v_type == VCHR)
		return;

	/*
	 * Handle the partial zeroing of the last page.
	 */
	if (vplen > 0 && zbytes != 0) {
		addr_t addr;

		if ((zbytes + (vplen & MAXBOFFSET)) > MAXBSIZE)
			cmn_err(CE_PANIC, "pvn_vptrunc zbytes");
		addr = segmap_getmap(segkmap, vp, vplen & MAXBMASK);
		(void) kzero(addr + (vplen & MAXBOFFSET),
		    MAX(zbytes, PAGESIZE - (vplen & PAGEOFFSET)));
		(void) segmap_release(segkmap, addr, SM_WRITE | SM_ASYNC);
	}

	if ((spp = (page_t *)kmem_zalloc(2 * sizeof(page_t), KM_SLEEP)) == NULL)
		cmn_err(CE_PANIC, "pvn_vplist_dirty: cannot allocate marker pages");
	s = splvm();
	if ((pp = vp->v_pages) == NULL) {
		splx(s);
		kmem_free(spp, 2 * sizeof(page_t));
		return;
	}
	mpp = spp + 1;
	spp->p_vnode = (struct vnode *)&pvn_vptrunc;
	mpp->p_vnode = (struct vnode *)&pvn_vptrunc;
#ifdef DEBUG
	spp->p_offset = (u_int)u.u_procp;
	mpp->p_offset = (u_int)u.u_procp;
#endif
	/*
	 * Insert a start marker at the front of the v_pages list,
	 * so we'll know when we've traversed the entire list.
	 */
	spp->p_vpnext = pp;
	spp->p_vpprev = pp->p_vpprev;
	pp->p_vpprev = spp;
	spp->p_vpprev->p_vpnext = spp;
	vp->v_pages = spp;

	for (; pp != spp;) {
		if (pp->p_vnode != vp) {
			pp = pp->p_vpnext;
			continue;
		}

		/* Process the current page */
		if (pp->p_offset < vplen || pp->p_gone) {
			pp = pp->p_vpnext;
			continue;
		}
		offset = pp->p_offset;

		/*
		 * put marker in front of current page
		 * do it now to avoid several marker adds
		 */
		mpp->p_vpnext = pp;
		mpp->p_vpprev = pp->p_vpprev;
		pp->p_vpprev = mpp;
		mpp->p_vpprev->p_vpnext = mpp;

		/*
		 * When aborting these pages, we make sure that
		 * we wait to make sure they are really gone.
		 */
		if (pp->p_keepcnt != 0) {
			(void) splx(s);
			page_wait(pp);
			s = splvm();
			if (pp->p_vnode != vp || pp->p_offset != offset) {
				pp = mpp->p_vpnext;
				mpp->p_vpnext->p_vpprev = mpp->p_vpprev;
				mpp->p_vpprev->p_vpnext = mpp->p_vpnext;
				continue;
			}
			ASSERT(!pp->p_gone);
		}
		if (pp->p_lock) {
			while (pp->p_lock) {
				page_cv_wait(pp);
				if (pp->p_vnode != vp || pp->p_offset != offset) {
					pp = mpp->p_vpnext;
					mpp->p_vpnext->p_vpprev = mpp->p_vpprev;
					mpp->p_vpprev->p_vpnext = mpp->p_vpnext;
					continue;
				}
			}
			ASSERT(!pp->p_gone);
		}
		pp->p_lock = 1;
		if (pp->p_free)
			page_reclaim(pp);
		page_abort(pp);
		pp = mpp->p_vpnext;
		mpp->p_vpnext->p_vpprev = mpp->p_vpprev;
		mpp->p_vpprev->p_vpnext = mpp->p_vpnext;
	}

	/* remove spp and kmem_free markers */
	if (spp->p_vpnext == spp)
		vp->v_pages = NULL;
	else {
		spp->p_vpnext->p_vpprev = spp->p_vpprev;
		spp->p_vpprev->p_vpnext = spp->p_vpnext;
		if (vp->v_pages == spp)
			vp->v_pages = spp->p_vpnext;
	}
	kmem_free(spp, 2 * sizeof(page_t));
	(void) splx(s);
}

/*
 * This routine is called when the low level address translation
 * code decides to unload a translation.  It calls back to the
 * segment driver which in many cases ends up here.
 */
/*ARGSUSED*/
void
pvn_unloadmap(vp, offset, ref, mod)
	struct vnode *vp;
	u_int offset;
	u_int ref, mod;
{

	/*
	 * XXX - what is the pvn code going to do w/ this information?
	 * This guy gets called for each loaded page when a executable
	 * using the segvn driver terminates...
	 */
}

/*
 * Handles common work of the VOP_GETPAGE routines when more than
 * one page must be returned by calling a file system specific operation
 * to do most of the work.  Must be called with the vp already locked
 * by the VOP_GETPAGE routine.
 */
int
pvn_getpages(getapage, vp, off, len, protp, pl, plsz, seg, addr, rw, cred)
	int (*getapage)();
	struct vnode *vp;
	u_int off, len;
	u_int *protp;
	page_t *pl[];
	u_int plsz;
	struct seg *seg;
	register addr_t addr;
	enum seg_rw rw;
	struct cred *cred;
{
	register page_t **ppp;
	register u_int o, eoff;
	u_int sz;
	int err;

	ASSERT(plsz >= len);		/* insure that we have enough space */

	/*
	 * Loop one page at a time and let getapage function fill
	 * in the next page in array.  We only allow one page to be
	 * returned at a time (except for the last page) so that we
	 * don't have any problems with duplicates and other such
	 * painful problems.  This is a very simple minded algorithm,
	 * but it does the job correctly.  We hope that the cost of a
	 * getapage call for a resident page that we might have been
	 * able to get from an earlier call doesn't cost too much.
	 */
	ppp = pl;
	sz = PAGESIZE;
	eoff = off + len;
	for (o = off; o < eoff; o += PAGESIZE, addr += PAGESIZE) {
		if (o + PAGESIZE >= eoff) {
			/*
			 * Last time through - allow the all of
			 * what's left of the pl[] array to be used.
			 */
			sz = plsz - (o - off);
		}
		err = (*getapage)(vp, o, protp, ppp, sz, seg, addr, rw, cred);
		if (err) {
			/*
			 * Release any pages we already got.
			 */
			if (o > off && pl != NULL) {
				for (ppp = pl; *ppp != NULL; *ppp++ = NULL) {
					PAGE_RELE(*ppp);
				}
			}
			break;
		}
		if (pl != NULL)
			ppp++;
	}

	return (err);
}

pvn_vpempty(vp)
	register struct vnode *vp;
{
	register page_t *pp;
	register int s;

	s = splhi();
	if ((pp = vp->v_pages) == NULL) {
		splx(s);
		return 1;
	}

	do {
		if (pp->p_vnode == vp) {
			splx(s);
			return 0;
		}
	} while (pp = pp->p_vpnext, pp != vp->v_pages);
	splx(s);
	return 1;
}
