/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/decimal_bin.c	1.1.3.1"

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

/* Conversion between binary and decimal floating point. */

#include <sunfp.h>

void 
decimal_to_binary_integer(ps, ndigs, ntz, pb)
	char            ps[];	/* Decimal integer string input. */
unsigned        ndigs;		/* Number of explicit digits to read. */
unsigned        ntz;		/* Number of implicit trailing zeros after
				 * digits. */
big_float      *pb;		/* Pointer to big_float to return result. */

/*
 * Converts an explicit decimal string *ps[0]..*ps[ndigs-1] followed by ntz
 * implicit trailing zeros before the point into a big_float at *pb. If the
 * input is too big to fit exactly in a big_float, the least significant bit
 * of pb->significand[5] is stuck on. If the input is too big for a
 * decimal_binary_buffer, pb->exponent is set to 0x7ffffff. 
 */

{
	int             i, lastbuffer, firstbuffer;
	unsigned        t, j, factor;
	unsigned        carry;
	decimal_binary_integer_buffer b;

	if (ndigs < 1)
		return;
	t = ps[0] - '0';
	for (i = 1; (i < ndigs) && (i < 9); i++) {	/* Fast accumulation
							 * loop for <= 9 digits. */
		t = (t << 1) + (t << 3) + ps[i] - '0';
	}
	b[0] = t & 0xffff;
	b[1] = t >> 16;
	if (b[1] > 0)
		lastbuffer = 1;
	else
		lastbuffer = 0;
	for (; i < ((int) ndigs - 3); i = i + 4) {	/* Multiply by 10**4 and
							 * add four more digits. */
		carry = (((ps[i]) * 10 + ps[i + 1]) * 10 + ps[i + 2]) * 10 + ps[i + 3] - 1111 * '0';
		(void) _mul_10000(b, lastbuffer + 1, &carry);
		j = lastbuffer + 1;
		if (carry > 0) {/* Add another link to buffer. */
			lastbuffer++;
			if (lastbuffer >= DECIMAL_BINARY_INTEGER_SIZE) {
				pb->exponent = 0x7fffff;
				return;
			}
			b[lastbuffer] = carry;
		}
	}
	factor = 0;
	switch (ndigs - i) {
	case 3:
		factor = 1000;
		carry = (ps[i] * 10 + ps[i + 1]) * 10 + ps[i + 2] - 111 * '0';
		break;
	case 2:
		factor = 100;
		carry = ps[i] * 10 + ps[i + 1] - 11 * '0';
		break;
	case 1:
		factor = 10;
		carry = ps[i] - '0';
		break;
	}
	if (factor > 0) {
		for (j = 0; j <= lastbuffer; j++) {	/* Multiply buffer by
							 * power of 10. */
			t = b[j] * factor + carry;
			carry = t >> 16;
			b[j] = t & 0xffff;
		}
		if (carry > 0) {/* Add another link to buffer. */
			lastbuffer++;
			if (lastbuffer >= DECIMAL_BINARY_INTEGER_SIZE) {
				pb->exponent = 0x7fffff;
				return;
			}
			b[lastbuffer] = carry;
		}
	}
	firstbuffer = 0;	/* Once the least significant part of the
				 * integer is zero, it stays zero from now
				 * on. */
	for (i = 0; i < ((int) ntz - 3); i = i + 4) {	/* Multiply by 10**4. */
		carry = 0;
		(void) _mul_10000(&(b[firstbuffer]), lastbuffer - firstbuffer + 1, &carry);
		j = lastbuffer + 1;
		if (carry > 0) {/* Add another link to buffer. */
			lastbuffer++;
			if (lastbuffer >= DECIMAL_BINARY_INTEGER_SIZE) {
				pb->exponent = 0x7fffff;
				return;
			}
			b[lastbuffer] = carry;
		}
		if (b[firstbuffer] == 0)
			firstbuffer++;
	}
	factor = 0;
	switch (ntz - i) {
	case 3:
		factor = 1000;
		break;
	case 2:
		factor = 100;
		break;
	case 1:
		factor = 10;
		break;
	}
	if (factor > 0) {
		carry = 0;
		for (j = firstbuffer; j <= lastbuffer; j++) {	/* Multiply buffer by
								 * power of 10. */
			t = b[j] * factor + carry;
			carry = t >> 16;
			b[j] = t & 0xffff;
		}
		if (carry > 0) {/* Add another link to buffer. */
			lastbuffer++;
			if (lastbuffer >= DECIMAL_BINARY_INTEGER_SIZE) {
				pb->exponent = 0x7fffff;
				return;
			}
			b[lastbuffer] = carry;
		}
	}
	for (i = 0; (i < 6) && (i <= lastbuffer); i++)
		pb->significand[i] = b[lastbuffer - i];
	/* Reverse order of big digits. */
	for (; (i < 6); i++)
		pb->significand[i] = 0;
	/* Clear remaining significand. */
	for (; (b[i] == 0) && (i <= lastbuffer); i++);
	/* Search for discarded bits. */
	if (i <= lastbuffer)
		pb->significand[5] |= 1;	/* Set sticky bit. */
	pb->exponent = lastbuffer + 1;
}

