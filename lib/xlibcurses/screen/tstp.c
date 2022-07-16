/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/tstp.c	1.11"
#include	<signal.h>
#include	"curses_inc.h"


/* handle stop and start signals */

#ifdef	SIGTSTP
void
_tstp(dummy)
int dummy;
{
#ifdef	DEBUG
    if (outf)
	(void) fflush(outf);
#endif	/* DEBUG */
    curscr->_attrs = A_ATTRIBUTES;
    (void) endwin();
    (void) fflush(stdout);
    kill(0, SIGTSTP);
    (void) signal(SIGTSTP, _tstp);
    fixterm();
    /* changed ehr3 SP->doclear = 1; */
    curscr->_clear = TRUE;
    (void) wrefresh(curscr);
}
#endif	/* SIGTSTP */

void
_ccleanup(signo)
int	signo;
{
    (void) signal(signo, SIG_IGN);

    /*
     * Fake curses into thinking that all attributes are on so that
     * endwin will turn them off since the <BREAK> key may have interrupted
     * the sequence to turn them off.
     */

    curscr->_attrs = A_ATTRIBUTES;
    (void) endwin();
#ifdef	DEBUG
    fprintf(stderr, "signal %d caught. quitting.\n", signo);
#endif	/* DEBUG */
    if (signo == SIGQUIT)
	(void) abort();
    else
	exit(1);
}
