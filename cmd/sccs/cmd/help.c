/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:cmd/help.c	1.5"
#include	<stdio.h>
#include	<sys/types.h>
#include	<macros.h>


char	Ohelpcmd[]   =   "/usr/ccs/lib/help/lib/help2";
extern	int	errno;

main(argc,argv)
int argc;
char *argv[];
{
	execv(Ohelpcmd,argv);
	fprintf(stderr,"help: Could not exec: %s.  Errno=%d\n",Ohelpcmd,errno);
	exit(1);
}
