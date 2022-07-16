/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/sigsetops.c	1.5"

/*
 * POSIX signal manipulation functions. 
 */
#ifdef __STDC__
	#pragma weak sigfillset = _sigfillset
	#pragma weak sigemptyset = _sigemptyset
	#pragma weak sigaddset = _sigaddset
	#pragma weak sigdelset = _sigdelset
	#pragma weak sigismember = _sigismember
#endif
#include "synonyms.h"
#include <stdio.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/signal.h>

#define SIGSETSIZE	4
#define MAXBITNO (NBPW*8)

extern int errno;
static sigset_t sigs;
static sigsinit;

#define sigword(n) ((n-1)/MAXBITNO)
#define bitmask(n) (1L<<((n-1)%MAXBITNO))

static
sigvalid(sig)
{
	if (sig <= 0 || sig > (MAXBITNO * SIGSETSIZE))
		return 0;

	if (!sigsinit) {
		__sigfillset(&sigs);
		sigsinit++;
	}

	return (sigs.sigbits[sigword(sig)] & bitmask(sig)) != 0;
}

int
sigfillset(set)
sigset_t *set;
{
	if (!sigsinit) {
		__sigfillset(&sigs);
		sigsinit++;
	}

	*set = sigs;
	return 0;
}

int
sigemptyset(set)
sigset_t *set;
{
	set->sigbits[0] = 0;
	set->sigbits[1] = 0;
	set->sigbits[2] = 0;
	set->sigbits[3] = 0;
	return 0;
}

int 
sigaddset(set,sig)
sigset_t *set;
register int sig;
{
	if (!sigvalid(sig)) {
		errno = EINVAL;
		return -1;
	}
	set->sigbits[sigword(sig)] |= bitmask(sig);
	return 0;
}

int 
sigdelset(set,sig)
sigset_t *set;
register int sig;
{
	if (!sigvalid(sig)) {
		errno = EINVAL;
		return -1;
	}
	set->sigbits[sigword(sig)] &= ~bitmask(sig);
	return(0);
}

int
sigismember(set,sig)
const sigset_t *set;
register int sig;
{
	if (!sigvalid(sig)) {
		errno = EINVAL;
		return -1;
	}
	return (set->sigbits[sigword(sig)] & bitmask(sig)) != 0;
}
