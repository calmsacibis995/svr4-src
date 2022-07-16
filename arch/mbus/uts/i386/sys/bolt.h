/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_BOLT_H
#define _SYS_BOLT_H

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/bolt.h	1.3"

#define	BOLT_SIZE		42		/* 42 bytes of BOLT */
#define	MAGIC_WORD_1		0xb00f10ad
#define	VERSION			2
#define	TYPES			3		/* 32-bit code and data */
#define	SECOND_STAGE_OFFSET	1024		/* offset on disk/tape for 2nd stage */

struct	tbl_entry	{
	unsigned long	byte_offset;
	unsigned short	length;
};

/* packed size of this structure is BOLT_SIZE and should be consistent when
 * changed.
 */
struct	bolt_def	{
	unsigned long	reserved[4];
	unsigned long	magic_word_1;
	unsigned long	magic_word_2;
	unsigned short	version;
	unsigned short	types;
	unsigned long	data_size;
	unsigned long	num_entries;
	struct	tbl_entry	tbl_entries;
};

typedef	struct	bolt_def	BOLT;

#endif	/* _SYS_BOLT_H */
