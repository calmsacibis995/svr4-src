/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devintf:mkdtab/mkdtab.h	1.1.2.1"

#define	ALIASMAX	16
#define	DESCMAX		128

/* generic devices for auto-density detect; at386 floppy disk driver only */
#define	FBDEV	"/dev/dsk/f%ldt"		  /* generic block device */
#define	FCDEV	"/dev/rdsk/f%ldt"		  /* generic char  device */

#define	FDESC1	"5.25 inch 360 Kbyte (Low Density)"	/* floppy type      */
#define	FBDEV1	"/dev/dsk/f%ldd9dt"		/* floppy block device mask */
#define	FCDEV1	"/dev/rdsk/f%ldd9dt"		/* floppy char device mask  */
#define	FDENS1	"mdens%dLOW"			/* floppy density mask/value*/
#define	FBLK1	702				/* number of blocks	    */
#define	FINO1	160				/* number of inodes	    */
#define	FBPC1	18				/* blocks per cyllinder	    */

#define	FDESC2	"5.25 inch 1.2 Mbyte (High Density)"
#define	FBDEV2	"/dev/dsk/f%ldq15dt"
#define	FCDEV2	"/dev/rdsk/f%ldq15dt"
#define	FDENS2	"mdens%dHIGH"
#define	FBLK2	2370
#define	FINO2	592
#define	FBPC2	30

#define	FDESC3	"3.5 inch 720 Kbyte (Low Density)"
#define	FBDEV3	"/dev/dsk/f%ld3dt"
#define	FCDEV3	"/dev/rdsk/f%ld3dt"
#define	FDENS3	"mdens%dLOW"
#define	FBLK3	1422
#define	FINO3	355
#define	FBPC3	18

#define	FDESC4	"3.5 inch 1.44 Mbyte (High Density)"
#define	FBDEV4	"/dev/dsk/f%ld3ht"
#define	FCDEV4	"/dev/rdsk/f%ld3ht"
#define	FDENS4	"mdens%dHIGH"
#define	FBLK4	2844
#define	FINO4	711
#define	FBPC4	36

#define	FDESC5	"5.25 inch 720 Kbyte (Medium Density)"
#define	FBDEV5	"/dev/dsk/f%ld5ht"
#define	FCDEV5	"/dev/rdsk/f%ld5ht"
#define	FDENS5	"mdens%dMED"
#define	FBLK5	1404
#define	FINO5	351
#define	FBPC5	18

