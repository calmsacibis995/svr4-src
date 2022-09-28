/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:model/lp.cat.c	1.9.3.1"

#include "stdio.h"
#include "termio.h"
#include "sys/types.h"
#include "errno.h"
#include "signal.h"
#include "sys/times.h"
#include "string.h"
#include "limits.h"

#include "lp.h"

#define	IDENTICAL(A,B)	(A.st_dev==B.st_dev && A.st_ino==B.st_ino)
#define ISBLK(A)	((A.st_mode & S_IFMT) == S_IFBLK)
#define ISCHR(A)	((A.st_mode & S_IFMT) == S_IFCHR)

#define E_SUCCESS	0
#define E_BAD_INPUT	1
#define E_BAD_OUTPUT	2
#define E_BAD_TERM	3
#define E_IDENTICAL	4
#define	E_WRITE_FAILED	5
#define	E_TIMEOUT	6
#define E_HANGUP	7
#define E_INTERRUPT	8

#define SAFETY_FACTOR	2.0
#define R(F)		(int)((F) + .5)
#define DELAY(N,D)	R(SAFETY_FACTOR * ((N) / (double)(D)))

extern int		sys_nerr;

extern char		*sys_errlist[],
			*getenv();

extern int		atoi();

char			buffer[BUFSIZ];

void			sighup(),
			sigint(),
			sigquit(),
			sigpipe(),
			sigalrm(),
			sigterm();

#if	defined(baudrate)
# undef	baudrate
#endif

int			baudrate();

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	register int		nin,
				nout,
				effective_rate,
				max_delay	= 0,
				n;

	int			report_rate;

	short			print_rate;

	struct stat		in,
				out;

	struct tms		tms;

	long			epoch_start,
				epoch_end;

	char			*TERM;


	/*
	 * The Spooler can hit us with SIGTERM for three reasons:
	 *
	 *	- the user's job has been canceled
	 *	- the printer has been disabled while we were printing
	 *	- the Spooler heard that the printer has a fault,
	 *	  and the fault recovery is wait or beginning
	 *
	 * We should exit cleanly for the first two cases,
	 * but we have to be careful with the last. If it was THIS
	 * PROGRAM that told the Spooler about the fault, we must
	 * exit consistently.
	 *
	 * The method of avoiding any problem is to turn off the
	 * trapping of SIGTERM before telling the Spooler about
	 * the fault.
	 *
	 * Faults that we can detect:
	 *	- hangup (drop of carrier)
	 *	- interrupt (printer sent a break or quit character)
	 *	- SIGPIPE (output port is a FIFO, and was closed early)
	 *	- failed or incomplete write()
	 *	- excess delay in write() (handled with SIGALRM later)
	 *
	 * Pseudo-faults (errors in use):
	 *	- No input/output, or strange input/output
	 *	- Input/output identical
	 *	- No TERM defined or trouble reading Terminfo database
	 */
	signal (SIGTERM, sigterm);
	signal (SIGHUP, sighup);
	signal (SIGINT, sigint);
	signal (SIGQUIT, sigint);
	signal (SIGPIPE, sigpipe);


	if (argc > 1 && STREQU(argv[1], "-r")) {
		report_rate = 1;
		argc--;
		argv++;
	} else
		report_rate = 0;

	/*
	 * Stat the standard output to be sure it is defined.
	 */
	if (fstat(1, &out) < 0) {
		signal (SIGTERM, SIG_IGN);
		fprintf (
			stderr,
		"Can't stat output (%s);\nincorrect use of lp.cat!\n",
			PERROR
		);
		exit (E_BAD_OUTPUT);
	}

	/*
	 * Stat the standard input to be sure it is defined.
	 */
	if (fstat(0, &in) < 0) {
		signal (SIGTERM, SIG_IGN);
		fprintf (
			stderr,
		"Can't stat input (%s);\nincorrect use of lp.cat!\n",
			PERROR
		);
		exit (E_BAD_INPUT);
	}

	/*
	 * If the standard output is not a character special file or a
	 * block special file, make sure it is not identical to the
	 * standard input.
	 */
	if (!ISCHR(out) && !ISBLK(out) && IDENTICAL(out, in)) {
		signal (SIGTERM, SIG_IGN);
		fprintf (
			stderr,
	"Input and output are identical; incorrect use of lp.cat!\n"
		);
		exit (E_IDENTICAL);
	}

	/*
	 * The effective data transfer rate is the lesser
	 * of the transmission rate and print rate. If an
	 * argument was passed to us, it should be a data
	 * rate and it may be lower still.
	 * Based on the effective data transfer rate,
	 * we can predict the maximum delay we should experience.
	 * But there are other factors that could introduce
	 * delay, so let's be generous; after all, we'd rather
	 * err in favor of waiting too long to detect a fault
	 * than err too often on false alarms.
	 */

	if (
		!(TERM = getenv("TERM"))
	     || !*TERM
	) {
		signal (SIGTERM, SIG_IGN);
		fprintf (
			stderr,
	"No TERM variable defined! Trouble with the Spooler!\n"
		);
		exit (E_BAD_TERM);
	}
	if (
		!STREQU(TERM, NAME_UNKNOWN)
	     && tidbit(TERM, "cps", &print_rate) == -1
	) {
		signal (SIGTERM, SIG_IGN);
		fprintf (
			stderr,
"Trouble identifying printer type \"%s\"; check the Terminfo database.\n",
			TERM
		);
		exit (E_BAD_TERM);
	}
	if (STREQU(TERM, NAME_UNKNOWN))
		print_rate = -1;

	effective_rate = baudrate() / 10; /* okay for most bauds */
	if (print_rate != -1 && print_rate < effective_rate)
		effective_rate = print_rate;
	if (argc > 1 && (n = atoi(argv[1])) >= 0 && n < effective_rate)
		effective_rate = n;	  /* 0 means infinite delay */
	if (effective_rate)
		max_delay = DELAY(BUFSIZ, effective_rate);

	/*
	 * We'll use the "alarm()" system call to keep us from
	 * waiting too long to write to a printer in trouble.
	 */
	if (max_delay)
		signal (SIGALRM, sigalrm);

	/*
	 * While not end of standard input, copy blocks to
	 * standard output.
	 */
	while ((nin = read(0, buffer, BUFSIZ)) > 0) {

		if (max_delay)
			alarm (max_delay);
		if (report_rate)
			epoch_start = times(&tms);

		/*
		 * We should be safe from incomplete writes to a full
		 * pipe, as long as the size of the buffer we write is
		 * a even divisor of the pipe buffer limit. As long as
		 * we read from files or pipes (not communication devices)
		 * this should be true for all but the last buffer. The
		 * last will be smaller, and won't straddle the pipe max
		 * limit (think about it).
		 */
#if	PIPE_BUF < BUFSIZ || (PIPE_MAX % BUFSIZ)
		this_wont_compile;
#endif

		if ((nout = write(1, buffer, nin)) != nin) {
			signal (SIGTERM, SIG_IGN);
			if (nout < 0)
				fprintf (
					stderr,
	"Write failed (%s);\nperhaps the printer has gone off-line.\n",
					PERROR
				);
			else
				fprintf (
					stderr,
	"Incomplete write; perhaps the printer has gone off-line.\n"
				);
			exit (E_WRITE_FAILED);
		}
		if (max_delay)
			alarm (0);
		else if (report_rate) {
			epoch_end = times(&tms);
			if (epoch_end > epoch_start)
				fprintf (
					stderr,
					"%d CPS\n",
		R((100 * BUFSIZ) / (double)(epoch_end - epoch_start))
				);
		}

	}

	exit (E_SUCCESS);
	/*NOTREACHED*/
}

