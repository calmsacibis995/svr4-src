/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpstat/lpstat.c	1.12.3.1"

#include "stdio.h"
#include "errno.h"
#include "sys/types.h"
#include "signal.h"
#include "stdlib.h"

#include "lp.h"
#include "msgs.h"
#include "printers.h"
#include "debug.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


#ifdef SIGPOLL
static void
#else
static int
#endif
#if	defined(__STDC__)
			catch ( int );
#else
			catch();
#endif

#if	defined(__STDC__)
static void		mallocfail ( void );
#else
static void		mallocfail ();
#endif

int			exit_rc			= 0,
			inquire_type		= INQ_UNKNOWN,
			scheduler_active	= 0,
			r;		/* Says -r was specified */

char			*alllist[]	= {
	NAME_ALL,
	0
};

/**
 ** main()
 **/

int
#if	defined(__STDC__)
main (
	int			argc,
	char *			argv[]
)
#else
main (argc, argv)
	int			argc;
	char			*argv[];
#endif
{
	lp_alloc_fail_handler = mallocfail;
	OPEN_DEBUG_FILE ("/tmp/lpstat.debug")
	parse (argc, argv);
	done (0);
	/*NOTREACHED*/
}

/**
 ** def()
 **/

void
#if	defined(__STDC__)
def (
	void
)
#else
def ()
#endif
{
	char			*name;

	if ((name = getdefault()))
		(void) printf("system default destination: %s\n", name);
	else
		(void) printf("no system default destination\n");

	return;
}

/**
 ** running()
 **/

void
#if	defined(__STDC__)
running (
	void
)
#else
running ()
#endif
{
	(void) printf(
		"scheduler is %s\n",
		(scheduler_active ? "running" : "not running")
	);
	return;
}

/**
 ** startup()
 **/

void
#if	defined(__STDC__)
startup (
	void
)
#else
startup ()
#endif
{
	int			try;


	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		(void)signal (SIGHUP, catch);

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void)signal (SIGINT, catch);

	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		(void)signal (SIGQUIT, catch);

	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		(void)signal (SIGTERM, catch);

	for (try = 1; try <= 5; try++) {
		scheduler_active = (mopen() == 0);
		if (scheduler_active || errno != ENOSPC)
			break;
		sleep (3);
	}

	return;
}

/**
 ** catch()
 **/

#ifdef SIGPOLL
static void
#else
static int
#endif
#if	defined(__STDC__)
catch (
	int			ignore
)
#else
catch (ignore)
	int			ignore;
#endif
{
	(void)signal (SIGHUP, SIG_IGN);
	(void)signal (SIGINT, SIG_IGN);
	(void)signal (SIGQUIT, SIG_IGN);
	(void)signal (SIGTERM, SIG_IGN);
	done (2);
}

/**
 ** mallocfail()
 **/

static void
#if	defined(__STDC__)
mallocfail (
	void
)
#else
mallocfail ()
#endif
{
	LP_ERRMSG (ERROR, E_LP_MALLOC);
	done (1);
}
