/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dbfconv:nlsstr.c	1.7.2.1"

char *erropen = "%s: net_spec %s invalid\n";
char *errclose = "%s: error on close of %s, errno = %d\n";
char *errmalloc = "could not malloc enough memory";
char *errchown = "%s: could not change ownership on %s, errno = %d\n";
char *errscode = "%s: service code %s not found in %s\n";
char *errperm = "must be super user";
char *errdbf = "error in reading database file";
char *errsvc = "bad service code";
char *errarg = "embedded newlines illegal in arguments";
char *errcorrupt = "database file has been corrupted";

int Version = 4;	/* data base version, must match version # below */

char	*init[] = {
"# Listener _pmtab file format:",
"#     svctag:flags:id:res:res:res:addr:rpcinfo:lflags:modules:cmd # comment",
"#       where res respresents a field reserved for future use",
(char *)0};
