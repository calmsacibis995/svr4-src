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
#ident	"@(#)fmli:vt/vdebug.c	1.1"

#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"var_arrays.h"

void
vt_debug()
{
	register struct vt	*v;
	register vt_id	n;

	_debug(stderr, "current = %d, front = %d\n", VT_curid, VT_front);
	for (n = VT_front; n != VT_UNDEFINED; n = v->next) {
		v = &VT_array[n];
		_debug(stderr, "VT # %2d(%2d): next = %2d, flags = 0x%x, win = 0x%x, title = '%s'\n", n, v->number, v->next, v->flags, v->win, v->title);
	}
}
