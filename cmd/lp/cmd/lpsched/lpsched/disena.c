/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/disena.c	1.9.4.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lpsched.h"

extern long		time();

/**
 ** disable() - DISABLE PRINTER
 **/

int
#if	defined(__STDC__)
disable (
	register PSTATUS *	pps,
	char *			reason,
	int			when
)
#else
disable (pps, reason, when)
	register PSTATUS	*pps;
	char			*reason;
	int			when;
#endif
{
	ENTRY ("disable")

	if (pps->status & PS_DISABLED)
		return (-1);

	else {
		pps->status |= PS_DISABLED;
		time (&pps->dis_date);
		load_str (&pps->dis_reason, reason);

		dump_pstatus ();

		if (pps->status & PS_BUSY)
			switch (when) {

			case DISABLE_STOP:
				/*
				 * Stop current job, requeue.
				 */
				if (pps->request)
				    pps->request->request->outcome |= RS_STOPPED;
				terminate (pps->exec);
				break;

			case DISABLE_FINISH:
				/*
				 * Let current job finish.
				 */
				break;

			case DISABLE_CANCEL:
				/*
				 * Cancel current job outright.
				 */
				if (pps->request)
				    cancel (pps->request, 1);
				break;

			}

		/*
		 * Need we check to see if requests assigned to
		 * this printer should be assigned elsewhere?
		 * No, if the "validate()" routine is properly
		 * assigning requests. If another printer is available
		 * for printing requests (that would otherwise be)
		 * assigned to this printer, at least one of those
		 * requests will be assigned to that other printer,
		 * and should be currently printing. Once it is done
		 * printing, the queue will be examined for the next
		 * request, and the one(s) assigned this printer will
		 * be picked up.
		 */
/*		(void)queue_repel (pps, 0, (qchk_fnc_type)0);	*/

		return (0);
	}
}

/**
 ** enable() - ENABLE PRINTER
 **/

int
#if	defined(__STDC__)
enable (
	register PSTATUS *	pps
)
#else
enable (pps)
	register PSTATUS	*pps;
#endif
{
	ENTRY ("enable")

	/*
	 * ``Enabling a printer'' includes clearing a fault and
	 * clearing the do-it-later flag to allow the printer
	 * to start up again.
	 */
	if (!(pps->status & (PS_FAULTED|PS_DISABLED|PS_LATER)))
		return (-1);

	else {
		pps->status &= ~(PS_FAULTED|PS_DISABLED|PS_LATER);
		(void) time (&pps->dis_date);

		dump_pstatus ();

		if (pps->alert->active)
			cancel_alert (A_PRINTER, pps);

		/*
		 * Attract the FIRST request that is waiting to
		 * print to this printer. In this regard we're acting
		 * like the printer just finished printing a request
		 * and is looking for another.
		 */
		queue_attract (pps, qchk_waiting, 1);
		return (0);
	}
}
