/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ufs.cmds:ufs/volcopy/volcopy.h	1.1.3.1"

#define LOG
#define	MINUSA	1
#define	MINUSS	2
#define FIL_SYS	1
#define DEV_IN	2
#define VOL_IN	3
#define DEV_OUT	4
#define VOL_OUT	5
#define INPUT	0
#define OUTPUT	1
#define	T_TYPE	0xfd187e20	/* like FsMAGIC */
#define EQ(X,Y,Z) !strncmp(X,Y,Z)
#define NOT_EQ(X,Y,Z) strncmp(X,Y,Z)
#define	BLKSIZ		512	/* use physical blocks */
#define _2_DAYS		172800L
#define MAX_BLKS	20
#define Ft800x10	15L
#define Ft1600x4	22L
#define Ft1600x10	28L
#define Ft1600x16	30L
#define Ft1600x32	32L
#define Ft6250x10	90L
#define Ft6250x16	95L
#define Ft6250x32	115L
#define Ft6250x50	120L
#define	BUFCNT	2
#define	_128K	131072

/*
 * Special tape drive types.
 */

#define A_DRIVE	1	/* 3B15 Accellerated Tape Ctlr (32 blks/rec) */
#define C_DRIVE	2	/* 3B15 Compatibility Mode (10 blks/rec) */
#define K_DRIVE	3	/* 3B20 Kennedy tape drive (4 blks/rec max) */
#define T_DRIVE	4	/* 3B20 Tape File Controller (50 blks/rec) */

/* Synchronization flags */

#define	P_NONE	0	/* Null message */
#define	P_READ	1	/* Read allowed */
#define	P_WRITE	2	/* Write allowed */
#define	P_DONE	3	/* Done reading */
#define	P_ABORT	4	/* Abort due to unrecoverable error */
#define	P_TEST	5
