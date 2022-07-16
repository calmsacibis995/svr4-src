/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/abs.c	1.9"
/*LINTLIBRARY*/
#include "synonyms.h"
#include "shlib.h"
#include <stdlib.h>

int
abs(arg)
register int arg;
{
	return (arg >= 0 ? arg : -arg);
}

long
labs(arg)
register long int arg;
{
	return (arg >= 0 ? arg : -arg);
}
