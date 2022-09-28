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
#ident	"@(#)fmli:vt/wgetchar.c	1.1"

#include	<curses.h>
#include	"wish.h"
#include	"vtdefs.h"
#include	"vt.h"
#include	"token.h"

token
wgetchar()
{
	register struct vt	*v;

	v = &VT_array[VT_curid];
	return (token) wgetch(v->win);
}
