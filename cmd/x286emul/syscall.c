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

#ident	"@(#)x286emul:syscall.c	1.1"

#include "sysent.h"
#include "vars.h"
#include "h/syscall.h"
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

/*
 * syscall() provides an interface from 286 system calls to the corresponding
 * 386 system calls.
 */

jmp_buf bad_pointer;            /* for recovery from a bad pointer */

syscall(dummy)
		/* struct dummy locates stack frame args */
	struct { unsigned short n, argoff, dummy; } dummy;
{
	short *ap2;             /* pointer to 286 system call args */
	int *ap3;               /* pointer to 386 system call args */
	int n;                  /* system call number */
	int sub;		/* system call subfunction number */
	int a[NARGMAX];         /* args to be passed in 386 syscall */
	char *tp;               /* type pointer */
	int t;                  /* type */
	int rv;                 /* return value */
	extern int errno;       /* error number from system call */
	extern int nsyscalls;
	extern int nxsyscalls;
	extern int npsyscalls;
	struct sysent * table;
#if defined(TRACE) || defined(DEBUG)
	char ** names;
	extern char * SysCallNames[], * XsysCallNames[];
	extern char * V7sysCallNames[], * S3sysCallNames[];
	extern char * PsysCallNames[];
#endif

	/* initalize arg pointers and system call number */
	ap2 = (short *)(dummy.argoff + TheStack);
	ap3 = a;

	/*
	 * Hack for signals.  There is an inherent window in the Xenix 286
	 * signal system call handling.  When a process calls signal to
	 * SIG_IGN an active signal, there is approximately a 6 instruction
	 * window in the runtime after the signal delivery address has been
	 * updated and before kernel mode is achieved.  If a signal occurs
	 * during this time, the 286 process will attempt to deliver the
	 * signal to address 00:0001, which of course nets a memory fault.
	 * The emulator widens this window to several hundred instructions.
	 * This code allows us to catch such a signal and deliver it correctly
	 * (in combination with code in signal.c:sigs_go_here()) more often,
	 * but the window is still about 25 - 30 instructions wider.
	 */
	if (dummy.n == 48) {	/* signal is syscall 48 */
		extern int sigchange, sigreset, chgsigno, sigcs, sigip;

			/* save new values that must be used */
		chgsigno = ap2[0];
		sigip = ap2[1];
		sigcs = ap2[2];
		sigreset = 0;	/* reset sig-caught flag */
		sigchange++;	/* set signal-processing flag */
	}

		/* StackGrow is a nop under Xenix, but they do it a lot */
	if ( dummy.n == 0x0828 )
		return 0;

	/* get system call number and check it */
	n = dummy.n & 0xff;
	sub = (int) (dummy.n & 0xff00) >> 8;

	if ( !(X->x_renv & XE_V3)) {	/* make allowances for older x.outs */
		/*
		 * V7 system calls
		 */
		switch (n) {
		case OTFTIME:
			n = TFTIME;
			break;
		case OTCLOCAL:
			n = TCLOCAL;
			break;
		case OTCXENIX:
			if (sub > TRDCHK) { /* last V7 cxenix subcommand */
				return nosys(&dummy);
			}
			n = TCXENIX;
			break;
		case 38:	/* These are unused system call numbers
		case 39:	 * under V7 XENIX, so a V7 XENIX 286
		case 40:	 * program has no business asking for one.
		case 45:	 */
		case 49:
		case 50:
		case 55:
		case 57:
		case 58:
			return nosys(&dummy);
		}
	}

	/*
	 * Oh, what an evil hack!  System calls with more than 4 parameters
	 * pass them on the stack in small model, which puts them in a
	 * different place than the saved registers, where the normal ones
	 * pass their parameters.  This check corrects this problem for
	 * all of these miscreant system calls (e.g., msgrcv).  We correct
	 * this by adding the number of (2 byte) words (14) to the ap2 value
	 * to get past the sys call stack frame.
	 */
	if (!Ldata)
		switch(n) {
		case TCXENIX:
			switch(sub) {
			case TMSGRCV:
				ap2 += (Ltext ? 16 : 14);
				break;
			}
	}

	switch ( n ) {
	case TCXENIX:				/* cxenix system call */
		n = sub;
		table = Xsysent;
#if defined(TRACE) || defined(DEBUG)
		names = XsysCallNames;
#endif
		if ( n >= nxsyscalls ) {
			return nosys(&dummy);
		}
		break;
	case TPWBSYS:				/* pwb system call */
		n = sub;
		table = Psysent;
#if defined(TRACE) || defined(DEBUG)
		names = PsysCallNames;
#endif
		if ( n >= npsyscalls ) {
			return nosys(&dummy);
		}
		break;
	default:
		table = Sysent;
#if defined(TRACE) || defined(DEBUG)
		names = SysCallNames;
#endif
		break;
	}

	if (n < 0 || n >= NSYSENT) {
		return nosys(&dummy);
	}
	if ( n >= nsyscalls ) {
		return nosys(&dummy);
	}

#ifdef DEBUG
	fprintf( dbgfd,  "syscall %d\n", n ); fflush( stdout );
#endif
#ifdef TRACE
	if ( systrace )
	{
		fprintf( dbgfd,  "%05d CALLS: %s\n", getpid(), names[n] );
		dump( 32, ap2, 0L ); 
	}
#endif
	/* convert the args and do system call */

