/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:main.c	1.5.3.1"

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/fstyp.h>
#include <sys/fsid.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/param.h>

#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "proto.h"

/* This module is carefully coded to contain only read-only data */
/* All read/write data is defined in ramdata.c (see also ramdata.h) */

extern VOID * malloc();
extern VOID * realloc();
extern void free();
extern VOID * shmat();
extern void exit();
extern void perror();
extern unsigned alarm();
extern unsigned short getuid();
extern unsigned short getgid();
extern unsigned short geteuid();
extern unsigned short getegid();

extern time_t times();
extern long strtol();

extern int getopt();
extern char * optarg;
extern int optind;

/*
 * Function prototypes for static routines in this module.
 */
#if	defined(__STDC__)

static	int	xcreat( char * );
static	void	setoutput( int );
static	void	isptraced();
static	void	report( time_t );
static	void	prtim( timestruc_t * );
static	int	pids( char * );
static	void	psargs( process_t * );
static	int	control( process_t * , pid_t );
static	int	grabit( process_t * , char * , pid_t );
static	void	release( pid_t );
static	void	intr( int );
static	void	ssys( int );
static	int	wait4all();
static	void	letgo( process_t * );
static	int	prstat( process_t * , char * , struct stat * );

#else	/* defined(__STDC__) */

static	int	xcreat();
static	void	setoutput();
static	void	isptraced();
static	void	report();
static	void	prtim();
static	int	pids();
static	void	psargs();
static	int	control();
static	int	grabit();
static	void	release();
static	void	intr();
static	void	ssys();
static	int	wait4all();
static	void	letgo();
static	int	prstat();

#endif	/* defined(__STDC__) */

