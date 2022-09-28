/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/faults.c	1.2.4.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "lpsched.h"


/**
 ** printer_fault() - RECOGNIZE PRINTER FAULT
 **/

void
#if	defined(__STDC__)
printer_fault (
	register PSTATUS *	pps,
	register RSTATUS *	prs,
	char *			alert_text,
	int			err
)
#else
printer_fault (pps, prs, alert_text, err)
	register PSTATUS	*pps;
	register RSTATUS	*prs;
	char			*alert_text;
	int			err;
#endif
{
	ENTRY ("printer_fault")

	register char		*why;


	pps->status |= PS_FAULTED;

	/*  -F wait  */
	if (STREQU(pps->printer->fault_rec, NAME_WAIT))
		disable (pps, CUZ_FAULT, DISABLE_STOP);

	/*  -F beginning  */
	else if (STREQU(pps->printer->fault_rec, NAME_BEGINNING))
		terminate (pps->exec);

	/*  -F continue  AND  the interface program died  */
	else if (!(pps->status & PS_LATER) && !pps->request) {
		load_str (&pps->dis_reason, CUZ_STOPPED);
		schedule (EV_LATER, WHEN_PRINTER, EV_ENABLE, pps);
	}

	if (err) {
		errno = err;
		why = makestr(alert_text, "(", PERROR, ")\n", (char *)0);
		if (!why)
			why = alert_text;
	} else
		why = alert_text;
	alert (A_PRINTER, pps, prs, why);
	if (why != alert_text)
		Free (why);

	return;
}

/**
 ** dial_problem() - ADDRESS DIAL-OUT PROBLEM
 **/

void
#if	defined(__STDC__)
dial_problem (
	register PSTATUS *	pps,
	RSTATUS *		prs,
	int			rc
)
#else
dial_problem (pps, prs, rc)
	register PSTATUS	*pps;
	RSTATUS			*prs;
	int			rc;
#endif
{
	ENTRY ("dial_problem")

	static struct problem {
		char			*reason;
		int			retry_max,
					dial_error;
	}			problems[] = {
		"DIAL FAILED",			10,	 2, /* D_HUNG  */
		"CALLER SCRIPT FAILED",		10,	 3, /* NO_ANS  */
		"CAN'T ACCESS DEVICE",		 0,	 6, /* L_PROB  */
		"DEVICE LOCKED",		20,	 8, /* DV_NT_A */
		"NO DEVICES AVAILABLE",		 0,	10, /* NO_BD_A */
		"SYSTEM NOT IN Systems FILE",	 0,	13, /* BAD_SYS */
		"UNKNOWN dial() FAILURE",	 0,	0
	};

	register struct problem	*p;

	register char		*msg;

#define PREFIX	"Connect problem: "
#define SUFFIX	"This problem has occurred several times.\nPlease check the dialing instructions for this printer.\n"


	for (p = problems; p->dial_error; p++)
		if (p->dial_error == rc)
			break;

	if (!p->retry_max) {
		msg = Malloc(strlen(PREFIX) + strlen(p->reason) + 2);
		sprintf (msg, "%s%s\n", PREFIX, p->reason);
		printer_fault (pps, prs, msg, 0);
		Free (msg);

	} else if (pps->last_dial_rc != rc) {
		pps->nretry = 1;
		pps->last_dial_rc = (short)rc;

	} else if (pps->nretry++ > p->retry_max) {
		pps->nretry = 0;
		pps->last_dial_rc = (short)rc;
		msg = Malloc(
		strlen(PREFIX) + strlen(p->reason) + strlen(SUFFIX) + 1
		);
		sprintf (msg, "%s%s%s\n", PREFIX, p->reason, SUFFIX);
		printer_fault (pps, prs, msg, 0);
		Free (msg);
	}

	if (!(pps->status & PS_FAULTED)) {
		load_str (&pps->dis_reason, p->reason);
		schedule (EV_LATER, WHEN_PRINTER, EV_ENABLE, pps);
	}

	return;
}
