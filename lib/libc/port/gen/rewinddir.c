/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/rewinddir.c	1.4"
/*
	rewinddir -- C library extension routine
*/

#ifdef __STDC__
	#pragma weak rewinddir = _rewinddir
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <dirent.h>

#undef rewinddir

extern void seekdir();

void
#ifdef __STDC__
_rewinddir(dirp)
#else
rewinddir(dirp)
#endif
DIR *dirp;
{
	seekdir(dirp, 0L);
}