main(argc, argv)
	int argc;
	char **argv;
{
	struct tms tms;
	time_t starttime;
	int retc;
	int ofd = -1;
	int opt;
	int i;
	int what;
	int errflg = FALSE;
	int badname = FALSE;
	int req_flag = 0;
	register process_t *Pr = &Proc;

	/* make sure fd's 0, 1, and 2 are allocated */
	/* just in case truss was invoked from init */
	while ((i = open("/dev/null", O_RDWR)) >= 0 && i < 2)
		;
	if (i > 2)
		(void) close(i);

	starttime = times(&tms);	/* for elapsed timing */

	/* command name (e.g., "truss") */
	if ((command = strrchr(argv[0], '/')) != NULL)
		command++;
	else
		command = argv[0];

	Euid = geteuid();
	Egid = getegid();
	Ruid = getuid();
	Rgid = getgid();
	ancestor = getpid();

	prfillset(&trace);	/* default: trace all system calls */
	premptyset(&verbose);	/* default: no syscall verbosity */
	premptyset(&rawout);	/* default: no raw syscall interpretation */

	prfillset(&signals);	/* default: trace all signals */

	prfillset(&faults);	/* default: trace all faults */
	prdelset(&faults, FLTPAGE);	/* except this one */

	premptyset(&readfd);	/* default: dump no buffers */
	premptyset(&writefd);

	/* options */
	while ((opt = getopt(argc, argv,
	    (CONST char *)"dP:Fqpfcaeit:v:x:s:m:r:w:o:")) != EOF) {
		switch (opt) {
		case 'd':		/* debug */
			debugflag = TRUE;
			break;
		case 'P':		/* alternate /proc directory */
			procdir = optarg;
			break;
		case 'F':		/* force grabbing (no O_EXCL) */
			Fflag = TRUE;
			break;
		case 'q':		/* die on QUIT regardless */
			qflag = TRUE;
			break;
		case 'p':		/* grab processes */
			pflag = TRUE;
			break;
		case 'f':		/* follow children */
			fflag = TRUE;
			break;
		case 'c':		/* don't trace, just count */
			cflag = TRUE;
			iflag = TRUE;	/* implies no interruptable syscalls */
			break;
		case 'a':		/* display argument lists */
			aflag = TRUE;
			break;
		case 'e':		/* display environments */
			eflag = TRUE;
			break;
		case 'i':		/* don't show interruptable syscalls */
			iflag = TRUE;
			break;
		case 't':		/* system calls to trace */
			if (syslist(optarg, &trace, &tflag))
				badname = TRUE;
			break;
		case 'v':		/* verbose interpretation of syscalls */
			if (syslist(optarg, &verbose, &vflag))
				badname = TRUE;
			break;
		case 'x':		/* raw interpretation of syscalls */
			if (syslist(optarg, &rawout, &xflag))
				badname = TRUE;
			break;
		case 's':		/* signals to trace */
			if (siglist(optarg, &signals, &sflag))
				badname = TRUE;
			break;
		case 'm':		/* machine faults to trace */
			if (fltlist(optarg, &faults, &mflag))
				badname = TRUE;
			break;
		case 'r':		/* show contents of read(fd) */
			if (fdlist(optarg, &readfd))
				badname = TRUE;
			break;
		case 'w':		/* show contents of write(fd) */
			if (fdlist(optarg, &writefd))
				badname = TRUE;
			break;
		case 'o':		/* output file for trace */
			oflag = TRUE;
			if (ofd >= 0)
				(void) close(ofd);
			if ((ofd = xcreat(optarg)) < 0) {
				perror(optarg);
				badname = TRUE;
			}
			break;
		default:
			errflg = TRUE;
			break;
		}
	}

	/* if -a or -e was specified, force tracing of exec() */
	if (aflag || eflag) {
		praddset(&trace, SYS_exec);
		praddset(&trace, SYS_execve);
	}

	argc -= optind;
	argv += optind;

	/* collect the specified process ids */
	if (pflag) {
		while (argc > 0) {
			if (pids(*argv))
				badname = TRUE;
			argc--;
			argv++;
		}
	}

	if (badname)
		exit(2);

	if (errflg || (argc <= 0 && ngrab <= 0)) {
		(void) fprintf(stderr,
"usage:\t%s [-fcaei] [-[tvx] [!]syscalls] [-s [!]signals] \\\n",
			command);
		(void) fprintf(stderr,
"\t[-m [!]faults] [-[rw] [!]fds] [-o outfile]  command | -p pid ...\n");
		exit(2);
	}

	if (!isprocdir((process_t *)NULL, procdir)) {
		(void) fprintf(stderr, "%s: %s is not a PROC directory\n",
			command, procdir);
		exit(2);
	}

	if ((sys_path = (char *)malloc(sys_psize = 16)) == NULL)
		abend("cannot allocate memory for syscall pathname", 0);
	if ((sys_string = (char *)malloc(sys_ssize = 32)) == NULL)
		abend("cannot allocate memory for syscall string", 0);

	if (argc > 0) {		/* create the controlled process */
		if (Pcreate(Pr, &argv[0]) != 0
		 || Pr->state != PS_STOP) {
			(void) Pterm(Pr);
			abend("cannot create subject process", 0);
		}
		if (fflag && !cflag)
			(void) sprintf(pname, "%d:\t", Pr->upid);
		no_inherit = Ioctl(Pr->pfd, fflag? PIOCSFORK : PIOCRFORK, 0);
		created = Pr->upid;
		PR = Pr;	/* for abend() */
	}

	setoutput(ofd);		/* establish truss output */
	istty = isatty(1);

#ifndef NOSETVBUF
	if (setvbuf(stdout, (char *)NULL, _IOFBF, BUFSIZ) != 0) {
		abend("setvbuf() failure", 0);
		exit(2);
	}
#else
	{
		extern unsigned char _sobuf[];
		setbuf(stdout, (char *)_sobuf);
		stdout->_flag &= ~_IOLBF;
	}
#endif
	if (stdout->_cnt == 0)		/* stdio bug, compensate */
		stdout->_cnt = BUFSIZ;

	/*
	 * Set up signal dispositions.
	 */
	if (created && (oflag || !istty)) {	/* ignore interrupts */
		(void) sigset(SIGHUP, SIG_IGN);
		(void) sigset(SIGINT, SIG_IGN);
		(void) sigset(SIGQUIT, SIG_IGN);
	}
	else {					/* receive interrupts */
		if (sigset(SIGHUP, SIG_IGN) == SIG_DFL)
			(void) sigset(SIGHUP, intr);
		if (sigset(SIGINT, SIG_IGN) == SIG_DFL)
			(void) sigset(SIGINT, intr);
		if (sigset(SIGQUIT, SIG_IGN) == SIG_DFL)
			(void) sigset(SIGQUIT, intr);
	}
	if (qflag)	/* -q : die on SIGQUIT with a core dump */
		(void) sigset(SIGQUIT, SIG_DFL);
	(void) sigset(SIGALRM, intr);
	(void) sigset(SIGTERM, intr);
	(void) sigset(SIGUSR1, intr);
	(void) sigset(SIGPIPE, intr);

	if (fflag || ngrab > 1) {	/* multiple processes */
		void (*f)() = sigset(SIGSYS, ssys);	/* catch SIGSYS */

		sigsys = FALSE;
		shmid = shmget(IPC_PRIVATE, sizeof(struct counts), 0600);
		if (sigsys) {		/* stupid workaround */
			(void) fprintf(stderr,
		"%s: *** shared memory is not configured in this system ***\n",
				command);
			shmid = -1;
		}
		else if (shmid == -1)
			perror("cannot get shared memory");
		else {
			Cp = (struct counts *)shmat(shmid, (char *)NULL, 0);
			if ((int)Cp == -1) {
				perror("cannot attach shared memory");
				(void) shmctl(shmid, IPC_RMID,
					(struct shmid_ds *)0);
				Cp = (struct counts *)NULL;
				shmid = -1;
			}
		}

		sigsys = FALSE;
		semid = semget(IPC_PRIVATE, 2, 0600);
		if (sigsys) {		/* stupid workaround */
			(void) fprintf(stderr,
		"%s: *** semaphores are not configured in this system ***\n",
				command);
			semid = -1;
		}
		else if (semid == -1)
			perror("cannot get semaphores");
		else {
			if (semctl(semid, 0, SETVAL, 1) == -1
			 || semctl(semid, 1, SETVAL, 1) == -1) {
				perror("cannot set semaphores");
				(void) semctl(semid, 0, IPC_RMID, 0);
				semid = -1;
			}
		}

		(void) sigset(SIGSYS, f);	/* restore SIGSYS handler */
	}
	if (Cp == (struct counts *)NULL)
		Cp = (struct counts *)malloc(sizeof(struct counts));
	if (Cp == (struct counts *)NULL)
		abend("cannot allocate memory for counts", 0);

	(void) memset((char *)Cp, 0, sizeof(struct counts));
	if (istty)
		Cp->serialize = 1;

	if (created) {
		procadd(Pr->upid);
		show_cred(Pr, TRUE);
	}
	else {			/* grab the specified processes */
		int gotone = FALSE;

		i = 0;
		while (i < ngrab) {		/* grab first process */
			char * dir = grabdir[i];
			pid_t pid = grab[i++];

			if (grabit(Pr, dir, pid)) {
				gotone = TRUE;
				break;
			}
		}
		if (!gotone)
			abend(0, 0);
		while (i < ngrab) {		/* grab the remainder */
			char * dir = grabdir[i];
			pid_t pid = grab[i++];

			switch (fork()) {
			case -1:
				(void) fprintf(stderr,
			"%s: cannot fork to control process, pid# %d\n",
					command, pid);
			default:
				continue;	/* parent carries on */

			case 0:			/* child grabs process */
				(void) close(Pr->pfd);
				descendent = TRUE;
				if (grabit(Pr, dir, pid))
					break;
				exit(2);
			}
			break;
		}
		PR = Pr;	/* for abend() */
	}

	/*
	 * If running setuid-root, become root for real to avoid
	 * affecting the per-user limitation on the maximum number
	 * of processes (one benefit of running setuid-root).
	 */
	if (Rgid != Egid)
		(void) setgid(Egid);
	if (Ruid != Euid)
		(void) setuid(Euid);

	/* catch these syscalls in order to get started */
	(void) Psysentry(Pr, SYS_exit, TRUE);
	(void) Psysentry(Pr, SYS_exec, TRUE);
	(void) Psysentry(Pr, SYS_execve, TRUE);
	(void) Psysexit(Pr, SYS_exec, TRUE);
	(void) Psysexit(Pr, SYS_execve, TRUE);

	while (created
	 && Pr->state == PS_STOP
	 && Pr->why.pr_why == PR_SYSENTRY
	 && (Pr->why.pr_what == SYS_execve || Pr->why.pr_what == SYS_exec)) {
		(void) sysentry(Pr);
		(void) Pstart(Pr, 0);
		(void) Pwait(Pr);
		while (Pr->state == PS_LOST) {	/* we lost control */
			if (Preopen(Pr, Fflag) == 0)	/* we got it back */
				continue;

			/* we really lost it */
			if (sys_valid)
				(void) fprintf(stderr,
		"%s: cannot trace set-id or unreadable object file: %s\n",
					command, sys_path);
			else
				(void) fprintf(stderr,
				"%s: lost control of process\n",
					command);
			(void) Pterm(Pr);
			goto done;
		}
		if (Pr->state == PS_STOP
		 && Pr->why.pr_why == PR_SYSEXIT
		 && (Pr->why.pr_what==SYS_execve || Pr->why.pr_what==SYS_exec)) {
			int ps;

			(void) Pgetareg(Pr, R_PS);
			ps = Pr->REG[R_PS];
			if (ps & ERRBIT) {	/* exec() failed */
				(void) Pstart(Pr, 0);
				(void) Pwait(Pr);
				continue;
			}
		}

		/* successful exec() */
		length = 0;
		if (!cflag
		 && prismember(&trace,SYS_execve)
		 && Pr->state == PS_STOP) {
			if (pname[0])
				(void) fputs(pname, stdout);
			length += printf("%s", sys_string);
			sys_leng = 0;
			*sys_string = '\0';
		}
		sysbegin = syslast = Pr->why.pr_stime;
		usrbegin = usrlast = Pr->why.pr_utime;
		break;
	}

	if (created
	 && Pr->state == PS_STOP
	 && Pr->why.pr_why == PR_SYSENTRY
	 && Pr->why.pr_what == SYS_exit) {
		(void) Pstart(Pr, 0);
		abend("Cannot find program: ", argv[0]);
	}

	if (!created && aflag && prismember(&trace,SYS_execve)) {
		psargs(Pr);
		Flush();
	}

	PR = Pr;	/* for abend() */

	if (created && Pr->state != PS_STOP)	/* assertion */
		if (!(interrupt || sigusr1))
			abend("ASSERT error: process is not stopped", 0);

	Pr->sysexit = trace;		/* trace these system calls */
	Pr->setexit = TRUE;
	traceeven = trace;

	/* special case -- cannot trace sysexit because context is changed */
	if (prismember(&trace, SYS_context)) {
		(void) Psysentry(Pr, SYS_context, TRUE);
		(void) Psysexit(Pr, SYS_context, FALSE);
		prdelset(&traceeven, SYS_context);
	}

	/* special case -- sysexit not traced by OS */
	if (prismember(&trace, SYS_evtrapret)) {
		(void) Psysentry(Pr, SYS_evtrapret, TRUE);
		(void) Psysexit(Pr, SYS_evtrapret, FALSE);
		prdelset(&traceeven, SYS_evtrapret);
	}

	/* trace these regardless, even if we don't report results */
	(void) Psysexit(Pr, SYS_exec,   TRUE);
	(void) Psysexit(Pr, SYS_execve, TRUE);
	(void) Psysexit(Pr, SYS_open,   TRUE);
	(void) Psysexit(Pr, SYS_fork,   TRUE);
	(void) Psysexit(Pr, SYS_vfork,  TRUE);
	praddset(&traceeven, SYS_exec);
	praddset(&traceeven, SYS_execve);
	praddset(&traceeven, SYS_open);
	praddset(&traceeven, SYS_fork);
	praddset(&traceeven, SYS_vfork);

	/* for I/O buffer dumps, force tracing of read()s and write()s */
	if (!isemptyset(&readfd)) {
		(void) Psysexit(Pr, SYS_read, TRUE);
		(void) Psysexit(Pr, SYS_readv, TRUE);
		praddset(&traceeven, SYS_read);
		praddset(&traceeven, SYS_readv);
	}
	if (!isemptyset(&writefd)) {
		(void) Psysexit(Pr, SYS_write, TRUE);
		(void) Psysexit(Pr, SYS_writev, TRUE);
		praddset(&traceeven, SYS_write);
		praddset(&traceeven, SYS_writev);
	}

	if (Pr->why.pr_flags&PR_PTRACE)
		isptraced();

	Pr->sigmask = signals;		/* trace these signals */
	Pr->setsig = TRUE;

	Pr->faultmask = faults;		/* trace these faults */
	Pr->setfault = TRUE;

	if (!created
	 && Pr->state == PS_STOP
	 && Pr->why.pr_why == PR_REQUESTED
	 && Pstart(Pr, 0) == -1)
		abend("cannot start subject process", 0);

	for (;;) {		/* run until termination */
		if (timeout) {
			(void) alarm(0);
			timeout = FALSE;
		}
		if (interrupt || sigusr1) {
			if (length)
				(void) fputc('\n', stdout);
			Flush();
			if (sigusr1)
				letgo(Pr);
			else
				(void) Prelease(Pr);
			break;
		}
		if (Pr->state == PS_RUN) {
			/* timeout is for sleeping syscalls */
			unsigned tout = (iflag||req_flag)? 0 : 2;

			(void) Pstatus(Pr, PIOCWSTOP, tout);
			if (Pr->state == PS_RUN && !interrupt && !sigusr1)
				(void) Pstop(Pr);
			continue;
		}
		if (Pr->state == PS_DEAD) {
			if (!cflag) {
				if (length)
					(void) fputc('\n', stdout);
				(void) printf(
					"%s\t*** process killed ***\n",
					pname);
				Flush();
			}
			break;
		}
		if (Pr->state == PS_LOST) {	/* we lost control */
			if (Preopen(Pr, Fflag) == 0)	/* we got it back */
				continue;

			/* we really lost it */
			if (length)
				(void) fputc('\n', stdout);
			if (sys_valid)
				(void) printf(
			"%s\t*** cannot trace across exec() of %s ***\n",
					pname, sys_path);
			else
				(void) printf(
				"%s\t*** lost control of process ***\n",
					pname);
			length = 0;
			Flush();
			(void) Prelease(Pr);
			break;
		}
		if (Pr->state != PS_STOP) {
			(void) fprintf(stderr,
				"%s: state = %d\n", command, Pr->state);
			abend(pname, "uncaught status of subject process");
		}

		what = Pr->why.pr_what;
		switch (Pr->why.pr_why) {
		case PR_REQUESTED:
			/* avoid multiple alarm sigs */
			req_flag = requested(Pr, req_flag);
			Flush();
			break;
		case PR_SIGNALLED:
			req_flag = signalled(Pr);
			Flush();
			break;
		case PR_FAULTED:
			req_flag = 0;
			faulted(Pr);
			Flush();
			break;
		case PR_JOBCONTROL:	/* can't happen except first time */
			req_flag = jobcontrol(Pr);
			Flush();
			break;
		case PR_SYSENTRY:
			req_flag = 0;
			/* protect ourself from operating system error */
			if (what <= 0 || what > PRMAXSYS)
				what = PRMAXSYS;
			length = 0;
			if (!cflag && pname[0] && prismember(&trace,what))
				(void) fputs(pname, stdout);
			switch (what) {
			case SYS_exit:			/* exit() */
			case SYS_context:		/* [get|set]context() */
			case SYS_evtrapret:		/* evtrapret() */
				if (!cflag && prismember(&trace,what)) {
					(void) sysentry(Pr);
					length += printf("%s\n", sys_string);
					Flush();
				}
				sys_leng = 0;
				*sys_string = '\0';
				if (prismember(&trace,what)) {
					Cp->syscount[what]++;
					accumulate(&Cp->systime[what],
						&Pr->why.pr_stime, &syslast);
				}
				syslast = Pr->why.pr_stime;
				usrlast = Pr->why.pr_utime;

				if (what == SYS_exit) {
					(void) Pstart(Pr, 0);
					goto done;
				}
				break;
			case SYS_exec:
			case SYS_execve:
				(void) sysentry(Pr);
				if (!cflag && prismember(&trace,SYS_execve))
					length += printf("%s", sys_string);
				sys_leng = 0;
				*sys_string = '\0';
				break;
			}
			break;
		case PR_SYSEXIT:
			req_flag = 0;
			sysexit(Pr);
			if (what == SYS_open) {
				/* check for r/w open of /proc */
				if ((Errno == 0 || Errno == EBUSY)
				 && sys_valid
				 && (sys_nargs > 1
				 && (sys_args[1]&0x3) != O_RDONLY)
				 && checkproc(Pr, sys_path, Errno)) {
					letgo(Pr);
					goto done;
				}
			}
			Flush();
			break;
		default:
			req_flag = 0;
			abend("unknown reason for stopping", 0);
		}

		if (child) {		/* controlled process fork()ed */
			if (!fflag) {
				if (no_inherit)
					release(child);
			}
			else if (control(Pr, child)) {
				child = 0;
				continue;
			}
			child = 0;
		}

		if ((Pr->why.pr_flags&PR_PTRACE)
		 && !wasptraced
		 && Pr->why.pr_why != PR_SYSENTRY)	/* freshly ptrace()d */
			isptraced();

		if (!(Pr->why.pr_flags & PR_ISTOP))
			Pr->state = PS_RUN;
		else {
			int runopt = 0;

			switch (req_flag) {
			case PTRACED:
			case JOBSIG:
			case JOBSTOP:
				runopt = PRSTOP;
				break;
			}
			if (Psetrun(Pr, 0, runopt) != 0)
				abend("cannot start subject process", 0);
		}
	}
done:
	Flush();

	accumulate(&Cp->systotal, &syslast, &sysbegin);
	accumulate(&Cp->usrtotal, &usrlast, &usrbegin);
	procdel();
	retc = wait4all();
	if (!descendent) {
		interrupt = FALSE; /* another interrupt kills the report */
		if (cflag)
			report(times(&tms) - starttime);

		if (shmid != -1) {
			(void) shmdt((char *)Cp);
			(void) shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
		}
		if (semid != -1)
			(void) semctl(semid, 0, IPC_RMID, 0);
	}
	return retc;	/* exit with exit status of created process, else 0 */
}

