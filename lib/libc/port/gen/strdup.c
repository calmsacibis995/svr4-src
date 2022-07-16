/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strdup.c	1.11"
/*LINTLIBRARY*/
/* string duplication
   returns pointer to a new string which is the duplicate of string
   pointed to by s1
   NULL is returned if new string can't be created
*/

#ifdef __STDC__
	#pragma weak strdup = _strdup
#endif
#include "synonyms.h"
#include "shlib.h"
#include <string.h>
#include <stdlib.h>

char *
strdup(s1)
const char *s1;
{  
	char * s2 = malloc(strlen(s1)+1);

	if (s2)
		strcpy(s2, s1);
	return(s2);
}
