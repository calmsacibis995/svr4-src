/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libc-i386:gen/getctxt.c	1.5"

#ifdef __STDC__
	#pragma weak getcontext = _getcontext
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/user.h>
#include <sys/ucontext.h>

asm int *
getfp()
{
#ifdef	i386
	leal	0(%ebp),%eax
#else	/* 3b2 */
	MOVW	%fp,%r0
#endif
}

asm int  *
getap()
{
#ifdef	i386
	leal	8(%ebp),%eax
#else	/* 3b2 */
	MOVW	%ap,%r0
#endif
}

int
getcontext(ucp)
ucontext_t *ucp;
{
	int error;
	register greg_t *cpup;

	ucp->uc_flags = UC_ALL;
	if (error = __getcontext(ucp))
		return error;  
	cpup = (greg_t *)&ucp->uc_mcontext.gregs;
#ifdef	i386
	cpup[ EBP ] =  *((greg_t *)getfp()); /* get old ebp off stack */
	cpup[ EIP ] =  *((greg_t *)getfp()+1); /* get old eip off stack */
#else	/* 3b2 */
	cpup->fp = (int *)*(getfp()-7);		/* get old fp off stack */
	cpup->ap = (int *)*(getfp()-8);		/* get old ap off stack */
	cpup->pc = (caddr_t)*(getfp()-9);	/* get old pc off stack */
#endif
	cpup[ UESP ] =  (greg_t)getap();	/* get old esp off stack */

	return 0;
}
