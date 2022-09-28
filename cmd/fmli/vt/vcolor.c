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
#ident	"@(#)fmli:vt/vcolor.c	1.9"

#include <curses.h>
#include <term.h>
#include "wish.h"
#include "color_pair.h"
#include "moremacros.h"
#include "vtdefs.h"
#include "vt.h"

static int Numcolors = NUMDEFCOLORS; 
int Pair_set[MAXCOLPAIRS];

/*
 * Table of known colors
 */
struct ctab {
	char *colorstr;
	int id;
} Color_tab[MAXCOLORS] = {
	{ "black", COLOR_BLACK },
	{ "blue", COLOR_BLUE },
	{ "green", COLOR_GREEN },
	{ "cyan", COLOR_CYAN },
	{ "red", COLOR_RED },
	{ "magenta", COLOR_MAGENTA },
	{ "yellow", COLOR_YELLOW },
	{ "white", COLOR_WHITE },
	{ NULL, 0 }
};

/*
 * SET_SCR_COLOR sets the screen background color and refreshes
 * the screen
 */
set_scr_color(colpair, dorefresh)
int colpair;
int dorefresh;
{
	if (Pair_set[colpair]) { 
		wbkgd(stdscr, COL_ATTR(A_NORMAL, colpair));
		/*
		 * Set color attributes for the banner, message and
		 * command lines
		 */
		wbkgd(VT_array[STATUS_WIN].win, COL_ATTR(A_NORMAL, colpair));
		wattrset(VT_array[STATUS_WIN].win, COL_ATTR(A_NORMAL, colpair));
		wbkgd(VT_array[MESS_WIN].win, COL_ATTR(A_NORMAL, colpair));
		wattrset(VT_array[MESS_WIN].win, COL_ATTR(A_NORMAL, colpair));
		wbkgd(VT_array[CMD_WIN].win, COL_ATTR(A_NORMAL, colpair));
		wattrset(VT_array[CMD_WIN].win, COL_ATTR(A_NORMAL, colpair));
	}
	if (dorefresh) {
		refresh();
		/*
		 * The following lines are necessary since curses
		 * has problems with reverse video screens (e.g., xterm
		 * by default comes up with a white background)
		 */
		if (orig_colors)
			putp(orig_colors);
		if (orig_pair)
			putp(orig_pair);
	}
}

/*
 * SET_SLK_COLOR simply sets the slk color pair
 */ 
set_slk_color(colpair)
{
	slk_attrset(COL_ATTR(A_REVERSE | A_DIM, colpair));
}
	
/*
 * SETPAIR creates new color pair combinations
 */
setpair(pairnum, foreground, background)
int pairnum, foreground, background;
{
	if (foreground < 0 || background < 0) 
		Pair_set[pairnum] = FALSE;
	else if (init_pair(pairnum, foreground, background) != ERR)
		Pair_set[pairnum] = TRUE;
	else
		Pair_set[pairnum] = FALSE;
	return(Pair_set[pairnum]);
}

/*
 * SETCOLOR creates new color specifications or "tweeks" old ones.
 * (returns 1 on success and 0 on failure)
 */
setcolor(colorstr, r, g, b)
char *colorstr;
int r, g, b;
{
	register int cindex, id; 
	short oldr, oldg, oldb;
	int cant_init;

	if (!can_change_color())
		return(-1);
	cant_init = 0;
	if ((cindex = lookup_color(colorstr)) >= 0) {
		/*
		 * The color has been previously defined ...
		 * If you can't change the color specification then
		 * restore the old specification. 
		 */
		color_content(cindex, &oldr, &oldg, &oldb);
		if (init_color(cindex, r, g, b) == ERR) {
			cant_init++;
			if (init_color(cindex, oldr, oldg, oldb) == ERR)
				id = -1; 	/* just in case */
			else
				id = cindex;
		}
		else
			id = cindex;
		Color_tab[cindex].id = id;
	}
	else if ((cindex = add_color(colorstr)) >= 0) {
		/*
		 * The color is NEW ...
		 */
		if (init_color(cindex, r, g, b) == ERR)
			id = -1;
		else
			id = cindex;
		Color_tab[cindex].id = id;
	}
	else
		id = -1;
	return(cant_init ? 0 : (id >= 0));
} 

/*
 * GETCOLOR_ID returns the color identifier of the passed color string
 */
getcolor_id(colorstr)
char *colorstr;
{
	int index;

	index = lookup_color(colorstr);
	if (index >= 0)
		return(Color_tab[index].id);
	else
		return(-1);
}

/*
 * LOOKUP_COLOR returns the index of the passed color string from the
 * color table (or "-1" if the color is not in the table).
 */ 
static int
lookup_color(colorstr)
char *colorstr;
{
	register int i;

	/* put it in the color table */
	for (i = 0; i < Numcolors; i++) {
		if (strcmp(colorstr, Color_tab[i].colorstr) == 0)
			return(i);
	}
	return(-1);
}

/*
 * ADD_COLOR adds a new color to the color table if the number of colors
 * is less than COLORS (curses define for the number of colors the terminal
 * can support) and less than MAXCOLORS (color table size)
 */
static int 
add_color(colorstr)
char *colorstr;
{
	if (Numcolors < COLORS && Numcolors < MAXCOLORS) {
		Color_tab[Numcolors].colorstr = strsave(colorstr);
		return(Numcolors++);
	}
	else
		return(-1);
} 
