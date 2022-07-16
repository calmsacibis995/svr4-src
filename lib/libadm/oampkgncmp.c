/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libadm:oampkgncmp.c	1.1.2.1"

#include <string.h>


int
oampkgncmp(spec, p)
register char *spec, *p;
{
	if(p == (char *)NULL) {
		/* request to see if spec is a wildcard or an instance */
		if(!strcmp(spec, "all"))
			return(1); /* wild */
		spec = strchr(spec, '.');
		if(spec && !strcmp(spec, ".*"))
			return(1); /* wild */
		return(0); /* no wild */
	} else if(!strcmp(spec, "all")) {
		/* 'all' matches anything */
		return(0);
	}

	while(*spec) {
		if(*spec != *p)
			break;
		spec++, p++;
	}

	switch(spec[0]) {
	  case '\0':
		if(!p[0]) {
			/* identical match */
			return(0);
		}
		break;

	  case '.':
		if((spec[1] == '*') && !p[0]) {
			/* spec was a wildcard, but p was just an abrev */
			return(0);
		}
		break;

	  case '*':
		/* matches wildcard spec */
		return(0);
	}

	/* p does not match spec */
	return(1);
}
