/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:ns_error.c	1.2.3.1"

/*	@(#)nserror.c	*/

extern int ns_errno, ns_err, strlen(), write();
extern char *ns_errlist[];

void
nserror(s)
char	*s;
{
	register char *c;
	register int n;

	c = "Unknown error";
	if(ns_errno < ns_err)
		c = ns_errlist[ns_errno];
	n = strlen(s);
	if(n) {
		(void) write(2, s, (unsigned)n);
		(void) write(2, ": ", 2);
	}
	(void) write(2, c, (unsigned)strlen(c));
	(void) write(2, "\n", 1);
}
