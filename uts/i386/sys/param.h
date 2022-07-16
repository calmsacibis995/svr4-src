/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#ident	"@(#)head.sys:sys/param.h	11.28.4.1"

#include <sys/types.h>
#include <sys/fs/s5param.h>
/*
 * Fundamental variables; don't change too often.
 */
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0 /* Disable special character functions */
#endif

#ifndef MAX_INPUT
#define MAX_INPUT 512     /* Maximum bytes stored in the input queue */
#endif

#ifndef MAX_CANON
#define MAX_CANON 256     /* Maximum bytes in a line for canoical processing */
#endif

#define UID_NOBODY  60001   /* user ID no body */
#define GID_NOBODY  UID_NOBODY

#define UID_NOACCESS    60002   /* user ID no access */


#define	MAXPID	30000		/* max process id */
#define	MAXUID	60002		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	SSIZE	1		/* initial stack size (*4096 bytes) */
#define	SINCR	1		/* increment of stack (*4096 bytes) */
#define	USIZE	MINUSIZE	/* inital size of user block (*4096) */
#define	MINUSIZE  2		/* min size of user block (*4096 bytes) */
#define	MAXUSIZE 18		/* max size of user block (*4096 bytes) */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	100		/* 100 ticks/second of the clock */
#define TICK    10000000	/* nanoseconds per tick */


#define NOFILE	20		/* this define is here for	*/
				/* compatibility purposes only	*/
				/* and will be removed in a	*/
				/* later release		*/

/*
 * The following macros are no longer supported in SVR 4.0
 * since there is no longer a limit on the number of files that
 * a process can open. However, for SVR3.2 source compatibility, 
 * you may uncomment  NOFILES_MIN and NOFILES_MAX.
 */

/* #define	NOFILES_MIN	 20	SVR3.2 Source Compatibility */
/* #define	NOFILES_MAX	100	SVR3.2 Source Compatibility */

/*
 * These define the maximum and minimum allowable values of the
 * configurable parameter NGROUPS_MAX.
 */
#define	NGROUPS_UMAX	32
#define	NGROUPS_UMIN	0

/*
 * The following defines apply to the kernel virtual address space.
 */

/*
 * The size of the kernel segment table in pages.  The starting address
 * comes from the vuifile.
 */
#define MAXKSEG		127	/*max no of pages per kseg */

/*
 * To avoid prefetch errors at the end of a region, it must
 * be padded with the following number of bytes.
 */

#define	PREFETCH	0

/*
 * Priorities.  Should not be altered too much.
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PNOSTOP	01000
#define	PSWP	0
#define	PINOD	10
#define PSNDD	PINOD
#define	PRIBIO	20
#define	PZERO	25
#define PMEM	0
#define	NZERO	20
#define	PPIPE	26
#define PVFS	27
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * Fundamental constants of the implementation--cannot be changed easily.
 */

#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define	NCPPT	1024		/* Number of clicks per page table */
#define	CPPTSHIFT	10	/* LOG2(NCPPT) if exact */
#define	NBPC	4096		/* Number of bytes per click */
#define	BPCSHIFT	12	/* LOG2(NBPC) if exact */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<14)	/* default max write address */
#define NBPSCTR         512     /* Bytes per LOGICAL disk sector. */
#define	UBSIZE		512	/* unix block size.		*/
#define SCTRSHFT	9	/* Shift for BPSECT.		*/

#define	UMODE	3		/* current Xlevel == user */
#define	USERMODE(cs)	(((cs) & SEL_RPL) == UMODE)

#define	lobyte(X)	(((unsigned char *)&(X))[0])
#define	hibyte(X)	(((unsigned char *)&(X))[1])
#define	loword(X)	(((ushort *)&(X))[0])
#define	hiword(X)	(((ushort *)&(X))[1])

#define	MAXSUSE	255

/* REMOTE -- whether machine is primary, secondary, or regular */
#define SYSNAME 9		/* # chars in system name */
#define PREMOTE 39

/* XENIX compatibility */
#define	ktop(vaddr)	((paddr_t)svirtophys(vaddr))

/*
 * MAXPATHLEN defines the longest permissible path length,
 * including the terminating null, after expanding symbolic links.
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly.
 * MAXNAMELEN is the length (including the terminating null) of
 * the longest permissible file (component) name.
 */
#define	MAXPATHLEN	1024
#define	MAXSYMLINKS	20
#define	MAXNAMELEN	256

#ifndef NADDR
#define NADDR 13
#endif

/*
 * The following are defined to be the same as
 * defined in /usr/include/limits.h.  They are
 * needed for pipe and FIFO compatibility.
 */
#ifndef PIPE_BUF	/* max # bytes atomic in write to a pipe */
#ifdef u3b15
#define PIPE_BUF	4096
#else
#define PIPE_BUF	5120
#endif	/* u3b15 */
#endif	/* PIPE_BUF */

#ifndef PIPE_MAX	/* max # bytes written to a pipe in a write */
#ifdef u3b15
#define PIPE_MAX	4096
#else
#define PIPE_MAX	5120
#endif	/* u3b15 */
#endif	/* PIPE_MAX */

#define NBBY	8			/* number of bits per byte */

/*
 * File system parameters and macros.
 *
 * The file system is made out of blocks of at most MAXBSIZE units,
 * with smaller units (fragments) only in the last direct block.
 * MAXBSIZE primarily determines the size of buffers in the buffer
 * pool. It may be made larger without any effect on existing
 * file systems; however making it smaller make make some file
 * systems unmountable.
 *
 * Note that the blocked devices are assumed to have DEV_BSIZE
 * "sectors" and that fragments must be some multiple of this size.
 */
#define	MAXBSIZE	8192
#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define	MAXFRAG 	8

#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)

/*
 * MMU_PAGES* describes the physical page size used by the mapping hardware.
 * PAGES* describes the logical page size used by the system.
 */

#define	MMU_PAGESIZE	0x1000		/* 4096 bytes */
#define	MMU_PAGESHIFT	12		/* log2(MMU_PAGESIZE) */
#define	MMU_PAGEOFFSET	(MMU_PAGESIZE-1)/* Mask of address bits in page */
#define	MMU_PAGEMASK	(~MMU_PAGEOFFSET)

#define	PAGESIZE	0x1000		/* All of the above, for logical */
#define	PAGESHIFT	12
#define	PAGEOFFSET	(PAGESIZE - 1)
#define	PAGEMASK	(~PAGEOFFSET)

#ifndef NODEV
#define NODEV	(dev_t)(-1)
#endif

/*
 * Some random macros for units conversion.
 */

/*
 * MMU pages to bytes, and back (with and without rounding)
 */
#define	mmu_ptob(x)	((x) << MMU_PAGESHIFT)
#define	mmu_btop(x)	(((unsigned)(x)) >> MMU_PAGESHIFT)
#define	mmu_btopr(x)	((((unsigned)(x) + MMU_PAGEOFFSET) >> MMU_PAGESHIFT))

/*
 * pages to bytes, and back (with and without rounding)
 */
#define	ptob(x)		((x) << PAGESHIFT)
#define	btop(x)		(((unsigned)(x)) >> PAGESHIFT)
#define	btopr(x)	((((unsigned)(x) + PAGEOFFSET) >> PAGESHIFT))

#define shm_alignment	ctob(1)		/* segment size */


#endif	/* _SYS_PARAM_H */