static int
xcreat(path)		/* create output file, being careful about */
	register char * path;	/* suid/sgid and fd 0, 1, 2 issues */
{
	register int fd;
	register int mode = 0666;

	if (Euid == Ruid && Egid == Rgid)	/* not set-id */
		fd = creat(path, mode);
	else if (access(path, 0) != 0) {	/* file doesn't exist */
		/* if directory permissions OK, create file & set ownership */

		register char * dir;
		register char * p;
		char dot[4];

		/* generate path for directory containing file */
		if ((p = strrchr(path, '/')) == NULL) {	/* no '/' */
			p = dir = dot;
			*p++ = '.';		/* current directory */
			*p = '\0';
		}
		else if (p == path) {			/* leading '/' */
			p = dir = dot;
			*p++ = '/';		/* root directory */
			*p = '\0';
		}
		else {					/* embedded '/' */
			dir = path;		/* directory path */
			*p = '\0';
		}

		if (access(dir, 03) != 0) {	/* not writeable/searchable */
			*p = '/';
			fd = -1;
		}
		else {		/* create file and set ownership correctly */
			*p = '/';
			if ((fd = creat(path, mode)) >= 0)
				(void) chown(path, (int)Ruid, (int)Rgid);
		}
	}
	else if (access(path, 02) != 0)		/* file not writeable */
		fd = -1;
	else
		fd = creat(path, mode);

	/* make sure it's not one of 0, 1, or 2 */
	/* this allows truss to work when spawned by init(1m) */
	if (0 <= fd && fd <= 2) {
		int dfd = fcntl(fd, F_DUPFD, 3);

		(void) close(fd);
		fd = dfd;
	}

	/* mark it close-on-exec so created processes don't inherit it */
	if (fd >= 0)
		(void) fcntl(fd, F_SETFD, 1);	/* close-on-exec */

	return fd;
}

