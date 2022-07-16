/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/endwin.c	1.12"

/* Clean things up before exiting. */

#include	"curses_inc.h"

#ifdef	_VR2_COMPAT_CODE
char	_endwin = 0;
#endif	/* _VR2_COMPAT_CODE */

isendwin()
{
    /*
     * The test below must stay at TRUE because the value of 2
     * has special meaning to wrefresh but does not mean that
     * endwin has been called.
     */
    if (SP && (SP->fl_endwin == TRUE))
	return (TRUE);
    else
	return (FALSE);
}

endwin()
{
#ifdef __STDC__
    extern  int     _outch(char);
#else
    extern  int     _outch();
#endif

    /* See comment above why this test is explicitly against TRUE. */
    if (SP->fl_endwin == TRUE)
	return (ERR);

    /* Flush out any output not output due to typeahead. */
    if (_INPUTPENDING)
	(void) force_doupdate();

    /* Close things down. */
    (void) ttimeout(-1);
    if (SP->fl_meta)
	tputs(meta_off, 1, _outch);
    (void) mvcur(curscr->_cury, curscr->_curx, curscr->_maxy - 1, 0);

    if (SP->kp_state)
	tputs(keypad_local, 1, _outch);
    if (cur_term->_cursorstate != 1)
	tputs(cursor_normal, 0, _outch);

    /* don't bother turning off colors: it will be done later anyway */
    curscr->_attrs &= ~A_COLOR;		/* SS: colors */

    if ((curscr->_attrs != A_NORMAL) && (magic_cookie_glitch < 0) && (!ceol_standout_glitch))
	_VIDS(A_NORMAL, curscr->_attrs);

    if (SP->phys_irm)
	_OFFINSERT();

    SP->fl_endwin = TRUE;

#ifdef	_VR2_COMPAT_CODE
    _endwin = TRUE;
#endif	/* _VR2_COMPAT_CODE */

    curscr->_clear = TRUE;
    reset_shell_mode();
    tputs(exit_ca_mode, 0, _outch);

    /* restore colors and default color pair. SS: colors	*/
    if (orig_colors)
	tputs(orig_colors, 1, _outch);
    if (orig_pair)
        tputs(tparm (orig_pair), 1, _outch);

    /* SS-mouse: free the mouse. */

    if (get_mouse)
	tputs(tparm (get_mouse, 0), 1, _outch);

    (void) fflush(SP->term_file);
    return (OK);
}

force_doupdate()
{
    char	chars_onQ = cur_term->_chars_on_queue;
    int		ret;

    /*
     * This will cause _chkinput to return FALSE which will force wrefresh
     * to think there is no input waiting and it will finish its refresh.
     */

    cur_term->_chars_on_queue = INP_QSIZE;
    ret = doupdate();
    cur_term->_chars_on_queue = chars_onQ;
    return (ret);
}
