/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:signal.c	1.1"

#include "vars.h"
#include <signal.h>
#include <errno.h>

struct handler {
	unsigned short cs;      /* cs of signal handler */
	unsigned short ip;      /* ip of signal handler */
} handler[ MAXSIG ];

	/* Signal safety net variables, see comments in syscall.c */
int sigchange = 0, sigreset, chgsigno, sigcs, sigip;

Signal( SIG, IP, CS )
	int SIG, IP, CS;
{
	unsigned int retval;
	unsigned int csip;
	unsigned int sigarg;

	int sigs_go_here();

	if ( SIG >= MAXSIG ) {
		errno = EINVAL;
		return -1;
	}

	if ( ! Ltext ) {
		if ( IP > 1 )
			CS = 0x3f;
		else
			CS = 0;
	}

	retval = MAKEPTR(handler[SIG].cs, handler[SIG].ip);
	csip = MAKEPTR(CS, IP);

	if ( csip > 1 )
		sigarg = (int)sigs_go_here;
	else
		sigarg = csip;

	if ( (int)signal( SIG, (void(*)(int))sigarg ) >= 0 ) {
		handler[SIG].cs = CS;
		handler[SIG].ip = IP;
	} else {
		retval = -1;
	}

	/*
	 * Check to see if we caught this signal during processing.  If
	 * so, we have to reset the signal value cause it may have reverted
	 * to SIG_DFL.  Don't bother if the call failed in the first place.
	 */
	if (sigreset && retval != -1) {
		signal( SIG, (void(*)(int))sigarg);
	}
	sigreset = 0;
	sigchange = 0;

	return retval;
}

/*
 * Initialize the signal handler table.  Called at exec time.  Although
 * signals that have been set to be caught revert to SIG_DFL upon exec,
 * those that are SIG_IGN'ed do not.  Notice that this code contains the
 * usual signal window--if a process is execed and is not set to ignore
 * a particular signal, this code will create a window in which the signal
 * will be ignored.
 */
InitSignals()
{
		/* signals to check for SIG_IGN.  SIGKILL is not present */
	static int siglist[] = {
		SIGHUP,		/* hangup */
		SIGINT,		/* interrupt (rubout) */
		SIGQUIT,	/* quit (ASCII FS) */
		SIGILL,		/* illegal instruction (not reset when caught)*/
		SIGTRAP,	/* trace trap (not reset when caught) */
		SIGIOT,		/* IOT instruction */
		SIGABRT, 	/* used by abort, replace SIGIOT in future */
		SIGEMT,		/* EMT instruction */
		SIGFPE,		/* floating point exception */
		SIGBUS,		/* bus error */
		SIGSEGV,	/* segmentation violation */
		SIGSYS,		/* bad argument to system call */
		SIGPIPE,	/* write on a pipe with no one to read it */
		SIGALRM,	/* alarm clock */
		SIGTERM,	/* software termination signal from kill */
		SIGUSR1,	/* user defined signal 1 */
		SIGUSR2,	/* user defined signal 2 */
		SIGCLD,		/* death of a child */
		SIGPWR		/* power-fail restart */
	};
	int i;

	for (i=0; i < sizeof(siglist)/sizeof(int); i++) {
		if (signal(siglist[i], SIG_IGN) == SIG_IGN) {
#ifdef TRACE
			if (systrace) {
				fprintf( dbgfd, "ignoring signal %d at exec\n",siglist[i]);
			}
#endif
			handler[siglist[i]].ip = (int)SIG_IGN;
		} else {
			signal(siglist[i], SIG_DFL);
		}
	}
}

/*
 * function called when we get a signal
 */
sigs_go_here( signo )
{
	long csip;

#ifdef TRACE
	if (systrace) {
		fprintf( dbgfd, "%05d SIG %d\n", getpid(), signo);
		dump(32, &signo, &signo);
	}
#endif
		/* are we in the middle of changing handling for this sig? */
	if (!sigchange || signo != chgsigno) {
			/* normal signal processing */
		csip = MAKEPTR(handler[signo].cs, handler[signo].ip);
		switch ( signo ) {
		case 4:
		case 5:
		case 19:
			break;
		default:
			handler[signo].ip = 0;
			handler[signo].cs = 0;
			break;
		}
	} else {
		/*
		 * Must deliver signal as specified by the new signal
		 * parameters.  (See the comments in syscall.c).  These
		 * values are the ones that the Xenix runtime thinks are
		 * now in effect.
		 */
		csip = MAKEPTR(sigcs, sigip);
		sigreset++;	/* force a second try at the signal() */

		switch(csip) {
		case 1:	/* SIG_IGN, but cc doesn't think it's a constant! */
			/* ignore the signal */
			return;

		case 0:	/* SIG_DFL, but ditto above */
			/*
			 * Be certain that we get the default signal
			 * handling behavior from the system, then force
			 * the signal to be sent again.
			 */
			signal(signo, SIG_DFL);
			kill(getpid(), signo);
			break;

		default:
			/* just deliver the signal */
			break;
		}
	}

	sendsig( csip, signo );
#ifdef TRACE
	if (systrace) {
		fprintf(dbgfd, "%05d SIGRET from signal %d\n", getpid(), signo);
	}
#endif
}

Siginit() {
        int i; long l;
        for ( i = 1; i < MAXSIG; i++ ) {
                l = (long)signal( i, SIG_IGN );
                handler[i].ip = l;
                handler[i].cs = 0;
                if ( l == 0 )
                        signal( i, SIG_DFL );
        }
}