static void
setoutput(ofd)
	register int ofd;
{
	if (ofd < 0) {
		(void) close(1);
		(void) fcntl(2, F_DUPFD, 1);
	}
	else if (ofd != 1) {
		(void) close(1);
		(void) fcntl(ofd, F_DUPFD, 1);
		(void) close(ofd);
		/* if no stderr, make it the same file */
		if ((ofd = dup(2)) < 0)
			(void) fcntl(1, F_DUPFD, 2);
		else
			(void) close(ofd);
	}
}

static void
isptraced()	/* report newly ptraced() condition */
{
	wasptraced = TRUE;
	if (!cflag) {
		(void) printf("%s\t*** process under ptrace() ***\n",
			pname);
		Flush();
	}
}

/*
 * Accumulate time differencies:  a += e - s;
 */
void
accumulate(ap, ep, sp)
	register timestruc_t *ap;
	register timestruc_t *ep;
	register timestruc_t *sp;
{
	ap->tv_sec += ep->tv_sec - sp->tv_sec;
	ap->tv_nsec += ep->tv_nsec - sp->tv_nsec;
	if (ap->tv_nsec >= 1000000000) {
		ap->tv_nsec -= 1000000000;
		ap->tv_sec++;
	}
	else if (ap->tv_nsec < 0) {
		ap->tv_nsec += 1000000000;
		ap->tv_sec--;
	}
}

