/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/fabs.c	1.4"
/*LINTLIBRARY*/
/*
 *	fabs returns the absolute value of its double-precision argument.
 */

#ifdef __STDC__
	#pragma weak copysign = _copysign
#endif

#include "synonyms.h"
#include <values.h>
#include "fpparts.h"

double
fabs(x)
double x;
{

#if _IEEE
	SIGNBIT(x) = 0;
	return x;
#else
	return (x < 0 ? -x : x);
#endif
}

/* COPYSIGN(X,Y)
 * Return x with the sign of y  - no exceptions are raised
 */


double copysign(x,y)
double	x, y;
{
#if _IEEE
	SIGNBIT(x) = SIGNBIT(y);
	return x ;
#else
	if (y >= 0.0)
		return(x >= 0.0 ? x : -x);
	else 
		return(x >= 0.0 ? -x : x);
#endif
}
