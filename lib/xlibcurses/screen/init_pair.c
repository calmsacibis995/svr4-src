/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:init_pair.c	1.9"

#include "curses_inc.h"

init_pair (pair, f, b)
register short pair, f, b;
{
    register _Color_pair *ptp;       /* pairs table pointer */
    void     _init_HP_pair();

    /* check the validity of arguments                      */

    if (pair < 1 || pair >= COLOR_PAIRS ||
        f < 0 || b < 0 || f >= COLORS || b >= COLORS)
        return (ERR);

    ptp = cur_term->_pairs_tbl + pair;

    /* update the pairs table (if no changes just return)   */

    if (ptp->foreground == f && ptp->background == b)
        return (OK);

    ptp->foreground = f;
    ptp->background = b;

    /* if we are on terminal that cannot define color pairs (Tek) */
    /* and "pair" was previously defined, go through the curscr   */
    /* and erase information from the color field at all positions*/
    /* that use that color pair (this way when refresh will be    */
    /* called next time, it will be forced to change the color at */
    /* these positions                                            */

    if (!initialize_pair)
    {   if (ptp->init)
        {
            register short  i, j;
	    register short  lin = curscr->_maxy;
            register chtype **y = curscr->_y;
	    register bool change = FALSE;
	    register short top = -1;
	    register short bottom = -1;
            register chtype new_pair = COLOR_PAIR (pair);
    
	    /* must use lin=curscr->_maxy rather then LINES, because */
	    /* LINES could have been decremented by ripoffline()     */

            for (i=0; i<lin; i++)
	    {
		 change = FALSE;
                 for (j=0; j<COLS; j++)
                 {
                      if ((y[i][j] & A_COLOR) == new_pair)
		      {
                           y[i][j] &= ~A_COLOR;
			   change = TRUE;
		      }
		      if (change)
		      {
			  wtouchln(_virtscr, i, 1, -1);
			  if (top == -1)
			      top = i;
			  bottom = i;
			  curscr->_attrs &= ~A_COLOR;
		      }
                 }
		 if (top != -1)
		 {   _VIRTTOP = top;
		     _VIRTBOT = bottom;
		 }
	    }

        }
    }
    
    /* on terminals that can initialize color pairs (HP)    */
    /* send an escape to initialize the new pair            */

    else
	_init_HP_pair (pair, f, b);

    /* if that pair has not been previously initialized, it could not */
    /* have been  used on the screen, so we don't have to do refresh  */

    if (ptp->init)
        wrefresh (_virtscr);
    else
        ptp->init = TRUE;

    return (OK);
}



void _init_HP_pair (pair, f, b)
register short pair, f, b;
{
    register _Color *ctp = cur_term->_color_tbl;  /* color table pointer */
#ifdef __STDC__
    extern   int    _outch(char);
#else
    extern   int    _outch();
#endif

    if (initialize_pair)
        tputs (tparm (initialize_pair, (long) pair,
				       (long) ctp[f].r, (long) ctp[f].g, (long) ctp[f].b,
                                       (long) ctp[b].r, (long) ctp[b].g, (long) ctp[b].b),
                  1, _outch);
}
