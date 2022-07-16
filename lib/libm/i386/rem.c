/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:i386/rem.c	1.3"
/*LINTLIBRARY*/


/* remainder(x,y)
 * Return x rem y =x-n*y, n=[x/y] rounded (rounded to even 
 * in the half way case)
 *
 *  Domain error occurs for y == 0 or NaN, x == inf or NaN
 */

#ifdef __STDC__
	#pragma weak remainder = _remainder
#endif

#include "synonyms.h"
#include <math.h>
#include <errno.h>
#include <values.h>
#include <sys/fp.h>

#include "fpparts.h"
#define P754_NOFAULT 1
#include <ieeefp.h>

asm double xrem(x, y)
{
%mem	x,y;
	fldl	y
	fldl	x
.dorem1:
	fprem1
	fstsw	%ax
	testl	$0x400,%eax
	jne	.dorem1
	ffree	%st(1)
}


double	
remainder(x, y)
double	x, y;
{

	double	hy, y2, t, t1;
	double	xrem();
	short	k;
	long	n;
	fp_except	mask;
	unsigned short	xexp, yexp, nx, nf, sign;

	extern int _fp_hw;  /* detect hardware presence */


	xexp = EXPONENT(x);
	yexp = EXPONENT(y);
	sign = SIGNBIT(x);

	if ((y == 0.0) || (xexp == MAXEXP) || 
	   ((yexp == MAXEXP) && (LOWORD(y) || (HIWORD(y) & 0xfffff)))){
		/* IEEE specifies exception for y == 0 or NaN or x ==
		 * inf or Nan
		 */
		struct exception exc;
		double x1= 0.0, x2 = 0.0;

		exc.arg1 = x;
		exc.arg2 = y;
		exc.type = DOMAIN;
		exc.name = "remainder";
		HIQNAN(exc.retval);
		LOQNAN(exc.retval);
		x1 /= x2;  /* raise invalid op exception */
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void)write(2,"remainder: DOMAIN error\n",24);
			errno = EDOM;
		}
		return (exc.retval);
	}

	if (_fp_hw == FP_387) { /* 80387 present - use machine instruction */
		return xrem(x,y);
	}

	/* subnormal number */
	mask = fpsetmask(0);  /* mask all exceptions */
	nx = 0;
	if (yexp == 0) {
		t = 1.0, EXPONENT(t) += 57;
		y *= t; 
		nx = 57;
		yexp = EXPONENT(y);
	}

	/* if y is tiny (biased exponent <= 57), scale up y to y*2**57 */
	else if (yexp <= 57) {
		EXPONENT(y) += 57; 
		nx += 57; 
		yexp += 57;
	}

	nf = nx;
	SIGNBIT(x) = 0;
	SIGNBIT(y) = 0;
	/* mask off the least significant 27 bits of y */
	t = y; 
	LOFRACTION(t) &= 0xf8000000;
	y2 = t;

	/* LOOP: argument reduction on x whenever x > y */
loop:
	while ( x > y ) {
		t = y;
		t1 = y2;
		xexp = EXPONENT(x);
		k = xexp - yexp - 25;
		if (k > 0) 	/* if x/y >= 2**26, scale up y so that x/y < 2**26 */ {
			EXPONENT(t) += k;
			EXPONENT(t1) += k;
		}
		n = x / t; 
		x = (x - n * t1) - n * (t - t1);
	}
	/* end while (x > y) */

	if (nx != 0) {
		t = 1.0; 
		EXPONENT(t) += nx; 
		x *= t; 
		nx = 0; 
		goto loop;
	}

	/* final adjustment */

	hy = y / 2.0;
	if (x > hy || ((x == hy) && n % 2 == 1)) 
		x -= y;
	SIGNBIT(x) ^= sign;
	if (nf != 0) { 
		t = 1.0; 
		EXPONENT(t) -= nf; 
		x *= t;
	}
	(void)fpsetmask(mask);  /* reset exception masks */
	return(x);
}
