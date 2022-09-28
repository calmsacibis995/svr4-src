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
#ident	"@(#)fmli:vt/wprintf.c	1.2"

#include	<curses.h>
#include	<varargs.h>
#include	"wish.h"
#include	"vt.h"

void
wprintf(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;
	char	buf[BUFSIZ];

	va_start(ap);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	wputs(buf, NULL);
}
