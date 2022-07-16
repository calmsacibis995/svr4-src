/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/log.c	1.8"
/*LINTLIBRARY*/
/*
 *
 *	log returns the natural logarithm of its double-precision argument.
 *	log10 returns the base-10 logarithm of its double-precision argument.
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
 *	   since the last 20 bits of ln2hi is 0.)
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>
#include <values.h>
#include "fpparts.h"

static double log_error();

#if _IEEE
static double
ln2hi  =  6.9314718036912381649E-1    ,
ln2lo  =  1.9082149292705877000E-10   ;
#else
#if vax /* vax D format */
static double
ln2hi  =  6.9314718055829871446E-1    , 
ln2lo  =  1.6465949582897081279E-12   ;
#endif
#endif

/* coefficients for polynomial expansion  (log__L())*/
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

double
log(x)
double x;
{
	register double x1, z, t;
	double s;
	int n, k;

	if (x <= 0)
		return (log_error(x, "log", 3));
#if _IEEE
	/* inline expansion of logb()  - get unbiased exponent of x*/
	if ((n = EXPONENT(x)) == 0) /* de-normal */
		n = -1022;
	else n -= 1023;
	if (n != -1022)  /*  in-line expand ldexp if not de-normal */
		EXPONENT(x) -= n;	
	else  
		x = ldexp(x, -n);
#else
	n = (int)logb(x);
	x = ldexp(x, -n);
#endif
	if ( n == -1022) { /* sub-normal */
#if _IEEE
		/* inline expansion of logb() */
		k = EXPONENT(x) - 1023; /* can't be subnormal because of
				         * prior ldexp
				         */
		EXPONENT(x) -= k;	
#else
		k = (int)logb(x);
		x = ldexp(x, -k);
#endif
		n += k;
	}
	x1 = x; /* x can't be in register because of inline expansion
		 * above
		 */
        if (x1 >= M_SQRT2 ) {
		n += 1;
		x1 *= 0.5;
	}
	x1 += -1.0;

	/* compute log(1+x)  */
	s = x1 / (2 + x1);
	t = x1 * x1 * 0.5;
	z = s * s;
#if _IEEE
	z = n * ln2lo + s * (t + _POLY7(z, p));
#else
#if vax /* vax D format */
	z = n * ln2lo + s * (t + _POLY8(z, p));
#endif
#endif
	x1 += (z - t) ;
	return(n * ln2hi + x1);
}

double
log10(x)
register double x;
{
	return (x > 0 ? log(x) * M_LOG10E : log_error(x, "log10", 5));
}

static double
log_error(x, f_name, name_len)
double x;
char *f_name;
unsigned int name_len;
{
	register int zflag = 0;
	struct exception exc;

	if (!x) 
		zflag = 1;
	exc.name = f_name;
	if (_lib_version == c_issue_4)
		exc.retval = -HUGE;
	else
		exc.retval = -HUGE_VAL;
	exc.arg1 = x;
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
	return (exc.retval);
}
