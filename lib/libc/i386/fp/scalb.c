/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:fp/scalb.c	1.4.2.1"
/*LINTLIBRARY

/* SCALB(X,N)
 * return x * 2**N without computing 2**N - this is the standard
 * C library ldexp() routine except that signaling NANs generate
 * invalid op exception - errno = EDOM
 */

#ifdef __STDC__
	#pragma weak scalb = _scalb
#endif
#include "synonyms.h"
#include <values.h>
#include <math.h>
#include <errno.h>
#include "fpparts.h"
#include <limits.h>
#if	_IEEE
#include <nan.h>
#endif

double scalb(x,n)
double	x, n;
{
#if _IEEE
	if ((EXPONENT(x) == MAXEXP) && !QNANBIT(x) && (HIFRACTION(x)
		|| LOFRACTION(x)) ) {
		errno = EDOM;
		return (x + 1.0); /* signaling NaN - raise exception */
	}
#endif

	if ((n >= (double)INT_MAX) || (n <= (double)INT_MIN)) 
	{
		/* over or underflow	*/
		errno = ERANGE;

		/* lim n -> -Inf of x * 2**n = 0 		*/
		if(n < 0.0) return(0.0); 	/* underflow	*/

		else { 	
#if _IEEE
			/* 0.0 * 2**+-Inf = NaN				*/
			if ((x == 0.0) && !QNANBIT(x)) {
				/* fake up a NaN			*/
				HIQNAN(x);
				LOQNAN(x);
				/* ensure the NaN is positive		*/
				((dnan *)&(n))->nan_parts.sign = 0x0;
				return(x);	/* returns a signaling NaN */
			}
#endif
			/* lim n -> Inf of x * 2**n = -Inf or +Inf 	*/
			if (_lib_version == c_issue_4)
				return(x > 0.0 ? HUGE : -HUGE);
			else
				return(x > 0.0 ? HUGE_VAL : -HUGE_VAL);
		}
	}

	/* 0.0 * 2**n = x or x * 2**0 = x * 1 = x	*/
	else if ((x == 0.0) || (n == 0.0)) return x;

	return(ldexp(x, (int)n));
}
