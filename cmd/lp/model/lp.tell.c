/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:model/lp.tell.c	1.6.2.1"

#include "signal.h"
#include "stdio.h"
#include "errno.h"

#include "lp.h"
#include "msgs.h"

void			startup(),
			cleanup(),
			done();

extern char		*getenv(),
			*malloc(),
			*realloc();

extern long		atol();

extern int		atoi();

static void		wakeup();

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	char			*alert_text,
				buf[BUFSIZ],
				msgbuf[MSGMAX],
				*printer,
				*s_key;

	int			mtype,
				oldalarm;

	short			status;

	long			key;

	void			(*oldsignal)();


	/*
	 * Run immune from typical interruptions, so that
	 * we stand a chance to get the fault message.
	 * EOF (or startup error) is the only way out.
	 */
	signal (SIGHUP, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
	signal (SIGTERM, SIG_IGN);

	/*
	 * Which printer is this? Do we have a key?
	 */
	if (
		argc != 2
	     || !(printer = argv[1])
	     || !*printer
	     || !(s_key = getenv("SPOOLER_KEY"))
	     || !*s_key
	     || (key = atol(s_key)) <= 0
	)
		exit (90);

	/*
	 * Wait for a message on the standard input. When a single line
	 * comes in, take a couple of more seconds to get any other lines
	 * that may be ready, then send them to the Spooler.
	 */
	while (fgets(buf, BUFSIZ, stdin)) {

		oldsignal = signal(SIGALRM, wakeup);
		oldalarm = alarm(2);

		alert_text = 0;
		do {
			if (alert_text)
				alert_text = realloc(
					alert_text,
					strlen(alert_text)+strlen(buf)+1
				);
			else {
				alert_text = malloc(strlen(buf) + 1);
				alert_text[0] = 0;
			}
			strcat (alert_text, buf);

		} while (fgets(buf, BUFSIZ, stdin));

		alarm (oldalarm);
		signal (SIGALRM, oldsignal);

		if (alert_text) {

			startup ();

			(void)putmessage (
				msgbuf,
				S_SEND_FAULT,
				printer,
				key,
				alert_text
			);

			if (msend(msgbuf) == -1)
				done (91);
			if (mrecv(msgbuf, sizeof(msgbuf)) == -1)
				done (92);

			mtype = getmessage(msgbuf, R_SEND_FAULT, &status);
			if (mtype != R_SEND_FAULT)
				done (93);

			if (status != MOK)
				done (94);
		}
	}
	done (0);
}

/**
 ** startup() - OPEN MESSAGE QUEUE TO SPOOLER
 ** cleanup() - CLOSE THE MESSAGE QUEUE TO THE SPOOLER
 **/

static int		have_contacted_spooler	= 0;

void			startup ()
{
	void			catch();

	/*
	 * Open a message queue to the Spooler.
	 * An error is deadly.
	 */
	if (!have_contacted_spooler) {
		if (mopen() == -1) {
	
			switch (errno) {
			case ENOMEM:
			case ENOSPC:
				break;
			default:
				break;
			}

			exit (1);
		}
		have_contacted_spooler = 1;
	}
	return;
}

void			cleanup ()
{
	if (have_contacted_spooler)
		mclose ();
	return;
}

/**
 ** wakeup() - TRAP ALARM
 **/

static void		wakeup ()
{
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
