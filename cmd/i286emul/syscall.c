/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:syscall.c	1.1"

#include <stdio.h>
#include "sysent.h"
#include "vars.h"
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

/* syscall() provides an interface from 286 system calls to the corresponding
 * 386 system calls.
 *
 *  At entry, the stack looks like this:
 *
 *  (one word per line)
 *
 *        ...
 *        arg2    |
 *        arg1    | 286 system call args
 *        arg0    |
 *        n       system call number
 *        --          Dword cs because of 386 call gate
 *        cs
 *        ip (hi)     Dword ip because of 386 call gate
 *        ip (lo)
 *        flags
 *        bp
 *        ds
 *        es      <-- esp
 */

#define FLAGS   3
#define N       8
#define ARG0    9

#define CARRY   1               /* carry bit in flags word */

jmp_buf bad_pointer;            /* for recovery from a bad pointer */

syscall(dummy)
	int dummy;              /* dummy arg for locating stack frame */
{
	short *ap2;             /* pointer to 286 system call args */
	int *ap3;               /* pointer to 386 system call args */
	int n;                  /* system call number */
	int a[NARGMAX];         /* args to be passed in 386 syscall */
	char *tp;               /* type pointer */
	int t;                  /* type */
	int rv;                 /* return value */
	extern int errno;       /* error number from system call */

	/* initalize arg pointers and system call number */
	ap2 = ((short *)&dummy) + ARG0;
	ap3 = a;

	/* get system call number and check it */
	n = ((ushort *)&dummy)[N];
	if (n < 0 || n >= NSYSENT) {
		emprintf( "illegal system call number = %d\n", n);
		exit(1);
	}

dprintf( "syscall %d\n", n ); fflush( stdout );
	/* convert the args and do system call */

	/* use specifications in table */
	for (tp = &sysent[n].types[0]; (t = *tp); tp++) {
		switch(t) {
		case INT:
dprintf( "...converted INT\n" ); fflush( stdout );
			*ap3++ = *ap2++;
			break;
		case UINT:
dprintf( "...converted UINT\n" ); fflush( stdout );
			*ap3++ = *(ushort *)ap2++;
			break;
		case PTR:
dprintf( "...converted PTR(%x --> %x)\n",*(int *)ap2, cvtptr(*(int *)ap2) );
fflush( stdout );
			*ap3++ = (int)cvtptr(*(int *)ap2);
			ap2 += 2;
			break;
		case LONG:
dprintf( "...converted LONG\n" ); fflush( stdout );
			*ap3++ = *(int *)ap2;
			ap2 += 2;
			break;
		case ZERO:           /* zero arg */
dprintf( "...converted ZERO\n" ); fflush( stdout );
			*ap3++ = 0;
			break;
		case SPECIAL:           /* special case processing */
dprintf( "...converted SPECIAL\n" ); fflush( stdout );
			*ap3 = (int)ap2;
			if ( setjmp( bad_pointer ) ) {
				errno = EFAULT;
				rv = -1;
				goto call_returns;
			}
			break;
		default:
			emprintf(
				"illegal arg type = %d for n = %d\n", t, n);
			exit(1);
		}
	}

	/* call the 386 system call routine */
dprintf( "...call 0x%x(0x%x,0x%x,0x%x,0x%x,0x%x)\n", sysent[n].routine,
		a[0], a[1], a[2], a[3], a[4] );
	rv = (*sysent[n].routine) (a[0],a[1],a[2],a[3],a[4],a[5]);
	/* if error, set carry flag and return errno */
call_returns:
	if (rv == -1) {
		((ushort *)&dummy)[FLAGS] |= CARRY;
		rv = errno;
	}
	else
		((ushort *)&dummy)[FLAGS] &= ~CARRY;

	fixesds( &dummy );

	return rv;
}


nosys()         /* no such system call */
{
	kill(getpid(), SIGSYS);
}

fixesds( rp )
	register ushort * rp;
{
	register int i;

	i = *rp++;                              /* fetch %es */
	if ( i ) {
		i = (i >> 3) & 0x1fff;          /* index */
		if ( dsegs[i].size == 0 )
			rp[-1] = 0;
	}
	i = *rp;                                /* fetch %ds */
	if ( i ) {
		i = (i >> 3) & 0x1fff;          /* index */
		if ( dsegs[i].size == 0 )
			*rp = 0;
	}
}
