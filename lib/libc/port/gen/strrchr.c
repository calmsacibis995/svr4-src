/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strrchr.c	1.6"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include "synonyms.h"
#include <string.h>
#include <stddef.h>

/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
*/

char *
strrchr(sp, c)
register const char *sp;
register int c;
{
	register const char *r = NULL;

	do {
		if(*sp == c)
			r = sp;
	} while(*sp++);
	return((char *)r);
}
