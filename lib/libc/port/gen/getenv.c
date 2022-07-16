/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getenv.c	1.8"
/*LINTLIBRARY*/
/*
 *	getenv(name)
 *	returns ptr to value associated with name, if any, else NULL
 */
#define NULL	0
#include "synonyms.h"
#include "shlib.h"
extern char **environ;
static char *nvmatch();

char *
getenv(name)
register char *name;
{
	register char *v, **p=environ;

	if(p == NULL)
		return(NULL);
	while(*p != NULL)
		if((v = nvmatch(name, *p++)) != NULL)
			return(v);
	return(NULL);
}

/*
 *	s1 is either name, or name=value
 *	s2 is name=value
 *	if names match, return value of s2, else NULL
 *	used for environment searching: see getenv
 */

static char *
nvmatch(s1, s2)
register char *s1, *s2;
{
	while(*s1 == *s2++)
		if(*s1++ == '=')
			return(s2);
	if(*s1 == '\0' && *(s2-1) == '=')
		return(s2);
	return(NULL);
}
