/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/errmsg.c	1.1.9.1"



#include	<stdio.h>
#include	<varargs.h>

extern	char	*errmsgs[];
extern	int	lasterrmsg;
extern	char	*cmdname;

/*
	synopsis: errmsg( msgid, (arg1, ..., argN) )
*/

/*VARARGS*/
void
errmsg( va_alist )
va_dcl
{
	va_list	args;
	int	msgid;

	va_start( args );

	msgid = va_arg( args, int );

	if( msgid >= 0 && msgid < lasterrmsg ) {
		(void) fprintf( stderr, "UX: %s: ", cmdname );
		(void) vfprintf( stderr, errmsgs[ msgid ], args );
	}

	va_end( args );
}
