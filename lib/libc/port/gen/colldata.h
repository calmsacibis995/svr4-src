/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/colldata.h	1.1"

/* structures common to colldata.c and strxfrm.c */

/* collation table entry */
typedef struct xnd {
	unsigned char	ch;	/* character or number of followers */
	unsigned char	pwt;	/* primary weight */
	unsigned char	swt;	/* secondary weight */
	unsigned char	ns;	/* index of follower state list */
} xnd;

/* substitution table entry */
typedef struct subnd {
	char	*exp;	/* expression to be replaced */
	long	explen; /* length of expression */
	char	*repl;	/* replacement string */
} subnd;

