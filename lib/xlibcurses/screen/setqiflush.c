/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setqiflush.c	1.3"
#include	"curses_inc.h"

/*
**	Set/unset flushing the output queue on interrupts or quits.
*/

void
_setqiflush(yes)
bool	yes;
{
#ifdef SYSV
    if (yes)
	cur_term->Nttyb.c_lflag &= ~NOFLSH;
    else
	cur_term->Nttyb.c_lflag |= NOFLSH;
    reset_prog_mode();
#else	/* BSD */
#endif /* SYSV */
}