static void
report(lapse)
	time_t lapse;	/* elapsed time, clock ticks */
{
	register int i;
	register long count;
	register CONST char * name;
	long error;
	register long total;
	long errtot;
	timestruc_t tickzero;
	timestruc_t ticks;
	timestruc_t ticktot;

	if (descendent)
		return;

	for (i = 0, total = 0; i <= PRMAXFAULT && !interrupt; i++) {
		if ((count = Cp->fltcount[i]) != 0) {
			if (total == 0)		/* produce header */
				(void) printf("faults -------------\n");
			name = fltname(i);
			(void) printf("%s%s\t%4d\n", name,
				(((int)strlen(name) < 8)?
				    (CONST char *)"\t" : (CONST char *)""),
				count);
			total += count;
		}
		Flush();
	}
	if (total && !interrupt) {
		(void) printf("total:\t\t%4d\n\n", total);
		Flush();
	}

	for (i = 0, total = 0; i <= PRMAXSIG && !interrupt; i++) {
		if ((count = Cp->sigcount[i]) != 0) {
			if (total == 0)		/* produce header */
				(void) printf("signals ------------\n");
			name = signame(i);
			(void) printf("%s%s\t%4d\n", name,
				(((int)strlen(name) < 8)?
				    (CONST char *)"\t" : (CONST char *)""),
				count);
			total += count;
		}
		Flush();
	}
	if (total && !interrupt) {
		(void) printf("total:\t\t%4d\n\n", total);
		Flush();
	}

	if (!interrupt) {
		(void) printf("syscall      seconds   calls  errors\n");
		Flush();
	}
	total = errtot = 0;
	tickzero.tv_sec = ticks.tv_sec = ticktot.tv_sec = 0;
	tickzero.tv_nsec = ticks.tv_nsec = ticktot.tv_nsec = 0;
	for (i = 0; i <= PRMAXSYS && !interrupt; i++) {
		if ((count = Cp->syscount[i]) != 0 || Cp->syserror[i]) {
			(void) printf("%-12.12s", sysname(i,-1));

			ticks = Cp->systime[i];
			accumulate(&ticktot, &ticks, &tickzero);
			prtim(&ticks);

			(void) printf("%8d", count);
			if ((error = Cp->syserror[i]) != 0)
				(void) printf(" %6d", error);
			(void) fputc('\n', stdout);
			total += count;
			errtot += error;

			Flush();
		}
	}

	if (!interrupt) {
		(void) printf("                ----     ---    ---\n");
		(void) printf("sys totals: ");
		prtim(&ticktot);
		(void) printf("%8d %6d\n", total, errtot);
		Flush();
	}

	if (!interrupt) {
		(void) printf("usr time:   ");
		prtim(&Cp->usrtotal);
		(void) fputc('\n', stdout);
		Flush();
	}

	if (!interrupt) {
		ticks.tv_sec = lapse/HZ;
		ticks.tv_nsec = (lapse%HZ)*(1000000000/HZ);
		(void) printf("elapsed:    ");
		prtim(&ticks);
		(void) fputc('\n', stdout);
		Flush();
	}
}

