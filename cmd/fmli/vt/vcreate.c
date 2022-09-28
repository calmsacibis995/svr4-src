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
#ident	"@(#)fmli:vt/vcreate.c	1.12"

#include	<curses.h>
#include	<term.h>
#include	<values.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"var_arrays.h"
#include	"moremacros.h"
#include	"color_pair.h"

#define area(sr1, sc1, r1, c1, sr2, sc2, r2, c2)	(_vt_overlap(sr1, r1, sr2, r2) * _vt_overlap(sc1, c1, sc2, c2))

extern int	VT_firstline;
extern int	VT_lastline;
static int	Nextrow;
static int	Nextcol;

extern int 	Color_terminal;

/*
 * only way to create a VT
 */
vt_id
vt_create(title, flags, srow, scol, rows, cols)
char	*title;
int	flags;
int	srow;
int	scol;
unsigned	rows;
unsigned	cols;
{
	register int	num;
	register int	wnum;
	register struct vt	*v;
	extern int	VT_firstline;

	if (!(flags & VT_NOBORDER)) {
		srow--;
		scol--;
		rows += 2;
		cols += 2;
	}
	if (best_place(flags, &srow, &scol, rows, cols) == FAIL) {
#ifdef _DEBUG3
		_debug3(stderr, "bestplace failed\n");
#endif
		return FAIL;
	}
	if (off_screen(srow, scol, rows, cols)) {
#ifdef _DEBUG3
		_debug3(stderr, "off_screen FAILED!!!  This should never happen here!!!\n");
#endif
		return FAIL;
	}
	srow += VT_firstline;
	/* pick a window number (if appropriate) */
	wnum = 0;
	if (!(flags & VT_NONUMBER)) {
		for (wnum = 1; ; wnum++) {
			for (v = VT_array, num = array_len(VT_array); num > 0; v++, num--)
				if (v->flags & VT_USED && v->number == wnum)
						break;
			if (num <= 0)
				break;
		}
	}
	/* find a free vt structure */
	for (v = VT_array, num = array_len(VT_array); num > 0; v++, num--)
		if (!(v->flags & VT_USED))
			break;
	if (num <= 0) {
		var_append(struct vt, VT_array, NULL);
		v = &VT_array[array_len(VT_array) - 1];
	}
	/* set up v */
	v->flags = VT_USED | (flags & (VT_NONUMBER | VT_NOBORDER));
	v->number = wnum;
	v->title = strsave(title);
	if ((v->win = newwin(rows, cols, srow, scol)) == NULL) {
#ifdef _DEBUG3
		_debug3(stderr, "newwin\n");
#endif
		return FAIL;
	}
	notimeout(v->win, TRUE);	/* no time limit for Fkey sequences */

	/*
	 * set up a subwindow for bordered windows only ....
	 */
	if (!(flags & VT_NOBORDER)) {
	    	if ((v->subwin = subwin(v->win, rows-2, cols-2, srow+1, scol+1)) == NULL) {
#ifdef _DEBUG3
			_debug3(stderr, "subwin\n");
#endif
			return FAIL;
		}
		notimeout(v->subwin, TRUE);
	}
	else
		v->subwin = NULL;

	/* syncok (sunwin, TRUE); */
 	if (Color_terminal == TRUE && !(flags & VT_NOBORDER) && Pair_set[WINDOW_PAIR]) {
 		wbkgd(v->win, COL_ATTR(A_NORMAL, WINDOW_PAIR));
 		wattrset(v->win, COL_ATTR(A_NORMAL, WINDOW_PAIR));
		if (v->subwin) { 	/* set attribute for sub window */
			wbkgd(v->subwin, COL_ATTR(A_NORMAL, WINDOW_PAIR));
 			wattrset(v->subwin, COL_ATTR(A_NORMAL, WINDOW_PAIR));
		}
 	}
	keypad(v->win, TRUE);
	if (flags & VT_NOBORDER)
		wmove(v->win, 0, 0);
	else
		wmove(v->win, 1, 1);
	v->flags |= VT_ANYDIRTY;
	return v - VT_array;
}

/*
 * find row with least overlap for any column (determined by bestcol)
 */
static int
nooverlap(sr, sc, r, c)
register int	*sr;
register int	*sc;
unsigned	r;
unsigned	c;
{
	register int	best;

	best = MAXINT;
	if (*sr >= 0)
		best = bestcol(best, sr, sc, r, c);
	else {
		register int	sofar;
		int	row;
		int	col;
		register int	savedcol;

		savedcol = *sc;
		for (row = 0; !off_screen(row, 0, r, c); row = Nextrow) {
			col = savedcol;
			if ((sofar = bestcol(best, &row, &col, r, c)) < best) {
				*sr = row;
				*sc = col;
				if ((best = sofar) == 0)
					break;
			}
		}
		if (best) {
			row = VT_lastline - VT_firstline - r;
			col = savedcol;
			if ((sofar = bestcol(best, &row, &col, r, c)) < best) {
				*sr = row;
				*sc = col;
				best = sofar;
			}
		}
	}
#ifdef _DEBUG3
	_debug3(stderr, "best is %d,%d: %d\n", *sr, *sc, best);
#endif
	return (best < MAXINT) ? SUCCESS : FAIL;
}

/*
 * find column with lowest cost, given row
 * do nothing if all columns have a cost higher than sofar
 */
