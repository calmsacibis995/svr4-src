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
#ident	"@(#)fmli:wish/stubs.c	1.2"

#include <stdio.h>
#include "wish.h"
#include "token.h"

stubs()
{
}

int
(*ignoresigs())()
{
}

int
restoresigs()
{
}

osystem()
{
#ifdef _DEBUG
	_debug(stderr, "SYSTEM NOT IMPLEMENTED\n");
#endif
}

prompt()
{
#ifdef _DEBUG
	_debug(stderr, "PROMPT NOT WORKING\n");
#endif
}

suspend()
{
#ifdef _DEBUG
	_debug(stderr, "SUSPEND NOT IMPLEMENTED\n");
#endif
}

change_owns()
{
#ifdef _DEBUG
	_debug(stderr, "CHANGE_OWNS STUBBED OUT\n");
#endif
}

edit()
{
#ifdef _DEBUG
	_debug(stderr, "EDIT STUBBED OUT\n");
#endif
}
