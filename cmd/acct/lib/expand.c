/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:lib/expand.c	1.7.2.2"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/acct.h>

#ifdef uts
float
#else
time_t
#endif
expand(ct)
register comp_t ct;
{
	register e;
#ifdef uts
	float f;
#else
	register long f;
#endif
	e = (ct >> 13) & 07;
	f = ct & 017777;

	while (e-- > 0) 
#ifdef uts
		f *= 8.0;		/* can't shift a float */
#else
		f <<=3;
#endif

	return f;
}
