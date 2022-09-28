/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/sys_calls.c	1.2.2.2"

/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "intf.h"

extern int	mkdir();

int
move_tmp(from, to)
char *from;		/* name of file to move */
char *to;		/* file name to move to */
{
	char	cmd[PATH_MAX*2+5];
	/* issue system(3) call to move 'from' to 'to' */

	(void) sprintf(cmd, "mv %s %s", from, to);
	if(system(cmd) < 0) {
		(void) printf("Error:  command failed <%s>\n", cmd);
		return(-1);
	}
	return(0);
}

int
mk_dir(dir, pkginst, logfile)
char *dir;	/* directory name to make */
char *pkginst;	/* package instance */
FILE *logfile;	/* file for logging */
{
	char	cmd[PATH_MAX+128];

	(void) fprintf(stderr, "%s\n", dir); /* log to terminal */
	(void) sprintf(cmd, "installf %s %s %s", pkginst, dir, DIRINFO);
	if(system(cmd) < 0) {
		(void) printf("ERROR:  command failed <%s>\n", cmd);
		return(-1);
	} else
		(void) fprintf(logfile, "NEWDIR:  %s\n", dir);
	return(0);
}

int
mk_tdir(dir, logfile)
char *dir;	/* directory name to make */ 
FILE *logfile;	/* file for logging */
{
	(void) fprintf(stderr, "%s\n", dir);
	if(mkdir(dir)) {
		(void) printf("Error:  failed to make directory '%s'\n", dir);
		return(-1);
	} else /* log fact that new directory was created */
		(void) fprintf(logfile, "NEWDIR:  %s\n", dir);
	return(0);
}
