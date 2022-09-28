/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:actions.c	1.5.3.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>

#include <signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "systable.h"
#include "print.h"
#include "proto.h"

/*
 * Actions to take when process stops.
 */

/* This module is carefully coded to contain only read-only data */
/* All read/write data is defined in ramdata.c (see also ramdata.h) */

extern	char *	malloc();
extern	char *	realloc();
extern	void	free();

/*
 * Function prototypes for static routines in this module.
 */
#if	defined(__STDC__)

static	int	ptraced(process_t *);
static	int	stopsig(process_t *);
static	unsigned getargp(process_t *, int *);
static	void	showpaths(CONST struct systable *);
static	void	showargs(process_t *, int);
static	void	dumpargs(process_t *, char *, CONST char *);

#else	/* defined(__STDC__) */

static	int	ptraced();
static	int	stopsig();
static	unsigned getargp();
static	void	showpaths();
static	void	showargs();
static	void	dumpargs();

#endif	/* defined(__STDC__) */

/*
 * requested() gets called for these reasons:
 *	flag == PTRACED:	report "Continued with ..."
 *	flag == JOBSIG:		report nothing; change state to JOBSTOP
 *	flag == JOBSTOP:	report "Continued ..."
 *	default:		report sleeping system call
 *
 * It returns a new flag:  JOBSTOP or SLEEPING or 0.
 */
int
requested(Pr, flag)
	register process_t *Pr;
	register int flag;
{
	register int sig = Pr->why.pr_cursig;
	register int sys;
	register int newflag = 0;

	switch (flag) {
	case JOBSIG:
		return JOBSTOP;

	case PTRACED:
	case JOBSTOP:
		if (sig > 0 && sig <= PRMAXSIG && prismember(&signals, sig)) {
			length = 0;
			(void) printf("%s    Continued with signal #%d, %s",
				pname, sig, signame(sig));
			if (Pr->why.pr_action.sa_handler == SIG_DFL)
				(void) printf(" [default]");
			else if (Pr->why.pr_action.sa_handler == SIG_IGN)
				(void) printf(" [ignored]");
			else
				(void) printf(" [caught]");
			(void) fputc('\n', stdout);
		}
		break;

	default:
		if (!(Pr->why.pr_flags & PR_ASLEEP)
		 || (sys = Pgetsysnum(Pr)) <= 0 || sys > PRMAXSYS)
			break;

		newflag = SLEEPING;

		/* Make sure we catch sysexit even if we're not tracing it. */
		if (!prismember(&trace, sys))
			(void) Psysexit(Pr, sys, TRUE);
		else if (!cflag) {
			length = 0;
			Pr->why.pr_what = sys;	/* cheating a little */
			Errno = 0;
			Rval1 = Rval2 = 0;
			(void) sysentry(Pr);
			if (pname[0])
				(void) fputs(pname, stdout);
			length += printf("%s", sys_string);
			sys_leng = 0;
			*sys_string = '\0';
			length >>= 3;
			if (length >= 4)
				(void) fputc(' ', stdout);
			for ( ; length < 4; length++)
				(void) fputc('\t', stdout);
			(void) fputs("(sleeping...)\n", stdout);
			length = 0;
		}
		break;
	}

	if ((flag = jobcontrol(Pr)) != 0)
		newflag = flag;

	return newflag;
}

static int
ptraced(Pr)
	register process_t *Pr;
{
	register int sig = Pr->why.pr_what;

	if (!(Pr->why.pr_flags & PR_PTRACE)
	 || Pr->why.pr_why != PR_SIGNALLED
	 || sig != Pr->why.pr_cursig
	 || sig <= 0 || sig > PRMAXSIG)
		return 0;

	if (!cflag
	 && prismember(&signals, sig)) {
		register int sys;

		length = 0;
		(void) printf("%s    Stopped on signal #%d, %s",
			pname, sig, signame(sig));
		if ((Pr->why.pr_flags & PR_ASLEEP)
		 && (sys = Pgetsysnum(Pr)) > 0 && sys <= PRMAXSYS) {
			int code;
			int nabyte;
			long ap = getargp(Pr, &nabyte);

			if (nabyte < sizeof(code)
			 || Pread(Pr, ap, (char *)&code, sizeof(code))
			    != sizeof(code))
				code = -1;

			(void) printf(", in %s()", sysname(sys, code));
		}
		(void) fputc('\n', stdout);
	}

	return PTRACED;
}

