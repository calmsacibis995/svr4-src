/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:rmdirp.c	1.5.3.2"

#ifdef __STDC__
	#pragma weak rmdirp = _rmdirp
#endif
#include "synonyms.h"

/* Rmdirp() removes directories in path "d". Removal starts from the
** right most directory in the path and goes backward as far as possible.
** The remaining path, which is not removed for some reason, is stored in "d1".
** If nothing remains, "d1" is empty.

** Rmdirp()
** returns 0 only if it succeeds in removing every directory in d.
** returns -1 if removal stops because of errors other than the following.
** returns -2 if removal stops when "." or ".." is encountered in path.
** returns -3 if removal stops because it's the current directory.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

char *strcpy(), *strrchr();
static int dotdot();

int
rmdirp(d, d1)
char *d, *d1;
{
	struct stat st, cst;
	int currstat;
	char *slash;
	int	stat();
	int	rmdir();
	
	slash = strrchr(d, '/');
	currstat = stat(".", &cst);

				    /* Starts from right most element */

	while (d) {			

					/* If it's under current directory */

		if (slash == NULL) {

						/* Stop if it's . or .. */

			if (dotdot(d)) {
				(void)strcpy(d1, d);
				return(-2);
			}

						/* Stop if can not stat it */

		}

					/* If there's a slash before it */

		else {

						/* If extra / in the end */

			if (slash != d) 
				if (++slash == strrchr(d,'\0')) {
					*(--slash) = '\0';
					slash = strrchr(d, '/');
					continue;
				}
				else
					slash--;

						/* Stop if it's . or .. */

			if (dotdot(++slash)) {
				(void)strcpy(d1, d);
				return(-2);
			}
			slash--;

						/* Stop if can not stat it */

			if (stat(d, &st) < 0) {
				(void)strcpy(d1, d);
				return(-1);
			}
			if (currstat == 0) {	

						/* Stop if it's current directory */

				if(st.st_ino==cst.st_ino && st.st_dev==cst.st_dev) {
					(void)strcpy(d1, d);
					return(-3);
				}
			}
		} /* End of else */


					/* Remove it */

		if (rmdir(d) != 0) {
			(void)strcpy(d1, d);
			return(-1);
		}

					/* Have reached left end, break*/

		if (slash == NULL || slash == d)
			break;

					/* Go backward to next directory */

		*slash = '\0';
		slash = strrchr(d, '/');
	}
	*d1 = '\0';
	return(0);
}


static int
dotdot(dir)
char *dir;
{
	if (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0)
		return(-1);
	return(0);
}
