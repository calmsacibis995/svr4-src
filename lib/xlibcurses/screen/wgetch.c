/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wgetch.c	1.8.1.1"
#include	"curses_inc.h"
#ifdef	DEBUG
#include	<ctype.h>
#endif	/* DEBUG */

/*
 * This routine reads in a character from the window.
 *
 * wgetch MUST return an int, not a char, because it can return
 * things like ERR, meta characters, and function keys > 256.
 */

int
wgetch(win)
register	WINDOW	*win;
{
#ifdef __STDC__
    extern	int	_outch(char);
#else
    extern	int	_outch();
#endif
    register	int	inp;
    bool		weset = FALSE;

#ifdef	DEBUG
    if (outf)
    {
	fprintf(outf, "WGETCH: SP->fl_echoit = %c, cur_term->fl_rawmode = %c\n",
	    SP->fl_echoit ? 'T' : 'F', cur_term->fl_rawmode ? 'T' : 'F');
	fprintf(outf, "_use_keypad %d, kp_state %d\n", win->_use_keypad, SP->kp_state);
	fprintf(outf, "file %x fd %d\n", SP->input_file, fileno(SP->input_file));
    }
#endif	/* DEBUG */

    if (SP->fl_echoit && cur_term->_fl_rawmode == 0)
    {
	cbreak();
	weset++;
    }

    /* Make sure we are in proper nodelay state and not in halfdelay state */
    if (cur_term->_delay <= 0 && cur_term->_delay != win->_delay)
        (void) ttimeout(win->_delay);

    if ((win->_flags & (_WINCHANGED | _WINMOVED)) && !(win->_flags & _ISPAD))
	(void) wrefresh(win);

    if ((cur_term->_ungotten == 0) && (req_for_input))
    {
	tputs(req_for_input, 1, _outch);
	(void) fflush(SP->term_file);
    }
    inp = tgetch((int) (win->_use_keypad ? 1 + win->_notimeout : 0));

	/* echo the key out to the screen */
    if (SP->fl_echoit && (inp < 0400) && (inp >= 0) && !(win->_flags&_ISPAD))
	(void) wechochar(win, (chtype) inp);

    /*
     * Do nl() mapping. nl() affects both input and output. Since
     * we turn off input mapping of CR->NL to not affect input
     * virtualization, we do the mapping here in software.
     */
    if (inp == '\r' && !SP->fl_nonl)
	inp = '\n';

    if (weset)
	nocbreak();

#ifdef	DEBUG
	if (outf) fprintf(outf, "getch returns 0%o (%s), pushed 0%o (%s) 0%o (%s) 0%o (%s)\n", inp, isascii(inp) ? unctrl(inp) : " ", SP->input_queue[0], isascii(SP->input_queue[0]) ? unctrl(SP->input_queue[0]) : " ", SP->input_queue[1], isascii(SP->input_queue[1]) ? unctrl(SP->input_queue[1]) : " ", SP->input_queue[2], isascii(SP->input_queue[2]) ? unctrl(SP->input_queue[2]) : " ");
#endif	/* DEBUG */
    return (inp);
}