int
jobcontrol(Pr)
	register process_t *Pr;
{
	register int sig = stopsig(Pr);

	if (sig == 0)
		return 0;

	if (!cflag				/* not just counting */
	 && prismember(&signals, sig)) {	/* tracing this signal */
		register int sys;

		length = 0;
		(void) printf("%s    Stopped by signal #%d, %s",
			pname, sig, signame(sig));
		if ((Pr->why.pr_flags & PR_ASLEEP)
		 && (sys = Pgetsysnum(Pr)) > 0
		 && sys <= PRMAXSYS) {
			int code;
			int nabyte;
			long ap = getargp(Pr, &nabyte);

			if (nabyte < sizeof(code)
			 || Pread(Pr, ap, (char *)&code, sizeof(code))
			    != sizeof(code))
				code = -1;

			(void) printf(", in %s()", sysname(sys, code));
		}
		(void) fputc('\n', stdout);
	}

	return JOBSTOP;
}

/*
 * Return the signal the process stopped on iff process is already stopped on
 * PR_JOBCONTROL or is stopped on PR_SIGNALLED or PR_REQUESTED with a current
 * signal that will cause a JOBCONTROL stop when the process is set running.
 */
static int
stopsig(Pr)
	register process_t *Pr;
{
	register int sig = 0;

	if (Pr->state == PS_STOP) {
		switch (Pr->why.pr_why) {
		case PR_JOBCONTROL:
			sig = Pr->why.pr_what;
			if (sig < 0 || sig > PRMAXSIG)
				sig = 0;
			break;
		case PR_SIGNALLED:
		case PR_REQUESTED:
			if (Pr->why.pr_action.sa_handler == SIG_DFL) {
				switch (Pr->why.pr_cursig) {
				case SIGSTOP:
				case SIGTSTP:
				case SIGTTIN:
				case SIGTTOU:
					sig = Pr->why.pr_cursig;
					break;
				}
			}
			break;
		}
	}

	return sig;
}

int
signalled(Pr)
	register process_t *Pr;
{
	register int sig = Pr->why.pr_what;
	register int flag = 0;

	if (sig <= 0 || sig > PRMAXSIG)	/* check bounds */
		return 0;

	if (cflag)			/* just counting */
		Cp->sigcount[sig]++;

	if ((flag = ptraced(Pr)) == 0
	 && (flag = jobcontrol(Pr)) == 0
	 && !cflag
	 && prismember(&signals, sig)) {
		register int sys;

		length = 0;
		(void) printf("%s    Received signal #%d, %s",
			pname, sig, signame(sig));
		if ((Pr->why.pr_flags & PR_ASLEEP)
		 && (sys = Pgetsysnum(Pr)) > 0
		 && sys <= PRMAXSYS) {
			int code;
			int nabyte;
			long ap = getargp(Pr, &nabyte);

			if (nabyte < sizeof(code)
			 || Pread(Pr, ap, (char *)&code, sizeof(code))
			    != sizeof(code))
				code = -1;

			(void) printf(", in %s()", sysname(sys, code));
		}
		if (Pr->why.pr_action.sa_handler == SIG_DFL)
			(void) printf(" [default]");
		else if (Pr->why.pr_action.sa_handler == SIG_IGN)
			(void) printf(" [ignored]");
		else
			(void) printf(" [caught]");
		(void) fputc('\n', stdout);
		if (Pr->why.pr_info.si_code != 0
		 || Pr->why.pr_info.si_pid != 0)
			print_siginfo(&Pr->why.pr_info);
	}

	if (flag == JOBSTOP)
		flag = JOBSIG;
	return flag;
}