static void
prtim(tp)
	register timestruc_t *tp;
{
	register unsigned long sec;

	if ((sec = tp->tv_sec) != 0)			/* whole seconds */
		(void) printf("%5lu", sec);
	else
		(void) printf("     ");

	(void) printf(".%2.2ld", tp->tv_nsec/10000000);	/* fraction */
}

static int		/* gather process id's */
pids(path)		/* return 0 on success, != 0 on failure */
	register char * path;		/* number or /proc/nnn */
{
	register char * name;
	register pid_t pid;
	register int i;
	char *next;
	int rc = 0;

	if ((name = strrchr(path, '/')) != NULL)	/* last component */
		*name++ = '\0';
	else {
		name = path;
		path = procdir;
	}

	pid = strtol(name, &next, 10);
	if (isdigit(*name) && pid >= 0 && pid <= MAXPID && *next == '\0') {
		for (i = 0; i < ngrab; i++)
			if (grab[i] == pid	/* duplicate */
			 && strcmp(grabdir[i], path) == 0)
				break;
		if (i < ngrab) {
			(void) fprintf(stderr,
				"%s: duplicate process id ignored: %d\n",
				command, pid);
		}
		else if (ngrab < MAXGRAB) {
			if (strcmp(procdir, path) != 0
			 && !isprocdir((process_t *)NULL, path)) {
				(void) fprintf(stderr,
					"%s: %s is not a PROC directory\n",
					command, path);
				rc = -1;
			}
			else {
				grabdir[ngrab] = path;
				grab[ngrab++] = pid;
			}
		}
		else {
			if (ngrab++ == MAXGRAB)	/* only one message */
				(void) fprintf(stderr,
			"%s: too many process id's, max is %d\n",
					command, MAXGRAB);
			rc = -1;
		}
	}
	else {
		(void) fprintf(stderr, "%s: invalid process id: %s\n",
			command, name);
		rc = -1;
	}

	return rc;
}

static void
psargs(Pr)	/* report psargs string */
	register process_t *Pr;
{
	struct prpsinfo prpsinfo;

	if (Ioctl(Pr->pfd, PIOCPSINFO,  (int)&prpsinfo) == -1)
		perror("PIOCPSINFO");
	else {
		prpsinfo.pr_psargs[PRARGSZ-16] = '\0';
		(void) printf("%spsargs: %s\n", pname, prpsinfo.pr_psargs);
	}
}

char *
fetchstring(addr, maxleng)
	register long addr;
	register int maxleng;
{
	register process_t *Pr = &Proc;
	register int nbyte;
	register leng = 0;
	char string[41];
	string[40] = '\0';

	if (str_bsize == 0) {	/* initial allocation of string buffer */
		str_buffer = (char *)malloc(str_bsize = 16);
		if (str_buffer == NULL)
			abend("cannot allocate string buffer", 0);
	}
	*str_buffer = '\0';

	for (nbyte = 40; nbyte == 40 && leng < maxleng; addr += 40) {
		if ((nbyte = Pread(Pr, addr, string, 40)) < 0)
			return leng? str_buffer : NULL;
		if (nbyte > 0
		 && (nbyte = strlen(string)) > 0) {
			while (leng+nbyte >= str_bsize) {
				str_buffer = (char *)realloc(str_buffer, str_bsize *= 2);
				if (str_buffer == NULL)
					abend("cannot reallocate string buffer", 0);
			}
			(void) strcpy(str_buffer+leng, string);
			leng += nbyte;
		}
	}

	if (leng > maxleng)
		leng = maxleng;
	str_buffer[leng] = '\0';

	return str_buffer;
}

