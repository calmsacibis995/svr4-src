/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:createmf.c	1.5.3.1"
#include "mail.h"
/*
	If mail file does not exist create it with the correct uid 
	and gid
*/
void createmf(uid, file)
uid_t uid;
char *file;
{
	void (*istat)(), (*qstat)(), (*hstat)();

	if (access(file, A_EXIST) == CERROR) {
		istat = signal(SIGINT, SIG_IGN);
		qstat = signal(SIGQUIT, SIG_IGN);
		hstat = signal(SIGHUP, SIG_IGN);
		umask(0);
		close(creat(file, MFMODE));
		umask(7);
		chown(file, uid, my_egid);
		(void) signal(SIGINT, istat);
		(void) signal(SIGQUIT, qstat);
		(void) signal(SIGHUP, hstat);
	}
}
