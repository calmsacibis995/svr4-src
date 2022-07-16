/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:i386/logf.c	1.3"

/*LINTLIBRARY*/

/*
 *	logf returns the natural logarithm of its single-precision argument.
 *	log10f returns the base-10 logarithm of its single-precision argument.
 *	Returns EDOM error and value -HUGE if argument < 0,
 *	ERANGE error and value -HUGE if argument == 0.
 *	Algorithm and coefficients from Cody and Waite (1980).
 *	Calls frexp.
 */

#include "synonyms.h"
#include <math.h>
#include <errno.h>

static float log_error(float, char*, int);

asm	float xlogf(float x)
{
%mem	x;
	fldln2				/ load log(e)2
	flds	x
	fyl2x
}

asm	float xlog10f(float x)
{
%mem	x;
	fldlg2				/ load log(10)2
	flds	x
	fyl2x
}

float
logf(float x)
{
	float xlogf(float);
	if (x <= (float)0.0)
		return (log_error(x, "logf", 4));

	return(xlogf(x));
}

float
log10f(float x)
{
	float xlog10f(float);
	if (x <= (float)0.0)
		return (log_error(x, "log10f", 6));

	return(xlog10f(x));
}

static float
log_error(float x, char *f_name,int  name_len)
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
	exc.arg1 = (double)x;
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
	return ((float)exc.retval);
}
