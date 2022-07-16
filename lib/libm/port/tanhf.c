/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/tanhf.c	1.1"
/*LINTLIBRARY*/
/*
 *	Single Precision Routine - float args and float return.
 *	tanh returns the hyperbolic tangent of its single-precision argument.
 *	It calls exp for absolute values of the argument > ~0.55.
 *	There are no error returns.
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <values.h>

#define X_BIG	(float)(0.5 * (LN_MAXFLOAT + M_LN2))
#define LN_3_2	(float) 0.54930614433405484570

float
tanhf(register float x)
{
	register int neg = 0;
	static float zero = 0.0;
	static float one = 1.0;
	static float half = 0.5;

	if (x < zero) {
		x = -x;
		neg++;
	}
	if (x > X_BIG)
		x = one;
	else if (x > LN_3_2) {
		x = half - one/(expf(x + x) + one); /* two steps recommended */
		x += x;
	} else if (x > FX_EPS) { /* skip for efficiency and to prevent underflow */
		static double p[] = {
			-0.3831010665e-2,
			-0.8237728127e0,
		}, q[] = {
			 1.0,
			 0.2471319654e1,
		};
		float y = x * x;

		x += x * y * _POLY1(y, p)/_POLY1(y, q);
	}
	return (neg ? -x : x);
}
