/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getlogin.c	1.16"
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak getlogin = _getlogin
#endif
#include "synonyms.h"
#include <string.h>
#include <sys/types.h>
#include "utmp.h"
#include <unistd.h>

#define NULL 0

extern long lseek();
extern int open(), read(), close(), ttyslot();


char *
getlogin()
{
	register me, uf;
	struct utmp ubuf ;
	static char answer[sizeof(ubuf.ut_user)+1] ;

	if((me = ttyslot()) < 0)
		return(NULL);
	if((uf = open((const char *)UTMP_FILE, 0)) < 0)
		return(NULL);
	(void) lseek(uf, (long)(me * sizeof(ubuf)), 0);
	if(read(uf, (char*)&ubuf, sizeof(ubuf)) != sizeof(ubuf)) {
		(void) close(uf);
		return(NULL);
	}
	(void) close(uf);
	if(ubuf.ut_user[0] == '\0')
		return(NULL);
	strncpy(&answer[0],&ubuf.ut_user[0],sizeof(ubuf.ut_user)) ;
	answer[sizeof(ubuf.ut_user)] = '\0' ;
	return(&answer[0]);
}
