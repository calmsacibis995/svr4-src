/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/gconvert.c	1.2.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * gcvt  - Floating output conversion to minimal length string
 */

#include <fp.h>

void
_gcvt(ndigit, pd, trailing, buf)
	int             ndigit;
	decimal_record *pd;
	char           *buf;
{
	char           *p, *pstring;
	int             i;
	static char     inf8[] = "Infinity";
	static char     inf3[] = "Inf";
	static char     nan[] = "NaN";

	p = buf;
	if (pd->sign)
		*(p++) = '-';
	switch (pd->fpclass) {
	case fp_zero:
		*(p++) = '0';
		break;
	case fp_infinity:
		if (ndigit < 8)
			pstring = inf3;
		else
			pstring = inf8;
		goto copystring;
	case fp_quiet:
	case fp_signaling:
		pstring = nan;
copystring:
		for (i = 0; *pstring != 0;)
			*(p++) = *(pstring++);
		break;
	default:
		if ((pd->exponent > 0) || (pd->exponent < -(ndigit + 3))) {	/* E format. */
			char            estring[4];
			int             n;

			i = 0;
			*(p++) = pd->ds[0];
			*(p++) = '.';
			for (i = 1; pd->ds[i] != 0;)
				*(p++) = pd->ds[i++];
			if ((ndigit == 1) || (trailing == 0)) {	/* Remove trailing zeros
								 * and . */
				p--;
				while (*p == '0')
					p--;
				if (*p != '.')
					p++;
			}
			*(p++) = 'e';
			n = pd->exponent + i - 1;
			if (n >= 0)
				*(p++) = '+';
			else {
				*(p++) = '-';
				n = -n;
			}
			(void) _fourdigits(n, estring);
			for (i = 0; estring[i] == '0'; i++);	/* Find end of zeros. */
			if (i > 2)
				i = 2;	/* Guarantee two zeros. */
			for (; i <= 3;)
				*(p++) = estring[i++];	/* Copy exp digits. */
		} else {	/* F format. */
			if (pd->exponent > -(ndigit + 1)) {	/* x.xxx */
				for (i = 0; i < (ndigit + pd->exponent);)
					*(p++) = pd->ds[i++];
				*(p++) = '.';
				if (pd->ds[i] != 0) {	/* More follows point. */
					for (; i < ndigit;)
						*(p++) = pd->ds[i++];
				}
			} else {/* 0.00xxxx */
				*(p++) = '0';
				*(p++) = '.';
				for (i = 0; i < -(pd->exponent + ndigit); i++)
					*(p++) = '0';
				for (i = 0; pd->ds[i] != 0;)
					*(p++) = pd->ds[i++];
			}
			if ((ndigit == 1) || (trailing == 0)) {	/* Remove trailing zeros
								 * and point. */
				p--;
				while (*p == '0')
					p--;
				if (*p != '.')
					p++;
			}
		}
	}
	*(p++) = 0;
}

char           *
gconvert(number, ndigit, trailing, buf)
	double          number;
	int             ndigit, trailing;
	char           *buf;
{
	decimal_mode    dm;
	decimal_record  dr;
	fp_exception_field_type fef;

	dm.rd = fp_direction;
	dm.df = floating_form;
	dm.ndigits = ndigit;
	double_to_decimal(&number, &dm, &dr, &fef);
	_gcvt(ndigit, &dr, trailing, buf);
	return (buf);
}
