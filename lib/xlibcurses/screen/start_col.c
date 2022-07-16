/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:start_col.c	1.6"

#include "curses_inc.h"

#ifdef PC6300PLUS
#include <fcntl.h>
#include <sys/console.h>
#endif

int start_color()
{
    register short  i, j;
    register _Color *color_tbl;
#ifdef __STDC__
    extern  int     _outch(char);
#else
    extern  int     _outch();
#endif
#ifdef PC6300PLUS
    struct console  con;
#endif

    /* if not a color terminal, return error    */

    if ((COLOR_PAIRS = max_pairs) == -1)
         return (ERR);

    /* we have only 6 bits to store color-pair info	*/

    if (COLOR_PAIRS > 64)
	COLOR_PAIRS = 64;

#ifdef PC6300PLUS
    ioctl (cur_term->Filedes, CONIOGETDATA, &con);
    if (!con.color)
	return (ERR);
#endif

    /* allocate pairs_tbl      */

    if ((cur_term->_pairs_tbl =
         (_Color_pair *) malloc ((COLOR_PAIRS+1) * sizeof (_Color_pair))) == NULL) 
         goto err2;

    COLORS = max_colors;

/*  the following is not required because we assume that color 0 is */
/*  always a default background.  if this will change, we may want  */
/*  to store the default colors in entry 0 of pairs_tbl.	    */
/*
    cur_term->_pairs_tbl[0].foreground = 0;
    cur_term->_pairs_tbl[0].background = COLORS;
*/

    /* if terminal can change the definition of the color */
    /* allocate color_tbl                  		  */

    if (can_change)
       if ((color_tbl = (cur_term->_color_tbl =
            (_Color *) malloc (COLORS * sizeof (_Color)))) == NULL)
	    goto err1;

    /* allocate color mark map for cookie terminals */

    if (ceol_standout_glitch || (magic_cookie_glitch >= 0))
    {
	register	int	i, nc;
	register	char	**marks;

	if ((marks = (char **) calloc((unsigned) LINES, sizeof(char *))) == NULL)
	    goto err;
	SP->_color_mks = marks;
	nc = (COLS / BITSPERBYTE) + (COLS % BITSPERBYTE ? 1 : 0);
	if ((*marks = (char *) calloc((unsigned) nc * LINES, sizeof(char))) == NULL)
	{
	    free (marks);
err:	    free (color_tbl);
err1:	    free (cur_term->_pairs_tbl);
err2:	    return (ERR);
	}

	for (i = LINES - 1; i-- > 0; ++marks)
	    *(marks + 1) = *marks + nc;
    }

    if (can_change)
    {
       /* initialize color_tbl with the following colors: black, blue, */
       /* green, cyan, red, magenta, yellow, black.  if table has more */
       /* than 8 entries, use the same 8 colors for the following 8    */
       /* positions, and then again, and again ....   If table has less*/
       /* then 8 entries, use as many colors as will fit in.           */

       for (i=0; i<COLORS; i++)
       {    j = i%8;

        if (j%2)
            color_tbl[i].b = 1000;
	else
            color_tbl[i].b = 0;

        if ((j%4) > 1)
            color_tbl[i].g = 1000;
	else
            color_tbl[i].g = 0;

        if (j > 3)
            color_tbl[i].r = 1000;
	else
            color_tbl[i].r = 0;
       }

       if (orig_colors)
           tputs (orig_colors, 1, _outch);
    }

    if (orig_pair)
        tputs (tparm (orig_pair), 1, _outch);

    /* for Tek terminals set the background color to zero */

    if (set_background)
    {
    	tputs (tparm (set_background, 0), 1, _outch);
	cur_term->_cur_pair.background = 0;
	cur_term->_cur_pair.foreground = -1;
    }
    return (OK);
}
