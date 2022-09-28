/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:cmd/lpstat/request.c	1.11.3.1"

#include "stdio.h"
#include "pwd.h"
#include "sys/types.h"

#include "lp.h"
#include "msgs.h"
#include "requests.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


/**
 ** do_request()
 **/

void
#if	defined(__STDC__)
do_request (
	char **			list
)
#else
do_request (list)
	char			**list;
#endif
{
	while (*list) {
		if (STREQU(NAME_ALL, *list)) {
			if (verbosity & V_RANK)
			{
				send_message (S_INQUIRE_REQUEST_RANK, 1,
					      "", "", "", "", "");
				(void)output (R_INQUIRE_REQUEST_RANK);
			}
			else
			{
				send_message (S_INQUIRE_REQUEST,
					      "", "", "", "", "");
				(void)output (R_INQUIRE_REQUEST);
			}

		} else if (isrequest(*list)) {
			if (verbosity & V_RANK)
			{
				send_message (S_INQUIRE_REQUEST_RANK, 1,
					      "", "", *list, "", "");
				switch (output(R_INQUIRE_REQUEST_RANK)) {
				case MNOINFO:
					LP_ERRMSG1 (ERROR,
					E_STAT_DONE, *list);
					exit_rc = 1;
					break;
				}
			}
			else
			{
				send_message (S_INQUIRE_REQUEST,
					      "", "", *list, "", "");
				switch (output(R_INQUIRE_REQUEST)) {
				case MNOINFO:
					LP_ERRMSG1 (ERROR, 
					E_STAT_DONE, *list);
					exit_rc = 1;
					break;
				}
			}

		} else {
			if (verbosity & V_RANK)
			{
				send_message (S_INQUIRE_REQUEST_RANK, 1,
					      "", *list, "", "", "");
				switch (output(R_INQUIRE_REQUEST_RANK)) {
				case MNOINFO:
					if (!isprinter(*list) &&
				            !isclass(*list)) {
						LP_ERRMSG1 (ERROR,
						E_STAT_BADSTAT, *list);
						exit_rc = 1;
					}
					break;
				}
			}
			else
			{
				send_message (S_INQUIRE_REQUEST,
					      "", *list, "", "", "");
				switch (output(R_INQUIRE_REQUEST)) {
				case MNOINFO:
					if (!isprinter(*list) &&
					    !isclass(*list)) {
						LP_ERRMSG1 (ERROR,
						E_STAT_BADSTAT, *list);
						exit_rc = 1;
					}
					break;
				}
			}

		}
		list++;
	}
	return;
}

/**
 ** do_user()
 **/

void
#if	defined(__STDC__)
do_user (
	char **			list
)
#else
do_user (list)
	char			**list;
#endif
{
	while (*list) {
		if (STREQU(NAME_ALL, *list)) {
			if (verbosity & V_RANK)
			{
				send_message (S_INQUIRE_REQUEST_RANK, 1,
					      "", "", "", "", "");
				(void)output (R_INQUIRE_REQUEST_RANK);
			}
			else
			{
				send_message (S_INQUIRE_REQUEST,
					      "", "", "", "", "");
				(void)output (R_INQUIRE_REQUEST);
			}
		} else {
			if (verbosity & V_RANK)
			{
				send_message (S_INQUIRE_REQUEST_RANK, 1,
					      "", "", "", *list, "");
				switch (output(R_INQUIRE_REQUEST_RANK)) {
				case MNOINFO:
					if (!getpwnam(*list))
						LP_ERRMSG1 (WARNING,
						E_STAT_USER, *list);
					break;
				}
			}
			else
			{
				send_message (S_INQUIRE_REQUEST,
					      "", "", "", *list, "");
				switch (output(R_INQUIRE_REQUEST)) {
				case MNOINFO:
					if (!getpwnam(*list))
						LP_ERRMSG1 (WARNING,
						E_STAT_USER, *list);
					break;
				}
			}
		}
		list++;
	}
	return;
}

/**
 ** putoline()
 **/

void
#if	defined(__STDC__)
putoline (
	char *			request_id,
	char *			user,
	long			size,
	char *			date,
	int			state,
	char *			printer,
	char *			form,
	char *			character_set,
	int			rank
)
#else
putoline (request_id, user, size, date, state, printer, form, character_set, rank)
	char			*request_id,
				*user,
				*date,
				*printer,
				*form,
				*character_set;
	long			size;
	int			state;
	int			rank;