void
faulted(Pr)
	process_t *Pr;
{
	register int flt = Pr->why.pr_what;

	if ((unsigned)flt > PRMAXFAULT)	/* check bounds */
		flt = 0;
	Cp->fltcount[flt]++;

	if (cflag)		/* just counting */
		return;
	length = 0;
	(void) printf("%s    Incurred fault #%d, %s  %%pc = 0x%.8X",
		pname, flt, fltname(flt), Pr->why.pr_reg[R_PC]);
	if (flt == FLTPAGE)
		(void) printf("  addr = 0x%.8X", Pr->why.pr_info.si_addr);
	(void) fputc('\n', stdout);
	if (Pr->why.pr_info.si_signo != 0)
		print_siginfo(&Pr->why.pr_info);
}

#ifdef i386 /* XENIX Support */
pid_t g_upid = 0;
#endif

int
sysentry(Pr)		/* return TRUE iff syscall is being traced */
	register process_t *Pr;
{
	register int arg;
	int nabyte;
	int nargs;
	register int i;
	register int x;
	int len;
	char * s;
	unsigned ap;
	register CONST struct systable *stp;
	register int what = Pr->why.pr_what;
	int subcode = -1;
	int istraced;
	int raw;
	int argprinted;

	/* protect ourself from operating system error */
	if (what <= 0 || what > PRMAXSYS)
		what = 0;

#ifdef i386 /* XENIX Support */
	g_upid = Pr->upid;
#endif
	/* get systable entry for this syscall */
	stp = subsys(what, -1);

	/* get address of argument list + number of bytes of arguments */
	ap = getargp(Pr, &nabyte);

	if (nabyte > sizeof(sys_args))
		nabyte = sizeof(sys_args);

	nargs = nabyte / sizeof(int);
	if (nargs > stp->nargs)
		nargs = stp->nargs;
	sys_nargs = nargs;
	nabyte = nargs * sizeof(int);

	if (stp->nargs)
		(void)memset((char *)sys_args,0,(int)(stp->nargs*sizeof(int)));
	
	if (nabyte > 0
	 && (i = Pread(Pr, (long)ap, (char *)sys_args, nabyte)) != nabyte) {
		(void) printf("%s\t*** Bad argument pointer: 0x%.8X ***\n",
			pname, ap);
		if (i < 0)
			i = 0;
		sys_nargs = nargs = i / sizeof(int);
		nabyte = nargs * sizeof(int);
	}

	if (nargs > 0) {	/* interpret syscalls with sub-codes */
		if (what == SYS_utssys && nargs > 2)	/* yuck */
			subcode = sys_args[2];
		else
			subcode = sys_args[0];
		stp = subsys(what, subcode);
	}
	if (nargs > stp->nargs)
		nargs = stp->nargs;
	sys_nargs = nargs;

	/* fetch and remember first argument if it's a string */
	sys_valid = FALSE;
	if (nargs > 0
	 && stp->arg[0] == STG
	 && (s = fetchstring((long)sys_args[0], 400)) != NULL) {
		sys_valid = TRUE;
		len = strlen(s);
		while (len >= sys_psize) {	/* reallocate if necessary */
			free(sys_path);
			sys_path = malloc(sys_psize *= 2);
			if (sys_path == NULL)
				abend("cannot allocate pathname buffer", 0);
		}
		(void) strcpy(sys_path, s);	/* remember pathname */
	}

	istraced = prismember(&trace, what);
	raw = prismember(&rawout, what);

	/* force tracing of read/write buffer dump syscalls */
	if (!istraced && nargs > 2) {
		int fdp1 = sys_args[0] + 1;

		switch (what) {
		case SYS_read:
		case SYS_readv:
			if (prismember(&readfd, fdp1))
				istraced = TRUE;
			break;
		case SYS_write:
		case SYS_writev:
			if (prismember(&writefd, fdp1))
				istraced = TRUE;
			break;
		}
	}

	/* initial argument index, skipping over hidden arguments */
	for (i = 0; !raw && i < nargs && stp->arg[i] == HID; i++)
		;

	sys_leng = 0;
	if (cflag || !istraced)		/* just counting */
		*sys_string = 0;
	else if (i >= nargs)
		sys_leng = sprintf(sys_string, "%s()",
			sysname(what, raw? -1 : subcode));
	else {
		sys_leng = sprintf(sys_string, "%s(",
			sysname(what, raw? -1 : subcode));
		argprinted = 0;
		for (;;) {
			arg = sys_args[i];
			x = stp->arg[i];

			if (x == STG && !raw
			 && i == 0 && sys_valid) {	/* already fetched */
				outstring("\"");
				outstring(sys_path);
				outstring("\"");
				argprinted = 1;
			} else if (x != HID) {
				if (argprinted)
					outstring(", ");
				(*Print[x])(arg, raw);
				argprinted = 1;
			}

			if (++i >= nargs)
				break;
		}
		outstring(")");
	}

	return istraced;
}

