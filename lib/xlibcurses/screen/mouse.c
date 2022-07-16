/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/mouse.c	1.4"

#include "curses_inc.h"
#ifdef __STDC__
extern  int     _outch(char);
#else
extern  int     _outch();
#endif

int mouse_set(mbe)
long int mbe;
{
    if (get_mouse)
    {
	SP->_trap_mbe = mbe;
	tputs (tparm (get_mouse, mbe), 1, _outch);
	(void) fflush(SP->term_file);
	return (OK);
    }
    return (ERR);
}

int mouse_on(mbe)
long int mbe;
{
    if (get_mouse)
    {
	SP->_trap_mbe |= mbe;
	tputs (tparm (get_mouse, SP->_trap_mbe), 1, _outch);
	(void) fflush(SP->term_file);
	return (OK);
    }
    return (ERR);
}

int mouse_off(mbe)
long int mbe;
{
    if (get_mouse)
    {
	SP->_trap_mbe &= ~mbe;
	tputs (tparm (get_mouse, SP->_trap_mbe), 1, _outch);
	(void) fflush(SP->term_file);
	return (OK);
    }
    return (ERR);
}


int request_mouse_pos()
{
    register i;

    if (req_mouse_pos)
    {
	tputs (req_mouse_pos, 1, _outch);
	(void) fflush(SP->term_file);

	/* we now must wait for report of mouse position. How do */
	/* we know that this is mouse position report an not any-*/
	/* thing else?  thetch() returns KEY_MOUSE and the status*/
	/* off all the buttons remains unchanged.		 */
	/* just to avoid going into infinite loop, we have a     */
	/* counter.  if 1000 responses won't have what we need,  */
	/* we'll return error					 */

	for (i=0; i<1000; i++)
	{
	   if ((tgetch(1) == KEY_MOUSE) && MOUSE_POS_REPORT)
		break;
	}
	if (i == 1000)
	    return (ERR);
	return (OK);
    }
    return (ERR);
}

void wmouse_position (win, x, y)
WINDOW *win;
int    *x, *y;
{
	/* mouse pointer outside the window, store -1's into x and y */

	if (win->_begy > MOUSE_Y_POS || win->_begx > MOUSE_X_POS ||
	    win->_begy+win->_maxy < MOUSE_Y_POS ||
	    win->_begx+win->_maxx < MOUSE_X_POS)
	{
	    *x = -1;  *y = -1;
	}
	else
	{
	    *x = MOUSE_X_POS - win->_begx;
	    *y = MOUSE_Y_POS - win->_begy;
	}
}


map_button (a)
unsigned long	a;
{
    SP->_map_mbe_to_key = a;
    return (OK);
}


unsigned long getmouse()
{
    return (SP->_trap_mbe);
}


unsigned long getbmap()
{
    return (SP->_map_mbe_to_key);
}
