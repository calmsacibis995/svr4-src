/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:errafter.c	1.1"

/*
	Customized routine called after error message has been printed.
	Command and library version.
	Return a value to indicate action.
*/

#include	"errmsg.h"
#include	<stdio.h>
#include	<varargs.h>

int
errafter( severity, format, print_args )
int	severity;
char	*format;
va_list print_args;
{
	switch( severity ) {
	case EHALT:
		return EABORT;
	case EERROR:
		return EEXIT;
	case EWARN:
		return ERETURN;
	case EINFO:
		return ERETURN;
	}
	return ERETURN;
}
