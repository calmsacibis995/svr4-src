/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/cbrt.c	1.3"
/*LINTLIBRARY*/
/* 
 */

/* Two versions of the cbrt function are presented - the first depends on
 * the format of double precision floating point values in IEEE 
 * architectures - the second works on both IEEE and non-IEEE machines
 */

#include <math.h>
#include <values.h>
#include "fpparts.h"

#if _IEEE /* only IEEE machines */

static unsigned B1 = 715094163, /* B1 = (682-0.03306235651)*2**20 */
	        B2 = 696219795; /* B2 = (664-0.03306235651)*2**20 */
static double
	    C= 19./35.,
	    D= -864./1225.,
	    E= 99./70.,
	    F= 45./28.,
	    G= 5./14.;

double cbrt(x) 
double x;
{
	register double r,s,w;
	double t = 0.0;
	unsigned int	sign;


	if (ISMAXEXP(x) || x == 0.0)
		return x;  /* cbrt(+-0.0) == +- 0.0 
			    * cbrt(+-inf) == +- inf 
			    */

	sign = SIGNBIT(x);
	SIGNBIT(x) = 0;

    /* rough cbrt to 5 bits */
	if (EXPONENT(x) == 0) { /* sub-normal */
		EXPONENT(t) = 1077; 	/* set t= 2**54 */
	   	t *= x; 
		HIWORD(t) = HIWORD(t) / 3 + B2;
	}
	else
		HIWORD(t) = HIWORD(x) / 3 + B1;

    /* new cbrt to 23 bits, may be implemented in single precision */
	r = t * t / x;
	s = C + r * t;
	t *= G + F / (s + E + D / s);	

    /* chopped to 20 bits and make it larger than cbrt(x) */ 
	LOFRACTION(t) = 0;
	HIWORD(t) += 0x00000001;

    /* one step newton iteration to 53 bits with error less than 0.667 ulps */
	s = t * t;		/* t*t is exact */
	r = x / s;
	w = t + t;
	r = (r - t)/(w + r);	/* r-s is exact */
	t = t + t * r;

    /* retore the sign bit */
	SIGNBIT(t) |= sign;
	return(t);
}
#else /* non -IEEE machines */

/* This routine is designed to work for IEEE as well as non-IEEE architectures.
* It generates a first guess from the decomposition of $x$ by frexp().
* It then iterates using Newton's method until a solution is found.  
* Convergence is defined as: y(n) = y(n-1) or y(n) = y(n-2).  
*
* Tests on 10000 random points in three different intervals
*  	(total of 30000 points over the entire domain) showed:
* On 3b's:    maximum of 5 iterations.
* On vax 750: maximum of 5 iterations.
* On Amdahl:  maximum of 6 iterations.
*
*/

double
cbrt(x)
double x;
{
	double y_nm2, y_nm1, s;
	register double y_n, r, w;

	int iexp; 
	int sign = 0;

	/* get proper sign */
	if (x < 0.0) {
		x = -x;
		sign = 1;
	}
	y_n = frexp(x, &iexp); /* 0.5 <= y_n < 1 */

	switch(iexp % 3) {
		case 1: /* iexp = (3 * i) + 1 */ 
			--iexp;
			y_n += y_n; 
			break;
		case 2: /* iexp = (3 * i) + 2 */
			++iexp;
			y_n /= 2;
			break;
		default:
			;
	}

	y_n = y_nm1 = y_nm2 = ldexp(y_n + 1.0, iexp/3 - 1);

	/* iterate using Newton's method until we converge */
	do {
		y_nm2 = y_nm1;
		y_nm1 = y_n;

		s = y_n * y_n;
		r = x/s;
		w = y_n + y_n;
		r = (r - y_n) / (w + r);
		y_n = y_n + (y_n * r);

	} while (y_nm1 != y_n && y_nm2 != y_n);

	return(sign ? -y_n : y_n);
}
#endif
