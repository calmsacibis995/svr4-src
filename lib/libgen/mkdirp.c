/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:mkdirp.c	1.4.5.2"

#ifdef __STDC__
	#pragma weak mkdirp = _mkdirp
#endif
#include "synonyms.h"

/* Creates directory and it's parents if the parents do not
** exist yet.
**
** Returns -1 if fails for reasons other than non-existing
** parents.
** Does NOT compress pathnames with . or .. in them.
*/
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static char *compress();
VOID *malloc();
void free();
extern int	access();
extern int	mkdir();

int
mkdirp(d, mode)
const char *d;
mode_t mode;
{
	char  *endptr, *ptr, *slash, *str;

	str=compress(d);

	/* If space couldn't be allocated for the compressed names, return. */

	if ( str == NULL )
		return(-1);
	ptr = str;


		/* Try to make the directory */

	if (mkdir(str, mode) == 0){
		free(str);
		return(0);
	}
	if (errno != ENOENT) {
		free(str);
		return(-1);
	}
	endptr = strrchr(str, '\0');
	ptr = endptr;
	slash = strrchr(str, '/');

		/* Search upward for the non-existing parent */

	while (slash != NULL) {

		ptr = slash;
		*ptr = '\0';

			/* If reached an existing parent, break */

		if (access(str, 00) ==0)
			break;

			/* If non-existing parent*/

		else {
			slash = strrchr(str,'/');

				/* If under / or current directory, make it. */

			if (slash  == NULL || slash== str) {
				if (mkdir(str, mode)) {
					free(str);
					return(-1);
				}
				break;
			}
		}
	}
		/* Create directories starting from upmost non-existing parent*/

	while ((ptr = strchr(str, '\0')) != endptr){
		*ptr = '/';
		if (mkdir(str, mode)) {
			free(str);
			return(-1);
		}
	}
	free(str);
	return(0);
}

static char *
compress(str)
char *str;
{

	char *tmp;
	char *front;

	tmp=(char *)malloc(strlen(str)+1);
	if ( tmp == NULL )
		return(NULL);
	front = tmp;
	while ( *str != '\0' ) {
		if ( *str == '/' ) {
			*tmp++ = *str++;
			while ( *str == '/' )
				str++;
		}
		*tmp++ = *str++;
	}
	*tmp = '\0';
	return(front);
}
