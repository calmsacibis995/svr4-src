/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:fp/logb.c	1.2"
/*LINTLIBRARY*/

/* IEEE recommended functions */

#ifdef __STDC__
	#pragma weak logb = _logb
	#pragma weak nextafter = _nextafter
#endif

#include <values.h>
#include <errno.h>
#include "fpparts.h"
#include "synonyms.h"

/* LOGB(X)
 * logb(x) returns the unbiased exponent of x as a double precision
 * number, except that logb(NaN) is NaN, logb(infinity) is +infinity,
 * logb(0) is -infinity and raises the divide by zero exception,
 * errno = EDOM.
 */
double logb(x)
double x;
{
	register int iexp = EXPONENT(x);

	if (iexp == MAXEXP) { /* infinity  or NaN */
		SIGNBIT(x) = 0;
		errno = EDOM;
		return x;
	}
	if (iexp == 0)  {  /* de-normal  or 0*/
		if ((HIFRACTION(x) == 0) && (LOFRACTION(x) == 0)) { /*zero*/
			double zero = 0.0;
			errno = EDOM;
			return(-1.0/zero); /* return -inf - raise div by 
					    * zero exception
					    */
		}
		else  /* de-normal */
			return(-1022.0);
	}
	return((double)(iexp - 1023)); /* subtract bias */
}

/* NEXTAFTER(X,Y)
 * nextafter(x,y) returns the next representable neighbor of x
 * in the direction of y
 * Special cases:
 * 1) if either x or y is a NaN then the result is one of the NaNs, errno
 * 	=EDOM
 * 2) if x is +-inf, x is returned and errno = EDOM
 * 3) if x == y the results is x without any exceptions being signalled
 * 4) overflow  and inexact are signalled when x is finite,
 *	but nextafter(x,y) is not, errno = ERANGE
 * 5) underflow and inexact are signalled when nextafter(x,y)
 * 	lies between +-(2**-1022), errno = ERANGE
 */
 double nextafter(x,y)
 double x,y;
 {
	if (EXPONENT(x) == MAXEXP) { /* Nan or inf */
		errno = EDOM;
		return x; 
	}
	if ((EXPONENT(y) == MAXEXP) && (HIFRACTION(y) || 
		LOFRACTION(y))) {
		errno = EDOM;
		return y;  /* y a NaN */
	}
	if (x == y)
		return x;
	if (((y > x) && !SIGNBIT(x)) || ((y < x) && SIGNBIT(x))) {
		/* y>x, x negative or y<x, x positive */

		if (LOFRACTION(x) != (unsigned)0xffffffff)
			LOFRACTION(x) += 0x1;
		else {
			LOFRACTION(x) = 0x0;
			if (((unsigned)(HIWORD(x) & 0x7fffffff)) < (unsigned)0x7ff00000)
				HIWORD(x) += 0x1;
		}
	}
	else { /* y<x, x pos or y>x, x neg */

		if (LOFRACTION(x) != 0x0)
			LOFRACTION(x) -= 0x1;
		else {
			LOWORD(x) = (unsigned)0xffffffff;
			if ((HIWORD(x) & 0x7fffffff) != 0x0)
				HIWORD(x) -=0x1;
		}
	}
	if (EXPONENT(x) == MAXEXP) { /* signal overflow and inexact */
		x = (SIGNBIT(x) ? -MAXDOUBLE : MAXDOUBLE);
		errno = ERANGE;
		return( x * 2.0);
	}
	if (EXPONENT(x) == 0) {
	/* de-normal - signal underflow and inexact */
		x = (SIGNBIT(x) ? -MINDOUBLE : MINDOUBLE);
		errno = ERANGE;
		return(x / 2.0);
	}
	return x;
}
