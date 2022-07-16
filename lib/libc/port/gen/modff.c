/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/modff.c	1.7"
/*LINTLIBRARY*/
/*
 * modff(value, iptr) returns the signed fractional part of value
 * and stores the integer part indirectly through iptr.
 *
 */

#ifdef __STDC__
	#pragma weak modff = _modff
#endif
#include <sys/types.h>
#include "synonyms.h"
#include <values.h>
#if _IEEE /* machines with IEEE floating point only */
#include <signal.h>
#include <unistd.h>


typedef union {
	float f;
	unsigned long word;
} _fval;
#define EXPMASK	0x7f800000
#define FRACTMASK 0x7fffff
#define FWORD(X)	(((_fval *)&(X))->word)

#endif

float
#ifdef __STDC__
modff(float value, register float *iptr)
#else
modff(value, iptr)
float value;
register float *iptr;
#endif
{
	register float absvalue;

#if _IEEE
	/* raise exception on NaN - 3B only */
	if (((FWORD(value) & EXPMASK) == EXPMASK) &&
		(FWORD(value) & FRACTMASK))
		(void)kill(getpid(), SIGFPE);
#endif
	if ((absvalue = (value >= (float)0.0) ? value : -value) >= FMAXPOWTWO)
		*iptr = value; /* it must be an integer */
	else {
		*iptr = absvalue + FMAXPOWTWO; /* shift fraction off right */
		*iptr -= FMAXPOWTWO; /* shift back without fraction */
		while (*iptr > absvalue) /* above arithmetic might round */
			*iptr -= (float)1.0; /* test again just to be sure */
		if (value < (float)0.0)
			*iptr = -*iptr;
	}
	return (value - *iptr); /* signed fractional part */
}
