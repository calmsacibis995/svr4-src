/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/alloc.c	1.2"
/* memory allocation functions */

#include <stdio.h>
#include <string.h>

extern	char	*argv0;	/* command name (must be set in main function) */

char	*mycalloc(), *mymalloc(), *myrealloc(), *stralloc();
static	char	*alloctest();
#ifdef __STDC__
#include <stdlib.h>
# else
char	*calloc(), *malloc(), *realloc(), *strcpy();
void	exit();
# endif

/* allocate a string */

char *
stralloc(s)
char	*s;
{
	return(strcpy(mymalloc(strlen(s) + 1), s));
}

/* version of malloc that only returns if successful */

char *
mymalloc(size)
int	size;
{
	return(alloctest(malloc((unsigned) size)));
}

/* version of calloc that only returns if successful */

char *
mycalloc(nelem, size)
int	nelem;
int	size;
{
	return(alloctest(calloc((unsigned) nelem, (unsigned) size)));
}

/* version of realloc that only returns if successful */

char *
myrealloc(p, size)
char	*p;
int	size;
{
	return(alloctest(realloc(p, (unsigned) size)));
}

/* check for memory allocation failure */

static	char *
alloctest(p)
char	*p;
{
	if (p == NULL) {
		(void) fprintf(stderr, "\n%s: out of storage\n", argv0);
		exit(1);
	}
	return(p);
}
