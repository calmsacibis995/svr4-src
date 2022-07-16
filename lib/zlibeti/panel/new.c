/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* A panels subsystem built on curses--create a new panel */

#ident	"@(#)sti:panel/new.c	1.3"

#include <curses.h>
#include "private.h"

	/*************/
	/*  add_top  */
	/*************/

/* Put a new or hidden panel on top of the pile */

static void add_top (panel)
register PANEL *panel;
{
	if (!_Top_panel)
	{
		panel-> below = 0;
		_Bottom_panel = panel;
	}
	else
	{
		_Top_panel -> above = panel;
		panel -> below = _Top_panel;
	}

	_Top_panel = panel;
	panel -> above = 0;
	panel -> obscured = 0;
	_Panel_cnt++;

	/* Determine which panels the new panel obscures */

	_intersect_panel (panel);
}

	/***************/
	/*  new_panel  */
	/***************/

PANEL *new_panel (window)
WINDOW *window;
{
	register PANEL	*panel;
	int	lines, cols;

	/* create a panel */

	if (!window || !_alloc_overlap (_Panel_cnt) ||
	    !(panel = (PANEL *) malloc (sizeof (PANEL))))
		return ((PANEL *) 0);

	panel -> win = window;
	getbegyx (window, panel -> wstarty, panel -> wstartx);
	getmaxyx (window, lines, cols);
	panel -> wendy = panel->wstarty + lines - 1;
	panel -> wendx = panel->wstartx + cols - 1;
	panel -> user = 0;

	/* put the new panel on top of the pile */

	add_top (panel);
	return (panel);
}

	/****************/
	/*  show_panel  */
	/****************/

int show_panel (panel)
register PANEL *panel;
{
	/* Allocate the obscured nodes for the new panel */

	if (!panel || panel != panel -> below || !_alloc_overlap (_Panel_cnt))
		return ERR;

	/* put the new panel on top of the pile */

	add_top (panel);
	(void)touchwin (panel -> win);
	return OK;
}
