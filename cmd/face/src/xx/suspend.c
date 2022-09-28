/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)face:src/xx/suspend.c	1.1"

#include <stdio.h>
#include <signal.h>
#include "wish.h"

bool _Debug = FALSE;

main(argc, argv)
int argc;
char *argv[];
{
	if (getenv("DEBUG"))
		_Debug = TRUE;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	if (suspend(argv[1]) == FAIL)
		exit(1);
	else
		exit(0);
}
