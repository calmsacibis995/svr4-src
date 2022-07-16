/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* A panels subsystem built on curses--Move a panel to the bottom */

#ident	"@(#)sti:panel/bottom.c	1.3"

#include <curses.h>
#include "private.h"

	/******************/
	/*  bottom_panel  */
	/******************/

int bottom_panel (panel)
register PANEL	*panel;
{
	register PANEL	*pnl;
	register _obscured_list	*obs;

	if (!panel || panel == panel -> below)
		return ERR;

	/* If the panel is already on bottom, there is nothing to do */

	if (_Bottom_panel == panel)
		return OK;

	/* All the panels that this panel used to obscure now
	 * obscure this panel.
	 */

	for (pnl = panel->below; pnl; pnl = pnl->below)
	{
		if (obs = _unlink_obs (pnl, panel))
		{
			obs -> panel_p = pnl;
			if (panel -> obscured)
			{
				obs -> next = panel -> obscured -> next;
				panel->obscured = panel->obscured->next = obs;
			}
			else
				obs -> next = panel -> obscured = obs;
		}
	}

	/* Move the panel to the bottom */

	if (panel == _Top_panel)
		(_Top_panel = panel -> below) -> above = 0;
	else
	{
		panel -> above -> below = panel -> below;
		panel -> below -> above = panel -> above;
	}

	panel -> below = 0;
	panel -> above = _Bottom_panel;
	_Bottom_panel = _Bottom_panel -> below = panel;
	(void)touchwin (panel -> win);

	return OK;
}