#endif
{
	if (verbosity & V_RANK)
		(void) printf("%3d ", rank);

	(void) printf(
		"%-*s %-*s %*ld %s%.12s",
		((verbosity & V_RANK)? IDSIZE - 2 : IDSIZE),
		request_id,
		LOGMAX-1,
		user,
		OSIZE,
		size,
		((verbosity & V_RANK)? "" : "  "),
		date + 4
	);


	if (!(verbosity & (V_LONG|V_BITS))) {

		/*
		 * Unless the -l option is given, we show the CURRENT
		 * status. Check the status bits in reverse order of
		 * chronology, i.e. go with the bit that would have been
		 * set last. Old bits don't get cleared by the Spooler.
		 * We only have space for 21 characters!
		 */

		if (state & RS_NOTIFYING)
			(void) printf(" notifying user");

		else if (state & RS_CANCELLED)
			(void) printf(" canceled");

		else if (state & RS_PRINTED)
			(void) printf(" finished printing");

		else if (state & RS_PRINTING)
			(void) printf(" on %s", printer);

		else if (state & RS_ADMINHELD)
			(void) printf(" held by admin");

		else if (state & RS_HELD)
			(void) printf(" being held");

		else if (state & RS_FILTERED)
			(void) printf(" filtered");

		else if (state & RS_FILTERING)
			(void) printf(" being filtered");

		else if (state & RS_CHANGING)
			(void) printf(" held for change");

	} else if (verbosity & V_BITS) {
		register char		*sep	= "\n	";

			BITPRINT (state, RS_HELD);
			BITPRINT (state, RS_FILTERING);
			BITPRINT (state, RS_FILTERED);
			BITPRINT (state, RS_PRINTING);
			BITPRINT (state, RS_PRINTED);
			BITPRINT (state, RS_CHANGING);
			BITPRINT (state, RS_CANCELLED);
			BITPRINT (state, RS_IMMEDIATE);
			BITPRINT (state, RS_FAILED);
			BITPRINT (state, RS_SENDING);
			BITPRINT (state, RS_NOTIFY);
			BITPRINT (state, RS_NOTIFYING);
			BITPRINT (state, RS_SENT);
			BITPRINT (state, RS_ADMINHELD);
			BITPRINT (state, RS_REFILTER);
			BITPRINT (state, RS_STOPPED);

	} else if (verbosity & V_LONG) {

		register char		*sep	= "\n	";

		/*
		 * Here we show all the interesting states the job
		 * has gone through. Left to right they are in
		 * chronological order.
		 */

		if (state & RS_PRINTING) {
			(void) printf("%son %s", sep, printer);
			sep = ", ";

		} else if (state & RS_CANCELLED) {
			(void) printf("%scanceled", sep);
			sep = ", ";

		} else if (state & RS_FAILED) {
			(void) printf("%sfailed", sep);
			sep = ", ";

		} else if (state & RS_PRINTED) {
			(void) printf("%sfinished on %s", sep, printer);
			sep = ", ";

		/*
		 * WATCH IT! We make the request ID unusable after
		 * the next line.
		 */
		} else if (!STREQU(strtok(request_id, "-"), printer)) {
			(void) printf("%sassigned %s", sep, printer);
			sep = ", ";

		} else {
			if (state & RS_SENT)
				(void)printf ("%squeued remotely for %s", sep, printer);
			else
				(void)printf ("%squeued for %s", sep, printer);
			sep = ", ";
		}

		if (!(state & RS_DONE)) {
			if (form && *form) {
				(void) printf("%sform %s", sep, form);
				sep = ", ";
			}
			if (character_set && *character_set) {
				(void) printf("%scharset %s", sep, character_set);
				sep = ", ";
			}
		}

		if (state & RS_NOTIFYING) {
			(void) printf("%snotifying user", sep);
			sep = ", ";

		} else if (state & RS_CHANGING) {
			(void) printf("%sheld for change", sep);
			sep = ", ";

		} else if (state & RS_ADMINHELD) {
			(void) printf("%sheld by admin", sep);
			sep = ", ";

		} else if (state & RS_HELD) {
			(void) printf("%sbeing held", sep);
			sep = ", ";

		}

		if (state & RS_FILTERED) {
			(void) printf("%sfiltered", sep);
			sep = ", ";

		} else if (state & RS_FILTERING) {
			(void) printf("%sbeing filtered", sep);
			sep = ", ";
		}

	}
	(void) printf("\n");
	return;
}