	/* use specifications in table */
	for (tp = &table[n].types[0]; (t = *tp); tp++) {
		int x;

		switch(t) {
		case INT:
#ifdef DEBUG
	fprintf( dbgfd,  "...converted INT\n" ); fflush( stdout );
#endif
			*ap3++ = *ap2++;
			break;
		case UINT:
#ifdef DEBUG
	fprintf( dbgfd,  "...converted UINT\n" ); fflush( stdout );
#endif
			*ap3++ = *(unsigned short *)ap2++;
			break;
		case TPTR:
			if (Ltext) goto long_ptr;
			x = *(unsigned short *)ap2++;
			*ap3++ = (!x) ? 0 :
				(int)cvtptr( x | (*((short *)&dummy+CS) << 16));
#ifdef TRACE
			if ( systrace )
				dump( 32, *(ap3-1), *(unsigned short *)(ap2-1) );
#endif
			break;
		case RDPTR:
			if ( !Ldata ) {
				x = *(unsigned short *)ap2++;
				*ap3++ = (!x) ? 0 :
					( x | (*((short *)&dummy+DS) << 16));
			} else {
				*ap3++ = (ap2[1] << 16) + ap2[0];
				ap2 += 2;
			}
			break;
		case PTR:
			if (!Ldata) {
				x = *(unsigned short *)ap2++;
				*ap3++ = (!x) ? 0 :
					(int)cvtptr( x | (*((short *)&dummy+DS) << 16));
#ifdef TRACE
				if ( systrace )
						dump( 32, *(ap3-1), *(unsigned short *)(ap2-1) );
#endif
				break;
			}
			/* FALL THRU */
		case LPTR:
	long_ptr:
#ifdef DEBUG
	fprintf( dbgfd,  "...converted PTR(%x --> %x)\n",*(int *)ap2, cvtptr(*(int *)ap2) );
	fflush( stdout );
#endif
			*ap3++ = (int)cvtptr(*(int *)ap2);
#ifdef TRACE
			if (systrace)
				dump( 32, *(ap3-1), *(unsigned int *)ap2 );
#endif
			ap2 += 2;
			break;
		case LONG:
#ifdef DEBUG
	fprintf( dbgfd,  "...converted LONG\n" ); fflush( stdout );
#endif
			*ap3++ = *(int *)ap2;
			ap2 += 2;
			break;
		case ZERO:           /* zero arg */
#ifdef DEBUG
	fprintf( dbgfd,  "...converted ZERO\n" ); fflush( stdout );
#endif
			*ap3++ = 0;
			break;
		case SPECIAL:           /* special case processing */
#ifdef DEBUG
	fprintf( dbgfd,  "...converted SPECIAL\n" ); fflush( stdout );
#endif
			*ap3 = (int)ap2;
			if ( setjmp( bad_pointer ) ) {
				errno = EFAULT;
				rv = -1;
				goto call_returns;
			}
			break;
		case STACKFRAME:	/* need to look at 286 regs */
			*ap3++ = (int)&dummy;
			break;
		default:
			emprintf( "illegal arg type = %d for n = %d\n", t, n);
			exit(1);
		}
	}

	/* call the 386 system call routine */
#ifdef DEBUG
	fprintf( dbgfd,  "...call 0x%x(0x%x,0x%x,0x%x,0x%x,0x%x)\n", table[n].routine,
		a[0], a[1], a[2], a[3], a[4] );
#endif
	if ( table[n].routine == 0 ) {
#if defined(TRACE) || defined(DEBUG)
		fprintf( dbgfd,  "%05d: unsupported %s system call %d, sub %d\n", getpid(), table == Sysent ? "unix" : "xenix", n, sub );
#endif
		errno = EINVAL;
		return -1;
	}
	rv = (*table[n].routine) (a[0],a[1],a[2],a[3],a[4],a[5]);
#ifdef TRACE
	if ( systrace )
		fprintf( dbgfd,  "Return %d (0x%x) errno is %d\n", rv, rv, errno );
#endif
	/* if error, set carry flag and return errno */
call_returns:
	if (rv == -1) {
		((unsigned short *)&dummy)[FLAGS] |= CARRY;
		rv = errno_map[errno];
	}
	else
		((unsigned short *)&dummy)[FLAGS] &= ~CARRY;

	fixesds( (short *)&dummy + ES );

	return rv;
}


nosys(stack)		/* no such system call */
char *stack;
{
#if defined(TRACE) || defined(DEBUG)
	if (systrace) {
		printf("Nosys:  stack for `%s'--\n", Pgmname);
		dump(64, stack, stack);
	}
#endif
	errno = EINVAL;
	kill(getpid(), SIGSYS);
	return -1;
}

fixesds( rp )
	register unsigned short * rp;
{
	register int i;

	i = *rp++;                              /* fetch %es */
	if ( i ) {
		i = (i >> 3) & 0x1fff;          /* index */
		if ( Dsegs[i].lsize == 0 )
			rp[-1] = 0;
	}
	i = *rp;                                /* fetch %ds */
	if ( i ) {
		i = (i >> 3) & 0x1fff;          /* index */
		if ( Dsegs[i].lsize == 0 )
			*rp = 0;
	}
}
