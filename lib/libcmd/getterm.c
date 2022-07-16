/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libcmd:getterm.c	1.1.1.1"

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


#include <stdio.h>
#include <string.h>
#include <limits.h>

/*
 * make a reasonable guess as to the kind of terminal the user is on.
 * We look in /etc/ttytype for this info (format: each line has two
 * words, first word is a term type, second is a tty name), and default
 * to deftype if we can't find any better.  In the case of dialups we get
 * names like "dialup" which is a lousy guess but tset can
 * take it from there.
 */

/* This value is the maximum string length returned by longname() in curses */
#define MAXTTYTYPE 128  

int
getterm(tname, buffer, filename, deftype)
register char	*tname;
char *buffer, *filename, *deftype;
	{
	register FILE	*fdes;
	char		*type, *t;
	char		ttline[MAXTTYTYPE + PATH_MAX + 1]; 
	int		retval = -1;

	if (filename == NULL)
		filename = "/etc/ttytype";
	if ((fdes = fopen(filename, "r")) != NULL)
		{
		while (fgets(ttline, sizeof(ttline), fdes) != NULL)
			{
			if (((type = strtok(ttline, " \t\n\r")) != NULL) &&
			    ((t = strtok(NULL, " \t\n\r")) != NULL) &&
			    (strcmp(t, tname) == 0))
				{
				strcpy(buffer, type);
				return fclose(fdes);
				}
			}
		retval = fclose(fdes);
		}
	strcpy(buffer, deftype);
	return retval;
	}
