/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:i386/log.c	1.5"

/*LINTLIBRARY*/

/*
 *	log returns the natural logarithm of its double-precision argument.
 *	log10 returns the base-10 logarithm of its double-precision argument.
 *	Returns EDOM error and value -HUGE if argument < 0,
 *	ERANGE error and value -HUGE if argument == 0.
 *	Algorithm and coefficients from Cody and Waite (1980).
 *	Calls frexp.
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>

static double log_error();

asm	double xlog(x)
{
%mem	x;
	fldln2				/ load log(e)2
	fldl	x
	fyl2x
}

asm	double xlog10(x)
{
%mem	x;
	fldlg2				/ load log(10)2
	fldl	x
	fyl2x
}

double
log(x)
double x;
{
	if (x <= 0)
		return (log_error(x, "log", 3));

	return(xlog(x));
}

double
log10(x)
double x;
{
	if (x <= 0)
		return (log_error(x, "log10", 5));

	return(xlog10(x));
}

static double
log_error(x, f_name, name_len)
double x;
char *f_name;
unsigned int name_len;
{
	register int zflag = 0;
	struct exception exc;

	if (!x) 
		zflag = 1;
	exc.name = f_name;
	if (_lib_version == c_issue_4)
		exc.retval = -HUGE;
	else
		exc.retval = -HUGE_VAL;
	exc.arg1 = x;
	if (_lib_version == strict_ansi) {
		if (!zflag)
			errno = EDOM;
		else 
			errno = ERANGE;
	}
	else {
		if (zflag)
			exc.type = SING;
		else
			exc.type = DOMAIN;
		if (!matherr(&exc)) {
			if (_lib_version == c_issue_4) {
				(void)write(2, f_name, name_len);
				if (zflag)
					(void)write(2,": SING error\n",13);
				else
					(void)write(2,": DOMAIN error\n",15);
			}
			errno = EDOM;
		}
	}
	return (exc.retval);
}