void
show_cred(Pr, new)
	register process_t *Pr;
	int new;
{
	prcred_t cred;

	(void) Ioctl(Pr->pfd, PIOCCRED, (int)&cred);

	if (!cflag && prismember(&trace,SYS_exec)) {
		if (new)
			credentials = cred;
		if ((new && cred.pr_ruid != cred.pr_suid)
		 || cred.pr_ruid != credentials.pr_ruid
		 || cred.pr_suid != credentials.pr_suid)
			(void) printf(
			"%s    *** SUID: ruid/euid/suid = %d / %d / %d  ***\n",
			pname,
			cred.pr_ruid,
			cred.pr_euid,
			cred.pr_suid);
		if ((new && cred.pr_rgid != cred.pr_sgid)
		 || cred.pr_rgid != credentials.pr_rgid
		 || cred.pr_sgid != credentials.pr_sgid)
			(void) printf(
			"%s    *** SGID: rgid/egid/sgid = %d / %d / %d  ***\n",
			pname,
			cred.pr_rgid,
			cred.pr_egid,
			cred.pr_sgid);
	}

	credentials = cred;
}

static int
control(Pr, pid)	/* take control of a child process */
	register process_t *Pr;
	pid_t pid;
{
	pid_t mypid = 0;

	if (interrupt || sigusr1 || (mypid = fork()) == -1) {
		if (mypid == -1)
			(void) printf(
			"%s\t*** Cannot fork() to control process #%d\n",
				pname, pid);
		release(pid);
		return FALSE;
	}

	if (mypid != 0) {		/* parent carries on */
		if (!sigusr1)
			(void) pause();		/* after a brief pause */
		sigusr1 = FALSE;
		return FALSE;
	}

	descendent = TRUE;
	(void) close(Pr->pfd);		/* forget old process */

	if (!cflag)
		(void) sprintf(pname, "%d:\t", pid);

	if (Pgrab(Pr, pid, FALSE) != 0) {	/* child grabs new process */
		(void) fprintf(stderr,
			"%s: cannot control child process, pid# %d\n",
			command, pid);
		(void) kill(getppid(), SIGUSR1);	/* wake up parent */
		exit(2);
	}

	if (Pr->why.pr_why != PR_SYSEXIT
	 || (Pr->why.pr_what != SYS_fork && Pr->why.pr_what != SYS_vfork))
		(void) printf("%s\t*** Expected SYSEXIT, SYS_[v]fork\n", pname);

	sysbegin = syslast = Pr->why.pr_stime;
	usrbegin = usrlast = Pr->why.pr_utime;

	wasptraced = FALSE;	/* child is not ptrace()d */

	no_inherit = Ioctl(Pr->pfd, PIOCSFORK, 0);
	procadd(pid);
	(void) kill(getppid(), SIGUSR1);	/* wake up parent */

	return TRUE;
}

static int
grabit(Pr, dir, pid)		/* take control of an existing process */
	register process_t *Pr;
	char * dir;
	pid_t pid;
{
	procdir = dir;		/* /proc directory */

	if ((fflag || ngrab > 1) && !cflag)
		(void) sprintf(pname, "%d:\t", pid);

	/* don't force the takeover unless the -F option was specified */
	switch (Pgrab(Pr, pid, Fflag)) {
	case 0:
		break;
	case 1:
		(void) fprintf(stderr,
			"%s: someone else is tracing process %d\n",
			command, pid);
		return FALSE;
	default:
		goto bad;
	}

	if ((Pr->state != PS_STOP || Pr->why.pr_why != PR_REQUESTED)
	 && !(Pr->why.pr_flags & PR_DSTOP)) {
#if 0
		(void) fprintf(stderr,
			"%s: expected REQUESTED stop, pid# %d\n",
			command, pid);
#endif
		if (Pr->state != PS_STOP && !(Pr->why.pr_flags & PR_DSTOP)) {
			(void) Prelease(Pr);
			goto bad;
		}
	}

	sysbegin = syslast = Pr->why.pr_stime;
	usrbegin = usrlast = Pr->why.pr_utime;

	no_inherit = Ioctl(Pr->pfd, fflag? PIOCSFORK : PIOCRFORK, 0);
	procadd(pid);
	show_cred(Pr, TRUE);
	return TRUE;

bad:
	if (errno == ENOENT)
		(void) fprintf(stderr, "%s: no process %d\n",
			command, pid);
	else
		(void) fprintf(stderr, "%s: cannot control process %d\n",
			command, pid);
	return FALSE;
}

static void
release(pid)	/* release process from control */
	pid_t pid;
{
	/* The process in question is the child of a traced process. */
	/* We are here to turn off the inherited tracing flags. */

	register int fd;
	char procname[100];

	/* open the /proc/upid file */
	(void) sprintf(procname, "%s/%d", procdir, pid);

	/* this process is freshly forked, no need for exclusive open */
	if ((fd = open(procname, O_RDWR)) < 0
	 || Ioctl(fd, PIOCSRLC,  0) != 0) {
		perror("release()");
		(void) printf("%s\t*** Cannot release child process, pid# %d\n",
			pname, pid);
	}
	if (fd >= 0)
		(void) close(fd);
}

