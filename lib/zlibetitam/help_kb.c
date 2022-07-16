/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:help_kb.c	1.1"
	/*
	 *	Keyboard input and processing module for the help process
	 */

#include <tam.h>
#include <menu.h>
#include <message.h>

#include "help.h"

mitem_t	*help_items = 0;		/* Array of help menu items */

menu_t	help_menu =			/* Help menu */
	{
	"",
	"",
	"Select help display and touch ENTER",
	0, 0, 0, 0, M_SINGLE | M_WINSON,
	{0},
	0, 0, 0, 0, 0, 0, 0, 0
	};

char	index_displayed;		/* Flag if display successful */

	/*
	 *  dsp_index  -  Display table of contents of help file as a menu
	 */

dsp_index ()

	{
	register	ix;
	register char	*ptr;
	register struct	s_help	*hptr;
	register	n;
	register mitem_t	*hitems;
	int	chr;

	index_displayed = FALSE;
	if (help_items == 0)
		{
		help_menu.m_title = label;
		help_items = (mitem_t *)malloc
				((max_help_ix + 1) * sizeof (mitem_t));
		if (help_items == 0)
			{
			message (MT_ERROR, (char*)0, (char *)0, "Can't display table of contents, out of memory");
			return;
			}
		hitems = help_items;
		for (ix = 1; ix <= max_help_ix; ix++)
			{
			ptr = (char *)malloc (81);
			if (ptr == 0)
				{
				message (MT_ERROR, (char*)0, (char*)0, "Can't display table of contents, out of memory");
				return;
				}
			hptr = help_ptr + ix;
			strncpy (ptr, names [ix], Max_namelen);
			*(ptr + Max_namelen) = 0;
			n = strlen (ptr);
			while (n < Max_namelen)
				*(ptr + n++) = ' ';
			strcat (ptr, " - ");
			strncat (ptr, hptr -> title, 80 - (Max_namelen + 4));
			hitems -> mi_name = ptr;
			hitems -> mi_flags = 0;
			hitems -> mi_val = ix;
			hitems++;
			}
		hitems -> mi_name = 0;	/* Null terminate item list */
		help_menu.m_items = help_items;
		}
	help_menu.m_curi = help_items;
/***
 *** The following line makes the cursor visible
	wputs (win_id, "\033[=0C");
 ***/
	chr = menu (&help_menu, M_BEGIN);
	if (chr < 0)
		{
		message (MT_ERROR, (char*)0, (char*)0, "Can't display table of contents, out of memory");
		return;
		}
	index_displayed = TRUE;
	}

	/*
	 *  prc_kbd  -  Process keyboard input until another page selected
	 *
	 *	Returns index of new page to be displayed, help_exit to
	 *	terminate, or help_intr if window signal caught.
	 */

prc_kbd (ix)

	int	ix;			/* Index of current help display */

	{
	WSTAT	win_stat;
	register	ch;
	register struct	s_help	*hptr;
	char	looping;

	int	x, y, b, r;		/* Mouse report variables */


	if (ix == 0)
		{
		if (index_displayed)
			{
			while (TRUE)
				{
				ch = menu (&help_menu, M_DESEL | M_INPUT);
				if ((ch == Page) || (ch == s_Page) ||
				    (ch == RollUp) || (ch == RollDn))
					;
				else
				if (ch == Enter)
					{
					ch = menu (&help_menu, M_END);
					return (help_menu.m_curi -> mi_val);
					}
				else
				if ((ch == MERR_SYS) || (ch == Cancl) ||
				    (ch == s_Cancl))
					{
					ch = menu (&help_menu, M_END);
					return (last_help_ix);
					}
				else
				if ((ch == Close) || (ch == Exit))
					{
					ch = menu (&help_menu, M_END);
					return (help_exit);
					}
				else
				if (ch == Help)
					{
					ch = menu (&help_menu, M_END);
					return (fnd_name ("Using help"));
					}
				else
					kb_beep ();
				}
			}
		else
			ix = last_help_ix;
		}

	hptr = help_ptr + ix;
	looping = TRUE;
	while (looping)
		{
		wgetstat (win_id, &win_stat);
		if ((scrn_sz == win_stat.height) &&
		    (scrn_wd == win_stat.width))
			;
		else
			{
			scrn_sz = win_stat.height;
			scrn_wd = win_stat.width;
			ix = help_intr;
			break;
			}
		if ((ch = wgetc (win_id)) < 0)
			continue;
		switch (ch)
			{
			case Beg:
				dsp_beg ();
				break;

			case End:
				dsp_end ();
				break;

			case Home:
				break;

			case Next:
			case Page:
				dsp_next ();
				break;

			case Prev:
			case s_Page:
				dsp_prev ();
				break;

			case Up:
			case RollUp:
				dsp_sdn ();
				break;

			case Down:
			case RollDn:
				dsp_sup ();
				break;

			case Cmd:
			case F1:
			    looping = FALSE;
			    ix = 0;		/* 0 = Table of contents */
			    break;

			case F2:
			    if (hptr -> branch [0] <= 0)
				message (MT_ERROR, (char*)0, (char*)0, "Invalid input");
			    else
				{
				looping = FALSE;
				ix = hptr -> branch [0];
				}
			    break;

			case F3:
			    if (hptr -> branch [1] <= 0)
				message (MT_ERROR, (char*)0, (char*)0, "Invalid input");
			    else
				{
				looping = FALSE;
				ix = hptr -> branch [1];
				}
			    break;

			case F4:
			    if (hptr -> branch [2] <= 0)
				message (MT_ERROR, (char*)0, (char*)0, "Invalid input");
			    else
				{
				looping = FALSE;
				ix = hptr -> branch [2];
				}
			    break;

			case F5:
			    if (hptr -> branch [3] <= 0)
				message (MT_ERROR, (char*)0, (char*)0, "Invalid input");
			    else
				{
				looping = FALSE;
				ix = hptr -> branch [3];
				}
			    break;

			case F6:
			    if (hptr -> branch [4] <= 0)
				message (MT_ERROR, 0, 0, "Invalid input");
			    else
				{
				looping = FALSE;
				ix = hptr -> branch [4];
				}
			    break;

			case F7:
			    if (hptr -> branch [5] <= 0)
				message (MT_ERROR, (char*)0, (char*)0, "Invalid input");
			    else
				{
				looping = FALSE;
				ix = hptr -> branch [5];
				}
			    break;

			case F8:
			    if (hptr -> branch [6] <= 0)
				message (MT_ERROR, (char*)0, (char*)0, "Invalid input");
			    else
				{
				looping = FALSE;
				ix = hptr -> branch [6];
				}
			    break;

			case Help:
			    looping = FALSE;
			    ix = fnd_name ("Using help");
			    break;


			case Redraw:
		                wrefresh (-1);
			        break;


			case Exit:
			case Close:
			case s_Cancl:
			case Cancl:
			    looping = FALSE;
			    ix = help_exit;
			    break;

			default:
			    kb_beep ();
			    break;
			}
		}
	return (ix);
	}
