/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V3.pechochar.c	1.1"

#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	pechochar
pechochar(win, c)
WINDOW		*win;
_ochtype	c;
{
    return (p32echochar(win, _FROM_OCHTYPE(c)));
}
#endif	/* _VR3_COMPAT_CODE */
