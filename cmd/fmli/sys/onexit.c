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
#ident	"@(#)fmli:sys/onexit.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"

extern int	(**Onexit)();

void
onexit(func)
int	(*func)();
{
	Onexit = (int (**)()) array_check_append(sizeof(int (*)()), (struct v_array *) Onexit, &func);
}
