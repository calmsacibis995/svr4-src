/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:qued/syncfield.c	1.2"

#include <stdio.h>
#include <curses.h>
#include "token.h"
#include "winp.h"
#include "fmacs.h"

syncfield(fld)
ifield *fld;
{
	savefield = Cfld;
	if (fld != NULL)
		Cfld = fld;
	/*
	 * syncronizes the visible window with the value 
	 * buffer (or scroll buffer for scrolling fields)
	 */
	Cfld = savefield;
}
