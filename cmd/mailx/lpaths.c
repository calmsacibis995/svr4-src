/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:lpaths.c	1.4.5.1"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 */

/*
 *	libpath(file) - return the full path to the library file
 *	If POST is defined in the environment, use that.
 */

#include <stdio.h>
#include "uparm.h"
#ifndef preSVr4
# include <stdlib.h>
#else
extern char	*getenv();
#endif

char *
libpath(file)
char	*file;
{
	static char	buf[500];
	sprintf(buf, "%s/%s", LIBPATH, file);
	return buf;
}
