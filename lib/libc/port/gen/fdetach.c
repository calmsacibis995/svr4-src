/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/fdetach.c	1.2"
/*
 * Detach a STREAMS-based file descriptor from an object in the
 * file system name space.
 */

#ifdef __STDC__
	#pragma weak fdetach = _fdetach
#endif
#include "synonyms.h"

int
fdetach(path)
	char *path;
{
	extern int umount();

	return (umount(path));
}
