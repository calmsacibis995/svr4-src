/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/sqrtf.c	1.7"
/*LINTLIBRARY*/
/*
 *	Single Precision Routine - float args and float return.
 *	sqrt returns the square root of its single-precision argument,
 *	using Newton's method.
 *	Returns EDOM error and value 0 if argument negative.
 *	Calls frexp and ldexp.
 *
 *	If on a 3B machine and MAU is attached, use machine instruction.
 *	If on an IEEE machine, make sure last bit is rounded correctly
 *	according to the currently set rounding mode.
 */

#include "synonyms.h"
#include <errno.h>
#include <math.h>
#include <values.h>

#if _IEEE /* for IEEE machines only */
#define P754_NOFAULT	1	/* avoid generating extra code */
#include <ieeefp.h>
#include "fpparts.h"
#endif

#define ITERATIONS	4

float
sqrtf(float x)
{
	float y, t;
	int iexp;
	register int i = ITERATIONS;
#if _IEEE
/* used to store rounding modes and sticky bits on IEEE machines */
	fp_rnd	r;
	register fp_except j;
	extern fp_rnd _fpsetround();
	extern fp_except _fpsetsticky();
#endif
#ifdef M32
/* detect presence of MAU on 3B machines */
	extern	int _fp_hw;
#endif

	static float zero = 0.0;
	static float half = 0.5;

	if (x <= zero) {
		struct exception exc;
		double q1 = 0.0, q2 = 0.0;

		if (!x)
			return (x); /* sqrt(0) == 0 */
		exc.type = DOMAIN;
		exc.name = "sqrtf";
		exc.arg1 = (double)x;
		q1 /= q2; /* raise invalid op exception */
#if _IEEE
		if (_lib_version == strict_ansi) {
			HIQNAN(exc.retval); /* return NaN */
			LOQNAN(exc.retval);
		}
		else
#endif
		exc.retval = 0.0;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void) write(2, "sqrtf: DOMAIN error\n", 20);
			errno = EDOM;
		}
		return (float)(exc.retval);
	}
#ifdef	M32
	/* if MAU present use machine instruction */
	if (_fp_hw) { 
		asm("	mfsqrs2	0(%ap),0(%ap)");
		return x;
	}
#endif
#if _IEEE
	r = _fpsetround(FP_RN);  /* set round to nearest */
#endif
	y = frexp((double)x, &iexp); /* 0.5 <= y < 1 */
	if (iexp % 2) { /* iexp is odd */
		--iexp;
		y += y; /* 1 <= y < 2 */
	}
	y = ldexp((double)y + 1.0, iexp/2 - 1); /* first guess for sqrt */
	do {
		y = half * (y + x/y);
	} while (--i > 0);
#if _IEEE
	/* twiddle last bit to force y correctly rounded 
	 * on IEEE architectures
	 */
	(void)_fpsetround(FP_RZ); /* round toward zero */
	j = _fpsetsticky(0);
	t = x/y;          /* ...chopped quotient, possibly inexact */
	j = _fpsetsticky(j) & FP_X_IMP; /* was division exact? */
	if (j == 0) { 
		if (t == y)
			goto end;
		else if (r == FP_RM || r == FP_RZ) {
			/* t = nextafter(t, NINF);    ...t=t-ulp */
			if ((FWORD(t) & 0x7fffffff) != 0)
				FWORD(t) -= 0x1;
		}
	}
	else {	/* j == 1 */
		if (r == FP_RN || r == FP_RP) {
			/* t = nextafter(t, PINF);    ...t=t+ulp */
			if ((FWORD(t) & 0x7fffffff) != 0x7f800000)
				FWORD(t) += 0x1;
		}
	}
	if (r == FP_RP) {
		/* y = nextafter(y, PINF);    ...y=y+ulp */
		if ((FWORD(y) & 0x7fffffff) != 0x7f800000)
			FWORD(y) += 0x1;
	}
	y += t;                          /* ...chopped sum */
	FEXPONENT(y) -= 1;	/* correctly rounded sqrt */
end:
	(void)_fpsetround(r); /* restore user rounding mode */
#endif
	return (y);
}
