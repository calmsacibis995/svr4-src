/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/
#ident	"@(#)libadm:oampkgnm.c	1.1.2.1"

#include <ctype.h>
#include <string.h>

oampkgnm(name)
char *name;
{
	int count;

	if(!isalpha(*name))
		return(-1);

	if(!strncmp(name, "install", 7) && (!name[7] || (name[7] == '.')))
		return(-1);
	if(!strncmp(name, "all", 3) && (!name[3] || (name[3] == '.')))
		return(-1);
	if(!strncmp(name, "new", 3) && (!name[3] || (name[3] == '.')))
		return(-1);

	count = 1;
	while(*(++name) && (*name != '.')) {
		count++;
		if(!isalpha(*name) && !isdigit(*name) && !strpbrk(name, "-+"))
			return(-1);
	}

	if(count > 9)
		return(-1);
	else if(!*name)
		return(0);

	count = 0;
	while(*++name) {
		count++;
		if(!isalpha(*name) && !isdigit(*name) && !strpbrk(name, "-+"))
			return(-1);
	}
	if(!count || (count > 4))
		return(-1);

	return(0); /* name is valid */
}
