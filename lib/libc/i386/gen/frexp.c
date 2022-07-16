/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:libc-i386/gen/frexp.c	1.4"
/*LINTLIBRARY*/
/*
 * frexp(value, eptr)
 * returns a double x such that x = 0 or 0.5 <= |x| < 1.0
 * and stores an integer n such that value = x * 2 ** n
 * indirectly through eptr.
 *
 */
#include "synonyms.h"
#include "shlib.h"
#include <nan.h>

asm	double
xfrexp(val,contwo,ptr)
{
%mem	val,contwo,ptr;
	movl	ptr,%eax
	fldl	val
	fxtract
	fxch	%st(1)
	fistpl	(%eax)
	fidiv	contwo
	incl	(%eax)
}

double
frexp(value, eptr)
double value; /* don't declare register, because of KILLNan! */
int *eptr;
{
	static int contwo = 2;

	KILLNaN(value); /* raise exception on Not-a-Number (3b only) */
	*eptr = 0;
	if (value == 0.0) /* nothing to do for zero */
		return (value);
	return(xfrexp(value,contwo,eptr));
}
