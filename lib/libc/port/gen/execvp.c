/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/execvp.c	1.21"
/*LINTLIBRARY*/
/*
 *	execlp(name, arg,...,0)	(like execl, but does path search)
 *	execvp(name, argv)	(like execv, but does path search)
 */
#ifdef __STDC__
	#pragma weak execlp = _execlp
	#pragma weak execvp = _execvp
#endif
#include "synonyms.h"
#include <sys/errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#define	NULL	0

static const char *execat();
extern char *getenv(), *strchr();
extern unsigned sleep();
extern int errno, execv(), execvp();

/*VARARGS1*/
int
#ifdef __STDC__
execlp(char *name, ...)
#else
execlp(name, va_alist) char *name; va_dcl
#endif
{
	va_list args;

#ifdef __STDC__
	va_start(args,);
#else
	va_start(args);
#endif
	return(execvp(name, (char **)args));
}

int
execvp(name, argv)
char	*name, *const *argv;
{
	const char	*pathstr;
	char	fname[PATH_MAX+2];
	const char	*newargs[256];
	int	i;
	register const char	*cp;
	register unsigned etxtbsy=1;
	register int eacces=0;

	if (*name == '\0') {
		errno = ENOENT;
		return(-1);
	}
	if((pathstr = getenv("PATH")) == NULL)
		pathstr = "/sbin:/usr/bin:";
	cp = strchr(name, '/')? (const char *)"": pathstr;

	do {
		cp = execat(cp, name, fname);
	retry:
		(void) execv(fname, argv);
		switch(errno) {
		case ENOEXEC:
			newargs[0] = "sh";
			newargs[1] = fname;
			for(i=1; newargs[i+1]=argv[i]; ++i) {
				if(i >= 254) {
					errno = E2BIG;
					return(-1);
				}
			}
			(void) execv((const char *)"/sbin/sh", newargs);
			return(-1);
		case ETXTBSY:
			if(++etxtbsy > 5)
				return(-1);
			(void) sleep(etxtbsy);
			goto retry;
		case EACCES:
			++eacces;
			break;
		case ENOMEM:
		case E2BIG:
		case EFAULT:
			return(-1);
		}
	} while(cp);
	if(eacces)
		errno = EACCES;
	return(-1);
}

static const char *
execat(s1, s2, si)
register const char *s1, *s2;
char	*si;
{
	register char	*s;
	register int cnt = PATH_MAX + 1; /* number of characters in s2 */

	s = si;
	while(*s1 && *s1 != ':') {
		if (cnt > 0) {
			*s++ = *s1++;
			cnt--;
		} else
			s1++;
	}
	if(si != s && cnt > 0) {
		*s++ = '/';
		cnt--;
	}
	while(*s2 && cnt > 0) {
		*s++ = *s2++;
		cnt--;
	}
	*s = '\0';
	return(*s1? ++s1: 0);
}
