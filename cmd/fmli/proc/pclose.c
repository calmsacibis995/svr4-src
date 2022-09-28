/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:proc/pclose.c	1.5"

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>	/* EFT abs k16 */
#include "wish.h"
#include "token.h"
#include "slk.h"
#include "actrec.h"
#include "proc.h"
#include "procdefs.h"
#include "terror.h"

extern struct proc_rec PR_all[];
extern int Vflag;

int
proc_close(rec)
register struct actrec	*rec;
{
	int	i;
	int	id;
	pid_t	pid;		/* EFT abs k16 */
	int	oldsuspend;

	if (Vflag)
		showmail(TRUE);
	id = rec->id;
	pid = PR_all[id].pid;
#ifdef _DEBUG
	_debug(stderr, "closing process table %d, pid=%d\n", id, pid);
#endif
	if (pid != NOPID) {	/* force the user to close by resuming it */
#ifdef _DEBUG
		_debug(stderr, "FORCING CLOSE ON PID %d\n", pid);
#endif
		oldsuspend = suspset(FALSE);	/* disallow suspend */
		PR_all[id].flags |= PR_CLOSING;
		ar_current(rec, TRUE); /* abs k15 */
		suspset(oldsuspend);
	}
	for (i = 0; i < MAX_ARGS && PR_all[id].argv[i]; i++)
		free(PR_all[id].argv[i]);
	PR_all[id].name = NULL;
	PR_all[id].status = ST_DEAD;
	if (rec->path)
		free(rec->path);
	if (rec->odptr)
		free(rec->odptr);
	return SUCCESS;
}
