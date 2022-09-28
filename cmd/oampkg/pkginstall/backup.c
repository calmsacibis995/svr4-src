/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginstall/backup.c	1.3.2.1"
#include <stdio.h>

extern char	savlog[];
extern int	warnflag;

extern void	logerr();

void
backup(path, mode)
char	*path;
int	mode;
{
	static int	count = 0;
	static FILE	*fp;

	/* mode probably used in the future */
	if(count++ == 0) {
		if((fp = fopen(savlog, "w")) == NULL) {
			logerr("WARNING: unable to open logfile <%s>", savlog);
			warnflag++;
		}
	}

	if(fp == NULL)
		return;

	(void) fprintf(fp, "%s%s", path, mode ? "\n" : " <attributes only>\n");
	/* we don't really back anything up; we just log the pathname */
}
