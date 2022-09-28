/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:errbefore.c	1.1"

/*
	Routine called before error message has been printed.
	Command and library version.
*/

#include	<varargs.h>
#include	"errmsg.h"


void
errbefore(severity, format, print_args)
int      severity;
char     *format;
va_list  print_args;
{
	switch( severity ) {
	case EHALT:
	case EERROR:
	case EWARN:
	case EINFO:
		break;
	}
	return;
}
