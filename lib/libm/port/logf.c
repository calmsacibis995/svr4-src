/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/logf.c	1.8"
/*LINTLIBRARY*/
/*
 *
 *	Single Precision Log
 *	logf returns the natural logarithm of its single-precision argument.
 *	log10f returns the base-10 logarithm of its single-precision argument.
 *	Returns EDOM error and value -HUGE if argument < 0,
 *	ERANGE error if argument == 0
 * Method :
 *	1. Argument Reduction: find k and f such that 
 *			x = 2^k * (1+f), 
 *	   where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *	2. Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *		 = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *	   log(1+f) is computed by
 *
 *	     		log(1+f) = 2s + s*log__L(s*s)
 *	   where
 *		log__L(z) = z*(L1 + z*(L2 + z*(... (L6 + z*L7)...)))
 *
 *
 *	3. Finally,  log(x) = n*ln2 + log(1+f).  (Here n*ln2 will be stored
 *	   in two floating point number: n*ln2hi + n*ln2lo, n*ln2hi is exact
 *	   since the last 8 bits of ln2hi is 0.)
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>
#include <values.h>
#include "fpparts.h"

static float log_error(float, char *, unsigned int);


/* coefficients for polynomial expansion  (log__L())*/
static float p[] = {
	2.2222198607186277597E-1,
	2.8571428742008753154E-1,   
	3.9999999999416702146E-1,    
	6.6666666666667340202E-1
};
static float
ln2hi = 6.9314575195e-1,
ln2lo = 1.4286067653e-6;

static float zero = 0.0,
	     half = 0.5,
	     two = 2.0,
	     negone = -1.0;

float
logf(float x)
{
	register float x1, z, t, s;
	register int n, k;

	if (x <= zero)
		return (log_error(x, "logf", 4));
#if _IEEE
	/* inline expansion of logb()  - get unbiased exponent of x*/
	if ((n = FEXPONENT(x)) == 0) /* de-normal */
		n = -126;
	else n -= 127;
	if (n != -126)  /*  in-line expand ldexp if not de-normal */
		FEXPONENT(x) -= n;	
	else  
		x = (float)ldexp((double)x, -n);
#else
	n = (int)logb((double)x);
	x = (float)ldexp((double)x, -n);
#endif
	if ( n == -126) { /* sub-normal */
#if _IEEE
		/* inline expansion of logb() */
		k = FEXPONENT(x) - 127; /* can't be subnormal because of
				         * prior ldexp
				         */
		FEXPONENT(x) -= k;	
#else
		k = (int)logb((double)x);
		x = (float)ldexp((double)x, -k);
#endif
		n += k;
	}
	x1 = x; /* x can't be in register because of inline expansion
		 * above
		 */
        if (x1 >= (float)M_SQRT2 ) {
		n += 1;
		x1 *= half;
	}
	x1 += negone;

	/* compute log(1+x)  */
	s = x1 / (two + x1);
	t = x1 * x1 * half;
	z = s * s;
	z = n * ln2lo + s * (t + _POLY3(z, p) * z);
	x1 += (z - t) ;
	return(n * ln2hi + x1);
}

float
log10f(register float x)
{
	return (x > zero ? logf(x) * M_LOG10E : log_error(x, "log10f", 6));
}

static float
log_error(float x, char *f_name, unsigned int name_len)
{
	register int zflag = 0;
	struct exception exc;

	exc.name = f_name;
	if (_lib_version == c_issue_4)
		exc.retval = -HUGE;
	else
		exc.retval = -HUGE_VAL;
	if (!x)
		zflag = 1;
	exc.arg1 = (double)x;
	if (_lib_version == strict_ansi) {
		if (!zflag)
			errno = EDOM;
		else 
			errno = ERANGE;
	}
	else {
		if (zflag)
			exc.type = SING;
		else
			exc.type = DOMAIN;
		if (!matherr(&exc)) {
			if (_lib_version == c_issue_4) {
				(void) write(2, f_name, name_len);
				if (zflag)
					(void) write(2,": SING error\n",13);
				else
					(void) write(2,": DOMAIN error\n",15);
			}
			errno = EDOM;
		}
	}
	return ((float)exc.retval);
}
