/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/fabsf.c	1.1"
/*LINTLIBRARY*/
/*
 *	Single Precision Routine - float args and float return.
 *	fabs returns the absolute value of its single-precision argument.
 */
#include <values.h>
#include "fpparts.h"

float
fabsf(float x)
{
#ifdef _IEEE
	FSIGNBIT(x) = 0;
	return x;
#else
	return (x < 0 ? -x : x);
#endif
}