void 
decimal_to_binary_fraction(ps, ndigs, nlz, pb, significant_bits)
	char            ps[];	/* Decimal integer string input. */
unsigned        ndigs;		/* Number of explicit digits to read. */
unsigned        nlz;		/* Number of implicit leading zeros before
				 * digits. */
big_float      *pb;		/* Pointer to big_float result. */
unsigned        significant_bits;	/* Number of significant bits needed
					 * output. */

/*
 * Converts an explicit decimal string *ps[0]..*ps[ndigs-1] preceded by nlz
 * implicit leading zeros after the point into a big_float at *pb. If the
 * input does not fit exactly in a big_float, the least significant bit of
 * pb->significand[5] is stuck on. If the input is too big for a
 * decimal_binary_integer_buffer, pb->exponent is set to -0x7ffffff. 
 */

#define DIGIT(n) (ps[n] - '0')
{
	decimal_binary_integer_buffer f;	/* Buffer of words of base
						 * 10000 to be used for
						 * decimal digits. */
	unsigned        zerobound;
	int             i, pf;
	unsigned        nonzero, t;
	unsigned        carry;
	unsigned        ib, eb, wordswanted, wordscomputed;

	if ((nlz + ndigs) > (4 * DECIMAL_BINARY_INTEGER_SIZE)) {	/* Overflow buffer -
									 * number too small. */
		pb->exponent = -0x7ffffff;
		return;
	}
	zerobound = nlz / 4;
	pf = zerobound;		/* Words to left of zerobound are all zero. */
	switch (nlz % 4) {	/* Handle 1..3 odd leading zeros. */
	case 0:
		i = 0;
		t = 0;
		break;
	case 1:
		t = 100 * DIGIT(0);
		if (ndigs >= 2)
			t += 10 * DIGIT(1);
		if (ndigs >= 3)
			t += DIGIT(2);
		f[pf++] = t;
		i = 3;
		break;
	case 2:
		t = 10 * DIGIT(0);
		if (ndigs >= 2)
			t += DIGIT(1);
		f[pf++] = t;
		i = 2;
		break;
	case 3:
		t = DIGIT(0);
		f[pf++] = t;
		i = 1;
		break;
	}
	nonzero = t;
	if (ndigs >= 4)
		for (; i < (ndigs - 3); i = i + 4) {	/* Convert digits in
							 * groups of four. */
			t = ((DIGIT(i) * 10 + DIGIT(i + 1)) * 10 + DIGIT(i + 2)) * 10 + DIGIT(i + 3);
			nonzero = nonzero | t;
			f[pf++] = t;
		}
	switch (ndigs - i) {	/* Pick up 1..3 digits left over. */
	case 1:
		f[pf++] = 1000 * DIGIT(i);
		break;
	case 2:
		f[pf++] = 1000 * DIGIT(i) + 100 * DIGIT(i + 1);
		break;
	case 3:
		f[pf++] = 1000 * DIGIT(i) + 100 * DIGIT(i + 1) + 10 * DIGIT(i + 2);
		break;
	}
	if (pf > zerobound)
		nonzero = nonzero | f[pf - 1];
	if (nonzero == 0) {	/* Fraction is identically zero. */
		pb->exponent = 0;
		pb->significand[0] = 0;
		pb->significand[1] = 0;
		pb->significand[2] = 0;
		pb->significand[3] = 0;
		pb->significand[4] = 0;
		pb->significand[5] = 0;
		return;
	}
	/*
	 * Now the array f contains a base-10000 fraction of pf digits.
	 * Convert to base 2**16 by multiplying by 2**16 and taking integer
	 * parts. 
	 */

	wordswanted = 2 + (significant_bits / 16);	/* Number of base 2**16
							 * digits required. */
	ib = 0;
	eb = 0;
	wordscomputed = 0;
	while (wordscomputed < wordswanted) {	/* Compute another leading
						 * zero or significant word. */
		carry = 0;
		(void) _mul_65536(&(f[pf - 1]), -(pf - zerobound), &carry);
		if (ib > 0) {	/* Carry is significant even if zero. */
			pb->significand[ib++] = carry;
			wordscomputed++;
		} else
			while (carry > 0) {	/* Carried past leading zero
						 * boundary. */
				if (zerobound > 0) {	/* Adjust leading zero
							 * boundary. */
					carry = _quorem10000(carry, &(f[--zerobound]));
				} else {	/* Output significant digits. */
					pb->significand[ib++] = carry;
					wordscomputed++;
					carry = 0;
				}
			}
		if (ib == 0)
			eb++;	/* Count leading zeros produced. */
	}
	pb->exponent = -eb;
	for (i = wordscomputed; i < 6; i++)
		pb->significand[i] = 0;
	for (i = zerobound; (i < pf) && (f[i] == 0); i++);	/* Search for lost
								 * nonzero digits. */
	if (i < pf)
		pb->significand[5] |= 1;	/* Sticky inexact bit. */
}

