/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/noraw.c	1.9"

#include	"curses_inc.h"

noraw()
{
#ifdef	SYSV
    /* Enable interrupt characters */
    PROGTTY.c_lflag |= (ISIG|ICANON);
    PROGTTY.c_cc[VEOF] = _CTRL('D');
    PROGTTY.c_cc[VEOL] = 0;
    PROGTTY.c_iflag |= IXON;
#else	/* SYSV */
    PROGTTY.sg_flags &= ~(RAW|CBREAK);
#endif	/* SYSV */

#ifdef	DEBUG
#ifdef	SYSV
    if (outf)
	fprintf(outf, "noraw(), file %x, flags %x\n",
	    cur_term->Filedes, PROGTTY.c_lflag);
#else	/* SYSV */
    if (outf)
	fprintf(outf, "noraw(), file %x, flags %x\n",
	    cur_term->Filedes, PROGTTY.sg_flags);
#endif	/* SYSV */
#endif	/* DEBUG */

    cur_term->_fl_rawmode = FALSE;
    cur_term->_delay = -1;
    reset_prog_mode();
#ifdef	FIONREAD
    cur_term->timeout = 0;
#endif	/* FIONREAD */
    return (OK);
}
