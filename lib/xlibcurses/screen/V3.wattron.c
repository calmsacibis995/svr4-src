/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V3.wattron.c	1.1"

#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
#undef	wattron
wattron(win, attrs)
WINDOW		*win;
_ochtype	attrs;
{
    win->_attrs |= _FROM_OCHTYPE(attrs) & A_ATTRIBUTES;
    return (OK);
}
#endif	/* _VR3_COMPAT_CODE */
