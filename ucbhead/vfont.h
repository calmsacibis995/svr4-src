/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:vfont.h	1.1.1.1"

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


/*	@(#)vfont.h 1.7 88/08/19 SMI; from UCB 4.1 83/05/03	*/

/*
 * The structures header and dispatch define the format of a font file.
 *
 * A font file contains one struct 'header', an array of NUM_DISPATCH struct
 * 'dispatch'es, then an array of bytes containing bit maps.
 *
 * See vfont(5) for more details.
 */

#ifndef _vfont_h
#define _vfont_h

struct header {
	short		magic;		/* Magic number VFONT_MAGIC */
	unsigned short	size;		/* Total # bytes of bitmaps */
	short		maxx;		/* Maximum horizontal glyph size */
	short		maxy;		/* Maximum vertical   glyph size */
	short		xtend;		/* (unused?) */
}; 
#define	VFONT_MAGIC	0436

struct dispatch {
	unsigned short	addr;		/* &(glyph) - &(start of bitmaps) */
	short		nbytes;		/* # bytes of glyphs (0 if no glyph) */
	char		up, down, left, right;	/* Widths from baseline point */
	short		width;		/* Logical width, used by troff */
};
#define	NUM_DISPATCH	256

#endif /*!_vfont_h*/
