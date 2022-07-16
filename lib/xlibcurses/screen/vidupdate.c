/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/vidupdate.c	1.17.1.1"
#include "curses_inc.h"

#ifdef PC6300PLUS
#include <fcntl.h>
#include <sys/console.h>
#endif

#define	NUM_OF_SPECIFIC_TURN_OFFS	3
extern	chtype	bit_attributes[];
 
int Oldcolors[] = { COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN,
		COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE };

void
vidupdate(newmode, oldmode, outc)
register chtype newmode, oldmode;
#ifdef __STDC__
register int (*outc)(char);
#else
register int (*outc)();
#endif
{
    bool color_terminal = (cur_term->_pairs_tbl) ? TRUE : FALSE;
    register chtype oldvideo = (oldmode & A_ATTRIBUTES) & ~A_COLOR;
    register chtype newvideo = (newmode & A_ATTRIBUTES) & ~A_COLOR;
    int  _change_video();
    void _change_color();

    /* if colors are used, extract the color related information from */
    /* the old and new modes and then erase color-pairs fields in     */
    /* both arguments.  					      */

    if (color_terminal)
    {
	register short oldcolor = PAIR_NUMBER (oldmode & A_COLOR);
	register short newcolor = PAIR_NUMBER (newmode & A_COLOR);
	register chtype turn_off = A_COLOR;

	/* erase information about video attributes that could not */
	/* have been used with colors				   */

	if (oldcolor == 0)
	    oldvideo &= ~turn_off;

	if (no_color_video != -1)
	    turn_off |= (((chtype) no_color_video) << 16);

	if (oldcolor != 0)
	    oldvideo &= ~turn_off;
	   

	/* if the new mode contains color information, then first deal  */
	/* with video attributes, and then with colors.  This way, color*/
	/* information will overwrite video information.		*/

	if (newcolor != 0)
	{
	    /* erase information about video attributes that should not */
	    /* be used with colors				        */

	    newvideo &= ~turn_off;

	    /* if the new and the old video modes became the same	*/
	    /* don't bother with them					*/ 

            if (newvideo != oldvideo)
	    {
	        if ((_change_video (newvideo, oldvideo, outc, TRUE)) == -1)
		{
    		    register _Color_pair *cur_pair = &cur_term->_cur_pair;
		    oldcolor = -1;
    	    	    cur_pair->background = cur_pair->foreground = -1;
		}
	    }
	    if (newcolor != oldcolor)
	        _change_color (newcolor, oldcolor, outc);
     	}

	/* new mode doesn't contain any color information.  Deal with	*/
	/* colors first (possibly turning of the colors that were 	*/
	/* contained in the oldmode, and then deal with video.  This way*/
	/* video attributes will overwrite colors.			*/

	else
	{
	    if (newcolor != oldcolor)
	        _change_color (newcolor, oldcolor, outc);
            if (newvideo != oldvideo)
	        _change_video (newvideo, oldvideo, outc, FALSE);
	}
    }
    else
	_change_video (newvideo, oldvideo, outc, FALSE);
}


