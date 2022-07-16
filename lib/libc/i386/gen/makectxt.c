/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:gen/makectxt.c	1.7"

#ifdef __STDC__
	#pragma weak makecontext = _makecontext
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/user.h>
#include <sys/ucontext.h>

void
#ifdef	__STDC__

makecontext(ucontext_t *ucp, void (*func)(), int argc, ...)

#else

makecontext(ucp, func, argc)
ucontext_t *ucp; void (*func)(); int argc; 

#endif
{
	int *sp;
	int *argp;
	static void set_old_context();

	ucp->uc_mcontext.gregs[ EIP ] = (ulong)func;

	sp = (int *)(ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
	*--sp = (int)(ucp->uc_link);


	argp = ((int *)&argc) + argc;
	while (argc-- > 0)
		*--sp = *argp--;
	
	*--sp = (int)set_old_context;		/* return address */

	ucp->uc_mcontext.gregs[ UESP ] = (ulong)sp;
}


static void
set_old_context()
{
	ucontext_t uc;
	int *sp;
	int __getcontext();
	void setcontext();

	uc.uc_flags = UC_ALL;
	(void) __getcontext(&uc);
	sp = ((int *)(uc.uc_stack.ss_sp + uc.uc_stack.ss_size)) - 1;
	setcontext(*(ucontext_t **)sp);
	/* NOTREACHED */
}
