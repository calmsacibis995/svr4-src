/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/checkchild.c	1.4.3.1"

#include	"lpsched.h"

/**
 ** ev_checkchild() - CHECK FOR DECEASED CHILDREN
 **/

#if	defined(CHECK_CHILDREN)

void
#if	defined(__STDC__)
ev_checkchild (
	void
)
#else
ev_checkchild ()
#endif
{
	ENTRY ("ev_checkchild")

	register EXEC		*ep	= &Exec_Table[0],
				*epend	= &Exec_Table[ET_Size];


	/*
	 * This routine is necessary to find out about child
	 * processes that disappear without a trace. An example
	 * of how they might disappear: kill -9 pid.
	 * To minimize a race condition with a dying child,
	 * we don't mark the child as gone unless it hasn't been
	 * seen for two cycles.
	 */
	for ( ; ep < epend; ep++)
		if (ep->pid > 0 && kill(ep->pid, 0) == -1)
			if (ep->flags & EXF_GONE) {
				ep->pid = -99;
				ep->status = SIGTERM;
				ep->errno = 0;
				DoneChildren++;
#if	defined(DEBUG)
				if (debug & (DB_EXEC|DB_DONE)) {
					execlog (
						"LOST! slot %d pid %d\n",
						ep - Exec_Table,
						ep->pid
					);
					execlog ("%e", ep);
				}
#endif
			} else
				ep->flags |= EXF_GONE;

	schedule (EV_LATER, WHEN_CHECKCHILD, EV_CHECKCHILD);
	return;
}

#endif
