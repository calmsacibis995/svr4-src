/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/atanh.c	1.6"
/*LINTLIBRARY*/
/* 
 */

/* ATANH(X)
 *	log1p(x) 	...return log(1+x)
 * Method :
 *	Return 
 *                          1              2x                          x
 *		atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *                          2             1 - x                      1 - x
 *
 *	atanh(x) is domain error |x| > 1.0
 *	singularity error if |x| == 1.0;
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>
#include <values.h>

#if _IEEE
#include "fpparts.h"
#endif

double atanh(x)
double x;
{
	double _log1p();
	register double y, z;

	if ((y = _ABS(x)) >= 1.0) {
		struct exception exc;

#if _IEEE	/* if IEEE return NaN */
		HIQNAN(exc.retval);
		LOQNAN(exc.retval);
#else
		exc.retval = 0.0;
#endif
		if (y == 1.0)
			exc.type = SING;
		else exc.type = DOMAIN;
		exc.name = "atanh";
		exc.arg1 = x;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4) {
				if (exc.type == DOMAIN)
					(void) write(2, "atanh: DOMAIN error\n", 20);
				else
					(void) write(2, "atanh: SING error\n", 19);
			}
			errno = EDOM;
		}
		return (exc.retval);
	}
	z = ((x < 0.0) ? -0.5 : 0.5);
	y /=  (1.0 - y);
	return(z * _log1p(y + y));
}
