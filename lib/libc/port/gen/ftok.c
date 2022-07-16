/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/ftok.c	1.4.1.5"
#ifdef __STDC__
	#pragma weak ftok = _ftok
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mkdev.h>


extern int stat();

key_t
ftok(path, id)
char *path;
char id;
{
	struct stat st;

	return(stat(path, &st) < 0 ? (key_t)-1 :
	    (key_t)((key_t)id << 24 | ((long)(unsigned)minor(st.st_dev)&0x0fff) << 12 |
		((unsigned)st.st_ino&0x0fff)));
}
