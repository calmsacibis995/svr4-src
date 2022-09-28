/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/common/dev.h	1.1.2.1"

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
