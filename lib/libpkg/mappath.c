/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:mappath.c	1.7.2.1"
#include <limits.h>
#include <string.h>
#include <ctype.h>

extern char	*getenv();
/* 0 = both upper and lower case */
/* 1 = lower case only */
/* 2 = upper case only */
#define mode(flag, pt)	(!flag || ((flag==1)&&islower(pt[1])) ||\
			((flag==2)&&isupper(pt[1])))

void
mappath(flag, path)
int flag;
char *path;
{
	char buffer[PATH_MAX];
	char varname[64];
	char *npt, *pt, *pt2, *copy;
	char *token;

	copy = buffer;
	for(pt=path; *pt; ) {
		if((*pt == '$') && isalpha(pt[1]) && mode(flag, pt) &&
		  ((pt == path) || (pt[-1] == '/'))) {
			pt2 = varname;
			for(npt=pt+1; *npt && (*npt != '/');)
				*pt2++ = *npt++;
			*pt2 = '\0';
			if((token = getenv(varname)) && *token) {
				/* copy in parameter value */
				while(*token)
					*copy++ = *token++;
				pt = npt;
			} else
				*copy++ = *pt++;
		} else if(*pt == '/') {
			while(pt[1] == '/')
				pt++;
			if((pt[1] == '\0') && (pt > path))
				break;
			*copy++ = *pt++;
		} else
			*copy++ = *pt++;
	}
	*copy = '\0';
	(void) strcpy(path, buffer);
}

void
basepath(path, basedir)
char *path;
char *basedir;
{
	char buffer[PATH_MAX];

	if(*path != '/') {
		(void) strcpy(buffer, path);
		if(basedir && *basedir) {
			while(*basedir)
				*path++ = *basedir++;
			if(path[-1] == '/')
				path--;
		}
		*path++ = '/';
		(void) strcpy(path, buffer);
	}
}

void
mapvar(flag, varname)
int flag;
char *varname;
{
	char	*token;

	if(*varname != '$')
		return;

	if(isalpha(varname[1]) && mode(flag, varname) &&
	  (token = getenv(&varname[1])) && *token) {
		/* copy token into varname */
		while(*token)
			*varname++ = *token++;
		*varname = '\0';
	}
}
