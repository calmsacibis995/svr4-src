/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rmount:wr_rmnttab.c	1.1.5.1"


/*
	wr_mnttab - write the mount table
		write successive struct mnttab entries from "rp"
		up to (not including) "last" where the mt_dev field
		is a non-empty string.
	return:
		1 on success, 0 on failure of write
*/

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

#define RMNTTAB	"/etc/rfs/rmnttab"

extern char *cmd;

wr_rmnttab(res, dir, opt, fs)
char *res, *dir, *opt, *fs;
{
	int  ret = 1;
	FILE *rfp;
	long ltime;

	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	
	if ((rfp=fopen (RMNTTAB, "a")) == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", cmd, RMNTTAB);
		return 0;
	}
	ltime = time((long *)0);
	fprintf(rfp, "%s\t%s\t%s\t%s\t%d\n",res, dir, fs, opt, ltime);
	fclose(rfp);

	signal(SIGHUP,  SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT,  SIG_DFL);
	
	return ret;
}
