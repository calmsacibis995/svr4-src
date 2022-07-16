/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/meta.c	1.5"

#include	"curses_inc.h"

/* TRUE => all 8 bits of input character should be passed through. */

_meta(bf)
bool	bf;
{
#ifdef __STDC__
    extern  int     _outch(char);
#else
    extern  int     _outch();
#endif
    /*
     * Do the appropriate fiddling with the tty driver to make it send
     * all 8 bits through.  On SYSV this means clearing ISTRIP, on
     * V7 you have to resort to RAW mode.
     */
#ifdef	SYSV
    if (bf)
	PROGTTY.c_iflag &= ~ISTRIP;
    else
	PROGTTY.c_iflag |= ISTRIP;
    reset_prog_mode();
#else	/* SYSV */
    if (bf)
	raw();
    else
	noraw();
#endif	/* SYSV */

    /* Do whatever is needed to put the terminal into meta-mode. */

    if (SP->fl_meta = bf)
	tputs(meta_on, 1, _outch);
    else
	tputs(meta_off, 1, _outch);
    (void) fflush(SP->term_file);
    return (OK);
}
