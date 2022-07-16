/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/hypot.c	1.8"
/*LINTLIBRARY*/
/* 
 */

/* hypot(x,y)
 * return the square root of x^2 + y^2 
 *
 * Method :
 *	1. replace x by |x| and y by |y|, and swap x and
 *	   y if y > x (hence x is never smaller than y).
 *	2. Hypot(x,y) is computed by:
 *	   Case I, x/y > 2
 *		
 *				       y
 *		hypot = x + -----------------------------
 *			 		    2
 *			    sqrt ( 1 + [x/y]  )  +  x/y
 *
 *	   Case II, x/y <= 2 
 *				                   y
 *		hypot = x + --------------------------------------------------
 *				          		     2 
 *				     			[x/y]   -  2
 *			   (sqrt(2)+1) + (x-y)/y + -----------------------------
 *			 		    			  2
 *			    			  sqrt ( 1 + [x/y]  )  + sqrt(2)
 *
 *
 *
 * Special cases:
 *	hypot(x,y) generates a range error if result would be too big to
 *	represent as a double precision value
 */
#include "synonyms.h"
#include <values.h>
#include <errno.h>
#include <math.h>
#include "fpparts.h"

#if _IEEE
static double
r2p1hi =  2.4142135623730949234E0, 
r2p1lo =  1.2537167179050217666E-16,
sqrt2  =  1.4142135623730951455E0; 
#else  /* vax D format */
static double
r2p1hi =  2.4142135623730950345E0, 
r2p1lo =  1.4349369327986523769E-17,
sqrt2  =  1.4142135623730950622E0; 
#endif

double hypot(x, y)
double x, y;
{
	static int ibig = 30;	/* 1+2**(2*ibig)==1 */
	register double t, r;
	register int  xexp, yexp;
	struct exception exc;

	exc.arg1 = x;
	exc.arg2 = y;

	x = _ABS(x);
	y = _ABS(y);

	if (y > x) {
		t = x;
		x = y;
		y = t;
	}
	if (x == 0.0)  /* x and y must both be 0 */
		return 0.0;
	if (y == 0.0)
		return(x);
#if _IEEE /* find unbiased exponent of x and y
	      *	expand logb() inline on IEEE machines
              */
	if ((xexp = EXPONENT(x)) == 0) /* de-normal */
		xexp = -1022;
	else xexp -= 1023;
	if ((yexp = EXPONENT(y)) == 0) /* de-normal */
		yexp = -1022;
	else yexp -= 1023;
#else
	xexp = (int)logb(x);
	yexp = (int)logb(y);
#endif
	if (xexp - yexp > ibig ) 	
		return x;
	/* start computing sqrt(x^2 + y^2) */
	r = x - y;
	if (r > y) { 	/* x/y > 2 */
		r = x/y;
		r = r + sqrt(1.0 + r * r);
	}
	else {		/* 1 <= x/y <= 2 */
		r /= y;
		t = r * (r + 2.0);
		r += t / (sqrt2 + sqrt(2.0 + t));
		r += r2p1lo; 
		r += r2p1hi;
	}
	r = y / r;
	if (x <= MAXDOUBLE - r)  /* check for overflow */
		return(x + r);
	else { /* error return for overflow */
		exc.name = "hypot";
		if (_lib_version == c_issue_4)
			exc.retval = HUGE;
		else
			exc.retval = HUGE_VAL;
		exc.type = OVERFLOW;
		if (_lib_version == strict_ansi)
			errno = ERANGE;
		else if (!matherr(&exc))
			errno = ERANGE;
		return exc.retval;
	}
}
