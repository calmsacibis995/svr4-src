/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)i286emu:i286sys/param.h	1.1"

/*
 * fundamental variables
 * don't change too often
 */
#include "sys/fs/s5param.h"

#undef pdp11

#define	OSMERGE 1		/* Compile in OS Merge code */
#define KRELOC	1		/* bootstrap relocates the kernel */

#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	SSIZE	16		/* initial stack size (*512 bytes) */
#define	SINCR	2		/* increment of stack (*512 bytes) */
#define USIZE   6               /* size of user block (*512 bytes) */
#define KSTACKSZ        1024    /* size of kernel stack in words          */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	HZ	60		/* Ticks/second of the clock */

#define	NCARGS	5120		/* # characters in exec arglist */

/*	The following define is here for temporary compatibility
**	and should be removed in the next release.  It gives a
**	value for the maximum number of open files per process.
**	However, this value is no longer a constant.  It is a
**	configurable parameter, NOFILES, specified in the kernel
**	master file and available in v.v_nofiles.  Programs which
**	include this header file and use the following value may
**	not operate correctly if the system has been configured
**	to a different value.
*/

#define	NOFILE	20

/*	The following represent the minimum and maximum values to
**	which the paramater NOFILES in the kernel master file may
**	be set.
*/

#define	NOFILES_MIN	 20
#define	NOFILES_MAX	100

#define	MAXMEM	(16*128)	/* max core in 512-byte clicks */
#define	MAXPHYS	32768		/* max physical memory possible  in clicks */
#define	MAXBLK	124		/* max blocks possible for phys IO */

/*
 * priorities
 * should not be altered too much
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PZERO	25
#define PMEM    0
#define	NZERO	20
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */

#define NBPLP   4               /* Number of bytes per large model pointer */
#define NBPSP   2               /* Number of bytes per small model pointer */
#define NBPS    0x10000L        /* Number of bytes per segment */
#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define	NCPS	128		/* Number of clicks per segment */
#define	NBPC	512		/* Number of bytes per click */
#define	NCPD	1		/* Number of clicks per disk block */
#define BPCSHIFT 9              /* LOG2(NBPC) if exact */
#define SOFFMASK (NBPS-1)       /* mask for offset within a segment */

/* define NULL as a short or long pointer depending on model */

#ifndef NULL
#if SMALL_M | MIDDLE_M
#define	NULL	( 0 )
#else
#define	NULL	( 0L )
#endif
#endif

#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<11)	/* default max write address */
#define	NODEV	(dev_t)(-1)
#define	NBPSCTR		512	/* Bytes per disk sector.	*/
#define SCTRSHFT	9	/* Shift for BPSECT.		*/
#define	LBLOCK	512		/* size of logical disk block */
#define	LBSHIFT	9		/* LOG2(LBLOCK) */
#define	CLKTICK	16667		/* microseconds in a clock tick */

#define	USERMODE( cs )	( ( cs ) & SEL_RPL )

#define	lobyte(X)	(((unsigned char *)&X)[0])
#define	hibyte(X)	(((unsigned char *)&X)[1])
#define	loword(X)	(((unsigned short *)&X)[0])
#define	hiword(X)	(((unsigned short *)&X)[1])

#define UNMODEM(X)	((X) & 0xff7f)
