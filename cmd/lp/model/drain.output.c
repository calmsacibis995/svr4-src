/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:model/drain.output.c	1.5.2.1"

#include "termio.h"

/*
 * The following macro computes the number of seconds to sleep
 * AFTER waiting for the system buffers to be drained.
 *
 * Various choices:
 *
 *	- A percentage (perhaps even >100%) of the time it would
 *	  take to print the printer's buffer. Use this if it appears
 *	  the printers are affected if the port is closed before they
 *	  finish printing.
 *
 *	- 0. Use this to avoid any extra sleep after waiting for the
 *	  system buffers to be flushed.
 *
 *	- N > 0. Use this to have a fixed sleep after flushing the
 *	  system buffers.
 *
 * The sleep period can be overridden by a single command line argument.
 */
			/* 25% of the print-full-buffer time, plus 1 */
#define LONG_ENOUGH(BUFSZ,CPS)	 (1 + ((250 * BUFSZ) / CPS) / 1000)

extern int		tidbit();

/**
 ** main()
 **/

int			main (argc, argv)
	int			argc;
	char			*argv[];
{
	extern char		*getenv();

	short			bufsz	= -1,
				cps	= -1;

	char			*TERM;

	int			sleep_time	= 0;


	/*
	 * Wait for the output to drain.
	 */
	ioctl (1, TCSBRK, (struct termio *)1);

	/*
	 * Decide how long to sleep.
	 */
	if (argc != 2 || (sleep_time = atoi(argv[1])) < 0)
		if ((TERM = getenv("TERM"))) {
			tidbit (TERM, "bufsz", &bufsz);
			tidbit (TERM, "cps", &cps);
			if (cps > 0 && bufsz > 0)
				sleep_time = LONG_ENOUGH(bufsz, cps);
		} else
			sleep_time = 2;

	/*
	 * Wait ``long enough'' for the printer to finish
	 * printing what's in its buffer.
	 */
	if (sleep_time)
		sleep (sleep_time);

	exit (0);
	/*NOTREACHED*/
}
