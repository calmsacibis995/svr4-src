/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/daisyforms.c	1.5.4.1"

#include "lpsched.h"

#if	defined(__STDC__)
static int	max_requests_needing_form_mounted ( FSTATUS * );
static int	max_requests_needing_pwheel_mounted ( char * );
#else
static int	max_requests_needing_form_mounted();
static int	max_requests_needing_pwheel_mounted();
#endif

/**
 ** queue_form() - ADD A REQUEST TO A FORM QUEUE
 **/

void
#if	defined(__STDC__)
queue_form (
	register RSTATUS *	prs,
	FSTATUS *		pfs
)
#else
queue_form (prs, pfs)
	register RSTATUS	*prs;
	FSTATUS			*pfs;
#endif
{
	ENTRY ("queue_form")

	if ((prs->form = pfs)) {
		prs->form->requests++;
		check_form_alert (prs->form, (_FORM *)0);
	}
	return;
}

/**
 ** unqueue_form() - REMOVE A REQUEST FROM A FORM QUEUE
 **/

void
#if	defined(__STDC__)
unqueue_form (
	register RSTATUS *	prs
)
#else
unqueue_form (prs)
	register RSTATUS	*prs;
#endif
{
	ENTRY ("unqueue_form")

	FSTATUS *		pfs	= prs->form;

	prs->form = 0;
	if (pfs) {
		pfs->requests--;
		check_form_alert (pfs, (_FORM *)0);
	}
	return;
}

/**
 ** queue_pwheel() - ADD A REQUEST TO A PRINT WHEEL QUEUE
 **/

void
#if	defined(__STDC__)
queue_pwheel (
	register RSTATUS *	prs,
	char *			name
)
#else
queue_pwheel (prs, name)
	register RSTATUS	*prs;
	char			*name;
#endif
{
	ENTRY ("queue_pwheel")

	if (name) {
		prs->pwheel_name = Strdup(name);
		/*
		 * Don't bother queueing the request for
		 * a print wheel if this request is destined for
		 * only this printer and the printer doesn't take
		 * print wheels.
		 */
		if (
			!one_printer_with_charsets(prs)
		     && (prs->pwheel = search_pwtable(name))
		) {
			prs->pwheel->requests++;
			check_pwheel_alert (prs->pwheel, (PWHEEL *)0);
		}
	}
	return;
}

/**
 ** unqueue_pwheel() - REMOVE A REQUEST FROM A PRINT WHEEL QUEUE
 **/

void
#if	defined(__STDC__)
unqueue_pwheel (
	register RSTATUS *	prs
)
#else
unqueue_pwheel (prs)
	register RSTATUS	*prs;
#endif
{
	ENTRY ("unqueue_pwheel")

	PWSTATUS *		ppws	= prs->pwheel;

	prs->pwheel = 0;
	unload_str (&(prs->pwheel_name));
	if (ppws) {
		ppws->requests--;
		check_pwheel_alert (ppws, (PWHEEL *)0);
	}
	return;
}

/**
 ** check_form_alert() - CHECK CHANGES TO MOUNT FORM ALERT
 **/

void
#if	defined(__STDC__)
check_form_alert (
	register FSTATUS *	pfs,
	register _FORM *	pf
)
#else
check_form_alert (pfs, pf)
	register FSTATUS	*pfs;
	register _FORM		*pf;
