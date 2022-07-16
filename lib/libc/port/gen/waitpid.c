/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/waitpid.c	1.2"

#ifdef __STDC__
	#pragma weak waitpid = _waitpid
#endif
#include "synonyms.h"
#include <wait.h>

pid_t
waitpid(pid,stat_loc,options)
pid_t pid;
int *stat_loc;
int options;
{
	idtype_t idtype;
	id_t id;
	siginfo_t info;
	int error;

	if (pid > 0) {
		idtype = P_PID;
		id = pid;
	} else if (pid < -1) {
		idtype = P_PGID;
		id = -pid;
	} else if (pid == -1) {
		idtype = P_ALL;
		id = 0;
	} else {
		idtype = P_PGID;
		id = getpgid(0);
	}

	options |= (WEXITED|WTRAPPED);

	if ((error = waitid(idtype, id, &info, options)) < 0)
		return error;

	if (stat_loc) {

		register stat = (info.si_status & 0377);

		switch (info.si_code) {
		case CLD_EXITED:
			stat <<= 8;
			break;
		case CLD_DUMPED:
			stat |= WCOREFLG;
			break;
		case CLD_KILLED:
			break;
		case CLD_TRAPPED:
		case CLD_STOPPED:
			stat <<= 8;
			stat |= WSTOPFLG;
			break;
		case CLD_CONTINUED:
			stat = WCONTFLG;
			break;
		}

		*stat_loc = stat;
	}

	return info.si_pid;
}
