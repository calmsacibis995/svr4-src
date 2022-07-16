/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/cuserid.c	1.14"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak cuserid = _cuserid
#endif
#include "synonyms.h"
#include <stdio.h>
#include <pwd.h>
#include <string.h>

extern char *getlogin();
extern int getuid();
extern struct passwd *getpwuid();
static char res[L_cuserid];

char *
cuserid(s)
	char	*s;
{
	register struct passwd *pw;
	register char *p;

	if (s == 0)
		s = res;
	p = getlogin();
	if (p != 0)
		return strcpy(s, p);
	pw = getpwuid(getuid());
	if (pw != 0)
		return strcpy(s, pw->pw_name);
	*s = '\0';
	return 0;
}
