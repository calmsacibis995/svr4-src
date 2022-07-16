/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:demo/menu1.c	1.1"
/*
	This program displays a sample menu.

	The following key mapping is defined by get_request().
	Note that ^X represents the character control-X.

		^Q		- end menu processing

		^N		- move to next item
		^P		- move to previous item
		home key	- move to first item
		home down	- move to last item

		left arrow	- move left to item
		right arrow	- move right to item
		down arrow	- move down to item
		up arrow	- move up to item

		^U		- scroll up a line
		^D		- scroll down a line
		^B		- scroll up a page
		^F		- scroll down a page

		^X		- clear pattern buffer
		^H <BS>		- delete character from pattern buffer
		^A		- request next pattern match
		^Z		- request previous pattern match

		^T		- toggle item
*/

#include <string.h>
#include <menu.h>

static char *	PGM	= (char *) 0;	/* program name	 */
static int	CURSES	= FALSE;	/* is curses initialized ? */

static void start_curses ()	/* curses initialization */
{
	CURSES = TRUE;
	initscr ();
	nonl ();
	raw ();
	noecho ();
	wclear (stdscr);
}

static void end_curses ()	/* curses termination */
{
	if (CURSES)
	{
		CURSES = FALSE;
		endwin ();
	}
}

static void error (f, s)	/* fatal error handler */
char * f;
char * s;
{
	end_curses ();
	printf ("%s: ", PGM);
	printf (f, s);
	printf ("\n");
	exit (1);
}

static void display_menu (m)	/* create menu windows and post */
MENU * m;
{
	WINDOW *	w;
	int		rows;
	int		cols;

	scale_menu (m, &rows, &cols);	/* get dimensions of menu */
/*
	create window 2 characters larger than menu dimensions
	with top left corner at (0, 0).  sub-window is positioned
	at (1, 1) relative to menu window origin with dimensions
	equal to the menu dimensions.
*/
	if (w = newwin (rows+2, cols+2, 0, 0))
	{
		set_menu_win (m, w);
		set_menu_sub (m, derwin (w, rows, cols, 1, 1));
		box (w, 0, 0);
		keypad (w, 1);
	}
	else
		error ("error return from newwin", NULL);

	if (post_menu (m) != E_OK)
		error ("error return from post_menu", NULL);
}

static void erase_menu (m)	/* unpost and delete menu windows */
MENU * m;
{
	WINDOW * w = menu_win (m);
	WINDOW * s = menu_sub (m);

	unpost_menu (m);
	werase (w);
	wrefresh (w);
	delwin (s);
	delwin (w);
}

/*
	define application commands
*/
#define QUIT		(MAX_COMMAND + 1)

static int get_request (w)	/* virtual key mapping */
WINDOW * w;
{
	int c = wgetch (w);	/* read a character */

	switch (c)
	{
		case 0x11:	/* ^Q */	return	QUIT;

		case 0x0e:	/* ^N */	return	REQ_NEXT_ITEM;
		case 0x10:	/* ^P */	return	REQ_PREV_ITEM;
		case KEY_HOME:			return	REQ_FIRST_ITEM;
		case KEY_LL:			return	REQ_LAST_ITEM;

		case KEY_LEFT:			return	REQ_LEFT_ITEM;
		case KEY_RIGHT:			return	REQ_RIGHT_ITEM;
		case KEY_UP:			return	REQ_UP_ITEM;
		case KEY_DOWN:			return	REQ_DOWN_ITEM;

		case 0x15:	/* ^U */	return	REQ_SCR_ULINE;
		case 0x04:	/* ^D */	return	REQ_SCR_DLINE;
		case 0x06:	/* ^F */	return	REQ_SCR_DPAGE;
		case 0x02:	/* ^B */	return	REQ_SCR_UPAGE;

		case 0x18:	/* ^X */	return	REQ_CLEAR_PATTERN;
		case 0x08:	/* ^H */	return	REQ_BACK_PATTERN;
		case 0x01:	/* ^A */	return	REQ_NEXT_MATCH;
		case 0x1a:	/* ^Z */	return	REQ_PREV_MATCH;

		case 0x14:	/* ^T */	return	REQ_TOGGLE_ITEM;
	}
	return c;
}

static int my_driver (m, c)	/* handle application commands */
MENU * m;
int c;
{
	switch (c)
	{
		case QUIT:
			return TRUE;
			break;
	}
	beep ();	/* signal error */
	return FALSE;
}

main (argc, argv)
int argc;
char * argv[];
{
	WINDOW *	w;
	MENU *		m;
	ITEM **		i;
	ITEM **		make_items ();
	void		free_items ();
	int		c, done = FALSE;

	PGM = argv[0];

	if (! (m = new_menu (make_items ())))
		error ("error return from new_menu", NULL);

	start_curses ();
	display_menu (m);
/*
	interact with user
*/
	w = menu_win (m);

	while (! done)
	{
		switch (menu_driver (m, c = get_request (w)))
		{
			case E_OK:
				break;
			case E_UNKNOWN_COMMAND:
				done = my_driver (m, c);
				break;
			default:
				beep ();	/* signal error */
				break;
		}
	}
	erase_menu (m);
	end_curses ();
	i = menu_items (m);
	free_menu (m);
	free_items (i);
	exit (0);
}

typedef struct
{
	char *		name;
	char *		desc;
}
	ITEM_RECORD;

/*
	item definitions
*/
static ITEM_RECORD signs [] =
{
	"Aries",	"The Ram",
	"Taurus",	"The Bull",
	"Gemini",	"The Twins",
	"Cancer",	"The Crab",
	"Leo",		"The Lion",
	"Virgo",	"The Virgin",
	"Libra",	"The Balance",
	"Scorpio",	"The Scorpion",
	"Sagittarius",	"The Archer",
	"Capricorn",	"The Goat",
	"Aquarius",	"The Water Bearer",
	"Pices",	"The Fishes",
	(char *) 0,	(char *) 0,
};

#define MAX_ITEM	512

static ITEM *		items [MAX_ITEM + 1]; /* item buffer */

static ITEM ** make_items () /* create the items */
{
	int i;

	for (i = 0; i < MAX_ITEM && signs[i].name; ++i)
		items[i] = new_item (signs[i].name, signs[i].desc);

	items[i] = (ITEM *) 0;
	return items;
}

static void free_items (i) /* free the items */
ITEM ** i;
{
	while (*i)
		free_item (*i++);
}

