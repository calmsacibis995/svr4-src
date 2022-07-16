/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/atexit.c	1.4"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <stdlib.h>

#define MAXEXITFNS	37

static void	(*exitfns[MAXEXITFNS])();
static int	numexitfns = 0;

int
atexit(func)
void	(*func)();
{
	if (numexitfns >= MAXEXITFNS)
		return(-1);

	exitfns[numexitfns++] = func;
	return(0);
}

void
_exithandle()
{
	while (--numexitfns >= 0)
		(*exitfns[numexitfns])();
}
