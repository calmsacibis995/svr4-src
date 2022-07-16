/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/ecvt.c	2.19"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 *	ecvt converts to decimal
 *	the number of digits is specified by ndigit
 *	decpt is set to the position of the decimal point
 *	sign is set to 0 for positive, 1 for negative
 *
 */
#ifndef DSHLIB
#ifdef __STDC__
	#pragma weak ecvt = _ecvt
	#pragma weak fcvt = _fcvt
#endif
#endif
#include "synonyms.h"
#include "shlib.h"
#include <values.h>
#include <nan.h>
#include <string.h>

#define	NMAX	((DSIGNIF * 3 + 19)/10) /* restrict max precision */
#define	NDIG	80

static char *cvt();

char *
ecvt(value, ndigit, decpt, sign)
double	value;
int	ndigit, *decpt, *sign;
{
	return (cvt(value, ndigit, decpt, sign, 0));
}

char *
fcvt(value, ndigit, decpt, sign)
double	value;
int	ndigit, *decpt, *sign;
{
	return (cvt(value, ndigit, decpt, sign, 1));
}

static char buf[NDIG];

static char *
cvt(value, ndigit, decpt, sign, f_flag)
double value;
int	ndigit, *sign, f_flag;
register int	*decpt;
{
	register char *p = &buf[0], *p_last = &buf[ndigit];

	buf[0] = '\0';

	if(IsNANorINF(value)) {
		if(IsINF(value))  /* value is an INF, return "inf" */
			strncpy(buf, "inf", NDIG);
		else /* value is a NaN, return "NaN" */
			strncpy(buf, "nan", NDIG);

		return(buf);
	}
	
	if (*sign = (value < 0.0))
		value = -value;
	*decpt = 0;
	if (value != 0.0) { /* rescale to range [1.0, 10.0) */
		/* in binary for speed and to minimize error build-up */
		/* even for the IEEE standard with its high exponents,
		   it's probably better for speed to just loop on them */
		static const struct s { double p10; int n; } s[] = {
			1e32,	32,
			1e16,	16,
			1e8,	8,
			1e4,	4,
			1e2,	2,
			1e1,	1,
		};
		register const struct s *sp = s;

		++*decpt;
		if (value >= 2.0 * MAXPOWTWO) /* can't be precisely integral */
			do {
				for ( ; value >= sp->p10; *decpt += sp->n)
					value /= sp->p10;
			} while (sp++->n > 1);
		else if (value >= 10.0) { /* convert integer part separately */
			register double pow10 = 10.0, powtemp;

			while ((powtemp = 10.0 * pow10) <= value)
				pow10 = powtemp;
			for ( ; ; pow10 /= 10.0) {
				register int digit = value/pow10;
				*p++ = digit + '0';
				value -= digit * pow10;
				++*decpt;
				if (pow10 <= 10.0)
					break;
			}
		} else if (value < 1.0)
			do {
				for ( ; value * sp->p10 < 10.0; *decpt -= sp->n)
					value *= sp->p10;
			} while (sp++->n > 1);
	}
	if (f_flag)
		p_last += *decpt;
	if (p_last >= buf) {
		if (p_last > &buf[NDIG - 2])
			p_last = &buf[NDIG - 2];
		for ( ; ; ++p) {
			if (value == 0 || p >= &buf[NMAX])
				*p = '0';
			else {
				register int intx; /* intx in [0, 9] */
				*p = (intx = (int)value) + '0';
				value = 10.0 * (value - (double)intx);
			}
			if (p >= p_last) {
				p = p_last;
				break;
			}
		}
		if (*p >= '5') /* check rounding in last place + 1 */
			do {
				if (p == buf) { /* rollover from 99999... */
					buf[0] = '1'; /* later digits are 0 */
					++*decpt;
					if (f_flag)
						++p_last;
					break;
				}
				*p = '0';
			} while (++*--p > '9'); /* propagate carries left */
		*p_last = '\0';
	}
	return (buf);
}
