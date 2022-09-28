/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/lpsched.c	1.15.4.1"

#include "limits.h"
#include "ulimit.h"
#include "sys/utsname.h"

#include "lpsched.h"
#include "debug.h"

#include "sys/stat.h"

#if	defined(MALLOC_3X)
#include "malloc.h"
#endif

FILE			*LockFp		= 0;

int			Starting	= 0;
int			Shutdown	= 0;
int			DoneChildren	= 0;
int			Sig_Alrm	= 0;
int			OpenMax		= OPEN_MAX;
int			Reserve_Fds	= 0;

char			*Local_System	= 0;
char			*SHELL		= 0;

gid_t			Lp_Gid;
uid_t			Lp_Uid;

#if	defined(DEBUG)
unsigned long		debug = 0;
int			signals = 0;
#endif

extern int		errno;
extern void		shutdown_messages();

int			am_in_background	= 0;

static void		disable_signals();
static void		startup();
static void		process();
static void		ticktock();
static void		background();
static void		usage();
static void		Exit();
static void		disable_signals();

/**
 ** main()
 **/

#if	defined(__STDC__)
main (
	int			argc,
	char *			argv[]
)
#else
main (argc, argv)
    int		argc;
    char	*argv[];
#endif
{
    ENTRY ("main")

    int		c;
#if	defined(MALLOC_3X)
    extern int	optind;
#endif
    extern char	*optarg;
    extern int	optopt;
    extern int	opterr;
#if	defined(DEBUG)
    char *	cp;
#endif

    if (!(SHELL = getenv("SHELL")))
	SHELL = DEFAULT_SHELL;

#if	defined(MDL)
# ident "lpsched has MDL"
    /*
    **	Set the output of MDL to be MDL-LOG
    */
    mdl(SET_FILE_STREAM, 0, 0, 0, "MDL-LOG");
    /*
    **	Set the toggle Flag to cause the file to be opened
    **	and closed as needed rather than opened once and kept.
    **	(ie, it saves a file descriptor at the cost or performance).
    */
    mdl(TOGGLE_OPEN, 0, 0, 0, 0);
#endif

#if	defined(MALLOC_3X)

# if	defined(DEF_MXFAST)
    mallopt (M_MXFAST, DEF_MXFAST);
# endif
# if	defined(DEF_NLBLKS)
    mallopt (M_NLBLKS, DEF_NLBLKS);
# endif
# if	defined(DEF_GRAIN)
    mallopt (M_GRAIN, DEF_GRAIN);
# endif
    mallopt (M_KEEP, 0);

#endif

    opterr = 0;
    while((c = getopt(argc, (char * const *)argv, "D:dsf:n:r:M:")) != EOF)
        switch(c)
        {
# if defined (DEBUG)
	    case 'd':
		debug = DB_ALL;
		goto SkipD;
	    case 'D':
		if (*optarg == '?') {
			note ("\
-D flag[,flag...]    (all logs \"foo\" are in /var/lp/logs,\n\
                      although \"lpsched\" goes to stdout if SDB)\n\
\n\
  EXEC               (log all exec's in \"exec\")\n\
  DONE               (log just exec finishes in \"exec\")\n\
  INIT               (log initialization info in \"lpsched\" or stdout)\n\
  ABORT              (issue abort(2) on fatal error)\n\
  SCHEDLOG           (log additional debugging info in \"lpsched\")\n\
  SDB                (don't start lpsched as background process)\n\
  MESSAGES           (log all message traffic in \"messages\")\n"
			);
#if	defined(TRACE_MALLOC)
			note ("\
  MALLOC             (track malloc use; dump on SIGUSR1 in \"lpsched\")\n"
			);
#endif
			note ("\
  ALL                (all of the above; equivalent to -d)\n"
			);
			exit (0);
		}
		while ((cp = strtok(optarg, ", "))) {
#define IFSETDB(P,S,F)	if (STREQU(P, S)) debug |= F
			IFSETDB (cp, "EXEC", DB_EXEC);
			else IFSETDB (cp, "DONE", DB_DONE);
			else IFSETDB (cp, "INIT", DB_INIT);
			else IFSETDB (cp, "ABORT", DB_ABORT);
			else IFSETDB (cp, "SCHEDLOG", DB_SCHEDLOG);
			else IFSETDB (cp, "SDB", DB_SDB);
			else IFSETDB (cp, "MESSAGES", DB_MESSAGES);
			else IFSETDB (cp, "MALLOC", DB_MALLOC);
			else IFSETDB (cp, "ALL", DB_ALL);
			else {
				note ("-D flag not recognized; try -D?\n");
				exit (1);
			}
			optarg = 0;
		}
SkipD:
		SET_DEBUG_PATH ("/tmp/lpsched.debug")
		OPEN_DEBUG_FILE ("/tmp/lpsched.debug")

#   if	defined(TRACE_MALLOC)
		if (debug & DB_MALLOC) {
#if	defined(__STDC__)
			extern void	(*mdl_logger)( char * , ... );
#else
			extern void	(*mdl_logger)();
#endif
			mdl_logger = note;
		}
#   endif
		break;

	    case 's':
		signals++;
		break;
# endif /* DEBUG */

	    case 'f':
		if ((ET_SlowSize = atoi(optarg)) < 1)
		    ET_SlowSize = 1;
		break;

	    case 'n':
		if ((ET_NotifySize = atoi(optarg)) < 1)
		    ET_NotifySize = 1;
		break;

	    case 'r':
		if ((Reserve_Fds = atoi(optarg)) < 1)
		    Reserve_Fds = 0;
		break;

#if	defined(MALLOC_3X)
	    case 'M':
		{
			int			value;

			value = atoi(optarg);
			printf ("M_MXFAST set to %d\n", value);
			mallopt (M_MXFAST, value);

			value = atoi(argv[optind++]);
			printf ("M_NLBLKS set to %d\n", value);
			mallopt (M_NLBLKS, value);

			value = atoi(argv[optind++]);
			printf ("M_GRAIN set to %d\n", value);
			mallopt (M_GRAIN, value);
		}
		break;
#endif

	    case '?':
		if (optopt == '?') {
		    usage ();
		    exit (0);
		} else
		    fail ("%s: illegal option -- %c\n", argv[0], optopt);
	}
    
    lp_alloc_fail_handler = mallocfail;

    startup();

    process();

    lpshut(1);	/* one last time to clean up */
    /*NOTREACHED*/
}

