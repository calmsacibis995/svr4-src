/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/sigfpe.c	1.3.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/* Swap handler for SIGFPE codes.	 */

#include <errno.h>
#include <signal.h>
#include <fp.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <sys/siginfo.h>

#ifndef FPE_INTDIV
#define     FPE_INTDIV     1	/* integer divide by zero */
#endif
#ifndef FPE_INTOVF
#define     FPE_INTOVF     2	/* integer divide by zero */
#endif
#ifndef FPE_FLTINV
#define     FPE_FLTINV     3	/* [floating operand error] */
                               	/* [floating Not-A-Number] */
#endif
#ifndef FPE_FLTRES
#define     FPE_FLTRES     4	/* [floating inexact result] */
#endif
#ifndef FPE_FLTDIV
#define     FPE_FLTDIV     5	/* [floating divide by zero] */
#endif
#ifndef FPE_FLTUND
#define     FPE_FLTUND     6	/* [floating underflow] */
#endif
#ifndef FPE_FLTOVF
#define     FPE_FLTOVF     7	/* [floating overflow] */
#endif

#define N_SIGFPE_CODE 8

/* Array of SIGFPE codes. */

static sigfpe_code_type sigfpe_codes[N_SIGFPE_CODE] = {
						       FPE_INTDIV,
						       FPE_INTOVF,
						       FPE_FLTINV,
						       FPE_FLTRES,
						       FPE_FLTDIV,
						       FPE_FLTUND,
						       FPE_FLTOVF,
						       0};

/* Array of handlers. */

static sigfpe_handler_type sigfpe_handlers[N_SIGFPE_CODE];

static int      _sigfpe_master_enabled;
/* Originally zero, set to 1 by _enable_sigfpe_master. */

#ifndef BADSIG
#define BADSIG		(void (*)())-1
#endif

void
_sigfpe_master(sig, siginfo, ucontext)
	int             sig;
	siginfo_t	*siginfo;
	ucontext_t	*ucontext;
{
	int             i;
	int		code;
	enum fp_exception_type exception;

	code = siginfo->si_code;
	for (i = 0; (i < N_SIGFPE_CODE) && (code != sigfpe_codes[i]); i++) ;
	/* Find index of handler. */
	if (i >= N_SIGFPE_CODE)
		i = N_SIGFPE_CODE - 1;
	switch ((int) sigfpe_handlers[i]) {
	case ((int) (SIGFPE_DEFAULT)):
		switch (code) {
		case FPE_FLTINV:
			exception = fp_invalid;
			goto ieee;
		case FPE_FLTRES:
			exception = fp_inexact;
			goto ieee;
		case FPE_FLTDIV:
			exception = fp_division;
			goto ieee;
		case FPE_FLTUND:
			exception = fp_underflow;
			goto ieee;
		case FPE_FLTOVF:
			exception = fp_overflow;
			goto ieee;
		default:	/* The common default treatment is to abort. */
			break;
		}
	case ((int) (SIGFPE_ABORT)):
		abort();
	case ((int) (SIGFPE_IGNORE)):
		return;
	default:		/* User-defined not SIGFPE_DEFAULT or
				 * SIGFPE_ABORT. */
		(sigfpe_handlers[i]) (sig, siginfo, ucontext);
		return;
	}
ieee:
	switch ((int) ieee_handlers[(int) exception]) {
	case ((int) (SIGFPE_DEFAULT)):	/* Error condition but ignore it. */
	case ((int) (SIGFPE_IGNORE)):	/* Error condition but ignore it. */
		return;
	case ((int) (SIGFPE_ABORT)):
		abort();
	default:
		(ieee_handlers[(int) exception]) (sig, siginfo, ucontext);
		return;
	}
}

int
_enable_sigfpe_master()
{
	/* Enable the sigfpe master handler always.	 */
	struct sigaction  newsigact, oldsigact;

	newsigact.sa_handler = _sigfpe_master;
	newsigact.sa_mask.word[0] = 0;
	newsigact.sa_mask.word[1] = 0;
	newsigact.sa_mask.word[2] = 0;
	newsigact.sa_mask.word[3] = 0;
	newsigact.sa_flags = SA_SIGINFO;	/* enhanced handler */
	_sigfpe_master_enabled = 1;
	return sigaction(SIGFPE, &newsigact, &oldsigact);
}

int
_test_sigfpe_master()
{
	/*
	 * Enable the sigfpe master handler if it's never been enabled
	 * before. 
	 */

	if (_sigfpe_master_enabled == 0)
		return _enable_sigfpe_master();
	else
		return _sigfpe_master_enabled;
}

sigfpe_handler_type
sigfpe(code, hdl)
	sigfpe_code_type code;
	sigfpe_handler_type hdl;
{
	sigfpe_handler_type oldhdl;
	int             i;

	_test_sigfpe_master();
	for (i = 0; (i < N_SIGFPE_CODE) && (code != sigfpe_codes[i]); i++) ;
	/* Find index of handler. */
	if (i >= N_SIGFPE_CODE) {
		errno = EINVAL;
		return (sigfpe_handler_type) BADSIG;/* Not 0 or SIGFPE code */
	}
	oldhdl = sigfpe_handlers[i];
	sigfpe_handlers[i] = hdl;
	return oldhdl;
}
