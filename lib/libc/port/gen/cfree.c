/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/cfree.c	1.1"
/*LINTLIBRARY*/
/*	cfree - clear memory block
*/
#define NULL 0
#ifdef __STDC__
	#pragma weak cfree = _cfree
#endif
#include "synonyms.h"
#include "shlib.h"
#include <stdlib.h>

/*ARGSUSED*/
void
cfree(p, num, size)
char *p;
unsigned num, size;
{
	free(p);
}