void 
decimal_to_unpacked(px, pd, significant_bits)
	unpacked       *px;
	decimal_record *pd;
	unsigned        significant_bits;

/*
 * Converts *pd to *px so that *px can be correctly rounded. significant_bits
 * tells how many bits will be significant in the final result to avoid
 * superfluous computation. Inexactness is communicated by sticking on the
 * lsb of px->significand[2]. Integer buffer overflow is indicated with a
 * huge positive exponent. 
 */

{
	int             i, frac_bits, expdiff;
	unsigned        length, ndigs, ntz, nlz, ifrac, nfrac, morebits = 0;
	big_float       bi, bf;

	px->sign = pd->sign;
	px->fpclass = pd->fpclass;
	if ((px->fpclass != fp_normal) && (px->fpclass != fp_subnormal))
		return;
	for (length = 0; pd->ds[length] != 0; length++);
	if (length == 0) { /* A zero significand slipped by. */ 
		px->fpclass = fp_zero;
		return;
		}
	/* Length contains the number of explicit digits in string. */
	if (pd->exponent >= 0) {/* All integer digits. */
		ndigs = length;
		ntz = pd->exponent;	/* Trailing zeros. */
		ifrac = 0;
		nfrac = 0;	/* No fraction digits. */
		nlz = 0;
	} else if (length <= -pd->exponent) {	/* No integer digits. */
		ndigs = 0;
		ntz = 0;
		ifrac = 0;
		nfrac = length;
		nlz = -pd->exponent - length;	/* Leading zeros. */
	} else {		/* Some integer digits, some fraction digits. */
		ndigs = length + pd->exponent;
		ntz = 0;
		ifrac = ndigs;
		nfrac = -pd->exponent;
		nlz = 0;
	}
	if (ndigs > 0) {	/* Convert integer digits. */
		decimal_to_binary_integer(pd->ds, ndigs, ntz, &bi);
		if (bi.exponent == 0x7fffff) {	/* Too big for buffer. */
			px->exponent = bi.exponent;
			px->significand[0] = 0x80000000;
			return;
		}
	} else {		/* No integer digits. */
		bi.significand[0] = 0;
		bi.exponent = 0;
	}
	if (bi.exponent <= 1)
		frac_bits = 1 + significant_bits;
	else
		frac_bits = 17 + significant_bits - 16 * bi.exponent;
	if ((nfrac > 0) && (frac_bits > 0)) {	/* Convert fraction digits. */
		decimal_to_binary_fraction(&(pd->ds[ifrac]), nfrac, nlz, &bf, (unsigned) frac_bits);
		if (bi.significand[0] == 0) {	/* No integer digits; all
						 * fraction. */
			if (bf.exponent == -0x7ffffff) {	/* Buffer overflowed. */
				px->exponent = bf.exponent;
				px->significand[0] = 0x80000000;
				return;
			}
			bi = bf;
		} else {	/* Combine integer and fraction bits. */
			expdiff = bi.exponent - bf.exponent;
			if (bi.significand[5] & 1) {	/* Record and clear
							 * sticky bit from
							 * integer part. */
				morebits = 1;
				bi.significand[5] &= 0xfffffffe;
			}
			if (expdiff <= 5)
				if (bf.significand[5 - expdiff] & 1) {	/* Record and clear
									 * sticky bit from
									 * fraction part. */
					morebits = 1;
					bf.significand[5 - expdiff] &= 0xfffffffe;
				}
			for (i = 0; i <= (5 - expdiff); i++)
				bi.significand[i + expdiff] += bf.significand[i];	/* Add fraction to
											 * integer. */
			for (; i <= 5; i++)
				if (bf.significand[i] != 0)
					morebits = 1;	/* Check discarded
							 * fraction. */
		}
	} else {		/* No fraction digits available or needed. */
		morebits = pd->more;
		if ((morebits == 0) && (nfrac > 0)) {	/* Check if we have
							 * discarded some
							 * significant fraction
							 * digits. */
			for (i = ifrac; (i < length) && (pd->ds[i] == '0'); i++);
			if (i < length)
				morebits = 1;
		}
	}
	if (morebits)
		bi.significand[5] |= 1;	/* Set sticky bit on. */
	px->exponent = 16 * bi.exponent - 1;
	px->significand[0] = (bi.significand[0] << 16) + bi.significand[1];
	px->significand[1] = (bi.significand[2] << 16) + bi.significand[3];
	px->significand[2] = (bi.significand[4] << 16) + bi.significand[5];
	_fp_normalize(px);
}

