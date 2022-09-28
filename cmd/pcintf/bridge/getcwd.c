/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/getcwd.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)getcwd.c	3.7	LCC);	/* Modified: 10:04:01 1/30/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	<string.h>
#include	"pci_types.h"
#include	"xdir.h"



/*
 * getcwd() -	parses input string and then stores current working directory.
 */

void
getcwd(ptr, cwd)
register char *ptr;			/* Points to CHDIR input string */
register char *cwd;                     /* Points to place to write the cwd */
{
    register char 
	*component,
	*tptr;

    if ((*ptr == '\0') || (strcmp(ptr, "/") == 0)) {
	strcpy(cwd, "/");
	return;
    }

    if (*ptr == '/')
	strcpy(cwd, "");

/*
 * Parse the input string with '/' as a token and maintain 
 * a string which represents our location within the tree.
 */

    while (component = strtok(ptr, "/")) {
    /* Reset pointer to NULL */
	ptr = 0;

    /* Case 1: append string to cwd */
	if (component[0] != '.') {
	    if (strcmp(cwd, "/") == 0)
		strcat(cwd, component);
	    else {
	        strcat(cwd, "/");
		strcat(cwd, component);
	    }
	/* String must be null terminated! */
	    if (strlen(component) >= (size_t)MAXDIRLEN)
		cwd[strlen(cwd)+1] = '\0';
	}

    /* Case 2: string refers to parent directory */
	else if ((strcmp(component, "..")) == 0) {
	    tptr = strrchr(cwd, '/');
	    *tptr = 0;
	    if (strcmp(cwd, "") == 0)
		strcpy(cwd, "/");
	}
    }
    if (*cwd == '\0')
	strcpy(cwd, "/");
}