int
_change_video (newmode, oldmode, outc, color_terminal)
register chtype newmode, oldmode;
#ifdef __STDC__
register int (*outc)(char);
#else
register int (*outc)();
#endif
register bool color_terminal;
{
    int rc = 0;

    /* If you have set_attributes let the terminfo writer worry about it. */

    if (!set_attributes)
    {
	/*
	 * The trick is that we want to pre-process the new and oldmode
	 * so that we now know what they will really translate to on
	 * the physical screen.
	 * In the case where some attributes are being faked
	 * we get rid of the attributes being asked for and just have
	 * STANDOUT mode set.  Therefore, if STANDOUT and UNDERLINE were
	 * on the screen but UNDERLINE was being faked to STANDOUT; and
	 * the new mode is just UNDERLINE, we will get rid of any faked
	 * modes and be left with and oldmode of STANDOUT and a new mode
	 * of STANDOUT, in which case the check for newmode and oldmode
	 * being equal will be true.
	 *
	 *
	 * This test is similar to the concept explained above.
	 * counter is the maximum attributes allowed on a terminal.
	 * For instance, on an hp/tvi950 without set_attributes
	 * the last video sequence sent will be the one the terminal
	 * will be in (on that spot).  Therefore, in setupterm.c
	 * if ceol_standout_glitch or magic_cookie_glitch is set
	 * max_attributes is set to 1.  This is because on those terminals
	 * only one attribute can be on at once.  So, we pre-process the
	 * oldmode and the newmode and only leave the bits that are
	 * significant.  In other words, if on an hp you ask for STANDOUT
	 * and UNDERLINE it will become only STANDOUT since that is the
	 * first bit that is looked at.  If then the user goes from
	 * STANDOUT and UNDERLINE to STANDOUT and REVERSE the oldmode will
	 * become STANDOUT and the newmode will become STANDOUT.
	 *
	 * This also helps the code below in that on a hp or tvi950 only
	 * one bit will ever be set so that no code has to be added to
	 * cut out early in case two attributes were asked for.
	 */

	chtype	check_faked;
	int	counter = max_attributes, i, j, tempmode;
	int	modes[2], k = (cur_term->sgr_mode == oldmode) ? 1 : 2;

	modes[0] = newmode;
	modes[1] = oldmode;

	while (k-- > 0)
	{
	    if ((check_faked = (modes[k] & cur_term->sgr_faked)) != A_NORMAL)
	    {
		modes[k] &= ~check_faked;
		modes[k] |= A_STANDOUT;
	    }

	    if ((j = counter) >= 0)
	    {
		tempmode = A_NORMAL;
		if (j > 0)
		{
		    for (i = 0; i < NUM_ATTRIBUTES; i++)
		    {
			if (modes[k] & bit_attributes[i])
			{
			    tempmode |= bit_attributes[i];
			    if (--j == 0)
				break;
			}
		    }
		}
		modes[k] = tempmode;
	    }
	}
	newmode = modes[0];
	oldmode = modes[1];
    }

    if (newmode == oldmode)
	return (rc);

#ifdef DEBUG
    if (outf)
	fprintf(outf, "vidupdate oldmode=%o, newmode=%o\n", oldmode, newmode);
#endif

    if (set_attributes)
    {
	tputs(tparm(set_attributes,
			newmode & A_STANDOUT,
			newmode & A_UNDERLINE,
			newmode & A_REVERSE,
			newmode & A_BLINK,
			newmode & A_DIM,
			newmode & A_BOLD,
			newmode & A_INVIS,
			newmode & A_PROTECT,
			newmode & A_ALTCHARSET),
		1, outc);
	rc = -1;
    }
    else
    {
	register	chtype	turn_on, turn_off;
	int			i;

	/*
	 * If we are going to turn something on anyway and we are
	 * on a glitchy terminal, don't bother turning it off
	 * since by turning something on you turn everything else off.
	 */

	if ((ceol_standout_glitch || magic_cookie_glitch >= 0) &&
		((turn_on = ((oldmode ^ newmode) & newmode)) != A_NORMAL))
	{
	    goto turn_on_code;
	}

	if ((turn_off = (oldmode & newmode) ^ oldmode) != A_NORMAL)
	{
	    /*
	     * Check for things to turn off.
	     * First see if we are going to turn off something
	     * that doesn't have a specific turn off capability.
	     * 
	     * Then check to see if, even though there may be a specific
	     * turn off sequence, this terminal doesn't have one or
	     * the turn off sequence also turns off something else.
	     */
	    if ((turn_off & ~(A_ALTCHARSET | A_STANDOUT | A_UNDERLINE)) ||
		(turn_off != (turn_off & cur_term->check_turn_off)))
	    {
		tputs(tparm (exit_attribute_mode), 1, outc);
		rc = -1;
		oldmode = A_NORMAL;
	    }
	    else
	    {
		for (i = 0; i < NUM_OF_SPECIFIC_TURN_OFFS; i++)
		{
		    if (turn_off & bit_attributes[i])
		    {
			tputs(tparm(cur_term->turn_off_seq[i]), 1, outc);
			oldmode &= ~bit_attributes[i];
			rc = -1;
		    }
		}
	    }
	}

	if ((turn_on = ((oldmode ^ newmode) & newmode)) != A_NORMAL)
	{
turn_on_code:

	    /* Check for modes to turn on. */

	    for (i = 0; i < NUM_ATTRIBUTES; i++)
		if (turn_on & bit_attributes[i])
		{
		    tputs(tparm(cur_term->turn_on_seq[i]), 1, outc);
		    rc = -1;
		    /*
		     * Keep turning off the bit(s) that we just sent to
		     * the screen.  As soon as turn_on reaches A_NORMAL
		     * we don't have to turn anything else on and we can
		     * break out of the loop.
		     */
		    if ((turn_on &= ~bit_attributes[i]) == A_NORMAL)
			break;
		}
	}

	if (magic_cookie_glitch > 0)
	    tputs(cursor_left, 1, outc);
    }
    cur_term->sgr_mode = newmode;
    return (rc);
}