/* PUBLIC FUNCTIONS */

/* 
 * decimal_to_floating routines convert the decimal record at *pd to the
 * floating type item at *px, observing the modes specified in *pm and
 * setting exceptions in *ps. 
 *
 * atof.3, strtod.3, and through them ?scanf.3s will be rewritten to call
 * decimal_to_double. 
 *
 * pd->sign and pd->fpclass are always taken into account.  pd->exponent and
 * pd->ds are used when pd->fpclass is fp_normal or fp_subnormal. In these
 * cases pd->ds is expected to contain one or more ascii digits followed by a
 * null. px is set to a correctly rounded approximation to
 * (sign)*(ds)*10**(exponent) If pd->more != 0 then additional nonzero digits
 * are assumed to follow those in ds; fp_inexact is set accordingly. 
 *
 * Thus if pd->exponent == -2 and pd->ds = "1234", *px will get 12.34 rounded to
 * storage precision. 
 *
 * px is correctly rounded according to the IEEE rounding modes in pm->rd.  *ps
 * is set to contain fp_inexact, fp_underflow, or fp_overflow if any of these
 * arise. 
 *
 * pd->ndigits, pm->df, and pm->ndigits are never used. 
 *
 */

void 
decimal_to_single(px, pm, pd, ps)
	single         *px;
	decimal_mode   *pm;
	decimal_record *pd;
	fp_exception_field_type *ps;
{
	single_equivalence kluge;
	unpacked        u;

	kluge.f.msw.sign = pd->sign ? 1 : 0;
	switch (pd->fpclass) {
	case fp_zero:
		kluge.f.msw.exponent = 0;
		kluge.f.msw.significand = 0;
		break;
	case fp_infinity:
		kluge.f.msw.exponent = 0xff;
		kluge.f.msw.significand = 0;
		break;
	case fp_quiet:
		kluge.f.msw.exponent = 0xff;
		kluge.f.msw.significand = 0x7fffff;
		break;
	case fp_signaling:
		kluge.f.msw.exponent = 0xff;
		kluge.f.msw.significand = 0x3fffff;
		break;
	default:
		if (pd->exponent > SINGLE_MAXE) {	/* Guaranteed overflow. */
			u.sign = pd->sign == 0 ? 0 : 1;
			u.fpclass = fp_normal;
			u.exponent = 0x000fffff;
			u.significand[0] = 0x80000000;
		} else if (pd->exponent >= -SINGLE_MAXE) {	/* Guaranteed in range. */
			goto inrange;
		} else if (pd->exponent <= (-SINGLE_MAXE - DECIMAL_STRING_LENGTH)) {	/* Guaranteed deep
											 * underflow. */
			goto underflow;
		} else {	/* Deep underflow possible, depending on
				 * string length. */
			int             i;

			for (i = 0; (pd->ds[i] != 0) && (i < (-pd->exponent - SINGLE_MAXE)); i++);
			if (i < (-pd->exponent - SINGLE_MAXE)) {	/* Deep underflow */
		underflow:
				u.sign = pd->sign == 0 ? 0 : 1;
				u.fpclass = fp_normal;
				u.exponent = -0x000fffff;
				u.significand[0] = 0x80000000;
			} else {/* In range. */
		inrange:
				decimal_to_unpacked(&u, pd, 24);
			}
		}
		_fp_current_exceptions = 0;
		_fp_current_direction = pm->rd;
		_pack_single(&u, &kluge.x);
		*ps = _fp_current_exceptions;
	}
	*px = kluge.x;
}