#endif
{
	ENTRY ("check_form_alert")

	register short		trigger,
				fire_off_alert	= 0;

	int			requests_waiting;


	/*
	 * Call this routine whenever a requests has been queued
	 * or dequeued for a form, and whenever the form changes.
	 * If a pointer to a new _FORM is passed, the FSTATUS
	 * structure is updated with the changes. Use a second
	 * argument of 0 if no change.
	 *
	 * WARNING: It is valid to call this routine when adding
	 * a NEW form (not just changing it). Thus the members of
	 * the structure "pfs->form" may not be set.
	 * In this case, though, "pf" MUST be set, and there can
	 * be NO alert active.
	 */

	if (pf) {
		if ((trigger = pf->alert.Q) <= 0)
			trigger = 1;
	} else
		trigger = pfs->trigger;

	if (Starting)
		goto Return;

#define	OALERT	pfs->form->alert
#define NALERT	pf->alert

	requests_waiting = max_requests_needing_form_mounted(pfs);

	/*
	 * Cancel an active alert if the number of requests queued
	 * has dropped below the threshold (or the threshold has been
	 * raised), or if the alert command or period has changed.
	 * In the latter case we'll reactive the alert later.
	 */
	if (pfs->alert->active)
		if (!requests_waiting || requests_waiting < trigger)
			cancel_alert (A_FORM, pfs);

		else if (
			pf
		     && (
				!SAME(NALERT.shcmd, OALERT.shcmd)
			     || NALERT.W != OALERT.W
			     || NALERT.Q != OALERT.Q
			)
		)
			cancel_alert (A_FORM, pfs);

	/*
	 * If we still have the condition for an alert, we'll fire
	 * one off. It is possible the alert is still running, but
	 * that's okay. First, we may want to change the alert message;
	 * second, the "alert()" routine doesn't execute an alert
	 * if it is already running.
	 */
	if (trigger > 0 && requests_waiting >= trigger)
		if ((pf && NALERT.shcmd) || OALERT.shcmd)
			fire_off_alert = 1;

#undef	OALERT
#undef	NALERT

Return:	if (pf) {
		/*
		 * Watch it! We may be adding a new form, so there
		 * may be nothing to toss out.
		 */
		if (pfs->form->name)
			free_form (pfs->form);

		*(pfs->form) = *pf;
		pfs->trigger = trigger;
	}

	/*
	 * Have to do this after updating the changes.
	 */
	if (fire_off_alert)
		alert (A_FORM, pfs);

	return;
}

/**
 ** check_pwheel_alert() - CHECK CHANGES TO MOUNT PRINTWHEEL ALERT
 **/

void
#if	defined(__STDC__)
check_pwheel_alert (
	register PWSTATUS *	ppws,
	register PWHEEL *	ppw
)
#else
check_pwheel_alert (ppws, ppw)
	register PWSTATUS	*ppws;
	register PWHEEL		*ppw;
#endif
{
	ENTRY ("check_pwheel_alert")

	register short		trigger,
				fire_off_alert	= 0;

	int			requests_waiting;


	/*
	 * Call this routine whenever a request has been queued
	 * or dequeued for a print-wheel, and whenever the print-wheel
	 * changes. If a pointer to a new PWHEEL is passed, the
	 * PWSTATUS structure is updated with the changes. Use a
	 * second argument of 0 if no change.
	 *
	 * WARNING: It is valid to call this routine when adding
	 * a NEW print wheel (not just changing it). Thus the members
	 * of the structure "ppws->pwheel" may not be set.
	 * In this case, though, "ppw" MUST be set, and there can
	 * be NO alert active.
	 */

	if (ppw) {
		if ((trigger = ppw->alert.Q) <= 0)
			trigger = 1;
	} else
		trigger = ppws->trigger;

	if (Starting)
		goto Return;

#define	OALERT	ppws->pwheel->alert
#define NALERT	ppw->alert

	requests_waiting = max_requests_needing_pwheel_mounted(ppws->pwheel->name);

	/*
	 * Cancel an active alert if the number of requests queued
	 * has dropped below the threshold (or the threshold has been
	 * raised), or if the alert command or period has changed.
	 * In the latter case we'll reactive the alert later.
	 */
	if (ppws->alert->active)
		if (!requests_waiting || requests_waiting < trigger)
			cancel_alert (A_PWHEEL, ppws);

		else if (
			ppw
		     && (
				!SAME(NALERT.shcmd, OALERT.shcmd)
			     || NALERT.W != OALERT.W
			     || NALERT.Q != OALERT.Q
			)
		)
			cancel_alert (A_PWHEEL, ppws);

	/*
	 * If we still have the condition for an alert, we'll fire
	 * one off. It is possible the alert is still running, but
	 * that's okay. First, we may want to change the alert message;
	 * second, the "alert()" routine doesn't execute an alert
	 * if it is already running.
	 */
	if (trigger > 0 && requests_waiting >= trigger)
		if ((ppw && NALERT.shcmd) || OALERT.shcmd)
			fire_off_alert = 1;

#undef	OALERT
#undef	NALERT

Return:	if (ppw) {
		/*
		 * Watch it! We may be adding a new print wheel, so there
		 * may be nothing to toss out.
		 */
		if (ppws->pwheel->name)
			freepwheel (ppws->pwheel);

		*(ppws->pwheel) = *ppw;
		ppws->trigger = trigger;
	}

	/*
	 * Have to do this after updating the changes.
	 */
	if (fire_off_alert)
		alert (A_PWHEEL, ppws);

	return;
}

