/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/sqrt.c	1.9"
/*LINTLIBRARY*/
/* 
 */

/* Two versions of the sqrt function are provided:
 * the first works only on IEEE architectures, the second is
 * more portable.  Both return the same result except that
 * the first rounds the last bit correctly as per the IEEE
 * rounding mode
 */

/* Special cases:
 * sqrt of a negative number results in a domain error
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>
#include <values.h>

#if _IEEE /* for IEEE machines only */

#define P754_NOFAULT	1	/* avoid generating extra code */
#include <ieeefp.h>

#include "fpparts.h"

#define M_2_54	18014398509481984.0  /*  2**54  */

static unsigned long	table[] = {
	0, 1204, 3062, 5746, 9193, 13348, 18162, 23592, 29598, 
	36145, 43202, 50740, 58733, 67158, 75992, 85215, 83599, 
	71378, 60428, 50647, 41945, 34246, 27478, 21581, 16499, 
	12183, 8588, 5674, 3403, 1742, 661, 130 
};

double	sqrt(x)
double	x;
{
	double	y, t;
	register double z;
	unsigned int	mx; 
	fp_rnd	r;
	register fp_except j;
	extern fp_except fpsetsticky();
	extern fp_rnd fpsetround();
	register int scalx = 0;
#ifdef	M32	/*MAU presence on 3B2/3B5,3B15 */
	extern int _fp_hw;
#endif
	if (x <= 0.0) {
		struct exception exc;
		double q1 = 0.0, q2 = 0.0;

		if (!x) /* sqrt(0) == 0 */
			return x;
		exc.type = DOMAIN;
		exc.name = "sqrt";
		exc.arg1 = x;
		q1 /= q2; /* raise invalid op exception */
		if (_lib_version == strict_ansi) {
			HIQNAN(exc.retval); /* return NaN */
			LOQNAN(exc.retval);
			errno = EDOM;
		}
		else {
			exc.retval = 0.0;
			if (!matherr(&exc)) {
				if (_lib_version == c_issue_4)
					(void)write(2,"sqrt: DOMAIN error\n",19);
				errno = EDOM;
			}
		}
		return exc.retval;
	}
#ifdef	M32
	if (_fp_hw) {
		/* on 3b2/3b5/3b15, if MAU, call machine routine */
		asm("	mfsqrd2	0(%ap),0(%ap)"	);
		return x;
	}
#endif
	mx = EXPONENT(x);
	r = fpsetround(FP_RN); /* save rounding mode and set to default */
	/* subnormal number, scale up x to x*2**54 */
	if (mx == 0) {
		x *= M_2_54;
		scalx = -27;
	}
	/* scale x to avoid intermediate over/underflow:
	 * if (x > 2**512) x=x/2**512; if (x < 2**-512) x=x*2**512 
	 */
	if (mx > 1535) {  /* 512 + 1023 */
		EXPONENT(x) -= 512;
		scalx += 256;
	}
	if (mx < 511) {  /* 1023 - 512 */
		EXPONENT(x) += 512;
		scalx -= 256;
	}
	/* magic initial approximation to almost 8 sig. bits */
	HIWORD(y) = ((int)HIWORD(x) >> 1) + 0x1ff80000;
	HIWORD(y) -= table[((int)(HIWORD(y)) >> 15) & 31];

	/* Heron's rule once with correction to improve y to 
	 * almost 18 sig. bits 
	 */
	t = x / y; 
	y +=  t; 
	HIWORD(y) -= 0x00100006;
	LOFRACTION(y) = 0;
	/* triple to almost 56 sig. bits; now y approx. sqrt(x) to within 1 ulp */
	t = y * y; 
	z = t;  
	EXPONENT(t) += 0x1;
	t += z; 
	z = (x - z) * y;
	t = z / (t + x);  
	EXPONENT(t) += 0x1;
	y += t;

	/* twiddle last bit to force y correctly rounded */
	(void)fpsetround(FP_RZ);
	j = fpsetsticky(0);
	t = x/y;          /* ...chopped quotient, possibly inexact */
	j = fpsetsticky(j) & FP_X_IMP; /* was division exact? */
	if (j == 0) { 
		if (t == y)
			goto end;
		else if (r == FP_RM || r == FP_RZ) {
			/* t = nextafter(t, NINF);    ...t=t-ulp */
			if (HIWORD(t) != 0)
				LOFRACTION(t) -= 0x1;
			else {
				LOWORD(t) = (unsigned)0xffffffff;
				if ((HIWORD(t) & 0x7fffffff) != 0)
					HIWORD(t) -= 0x1;
			}
		}
	}
	else {	/* j == 1 */
		if (r == FP_RN || r == FP_RP) {
			/* t = nextafter(t, PINF);    ...t=t+ulp */
			if (LOFRACTION(t) != (unsigned)0xffffffff)
				LOFRACTION(t) += 0x1;
			else {
				LOFRACTION(t) = 0;
				if ((HIWORD(t) & 0x7fffffff) != 0x7ff00000)
					HIWORD(t) += 0x1;
			}
		}
	}
	if (r == FP_RP) {
		/* y = nextafter(y, PINF);    ...y=y+ulp */
		if (LOFRACTION(y) != (unsigned)0xffffffff)
			LOFRACTION(y) += 0x1;
		else {
			LOFRACTION(y) = 0;
			if ((HIWORD(y) & 0x7fffffff) != 0x7ff00000)
				HIWORD(y) += 0x1;
		}
	}
	y += t;                          /* ...chopped sum */
	EXPONENT(y) -= 1;	/* correctly rounded sqrt */
end:
	EXPONENT(y) += scalx;	/* ...scale back y */
	(void)fpsetround(r);
	return(y);
}
#else	/* non-IEEE machines */

#define ITERATIONS	4

double
sqrt(x)
register double x;
{
	register double y;
	int iexp; /* can't be in register because of frexp() below */
	register int i = ITERATIONS;

	if (x <= 0) {
		struct exception exc;

		if (!x)
			return (x); /* sqrt(0) == 0 */
		exc.type = DOMAIN;
		exc.name = "sqrt";
		exc.arg1 = x;
		exc.retval = 0.0;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void) write(2, "sqrt: DOMAIN error\n", 19);
			errno = EDOM;
		}
		return (exc.retval);
	}
	y = frexp(x, &iexp); /* 0.5 <= y < 1 */
	if (iexp % 2) { /* iexp is odd */
		--iexp;
		y += y; /* 1 <= y < 2 */
	}
	y = ldexp(y + 1.0, iexp/2 - 1); /* first guess for sqrt */
	do {
		y = 0.5 * (y + x/y);
	} while (--i > 0);
	return (y);
}
#endif
