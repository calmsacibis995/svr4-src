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
#ident	"@(#)fmli:wish/error.c	1.2"

#include	<stdio.h>
#include	"wish.h"
#include	"token.h"

token
error_stream(t)
register token	t;
{
	if (t == TOK_LOGOUT || t == TOK_NOP)
		return t;
	flushinp();
	switch (t) {
	case TOK_BADCHAR:
	default:
#ifdef _DEBUG
		_debug(stderr, "error: token = %d\n", t);
#endif
		beep();
		break;
	}
	return TOK_NOP;
}
