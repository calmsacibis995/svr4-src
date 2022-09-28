/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkerror.c	1.1.2.1"

#include	<stdio.h>
#include	<varargs.h>

extern	char	*errmsgs[];
extern	int	lasterrmsg;
extern	char	*brcmdname;

/*VARARGS*/
void
bkerror( va_alist )
va_dcl
{
	va_list	args;
	FILE	*fptr;
	int	msgid;
	va_start( args );
	fptr = va_arg( args, FILE *);
	msgid = va_arg( args, int );
	if( msgid < lasterrmsg ) {
		(void) fprintf( fptr, "%s: ", brcmdname );
		(void) vfprintf( fptr, errmsgs[ msgid ], args );
	}
	va_end( args );
}