void
_change_color (newcolor, oldcolor, outc)
register short newcolor, oldcolor;
#ifdef __STDC__
register int (*outc)(char);
#else
register int (*outc)();
#endif
{
#ifndef PC6300PLUS
  {
    register _Color_pair *ptp = cur_term->_pairs_tbl; /* pairs table pointer */
    register _Color_pair *cur_pair = &cur_term->_cur_pair;

    /* MORE: we may have to change some stuff, depending on whether HP terminals */
    /* will be changing the background, or not				     */

    if (newcolor == 0)
    {   
	if (orig_pair)
	    tputs (tparm (orig_pair), 1, outc);
	if (set_a_background || set_a_foreground ||
	    set_background || set_foreground)
	{
	    cur_pair->background = -1;
	    cur_pair->foreground = -1;
	}
	return;
    }

    /* if we are on HP type terminal, just send an escape sequence      */
    /* to use desired color pair (we could have done some optimization: */
    /* check if both the foreground and the background of newcolor match*/
    /* the ones of cur_term->_cur_pair.  but that will happen only when */
    /* two color pairs are defined exacly the same, and probably not    */
    /* worth the effort).                                               */

    if (set_color_pair)
        tputs (tparm (set_color_pair, newcolor), 1, outc);

    /* on Tek model we can do some optimization.  		   */

    else
    {
        if (ptp[newcolor].background != cur_pair->background)
        {
	    if (set_a_background)
                tputs (tparm (set_a_background, ptp[newcolor].background), 1, outc);
	    else if (set_background)
                tputs (tparm (set_background, Oldcolors[ptp[newcolor].background]), 1, outc);
    	    cur_pair->background = ptp[newcolor].background;
        }
        if (ptp[newcolor].foreground != cur_pair->foreground)
	{
	    if (set_a_foreground)
                tputs (tparm (set_a_foreground, ptp[newcolor].foreground), 1, outc);
	    else if (set_foreground)
                tputs (tparm (set_foreground, Oldcolors[ptp[newcolor].foreground]), 1, outc);
    	    cur_pair->foreground = ptp[newcolor].foreground;
	}
    }
  }
#else
  {
    /* the following code is for PC6300 PLUS: it uses BOLD terminfo entry for */
    /* turning on colors, and SGR0 for turning them off.  Every time a new    */
    /* color-pair is used, we are forced to do an ioctl read, and the send    */
    /* 'enter_bold_mode' escape sequence.  This could be improved  by using   */
    /* DIM, UNDERLINE, and REVERSE in addition to BOLD			      */

    struct console con;
    register _Color_pair *ptp = cur_term->_pairs_tbl; /* pairs table pointer */
    register back = ptp[newcolor].background;
    register fore = ptp[newcolor].foreground;

    (void) fflush (SP->term_file);
    ioctl (cur_term->Filedes, CONIOGETDATA, &con);
#define BOLD	4
    con.l[con.page].colors[BOLD] = ((back+back+(fore>5))*8 + fore) & 0177;
    ioctl (cur_term->Filedes, CONIOSETDATA, &con);
    tputs (enter_bold_mode, 1, outc);
  }
#endif
}
