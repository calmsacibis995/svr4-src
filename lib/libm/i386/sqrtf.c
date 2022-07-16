/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:i386/sqrtf.c	1.2"

/*LINTLIBRARY*/

/*
 *	sqrtf returns the square root of its single-precision argument
 *	using the 80387 processor or the 80387 emulator.
 *	Returns EDOM error and value 0 if argument negative.
 */

#include "synonyms.h"
#include <errno.h>
#include <math.h>			/* temporary location	*/
#include <values.h>
#include "fpparts.h"

asm	float xsqrtf(float x)
{
%mem	x;
	flds	x
	fsqrt
}

float
sqrtf(float x)
{
	float xsqrtf(float);

	if (x <= (float)0.0) {
		struct exception exc;
		float x1 = 0.0, x2 = 0.0;

		if (!x)
			return (x); /* sqrt(0) == 0 */
		exc.type = DOMAIN;
		exc.name = "sqrtf";
		exc.arg1 = (double)x;
		x1 /= x2; /* raise invalid op exception */
		if (_lib_version == strict_ansi) {
			HIQNAN(exc.retval);
			LOQNAN(exc.retval);
			errno = EDOM;
		}
		else {
			exc.retval = 0.0;
			if (!matherr(&exc)) {
				if (_lib_version == c_issue_4)
					(void)write(2, "sqrt: DOMAIN error\n", 19);
				errno = EDOM;
			}
		
		}
		return(exc.retval);
	}

	return(xsqrtf(x));
}
