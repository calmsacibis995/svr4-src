/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/acosh.c	1.8"
/*LINTLIBRARY*/
/* 
 */

/* ACOSH(X)
 * acosh(x) returns the inverse hyperbolic cosine of x
 *
 * Method :
 *	Based on 
 *		acosh(x) = log [ x + sqrt(x*x-1) ]
 *	we have
 *		acosh(x) := log1p(x/2)+log4,if (x > MAXDOUBLE/2); 
 *		acosh(x) := log1p(2*x)		if (x > 1.0E20)
 *		acosh(x) := log1p( sqrt(x-1) * (sqrt(x-1) + sqrt(x+1)) ) .
 *	These formulae avoid the over/underflow complication.
 *
 *	acosh(x) is a domain error if x < 1
 *
 * Note log(2*x)=log(x/n)+log(2*n) for large x.
 *  	  n=2 for IEEE double precision gives small addition roundoff
 *	  (n=1048576 for VAX double precision)
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>
#include <values.h>

#define	BIG	1.0E20		/* 1 + BIG == BIG */

#if _IEEE
#include "fpparts.h"
#endif

static double ln4=1.386294361119890618834464242916;
double acosh(x)
register double x;
{	
	double _log1p();
	register double t;

	if (x < 1.0) {
		struct exception exc;
#if _IEEE	/* if IEEE return NaN */
		HIQNAN(exc.retval);
		LOQNAN(exc.retval);
#else
		exc.retval = 0.0;
#endif
		exc.type = DOMAIN;
		exc.name = "acosh";
		exc.arg1 = x;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void) write(2, "acosh: DOMAIN error\n", 20);
			errno = EDOM;
		}
		return (exc.retval);
	}

	if (x >= MAXDOUBLE/2) {
		return(_log1p(x/2.0)+ln4);
	} else if(x > BIG ){
		/* x*2 will not overflow */
		return _log1p(x*2);
	}
	t = sqrt(x - 1.0);
	return(_log1p(t * (t + sqrt(x + 1.0))));
}
