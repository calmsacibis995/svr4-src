/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/postplot/postplot.h	1.1.2.1"
/*
 *
 * Definitions used by the PostScript translator for Unix plot files.
 *
 * Recognized line styles are saved in an array of type Styles that's initialized
 * using STYLES.
 *
 */

typedef struct {

	char	*name;
	char	*val;

} Styles;

#define STYLES								\
									\
	{								\
	    "solid", "[]",						\
	    "dotted", "[.5 2]",						\
	    "dashed", "[4 4]",						\
	    "dotdashed", "[.5 2 4 2]",					\
	    "shortdashed", "[4 4]",					\
	    "longdashed", "[8 8]",					\
	    NULL, "[]"							\
	}

/*
 *
 * An array of type Fontmap helps convert font names requested by users into
 * legitimate PostScript names. The array is initialized using FONTMAP, which must
 * end with an entry that has NULL defined as its name field. The only fonts that
 * are guaranteed to work well are the constant width fonts.
 *
 */

typedef struct {

	char	*name;			/* user's font name */
	char	*val;			/* corresponding PostScript name */

} Fontmap;

#define FONTMAP								\
									\
	{								\
	    "R", "Courier",						\
	    "I", "Courier-Oblique",					\
	    "B", "Courier-Bold",					\
	    "CO", "Courier",						\
	    "CI", "Courier-Oblique",					\
	    "CB", "Courier-Bold",					\
	    "CW", "Courier",						\
	    "PO", "Courier",						\
	    "courier", "Courier",					\
	    "cour", "Courier",						\
	    "co", "Courier",						\
	    NULL, NULL							\
	}

/*
 *
 * Some of the non-integer functions in postplot.c.
 *
 */

char	*get_font();

