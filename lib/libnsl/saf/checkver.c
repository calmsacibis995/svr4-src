/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libnet:saf/checkver.c	1.1.2.1"

#include <stdio.h>

#define VSTR	"# VERSION="


/*
 * check_version - check to make sure designated file is the correct version
 *		returns : 0 - version correct
 *			  1 - version incorrect
 *			  2 - could not open file
 *			  3 - corrupt file
 */


check_version(ver, fname)
int ver;
char *fname;
{
	FILE *fp;		/* file pointer for sactab */
	char line[BUFSIZ];	/* temp buffer for input */
	char *p;		/* working pointer */
	int version;		/* version number from sactab */

	if ((fp = fopen(fname, "r")) == NULL)
		return(2);
	p = line;
	while (fgets(p, BUFSIZ, fp)) {
		if (!strncmp(p, VSTR, strlen(VSTR))) {
			p += strlen(VSTR);
			if (*p)
				version = atoi(p);
			else {
				return(3);
			}
			(void) fclose(fp);
			return((version != ver) ? 1 : 0);
		}
		p = line;
	}
	return(3);
}
