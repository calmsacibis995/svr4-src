/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:Signal.c	1.1"

#include "vars.h"
#include <sys/signal.h>         /* to get MAXSIG */
#include <stdio.h>
#include <errno.h>

struct handler {
	ushort cs;      /* cs of signal handler */
	ushort ip;      /* ip of signal handler */
} handler[ MAXSIG ];

Signal( args )
	ushort * args;
{
#define SIG     (args[0])
#define CS      (args[2])
#define IP      (args[1])

	unsigned int retval;
	unsigned int csip;
	unsigned int sigarg;

	int sigs_go_here();

dprintf( "Signal: SIG=%x, CS=%x, IP=%x\n", SIG, CS, IP );
fflush( stdout );

	if ( SIG >= MAXSIG ) {
		errno = EINVAL;
		return -1;
	}

/***** this code must be extended to cope with sigset et al, which are
       subcommands of signal()
 *****/
	retval  = handler[SIG].cs << 16;
	retval |= handler[SIG].ip;

	csip = (CS << 16) | IP;
	if ( csip > 1 )
		sigarg = (int)sigs_go_here;
	else
		sigarg = csip;

	if ( signal( SIG, sigarg ) >= 0 ) {
		handler[SIG].cs = CS;
		handler[SIG].ip = IP;
	} else
		retval = -1;

dprintf( "returning %x\n", retval ); fflush( stdout );
	return retval;
}

/*
 * function called when we get a signal
 */
sigs_go_here( signo )
{
	long csip;

	csip  = handler[signo].cs << 16;
	csip |= handler[signo].ip;
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
dprintf( "sigs_go_here: signal %d for %x\n", signo, csip ); fflush( stdout );
	sendsig( csip, signo );
dprintf( "sendsig returns\n" ); fflush( stdout );
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
