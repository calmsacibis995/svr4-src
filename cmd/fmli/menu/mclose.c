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

#ident	"@(#)fmli:menu/mclose.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"menu.h"
#include	"var_arrays.h"

void
menu_close(mid)
menu_id	mid;
{
	register struct menu	*m;

	if (MNU_curid == mid)
		MNU_curid = -1;
	_menu_cleanup();
	m = &MNU_array[mid];
	vt_close(m->vid);
	m->flags = 0;
}
