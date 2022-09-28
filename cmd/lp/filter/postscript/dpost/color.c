/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/dpost/color.c	1.1.2.1"

/*
 *
 * Routines that handle color requests passed through as device control commands
 * in the form "x X SetColor:red". The following PostScript procedures are needed:
 *
 *	setcolor
 *
 *	  mark /color setcolor mark
 *	  mark /color1 /color2 setcolor mark
 *
 *	    Called whenever we want to change PostScript's current color graphics
 *	    state parameter. One or two color arguments can be given. In each case
 *	    the colors are looked up in the PostScript colordict dictionary that's
 *	    defined in *colorfile. Two named colors implies reverse video printing
 *	    with the background given in /color2 and the text printed in /color1.
 *	    Unknown colors are mapped into defaults - black for a single color and
 *	    white on black for reverse video.
 *
 *	drawrvbox
 *
 *	  leftx rightx drawrvbox -
 *
 *	    Fills a box that extends from leftx to rightx with the background color
 *	    that was requested when setcolor set things up for reverse video mode.
 *	    The vertical extent of the box is determined using FontBBox just before
 *	    the first string is printed, and the height remains in effect until
 *	    there's an explicit color change. In otherwords font or size changes
 *	    won't always produce correct result in reverse video mode.
 *
 *	setdecoding
 *
 *	  num setdecoding -
 *
 *	    Selects the text decoding procedure (ie. what's assigned to PostScript
 *	    procedure t) from the decodingdefs array defined in the prologue. num
 *	    should be the value assigned to variable encoding (in dpost) and will
 *	    remain constant throughout a job, unless special features, like reverse
 *	    video printing, are requested. The text encoding scheme can be set on
 *	    the command line using the -e option. Print time and the size of the
 *	    output file will usually decrease as the value assigned to encoding
 *	    increases.
 *
 *
 * The recognized collection of "x X SetColor:" commands are:
 *
 *	x X SetColor:				selects black
 *	x X SetColor:color			selects color
 *	x X SetColor:color1 on color2		reverse video
 *	x X SetColor:color1 color2		reverse video again
 *	x X SetColor:num1 num2 num3 rgb		explicit rgb color request
 *	x X SetColor:num1 num2 num3 hsb		explicit hsb color request
 *
 * In the last three examples num1, num2, and num3 should be numbers between 0 and
 * 1 inclusive and are passed on as aguments to the approrpriate PostScript color
 * command (eg. setrgbcolor). Unknown color names (ie. the ones that setcolor
 * doesn't find in colordict) are mapped into defaults. For one color the default
 * is black, while for reverse video it's white text on a black background.
 *
 * dpost makes sure the current color is maintained across page boundaries, which
 * may not be what you want if you're using a macro package like mm that puts out
 * page footers and headers. Adding a color request to troff and keeping track of
 * the color in each environment may be the best solution.
 *
 * To get reverse video printing follow the "x X SetColor:" command with two or
 * three arguments. "x X SetColor:white on black" or "x X SetColor:white black"
 * both produce white text on a black background. Any two colors named in colordict
 * (in file *colorfile) can be chosen so "x X SetColor:yellow on blue" also works.
 * Each reverse video mode request selects the vertical extent of the background
 * box based on the font and size in use just before the first string is printed.
 * Font and/or size changes aren't guaranteed to work properly in reverse video
 * printing.
 *
 */


#include <stdio.h>
#include <ctype.h>

#include "gen.h"			/* general purpose definitions */
#include "ext.h"			/* external variable definitions */


#define DEFAULTCOLOR	"black"

char	color[50] = DEFAULTCOLOR;	/* current color */
int	gotcolor = FALSE;		/* TRUE after *colorfile is downloaded */
int	wantcolor = FALSE;		/* TRUE if we really ask for a color */


/*
 *
 * All these should be defined in dpost.c.
 *
 */


extern int	lastend;
extern int	encoding;
extern int	maxencoding;
extern int	realencoding;

extern char	*colorfile;
extern FILE	*tf;


/*****************************************************************************/


getcolor()


{


/*
 *
 * Responsible for making sure the PostScript color procedures are downloaded from
 * *colorfile. Done at most once per job, and only if the job really uses color.
 * For now I've decided not to quit if we can't read the color file.
 *
 */


    if ( gotcolor == FALSE && access(colorfile, 04) == 0 )
	doglobal(colorfile);

    if ( tf == stdout )
	gotcolor = TRUE;

}   /* End of getcolor */


/*****************************************************************************/


newcolor(name)


    char	*name;			/* of the color */


{


    char	*p;			/* next character in *name */
    int		i;			/* goes in color[i] */


/*
 *
 * Converts *name to lower case and saves the result in color[] for use as the
 * current color. The first time something other than DEFAULTCOLOR is requested
 * sets wantcolor to TRUE. Characters are converted to lower case as they're put
 * in color[] and we quit when we find a newline or get to the end of *name. The
 * isupper() test is for Berkley systems.
 *
 */


    for ( p = name; *p && (*p == ' ' || *p == ':'); p++ ) ;

    for ( i = 0; i < sizeof(color) - 1 && *p != '\n' && *p; i++, p++ )
	if ( isupper(*p) )
	    color[i] = tolower(*p);
	else color[i] = *p;

    if ( i == 0 )
	strcpy(color, DEFAULTCOLOR);
    else color[i] = '\0';

    if ( strcmp(color, DEFAULTCOLOR) != 0 )
	wantcolor = TRUE;

}   /* End of newcolor */


/*****************************************************************************/


setcolor()


{


    int		newencoding;		/* text encoding scheme that's needed */
    char	*p;			/* for converting what's in color[] */


/*
 *
 * Sets the color being used by the printer to whatever's stored as the current
 * color (ie. the string in color[]). wantcolor is only set to TRUE if we've been
 * through newcolor() and asked for something other than DEFAULTCOLOR (probably
 * black). While in reverse video mode encoding gets set to maxencoding + 1 in
 * dpost and 0 on the printer. Didn't see much point in trying to extend reverse
 * video to all the different encoding schemes. realencoding is restored when we
 * leave reverse video mode.
 *
 */


    if ( wantcolor == TRUE )  {
	endtext();
	getcolor();

	lastend = -1;
	newencoding = realencoding;

	if ( islower(color[0]) == 0 )		/* explicit rgb or hsb request */
	    fprintf(tf, "%s\n", color);
	else {
	    putc('/', tf);
	    for ( p = color; *p && *p != ' '; p++ )
		putc(*p, tf);
	    for ( ; *p && *p == ' '; p++ ) ;
	    if ( strncmp(p, "on ", 3) == 0 ) p += 3;
	    if ( *p != '\0' )  {
		fprintf(tf, " /%s", p);
		newencoding = maxencoding + 1;
	    }	/* End if */
	    fprintf(tf, " setcolor\n");
	}   /* End else */

	if ( newencoding != encoding )  {
	    encoding = newencoding;
	    fprintf(tf, "%d setdecoding\n", encoding);
	    resetpos();
	}   /* End if */
    }	/* End if */

}   /* End of setcolor */


/*****************************************************************************/

