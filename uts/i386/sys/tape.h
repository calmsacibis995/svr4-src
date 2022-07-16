/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TAPE_H
#define _SYS_TAPE_H

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

#ident	"@(#)head.sys:sys/tape.h	1.1.3.1"

/* 
 * Standard tape ioctl commands  
 */

#define T_BASE		('t' << 8)
#define T_RETENSION	(T_BASE | 001) 	/* Retension Tape 		*/
#define T_RWD		(T_BASE | 002)	/* Rewind Tape 			*/
#define T_ERASE		(T_BASE | 003)	/* Erase Tape 			*/
#define T_WRFILEM	(T_BASE | 004)	/* Write Filemarks		*/
#define T_RST		(T_BASE | 005)	/* Reset Tape Drive 		*/
#define T_RDSTAT	(T_BASE | 006)	/* Read Tape Drive Status 	*/
#define T_SFF		(T_BASE | 007)	/* Space Filemarks Forward 	*/
#define T_SBF		(T_BASE | 010)	/* Space Blocks Forward		*/
#define T_LOAD		(T_BASE | 011)	/* Load Tape		 	*/
#define T_UNLOAD	(T_BASE | 012)	/* Unload Tape 			*/
/* The two following ioctls are for Kennedy drives which are now not supported */
#define T_SFREC		(T_BASE | 013)	/* Seek Forward a Record 	*/
#define T_SBREC 	(T_BASE | 014)	/* Seek Backward a Record 	*/
/* The following ioctl is for older controllers which are now not supported */
#define T_TINIT 	(T_BASE | 015)	/* Initialize Tape Interface 	*/

/*	Additional tape ioctls		*/

#define	T_RDBLKLEN	(T_BASE | 016)	/* Read Block Size		*/
#define	T_WRBLKLEN	(T_BASE | 017)	/* Set Block Size		*/
#define	T_PREVMV	(T_BASE | 020)	/* Prevent Media Removal	*/
#define	T_ALLOMV	(T_BASE | 021)	/* Allow Media Removal		*/
#define T_SBB		(T_BASE | 022)	/* Space Blocks Backwards 	*/
#define T_SFB		(T_BASE | 023)	/* Space Filemarks Backwards 	*/
#define T_EOD		(T_BASE | 024)	/* Space to End Of Data		*/
#define T_SSFB		(T_BASE | 025)	/* Space Sequential Filemarks Backwards */
#define T_SSFF		(T_BASE | 026)	/* Space Sequential Filemarks Forward	*/
#define T_STS		(T_BASE | 027)	/* Set Tape Speed (1600/6250 bpi etc.)	*/
#define T_STD		(T_BASE | 030)	/* Set Tape Density (QIC-120/150 etc.)	*/

#endif	/* _SYS_TAPE_H */

