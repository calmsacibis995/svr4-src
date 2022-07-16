/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_ALTTBL_H
#define _SYS_ALTTBL_H
#ident	"@(#)head.sys:sys/alttbl.h	1.1.2.1"

/*
 * ALTTBL.H
 *
 * This file defines the bad block table for the hard disk driver.
 *	The same table structure is used for the bad track table.
*/

#define MAX_ALTENTS     253	/* Maximum # of slots for alts	*/
				/* allowed for in the table.	*/

#define ALT_SANITY      0xdeadbeef      /* magic # to validate alt table */
#define ALT_VERSION	0x02		/* version of table 		 */

struct  alt_table {
	ushort  alt_used;	/* # of alternates already assigned	*/
	ushort  alt_reserved;	/* # of alternates reserved on disk	*/
	daddr_t alt_base;	/* 1st sector (abs) of the alt area	*/
	daddr_t alt_bad[MAX_ALTENTS];	/* list of bad sectors/tracks	*/
};

struct alt_info {	/* table length should be multiple of 512	*/
	long    alt_sanity;	/* to validate correctness		*/
	ushort  alt_version;	/* to corroborate vintage		*/
	ushort  alt_pad;	/* padding for alignment		*/
	struct alt_table alt_trk;	/* bad track table	*/
	struct alt_table alt_sec;	/* bad sector table	*/
};

#endif /* _SYS_ALTTBL_H */