static void
intr(sig)
	register int sig;
{
	/* SIGUSR1 is special.  It is used by one truss process to tell */
	/* another truss processes to release its controlled process. */
	/* SIGALRM is used for timing out indefinite waits. */
	if (sig == SIGUSR1) {
		sigusr1 = TRUE;
	} else if (sig == SIGALRM) {		/* reset alarm clock */
		timeout = TRUE;
		(void) alarm(1);
	} else {
		interrupt = TRUE;
	}
}

/* For receiving SIGSYS while getting semaphores and shared memory */
/*ARGSUSED*/
static void
ssys(sig)
	int sig;
{
	sigsys = TRUE;
}

void
errmsg(s,q)
	register CONST char *s;
	register CONST char *q;
{
	char msg[200];

	msg[0] = '\0';
	if (command) {
		(void) strcpy(msg, command);
		(void) strcat(msg, ": ");
	}
	if (s) (void) strcat(msg, s);
	if (q) (void) strcat(msg, q);
	(void) strcat(msg, "\n");
	(void) write(2, msg, (unsigned)strlen(msg));
}

void
abend(s,q)
	register CONST char *s;
	register CONST char *q;
{
	Flush();
	if (s || q)
		errmsg(s, q);
	if (PR) {
		if (created)
			(void) Pterm(PR);
		else
			(void) Prelease(PR);
	}
	procdel();
	(void) wait4all();
	if (!descendent) {
		if (shmid != -1) {
			(void) shmdt((char *)Cp);
			(void) shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
		}
		if (semid != -1)
			(void) semctl(semid, 0, IPC_RMID, 0);
	}
	exit(2);
}

static int
wait4all()
{
	register int i;
	register pid_t pid;
	register int rc = 0;
	int status;

	for (i = 0; i < 10; i++) {
		while ((pid = wait(&status)) != -1) {
			/* return exit() code of the created process */
			if (pid == created) {
				if ((rc = status&0xff) == 0)
					rc = (status>>8)&0xff;	/* exit()ed */
				else
					rc |= 0x80;		/* killed */
			}
		}
		if (errno != EINTR)
			break;
	}

	if (i >= 10)	/* repeated interrupts */
		rc = 2;

	return rc;
}

static void
letgo(Pr)
	register process_t *Pr;
{
	(void) printf("%s\t*** process otherwise traced, releasing ...\n",
		pname);
	Flush();
	(void) Prelease(Pr);
}

int
isprocdir(Pr, dir)	/* return TRUE iff dir is a PROC directory */
	process_t *Pr;
	CONST char *dir;	/* this is filthy */
{
	/* This is based on the fact that "/proc/0" and "/proc/00" are the */
	/* same file, namely process 0, and are not linked to each other. */

	struct stat stat1;	/* dir/0  */
	struct stat stat2;	/* dir/00 */
	char path[200];
	register char * p;

	/* make a copy of the directory name without trailing '/'s */
	if (dir == NULL)
		(void) strcpy(path, ".");
	else {
		(void) strncpy(path, dir, (int)sizeof(path)-4);
		path[sizeof(path)-4] = '\0';
		p = path + strlen(path);
		while (p > path && *--p == '/')
			*p = '\0';
		if (*path == '\0')
			(void) strcpy(path, ".");
	}

	/* append "/0" to the directory path and stat() the file */
	p = path + strlen(path);
	*p++ = '/';
	*p++ = '0';
	*p = '\0';
	if (prstat(Pr, path, &stat1) != 0)	/* make process execute stat */
		return FALSE;

	/* append "/00" to the directory path and stat() the file */
	*p++ = '0';
	*p = '\0';
	if (prstat(Pr, path, &stat2) != 0)	/* make process execute stat */
		return FALSE;

	/* see if we ended up with the same file */
	if (stat1.st_dev   != stat2.st_dev
	 || stat1.st_ino   != stat2.st_ino
	 || stat1.st_mode  != stat2.st_mode
	 || stat1.st_nlink != stat2.st_nlink
	 || stat1.st_uid   != stat2.st_uid
	 || stat1.st_gid   != stat2.st_gid
	 || stat1.st_size  != stat2.st_size)
		return FALSE;

	/* return TRUE iff we have a regular file with a single link */
	return ((stat1.st_mode&S_IFMT) == S_IFREG && stat1.st_nlink == 1);
}

static int	/* stat() system call -- executed by subject process */
prstat(Pr, path, buf)
	process_t *Pr;
	char * path;
	struct stat * buf;
{
	struct sysret rval;		/* return value from stat() */
	struct argdes argd[2];		/* arg descriptors for stat() */
	register struct argdes *adp;

	if (Pr == (process_t *)NULL)	/* no subject process */
		return stat(path, buf);

	adp = &argd[0];		/* pathname argument */
	adp->value = 0;
	adp->object = (char *)path;
	adp->type = AT_BYREF;
	adp->inout = AI_INPUT;
	adp->len = strlen(path)+1;

	adp++;			/* buffer argument */
	adp->value = 0;
	adp->object = (char *)buf;
	adp->type = AT_BYREF;
	adp->inout = AI_OUTPUT;
	adp->len = sizeof(struct stat);

	rval = Psyscall(Pr, SYS_stat, 2, &argd[0]);

	if (rval.errno < 0) {
		(void) fprintf(stderr,
			"%s\t*** error from Psyscall(SYS_stat): %d\n",
			pname, rval.errno);
		rval.errno = EINVAL;
	}

	if (rval.errno == 0)
		return 0;
	errno = rval.errno;
	return -1;
}
