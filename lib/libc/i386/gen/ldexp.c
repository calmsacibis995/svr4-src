/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:libc-i386/gen/ldexp.c	1.6"
/*LINTLIBRARY*/
/*
 *	double ldexp (value, exp)
 *		double value;
 *		int exp;
 *
 *	Ldexp returns value * 2**exp, if that result is in range.
 *	If underflow occurs, it returns zero.  If overflow occurs,
 *	it returns a value of appropriate sign and largest single-
 *	precision magnitude.  In case of underflow or overflow,
 *	the external int "errno" is set to ERANGE.  Note that errno is
 *	not modified if no error occurs, so if you intend to test it
 *	after you use ldexp, you had better set it to something
 *	other than ERANGE first (zero is a reasonable value to use).
 */

#include "synonyms.h"
#include "shlib.h"
#include <values.h>
#include <math.h>
#include <errno.h>
/* Largest signed long int power of 2 */
#define MAXSHIFT	(BITSPERBYTE * sizeof(long) - 2)

asm	double
xldexp(x,n)
{
%mem	x; reg n;
	pushl	n
	fildl	(%esp)
	popl	%eax
	fldl	x
	fscale
	fstp	%st(1)
%mem	x,n;
	fildl	n
	fldl	x
	fscale
	fstp	%st(1)
}

extern double frexp();

double
ldexp(value, exp)
double value;
int exp;
{
	int old_exp;

	if (exp == 0 || value == 0.0) /* nothing to do for zero */
		return (value);
	(void) frexp(value, &old_exp);
	if (exp > 0) {
		if ( (exp > (MAXINT-MAXBEXP))
		/* guard against integer overflow in next addition */
	    	|| ((exp + old_exp) > MAXBEXP) ) {
		/* we have floating point overflow condition */

			errno = ERANGE;
			if (_lib_version == c_issue_4)
				return (value < 0 ? -HUGE : HUGE);
			else
				return (value < 0 ? -HUGE_VAL : HUGE_VAL);
		}
		return (xldexp(value,exp));
	}
	if ( (exp < -(MAXINT-MAXBEXP)) ||
		/* guard against integer overflow in next addition */
		(exp + old_exp < MINBEXP) ) { /* underflow */
		errno = ERANGE;
		return (0.0);
	}
	return (xldexp(value,exp));
}
