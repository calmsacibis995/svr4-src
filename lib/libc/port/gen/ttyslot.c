/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/ttyslot.c	1.16"
/*LINTLIBRARY*/
/*
 * Return the number of the slot in the utmp file
 * corresponding to the current user: try for file 0, 1, 2.
 * Returns -1 if slot not found.
 */
#if !defined(ABI) && !defined(DSHLIB)
#ifdef __STDC__
	#pragma weak ttyslot = _ttyslot
#endif
#endif
#include "synonyms.h"
#include <string.h>
#include <sys/types.h>
#include "utmp.h"
#define	NULL	0

extern char *ttyname(), *strrchr();
extern int strncmp(), open(), read(), close();


int
ttyslot()
{
	struct utmp ubuf;
	register char *tp, *p;
	register int s, fd;

	if((tp=ttyname(0)) == NULL && (tp=ttyname(1)) == NULL &&
					(tp=ttyname(2)) == NULL)
		return(-1);

	p = tp;
	if (strncmp(tp, "/dev/", 5) == 0)
		p += 5;

	if((fd=open((const char *)UTMP_FILE, 0)) < 0)
		return(-1);
	s = 0;
	while(read(fd, (char*)&ubuf, sizeof(ubuf)) == sizeof(ubuf)) {
		if(    (ubuf.ut_type == INIT_PROCESS ||
			ubuf.ut_type == LOGIN_PROCESS ||
			ubuf.ut_type == USER_PROCESS ||
			ubuf.ut_type == DEAD_PROCESS ) &&
			strncmp(p, ubuf.ut_line, sizeof(ubuf.ut_line)) == 0){

			(void) close(fd);
			return(s);
		}
		s++;
	}
	(void) close(fd);
	return(-1);
}
