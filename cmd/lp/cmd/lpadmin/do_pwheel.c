/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/lpadmin/do_pwheel.c	1.11.3.1"

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "printers.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPADMIN
#include "oam.h"

#include "lpadmin.h"


extern char		*nameit(),
			*label;

static void		configure_pwheel();

/**
 ** do_pwheel() - SET ALERT FOR NEED TO MOUNT PRINT WHEEL
 **/

void			do_pwheel ()
{
	int			rc;


	if (A && STREQU(A, NAME_NONE)) {
		BEGIN_CRITICAL
			if (delpwheel(*S) == -1) {
				LP_ERRMSG1 (WARNING, E_ADM_BADPWHEEL, *S);
				return;
			}
		END_CRITICAL

	} else if (strlen(modifications))
		configure_pwheel (modifications);

	if (A && STREQU(A, NAME_LIST)) {
		if (label)
			(void) printf("Print wheel %s: ", label);
		printalert (stdout, &(oldS->alert), 0);
		return;
	}

	if (A && STREQU(A, NAME_QUIET)) {

		send_message(S_QUIET_ALERT, *S, (char *)QA_PRINTWHEEL, "");
		rc = output(R_QUIET_ALERT);

		switch(rc) {
		case MOK:
			break;

		case MNODEST:	/* not quite, but not a lie either */
		case MERRDEST:
			LP_ERRMSG1 (WARNING, E_LP_NOQUIET, *S);
			break;

		case MNOPERM:	/* taken care of up front */
		default:
			LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
			done (1);
			/*NOTREACHED*/
		}

		return;
	}

	if (A && STREQU(A, NAME_NONE)) {
		send_message(S_UNLOAD_PRINTWHEEL, *S);
		rc = output(R_UNLOAD_PRINTWHEEL);
	} else {
		send_message(S_LOAD_PRINTWHEEL, *S);
		rc = output(R_LOAD_PRINTWHEEL);
	}

	switch(rc) {
	case MOK:
		break;

	case MNODEST:
		/*
		 * Should only occur if we're deleting a print wheel
		 * alert that doesn't exist.
		 */
		break;

	case MERRDEST:
		LP_ERRMSG (ERROR, E_ADM_ERRDEST);
		done (1);
		/*NOTREACHED*/

	case MNOSPACE:
		LP_ERRMSG (WARNING, E_ADM_NOPWSPACE);
		break;

	case MNOPERM:	/* taken care of up front */
	default:
		LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, rc);
		done (1);
		/*NOTREACHED*/
	}
	return;
}

/**
 ** configure_pwheel() - SET OR CHANGE CONFIGURATION OF A PRINT WHEEL
 **/

static void		configure_pwheel (list)
	char			*list;
{
	register PWHEEL		*ppw;

	PWHEEL			pwheel_buf;

	char			type;


	if (oldS)
		ppw = oldS;
	else {
		ppw = &pwheel_buf;
		ppw->alert.shcmd = 0;
		ppw->alert.Q = 0;
		ppw->alert.W = 0;
	}

	while ((type = *list++) != '\0')  switch(type) {

	case 'A':
		if (STREQU(A, NAME_MAIL) || STREQU(A, NAME_WRITE))
			ppw->alert.shcmd = nameit(A);
		else
			ppw->alert.shcmd = A;

		break;

	case 'Q':
		ppw->alert.Q = Q;
		break;

	case 'W':
		ppw->alert.W = W;
		break;

	}

	BEGIN_CRITICAL
		if (putpwheel(*S, ppw) == -1) {
			LP_ERRMSG2 (
				ERROR,
				E_ADM_PUTPWHEEL,
				*S,
				PERROR
			);
			done(1);
		}
	END_CRITICAL

	return;
}
