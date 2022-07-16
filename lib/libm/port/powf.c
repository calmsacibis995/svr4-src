/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/powf.c	1.8"
/*LINTLIBRARY*/
/*
 *	Single precision power function.
 *	powf(x, y) returns x ** y.
 *	Returns EDOM error and value 0 for 0 to a non-positive power
 *	or negative to a non-integral power;
 *	ERANGE error and value HUGE or -HUGE when the correct value
 *	would overflow, or 0 when the correct value would underflow.
 *	uses double precision log and exp to preserve accuracy
 */

#include "synonyms.h"
#include <math.h>
#include <values.h>
#include <errno.h>

float powf(register float x, register float y)
{
	register double a, b;
	int neg;
	struct exception exc;

	static float one = 1.0,
		     half = 0.5,
		     zero = 0.0;

	if (y == one) /* easy special case */
		return (x);
	a = (double)x;
	b = (double)y;
	exc.name = "powf";
	exc.arg1 = a;
	exc.arg2 = b;
	if (!x) {
		if (y > zero)
			return (x); /* (0 ** pos) == 0 */
		if ((y == (float)0.0) && (_lib_version != c_issue_4))
			return((float)1.0);
		goto domain;
	}
	neg = 0;
	if (x < zero) { /* test using integer arithmetic if possible */
		if (y >= -FMAXPOWTWO && y <= FMAXPOWTWO) {
			register long ly = (long)y;

			if ((float)ly != y)
				goto domain; /* y not integral */
			neg = ly % 2;
		} else { /* y must be an integer */
			float  dum;

			if (modff(y * half, &dum))
				neg++; /* y is odd */
		}
		a = -a;
	}
	if (a != 1.0) { /* x isn't the final result */
		/* the following code protects against multiplying x and y
		 * until there is no chance of multiplicative overflow */
		if ((a = log(a)) < 0) { /* preserve sign of product */
			a = -a;
			b = -b;
		}
		if (b > (double)LN_MAXFLOAT/a) {
			exc.type = OVERFLOW;
			if (_lib_version == c_issue_4)
				exc.retval = neg ? -HUGE : HUGE;
			else
				exc.retval = neg ? -HUGE_VAL : HUGE_VAL;
			if (_lib_version == strict_ansi)
				errno = ERANGE;
			else if (!matherr(&exc))
				errno = ERANGE;
			return (float)exc.retval;
		}
		if (b < (double)LN_MINFLOAT/a) {
			exc.retval = 0.0;
			exc.type = UNDERFLOW;
			if (_lib_version == strict_ansi)
				errno = ERANGE;
			else if (!matherr(&exc))
				errno = ERANGE;
			return (float)exc.retval;
		}
		a = exp(a * b); /* finally; no mishap can occur */
	}
	return (float)(neg ? -a : a);

domain:
	exc.type = DOMAIN;
	if ((x == (float)0.0) && (_lib_version != c_issue_4))
		exc.retval = -HUGE_VAL;
	else
		exc.retval = 0.0;
	if (_lib_version == strict_ansi)
		errno = EDOM;
	else if (!matherr(&exc)) {
		if (_lib_version == c_issue_4)
			(void) write(2, "powf: DOMAIN error\n", 19);
		errno = EDOM;
	}
	return (float)exc.retval;
}
