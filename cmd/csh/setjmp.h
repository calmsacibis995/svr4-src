/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:setjmp.h	1.1.3.1"

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

/*
 * 4.3BSD setjmp compatibility header
 *
 * 4.3BSD setjmp/longjmp is equivalent to SVR4 sigsetjmp/siglongjmp -
 * 4.3BSD _setjmp/_longjmp is equivalent to SVR4 setjmp/longjmp
 */

#ifndef _SETJMP

#include <sys/types.h>
#include <sys/signal.h>
#ifdef i386
#include <sys/tss.h>
#include <sys/user.h>
#else
#include <sys/psw.h>
#include <sys/pcb.h>
#include <sys/mau.h>
#endif /* i386 */
#include <sys/ucontext.h>

#define _JBLEN		((sizeof(ucontext_t)+sizeof(int)-1)/sizeof(int))
#define _SIGJBLEN	_JBLEN

typedef int jmp_buf[_JBLEN];

#define sigjmp_buf 	jmp_buf

#if defined(__STDC__)

#if __STDC__ == 0	/* non-ANSI standard compilation */
extern int	_setjmp(jmp_buf);
extern void	_longjmp(jmp_buf, int);
extern int	setjmp(jmp_buf);
extern void	longjmp(jmp_buf, int);
extern int	sigsetjmp(sigjmp_buf, int);
extern void	siglongjmp(sigjmp_buf, int);

#else
extern int	_setjmp();
extern void	_longjmp();
extern int	setjmp();
extern void	longjmp();
extern int	sigsetjmp();
extern void	siglongjmp();
#endif  /* __STDC__  == 0 */

#endif	/* __STDC__ */

#endif  /* _SETJMP */
