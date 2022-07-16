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

#ifndef _VM_HAT_H
#define _VM_HAT_H

#ident	"@(#)kern-vm:hat.h	1.3.1.2"

/*
 * VM - Hardware Address Translation management.
 *
 * This file describes the machine independent interfaces to
 * the hardware address translation management routines.  Other
 * machine specific interfaces and structures are defined
 * in <vm/vm_hat.h>.  The hat layer manages the address
 * translation hardware as a cache driven by calls from the
 * higher levels of the VM system.
 */

#include "vm/vm_hat.h"

#ifdef _KERNEL
/*
 * One time hat initialization
 */
void	hat_init();

/*
 * Operations on hat resources for an address space:
 *	- initialize any needed hat structures for the address space
 *	- free all hat resources now owned by this address space
 *	- initialize any needed hat structures when the process is
 *	  swapped in.
 *	- free all hat resources that are not needed while the process
 *	  is swapped out.
 *	- dup any hat resources that can be created when duplicating
 *	  another process' address space.
 *
 * N.B. - The hat structure is guaranteed to be zeroed when created.
 * The hat layer can choose to define hat_alloc as a macro to avoid
 * a subroutine call if this is sufficient initialization.
 */
void	hat_alloc(/* as */);
void	hat_free(/* as */);
void	hat_swapin(/* as */);
void	hat_swapout(/* as */);
int	hat_dup(/* as */);

/* Operation to allocate/reserve mapping structures
 *	- allocate/reserve mapping structures for a segment.
 */
u_int	hat_map(/* seg, vp, vp_base, prot, flags */);

/*
 * Operations on a named address with in a segment:
 *	- load/lock the given page struct
 *	- load/lock the given page frame number
 *	- unlock the given address
 *
 * (Perhaps we need an interface to load several pages at once?)
 */
void	hat_memload(/* seg, addr, pp, prot, flags */);
void	hat_devload(/* seg, addr, ppid, prot, flags */);
void	hat_unlock(/* seg, addr */);

/*
 * Operations over an address range:
 *	- change protections
 *	- change mapping to refer to a new segment
 *	- unload mapping
 */
void	hat_chgprot(/* seg, addr, len, prot */);
void	hat_newseg(/* seg, addr, len, nseg */);
void	hat_unload(/* seg, addr, len, flags */);

/*
 * Operations that work on all active translation for a given page:
 *	- unload all translations to page
 *	- get hw stats from hardware into page struct and reset hw stats
 */
void	hat_pageunload(/* pp */);
void	hat_pagesync(/* pp */);

/*
 * Operations that return physical page IDs (ie - used by mapin):
 *	- return the ppid for kernel virtual address
 *	- return the ppid for arbitrary virtual address
 *	- return the ppid for arbitrary physical address
 *
 * XXX - The second  one is not yet implemented - not yet needed.
 * u_int hat_getpfnum(as, addr);
 */
u_int	hat_getkpfnum(/* addr */);
u_int	hat_getppfnum(/* addr, pspace */);

/*
 * Flags to pass to hat routines.
 *
 * Certain flags only apply to some interfaces:
 *
 * 	HAT_NOFLAGS   No flags specified.
 * 	HAT_LOCK      Lock down mapping resources; hat_map(), hat_memload(),
 * 	              and hat_devload().
 * 	HAT_UNLOCK    Unlock mapping resources; hat_memload(), hat_devload(),
 * 	              and hat_unload().
 *	HAT_PUTPP     VOP_PUTPAGE() pp if dirty and last mapping; hat_unload().
 * 	HAT_FREEPP    Free pp if unloading last mapping (implies HAT_PUTPP);
 *		      hat_unload().
 * 	HAT_RELEPP    PAGE_RELE() pp after mapping is unloaded; hat_unload().
 * 	HAT_PRELOAD   Pre-load pages for new segment; hat_map().
 */
#define	HAT_NOFLAGS	0x0
#define	HAT_LOCK	0x1
#define	HAT_UNLOCK	0x2
#define	HAT_FREEPP	0x4
#define	HAT_RELEPP	0x8
#define	HAT_PRELOAD	0x10
#define HAT_PUTPP	0x20

#endif /* _KERNEL */

#endif	/* _VM_HAT_H */