/**
 ** max_requests_needing_form_mounted()
 ** max_requests_needing_pwheel_mounted()
 **/

static int
#if	defined(__STDC__)
max_requests_needing_form_mounted (
	FSTATUS *		pfs
)
#else
max_requests_needing_form_mounted (pfs)
	FSTATUS *		pfs;
#endif
{
	ENTRY ("max_requests_needing_form_mounted")

	PSTATUS *		pps;

	RSTATUS *		prs;

	int			max	= 0;


	/*
	 * For each printer that doesn't have this form mounted,
	 * count the number of requests needing this form and
	 * assigned to the printer. Find the maximum across all such
	 * printers. Sorry, the code actually has a different loop
	 * (it steps through the requests) but the description of what
	 * happens below is easier to understand as given. (Looping
	 * through the printers would result in #printers x #requests
	 * steps, whereas this entails #requests steps.)
	 */
	for (pps = walk_ptable(1); pps; pps = walk_ptable(0))
		pps->nrequests = 0;
	BEGIN_WALK_BY_FORM_LOOP (prs, pfs)
		if ((pps = prs->printer) && pps->form != pfs)
			if (++pps->nrequests >= max)
				max = pps->nrequests;
	END_WALK_LOOP
	if (NewRequest)
		if ((pps = NewRequest->printer) && pps->form != pfs)
			if (++pps->nrequests >= max)
				max = pps->nrequests;
	return (max);
}

static int
#if	defined(__STDC__)
max_requests_needing_pwheel_mounted (
	char *			pwheel_name
)
#else
max_requests_needing_pwheel_mounted (pwheel_name)
	char *			pwheel_name;
#endif
{
	ENTRY ("max_requests_needing_pwheel_mounted")

	PSTATUS *		pps;

	RSTATUS *		prs;

	int			max	= 0;


	/*
	 * For each printer that doesn't have this print-wheel mounted,
	 * count the number of requests needing this print-wheel and
	 * assigned to the printer. Find the maximum across all such
	 * printers. Sorry, the code actually has a different loop
	 * (it steps through the requests) but the description of what
	 * happens below is easier to understand as given. (Looping
	 * through the printers would result in #printers x #requests
	 * steps, whereas this entails #requests steps.)
	 */
	for (pps = walk_ptable(1); pps; pps = walk_ptable(0))
		pps->nrequests = 0;
	BEGIN_WALK_BY_PWHEEL_LOOP (prs, pwheel_name)
		if (
			(pps = prs->printer)
		     && pps->printer->daisy
		     && !SAME(pps->pwheel_name, pwheel_name)
		)
			if (++pps->nrequests >= max)
				max = pps->nrequests;
	END_WALK_LOOP
	if (NewRequest)
		if (
			(pps = NewRequest->printer)
		     && pps->printer->daisy
		     && !SAME(pps->pwheel_name, pwheel_name)
		)
			if (++pps->nrequests >= max)
				max = pps->nrequests;
	return (max);
}

/**
 ** one_printer_with_charsets() 
 **/

int
#if	defined(__STDC__)
one_printer_with_charsets (
	register RSTATUS *	prs
)
#else
one_printer_with_charsets (prs)
	register RSTATUS	*prs;
#endif
{
	ENTRY ("one_printer_with_charsets")

	/*
	 * This little function answers the question: Is a request
	 * that needs a character set destined for a particular
	 * printer that has selectable character sets instead of
	 * mountable print wheels?
	 */
	return (
	    STREQU(prs->request->destination, prs->printer->printer->name)
	 && !prs->printer->printer->daisy
	);
}
