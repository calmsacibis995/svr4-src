/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpshut.c	1.8.3.1"

#include "stdio.h"
#include "signal.h"
#include "string.h"
#include "sys/types.h"
#include "errno.h"
#include "stdlib.h"

#include "lp.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPSHUT
#include "oam.h"

void			startup(),
			cleanup(),
			done();

/*
 * There are no sections of code in this progam that have to be
 * protected from interrupts. We do want to catch them, however,
 * so we can clean up properly.
 */

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	char			msgbuf[MSGMAX];
	char *			tempo;

	int			mtype;

	short			status;


	if (argc > 1)
		if (STREQU(argv[1], "-?")) {
			printf ("usage: lpshut\n");
			exit (0);

		} else {
			LP_ERRMSG1 (ERROR, E_LP_OPTION, argv[1]);
			exit (1);
		}


	startup ();

	if ((tempo = getenv("LPSHUT")) && STREQU(tempo, "slow"))
		(void)putmessage (msgbuf, S_SHUTDOWN, 0);
	else
		(void)putmessage (msgbuf, S_SHUTDOWN, 1);

	if (msend(msgbuf) == -1) {
		LP_ERRMSG (ERROR, E_LP_MSEND);
		done (1);
	}
	if (mrecv(msgbuf, sizeof(msgbuf)) == -1) {
		LP_ERRMSG (ERROR, E_LP_MRECV);
		done (1);
	}

	mtype = getmessage(msgbuf, R_SHUTDOWN, &status);
	if (mtype != R_SHUTDOWN) {
		LP_ERRMSG1 (ERROR, E_LP_BADREPLY, mtype);
		done (1);
	}

	switch (status) {

	case MOK:
		printf ("Print services stopped.\n");
		done (0);

	case MNOPERM:
		LP_ERRMSG (WARNING, E_SHT_CANT);
		done (1);

	default:
		LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, status);
		done (1);
	}
	/*NOTREACHED*/
}

/**
 ** startup() - OPEN MESSAGE QUEUE TO SPOOLER
 **/

void			startup ()
{
	void			catch();

	/*
	 * Open a private queue for messages to the Spooler.
	 * An error is deadly.
	 */
	if (mopen() == -1) {

		switch (errno) {
		case ENOMEM:
		case ENOSPC:
			LP_ERRMSG (ERROR, E_LP_MLATER);
			exit (1);
			/*NOTREACHED*/

		default:
			printf ("Print services already stopped.\n");
			exit (1);
			/*NOTREACHED*/
		}
	}

	/*
	 * Now that the queue is open, quickly trap signals
	 * that we might get so we'll be able to close the
	 * queue again, regardless of what happens.
	 */
	if(signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, catch);
	if(signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, catch);
	if(signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, catch);
	if(signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, catch);

	return;
}

/**
 ** catch() - CATCH INTERRUPT, HANGUP, ETC.
 **/

void			catch (sig)
	int			sig;
{
	signal (sig, SIG_IGN);
	done (1);
}

/**
 ** cleanup() - CLOSE THE MESSAGE QUEUE TO THE SPOOLER
 **/

void			cleanup ()
{
	mclose ();
	return;
}

/**
 ** done() - CLEANUP AND EXIT
 **/

void			done (ec)
	int			ec;
{
	cleanup ();
	exit (ec);
}
