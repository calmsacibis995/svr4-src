/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Copyright (c) 1981 Regents of the University of California */
#ident	"@(#)vi:port/ex.c	1.30"
#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include <stdlib.h>
#include <locale.h>
#ifdef TRACE
unsigned char	tttrace[]	= { '/','d','e','v','/','t','t','y','x','x',0 };
#endif

#define EQ(a,b)	!strcmp(a,b)

char	*strrchr();

/*
 * The code for ex is divided as follows:
 *
 * ex.c			Entry point and routines handling interrupt, hangup
 *			signals; initialization code.
 *
 * ex_addr.c		Address parsing routines for command mode decoding.
 *			Routines to set and check address ranges on commands.
 *
 * ex_cmds.c		Command mode command decoding.
 *
 * ex_cmds2.c		Subroutines for command decoding and processing of
 *			file names in the argument list.  Routines to print
 *			messages and reset state when errors occur.
 *
 * ex_cmdsub.c		Subroutines which implement command mode functions
 *			such as append, delete, join.
 *
 * ex_data.c		Initialization of options.
 *
 * ex_get.c		Command mode input routines.
 *
 * ex_io.c		General input/output processing: file i/o, unix
 *			escapes, filtering, source commands, preserving
 *			and recovering.
 *
 * ex_put.c		Terminal driving and optimizing routines for low-level
 *			output (cursor-positioning); output line formatting
 *			routines.
 *
 * ex_re.c		Global commands, substitute, regular expression
 *			compilation and execution.
 *
 * ex_set.c		The set command.
 *
 * ex_subr.c		Loads of miscellaneous subroutines.
 *
 * ex_temp.c		Editor buffer routines for main buffer and also
 *			for named buffers (Q registers if you will.)
 *
 * ex_tty.c		Terminal dependent initializations from termcap
 *			data base, grabbing of tty modes (at beginning
 *			and after escapes).
 *
 * ex_unix.c		Routines for the ! command and its variations.
 *
 * ex_v*.c		Visual/open mode routines... see ex_v.c for a
 *			guide to the overall organization.
 */

/*
 * This sets the Version of ex/vi for both the exstrings file and
 * the version command (":ver"). 
 */

unsigned char *Version = (unsigned char *)"Version SVR4.0";	/* variable used by ":ver" command */

/*
 * NOTE: when changing the Version number, it must be changed in the
 *  	 following files:
 *
 *			  port/READ_ME
 *			  port/ex.c
 *			  port/ex.news
 *
 */

/*
 * Main procedure.  Process arguments and then
 * transfer control to the main command processing loop
 * in the routine commands.  We are entered as either "ex", "edit", "vi"
 * or "view" and the distinction is made here. For edit we just diddle options;
 * for vi we actually force an early visual command.
 */
static unsigned char cryptkey[19]; /* contents of encryption key */

