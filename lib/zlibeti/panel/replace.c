/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* A panels subsystem built on curses--Replace the window in a panel */

#ident	"@(#)sti:panel/replace.c	1.4"

#include <curses.h>
#include "private.h"

	/*******************/
	/*  replace_panel  */
	/*******************/

int replace_panel (panel, window)
register PANEL	*panel;
WINDOW	*window;
{
	if (!panel || !window)
		return ERR;

	/* pre-allocate the overlap nodes if the panel is not hidden */

	if (panel != panel -> below)
	{
		if (!_alloc_overlap (_Panel_cnt - 1))
			return ERR;

		/* Remove the window from the old location. */

		_remove_overlap (panel);
	}

	/* Find the size of the new window */

	getbegyx (window, panel -> wstarty, panel -> wstartx);
	getmaxyx (window, panel -> wendy, panel -> wendx);
	panel -> win = window;
	panel -> wendy += panel -> wstarty - 1;
	panel -> wendx += panel -> wstartx - 1;

	/* Determine which panels the new panel obscures (if not hidden) */

	if (panel != panel -> below)
		_intersect_panel (panel);
	(void)touchwin (window);
	return OK;
}
