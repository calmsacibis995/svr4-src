/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/expf.c	1.3"
/*LINTLIBRARY*/
/* 
 */

/* expf(x)
 * return the single precision exponential of x
 * Method:
 *	1. Argument Reduction: given the input x, find r and integer k such 
 *	   that
 *	                   x = k*ln2 + r,  |r| <= 0.5*ln2 .  
 *	   r will be represented as r := z+c for better accuracy.
 *
 *	2. Compute expm1(r)=exp(r)-1 by 
 *
 *			expm1(r=z+c) := z + exp__E(z,r)
 *
 *	3. exp(x) = 2^k * ( expm1(r) + 1 ).
 */

#include <math.h>
#include <errno.h>
#include <values.h>
#include "fpparts.h"

static float
ln2hi = 6.9314575195e-1,
ln2lo = 1.4286067653e-6,
invln2 =  1.442695040888963387;

#define SMALL	1.0E-8

/* coefficients for exp__E() */
static float 
p1 = 1.3887401997267371720E-2,
p2 = 3.3044019718331897649E-5, 
q1 = 1.1110813732786649355E-1,
q2 = 9.9176615021572857300E-4;

float expf(register float x)
{
	register float z, z1, c ;
	register float hi, lo;
	float  p3, q;
	float xp, xh, w;
	int k, n;
	struct exception exc;
	static float zero = 0.0,
		     one = 1.0,
		     half = 0.5;

	if (x <= LN_MINFLOAT) {
		if (x == LN_MINFLOAT) /* protect against roundoff */
			return (MINFLOAT); /* causing ldexp to underflow */
		exc.type = UNDERFLOW;
		exc.arg1 = (double)x;
		exc.retval = 0.0;
		exc.name = "expf";
		if (_lib_version == strict_ansi)
			errno = ERANGE;
		else if (!matherr(&exc))
			errno = ERANGE;
		return ((float)exc.retval);
	}
	if (x >= LN_MAXFLOAT) {
		if (x == LN_MAXFLOAT) /* protect against roundoff */
			return (MAXFLOAT); /* causing ldexp to overflow */
		exc.type = OVERFLOW;
		if (_lib_version == c_issue_4)
			exc.retval = HUGE;
		else
			exc.retval = HUGE_VAL;
		exc.arg1 = (double)x;
		exc.name = "expf";
		if (_lib_version == strict_ansi)
			errno = ERANGE;
		else if (!matherr(&exc)) 
			errno = ERANGE;
		return ((float)exc.retval);
	}
	/* argument reduction : x --> x - k*ln2 */
	k = invln2 * x + (x < zero ? -half : half);	/* k=NINT(x/ln2) */
	/* express x-k*ln2 as z+c */
	hi = x - k * ln2hi;
	z = hi - (lo = k * ln2lo);
	c = (hi - z) - lo;

	/* return 2^k*[expm1(x) + 1]  */
/* calculate exp__E(z,c) 
 * where 
 *			 /  exp(x+c) - 1 - x ,  1E-19 < |x| < .3465736
 *       exp__E(x,c) = 	| 		     
 *			 \  0 ,  |x| < 1E-19.
 *
 * Method:
 *	1. Rational approximation. Let r=x+c.
 *	   Based on
 *                                   2 * sinh(r/2)     
 *                exp(r) - 1 =   ----------------------   ,
 *                               cosh(r/2) - sinh(r/2)
 *	   exp__E(r) is computed using
 *                   x*x            (x/2)*W - ( Q - ( 2*P  + x*P ) )
 *                   --- + (c + x*[---------------------------------- + c ])
 *                    2                          1 - W
 * 	   where  P := _POLY2(x^2, p1)
 *	          Q := _POLY2(x^2, q1)
 *	          W := x/2-(Q-x*P),
 *
 *	    and cosh :
 *		sinh(r/2) =  r/2 + r * P  ,  cosh(r/2) =  1 + Q . )
 */
 
	if (_ABS(z) > SMALL) { 
           z1 = z * z;
	   p3 = z1 * (p1 + z1 * p2);
           q = z1 * (q1 + z1 * q2);
           xp = z * p3; 
	   xh = z * half;
           w = xh - (q - xp);
	   p3 += p3;
	   c += z * ((xh * w - (q - (p3 + xp)))/(one - w) + c);
	   z += (z1 * half + c);
	}
#if _IEEE /*in-line expansion of ldexp, but call function if argument
	  * de-normal 
	  */
	w = z + one;
	if (((n = FEXPONENT(w)) == 0) || (n + k <= 0))
		return(float)ldexp((double)w, k);
	FEXPONENT(w) = n + k;
	return(w);
#else
	return(float)ldexp((double)z + 1.0, k);
#endif
}
