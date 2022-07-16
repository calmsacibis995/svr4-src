/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/system.c	1.21"
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
#include "synonyms.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/utsname.h>
#undef uname

#define BIN_SH "/sbin/sh"

int
system(s)
const char *s;
{
	int	status, pid, w;
	void (*istat)(), (*qstat)(), (*cstat)();
	struct stat buf;
	static int vers;
	struct utsname uname_buf;

	if (s == NULL) {
		if (stat(BIN_SH, &buf) != 0) {
			return(0);
		} else if (getuid() == buf.st_uid) {
			if ((buf.st_mode & 0100) == 0)
				return(0);
		} else if (getgid() == buf.st_gid) {
			if ((buf.st_mode & 0010) == 0)
				return(0);
		} else if ((buf.st_mode & 0001) == 0) {
			return(0);
		}
		return(1);
	}
		
	if((pid = fork()) == 0) {
		(void) execl(BIN_SH, "sh", (const char *)"-c", s, (char *)0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	cstat = signal(SIGCLD, SIG_DFL);

	if (vers == 0) {
		if (uname(&uname_buf) > 0)
			vers = 2;	/* SVR4 system */
		else
			vers = 1;	/* non-SVR4 system */
	}

	if ( vers == 1 ) {
		while((w = wait(&status)) != pid && w != -1)
			;
	} else
		w = waitpid(pid, &status, 0);

	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGCLD, cstat);
	return((w == -1)? w: status);
}
