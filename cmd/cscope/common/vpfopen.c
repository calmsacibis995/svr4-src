/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/vpfopen.c	1.1"
/* vpfopen - view path version of the fopen library function */

#include <stdio.h>
#include "vp.h"

FILE *
vpfopen(filename, type)
char	*filename, *type;
{
	char	buf[MAXPATH + 1];
	FILE	*returncode;
	int	i;

	if ((returncode = fopen(filename, type)) == NULL && filename[0] != '/' &&
	    strcmp(type, "r") == 0) {
		vpinit((char *) 0);
		for (i = 1; i < vpndirs; i++) {
			(void) sprintf(buf, "%s/%s", vpdirs[i], filename);
			if ((returncode = fopen(buf, type)) != NULL) {
				break;
			}

		}
	}
	return(returncode);
}
