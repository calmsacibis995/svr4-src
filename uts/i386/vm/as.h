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

#ifndef _VM_AS_H
#define _VM_AS_H

#ident	"@(#)kern-vm:as.h	1.3"

#include "vm/faultcode.h"
#include "vm/vm_hat.h"

/*
 * VM - Address spaces.
 */

/*
 * Each address space consists of a list of sorted segments
 * and machine dependent address translation information.
 *
 * All the hard work is in the segment drivers and the
 * hardware address translation code.
 */
struct as {
	u_int	a_lock: 1;
	u_int	a_want: 1;
	u_int	a_paglck: 1;
	u_int	: 13;
	u_short	a_keepcnt;	/* number of `keeps' */
	struct	seg *a_segs;	/* segments in this address space */
	struct	seg *a_seglast;	/* last segment hit on the address space */
	size_t	a_size;		/* size of address space */
	size_t	a_rss;		/* memory claim for this address space */
	struct	hat a_hat;	/* hardware address translation */
};

#ifdef _KERNEL

/*
 * Flags for as_gap.
 */
#define AH_DIR		0x1	/* direction flag mask */
#define AH_LO		0x0	/* find lowest hole */
#define AH_HI		0x1	/* find highest hole */
#define AH_CONTAIN	0x2	/* hole must contain `addr' */

#ifdef __STDC__
extern size_t rm_assize(struct as *);
extern int as_lock(struct as *, int, u_int);
#else
extern size_t rm_assize();
extern int as_lock();
#endif

struct	seg *as_segat(/* as, addr */);
struct	as *as_alloc();
void	as_free(/* as */);
struct	as *as_dup(/* as */);
int	as_addseg(/* as, seg */);
faultcode_t as_fault(/* as, addr, size, type, rw */);
faultcode_t as_faulta(/* as, addr, size */);
int	 as_setprot(/* as, addr, size, prot */);
int	as_checkprot(/* as, addr, size, prot */);
int	as_unmap(/* as, addr, size */);
int	as_map(/* as, addr, size, crfp, crargsp */);
int	as_gap(/* as, minlen, basep, lenp, flags, addr */);
int	as_memory(/* as, addrp, sizep */);
u_int	as_swapout(/* as */);
int	as_incore(/* as, addr, size, vecp, sizep */);
int	as_ctl(/* as, addr, size, func, attr, arg, bitmap, pos */);
u_int	as_getprot(/* as, addr, naddrp */);

struct anon_map *as_shmlookup(/* as, addr */);
#endif /* _KERNEL */

#endif	/* _VM_AS_H */