main(ac, av)
	register int ac;
	register unsigned char *av[];
{
	extern 	char 	*optarg;
	extern 	int	optind;
	unsigned char	*rcvname = 0;
	unsigned char	usage[80];
	register unsigned char *cp;
	register int c;
	unsigned char	*cmdnam;
	bool recov = 0;
	bool ivis = 0;
	bool itag = 0;
	bool fast = 0;
	extern int verbose;
#ifdef TRACE
	register unsigned char *tracef;
#endif
	(void)setlocale(LC_ALL, "");

	/*
	 * Immediately grab the tty modes so that we won't
	 * get messed up if an interrupt comes in quickly.
	 */
	gTTY(2);
	normf = tty;
	ppid = getpid();
	/* Note - this will core dump if you didn't -DSINGLE in CFLAGS */
	lines = 24;
	columns = 80;	/* until defined right by setupterm */
	/*
	 * Defend against d's, v's, w's, and a's in directories of
	 * path leading to our true name.
	 */
	if ((cmdnam = (unsigned char *)strrchr(av[0], '/')) != 0)
		cmdnam++;
	else
		cmdnam = av[0];

	if (EQ(cmdnam, "vi")) 
		ivis = 1;
	else if (EQ(cmdnam, "view")) {
		ivis = 1;
		value(vi_READONLY) = 1;
	} else if (EQ(cmdnam, "vedit")) {
		ivis = 1;
		value(vi_NOVICE) = 1;
		value(vi_REPORT) = 1;
		value(vi_MAGIC) = 0;
		value(vi_SHOWMODE) = 1;
	} else if (EQ(cmdnam, "edit")) {
		value(vi_NOVICE) = 1;
		value(vi_REPORT) = 1;
		value(vi_MAGIC) = 0;
		value(vi_SHOWMODE) = 1;
	}

	draino();
	pstop();

	/*
	 * Initialize interrupt handling.
	 */
	oldhup = signal(SIGHUP, SIG_IGN);
	if (oldhup == SIG_DFL)
		signal(SIGHUP, onhup);
	oldquit = signal(SIGQUIT, SIG_IGN);
	ruptible = signal(SIGINT, SIG_IGN) == SIG_DFL;
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
		signal(SIGTERM, onhup);
	if (signal(SIGEMT, SIG_IGN) == SIG_DFL)
		signal(SIGEMT, onemt);
	signal(SIGILL, oncore);
	signal(SIGTRAP, oncore);
	signal(SIGIOT, oncore);
	signal(SIGFPE, oncore);
	signal(SIGBUS, oncore);
	signal(SIGSEGV, oncore);
	signal(SIGPIPE, oncore);
	while(1) {
#ifdef TRACE
		while ((c = getopt(ac,av,"VS:Lc:Tvt:rlw:xRCs")) != EOF)
#else
		while ((c = getopt(ac,(char **)av,"VLc:vt:rlw:xRCs")) != EOF)
#endif
			switch(c) {
			case 's':
				hush = 1;
				value(vi_AUTOPRINT) = 0;
				fast++;
				break;
				
			case 'R':
				value(vi_READONLY) = 1;
				break;
#ifdef TRACE
			case 'T':
				tracef = (unsigned char *)"trace";
				goto trace;
			
			case 'S':
				tracef = tttrace;
		trace:			
				strcat(tracef,optarg);	
				trace = fopen(tracef,"w");
#define	tracebuf	NULL
				if (trace == NULL)
					printf("Trace create error\n");
				else
					setbuf(trace, (char *)tracbuf);
				break;
#endif
			case 'c':
				firstpat = (unsigned char *)optarg;
					break;
				
			case 'l':
				value(vi_LISP) = 1;
				value(vi_SHOWMATCH) = 1;
				break;

			case 'r':
				if(av[optind] && (c = av[optind][0]) && c != '-') {
					rcvname = (unsigned char *)av[optind];
					optind++;
				}
		
			case 'L':
				recov++;
				break;

			case 'V':
				verbose = 1;
				break;

			case 't':
				itag = 1;
				CP(lasttag, optarg);
				break;
					
			case 'w':
				defwind = 0;
				if (optarg[0] == NULL)
					defwind = 3;
				else for (cp = (unsigned char *)optarg; isdigit(*cp); cp++)
					defwind = 10*defwind + *cp - '0';
				break;
					
			case 'C':
				crflag = 1;
				xflag = 1;
				break;

			case 'x':
				/* 
				 * encrypted mode
				 */
				xflag = 1;
				crflag = -1;
				break;
				
			case 'v':
				ivis = 1;
				break;

			default:
#ifdef TRACE
				sprintf((char *)usage,"Usage: %s [- | -s] [-l] [-L] [-R] [-r [file]] [-t tag] [-T [-S suffix]]\n", cmdnam);
#else
				sprintf((char *)usage,"Usage: %s [- | -s] [-l] [-L] [-R] [-r [file]] [-t tag]\n", cmdnam);
#endif
				write(2,(char *)usage,strlen((char *)usage));
				sprintf((char *)usage, "       [-v] [-V] [-x] [-C] [+cmd | -c cmd] file...\n");
				write(2, (char *)usage, strlen((char *)usage));
				exit(1);
			}
		if(av[optind] && av[optind][0] == '+' && strcmp(av[optind-1],"--")) {
				firstpat = &av[optind][1];
				optind++;
				continue;
		} else if(av[optind] && av[optind][0] == '-'  && strcmp(av[optind-1], "--")) {
			hush = 1;
			value(vi_AUTOPRINT) = 0;
			fast++;
			optind++;
			continue;
		}
		break;
	}
	ac -= optind;
	av  = &av[optind];
					
#ifdef SIGTSTP
	if (!hush && signal(SIGTSTP, SIG_IGN) == SIG_DFL)
		signal(SIGTSTP, onsusp), dosusp++;
#endif

	if(xflag) {
		permflag = 1;
		if ((kflag = run_setkey(perm, (key = (unsigned char *)getpass("Enter key:")))) == -1) {
			kflag = 0;
			xflag = 0;
			smerror("Encryption facility not available\n");
		}
		if(kflag == 0)
			crflag = 0;
		else {
			strcpy(cryptkey, "CrYpTkEy=XXXXXXXXX");
			strcpy(cryptkey + 9, key);
			if(putenv((char *)cryptkey) != 0)
			smerror(" Cannot copy key to environment");
		}

	}

	/*
	 * Initialize end of core pointers.
	 * Normally we avoid breaking back to fendcore after each
	 * file since this can be expensive (much core-core copying).
	 * If your system can scatter load processes you could do
	 * this as ed does, saving a little core, but it will probably
	 * not often make much difference.
	 */
	fendcore = (line *) sbrk(0);
	endcore = fendcore - 2;
	
	/*
	 * If we are doing a recover and no filename
	 * was given, then execute an exrecover command with
	 * the -r option to type out the list of saved file names.
	 * Otherwise set the remembered file name to the first argument
	 * file name so the "recover" initial command will find it.
	 */
	if (recov) {
		if (ac == 0 && (rcvname == NULL || *rcvname == NULL)) {
			ppid = 0;
			setrupt();
			execlp(EXRECOVER, "exrecover", "-r", (char *)0);
			filioerr(EXRECOVER);
			exit(++errcnt);
		}
		if (rcvname && *rcvname)
			CP(savedfile, rcvname);
		else
			CP(savedfile, *av++), ac--;
	}

	/*
	 * Initialize the argument list.
	 */
	argv0 = av;
	argc0 = ac;
	args0 = av[0];
	erewind();

	/*
	 * Initialize a temporary file (buffer) and
	 * set up terminal environment.  Read user startup commands.
	 */
	if (setexit() == 0) {
		setrupt();
		intty = isatty(0);
		value(vi_PROMPT) = intty;
		if (cp = (unsigned char *)getenv("SHELL"))
			CP(shell, cp);
		if (fast)
			setterm("dumb");
		else {
			gettmode();
			cp = (unsigned char *)getenv("TERM");
			if (cp == NULL || *cp == '\0')
				cp = (unsigned char *)"unknown";
			setterm(cp);
		}
	}
	if (setexit() == 0 && !fast) {
		if ((globp = (unsigned char *)getenv("EXINIT")) && *globp)
			commands(1,1);
		else {
			globp = 0;
			if ((cp = (unsigned char *)getenv("HOME")) != 0 && *cp)
				source(strcat(strcpy(genbuf, cp), "/.exrc"), 1);
		}
		/*
		 * Allow local .exrc if the "exrc" option was set. This
		 * loses if . is $HOME, but nobody should notice unless
		 * they do stupid things like putting a version command
		 * in .exrc.
		 * Besides, they should be using EXINIT, not .exrc, right?
		 */
		if (value(vi_EXRC))
			source(".exrc", 1);
	}
	init();	/* moved after prev 2 chunks to fix directory option */

	/*
	 * Initial processing.  Handle tag, recover, and file argument
	 * implied next commands.  If going in as 'vi', then don't do
	 * anything, just set initev so we will do it later (from within
	 * visual).
	 */
	if (setexit() == 0) {
		if (recov)
			globp = (unsigned char *)"recover";
		else if (itag)
			globp = ivis ? (unsigned char *)"tag" : (unsigned char *)"tag|p";
		else if (argc)
			globp = (unsigned char *)"next";
		if (ivis)
			initev = globp;
		else if (globp) {
			inglobal = 1;
			commands(1, 1);
			inglobal = 0;
		}
	}

	/*
	 * Vi command... go into visual.
	 */
	if (ivis) {
		/*
		 * Don't have to be upward compatible 
		 * by starting editing at line $.
		 */
		if (dol > zero)
			dot = one;
		globp = (unsigned char *)"visual";
		if (setexit() == 0)
			commands(1, 1);
	}

	/*
	 * Clear out trash in state accumulated by startup,
	 * and then do the main command loop for a normal edit.
	 * If you quit out of a 'vi' command by doing Q or ^\,
	 * you also fall through to here.
	 */
	seenprompt = 1;
	ungetchar(0);
	globp = 0;
	initev = 0;
	setlastchar('\n');
	setexit();
	commands(0, 0);
	cleanup(1);
	exit(errcnt);
}

/*
 * Initialization, before editing a new file.
 * Main thing here is to get a new buffer (in fileinit),
 * rest is peripheral state resetting.
 */
init()
{
	register int i;
	void (*pstat)();
	fileinit();
	dot = zero = truedol = unddol = dol = fendcore;
	one = zero+1;
	undkind = UNDNONE;
	chng = 0;
	edited = 0;
	for (i = 0; i <= 'z'-'a'+1; i++)
		names[i] = 1;
	anymarks = 0;
        if(xflag) {
                xtflag = 1;
                /* ignore SIGINT before crypt process */
		pstat = signal(SIGINT, SIG_IGN);
		if(tpermflag)
			(void)crypt_close(tperm);
		tpermflag = 1;
		if (makekey(tperm) != 0) {
			xtflag = 0;
			smerror("Warning--Cannot encrypt temporary buffer\n");
        	}
		signal(SIGINT, pstat);
	}
}

/*
 * Return last component of unix path name p.
 */
unsigned char *
tailpath(p)
register unsigned char *p;
{
	register unsigned char *r;

	for (r=p; *p; p++)
		if (*p == '/')
			r = p+1;
	return(r);
}
