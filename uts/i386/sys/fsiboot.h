/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FSIBOOT_H
#define _SYS_FSIBOOT_H

#ident	"@(#)head.sys:sys/fsiboot.h	1.4.3.1"
#define FFSO	18	/* File system offset on floppy disk */
#define LBOOT "mUNIX"   /* The configuration program (Operating system) */
#define UNIX "unix"     /* The default AUTOBOOT program */
#define DGMON "dgmon"   	/* If either of these are being AUTOBOOTED, */
#define FILLEDT "filledt"	/* don't check for reconfig */
#define SYSTEM "system"   /* Used to compare dates */
#define AUBOOT "auto boot" 	/* auto boot after root fail to remount */
#define HMAJOR 1
#define TRUE (char)1
#define FALSE (char)0
#define FMAJOR 2
#define BSIZE 512
#define SECTSIZE 512
#define FASTBOOT "fast boot"
#define MYVTOC ((struct vtoc *) (BOOTADDR + BSIZE))
#define MYPDINFO ((struct pdinfo *)(BOOTADDR + 2 * BSIZE))
#define	restart() { RST=1; for (;;) ; }
#define MAGICMODE "magic mode"

#define S3BC_TCDRV	0x10	/* EDT name[] is a TC driver; board code is the
 				 * major number of this TC.
 				 */
#define EDT_START	P_EDT	/* origin of EDT */

struct badblock {
	long bad;
	long good;
};


#endif	/* _SYS_FSIBOOT_H */
