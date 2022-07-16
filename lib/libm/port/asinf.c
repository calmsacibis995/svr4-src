/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/asinf.c	1.4"
/*LINTLIBRARY*/
/*
 *	Single Precision Routine - float args and float return.
 *	C program for single-precision asin/acos.
 *	Returns EDOM error and value 0 if |argument| > 1.
 *	Algorithm and coefficients from Cody and Waite (1980).
 *	Calls sqrt if |argument| > 0.5.
 */

#include "synonyms.h"
#include <math.h>
#include <values.h>
#include <errno.h>

#define FM_PI	(float)M_PI
#define FM_PI_2	(float)M_PI_2

float
asinf(float x)
{
	static float asin_acos(float x, int acosflag);

	return (asin_acos(x, 0));
}

static float
asin_acos(register float x, int acosflag)
{
	register float y;
	register int neg = 0, large = 0;
	struct exception exc;
	
	static float zero = 0.0;
	static float one = 1.0;
	static float half = 0.5;
	
	exc.arg1 = (double)x;
	if (x < zero) {
		x = -x;
		neg++;
	}
	if (x > one) {
		exc.type = DOMAIN;
		exc.name = acosflag ? "acosf" : "asinf";
		exc.retval = 0.0;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4) {
				(void) write(2, exc.name, 5);
				(void) write(2, ": DOMAIN error\n", 15);
			}
			errno = EDOM;
		}
		return (float)(exc.retval);
	}
	if (x > FX_EPS) { /* skip for efficiency and to prevent underflow */
		static float p[] = {
			-0.504400557e0,
			 0.933935835e0,
		}, q[] = {
			 1.0,
			-0.554846723e1,
			 0.560363004e1,
		};

		if (x <= half)
			y = x * x;
		else {
			large++;
			y = half - half * x;
			x = -sqrtf(y);
			x += x;
		}
		x += x * y * _POLY1(y, p)/_POLY2(y, q);
	}
	if (acosflag) {
		if (!neg)
			x = -x;
		return (!large ? FM_PI_2 + x : neg ? FM_PI + x : x);
	}
	if (large)
		x += FM_PI_2;
	return (neg ? -x : x);
}

float
acosf(float x)
{
	static float asin_acos(float x, int acosflag);

	return (asin_acos(x, 1));
}
