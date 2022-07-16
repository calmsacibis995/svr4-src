/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/sinhf.c	1.3"
/*LINTLIBRARY*/
/*
 *	Single Precision Routine - float args and float return.
 *	sinh returns the hyperbolic sine of its single-precision argument.
 *	A series is used for arguments smaller in magnitude than 1.
 *	The exponential function is used for arguments
 *	greater in magnitude than 1.
 *
 *	cosh returns the hyperbolic cosine of its single-precision argument.
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

#define X_MAX	(float)(LN_MAXFLOAT + M_LN2)
#define LNV	(float)0.6931610107421875
#define V2M1	(float)0.13830277879601902638e-4

static float sinh_exc(register float, register float, register int);
static float one = 1.0;
static float half = 0.5;
static float zero = 0.0;

float
sinhf(register float x)
{
	register float y;
	register int sign = 0;

	if (x < zero) {
		y = -x;
		sign++;
	}
	else y = x;

	if (y <= one) {
		static double p[] = {
			-0.190333399e0,
			-0.713793159e1,
		}, q[] = {
			 1.0,
			-0.428277109e2,
		};

		if (y < FX_EPS) /* for efficiency and to prevent underflow */
			return (x);
		y = x * x;
		return (x + x * y * _POLY1(y, p)/_POLY1(y, q));
	}
	if (y > LN_MAXFLOAT) /* exp(x) would overflow */
		return (sinh_exc(x, y, 1));
	x = expf(y);
	y = (half * (x - one/x));
	return(sign ? -y : y);
}

float
coshf(float x)
{
	float y = _ABS(x);

	if (y > LN_MAXFLOAT) /* expf(x) would overflow */
		return (sinh_exc(x, y, 0));
	x = expf(y);
	return (half * (x + one/x));
}

static float
sinh_exc(register float x, register float y, register int sinhflag)
{
	int neg = (x < 0 && sinhflag); /* sinh of negative argument */
	struct exception exc;

	if (y < X_MAX) { /* result is still representable */
		x = expf(y - LNV);
		x += V2M1 * x;
		return (neg ? -x : x);
	}
	exc.type = OVERFLOW;
	exc.name = sinhflag ? "sinhf" : "coshf";
	exc.arg1 = (double)x;
	if (_lib_version == c_issue_4)
		exc.retval = neg ? -HUGE : HUGE;
	else
		exc.retval = neg ? -HUGE_VAL : HUGE_VAL;
	if (_lib_version == strict_ansi)
		errno = ERANGE;
	else if (!matherr(&exc))
		errno = ERANGE;
	return (float)(exc.retval);
}
