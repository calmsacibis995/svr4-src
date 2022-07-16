/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/resetty.c	1.5"

#include	"curses_inc.h"

resetty()
{
    if ((_BR(SP->save_tty_buf)) != 0)
    {
	PROGTTY = SP->save_tty_buf;
#ifdef	DEBUG
	if (outf)
#ifdef	SYSV
	    fprintf(outf, "resetty(), file %x, SP %x, flags %x, %x, %x, %x\n",
		cur_term->Filedes, SP, PROGTTY.c_iflag, PROGTTY.c_oflag,
		PROGTTY.c_cflag, PROGTTY.c_lflag);
#else	/* SYSV */
	    fprintf(outf, "resetty(), file %x, SP %x, flags %x\n",
		cur_term->Filedes, SP, PROGTTY.sg_flags);
#endif	/* SYSV */
#endif	/* DEBUG */
	reset_prog_mode();
    }
    return (OK);
}
