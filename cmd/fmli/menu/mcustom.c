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

#ident	"@(#)fmli:menu/mcustom.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"menu.h"
#include	"menudefs.h"
#include	"vtdefs.h"
#include	"var_arrays.h"
#include	"ctl.h"

struct menu	*MNU_array;

menu_id
menu_custom(vid, flags, mcols, hcols, dcols, total, disp, arg)
vt_id	vid;
unsigned	flags;
unsigned	mcols;
unsigned	hcols;
unsigned	dcols;
unsigned	total;
struct menu_line	(*disp)();
char	*arg;
{
	register int	i;
	int	cols;
	int	dummy;
	register struct menu	*m;
	struct menu_line	ml;

	vt_ctl(vid, CTGETSIZ, &dummy, &cols);
	/* find a free menu structure */
	for (m = MNU_array, i = array_len(MNU_array); i > 0; m++, i--)
		if (!(m->flags & MENU_USED))
			break;
	if (i <= 0) {
		var_append(struct menu, MNU_array, NULL);
		m = &MNU_array[array_len(MNU_array) - 1];
	}
	/* set up m */
	/* "givens" */
	m->vid = vid;
	m->flags = (MENU_DIRTY | MENU_USED | (flags & ALL_MNU_FLAGS));
	m->hwidth = hcols;
	m->dwidth = dcols;
	m->number = total;
	m->disp = disp;
	m->arg = arg;
	m->index = 0;
	m->hcols = MENU_ALL;
	m->topline = -2;	/* to force complete repaint */
	if (mcols < 1)
		mcols = 1;
	m->ncols = mcols;
	return m - MNU_array;
}
