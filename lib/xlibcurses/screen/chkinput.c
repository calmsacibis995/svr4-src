/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/chkinput.c	1.8"
/*
 * chkinput()
 *
 * Routine to check to see if any input is waiting.
 * It returns a 1 if there is input waiting, and a zero if
 * no input is waiting.
 *
 * This function replaces system calls to ioctl() with the FIONREAD
 * parameter. It enables curses to stop a screen refresh whenever
 * a character is input.
 * Standard BTL UNIX 4.0 or 5.0 does not handle FIONREAD.
 * Changes have been made to 'wrefresh.c' to
 * call this routine as "_inputpending = chkinput()".
 * (delay.c and getch.c also use FIONREAD for nodelay, select and fast peek,
 * but these routines have not been changed).
 *
 * Philip N. Crusius - July 20, 1983
 * Modified to handle various systems March 9, 1984 by Mark Horton.
 */
#include	"curses_inc.h"

#ifdef	FIONREAD
#define	HAVE_CHK

_chkinput()
{
    int	i;

    ioctl(SP->check_fd, FIONREAD, &i);
    return (i > 0);
}
#endif	/* FIONREAD */

#ifdef	SELECT
#ifndef	HAVE_CHK
#define	HAVE_CHK
_chkinput()
{
    struct	_timeval
    {
	long	tv_sec;
	long	tv_usec;
    };
    int		ifds, ofds, efds, n;
    struct	_timeval	tv;

    ifds = 1 << SP->check_fd;
    ofds = efds = 0;
    tv.tv_sec = tv.t_usec = 0;
    n = select(20, &ifds, &ofds, &efds, &tv);
    return (n > 0);
}
#endif	/* HAVE_CHK */
#endif	/* SELECT */

#ifndef	HAVE_CHK
#ifdef	SYSV

_chkinput()
{
    unsigned	char	c;	/* character input */

    /*
     * Only check typeahead if the user is using our input
     * routines. This is because the read below will put
     * stuff into the inputQ that will never be read and the
     * screen will never get updated from now on.
     * This code should GO AWAY when a poll() or FIONREAD can
     * be done on the file descriptor as then the check
     * will be non-destructive.
     */

    if (!cur_term->fl_typeahdok || (cur_term->_chars_on_queue == INP_QSIZE) ||
	(cur_term->_check_fd < 0))
    {
	goto bad;
    }

    /* If input is waiting in curses queue, return (TRUE). */

    if ((int) cur_term->_chars_on_queue > 0)
    {
#ifdef	DEBUG
	if (outf)
	{
	    (void) fprintf(outf, "Found a character on the input queue\n");
	    _print_queue();
	}
#endif	/* DEBUG */
	goto good;
    }

    if (read(cur_term->_check_fd, (char *) &c, 1) > 0)
    {
#ifdef	DEBUG
	if (outf)
	{
	    (void) fprintf(outf, "Reading ahead\n");
	}
#endif	/* DEBUG */
	/*
	 * A character was waiting.  Put it at the end
	 * of the curses queue and return 1 to show that
	 * input is waiting.
	 */
#ifdef	DEBUG
	if (outf)
	    _print_queue();
#endif	/* DEBUG */
	cur_term->_input_queue[cur_term->_chars_on_queue++] = c;
good:
	return (TRUE);
    }
    else	/* No input was waiting so return 0. */
    {
#ifdef	DEBUG
	if (outf)
	    (void) fprintf(outf, "No input waiting\n");
#endif	/* DEBUG */
bad:
	return (FALSE);
    }
}
#else	/* SYSV */
_chkinput()
{
    return (FALSE);
}
#endif	/* SYSV */
#endif	/* HAVE_CHK */

#ifdef	DEBUG
_print_queue()	/* FOR DEBUG ONLY */
{
    int		i, j = cur_term->_chars_on_queue;
    short	*inputQ = cur_term->_input_queue;

    if (outf)
	for (i = 0; i < j; i++)
	    (void) fprintf (outf, "inputQ[%d] = %c\n", i, inputQ[i]);
}
#endif	/* DEBUG */
