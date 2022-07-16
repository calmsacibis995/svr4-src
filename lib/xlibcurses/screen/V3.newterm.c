/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V3.newterm.c	1.1"

#include	"curses_inc.h"
extern	int	_outchar();

#ifdef	_VR3_COMPAT_CODE
extern	void	_update_old_y_area();

#undef	newterm
SCREEN	*
newterm(type, outfptr, infptr)
char	*type;
FILE	*outfptr, *infptr;
{
    _y16update = _update_old_y_area;
    return (newscreen(type, 0, 0, 0, outfptr, infptr));
}
#endif	/* _VR3_COMPAT_CODE */