static void
startup()
{
    ENTRY ("startup")

    struct passwd		*p;
    struct utsname		utsbuf;

    
    Starting = 1;
    getpaths();

    /*
     * There must be a user named "lp".
     */
    if ((p = lp_getpwnam(LPUSER)) == NULL)
	fail ("Can't find the user \"lp\" on this system!\n");
    lp_endpwent();
    
    Lp_Uid = p->pw_uid;
    Lp_Gid = p->pw_gid;

    /*
     * Only "root" and "lp" are allowed to run us.
     */
    if (getuid() && getuid() != Lp_Uid)
	fail ("You must be \"lp\" or \"root\" to run this program.\n");

    setuid (0);

    uname(&utsbuf);
    Local_System = Strdup(utsbuf.nodename);

    /*
     * Make sure that all critical directories are present and that 
     * symbolic links are correct.
     */
    lpfsck();
    
    /*
     * Try setting the lock file to see if another Spooler is running.
     * We'll release it immediately; this allows us to fork the child
     * that will run in the background. The child will relock the file.
     */
    if (!(LockFp = open_lpfile(Lp_Schedlock, "a", 0664)))
	if (errno == EAGAIN)
	    fail ("Print services already active.\n");
	else
	    fail ("Can't open file \"%s\" (%s).\n", NB(Lp_Schedlock), PERROR);
    close_lpfile(LockFp);

    background();
    /*
     * We are the child process now.
#if	defined(DEBUG)
     * (That is, unless the debug flag is set.)
#endif
     */

    if (!(LockFp = open_lpfile(Lp_Schedlock, "w", 0664)))
	fail ("Failed to lock the file \"%s\" (%s).\n", NB(Lp_Schedlock), PERROR);

    Close (0);
    Close (2);
    if (am_in_background)
	Close (1);

    if ((OpenMax = ulimit(4, 0L)) == -1)
	OpenMax = OPEN_MAX;

    disable_signals();

    init_messages();

    init_network();

    init_memory();

    note ("Print services started.\n");
    Starting = 0;
}

void
#if	defined(__STDC__)
lpshut (
	int			immediate
)
#else
lpshut (immediate)
	int			immediate;
#endif
{
	ENTRY ("lpshut")

	int			i;

	extern MESG *		Net_md;


	/*
	 * If this is the first time here, stop all running
	 * child processes, and shut off the alarm clock so
	 * it doesn't bug us.
	 */
	if (!Shutdown) {
		mputm (Net_md, S_SHUTDOWN, 1);
		for (i = 0; i < ET_Size; i++)
			terminate (&Exec_Table[i]);
		alarm (0);
		Shutdown = (immediate? 2 : 1);
	}

	/*
	 * If this is an express shutdown, or if all the
	 * child processes have been cleaned up, clean up
	 * and get out.
	 */
	if (Shutdown == 2) {

		/*
		 * We don't shut down the message queues until
		 * now, to give the children a chance to answer.
		 * This means an LP command may have been snuck
		 * in while we were waiting for the children to
		 * finish, but that's OK because we'll have
		 * stored the jobs on disk (that's part of the
		 * normal operation, not just during shutdown phase).
		 */
		shutdown_messages();
    
		(void) close_lpfile(LockFp);
		(void) Unlink(Lp_Schedlock);

		note ("Print services stopped.\n");
		exit (0);
		/*NOTREACHED*/
	}
}

