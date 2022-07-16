/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:fp/finite.c	1.3"
/*LINTLIBRARY*/

/*	IEEE recommended functions */

#ifdef __STDC__
	#pragma weak finite = _finite
	#pragma weak fpclass = _fpclass
	#pragma weak unordered = _unordered
#endif

#include <values.h>
#include "fpparts.h"
#include "synonyms.h"

#define P754_NOFAULT 1		/* avoid generating extra code */
#include <ieeefp.h>

/* FINITE(X)
 * finite(x) returns 1 if x > -inf and x < +inf and 0 otherwise 
 * NaN returns 0
 */

int finite(x)
double	x;
{	
	return((EXPONENT(x) != MAXEXP));
}

/* UNORDERED(x,y)
 * unordered(x,y) returns 1 if x is unordered with y, otherwise
 * it returns 0; x is unordered with y if either x or y is NAN
 */

int unordered(x,y)
double	x,y;
{	
	if ((EXPONENT(x) == MAXEXP) && (HIFRACTION(x) || LOFRACTION(x)))
		return 1;
	if ((EXPONENT(y) == MAXEXP) && (HIFRACTION(y) || LOFRACTION(y)))
		return 1;
	return 0;
}	

/* FPCLASS(X)
 * fpclass(x) returns the floating point class x belongs to 
 */

fpclass_t	fpclass(x)
double	x;
{	
	register int	sign, exp;

	exp = EXPONENT(x);
	sign = SIGNBIT(x);
	if (exp == 0) { /* de-normal or zero */
		if (HIFRACTION(x) || LOFRACTION(x)) /* de-normal */
			return(sign ? FP_NDENORM : FP_PDENORM);
		else
			return(sign ? FP_NZERO : FP_PZERO);
	}
	if (exp == MAXEXP) { /* infinity or NaN */
		if ((HIFRACTION(x) == 0) && (LOFRACTION(x) == 0)) /* infinity*/
			return(sign ? FP_NINF : FP_PINF);
		else
			if (QNANBIT(x))
			/* hi-bit of mantissa set - quiet nan */
				return(FP_QNAN);
			else	return(FP_SNAN);
	}
	/* if we reach here we have non-zero normalized number */
	return(sign ? FP_NNORM : FP_PNORM);
}
