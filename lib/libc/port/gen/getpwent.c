/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getpwent.c	1.21"
/*LINTLIBRARY*/
#ifndef DSHLIB
#ifdef __STDC__
	#pragma weak endpwent = _endpwent
	#pragma weak fgetpwent = _fgetpwent
	#pragma weak getpwent = _getpwent
	#pragma weak setpwent = _setpwent
#endif
#endif
#include "synonyms.h"
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>

static const char *PASSWD = "/etc/passwd";
static FILE *pwf = NULL;

#if DSHLIB
static struct passwd *save_passwd;
static char *line;
#else
static char line[BUFSIZ+1];
static struct passwd save_passwd;
#endif

void
setpwent()
{
	if(pwf == NULL)
		pwf = fopen(PASSWD, "r");
	else
		rewind(pwf);
}

void
endpwent()
{
	if(pwf != NULL) {
#if DSHLIB
		free(line);
		line = NULL;
		free(save_passwd);
		save_passwd = NULL;
#endif
		(void) fclose(pwf);
		pwf = NULL;
	}
}

static char *
pwskip(p)
register char *p;
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if(*p == '\n')
		*p = '\0';
	else if(*p)
		*p++ = '\0';
	return(p);
}

struct passwd *
getpwent()
{
	extern struct passwd *fgetpwent();

	if(pwf == NULL) {
		if((pwf = fopen(PASSWD, "r")) == NULL)
			return(NULL);
	}
	return (fgetpwent(pwf));
}

struct passwd *
fgetpwent(f)
FILE *f;
{
	register struct passwd *passwd;
	register char *p;
	char *end;
	long	x;

#if DSHLIB
	if (save_passwd == NULL && (((line = malloc(BUFSIZ+1)) == NULL)
		|| ((save_passwd = (struct passwd *)malloc(sizeof(struct passwd))) == NULL))) {
			errno = ENOMEM;
			return(NULL);
	}
	passwd = save_passwd;
#else
	passwd = &save_passwd;
#endif

	p = fgets(line, BUFSIZ, f);
	if(p == NULL)
		return(NULL);
	passwd->pw_name = p;
	p = pwskip(p);
	passwd->pw_passwd = p;
	p = pwskip(p);
	if (p == NULL || *p == ':') {
		/* check for non-null uid */
		errno = EINVAL;
		return (NULL);
	}
	x = strtol(p, &end, 10);	
	if (end != memchr(p, ':', strlen(p))){
		/* check for numeric value */
		errno = EINVAL;
		return (NULL);
	}
	p = pwskip(p);
	passwd->pw_uid = (x < 0 || x > MAXUID)? (UID_NOBODY): x;
	if (p == NULL || *p == ':') {
		/* check for non-null uid */
		errno = EINVAL;
		return (NULL);
	}
	x = strtol(p, &end, 10);	
	if (end != memchr(p, ':', strlen(p))) {
		/* check for numeric value */
		errno = EINVAL;
		return (NULL);
	}
	p = pwskip(p);
	passwd->pw_gid = (x < 0 || x > MAXUID)? (UID_NOBODY): x;
	passwd->pw_comment = p;
	passwd->pw_gecos = p;
	p = pwskip(p);
	passwd->pw_dir = p;
	p = pwskip(p);
	passwd->pw_shell = p;
	(void) pwskip(p);

	p = passwd->pw_passwd;
	while(*p && *p != ',')
		p++;
	if(*p)
		*p++ = '\0';
	passwd->pw_age = p;
	return(passwd);
}