static unsigned
getargp(Pr, nbp)		/* get address of arguments to syscall */
	register process_t *Pr;
	int *nbp;		/* also return # of bytes of arguments */
{
	unsigned ap, sp;
	int nabyte;

#ifdef i386
	(void) Pgetareg(Pr, R_SP); sp = Pr->REG[R_SP];
	ap = sp + sizeof(int);
	nabyte = 512;
#endif

#if u3b2 || u3b5
	unsigned fp;

	(void) Pgetareg(Pr, R_AP); ap = Pr->REG[R_AP];
	(void) Pgetareg(Pr, R_FP); fp = Pr->REG[R_FP];
	(void) Pgetareg(Pr, R_SP); sp = Pr->REG[R_SP];

	if ((nabyte = fp - ap - FRMSZ*sizeof(int)) < 0
	 && (nabyte = sp - ap - 2*sizeof(int)) < 0)
		nabyte = 0;
#endif
#if mc68k
	(void) Pgetareg(Pr, R_SP); sp = Pr->REG[R_SP];
	ap = sp + sizeof(int);
	nabyte = 512;		/* can't tell -- make it large */
#endif
	if (nbp != NULL)
		*nbp = nabyte;
	return ap;
}

void
sysexit(Pr)
	register process_t *Pr;
{
	int r0;
	int r1;
	int ps;
	int what = Pr->why.pr_what;
	register CONST struct systable *stp;
	int arg0;
	int istraced;
	int raw;

	/* protect ourself from operating system error */
	if (what <= 0 || what > PRMAXSYS)
		what = 0;
	
	/*
	 * If we aren't supposed to be tracing this one, then
	 * delete it from the traced signal set.  We got here
	 * because the process was sleeping in an untraced syscall.
	 */
	if (!prismember(&traceeven, what)) {
		(void) Psysexit(Pr, what, FALSE);
		return;
	}

	/* get systable entry for this syscall */
	stp = subsys(what, -1);

	/* pick up registers & set Errno before anything else */

#ifdef i386
	(void) Pgetareg(Pr, R_0); r0 = Pr->REG[R_0];
	(void) Pgetareg(Pr, R_1); r1 = Pr->REG[R_1];
#else
	(void) Pgetareg(Pr, 0); r0 = Pr->REG[0];
	(void) Pgetareg(Pr, 1); r1 = Pr->REG[1];
#endif

	(void) Pgetareg(Pr, R_PS); ps = Pr->REG[R_PS];

	Errno = (ps & ERRBIT)? r0 : 0;
	Rval1 = r0;
	Rval2 = r1;

	switch (what) {
	case SYS_exit:		/* these are traced on entry */
	case SYS_exec:
	case SYS_execve:
	case SYS_evtrapret:
	case SYS_context:
		istraced = prismember(&trace, what);
		break;
	default:
		istraced = sysentry(Pr);
		length = 0;
		if (!cflag && istraced) {
			if (pname[0])
				(void) fputs(pname, stdout);
			length += printf("%s", sys_string);
		}
		sys_leng = 0;
		*sys_string = '\0';
		break;
	}

	if (istraced) {
		Cp->syscount[what]++;
		accumulate(&Cp->systime[what], &Pr->why.pr_stime, &syslast);
	}
	syslast = Pr->why.pr_stime;
	usrlast = Pr->why.pr_utime;

	arg0 = sys_args[0];

	if (!cflag && istraced) {
		if ((what == SYS_fork || what == SYS_vfork)
		 && Errno == 0 && r1 != 0) {
			length &= ~07;
			length += 14 + printf("\t\t(returning as child ...)");
		}
		if (Errno != 0 || (what != SYS_exec && what != SYS_execve)) {
			/* prepare to print the return code */
			length >>= 3;
			if (length >= 6)
				(void) fputc(' ', stdout);
			for ( ; length < 6; length++)
				(void) fputc('\t', stdout);
		}
	}
	length = 0;

	if (sys_nargs > 0)		/* interpret syscalls with sub-codes */
		stp = subsys(what, arg0);

	raw = prismember(&rawout, what);

	if (Errno != 0) {		/* error in syscall */
		if (istraced) {
			Cp->syserror[what]++;
			if (!cflag) {
				CONST char * ename = errname(r0);

				(void) printf("Err#%d", r0);
				if (ename != NULL) {
					if (r0 < 10)
						(void) fputc(' ', stdout);
					(void) fputc(' ', stdout);
					(void) fputs(ename, stdout);
				}
				(void) fputc('\n', stdout);
			}
		}
	}
	else {
		/* show arguments on successful exec */
		if (what == SYS_exec || what == SYS_execve) {
			if (!cflag && istraced)
				showargs(Pr, raw);
		}
		else if (!cflag && istraced) {
			CONST char * fmt = NULL;

			switch (what) {
			case SYS_lseek:
			case SYS_ulimit:
				if (r0 & 0xff000000)
					fmt = "= 0x%.8X";
				break;
			case SYS_signal:
				if (raw)
					fmt = NULL;
				else if (r0 == (int)SIG_DFL)
					fmt = "= SIG_DFL";
				else if (r0 == (int)SIG_IGN)
					fmt = "= SIG_IGN";
				else if (r0 == (int)SIG_HOLD)
					fmt = "= SIG_HOLD";
				break;
			}

			if (fmt == NULL) switch (stp->rval[0]) {
			case HEX:
				fmt = "= 0x%.8X";
				break;
			case HHX:
				fmt = "= 0x%.4X";
				break;
			case OCT:
				fmt = "= %#o";
				break;
			default:
				fmt = "= %d";
				break;
			}

			(void) printf(fmt, r0);

			switch (stp->rval[1]) {
			case NOV:
				fmt = NULL;
				break;
			case HEX:
				fmt = "  [ 0x%.8X ]";
				break;
			case HHX:
				fmt = "  [ 0x%.4X ]";
				break;
			case OCT:
				fmt = "  [ %#o ]";
				break;
			default:
				fmt = "  [ %d ]";
				break;
			}

			if (fmt != NULL)
				(void) printf(fmt, r1);
			(void) fputc('\n', stdout);
		}

		if (what == SYS_fork || what == SYS_vfork) {
			if (r1 == 0)		/* child was created */
				child = r0;
			else if (istraced)	/* this is the child */
				Cp->syscount[what]--;
		}
	}

	if (!cflag && istraced) {
		int fdp1 = arg0+1;	/* read()/write() filedescriptor + 1 */

		if (raw) {
			showpaths(stp);
			if ((what == SYS_read || what == SYS_write)
			 && iob_buf[0] != '\0')
				(void) printf("%s     0x%.8X: %s\n",
					pname, sys_args[1], iob_buf);
		}

		/*
		 * Show buffer contents for read() or write().
		 * IOBSIZE bytes have already been shown;
		 * don't show them again unless there's more.
		 */
		if (((what==SYS_read && Errno==0 && prismember(&readfd,fdp1))
		  || (what==SYS_write && prismember(&writefd,fdp1)))) {
			int nb = what==SYS_write? sys_args[2] : r0;

			if (nb > IOBSIZE) {
				/* enter region of lengthy output */
				if (nb > BUFSIZ/4)
					Eserialize();

				showbuffer(Pr, (long)sys_args[1], nb);

				/* exit region of lengthy output */
				if (nb > BUFSIZ/4)
					Xserialize();
			}
		}

		/*
		 * Do verbose interpretation if requested.
		 * If buffer contents for read or write have been requested and
		 * this is a readv() or writev(), force verbose interpretation.
		 */
		if (prismember(&verbose, what)
		 || (what==SYS_readv && Errno == 0 && prismember(&readfd,fdp1))
		 || (what==SYS_writev && prismember(&writefd,fdp1)))
			expound(Pr, r0, raw);
	}
}

