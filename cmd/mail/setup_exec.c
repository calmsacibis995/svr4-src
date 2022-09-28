/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:setup_exec.c	1.8.3.1"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <libmail.h>

#define TRUE	1
#define FALSE	0

char **
setup_exec(s)
char	*s;
{
	register char	*p = s, *q;
	static char	*argvec[256]; /* is this enough? */
	register int	i = 0;
	register int	stop;
	register int	ignorespace = FALSE;

	/* Parse up string into arg. vec. for subsequent exec. Assume */
	/* whitespace delimits args. Any non-escaped double quotes will */
	/* be used to group multiple whitespace-delimited tokens into */
	/* a single exec arg. */
	p = skipspace(p);
	while (*p) {
		q = p;
		stop = FALSE;
		while (*q && (stop == FALSE)) {
		    again:
			switch (*q) {
			case '\\':
				/* Slide command string 1 char to left */
				strmove (q, q+1);
				break;
			case '"':
				ignorespace = ((ignorespace == TRUE) ?
								FALSE : TRUE);
				/* Slide command string 1 char to left */
				strmove (q, q+1);
				goto again;
			default:
				if (isspace(*q) && (ignorespace == FALSE)) {
					stop = TRUE;
					continue;
				}
				break;
			}
			q++;
		}
		if (*q == '\0') {
			argvec[i++] = p;
			break;
		}
		*q++ = '\0';
		argvec[i++] = p;
		p = skipspace(q);
	}
	argvec[i] = (char *)NULL;
	if (i == 0) {
		return ((char **)NULL);
	}
	return (argvec);
}
