/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/log1p.c	1.7"
/*LINTLIBRARY*/

/* 
 */

/* _LOG1P(x) 
 * _log1p(x) returns the natural logarithm of 1+x
 *
 * Method :
 *	1. Argument Reduction: find k and f such that 
 *			1+x  = 2^k * (1+f), 
 *	   where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *	2. Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *		 = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *	   log(1+f) is computed by
 *
 *	     		log(1+f) = 2s + s* _POLY7(s*s, p)
 *
 *	3. Finally,  log(1+x) = k*ln2 + log(1+f).  
 *
 *	Remarks:
 *	1. In step 1, f may not be representable. A correction term c
 *	   for f is computed. It follows that the correction term for
 *	   f - t (the leading term of log(1+f) in step 2) is c-c*x. We
 *	   add this correction term to n*ln2lo to attenuate the error.
 */

#include "synonyms.h"
#include <math.h>
#include <values.h>
#include "fpparts.h"

#define SMALL	1.0E-20		/* 1 + SMALL == 1 */

/* ln2 will be stored in two floating point
 * numbers ln2hi and ln2lo, where ln2hi is chosen such that the 
 * last 21 bits  are 0.  This ensures n*ln2hi is exactly representable.
 */
#if _IEEE
static double ln2hi = 6.9314718036912381649E-1;
static double ln2lo = 1.9082149292705877000E-10;
#else 
#if vax /* vax D format */
static double ln2hi  =  6.9314718055829871446E-1;
static double ln2lo  =  1.6465949582897081279E-12;
#endif
#endif


/* coefficients for polynomial expansion */
#if _IEEE
static double p[] = {
	1.4795612545334174692E-1,
	1.5314087275331442206E-1,
	1.8183562745289935658E-1, 
	2.2222198607186277597E-1,
	2.8571428742008753154E-1,   
	3.9999999999416702146E-1,    
	6.6666666666667340202E-1,
	0.0
};
#else
#if vax /* vax D format */
static double p[] = {
	1.2500000000000000000E-1,
	1.3338356561139403517E-1,
	1.5382888777946145467E-1,
	1.8181879517064680057E-1,
	2.2222221233634724402E-1,
	2.8571428579395698188E-1,
	3.9999999999970461961E-1,
	6.6666666666666703212E-1,
	0.0
};
#endif
#endif

double _log1p(x)
double x;
{
	double c;
	register double  s, z, t;
	register int k;

	if (x <= -1.0 ) { /* domain error */
		if (_lib_version == c_issue_4)
			return -HUGE;
		else
			return -HUGE_VAL;
	}

	/* argument reduction */
	if ((x = _ABS(x)) < SMALL)
		return(x);
#if _IEEE
	/* expand logb and ldexp in-line - don't have
	 * to worry about 0 or de-normal because of the
	 * comparison with SMALL above
	 */
	c = 1.0 + x;
	k = (EXPONENT(c)) - 1023;
	c = x;
	EXPONENT(c) -= k;
	z = c;
	c = 1.0;
	EXPONENT(c) -=k;
	t = c;
#else
	k = logb(1.0 + x);
	z = ldexp(x, -k);
	t = ldexp(1.0, -k);
#endif
	if (z + t >= M_SQRT2 ) {
		k += 1;
		z *= 0.5;
		t *= 0.5;
	}
	t += -1.0;
	x = z + t;
	c = (t - x) + z;		/* correction term for x */

	/* compute log(1+x)  */
       	s = x / (2 + x); 
	t = x * x * 0.5;
	c += (k * ln2lo - c * x);
	z = s * s;
#if _IEEE
	z = c + s * (t + _POLY7(z, p));
#else
#if vax /* vax D format */
	z = c + s * (t + _POLY8(z, p));
#endif
#endif
	x += (z - t);
	return(k * ln2hi + x);
}
