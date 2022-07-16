/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:sys/param.h	1.3.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

/*
 * Fundamental variables; don't change too often.
 * 
 * This file is included here for compatibility with
 * SunOS, but it does *not* include all the values
 * define in the SunOS version of this file.
 */

#include <limits.h>

#ifndef _POSIX_VERSION
#define _POSIX_VERSION	198808L
#endif

#define UID_NOBODY	60001
#define GID_NOBODY	60001

#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

/* The values below are defined in limits.h */
#define NOFILE	OPEN_MAX	/* max open files per process */

#define	SSIZE	1		/* initial stack size (*2048 bytes) */
#define	SINCR	1		/* increment of stack (*2048 bytes) */
#define	USIZE	3		/* size of user block (*2048 bytes) */
#ifdef i386
#define	MAXUSIZE 18		/* max size of user block (*4096 bytes) */
#endif

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	100		/* 100 ticks/second of the clock */

#define	NCARGS	5120		/* # characters in exec arglist */
				/*   must be multiple of NBPW.  */
/*
 * These define the maximum and minimum allowable values of the
 * configurable parameter NGROUPS_MAX.
 */
#define	NGROUPS_UMAX	32
#define	NGROUPS_UMIN	8

#define NGROUPS		NGROUPS_MAX	/* max number groups, from limits.h */
#define NOGROUP		-1	/* marker for empty group set member */

/*
 * The following defines apply to the kernel virtual address space.
 */

/*
 * The size of the kernel segment table in pages.  The starting address
 * comes from the vuifile.
 */
#define	SYSSEGSZ 1024

#define	NS0SDE	0x120	/* number of kernel segments in section 0. */
#define	NS1SDE	0x60	/* number of kernel segments in section 1. */
#define	NS2SDE	0x0	/* number of kernel segments in section 2. */
#define	NS3SDE	0x1	/* number of kernel segments in section 3. */

/*
 * To avoid prefetch errors at the end of a region, it must
 * be padded with the following number of bytes.
 */
	
#define	PREFETCH	12

/*
 * Priorities.  Should not be altered too much.
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PNOSTOP	01000
#define	PSWP	0
#define	PINOD	10
#define	PSNDD	PINOD
#define	PRIBIO	20
#define	PZERO	25
#define PMEM	0
#define	NZERO	20
#define	PPIPE	26
#define	PVFS	27
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * Fundamental constants of the implementation--cannot be changed easily.
 */

#define NBPS	0x20000		/* Number of bytes per segment */
#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define	NCPS	64		/* Number of clicks per segment */
#define	CPSSHIFT	6	/* LOG2(NCPS) if exact */
#define	NBPC	2048		/* Number of bytes per click */
#define	BPCSHIFT	11	/* LOG2(NBPC) if exact */
#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<11)	/* default max write address */
#define	NODEV	(dev_t)(-1)
#define	NBPSCTR		512	/* Bytes per disk sector.	*/
#define	UBSIZE		512	/* unix block size.		*/
#define SCTRSHFT	9	/* Shift for BPSECT.		*/

#define	UMODE	3		/* current Xlevel == user */
#define	USERMODE(psw)	((psw).CM == UMODE)
#define	BASEPRI(psw)	((psw).IPL > 8)

#define	lobyte(X)	(((unsigned char *)&(X))[1])
#define	hibyte(X)	(((unsigned char *)&(X))[0])
#define	loword(X)	(((ushort *)&(X))[1])
#define	hiword(X)	(((ushort *)&(X))[0])

/*
 *  Interrupt stack size in STKENT units
 */
#define QSTKSZ	1000
#define ISTKSZ	1000

#define	MAXSUSE	255

/* REMOTE -- whether machine is primary, secondary, or regular */
#define SYSNAME 9		/* # chars in system name */
#define PREMOTE 39

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

#define PIPEBUF PIPE_BUF	/* pipe buffer size */

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
#define	MAXFRAG 	4

#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)

/*
 * MMU_PAGES* describes the physical page size used by the mapping hardware.
 * PAGES* describes the logical page size used by the system.
 */

#define	MMU_PAGESIZE	0x800		/* 2048 bytes */
#define	MMU_PAGESHIFT	11		/* log2(MMU_PAGESIZE) */
#define	MMU_PAGEOFFSET	(MMU_PAGESIZE-1)/* Mask of address bits in page */
#define	MMU_PAGEMASK	(~MMU_PAGEOFFSET)

#define	PAGESIZE	0x800		/* All of the above, for logical */
#define	PAGESHIFT	11
#define	PAGEOFFSET	(PAGESIZE - 1)
#define	PAGEMASK	(~PAGEOFFSET)

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

#define shm_alignment	stob(1)		/* segment size */


/*
 * Signals 
 */
#include <sys/signal.h>

 
#include <sys/types.h>

/*
 * bit map related macros
 */
#define setbit(a,i)     ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i)     ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)      ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i)      (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)
 
/*
 * Macros for fast min/max.
 */
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
 
#define howmany(x, y)   (((x)+((y)-1))/(y))
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))

/*
 * Scale factor for scaled integers used to count
 * %cpu time and load averages.
 */
#define FSHIFT  8               /* bits to right of fixed binary point */
#define FSCALE  (1<<FSHIFT)
 
/*
 * Maximum size of hostname recognized and stored in the kernel.
 * Same as in /usr/include/netdb.h
 */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  64
#endif

#endif	/* _SYS_PARAM_H */
