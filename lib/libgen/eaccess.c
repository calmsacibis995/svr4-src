/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:eaccess.c	2.2.4.3"
/*	Determine if the effective user id has the appropriate permission
	on a file.  
*/

#ifdef __STDC__
	#pragma weak eaccess = _eaccess
#endif
#include "synonyms.h"

extern int	access();


int
eaccess( path, amode )
const char		*path;
register int	amode;
{
	/* Use effective id bits */
	return access(path, 010|amode); 
}
