/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/coff.h	1.3"

	/* COFF STRUCTURES */

#define FILE_MAGIC_NUM		0x14c
#define AOUT_MAGIC_NUM		0x10b

	/* File Header */

struct filehdr {
	unsigned short	f_magic;
	unsigned short	f_nscns;
	long			f_timdat;
	long			f_symptr;
	long			f_nsyms;
	unsigned short	f_opthdr;
	unsigned short	f_flags;
};

#define FILHDR	struct filehdr
#define FILHSZ	sizeof(FILHDR)

	/* A.OUT Header */

struct aouthdr {
	short		magic;
	short		vstamp;
	long		tsize;
	long		dsize;
	long		bsize;
	long		xentry;
	long		text_start;
	long		data_start;
};

#define AOUTHDR	struct aouthdr
#define AOUTHSZ	sizeof(AOUTHDR)

	/* Section Header */

struct scnhdr {
	char			s_name[8];
	long			s_paddr;
	long			s_vaddr;
	long			s_size;
	long			s_scnptr;
	long			s_relptr;
	long			s_lnnoptr;
	unsigned short	s_nreloc;
	unsigned short	s_nlnno;
	long			s_flags;
};

#define SCNHDR	struct scnhdr
#define SCNHSZ	sizeof(SCNHDR)

	/* Section Type */

#define STYP_TEXT	0x20
#define STYP_DATA	0x40
#define STYP_BSS	0x80
#define STYP_INFO	0x200
