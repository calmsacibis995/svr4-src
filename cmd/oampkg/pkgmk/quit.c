/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkgmk/quit.c	1.1.4.1"
#include <stdio.h>
#include <signal.h>
#include <pkgdev.h>

extern struct pkgdev pkgdev;
extern char	pkgloc[],
		*prog,
		*t_pkgmap,
		*t_pkginfo;
extern int	started;

extern int	unlink(),
		rrmdir(),
		pkgumount();
extern void	exit();

#define MSG_COMPLETE	"## Packaging complete.\n"
#define MSG_TERM	"## Packaging terminated at user request.\n"
#define MSG_ERROR	"## Packaging was not successful.\n"

void
quit(retcode) 
int retcode;
{
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);

	if(retcode == 3)
		(void) fprintf(stderr, MSG_TERM);
	else if(retcode)
		(void) fprintf(stderr, MSG_ERROR);
	else
		(void) fprintf(stderr, MSG_COMPLETE);

	if(retcode && started)
		(void) rrmdir(pkgloc); /* clean up output directory */

	if(pkgdev.mount)
		(void) pkgumount(&pkgdev);

	if(t_pkgmap)
		(void) unlink(t_pkgmap);
	if(t_pkginfo)
		(void) unlink(t_pkginfo);
	exit(retcode);
}
