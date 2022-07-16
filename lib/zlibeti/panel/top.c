/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* A panels subsystem built on curses--Move a panel to the top */

#ident	"@(#)sti:panel/top.c	1.3"

#include <curses.h>
#include "private.h"

	/***************/
	/*  top_panel  */
	/***************/

int top_panel (panel)
register PANEL	*panel;
{
	register _obscured_list	*obs;
	_obscured_list	*prev_obs, *tmp;

	if (!panel || panel == panel -> below)
		return ERR;

	/* If the panel is already on top, there is nothing to do */

	if (_Top_panel == panel)
		return OK;

	/* All the panels that used to obscure this panel are
	 * now obscured by this panel.
	 */

	if (obs = panel -> obscured)
	{
		do
		{
			prev_obs = obs;
			obs = obs -> next;
			if (tmp = prev_obs -> panel_p -> obscured)
			{
				prev_obs->next = tmp->next;
				tmp->next = prev_obs;
			}
			else
				prev_obs->next = prev_obs->panel_p->obscured = prev_obs;
			prev_obs -> panel_p = panel;
		}
		while (obs != panel -> obscured);
		panel -> obscured = 0;
	}

	/* Move the panel to the top */

	if (panel == _Bottom_panel)
		(_Bottom_panel = panel -> above) -> below = 0;
	else
	{
		panel -> above -> below = panel -> below;
		panel -> below -> above = panel -> above;
	}

	panel -> above = 0;
	panel -> below = _Top_panel;
	_Top_panel = _Top_panel -> above = panel;
	(void)touchwin (panel -> win);

	return OK;
}