static void
process()
{
    ENTRY ("process")

    register FSTATUS	*pfs;
    register PWSTATUS	*ppws;


    /*
     * Call the "check_..._alert()" routines for each form/print-wheel;
     * we need to do this at this point because these routines
     * short-circuit themselves while we are in startup mode.
     * Calling them now will kick off any necessary alerts.
     */
    for (pfs = walk_ftable(1); pfs; pfs = walk_ftable(0))
	check_form_alert (pfs, (_FORM *)0);
    for (ppws = walk_pwtable(1); ppws; ppws = walk_pwtable(0))
	check_pwheel_alert (ppws, (PWHEEL *)0);
    
    /*
     * Clear the alarm, then schedule an EV_ALARM. This will clear
     * all events that had been scheduled for later without waiting
     * for the next tick.
     */
    alarm (0);
    schedule (EV_ALARM);

    /*
     * Start the ball rolling.
     */
    schedule (EV_INTERF, (PSTATUS *)0);
    schedule (EV_NOTIFY, (RSTATUS *)0);
    schedule (EV_SLOWF, (RSTATUS *)0);

#if	defined(CHECK_CHILDREN)
    schedule (EV_CHECKCHILD);
#endif

    for (EVER)
    {
	RESTART_ENTRY

	take_message ();

	if (Sig_Alrm)
		schedule (EV_ALARM);

	if (DoneChildren)
		dowait ();

	if (Shutdown)
		check_children();
	if (Shutdown == 2)
		break;
    }
}

static void
#if	defined(__STDC__)
ticktock (
	int			sig
)
#else
ticktock(sig)
	int			sig;
#endif
{
	ENTRY ("ticktock")

	Sig_Alrm = 1;
	(void)signal (SIGALRM, ticktock);
	return;
}
			    
static void
background()
{
    ENTRY ("background")

#if	defined(DEBUG)
    if (debug & DB_SDB)
	return;
#endif
    
    switch(fork())
    {
	case -1:
	    fail ("Failed to fork child process (%s).\n", PERROR);
	    /*NOTREACHED*/

	case 0:
	    (void) setpgrp();
	    am_in_background = 1;
	    return;
	    
	default:
	    note ("Print services started.\n");
	    exit(0);
	    /* NOTREACHED */
    }
}

static void
usage()
{
	ENTRY ("usage")

	note ("\
usage: lpsched [ options ]\n\
    [ -f #filter-slots ]    (increase no. concurrent slow filters)\n\
    [ -n #notify-slots ]    (increase no. concurrent notifications)\n\
    [ -r #reserved-fds ]    (increase margin of file descriptors)\n"
	);

#if	defined(DEBUG)
	note ("\
    [ -D flag[,flag...] ]   (debug modes; use -D? for usage info.)\n\
    [ -d ]                  (same as -D ALL)\n\
    [ -s ]                  (don't trap most signals)\n"
	);
#endif

#if	defined(MALLOC_3X)
	note ("\
    [ -M \"M_MXFAST M_NLBLKS M_GRAIN\" ]\n\
                            (set these malloc(3X) parameters)\n"
	);
#endif

	note ("\
WARNING: all these options are currently unsupported\n"
	);

	return;
}

static void
Exit(n)
    int		n;
{
    ENTRY ("Exit")

    fail ("Received unexpected signal %d; terminating.\n", n);
}

static void
disable_signals()
{
    ENTRY ("disable_signals")

    int		i;

# if defined(DEBUG)
    if (!signals)
# endif
	for (i = 0; i < NSIG; i++)
		if (signal(i, SIG_IGN) != SIG_IGN)
			signal (i, Exit);
    
    (void) signal(SIGHUP, SIG_IGN);
    (void) signal(SIGINT, SIG_IGN);
    (void) signal(SIGQUIT, SIG_IGN);
    (void) signal(SIGALRM, ticktock);
    (void) signal(SIGTERM, lpshut);	/* needs arg, but sig# OK */
    (void) signal(SIGCLD, SIG_IGN);
    (void) signal(SIGTSTP, SIG_IGN);
    (void) signal(SIGCONT, SIG_DFL);
    (void) signal(SIGTTIN, SIG_IGN);
    (void) signal(SIGTTOU, SIG_IGN);
    (void) signal(SIGXFSZ, SIG_IGN);	/* could be a problem */

#if	defined(DEBUG)
    if (debug & DB_ABORT)
	(void) signal(SIGABRT, SIG_DFL);
#endif

#if	defined(TRACE_MALLOC)
    if (debug & DB_MALLOC) {
# if	defined(__STDC__)
	static void		sigusr1( int );
# else
	static void		sigusr1();
# endif
	(void) signal(SIGUSR1, sigusr1);
    }
#endif
}

#if	defined(TRACE_MALLOC)
static void
# if	defined(__STDC__)
sigusr1 ( int sig )
# else
sigusr1()
# endif
{
	(void) signal(SIGUSR1, sigusr1);
	mdl_dump ();
}
#endif
