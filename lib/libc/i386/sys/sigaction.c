/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:sys/sigaction.c	1.5"

#ifdef __STDC__ 
	#pragma weak sigaction = _sigaction
#endif
#include "synonyms.h"
#include <signal.h>
#include <errno.h>
#include <siginfo.h>
#include <sys/user.h>
#include <ucontext.h>

extern void (*_siguhandler[])();

void
_sigacthandler(sig, sip, uap)
	int sig;
	siginfo_t *sip;
	ucontext_t *uap;
{
	(*_siguhandler[sig])(sig, sip, uap);
	setcontext(uap);
}
	
sigaction(sig, nact, oact)
	int sig;
	const struct sigaction *nact;
	struct sigaction *oact;
{
	struct sigaction tact;
	register struct sigaction *tactp;
	void (*ohandler)();

	if (sig <= 0 || sig >= NSIG) {
		errno = EINVAL;
		return -1;
	}

	ohandler = _siguhandler[sig];

	if (tactp = (struct sigaction *)nact) {
		tact = *nact;
		tactp = &tact;
		_siguhandler[sig] = tactp->sa_handler;
		if (tactp->sa_handler != SIG_DFL && tactp->sa_handler != SIG_IGN)
			tactp->sa_handler = _sigacthandler;
	}

	if (__sigaction(sig, tactp, oact) == -1) {
		_siguhandler[sig] = ohandler;
		return -1;
	}

	if (oact && oact->sa_handler == _sigacthandler)
		oact->sa_handler = ohandler;
	
	return 0;
}
