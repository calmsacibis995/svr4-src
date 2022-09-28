/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)fmli:menu/mctl.c	1.8"

#include	<stdio.h>
#include	<varargs.h>
#include	"wish.h"
#include	"ctl.h"
#include	"menu.h"
#include	"menudefs.h"

menu_ctl(mid, cmd, va_alist)
menu_id	 mid;
unsigned cmd;
va_dcl
{
    register int  retval;
    register struct menu *m;
    struct menu_line men;
    va_list args;

    retval = SUCCESS;
    if (mid < 0)
	mid = MNU_curid;
    m = &MNU_array[mid];
    va_start(args);
    switch (cmd)
    {
    case CTGETCUR:
	retval = MNU_curid;
	break;
    case CTGETPOS:
	*(va_arg(args, int *)) = m->index;
	break;
    case CTSETPOS:
	menu_index(m, va_arg(args, int), MENU_ALL);
	break;
    case CTSETATTR:
	m->flags |= MENU_MSELECT;
	break;
    case CTGETITLE:
    case CTSETITLE:
	retval = vt_ctl(m->vid, cmd, va_arg(args, char **));
	break;
    case CTGETWDW:
	retval = vt_ctl(m->vid, cmd);
	break;
    case CTGETVT:
	retval = m->vid;
	break;
    case CTGETSIZ:
	retval = m->number;
	break;
    case CTSETARG:
	m->arg = va_arg(args, char *);
	break;
    case CTSETDIRTY:
	m->flags |= MENU_DIRTY;
	break;
    case CTGETPARMS:
	*(va_arg(args, int *)) = m->topline;
	*(va_arg(args, int *)) = m->index;
	break;
    case CTSETPARMS:
    {
	int	rows, cols;

	m->topline = va_arg(args, int);
	m->index = va_arg(args, int);
	m->flags |= MENU_DIRTY;
	vt_ctl(m->vid, CTGETSIZ, &rows, &cols);
	if (m->index >= m->number)
	    m->index = m->number - 1;
    }
	break;
    case CTGETLIST:
    {
	int itemnum;
	char *item;

	itemnum = va_arg(args, int);
	item = (char *) menu_list(m, itemnum);
	*(va_arg(args, char **)) = item;
	
    }
	break;
    case CTSETSHAPE:
    {
	int	srow, scol, rows, cols;

	srow = va_arg(args, int);
	scol = va_arg(args, int);
	rows = va_arg(args, int);
	cols = va_arg(args, int);
	if (srow >= 0)
	    _menu_reshape(m, srow, scol, rows, cols);
    }
	break;
    case CTSETSTRT:
	vt_current(m->vid);	/* abs k15 */
	menu_index(m, 0, MENU_ALL);
	/*
	 * This code determines the first menu item reached
	 * when the menu is updated.
	 *
	 * Do not match inactive menu items.
	 */
	men = (*m->disp)(m->index, m->arg);
	while (men.flags & MENU_INACT) {
		if (m->index < m->number - 1) {
			menu_index(m, m->index + 1, MENU_ALL);
			men = (*m->disp)(m->index, m->arg);
		}
		else {
			menu_index(m, 0, MENU_ALL);
			men = (*m->disp)(m->index, m->arg);
		}
	 }
	break;
    default:
#ifdef _DEBUG
	_debug(stderr, "menu_ctl(%d, %d, ...) unknown command\n", mid, cmd);
#endif
	retval = FAIL;
	break;
    }
    va_end(args);
    return retval;
}