static void
showpaths(stp)
	register CONST struct systable * stp;
{
	register int i;

	for (i = 0; i < sys_nargs; i++) {
		if ((stp->arg[i] == STG)
		 || (stp->arg[i] == RST && !Errno)
		 || (stp->arg[i] == RLK && !Errno && Rval1 > 0)) {
			register long addr = (long)sys_args[i];
			register int maxleng = (stp->arg[i]==RLK)? Rval1 : 400;
			register char * s;

			if (i == 0 && sys_valid)	/* already fetched */
				s = sys_path;
			else
				s = fetchstring(addr, maxleng>400?400:maxleng);

			if (s != (char *)NULL)
				(void) printf("%s     0x%.8X: \"%s\"\n",
					pname, addr, s);
		}
	}
}

static void
showargs(Pr, raw)	/* display arguments to successful exec() */
	register process_t *Pr;
	int raw;
{
	int nargs;
	char * ap;
	char * sp;

	length = 0;

#ifdef i386
	Pgetareg(Pr, R_SP); ap = (char *)Pr->REG[R_SP]; /* UESP */

	if ( Pread(Pr, (long)ap, (char *)&nargs, sizeof(nargs))
	     != sizeof(nargs)) {
		printf("\n%s\t*** Bad argument list? ***\n", pname);
		return;
	}
	if (debugflag) {
		int i, n, stack[256];
		n = 0x7fffffff - (long)ap;
		if ( n > 1024 ) n = 1024;
		fprintf(stderr, "ap = 0x%x, nargs = %d, stacksize = %d\n",
						ap, nargs, n);
		Pread(Pr, (long)ap, (char *)stack, n);
		for ( i = 0 ; i < 256 ; i++ ) {
			if ( (n -= 4) < 0 )
				break;
			fprintf(stderr, "%08x:	%8x\n", ap + 4 * i, stack[i]);
		}
	}
#endif

#if u3b2 || u3b5
	(void) Pgetareg(Pr, R_AP); ap = (char *)Pr->REG[R_AP];
	(void) Pgetareg(Pr, R_SP); sp = (char *)Pr->REG[R_SP];
#endif
#if mc68k
	(void) Pgetareg(Pr, R_SP); sp = (char *)Pr->REG[R_SP];
	ap = sp;	/* make it look like 3b2 */
	sp += sizeof(int) + 3*sizeof(char *);
#endif

#ifndef i386
	if (sp - ap < sizeof(int) + 3*sizeof(char *)
	 || Pread(Pr, (long)ap, (char *)&nargs, sizeof(nargs))
	     != sizeof(nargs)) {
		(void) printf("\n%s\t*** Bad argument list? ***\n", pname);
		return;
	}
#endif

	(void) printf("  argc = %d\n", nargs);
	if (raw)
		showpaths(&systable[SYS_exec]);

	show_cred(Pr, FALSE);

	if (aflag || eflag) {		/* dump args or environment */

		/* enter region of (potentially) lengthy output */
		Eserialize();

		ap += sizeof(int);

		if (aflag)		/* dump the argument list */
			dumpargs(Pr, ap, "argv:");

		ap += (nargs+1) * sizeof(char *);

		if (eflag)		/* dump the environment */
			dumpargs(Pr, ap, "envp:");

		/* exit region of lengthy output */
		Xserialize();
	}
}

