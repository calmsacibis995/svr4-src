/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#ident	"@(#)head.sys:sys/mman.h	1.15.3.1"

/*
 * Protections are chosen from these bits, or-ed together.
 * Note - not all implementations literally provide all possible
 * combinations.  PROT_WRITE is often implemented as (PROT_READ |
 * PROT_WRITE) and (PROT_EXECUTE as PROT_READ | PROT_EXECUTE).
 * However, no implementation will permit a write to succeed
 * where PROT_WRITE has not been set.  Also, no implementation will
 * allow any access to succeed where prot is specified as PROT_NONE.
 */
#define	PROT_READ	0x1		/* pages can be read */
#define	PROT_WRITE	0x2		/* pages can be written */
#define	PROT_EXEC	0x4		/* pages can be executed */

#ifdef _KERNEL
#define	PROT_USER	0x8		/* pages are user accessable */
#define	PROT_ALL	(PROT_READ | PROT_WRITE | PROT_EXEC | PROT_USER)
#endif /* _KERNEL */

#define	PROT_NONE	0x0		/* pages cannot be accessed */

/* sharing types:  must choose either SHARED or PRIVATE */
#define	MAP_SHARED	1		/* share changes */
#define	MAP_PRIVATE	2		/* changes are private */
#define	MAP_TYPE	0xf		/* mask for share type */

/* other flags to mmap (or-ed in to MAP_SHARED or MAP_PRIVATE) */
#define	MAP_FIXED	0x10		/* user assigns address */

/* these flags not yet implemented */
#define	MAP_RENAME	0x20		/* rename private pages to file */
#define	MAP_NORESERVE	0x40		/* don't reserve needed swap area */

/* these flags are used by memcntl */
#define PROC_TEXT	(PROT_EXEC | PROT_READ)
#define PROC_DATA	(PROT_READ | PROT_WRITE | PROT_EXEC)
#define SHARED		0x10
#define PRIVATE		0x20

#ifdef _KERNEL
#define PROT_EXCL	0x20
#endif /* _KERNEL */

#define VALID_ATTR  (PROT_READ|PROT_WRITE|PROT_EXEC|SHARED|PRIVATE)
#ifdef notdef
/*
 * Not clear that this flag will ever be implemented
 */
#define	MAP_INHERIT	0x80		/* inherit this mapping accross exec */
#endif /* notdef */

/*
 * For the sake of backward object compatibility, we use the _MAP_NEW flag.
 * This flag will be automatically or'ed in by the C library for all
 * new mmap calls.  Previous binaries with old mmap calls with continue
 * to get 0 or -1 for return values.  New mmap calls will get the mapped
 * address as the return value if successful and -1 on errors.  By default,
 * new mmap calls automatically have the kernel assign the map address
 * unless the MAP_FIXED flag is given.
 */
#define	_MAP_NEW	0x80000000	/* user's should not need to use this */

#if !defined(LOCORE) && !defined(_KERNEL)
#include <sys/types.h>

/*
 * Except for old binaries mmap() will return the resultant
 * address of mapping on success and (caddr_t)-1 on error.
 */
extern caddr_t mmap();
#endif /* !LOCORE && !_KERNEL */

/* advice to madvise */
#define	MADV_NORMAL	0		/* no further special treatment */
#define	MADV_RANDOM	1		/* expect random page references */
#define	MADV_SEQUENTIAL	2		/* expect sequential page references */
#define	MADV_WILLNEED	3		/* will need these pages */
#define	MADV_DONTNEED	4		/* don't need these pages */

/* flags to msync */
#define	MS_SYNC		0x0		/* wait for msync */
#define	MS_ASYNC	0x1		/* return immediately */
#define	MS_INVALIDATE	0x2		/* invalidate caches */

/* functions to mctl */
#define	MC_SYNC		1		/* sync with backing store */
#define	MC_LOCK		2		/* lock pages in memory */
#define	MC_UNLOCK	3		/* unlock pages from memory */
#define	MC_ADVISE	4		/* give advice to management */
#define	MC_LOCKAS	5		/* lock address space in memory */
#define	MC_UNLOCKAS	6		/* unlock address space from memory */

/* flags to mlockall */
#define	MCL_CURRENT	0x1		/* lock current mappings */
#define	MCL_FUTURE	0x2		/* lock future mappings */

#endif	/* _SYS_MMAN_H */
