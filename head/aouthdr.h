#ident	"@(#)aouthdr.h	1.2	92/02/17	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:aouthdr.h	2.8"

#ifndef _AOUTHDR_H
#define _AOUTHDR_H

struct aouthdr {
	short	magic;		/* see magic.h				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to FW
				   bdry					*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/

#if defined(__STDC__)
#if #machine(u3b)
	long	dum1;
	long	dum2;		/* pad to entry point	*/
#endif
#else
#if u3b
        long    dum1;
        long    dum2;           /* pad to entry point   */
#endif
#endif  /* __STDC__ */
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
	};

#define	AOUTHDR	struct aouthdr
#define	AOUTHSZ	sizeof(AOUTHDR)

#endif /* _AOUTHDR_H */
