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

#ident	"@(#)kern-os:local.c	1.3"

/* XENIX Support */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/immu.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/errno.h"


/*      local.c - Custom Local System Call(s)
 *
 *      The 'clocal' system call is processed here.
 *      This system call is used for all additions to the kernel
 *      made by OEMs or source licensees.  Programs making use of
 *      clocal calls will not be portable to other versions of the OS.
 *
 *      Microsoft's policy is to minimize the incompatibilities between
 *      different versions of the OS, whether they be versions for different
 *      CPUs, different OEMS, or whatever.  For this reason, every effort
 *      is made to satisfy a customer's unique requirements in a fashion
 *      cross compatible with other systems.  Customers requiring
 *      some extension are requested to consult with Microsoft to see
 *      if their needs can be met by some generally acceptable system
 *      extension, which would then be included in future general releases.
 *
 *      This system call is intended for features which cannot be
 *      made a part of standard OS.  Microsoft feels that this is
 *      acceptable only for features that in some way depend upon an OEM's
 *      special hardware or proprietary products, and thus cannot in
 *      any case be made portable.
 *
 *
 *      USE:
 *
 *      The clocal call always takes 5 arguments.  One of these arguments
 *      has the site(OEM) designation and the sub-function.  This was
 *      done so that the use of a clocal function on a machine which
 *      does not implement it (or implements some other set of clocal
 *      functions) can produce a meaningful diagnostic.  The individual
 *      subfunctions make use of the other 4 arguments.
 *
 *	Each system call has its user entry point through a similarly named
 *	function in /lib/libc.a.   This function is usually written in 
 *	assembler and sets up the argument passing sequence to the system call.
 *      The argument passing sequence is machine dependent.  
 *
 *	NOTE:   At present in this distribution clocal calls are not available
 *		To create your clocal calls remove the dummy clocal, and
 *		replace it with a real clocal.
 */

extern int nosys();
#ifdef TEST_CLOCAL
int	tclocal();
#endif /* TEST_CLOCAL*/

struct sysent clentry[] = {
	4, 0, nosys,	/* 0 = template only */
#ifdef TEST_CLOCAL
	4, 0, tclocal,	/* 1 = test routine for clocal */
#endif /* TEST_CLOCAL */
};

/* number of clocal subfunctions */	
int nclentry = sizeof(clentry)/sizeof(struct sysent);


#ifndef NODEBUGGER
char *clenames[] = {            /* names for CLOCAL debug printout */
	"=ERROR=",              /* 0 */
};
#endif /* NODEBUGGER */


/* 
 *  Clocal system call template.
 */

clocal(uap, rvp)
	int *uap;
	rval_t *rvp;
{
	register int subfunc;
	register struct sysent *callp;


	/* args to clocal are already in uap; first arg is subfunction */
	subfunc = *uap++; 
	if (subfunc >= nclentry) 
		return EINVAL;
	callp = &clentry[subfunc]; /* real clocal system call */
	return (*callp->sy_call)(uap, rvp);	/* do the system call */
}

#ifdef TEST_CLOCAL 
#define NARGS 4
struct tclocala {
	int args[NARGS];
};

tclocal(uap, rvp)
	struct tclocala *uap;
	rval_t *rvp;
{
	register int i;

	printf("tclocal (clocal test): args are 0x%x, 0x%x, 0x%x, and 0x%x\n",
			uap->args[0], uap->args[1], uap->args[2], uap->args[3]);
	rvp->r_val1 = 0;
	for (i = 0; i < NARGS; i++)
		rvp->r_val1 +=  uap->args[i];
	return 0;
}
#endif /* TEST_CLOCAL */
/* End XENIX Support */
