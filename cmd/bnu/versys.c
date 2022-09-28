/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:versys.c	2.4.3.1"

#include "uucp.h"

extern int getsysline();
extern void sysreset();
/*
 * verify system name
 * input:
 *	name	-> system name
 * returns:  
 *	0	-> success
 *	FAIL	-> failure
 */
int
versys(name)
char *name;
{
	register char *iptr;
	char line[300];

	if (name == 0 || *name == 0)
		return(FAIL);

	if (EQUALS(name, Myname))
		return(0);

	while (getsysline(line, sizeof(line))) {
		if((line[0] == '#') || (line[0] == ' ') || (line[0] == '\t') || 
			(line[0] == '\n'))
			continue;

		if ((iptr=strpbrk(line, " \t")) == NULL)
		    continue;	/* why? */
		*iptr = '\0';
		if (EQUALS(name, line)) {
			sysreset();
			return(0);
		}
	}
	sysreset();
	return(FAIL);
}
