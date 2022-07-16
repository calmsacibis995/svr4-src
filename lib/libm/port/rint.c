/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/rint.c	1.1"
/*LINTLIBRARY*/

/* RINT(X)
 * rint(x) returns the integer closest to x as a double precision
 * value; the return value is determined based on the rounding mode.
 * if round to nearest and the difference between the unrounded
 * operand and the result is exactly 0.5, the result will be even
 * Infinities, 0 and NaNs are returned as is.
 */

#include <math.h>
#include <values.h>

#define P754_NOFAULT 1	/* avoid generating extra code */
#include <ieeefp.h>
#include "fpparts.h"


double
rint(x)
double x;
{
	double y;
	register double z, sign = 1.0;
	double one = 1.0, three = 3.0;
	extern fp_rnd _fpgetround();
	fp_rnd	r;

#if _IEEE /* on IEEE machines, check for inf, NAN */
	if (EXPONENT(x) == MAXEXP)
		return(x); /* inf or nan */
#endif
	z = modf(x, &y);
	if (x == y) 	/* integer */
		return (y);
	one /= three;  /* non-integral input - 
			* raise inexact exception
			*/
	if (z < 0) {
		z = -z;
		sign = -sign;
	}
	r = _fpgetround(); /* rounding mode */
	switch(r) {
	case FP_RN :  /* to nearest*/
		if (z > 0.5)
			return (y + sign);
		if (z == 0.5)
			if ((modf(y/2.0,&x)) == 0)
			/* even */
				return (y);
			else return (y + sign);
		else return (y);
	case FP_RP : /* toward +inf */
		if (sign == 1.0)
			return (y + sign);
		else return (y);
	case FP_RM : /*toward -inf */
		if (sign == 1.0)
			return (y);
		else return (y + sign);
	case FP_RZ : /* toward 0 */
		return (y);
	}
/*NOTREACHED*/
}