static void
dumpargs(Pr, ap, str)
	register process_t *Pr;
	register char * ap;
	CONST char * str;
{
	register char * string;
	register int leng = 0;
	char * arg;
	char badaddr[32];

	if (interrupt)
		return;

	if (pname[0])
		(void) fputs(pname, stdout);
	(void) fputc(' ', stdout);
	(void) fputs(str , stdout);
	leng += 1 + strlen(str);
	while (!interrupt) {
		if (Pread(Pr, (long)ap, (char *)&arg, sizeof(char *))
		     != sizeof(char *)) {
			(void) printf("\n%s\t*** Bad argument list? ***\n",
				pname);
			return;
		}
		ap += sizeof(char *);
		if (arg == (char *)NULL)
			break;
		string = fetchstring((long)arg, 400);
		if (string == NULL) {
			(void) sprintf(badaddr, "BadAddress:0x%.8X", arg);
			string = badaddr;
		}
		if ((leng += (int)strlen(string)) < 71) {
			(void) fputc(' ', stdout);
			leng++;
		}
		else {
			(void) fputc('\n', stdout);
			leng = 0;
			if (pname[0])
				(void) fputs(pname, stdout);
			(void) fputs("  ", stdout);
			leng += 2 + strlen(string);
		}
		(void) fputs(string, stdout);
	}
	(void) fputc('\n', stdout);
}

