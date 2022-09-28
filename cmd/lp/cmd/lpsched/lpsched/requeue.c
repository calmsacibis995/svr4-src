/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpsched/requeue.c	1.6.4.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lpsched.h"
#include "validate.h"

/*
 * The routines in this file are used to examine queued requests
 * to see if something must be done about them. We don't bother
 * checking requests that are:
 *
 *	- printing (we could, to allow the administrator to stop
 *	  a request by making a configuration change, but that
 *	  can lead to trouble (yet another way to terminate a child)
 *	  and the administrator can always disable the request to
 *	  force it to stop printing and be reevaluated);
 *
 *	- changing, since once the change is complete the request
 *	  will be reevaluated again anyway;
 *
 *	- notifying, since the request is essentially finished
 *
 *	- being sent or already sent to a remote machine;
 *
 *	- done.
 *
 * Requests that are being held or are filtering ARE to be considered,
 * because things may have changed to make them impossible to print.
 */
#define RS_SKIP	((RS_ACTIVE & ~RS_FILTERING) | RS_GONEREMOTE | RS_DONE)
#define	SKIP_IT(PRS) ((PRS)->request->outcome & RS_SKIP)

/**
 ** queue_attract() - REASSIGN REQUEST(S) TO PRINTER, IF POSSIBLE
 **/

void
#if	defined(__STDC__)
queue_attract (
	PSTATUS *		pps,
	int			(*qchk_p)( RSTATUS * ),
	int			attract_just_one
)
#else
queue_attract (pps, qchk_p, attract_just_one)
	register PSTATUS	*pps;
	register int		(*qchk_p)();
	int			attract_just_one;
#endif
{
	ENTRY ("queue_attract")

	register RSTATUS	*prs;

	register CSTATUS	*pcs;

#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
	register int		inclass;
#endif

	int			called_schedule	= 0;


	/*
	 * Evaluate requests that:
	 *	- meet a criteria set by a function passed.
	 *	- are already queued for the printer
	 *	- are destined for a class containing this printer
	 *	- or are destined for any printer
	 * We stop on the first one that will work on the printer,
	 * and schedule an interface for the printer (which will
	 * find the first request ready, namely the one we stopped on).
	 */

#define	SAMECLASS(PRS,PPS) \
	( \
		(pcs = search_ctable(PRS->request->destination)) \
	     && searchlist(PPS->printer->name, pcs->class->members) \
	)

#define ISANY(PRS)	STREQU(PRS->request->destination, NAME_ANY)

	for (prs = Request_List; prs; prs = prs->next) {
#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
		inclass = 0;
#endif
		if (
			!SKIP_IT(prs)
		     && (!qchk_p || (*qchk_p)(prs))
		     && (
				prs->printer == pps
			     || ISANY(prs)
#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
			     || (inclass = SAMECLASS(prs, pps))
#else
			     || SAMECLASS(prs, pps)
#endif
			)
		)
			/*
			 * Don't need to evaluate the request if it
			 * is already queued!
			 */
			if (
				prs->printer == pps
#if	defined(CLASS_ACCEPT_PRINTERS_REJECT_SOWHAT)
			     || evaluate_request(prs, pps, inclass) == MOK
#else
			     || evaluate_request(prs, pps, 0) == MOK
#endif
			) {
				/*
				 * This request was attracted to the
				 * printer but maybe it now needs to be
				 * filtered. If so, filter it but see if
				 * there's another request all set to go.
				 */
				if (NEEDS_FILTERING(prs))
					schedule (EV_SLOWF, prs);
				else {
					if (!called_schedule) {
						schedule (EV_INTERF, pps);
						called_schedule = 1;
					}
					if (attract_just_one)
						break;
				}
			}
	}

	return;
}

/**
 ** queue_repel() - REASSIGN REQUESTS TO ANOTHER PRINTER, IF POSSIBLE
 **/

int
#if	defined(__STDC__)
queue_repel (
	PSTATUS *		pps,
	int			move_off,
	int			(*qchk_p)( RSTATUS * )
)
#else
queue_repel (pps, move_off, qchk_p)
	register PSTATUS	*pps;
	register int		move_off,
				(*qchk_p)();
