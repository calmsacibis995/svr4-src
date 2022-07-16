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

#ifndef _SYS_VMPARAM_H
#define _SYS_VMPARAM_H

#ident	"@(#)head.sys:sys/vmparam.h	1.6.4.1"


/*
 * Machine dependent constants 
 */

#define	SSIZE		1			/* initial stack size */
#define	SINCR		1			/* increment of stack */


/*
 * DISKRPM is used to estimate the number of paging i/o operations
 * which one can expect from a single disk controller.
 */
#define	DISKRPM		60

/*
 * Paging thresholds (see vm_pageout.c).
 * Strategy of 3/17/83:
 *	lotsfree is 256k bytes, but at most 1/8 of memory
 *	desfree is 100k bytes, but at most 1/16 of memory
 *	minfree is 32k bytes, but at most 1/2 of desfree
 */
#define	LOTSFREE	(256 * 1024)
#define	LOTSFREEFRACT	8
#define	DESFREE		(100 * 1024)
#define	DESFREEFRACT	16
#define	MINFREE		(32 * 1024)
#define	MINFREEFRACT	2

/*
 * There are two clock hands, initially separated by HANDSPREAD bytes
 * (but at most all of user memory).  The amount of time to reclaim
 * a page once the pageout process examines it increases with this
 * distance and decreases as the scan rate rises.
 */
#define	HANDSPREAD	(2 * 1024 * 1024)

/*
 * Paged text files that are less than PGTHRESH bytes
 * may be "prefaulted in" instead of demand paged.
 */
#define PGTHRESH	(32 * 1024)

/*
 * Functions which return a physical page ID return NOPAGE if
 * there is no valid physical page ID.
 */
#define NOPAGE		((unsigned int)-1)

/*
 * PSPACE_MAINSTORE is the code for the physical address space which includes,
 * at least, "mainstore" system memory, which is the memory that programs
 * (and the kernel) execute out of.  See hat_getppfnum(D3K).
 */
#define PSPACE_MAINSTORE	0

#endif	/* _SYS_VMPARAM_H */