/* display contents of read() or write() buffer */
void
showbuffer(Pr, offset, count)
	register process_t *Pr;
	long offset;
	int count;
{
	char buffer[320];
	int nbytes;
	register char * buf;
	register int n;

	while (count > 0 && !interrupt) {
		nbytes = (count < sizeof(buffer))? count : sizeof(buffer);
		if ((nbytes = Pread(Pr, offset, buffer, nbytes)) <= 0)
			break;
		count -= nbytes;
		offset += nbytes;
		buf = buffer;
		while (nbytes > 0 && !interrupt) {
			char obuf[65];

			n = (nbytes < 32)? nbytes : 32;
			showbytes(buf, n, obuf);

			if (pname[0])
				(void) fputs(pname, stdout);
			(void) fputs("  ", stdout);
			(void) fputs(obuf, stdout);
			(void) fputc('\n', stdout);
			nbytes -= n;
			buf += n;
		}
	}
}

void
showbytes(buf, n, obuf)
	register CONST char * buf;
	register int n;
	register char * obuf;
{
	register int c;

	while (--n >= 0) {
		register int c1 = '\\';
		register int c2;

		switch (c = (*buf++ & 0xff)) {
		case '\0':
			c2 = '0';
			break;
		case '\b':
			c2 = 'b';
			break;
		case '\t':
			c2 = 't';
			break;
		case '\n':
			c2 = 'n';
			break;
		case '\v':
			c2 = 'v';
			break;
		case '\f':
			c2 = 'f';
			break;
		case '\r':
			c2 = 'r';
			break;
		default:
			if (isprint(c)) {
				c1 = ' ';
				c2 = c;
			} else {
				c1 = c>>4;
				c1 += (c1 < 10)? '0' : 'A'-10;
				c2 = c&0xf;
				c2 += (c2 < 10)? '0' : 'A'-10;
			}
			break;
		}
		*obuf++ = c1;
		*obuf++ = c2;
	}

	*obuf = '\0';
}