#endif
{
	ENTRY ("queue_repel")

	register RSTATUS	*prs;

	register int		all_can		= 1;

	register PSTATUS	*stop_pps	= (move_off? pps : 0);


	/*
	 * Reevaluate all requests that are assigned to this
	 * printer, to see if there's another printer that
	 * can handle them.
	 *
	 * If the "move_off" flag is set, don't consider the current
	 * printer when reevaluating, but also don't cancel the request
	 * if it can't be moved off the printer.
	 * (Currently this is only used when deciding if a printer
	 * can be deleted.)
	 */
	BEGIN_WALK_BY_PRINTER_LOOP (prs, pps)

		/*
		 * "all_can" keeps track of whether all of the requests
		 * of interest to the caller (governed by "qchk_p") can
		 * be moved to another printer. Now we don't move certain
		 * requests (active, done, gone remote), and some of those
		 * matter in the ``all can'' consideration.
		 */
		if (qchk_p && !(*qchk_p)(prs))
			continue;
		else if (SKIP_IT(prs)) {
			if ( !(prs->request->outcome & RS_DONE) )
				all_can = 0;
			continue;

		} else

			if (reevaluate_request(prs, stop_pps) == MOK) {

				/*
				 * If this request needs to be filtered,
				 * try to schedule it for filtering,
				 * otherwise schedule it for printing.
				 * We are inefficient here, because we may
				 * try to schedule many requests but the
				 * filtering slot(s) and printers are
				 * busy; but the requests may languish
				 * if we don't check here.
				 */
				if (NEEDS_FILTERING(prs))
					schedule (EV_SLOWF, prs);
				else
					schedule (EV_INTERF, prs->printer);

			} else {
				all_can = 0;
				if (!move_off)
					cancel (prs, 1);
				else
					prs->reason = MOK;
			}
	END_WALK_LOOP

	return (all_can);
}

/**
 ** queue_check() - CHECK ALL REQUESTS AGAIN
 **/

void
#if	defined(__STDC__)
queue_check (
	int			(*qchk_p)( RSTATUS * )
)
#else
queue_check (qchk_p)
	register int		(*qchk_p)();
#endif
{
	ENTRY ("queue_check")

	register RSTATUS	*prs;


	for (prs = Request_List; prs; prs = prs->next)
		if (!SKIP_IT(prs) && (!qchk_p || (*qchk_p)(prs)))
			if (reevaluate_request(prs, (PSTATUS *)0) == MOK)
				if (NEEDS_FILTERING(prs))
					schedule (EV_SLOWF, prs);
				else
					schedule (EV_INTERF, prs->printer);
			else
				cancel (prs, 1);

	return;
}

/**
 ** qchk_waiting() - CHECK IF REQUEST IS READY TO PRINT
 ** qchk_filter() - CHECK IF REQUEST NEEDS A FILTER
 ** qchk_form() - CHECK IF REQUEST NEEDS A FORM
 ** qchk_pwheel() - CHECK IF REQUEST NEEDS PRINT A WHEEL
 **/

int
#if	defined(__STDC__)
qchk_waiting (
	RSTATUS *			prs
)
#else
qchk_waiting (prs)
	RSTATUS				*prs;
#endif
{
	ENTRY ("qchk_waiting")

	return (
		!(prs->request->outcome & (RS_HELD|RS_DONE|RS_ACTIVE|RS_GONEREMOTE))
	     && !NEEDS_FILTERING(prs)
	);
}

int
#if	defined(__STDC__)
qchk_filter (
	RSTATUS *		prs
)
#else
qchk_filter (prs)
	register RSTATUS	*prs;
#endif
{
	ENTRY ("qchk_filter")

	/*
	 * No need to reevaluate this request if it isn't using a filter
	 * or if it is done or is being changed.
	 */
	return (
		!(prs->request->outcome & (RS_DONE|RS_CHANGING|RS_NOTIFY))
	     && (prs->slow || prs->fast)
	);
}

FSTATUS *		form_in_question;

int
#if	defined(__STDC__)
qchk_form (
	RSTATUS *		prs
)
#else
qchk_form (prs)
	register RSTATUS	*prs;
#endif
{
	ENTRY ("qchk_form")

	return (prs->form == form_in_question);
}

char *			pwheel_in_question;

int
#if	defined(__STDC__)
qchk_pwheel (
	RSTATUS *		prs
)
#else
qchk_pwheel (prs)
	register RSTATUS	*prs;
#endif
{
	ENTRY ("qchk_pwheel")

	return (SAME(prs->pwheel_name, pwheel_in_question));
}
