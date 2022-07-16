/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/sinh.c	1.3"
/*LINTLIBRARY*/
/*
 *	sinh returns the hyperbolic sine of its double-precision argument.
 *	A series is used for arguments smaller in magnitude than 1.
 *	The exponential function is used for arguments
 *	greater in magnitude than 1.
 *
 *	cosh returns the hyperbolic cosine of its double-precision argument.
 *	cosh is computed from the exponential function for
 *	all arguments.
 *
 *	Returns ERANGE error and value HUGE (or -HUGE for sinh of
 *	negative argument) if the correct result would overflow.
 *
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <values.h>
#include <errno.h>
#define X_MAX	(LN_MAXDOUBLE + M_LN2)
#define LNV	0.6931610107421875
#define V2M1	0.13830277879601902638e-4

static double sinh_exc();

double
sinh(x)
register double x;
{
	register double y;
	register int sign = 0;

	if (x < 0.0) {
		y = -x;
		sign++;
	}
	else y = x;
	if (y <= 1) {
		static double p[] = {
			-0.78966127417357099479e0,
			-0.16375798202630751372e3,
			-0.11563521196851768270e5,
			-0.35181283430177117881e6,
		}, q[] = {
			 1.0,
			-0.27773523119650701667e3,
			 0.36162723109421836460e5,
			-0.21108770058106271242e7,
		};

		if (y < X_EPS) /* for efficiency and to prevent underflow */
			return (x);
		y = x * x;
		return (x + x * y * _POLY3(y, p)/_POLY3(y, q));
	}
	if (y > LN_MAXDOUBLE) /* exp(x) would overflow */
		return (sinh_exc(x, y, 1));
	x = exp(y);
	y = (0.5 * (x - 1.0/x));
	return(sign ? -y : y);
}

double
cosh(x)
register double x;
{
	register double y = _ABS(x);

	if (y > LN_MAXDOUBLE) /* exp(x) would overflow */
		return (sinh_exc(x, y, 0));
	x = exp(y);
	return (0.5 * (x + 1.0/x));
}

static double
sinh_exc(x, y, sinhflag)
register double x, y;
register int sinhflag;
{
	int neg = (x < 0 && sinhflag); /* sinh of negative argument */
	struct exception exc;

	if (y < X_MAX) { /* result is still representable */
		x = exp(y - LNV);
		x += V2M1 * x;
		return (neg ? -x : x);
	}
	exc.type = OVERFLOW;
	exc.name = sinhflag ? "sinh" : "cosh";
	exc.arg1 = x;
	if (_lib_version == c_issue_4)
		exc.retval = neg ? -HUGE : HUGE;
	else
		exc.retval = neg ? -HUGE_VAL : HUGE_VAL;
	if (_lib_version == strict_ansi)
		errno = ERANGE;
	else if (!matherr(&exc))
		errno = ERANGE;
	return (exc.retval);
}
