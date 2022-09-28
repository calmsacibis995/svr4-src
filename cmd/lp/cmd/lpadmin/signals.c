/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpadmin/signals.c	1.5.2.1"

#include "signal.h"

#include "lpadmin.h"

static int		trapping	= -1;	/* -1 means first time */

static
#ifdef	SIGPOLL
	void
#else
	int
#endif
			(*old_sighup)(),
			(*old_sigint)(),
			(*old_sigquit)(),
			(*old_sigterm)();

/**
 ** catch() - CLEAN UP AFTER SIGNAL
 **/

static void
catch (sig)
{
	(void)signal (SIGHUP, SIG_IGN);
	(void)signal (SIGINT, SIG_IGN);
	(void)signal (SIGQUIT, SIG_IGN);
	(void)signal (SIGTERM, SIG_IGN);
	done (1);
}

/**
 ** trap_signals() - SET SIGNALS TO BE CAUGHT FOR CLEAN EXIT
 **/

void			trap_signals ()
{
	switch (trapping) {

	case -1:	/* first time */

#define	SETSIG(SIG) \
		if (signal(SIG, SIG_IGN) != SIG_IGN) \
			signal (SIG, catch);

		SETSIG (SIGHUP);
		SETSIG (SIGINT);
		SETSIG (SIGQUIT);
		SETSIG (SIGTERM);
		break;

	case 0:		/* currently ignoring */
		signal (SIGHUP, old_sighup);
		signal (SIGINT, old_sigint);
		signal (SIGQUIT, old_sigquit);
		signal (SIGTERM, old_sigterm);
		trapping = 1;
		break;

	case 1:		/* already trapping */
		break;

	}
	return;
}

/**
 ** ignore_signals() - SET SIGNALS TO BE IGNORED FOR CRITICAL SECTIONS
 **/

void			ignore_signals ()
{
	switch (trapping) {

	case -1:	/* first time */
		trap_signals ();
		/*fall through*/

	case 1:		/* currently trapping */
		old_sighup = signal(SIGHUP, SIG_IGN);
		old_sigint = signal(SIGINT, SIG_IGN);
		old_sigquit = signal(SIGQUIT, SIG_IGN);
		old_sigterm = signal(SIGTERM, SIG_IGN);
		trapping = 0;
		break;

	case 0:		/* already ignoring */
		break;

	}
	return;
}
