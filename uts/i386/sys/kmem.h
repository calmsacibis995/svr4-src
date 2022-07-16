/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_KMEM_H
#define _SYS_KMEM_H

#ident	"@(#)head.sys:sys/kmem.h	1.8.5.1"

/*
 *	public definitions for kernel memory allocator
 */


/*
 * KM_SLEEP -- can sleep to get memory.
 * KM_NOSLEEP -- cannot sleep to get memory. *MUST* agree
 *			with NOSLEEP as defined in sys/immu.h
 * KM_NO_DMA -- By default, the seg_kmem segment driver will return DMA able
 *		pages. Since we do not want to penalize the KM allocator and
 *		other guys that rely on seg_kmem segment driver - we specify
 *		that they need not worry about DMAable pages.
 */
#define	KM_SLEEP	0
#define	KM_NOSLEEP	1
#define	KM_NO_DMA	2


/* function declarations */

#if defined(__STDC__)

extern void kmem_init(void);
extern _VOID * kmem_alloc(size_t, int);
extern _VOID * kmem_zalloc(size_t, int);
extern _VOID * kmem_fast_alloc(caddr_t *, size_t, int, int);
extern _VOID * kmem_fast_zalloc(caddr_t *, size_t, int, int);
extern void kmem_free(_VOID *, size_t);
extern void kmem_fast_free(caddr_t *, caddr_t);

#else

extern void kmem_init();
extern _VOID *kmem_alloc();
extern _VOID *kmem_zalloc();
extern _VOID *kmem_fast_alloc();
extern _VOID *kmem_fast_zalloc();
extern void kmem_free();
extern void kmem_fast_free();

#endif

#endif	/* _SYS_KMEM_H */
