/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/restart.c	1.9"

#include	"curses_inc.h"

/*
 * This is useful after saving/restoring memory from a file (e.g. as
 * in a rogue save game).  It assumes that the modes and windows are
 * as wanted by the user, but the terminal type and baud rate may
 * have changed.
 */

extern	char	_called_before;

restartterm(term, filenum, errret)
char	*term;
int	filenum;	/* This is a UNIX file descriptor, not a stdio ptr. */
int	*errret;
{
    int	saveecho = SP->fl_echoit;
    int	savecbreak = cur_term->_fl_rawmode;
    int	savenl;

#ifdef	SYSV
    savenl = PROGTTY.c_iflag & ONLCR;
#else	/* SYSV */
    savenl = PROGTTY.sg_flags & CRMOD;
#endif	/* SYSV */

    _called_before = 0;
    (void) setupterm(term, filenum, (int *) 0);

    /* Restore curses settable flags, leaving other stuff alone. */
    SP->fl_echoit = saveecho;

    nocbreak();
    noraw();
    if (savecbreak == 1)
	cbreak();
    else
	if (savecbreak == 2)
	    raw();

    if (savenl)
	nl();
    else
	nonl();

    reset_prog_mode();

    LINES = SP->lsize;
    COLS = columns;
    return (OK);
}
