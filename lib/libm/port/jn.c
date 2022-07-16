/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/jn.c	1.6"
/*LINTLIBRARY*/
/*
 *	Double-precision Bessel's function of
 *	the first and second kinds and of
 *	integer order.
 *
 *	jn(n, x) returns the value of Jn(x) for all
 *	integer values of n and all real values
 *	of x.
 *	Returns ERANGE error and value 0 for large arguments.
 *	Calls j0, j1.
 *
 *	For n = 0, j0(x) is called,
 *	For n = 1, j1(x) is called.
 *	For n < x, forward recursion is used starting
 *	from values of j0(x) and j1(x).
 *	For n > x, a continued fraction approximation to
 *	j(n, x)/j(n - 1, x) is evaluated and then backward
 *	recursion is used starting from a supposed value
 *	for j(n, x).  The resulting value of j(0, x) is
 *	compared with the actual value to correct the
 *	supposed value of j(n, x).
 *
 *	yn(n, x) is similar in all respects, except
 *	that y0 and y1 are called, and that forward recursion
 *	is used for values of n > 1.
 *	Returns EDOM error and value -HUGE if argument <= 0.
 */

#ifdef __STDC__
	#pragma weak jn = _jn
	#pragma weak yn = _yn
#endif

#include "synonyms.h"
#include <math.h>
#include <values.h>
#include <errno.h>

/* error codes for jn_error() */
#define	UNDER 0
#define LOSS 1
static double jn_error();


double jn(n, x)
register int n;
register double x;
{
	double a, b, temp, t;
	int i;

	if (_ABS(x) > X_TLOSS)
		return (jn_error(n, x, 1, LOSS));
	if (n == 0)
		return (j0(x));
	if (x == 0)
		return (x);
	if (n < 0) {
		n = -n;
		x = -x;
	}
	if (n == 1)
		return (j1(x));
	if (n <= x) {
		a = j0(x);
		b = j1(x);
		for (i = 1; i < n; i++) {
			temp = b;
			b = (i + i)/x * b - a;
			a = temp;
		}
		return (b);
	}
	temp = x * x;
	for (t = 0, i = n + 16; i > n; i--)
		t = temp/(i + i - t);
	a = t = x/(n + n - t);
	b = 1;
	for (i = n - 1; i > 0; i--) {
		register double c;
		double c1, b1;
		temp = b;
		c = (i + i)/x;
		c1 = _ABS(c);
		b1 = _ABS(b);
		if (c1 >= 1.0) { /* check for overflow before mult 
				 * must check 1st for which of
				 * terms is >1, otherwise the
				 * dvision with MAXDOUBLE could 
				 * overflow
				 */
			if (b1 > MAXDOUBLE/c1)
				return jn_error(n, x, 1, UNDER);
		}
		else if (b1 >= 1.0)
			if (c1 > MAXDOUBLE/b1)
				return jn_error(n, x, 1, UNDER);
		b = c * b - a;
		a = temp;
	}
	return (t * j0(x)/b);
}


double yn(n, x)
register int n;
register double x;
{
	double a, b, temp;
	int i, neg;

	if (x <= 0) {
		struct exception exc;

		exc.type = DOMAIN;
		exc.name = "yn";
		exc.arg1 = n;
		exc.arg2 = x;
		if (_lib_version == c_issue_4)
			exc.retval = -HUGE;
		else
			exc.retval = -HUGE_VAL;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void) write(2, "yn: DOMAIN error\n", 17);
			errno = EDOM;
		}
		return (exc.retval);
	}
	if (x > X_TLOSS)
		return (jn_error(n, x, 0, LOSS));
	if (n == 0)
		return (y0(x));
	neg = 0;
	if (n < 0) {
		n = -n;
		neg = n % 2;
	}
	b = y1(x);
	if (n > 1) {
		a = y0(x);
		for (i = 1; i < n; i++) {
			temp = b;
			b = (i + i)/x * b - a;
			a = temp;
		}
	}
	return (neg ? -b : b);
}

static double
jn_error(n, x, jnflag, type )
int n;
double x;
int jnflag, type;
{
	struct exception exc;

	if (type == LOSS)
		exc.type = TLOSS;
	else 
		exc.type = UNDERFLOW;
	exc.name = jnflag ? "jn" : "yn";
	exc.arg1 = n;
	exc.arg2 = x;
	exc.retval = 0.0;
	if (_lib_version == strict_ansi)
		errno = ERANGE;
	else if (!matherr(&exc)) {
		if (_lib_version == c_issue_4)
			if ( type == LOSS) {
				(void) write(2, exc.name, 2);
				(void) write(2, ": TLOSS error\n", 14);
			}
		errno = ERANGE;
	}
	return (exc.retval);
}
