/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* A panels subsystem built on curses--Miscellaneous routines */

#ident	"@(#)sti:panel/misc.c	1.4"

#include <curses.h>
#include "private.h"

PANEL	*_Bottom_panel;
PANEL	*_Top_panel;

_obscured_list	*_Free_list;

int	_Panel_cnt,
	_Free_list_cnt;

	/******************/
	/*  panel_window  */
	/******************/

/* Return the window pointer */

WINDOW *panel_window (panel)
register PANEL	*panel;
{
	return (panel ? panel -> win : 0);
}

	/*******************/
	/*  panel_userptr  */
	/*******************/

/* Return the user pointer */

char *panel_userptr (panel)
register PANEL	*panel;
{
	return (panel ? panel -> user : 0);
}

	/***********************/
	/*  set_panel_userptr  */
	/***********************/

/* set the user pointer */

int set_panel_userptr (panel, ptr)
register PANEL	*panel;
char *ptr;
{
	if (panel)
	{
		panel -> user = ptr;
		return OK;
	}
	else
		return ERR;
}

	/*****************/
	/*  panel_above  */
	/*****************/

/* Return the panel above the given panel (or the bottom panel in 0) */

PANEL *panel_above (panel)
register PANEL	*panel;
{

	if (!panel)
		return _Bottom_panel;

	return ((panel == panel -> below) ? ((PANEL *) 0) : panel -> above);
}

	/*****************/
	/*  panel_below  */
	/*****************/

/* Return the panel below the given panel (or the top panel in 0) */

PANEL *panel_below (panel)
register PANEL	*panel;
{

	if (!panel)
		return _Top_panel;

	return ((panel == panel -> below) ? ((PANEL *) 0) : panel -> below);
}

	/******************/
	/*  panel_hidden  */
	/******************/

/* Return TRUE if the panel is hidden, FALSE if not.  */

int panel_hidden (panel)
register PANEL	*panel;
{
	return ((!panel || (panel != panel -> below)) ? FALSE : TRUE);
}

	/******************/
	/*  _get_overlap  */
	/******************/

/* Get an overlap node from the free list. */

static _obscured_list *_get_overlap ()
{
	_obscured_list	*overlap;

	if (_Free_list_cnt-- > 0)
	{
		overlap = _Free_list;
		_Free_list = _Free_list -> next;
	}
	else
	{
		_Free_list_cnt = 0;
		overlap = 0;
	}

	return overlap;
}

	/*****************/
	/*  _unlink_obs  */
	/*****************/

/* Find the obscured node, if any, in the first panel which refers
 * the second panel.
 */

_obscured_list *_unlink_obs (pnl, panel)
register PANEL *pnl, *panel;
{
	register _obscured_list	*obs;
	_obscured_list	*prev_obs;

	if (!pnl -> obscured || !_panels_intersect (pnl, panel))
		return ((_obscured_list *) 0);

	obs = pnl -> obscured;
	do
	{
		prev_obs = obs;
		obs = obs -> next;
	}
	while (obs->panel_p != panel && obs != pnl->obscured);
	if (obs -> panel_p != panel)
	{
#ifdef DEBUG
		fprintf (stderr, "_unlink_obs:  Obscured panel lost\n");
#endif
		return ((_obscured_list *) 0);
	}

	if (obs == prev_obs)
		pnl -> obscured = 0;
	else
	{
		prev_obs -> next = obs -> next;
		if (obs == pnl -> obscured)
			pnl -> obscured = prev_obs;
	}
	return obs;
}

	/*************/
	/*  add_obs  */
	/*************/

/* Add an obscured node to a panel, ensuring that the obscured list is
 * ordered from top to bottom.
 */

static void add_obs (panel, obs)
register PANEL *panel;
_obscured_list *obs;
{
	register PANEL	*pnl;
	register _obscured_list	*curr_obs;
	_obscured_list	*prev_obs;

	if (!(prev_obs = panel -> obscured))
	{
		panel -> obscured = obs -> next = obs;
		return;
	}

	curr_obs = prev_obs -> next;

	for (pnl=_Top_panel; pnl != panel; pnl = pnl->below)
	{
		if (curr_obs -> panel_p == pnl)
		{
			prev_obs = curr_obs;
			curr_obs = curr_obs -> next;
			if (prev_obs == panel -> obscured)
			{
				panel -> obscured = obs;
				break;
			}
		}
	}

	obs -> next = curr_obs;
	prev_obs -> next = obs;
}

	/**********************/
	/*  _intersect_panel  */
	/**********************/

/* Create an obscured node for each panel that the given panel intersects.
 * The overlap record is always attached to the panel which is covered up.
 *
 * This routine assumes that _alloc_overlap() has been called to ensure
 * that there are enough overlap nodes to satisfy the requests.
 */

void _intersect_panel (panel)
register PANEL	*panel;
{
	register PANEL	*pnl;
	register _obscured_list	*obs;
	int	above_panel;

	above_panel = FALSE;

	for (pnl = _Bottom_panel; pnl; pnl = pnl -> above)
	{
		if (pnl == panel)
		{
			above_panel = TRUE;
			continue;
		}

		if (!_panels_intersect (pnl, panel))
			continue;	/* no overlap */

		obs = _get_overlap ();
		obs->start = (panel->wstarty >= pnl->wstarty) ?
				panel->wstarty : pnl->wstarty;
		obs->end = (panel->wendy <= pnl->wendy) ?
				panel->wendy : pnl->wendy;

		if (above_panel)
		{
			obs -> panel_p = pnl;
			if (panel -> obscured)
			{
				obs -> next = panel -> obscured -> next;
				panel -> obscured -> next = obs;
			}
			else
				obs -> next = panel -> obscured = obs;
		}
		else
		{
			obs -> panel_p = panel;
			add_obs (pnl, obs);
		}

	}
}

	/********************/
	/*  _alloc_overlap  */
	/********************/

/* Create enough obscured nodes to record all overlaps of a given
 * panel.  The obscured nodes must be pre-allocated by this routine
 * to preserve the integrity of the pile during move.
 * If the move operation fails, the pile is supposed to remain
 * unchanged.  If the obscured nodes are not allocated in advance,
 * then an allocation failure in the middle of a move could
 * leave the pile in a corrupted state with possibly no way to
 * restore the pile to its original state.
 *
 * The cnt parameter is the (worst case) number of overlap nodes which
 * are required to satisfy any request.  Return 0 on error, else non-zero
 */

int _alloc_overlap (cnt)
int	cnt;
{
	_obscured_list	*overlap;
	register int	i;

	for (i=cnt-_Free_list_cnt; i>0; i--)
	{
		if (!(overlap = (_obscured_list *) malloc (sizeof (_obscured_list))))
			return 0;

		overlap -> next = _Free_list;
		_Free_list = overlap;
		_Free_list_cnt++;
	}

	return 1;
}

	/*******************/
	/*  _free_overlap  */
	/*******************/

/* Free a single overlap node.  Don't really free it; just save it
 * on a list.
 */

void _free_overlap (overlap)
_obscured_list *overlap;
{
	overlap -> next = _Free_list;
	_Free_list = overlap;
	_Free_list_cnt++;
}
