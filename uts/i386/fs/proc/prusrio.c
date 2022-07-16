/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:proc/prusrio.c	1.3"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/inline.h"
#include "sys/mman.h"
#include "sys/proc.h"
#include "sys/sysmacros.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"

#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/procfs.h"

#include "vm/as.h"
#include "vm/seg.h"

#include "prdata.h"

/*
 * Perform I/O to/from a process.
 */
int
prusrio(p, rw, uiop)
	register proc_t *p;
	enum uio_rw rw;
	register struct uio *uiop;
{
	caddr_t addr, naddr, maxaddr, vaddr, a;
	register struct seg *seg;
	struct as *as;
	int count, n, error = 0;
	u_int prot;
	enum seg_rw srw;
	int protchanged, ocount;

	if ((p->p_flag & SSYS) || (as = p->p_as) == NULL)
		return EIO;

	addr = (caddr_t) uiop->uio_offset;

	/*
	 * Locate segment containing address of interest.
	 */
	if ((seg = as_segat(as, addr)) == NULL)
		return EIO;

more:
	maxaddr = seg->s_base + seg->s_size;

	/*
	 * Fault in the necessary pages one at a time, map them into
	 * kernel space, and copy in or out.
	 */
	srw = (rw == UIO_WRITE) ? S_WRITE : S_READ;
	a = addr;
	count = min(uiop->uio_resid, maxaddr - addr);

 	if (rw == UIO_WRITE
	  && ((prot = as_getprot(as, addr, &naddr)) & PROT_WRITE) == 0) {
		protchanged = 1;
		ocount = count;
		(void) as_setprot(as, addr, ocount, prot|PROT_WRITE);
	} else
		protchanged = 0;

	while (count > 0) {
		caddr_t page = (caddr_t)(((u_int)a) & PAGEMASK);
		caddr_t nextpage = page + PAGESIZE;
		int shortcut;

		n = min(count, nextpage - a);
		/*
		 * Short-cut fast access if the page is resident and
		 * doesn't require copy-on-write processing; in this
		 * case we can locate its address quickly and avoid
		 * the overhead of as_fault().
		 */
		if (vaddr = prfastmapin(p, a, srw == S_WRITE))
			shortcut = 1;
		else {
			shortcut = 0;
			if (as_fault(as, page, PAGESIZE, F_SOFTLOCK, srw)) {
				error = EIO;
				break;
			}
			if ((vaddr = prmapin(p, a, srw == S_WRITE)) == NULL) {
				error = EIO;
				break;
			}
		}
		error = uiomove(vaddr, n, rw, uiop);
		if (shortcut)
			prfastmapout(p, a, vaddr, srw == S_WRITE);
		else {
			prmapout(p, a, vaddr, srw == S_WRITE);
			(void) as_fault(as, page, PAGESIZE, F_SOFTUNLOCK, srw);
		}
		if (error)
			break;
		count -= n;
		a += n;
	}

	if (protchanged)
		(void) as_setprot(as, addr, ocount, prot);

	/*
	 * If we reached the end of the segment without exhausting
	 * the I/O count, see if there is another segment abutting
	 * the end of the previous segment.  Continue if so.
	 */
	if (error == 0 && uiop->uio_resid != 0) {
		addr = (caddr_t) uiop->uio_offset;
		if ((seg = as_segat(as, addr)) != NULL)
			goto more;
	}

	return error;
}		
