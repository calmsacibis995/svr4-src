/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:estrdup.c	1.1.3.1"
#include <string.h>

extern char	*calloc();
extern void	progerr(), exit();

char *
estrdup(s, exitcd)
char *s;
int exitcd;
{
	register char *pt;

	pt = calloc((unsigned)(strlen(s)+1), sizeof(char));
	if(!pt) {
		progerr("no memory, strdup() failed");
		exit(exitcd);
	}
	(void) strcpy(pt, s);
	return(pt);
}
