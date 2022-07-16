/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/perror.c	1.13"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Print the error indicated
 * in the cerror cell.
 */
#include "synonyms.h"

extern int errno, _sys_num_err, strlen(), write();
extern const char _sys_errs[];
extern const int _sys_index[];

void
perror(s)
char	*s;
{
	register const char *c;
	register int n;

	c = "Unknown error";
	if (errno < _sys_num_err && errno >= 0)
		c = &_sys_errs[_sys_index[errno]];
	if(s && (n = strlen(s))) {
		(void) write(2, s, (unsigned)n);
		(void) write(2, (const char *)": ", 2);
	}
	(void) write(2, c, (unsigned)strlen(c));
	(void) write(2, (const char *)"\n", 1);
}
