/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/asinh.c	1.6"
/*LINTLIBRARY*/
/* 
 */

/* ASINH(X)
 * asinh(x) returns the inverse hyperbolic sine of x
 * Method :
 *	Based on 
 *		asinh(x) = sign(x) * log [ |x| + sqrt(x*x+1) ]
 *	we have
 *	asinh(x) := x  if  1+x*x=1,
 *		 := sign(x)*(log1p(x/2)+log4) if sqrt(1+x*x)=x && x>=MAXDOUBLE/2
 *		 := sign(x)*(log1p(2*x))    if sqrt(1+x*x)=x && x<MAXDOUBLE/2
 *		 := sign(x)*log1p(|x| + |x|/(1/|x| + sqrt(1+(1/|x|)^2)) )  
 *
 */

#include <math.h>
#include <values.h>

#define SQSMALL	1.0E-10		/* 1 + SQSMALL * SQSMALL == 1 */
#define	BIG	1.0E20		/* 1 + BIG == BIG */

static double ln4=1.386294361119890618834464242916;
double asinh(x)
double x;
{	
	extern double _log1p();
	register int signx = 0;
	register double s, t;

	if (x < 0.0) {
		t = -x;
		signx = 1;
	}
	else t = x;
	if (t <= SQSMALL)
		return x;
	if (t < BIG) {
	     	s = 1.0 / t;
		t +=  (t / (s + sqrt(1.0 + s*s)));
		s = _log1p(t);
	}else if (t >= MAXDOUBLE/2) {	
		s = _log1p(t/2.0) + ln4;
	}else { /* if |x| > BIG && |x| <MAXDOUBLE/2 */
		s=_log1p(2.*t);
	}
	/* return copysign(s, x) */
	if (!signx) {
		if (s < 0.0)
			s = -s;
	}
	else {
		if (s > 0.0)
			s = -s;
	}
	return s;
}
