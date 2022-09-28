/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:critical.c	1.3"

/* #define		DEBUG		1	/* */

#include	<signal.h>

#include	<stdio.h>

void
critical(on)
int	on;
{
	(void) signal(SIGHUP, on ? SIG_IGN : SIG_DFL);
	(void) signal(SIGINT, on ? SIG_IGN : SIG_DFL);
	(void) signal(SIGQUIT, on ? SIG_IGN : SIG_DFL);

#ifdef DEBUG
	(void) fprintf(stderr, "critical(): DEBUG - Critical code is %s\n", on ? "ON" : "OFF");
#endif
}
