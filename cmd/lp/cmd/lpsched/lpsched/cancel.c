/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/cancel.c	1.2.4.1"

#include "lpsched.h"

/**
 ** cancel() - CANCEL A REQUEST
 **/

int
#if	defined(__STDC__)
cancel (
	register RSTATUS *	prs,
	int			spool
)
#else
cancel (prs, spool)
	register RSTATUS	*prs;
	int			spool;
#endif
{
	ENTRY ("cancel")

	if (prs->request->outcome & RS_DONE)
		return (0);

	prs->request->outcome |= RS_CANCELLED;

	if (spool || (prs->request->actions & ACT_NOTIFY))
		prs->request->outcome |= RS_NOTIFY;

	/*
	 * If the printer for this request is on a remote system,
	 * send a cancellation note across. HOWEVER, this isn't
	 * necessary if the request hasn't yet been sent!
	 */
	if (
		prs->printer->status & PS_REMOTE
	     && prs->request->outcome & (RS_SENT | RS_SENDING)
	) {
		/*
		 * Mark this request as needing sending, then
		 * schedule the send in case the connection to
		 * the remote system is idle.
		 */
		prs->status |= RSS_SENDREMOTE;
		schedule (EV_SYSTEM, prs->printer->system);

	} else if (prs->request->outcome & RS_PRINTING)
		terminate (prs->printer->exec);
	else if (prs->request->outcome & RS_FILTERING)
		terminate (prs->exec);
	else if (spool || (prs->request->actions & ACT_NOTIFY))
		notify (prs, (char *)0, 0, 0, 0);
	else
		check_request (prs);

	return (1);
}
