/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbtroff:nroff.d/tw.h	1.1.1.1"

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

/* typewriter driving table structure */

#define	NROFFCHARS	NCHARS	/* ought to be dynamic */

extern struct t {
	int	bset;		/* these bits have to be on */
	int	breset;		/* these bits have to be off */
	int	Hor;		/* #units in minimum horiz motion */
	int	Vert;		/* #units in minimum vert motion */
	int	Newline;	/* #units in single line space */
	int	Char;		/* #units in character width */
	int	Em;		/* ditto */
	int	Halfline;	/* half line units */
	int	Adj;		/* minimum units for horizontal adjustment */
	char	*twinit;	/* initialize terminal */
	char	*twrest;	/* reinitialize terminal */
	char	*twnl;		/* terminal sequence for newline */
	char	*hlr;		/* half-line reverse */
	char	*hlf;		/* half-line forward */
	char	*flr;		/* full-line reverse */
	char	*bdon;		/* turn bold mode on */
	char	*bdoff;		/* turn bold mode off */
	char	*iton;		/* turn italic mode on */
	char	*itoff;		/* turn italic mode off */
	char	*ploton;	/* turn plot mode on */
	char	*plotoff;	/* turn plot mode off */
	char	*up;		/* sequence to move up in plot mode */
	char	*down;		/* ditto */
	char	*right;		/* ditto */
	char	*left;		/* ditto */

	char	*codetab[NROFFCHARS-128];
	char	width[NROFFCHARS];
} t;