/**
 ** sighup() - CATCH A HANGUP (LOSS OF CARRIER)
 **/

void			sighup ()
{
	char			*msg = HANGUP_FAULT;

	
	signal (SIGTERM, SIG_IGN);
	signal (SIGHUP, SIG_IGN);
	msg[strlen(msg) - 2] = '.';
	fprintf (stderr, msg);
	exit (E_HANGUP);
}

/**
 ** sigint() - CATCH AN INTERRUPT
 **/

void			sigint ()
{
	signal (SIGTERM, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	fprintf (stderr, INTERRUPT_FAULT);
	exit (E_INTERRUPT);
}

/**
 ** sigpipe() - CATCH EARLY CLOSE OF PIPE
 **/

void			sigpipe ()
{
	signal (SIGTERM, SIG_IGN);
	signal (SIGPIPE, SIG_IGN);
	fprintf (stderr, PIPE_FAULT);
	exit (E_INTERRUPT);
}

/**
 ** sigalrm() - CATCH AN ALARM
 **/

void			sigalrm ()
{
	signal (SIGTERM, SIG_IGN);
	fprintf (
		stderr,
	"Excessive write delay; perhaps the printer has gone off-line.\n"
	);
	exit (E_TIMEOUT);
}

/**
 ** sigterm() - CATCH A TERMINATION SIGNAL
 **/

void			sigterm ()
{
	signal (SIGTERM, SIG_IGN);
	exit (E_SUCCESS);
}

/**
 ** baudrate() - RETURN BAUD RATE OF OUTPUT LINE
 **/

static int		baud_convert[] =
{
	0, 50, 75, 110, 135, 150, 200, 300, 600, 1200,
	1800, 2400, 4800, 9600, 19200, 38400
};

int			baudrate ()
{
	struct termio		tm;

	if (ioctl(1, TCGETA, &tm) < 0)
		return (1200);
	return (tm.c_cflag&CBAUD ? baud_convert[tm.c_cflag&CBAUD] : 1200);
}
