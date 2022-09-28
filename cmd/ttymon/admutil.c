/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:admutil.c	1.6.4.1"

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <ctype.h>
# include <sys/stat.h>
# include "tmstruct.h"
# include "ttymon.h"

/*
 *	find_label - return 1 if ttylabel already exists
 *		   - return 0 otherwise
 */

find_label(fp, ttylabel)
FILE *fp;
char *ttylabel;
{
	register char *p;	/* working pointer */
	int line = 0;		/* line number we found entry on */
	static char buf[BUFSIZ];/* scratch buffer */

	while (fgets(buf, BUFSIZ, fp)) {
		line++;
		p = buf;
		while (isspace(*p))
			p++;
		if ((p = strtok(p," :")) != NULL) {
			if (!(strcmp(p, ttylabel)))
				return(line);
		}
	}
	if (!feof(fp)) {
		(void)fprintf(stderr, "error reading \"%s\"\n", TTYDEFS);
		return(0);
	}
	return(0);
}
