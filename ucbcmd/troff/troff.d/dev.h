/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbtroff:troff.d/dev.h	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 


/*
	dev.h: characteristics of a typesetter
*/

struct dev {
	unsigned short	filesize;	/* number of bytes in file, */
				/* excluding dev part */
	short	res;		/* basic resolution in goobies/inch */
	short	hor;		/* goobies horizontally */
	short	vert;
	short	unitwidth;	/* size at which widths are given, in effect */
	short	nfonts;		/* number of fonts physically available */
	short	nsizes;		/* number of sizes it has */
	short	sizescale;	/* scaling for fractional point sizes */
	short	paperwidth;	/* max line length in units */
	short	paperlength;	/* max paper length in units */
	short	nchtab;		/* number of funny names in chtab */
	short	lchname;	/* length of chname table */
	short	biggestfont;	/* #chars in largest ever font */
	short	spare2;		/* in case of expansion */
};

struct Font {		/* characteristics of a font */
	char	nwfont;		/* number of width entries for this font */
	char	specfont;	/* 1 == special font */
	char	ligfont;	/* 1 == ligatures exist on this font */
	char	spare1;		/* unused for now */
	char	namefont[10];	/* name of this font (e.g., "R" */
	char	intname[10];	/* internal name (=number) on device, in ascii */
};

/* ligatures, ORed into ligfont */

#define	LFF	01
#define	LFI	02
#define	LFL	04
#define	LFFI	010
#define	LFFL	020
