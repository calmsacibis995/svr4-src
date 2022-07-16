/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/rem.c	1.10"
/*LINTLIBRARY*/
/* 
 */

/* remainder is an IEEE required function - two implementations 
 * are provided:
 * one which works only on IEEE architectures, and one which works
 * on both IEEE and non-IEEE machines
 */

/* remainder(x,y)
 * Return x remainder y =x-n*y, n=[x/y] rounded (rounded to even 
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

#if _IEEE
#include "fpparts.h"
#define P754_NOFAULT 1
#include <ieeefp.h>


double	remainder(x, y)
double	x, y;
{

	double	hy, y2, t, t1;
	short	k;
	long	n;
	fp_except	mask;
	unsigned short	xexp, yexp, nx, nf, sign;
#ifdef M32
	extern int _fp_hw;  /* detect MAU presence: 32100 processors */
#endif

	xexp = (short)EXPONENT(x);
	yexp = (short)EXPONENT(y);
	sign = (short)SIGNBIT(x);

	if ((y == 0.0) || (xexp == MAXEXP) || 
	   ((yexp == MAXEXP) && (LOWORD(y) || (HIWORD(y) & 0xfffff)))){
		/* IEEE specifies exception for y == 0 or NaN or x ==
		 * inf or Nan
		 */
		struct exception exc;
		double x1 = 0.0, x2 = 0.0;

		exc.arg1 = x;
		exc.arg2 = y;
		exc.type = DOMAIN;
		exc.name = "remainder";
		/* return NaN */
		HIQNAN(exc.retval);
		LOQNAN(exc.retval);
		x1 /= x2; /* raise invalid-op exception */
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void)write(2,"remainder: DOMAIN error\n",24);
			errno = EDOM;
		}
		return (exc.retval);
	}
#ifdef M32 
	if (_fp_hw) { /* MAU present - use machine instruction */
		/* x = rem(x,y) */
		asm("	mfremd2	8(%ap),0(%ap)" );
		return x;
	}
#endif
	/* subnormal number */
	mask = fpsetmask(0);  /* mask all exceptions */
	nx = 0;
	if (yexp == 0) {
		t = 1.0, EXPONENT(t) += 57;
		y *= t; 
		nx = 57;
		yexp = (short)EXPONENT(y);
	}

	/* if y is tiny (biased exponent <= 57), scale up y to y*2**57 */
	else if (yexp <= (unsigned)57) {
		EXPONENT(y) += 57; 
		nx += 57; 
		yexp += 57;
	}

	nf = nx;
	SIGNBIT(x) = 0;
	SIGNBIT(y) = 0;
	/* mask off the least significant 27 bits of y */
	t = y; 
	LOFRACTION(t) &= (unsigned)0xf8000000;
	y2 = t;

	/* LOOP: argument reduction on x whenever x > y */
loop:
	while ( x > y ) {
		t = y;
		t1 = y2;
		xexp = (short)EXPONENT(x);
		k = xexp - yexp - (short)25;
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
#else
/* non-IEEE architectures */

#define MINEXP	DMINEXP

double	remainder(x, y)
register double	x, y;
{
	short	sign, mode = 0;
	double	hy, dy, b;
	extern 	int write();
	int	k, i = 1, yexp, dyexp;
	register double  yfr;

	if (y == 0.0) {
		struct exception exc;
		exc.type = DOMAIN;
		exc.name = "remainder";
		exc.arg1 = x;
		exc.arg2 = y;
		exc.retval = 0.0;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void)write(2,"remainder: DOMAIN error\n",24);
			errno = EDOM;
		}
		return exc.retval;
	}
	y = _ABS(y);	
	yexp = (int)logb(y);
	if (yexp <= MINEXP ) {/* subnormal p, or almost subnormal p */
		b = ldexp(1.0, (int)DSIGNIF+1);
		y *= b; 
		mode = i = 2;
	}
	else if ( y >= MAXDOUBLE / 2) { 
		y /= 2 ; 
		x /= 2; 
		mode = 1;
	}
	dy = y + y; 
	hy = y / 2;
	yfr = frexp(dy, &dyexp);

	while (i--) {  /* do twice for de-normal y, else once */
		if (x < 0.0) {
			sign = 1;
			x = -x;
		}
		else sign = 0;
		while (x >= dy) {
			int xexp;
			double xfr = frexp(x, &xexp);
			x -= ldexp(dy, xexp - dyexp - (xfr < yfr));
		}
		if ( x > hy ) { 
			x -= y ;  
			if ( x >= hy ) 
				x -= y ; 
		}
		if (sign)
			x = -x;
		if (mode == 2 && i)
			x *= b;
	}
	if (mode == 1)
		x *= 2.0;
	else if (mode == 2)
		x /= b;
	return x;
}
#endif /* not IEEE */
