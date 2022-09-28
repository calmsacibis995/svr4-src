/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *      All Rights Reserved
 */

#ident	"@(#)fmli:oh/suffuncs.c	1.2"

#include <stdio.h>
#include <sys/types.h>


int
has_suffix(str,suf)
char *str, *suf;
{
	char *p;
	p = str + strlen(str) - strlen(suf);
	if (strcmp(p, suf) == 0) {
		return(1);
	} else {
		return(0);
	}
}

/*   the rest of this file is  dead code   abs 
char *
rem_suffix(str, num, len)
char *str; int num, len;
{
	static char buf[len +1];

	strcpy(buf, str);
	buf[strlen(str)-num] = '\0';
	return((char *)buf);
}

char *
add_suffix(str, suf, len)
char *str, *suf;
int len
{
	static char buf[len +1];
	char *strcat();

	if (strlen(str) +strlen(suf) > Stray_siz )
		return((char *)NULL);

	return(strcat(strcpy(buf, str), suf));
}

*/
