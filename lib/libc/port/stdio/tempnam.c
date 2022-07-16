/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/tempnam.c	1.7.1.9"
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak tempnam = _tempnam
#endif
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define max(A,B) (((A)<(B))?(B):(A))

extern int access();

#ifdef __STDC__
static char *pcopy(char *, const char *);
#else
static char *pcopy();
#endif

static char *seed="AAA";

char *
tempnam(dir, pfx)
const char *dir;		/* use this directory please (if non-NULL) */
const char *pfx;		/* use this (if non-NULL) as filename prefix */
{
	register char *p, *q, *tdir;
	int x=0, y=0, z;

	z=sizeof(P_tmpdir) - 1;
	if((tdir = getenv("TMPDIR")) != NULL) {
		x = strlen(tdir);
	}
	if(dir != NULL) {
		y=strlen(dir);
	}
	if((p=malloc((unsigned)(max(max(x,y),z)+16))) == NULL)
		return(NULL);
	if(x > 0 && access(pcopy(p, tdir), 3) == 0)
		goto OK;
	if(y > 0 && access(pcopy(p, dir), 3) == 0)
		goto OK;
	if(access(pcopy(p, P_tmpdir), 3) == 0)
		goto OK;
	if(access(pcopy(p, "/tmp"), 3) != 0)
		return(NULL);
OK:
	(void)strcat(p, "/");
	if(pfx) {
		*(p+strlen(p)+5) = '\0';
		(void)strncat(p, pfx, 5);
	}
	(void)strcat(p, seed);
	(void)strcat(p, "XXXXXX");
	q = seed;
	while(*q == 'Z')
		*q++ = 'A';
	if (*q != '\0')
		++*q;
	if(*mktemp(p) == '\0')
		return(NULL);
	return(p);
}

static char*
pcopy(space, arg)
char *space;
const char *arg;
{
	char *p;

	if(arg) {
		(void)strcpy(space, arg);
		p = space-1+strlen(space);
		if(*p == '/')
			*p = '\0';
	}
	return(space);
}
