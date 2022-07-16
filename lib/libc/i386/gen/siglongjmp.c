/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:gen/siglongjmp.c	1.1"

#ifdef __STDC__
	#pragma weak siglongjmp = _siglongjmp
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/ucontext.h>

#include <setjmp.h>

void 
siglongjmp(env,val)
sigjmp_buf env;
int val;
{
	register ucontext_t *ucp = (ucontext_t *)env;
	if (val)
		ucp->uc_mcontext.gregs[ EAX ] = val;
	setcontext(ucp);
}
