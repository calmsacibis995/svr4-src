/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/exp.c	1.3"
/*LINTLIBRARY*/
/* 
 */

/* exp(x)
 * return the exponential of x
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

#if _IEEE
static double
ln2hi  =  6.9314718036912381649E-1    , 
ln2lo  =  1.9082149292705877000E-10   ,
invln2 =  1.4426950408889633870E0     ;
#else 
#if vax /* vax D format */
static double
ln2hi  =  6.9314718055829871446E-1    , 
ln2lo  =  1.6465949582897081279E-12   , 
invln2 =  1.4426950408889634148E0     ;
#endif
#endif

#define SMALL	1.0E-19

/* coefficients for exp__E() */
#if _IEEE
static double 
p1 = 1.3887401997267371720E-2,
p2 = 3.3044019718331897649E-5, 
q1 = 1.1110813732786649355E-1,
q2 = 9.9176615021572857300E-4;
#else 
#if vax /* vax D format */
static double 
p1 =  1.5150724356786683059E-2, 
p2 =  6.3112487873718332688E-5,
q1 =  1.1363478204690669916E-1, 
q2 =  1.2624568129896839182E-3,
q3 =  1.5021856115869022674E-6;
#endif
#endif

double exp(x)
double x;
{
	register double z, z1, c ;
	double hi,lo, p3, q;
	double xp, xh, w;
	int n, k;
	struct exception exc;

	if (x <= LN_MINDOUBLE) {
		if (x == LN_MINDOUBLE) /* protect against roundoff */
			return (MINDOUBLE); /* causing ldexp to underflow */
		exc.type = UNDERFLOW;
		exc.arg1 = x;
		exc.retval = 0.0;
		exc.name = "exp";
		if (_lib_version == strict_ansi)
			errno = ERANGE;
		else if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
	}
	if (x >= LN_MAXDOUBLE) {
		if (x == LN_MAXDOUBLE) /* protect against roundoff */
			return (MAXDOUBLE); /* causing ldexp to overflow */
		exc.type = OVERFLOW;
		if (_lib_version == c_issue_4)
			exc.retval = HUGE;
		else
			exc.retval = HUGE_VAL;
		exc.arg1 = x;
		exc.name = "exp";
		if (_lib_version == strict_ansi)
			errno = ERANGE;
		else if (!matherr(&exc)) 
			errno = ERANGE;
		return (exc.retval);
	}
	/* argument reduction : x --> x - k*ln2 */
	k = invln2 * x + (x < 0.0 ? -0.5 : 0.5);	/* k=NINT(x/ln2) */
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
#if _IEEE
           q = z1 * (q1 + z1 * q2);
#else
#if vax /* vax D format */
           q = z1 * ( q1 + z1 * ( q2 + z1 * q3 ));
#endif
#endif
           xp = z * p3; 
	   xh = z * 0.5;
           w = xh - (q - xp);
	   p3 += p3;
	   c += z * ((xh * w - (q - (p3 + xp)))/(1.0 - w) + c);
	   z += (z1 * 0.5 + c);
	}
#if _IEEE /*in-line expansion of ldexp, but call function if argument
	  * de-normal 
	  */
	w = z + 1.0;
	if (((n = EXPONENT(w)) == 0) || (n + k <= 0))
		return(ldexp(w, k));
	EXPONENT(w) = n + k;
	return(w);
#else
	return(ldexp(z + 1.0, k));
#endif
}