static int
bestcol(sofar, sr, sc, r, c)
int	sofar;
register int	*sr;
register int	*sc;
unsigned	r;
unsigned	c;
{
	register int	best;
	int	col;

	best = sofar;
	Nextrow = VT_lastline;
	if (*sc >= 0)
		best = min(best, cover(*sr, *sc, r, c));
	else {
		Nextcol = 0;
		for (col = 0; !off_screen(*sr, col, r, c); col = Nextcol) {
#ifdef _DEBUG3
			_debug3(stderr, "%d,%d\n", *sr, col);
#endif
			if ((sofar = cover(*sr, col, r, c)) < best) {
				*sc = col;
				if ((best = sofar) == 0)
					break;
			}
		}
		if (best) {
#ifdef _DEBUG3
			_debug3(stderr, "%d,%d\n", *sr, columns - c);
#endif
			if ((sofar = cover(*sr, col = columns - c, r, c)) < best) {
				*sc = col;
				best = sofar;
			}
		}
	}
	return best;
}

/*
 * compute sum of overlapping areas of given window with all other windows
 */
static int
cover(sr, sc, r, c)
unsigned	sr;
unsigned	sc;
unsigned	r;
unsigned	c;
{
	register int	n;
	register int	sofar;
	register int	vtarea;
	register struct vt	*v;
	int	vsr, vsc, vr, vc;

	sofar = 0;
	Nextcol = columns;
	for (n = VT_front; n != VT_UNDEFINED; n = v->next) {
		v = &VT_array[n];
		getbegyx(v->win, vsr, vsc);
		vsr -= VT_firstline;
		getmaxyx(v->win, vr, vc);
		if ((vtarea = area(sr, sc, r, c, vsr, vsc, vr, vc)) > 0) {
			/* if there is an overlap with this VT */
			sofar += vtarea;
			Nextcol = min(Nextcol, vsc + vc);
			Nextrow = min(Nextrow, vsr + vr);
		}
	}
	return sofar;
}

/*
 * center the window
 */
static int
center(sr, sc, r, c)
int	*sr;
int	*sc;
unsigned	r;
unsigned	c;
{
	if (*sr < 0)
		*sr = (VT_lastline - VT_firstline - r) / 2;
	if (*sc < 0)
		*sc = (columns - c) / 2;
	return SUCCESS;
}

/*
 * make window as far as possible away from current window
 */
static int
nocovercur(sr, sc, r, c)
register int	*sr;
register int	*sc;
unsigned	r;
unsigned	c;
{
	register int	best, sofar;
	register struct vt	*v;
	int	crow, ccol;
	int	crows, ccols;

	v = &VT_array[VT_curid];
	getbegyx(v->win, crow, ccol);
	crow -= VT_firstline;
	getmaxyx(v->win, crows, ccols);
	best = sofar = area(*sr = 0, *sc = 0, r, c, crow, ccol, crows, ccols);
	if ((sofar = area(0, columns - c - 1, r, c, crow, ccol, crows, ccols)) < best) {
		best = sofar;
		*sc = columns - c - 1;
	}
	if ((sofar = area(VT_lastline - VT_firstline - r, columns - c - 1, r, c, crow, ccol, crows, ccols)) < best) {
		best = sofar;
		*sr = VT_lastline - VT_firstline - r;
		*sc = columns - c - 1;
	}
	if ((sofar = area(VT_lastline - VT_firstline - r, 0, r, c, crow, ccol, crows, ccols)) < best) {
		best = sofar;
		*sr = VT_lastline - VT_firstline - r;
		*sc = 0;
	}
	return SUCCESS;
}

/*
 * try to put window in the same place as current window
 */
static int
covercur(sr, sc, r, c)
register int	*sr;
register int	*sc;
unsigned	r;
unsigned	c;
{
	register struct vt	*v;
	int	y, x;
	int	my, mx;

	v = &VT_array[VT_curid];
	getbegyx(v->win, y, x);
	y -= VT_firstline;
	*sr = min(VT_lastline - VT_firstline - r, y);
	*sc = min(columns - c, x);
	if (!off_screen(*sr, *sc, r, c))
		return SUCCESS;
	return FAIL;
}

/*
 * find best place to put window
 */
static int
best_place(flags, startrow, startcol, rows, cols)
int	flags;
register int	*startrow;
register int	*startcol;
register unsigned	rows;
register unsigned	cols;
{
	int	(*cfunc)();		/* cost function to use */
	unsigned	cost;
	static int	(*cost_table[NUMCOSTS])() = {
		nooverlap,
		center,
		covercur,
		nocovercur,
	};

	/* we have already bumped the numbers to take care of the border.. */
	if (!fits(VT_NOBORDER, rows, cols))
		return FAIL;
	/* row and column are set */
	if (*startrow >= 0 && *startcol >= 0)
		return SUCCESS;
	/* get cost function */
	cost = (flags & VT_COSTS);
	cfunc = (cost > NUMCOSTS) ? nooverlap : cost_table[cost];
	return (*cfunc)(startrow, startcol, rows, cols);
}

/*
 * compute linear overlap between line starting at s1 and going for n1 units
 * and line starting at s2 and going for n2 units
 */
_vt_overlap(s1, n1, s2, n2)
register int	s1;
register int	n1;
register int	s2;
register int	n2;
{
	if (s2 < s1)
		return _vt_overlap(s2, n2, s1, n1);
	if (s1 + n1 < s2)
		return 0;
	if (s1 + n1 > s2 + n2)
		return n2;
	return s1 + n1 - s2;
}
