/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/floorf.c	1.2"
/*LINTLIBRARY*/
/*
 *	Single Precision Routine - float args and float return.
 *	floor(x) returns the largest integer (as a single-precision number)
 *	not greater than x.
 *	ceil(x) returns the smallest integer not less than x.
 */

#include <math.h>

float
floorf(register float x) 
{
	float y; /* can't be in register because of modf() below */

	return (modff(x, &y) < 0 ? y - 1 : y);
}

float
ceilf(register float x)
{
	float y; /* can't be in register because of modf() below */

	return (modff(x, &y) > 0 ? y + 1 : y);
}
