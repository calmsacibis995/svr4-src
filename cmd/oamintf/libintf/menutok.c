/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/menutok.c	1.1.2.2"
/*LINTLIBRARY*/
#include <string.h>
#include <ctype.h>
#include "intf.h"

char *
menutok(str)
char	*str;
{
	static char *save = NULL;
	char	*head;

	if(str == NULL)
		str = save;

	/* eat leading white space */
	while(isspace(*str))
		str++;

	head = str;

	/* find ending token, if any */
	while((*str != NULL) && (*str != '\n') && (*str != TAB_CHAR))
		str++;

	if(*str != NULL) {
		*str = NULL;
		save = str+1;
	} else
		save = str;
	return((head == str) ? NULL : head);
}

