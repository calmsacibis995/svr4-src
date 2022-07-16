/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/sleep.c	1.23"
/*      3.0 SID #       1.4     */
/*LINTLIBRARY*/

/*
 * Suspend the process for `sleep_tm' seconds - using alarm/pause
 * system calls.  If caller had an alarm already set to go off `n'
 * seconds from now, then Case 1: (sleep_tm >= n) sleep for n, and
 * cause the callers previously specified alarm interrupt routine
 * to be executed, then return the value (sleep_tm - n) to the caller
 * as the unslept amount of time, Case 2: (sleep_tm < n) sleep for
 * sleep_tm, after which, reset alarm to go off when it would have
 * anyway.  In case process is aroused during sleep by any caught
 * signal, then reset any prior alarm, as above, and return to the
 * caller the (unsigned) quantity of (requested) seconds unslept.
 *
 * For SVR4 sleep uses the new sigaction system call which restores
 * the proper flavor of the old SIGALRM signal handler, i.e. whether
 * it was set via signal() or sigset()
 */

#ifndef ABI
#ifdef __STDC__
	#pragma weak sleep = _sleep
#endif
#endif

#include <sys/utsname.h>
#undef uname
#include "synonyms.h"
#include <signal.h>

extern int uname();
extern int sigaction();
extern int sigprocmask();
extern int sigsuspend();
extern unsigned alarm();
static ver;

/* ARGSUSED */
static void
awake(sig)
	int sig;
{
}

unsigned
sleep(sleep_tm)
unsigned sleep_tm;
{
	int  alrm_flg;
	unsigned unslept, alrm_tm, left_ovr;

/* variable for the sigset version */
 	void (*alrm_sig)();

/* variables for the sigaction version */
	struct sigaction nact;
	struct sigaction oact;
	sigset_t alrm_mask;
	sigset_t nset;
	sigset_t oset;

	if(sleep_tm == 0)
		return(0);

	/* 
	 * check version on first time only
	 */

	if (ver == 0) {
		struct utsname uname_buf;
		if (uname(&uname_buf) > 0)
			ver = 2;
		else
			ver = 1;
	}

	alrm_tm = alarm(0);			/* prev. alarm time */
	if (ver == 1)				/* prev. alarm prog */
		alrm_sig = sigset(SIGALRM, awake);
	else {
		nact.sa_handler = awake;
		nact.sa_flags = 0;
		sigemptyset(&nact.sa_mask);
		sigaction(SIGALRM,&nact,&oact);
	}

	alrm_flg = 0;
	left_ovr = 0;

	if (alrm_tm != 0) {	/* skip all this ifno prev. alarm */
		if (alrm_tm > sleep_tm) {
			alrm_tm -= sleep_tm;
			++alrm_flg;
		} else {
			left_ovr = sleep_tm - alrm_tm;
			sleep_tm = alrm_tm;
			alrm_tm = 0;
			--alrm_flg;
			if (ver == 1)
				(void) sigset(SIGALRM,alrm_sig);
			else
				sigaction(SIGALRM,&oact,(struct sigaction *)0);
		}
	}

	if (ver == 1) {
		sighold(SIGALRM);
		(void) alarm(sleep_tm);
		sigpause(SIGALRM);
		unslept = alarm(0);
		if (alrm_flg >= 0)
			(void) sigset(SIGALRM, alrm_sig);
	} else {
		sigemptyset(&alrm_mask);
		sigaddset(&alrm_mask,SIGALRM);
		sigprocmask(SIG_BLOCK,&alrm_mask,&oset);
		nset = oset;
		sigdelset(&nset,SIGALRM);
		(void)alarm(sleep_tm);
		sigsuspend(&nset);
		unslept = alarm(0);
		if (!sigismember(&oset, SIGALRM))
			sigprocmask(SIG_UNBLOCK,&alrm_mask,(sigset_t *)0);
		if (alrm_flg >= 0)
			sigaction(SIGALRM,&oact,(struct sigaction *)0);
	}

	if(alrm_flg > 0 || (alrm_flg < 0 && unslept != 0))
		(void) alarm(alrm_tm + unslept);
	return(left_ovr + unslept);
}