void 
decimal_to_double(px, pm, pd, ps)
	double         *px;
	decimal_mode   *pm;
	decimal_record *pd;
	fp_exception_field_type *ps;
{
	double_equivalence kluge;
	unpacked        u;

	kluge.f.msw.sign = pd->sign ? 1 : 0;
	switch (pd->fpclass) {
	case fp_zero:
		kluge.f.msw.exponent = 0;
		kluge.f.msw.significand = 0;
		kluge.f.significand2 = 0;
		break;
	case fp_infinity:
		kluge.f.msw.exponent = 0x7ff;
		kluge.f.msw.significand = 0;
		kluge.f.significand2 = 0;
		break;
	case fp_quiet:
		kluge.f.msw.exponent = 0x7ff;
		kluge.f.msw.significand = 0xfffff;
		kluge.f.significand2 = 0xffffffff;
		break;
	case fp_signaling:
		kluge.f.msw.exponent = 0x7ff;
		kluge.f.msw.significand = 0x7ffff;
		kluge.f.significand2 = 0xffffffff;
		break;
	default:
		if (pd->exponent > DOUBLE_MAXE) {	/* Guaranteed overflow. */
			u.sign = pd->sign == 0 ? 0 : 1;
			u.fpclass = fp_normal;
			u.exponent = 0x000fffff;
			u.significand[0] = 0x80000000;
		} else if (pd->exponent >= -DOUBLE_MAXE) {	/* Guaranteed in range. */
			goto inrange;
		} else if (pd->exponent <= (-DOUBLE_MAXE - DECIMAL_STRING_LENGTH)) {	/* Guaranteed deep
											 * underflow. */
			goto underflow;
		} else {	/* Deep underflow possible, depending on
				 * string length. */
			int             i;

			for (i = 0; (pd->ds[i] != 0) && (i < (-pd->exponent - DOUBLE_MAXE)); i++);
			if (i < (-pd->exponent - DOUBLE_MAXE)) {	/* Deep underflow */
		underflow:
				u.sign = pd->sign == 0 ? 0 : 1;
				u.fpclass = fp_normal;
				u.exponent = -0x000fffff;
				u.significand[0] = 0x80000000;
			} else {/* In range. */
		inrange:
				decimal_to_unpacked(&u, pd, 53);
			}
		}
		_fp_current_exceptions = 0;
		_fp_current_direction = pm->rd;
		_pack_double(&u, &kluge.x);
		*ps = _fp_current_exceptions;
	}
	*px = kluge.x;
}

