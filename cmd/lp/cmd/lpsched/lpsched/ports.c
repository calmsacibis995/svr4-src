/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/ports.c	1.3.4.1"

#include "termio.h"
#include "dial.h"
#include "unistd.h"

#include "lpsched.h"

#if	defined(__STDC__)
static void		sigalrm ( int );
static int		push_module ( int , char * , char * );
#else
static void		sigalrm();
static int		push_module();
#endif

static int		SigAlrm;

/**
 ** open_dialup() - OPEN A PORT TO A ``DIAL-UP'' PRINTER
 **/

int
#if	defined(__STDC__)
open_dialup (
	char *			ptype,	/*UNUSED*/
	PRINTER *		pp
)
#else
open_dialup (ptype, pp)
	char *			ptype;	/*UNUSED*/
	register PRINTER *	pp;
#endif
{
	ENTRY ("open_dialup")

	static char		*baud_table[]	= {
		      0,
		   "50",
		   "75",
		  "110",
		  "134",
		  "150",
		  "200",
		  "300",
		  "600",
		 "1200",
		 "1800",
		 "2400",
		 "4800",
		 "9600",
		"19200",
		"38400"
	};

	struct termio		tio;

	CALL			call;

	int			speed,
				fd;

	char			*sspeed;


	if ((speed = atoi(pp->speed)) <= 0)
		speed = -1;

	call.attr = 0;
	call.speed = speed;
	call.line = 0;
	call.telno = pp->dial_info;

	if ((fd = dial(call)) < 0)
		return (EXEC_EXIT_NDIAL | (~EXEC_EXIT_NMASK & abs(fd)));

	/*
	 * "dial()" doesn't guarantee which file descriptor
	 * it uses when it opens the port, so we probably have to
	 * move it.
	 */
	if (fd != 1) {
		dup2 (fd, 1);
		Close (fd);
	}

	/*
	 * The "printermgmt()" routines move out of ".stty"
	 * anything that looks like a baud rate, and puts it
	 * in ".speed", if the printer port is dialed. Thus
	 * we are saved the task of cleaning out spurious
	 * baud rates from ".stty".
	 *
	 * However, we must determine the baud rate and
	 * concatenate it onto ".stty" so that that we can
	 * override the default in the interface progam.
	 * Putting the override in ".stty" allows the user
	 * to override us (although it would be probably be
	 * silly for him or her to do so.)
	 */
	ioctl (1, TCGETA, &tio);
	if ((sspeed = baud_table[(tio.c_cflag & CBAUD)])) {

		register char	*new_stty = Malloc(
			strlen(pp->stty) + 1 + strlen(sspeed) + 1
		);

		sprintf (new_stty, "%s %s", pp->stty, sspeed);

		/*
		 * We can trash "pp->stty" because
		 * the parent process has the good copy.
		 */
		pp->stty = new_stty;
	}

	return (0);
}

/**
 ** open_direct() - OPEN A PORT TO A DIRECTLY CONNECTED PRINTER
 **/

int
#if	defined(__STDC__)
open_direct (
	char *			ptype,
	PRINTER *		pp
)
#else
open_direct (ptype, pp)
	char *			ptype;
	register PRINTER *	pp;
#endif
{
	ENTRY ("open_direct")

	short			bufsz	    = -1,
				cps	    = -1;

	int			open_mode,
				fd;

	register unsigned int	oldalarm,
				newalarm    = 0;

	register void		(*oldsig)() = signal(SIGALRM, sigalrm);


	/*
	 * Set an alarm to wake us from trying to open the port.
	 * We'll try at least 60 seconds, or more if the printer
	 * has a huge buffer that, in the worst case, would take
	 * a long time to drain.
	 */
	tidbit (ptype, "bufsz", &bufsz);
	tidbit (ptype, "cps", &cps);
	if (bufsz > 0 && cps > 0)
		newalarm = (((long)bufsz * 1100) / cps) / 1000;
	if (newalarm < 60)
		newalarm = 60;
	oldalarm = alarm(newalarm);

	/*
	 * The following open must be interruptable.
	 * O_APPEND is set in case the ``port'' is a file.
	 * O_RDWR is set in case the interface program wants
	 * to get input from the printer. Don't fail, though,
	 * just because we can't get read access.
	 */

	open_mode = O_WRONLY;
	if (access(pp->device, R_OK) == 0)
		open_mode = O_RDWR;
	open_mode |= O_APPEND;

	SigAlrm = 0;
	while ((fd = open(pp->device, open_mode, 0)) == -1)
		if (errno != EINTR)
			return (EXEC_EXIT_NPORT);
		else if (SigAlrm)
			return (EXEC_EXIT_TMOUT);

	alarm (oldalarm);
	signal (SIGALRM, oldsig);

	/*
	 * We should get the correct channel number (1), but just
	 * in case....
	 */
	if (fd != 1) {
		dup2 (fd, 1);
		Close (fd);
	}

	/*
	 * Handle streams modules:
	 */
	if (isastream(1)) {

		/*
		 * First, pop all current modules off, unless
		 * instructed not to.
		 */
#if	defined(CAN_DO_MODULES)
		if (
			emptylist(pp->modules)
		     || !STREQU(pp->modules[0], NAME_KEEP)
		)
#endif
			while (ioctl(1, I_POP, 0) == 0)
				;

		/*
		 * Now push either the administrator specified modules
		 * or the standard modules, unless instructed to push
		 * nothing.
		 */
#if	defined(CAN_DO_MODULES)
		if (
			emptylist(pp->modules)
		     || STREQU(NAME_NONE, pp->modules[0])
		     || STREQU(NAME_KEEP, pp->modules[0])
		)
			/*EMPTY*/;

		else if (!STREQU(NAME_DEFAULT, pp->modules[0])) {
			char **			pm = pp->modules;

			while (*pm)
				if (push_module(1, pp->device, *pm++) == -1)
					return (EXEC_EXIT_NPUSH);
		} else
#endif
		{
			char **			pm = getlist(DEFMODULES, LP_WS, LP_SEP);

			while (*pm)
				if (push_module(1, pp->device, *pm++) == -1)
					return (EXEC_EXIT_NPUSH);
		}
	}

	return (0);
}

/**
 ** sigalrm()
 **/

static void
#if	defined(__STDC__)
sigalrm (
	int			ignore	/*UNUSED*/
)
#else
sigalrm (ignore)
	int			ignore;	/*UNUSED*/
#endif
{
	ENTRY ("sigalrm")

	signal (SIGALRM, SIG_IGN);
	SigAlrm = 1;
	return;
}


/**
 ** push_module()
 **/

static int
#if	defined(__STDC__)
push_module (
	int			fd,
	char *			device,
	char *			module
)
#else
push_module (fd, device, module)
	int			fd;
	char *			device;
	char *			module;
#endif
{
	ENTRY ("push_module")

	int			ret	= ioctl(fd, I_PUSH, module);

	if (ret == -1)
		note ("push (%s) on %s failed (%s)\n", module, device, PERROR);
	return (ret);
}