void 
decimal_to_extended(px, pm, pd, ps)
	extended       *px;
	decimal_mode   *pm;
	decimal_record *pd;
	fp_exception_field_type *ps;
{
	extended_equivalence kluge;
	unpacked        u;

	kluge.f.msw.sign = pd->sign ? 1 : 0;
	switch (pd->fpclass) {
	case fp_zero:
		kluge.f.msw.exponent = 0;
		kluge.f.significand = 0;
		kluge.f.significand2 = 0;
		break;
	case fp_infinity:
		kluge.f.msw.exponent = 0x7fff;
		kluge.f.significand = 0;
		kluge.f.significand2 = 0;
		break;
	case fp_quiet:
		kluge.f.msw.exponent = 0x7fff;
		kluge.f.significand = 0xffffffff;
		kluge.f.significand2 = 0xffffffff;
		break;
	case fp_signaling:
		kluge.f.msw.exponent = 0x7fff;
		kluge.f.significand = 0x3fffffff;
		kluge.f.significand2 = 0xffffffff;
		break;
	default:
		if (pd->exponent > EXTENDED_MAXE) {	/* Guaranteed overflow. */
			u.sign = pd->sign == 0 ? 0 : 1;
			u.fpclass = fp_normal;
			u.exponent = 0x000fffff;
			u.significand[0] = 0x80000000;
		} else if (pd->exponent >= -EXTENDED_MAXE) {	/* Guaranteed in range. */
			goto inrange;
		} else if (pd->exponent <= (-EXTENDED_MAXE - DECIMAL_STRING_LENGTH)) {	/* Guaranteed deep
											 * underflow. */
			goto underflow;
		} else {	/* Deep underflow possible, depending on
				 * string length. */
			int             i;

			for (i = 0; (pd->ds[i] != 0) && (i < (-pd->exponent - EXTENDED_MAXE)); i++);
			if (i < (-pd->exponent - EXTENDED_MAXE)) {	/* Deep underflow */
		underflow:
				u.sign = pd->sign == 0 ? 0 : 1;
				u.fpclass = fp_normal;
				u.exponent = -0x000fffff;
				u.significand[0] = 0x80000000;
			} else {/* In range. */
		inrange:
				decimal_to_unpacked(&u, pd, 64);
			}
		}
		_fp_current_exceptions = 0;
		_fp_current_direction = pm->rd;
		_pack_extended(&u, px);
		*ps = _fp_current_exceptions;
		return;
	}
	(*px)[0] = kluge.x[0];
	(*px)[1] = kluge.x[1];
	(*px)[2] = kluge.x[2];
}
