/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:gen/ptod.c	1.3"
/*
 *	Simulated extended precision floating point conversion functions
 *      ----------------------------------------------------------------
 *
 *	The most important thing to understand is the internal
 *	representation of the extended floating point numbers.
 *	They are stored as a set of five 14-bit numbers (for a
 *	total of 70 bits) in w1, w2, w3, w4, w5. It is the low-order
 *	14 bits of each word which are significant. There is no
 *	implied leading 1-bit.  The implied binary point precedes
 *	all 70 bits.  In addition, an unbiased binary exponent is
 *	stored in binexp.
 *
 *	The above is the concept.  But the program is meant to be
 *	accurate and fast, and so it is not always the case that
 *	w1-w5 each contain only 14 significant bits.  For example,
 *	w1 may contain a fifteen bit number.  When it does, the
 *	leading bit should be interpreted as preceding the implied
 *	binary point--in other words, the binary point does not
 *	move just because the number is 71 bits instead of 70.
 *
 *	After two 70-bit (or 71-bit) numbers are multiplied, w1-w4
 *	generally contain lots of big numbers.  Now for analogy
 *	observe that in the C language, 9 is a legitimate octal
 *	digit, so that 09 = 011, 019 = 021, 099 = 0121, and so 
 *	on.  The numbers in w1-w4 after a multiplication may be
 *	thought of as 14-bit "digits".  They may be bigger than
 *	14 bits, but that doesn't matter any more than it matters
 *	than 9 is bigger than 3 bits when it is treated as an octal
 *	digit.  The point is that the *	high-order bits of a word
 *	are really part of the next higher-order word.
 *
 *	Well, then, after a multiplication, w1 contains the 28
 *	most significant bits of the 70 bit result, w2 contains
 *	the next 14 bits, w3 the next 14, and so on.  Before another
 *	multiplication may be done, the number must be reorganized
 *	into strictly legitimate 14-bit pieces.  An occasional
 *	exception may be made (in this program, the leading word
 *	is allowed to be 15 bits), but it is essential that the
 *	sum of six products of pairs of words not exceed 31 bits;
 *	and because two 14-bit quantities multiply to give a 27 or
 *	28 bit quantity, excessive exceptions to the rule result
 *	in overflow and destruction of information.
 *
 *	If no further multiplication is to be done, wasted effort
 *	may be avoided by not immediately reorganizing the result.
 *	In such a case one must remember that the implied binary
 *	point precedes 28 bits of w1, rather than just 14 bits of w1.
 *
 *	It is permissible for w1 to accumulate more than 28 bits of
 *	the *	product.  In this case, once again, the 29th (and
 *	30th) bit, counting from right to left, should be thought
 *	of as preceding the implied binary point.
 *
 *	During part of the pair-to-floating conversion, floating
 *	arithmetic is not used at all.  The set w1-w5 are used as
 *	a 70-bit store for extended precision FIXED POINT arithmetic.
 *	This section of code may be confusing.
 *
 *	There are also times when w1-w5 may contain values completely
 *	unrelated to any floating point number.  w1-w4 are registers,
 *	and, when free, make convenient scratch variables.
 */

#include "synonyms.h"
#include <errno.h>
#include <math.h>
#include "values.h"

/*	macros and constants describing internal format	*/
#define WSIZE 14
#define WMASK LOW(WSIZE)
/*	general purpose macros and constants	*/
#define LOW(x) ((1 << (x)) - 1)
#if (i386)
#define EXP_BIAS	1024
#define EXP_SIZE	11	/* size of exponent	*/
#endif
#if (vax)
#define EXP_BIAS	128
#define EXP_SIZE	8	/* size of exponent	*/
#endif

extern double frexp(), ldexp();

/* Format of a simulated extended precision power of ten.  These 
 * numbers contain 71 bits; the first 70 bits are obtained by taking
 * the ceiling at the 70th bit.  The idea is roughly to compensate
 * for truncation errors during multiplication.  The 71st bit is whatever
 * actually belongs there.  For obtaining 64 bits of accuracy, exactly
 * how the 71st bit is handled is not critical.
 *
 * Each number comes with a binary exponent.
 */

struct simex {
	short signif[5];
	short expo;
};

/*
 * This table contains, in simulated extended format, 1e320, 1e288,
 * 1e256, 1e224, etc., descending by factors of 1e32.
 * The lowest power in the table is 1e-352.
 */
extern struct simex _bigpow[];

/*
 * This table contains 1e16, 1e15, 1e14, etc., down to 1e-15.
 */

extern struct simex _litpow[];


/*
 *	_ptod(high, low, lowlen, dec_exp, sign)
 *
 *	Convert a pair of longs to a scaled double.
 *
 *	_ptod returns ((high * 10**lowlen) + low) * 10**dec_exp * (-1)**sign,
 *	where "sign" must be 0 or 1, "lowlen" must be in [0, 10],
 *	and "low" must be zero if "lowlen" is zero.
 *	If "high" is zero, "low" is assumed to be zero also.
 *
 */

double
_ptod(w1, low, lowlen, dec_exp, sign)
register long w1;
long low;
int dec_exp, lowlen, sign;
{
	register long w2, w3, w4;
	long w5;
	int binexp;


	/* make sure it doesn't run of the bigpow table */
	if (dec_exp > 310) {
		errno = ERANGE;
		if (_lib_version == c_issue_4)
			return (sign ? -HUGE : HUGE);
		else
			return (sign ? -HUGE_VAL : HUGE_VAL);
	} else if (dec_exp < -350) {
		errno = ERANGE;
		return(0.0);
	}

	if (w1 == 0)
		return(0.0);

	/*
	 * Using extended fixed point arithmetic, multiply "high" (w1)
	 * times 10**lowlen and add "low".
	 */

	if (lowlen > 0) {
		{	register short *m;
			m = _litpow[16 - lowlen].signif;

			/* Make a 42-bit integer (w1, w2, w3) out of "high" */

			w2 = WMASK;
			w3 = w1 & w2;
			w1 >>= WSIZE;
			w2 &= w1;
			w1 >>= WSIZE;

			/* Multiply by 10**lowlen to get a 70-bit exact
			   product in (w2, w3, w4, w5) -- w2 gets the
			   high-order 28 bits and the rest each get 14.
			   The binary point precedes (28 - expo)
			   bits of the result, where "expo" is loaded into
			   w1 below.
			 */

			w5 = w3*m[1];
			w4 = w3*m[0] + w2*m[1];
			w3 = w2*m[0] + w1*m[1];
			w2 = w1*m[0];

			w1 = ((struct simex *) m)->expo;
		}

			/* In the following, t is the number of bits by
			 * which "low" must be shifted left so that it
			 * lines up properly with the product in
			 * (w2, w3, w4, w5).
			 */

		{	register int t = 2*WSIZE - w1;

			if (t > WSIZE) {
				w5 = w4; w4 = w3; w3 = w2; w2 = 0;
				t -= WSIZE;
			} else if (t == -1) { /* If neg., value is -1 */
				w2 <<= 1; w3 <<= 1; w4 <<= 1; w5 <<= 1;
				t = 0;
			}

			/* Shift "low" into position and add it
			 * piecemeal to (w2, w3, w4, w5).
			 * w1 is used for scratch.
			 */

			w1 = low;
			w5 += (w1 & WMASK) << t;
			w1 >>= WSIZE;
			w4 += (w1 & WMASK) << t;
			w1 >>= WSIZE; w1 <<= t;
			w3 += w1;

			/* Set the binary exponent according to the length
			 * of our extended fixed point number and the
			 * number of bits (t) which came after the fixed
			 * binary point.  Now (w1, w2, w3, w4, w5) may be
			 * regarded as a floating point number.
			 */

			binexp = 5*WSIZE - t;

			t = WMASK;
			w4 += (w5 >> WSIZE); w3 += (w4 >> WSIZE);
			w2 += (w3 >> WSIZE); w1 = (w2 >> WSIZE);
			w2 &= t; w3 &= t; w4 &= t; w5 &= t;
		}
	} else { 

		/* Make "high" into an unnormalized floating point
		 * number in (w1, w2, w3, w4, w5).
		 */

		w3 = w1;
		w1 >>= WSIZE;
		w2 = w1;
		w1 >>= WSIZE;
		w4 = WMASK; w3 &= w4; w2 &= w4;
		w5 = w4 = 0;
		binexp = 3*WSIZE;
	}

	/*
	 * Normalize (w1, w2, w3, w4, w5), so that the leading bit of w1
	 * is the 14th bit counting from the right.
	 */

	{	register int t = 0;

		while (w1 == 0) {
			w1 = w2; w2 = w3; w3 = w4; w4 = w5; w5 = 0;
			binexp -= WSIZE;
		}
		for ( ; w1 < (1 << (WSIZE - 4)); w1 <<= 4, t += 4);
		for ( ; w1 < (1 << (WSIZE - 1)); w1 <<= 1, t++);
		binexp -= t;

		w2 <<= t; w3 <<= t; w4 <<= t; w5 <<= t;
		w4 += (w5 >> WSIZE); w3 += (w4 >> WSIZE);
		w2 += (w3 >> WSIZE); w1 += (w2 >> WSIZE);

		t = WMASK; w5 &= t; w4 &= t; w3 &= t; w2 &= t;
	}

	/*
	 * Now multiply (w1, w2, w3, w4, w5) by 10**dec_exp in
	 * two stages.  This is more or less identical to the
	 * scaling operation in _dtop (which see).
	 */

	if ((16 - dec_exp) >> 5 != 0) {
		register short *m;

		m = _bigpow[(336 - dec_exp) >> 5].signif;
		w4 = w4*m[0] + w3*m[1] + w2*m[2] + w1*m[3] +
			     ((w5*m[0] + w4*m[1] + w3*m[2] + w2*m[3] +
							w1*m[4]) >> WSIZE);
		w3 = w3*m[0] + w2*m[1] + w1*m[2] + (w4 >> WSIZE);
		w2 = w2*m[0] + w1*m[1] + (w3 >> WSIZE);
		w1 = w1*m[0] + (w2 >> WSIZE);
		binexp += ((struct simex *) m)->expo;
	} else {
		w1 <<= WSIZE; w1 += w2;
		w2 = w3; w3 = w4; w4 = w5;
	}

	if ((dec_exp & 31) != 0) {
		{	register int t = WMASK;
			w5 = w4 & t; w4 = w3 & t; w3 = w2 & t; w2 = w1 & t;
			w1 >>= WSIZE;
		}

		{	register short *m;

			m = _litpow[(16 - dec_exp) & 31].signif;
			w4 = w4*m[0] + w3*m[1] + w2*m[2] + w1*m[3] +
				((w5*m[0] + w4*m[1] + w3*m[2] + w2*m[3] +
					w1*m[4]) >> WSIZE);
			w3 = w3*m[0] + w2*m[1] + w1*m[2] + (w4 >> WSIZE);
			w2 = w2*m[0] + w1*m[1] + (w3 >> WSIZE);
			w1 = w1*m[0] + (w2 >> WSIZE);
			binexp += ((struct simex *) m)->expo;
		}
	}

	/*
	 * The scaled result now resides in (w1, w2, w3, w4), with
	 * loosely speaking 28 bits in w1, and 14 in the others.
	 * In fact we know from the multiplications above that w1 must
	 * contain 28, 29, or 30 bits.  Now the problem is to extract
	 * a normalized double from (w1, w2, w3, w4).
	 *
	 * For convenience, we normalize the result so that w1 will
	 * contain 30 bits of the result.
	 * Then, taking account of their proper normalized position,
	 * we mash w1, w2, and w3, properly rounded, into the double
	 * precision result.
	 */

	{	register int t = WMASK;
		register int denormexp = 0;
		union {
			double dvalue;
			struct {
				unsigned long upper, lower;
			} longs;
			struct {
				unsigned short s1, s2, s3, s4;
			} shorts;
		} result;

		w3 &= t; w2 &= t;

		/* Normalize to 30 bits */
		for (t = 0; w1 < (1 << 29); w1 <<= 1, t++);
		w2 <<= t; w3 <<= t;

		/* Here t becomes the biased exponent LESS ONE.  The
		 * leading bit of w1 will have the effect of adding
		 * one back into the biased exponent.
		 */

		t = binexp - t + EXP_BIAS - 1;

		if (t < 0)  {
			denormexp = t;
			t = 0;
		}
		if (t > 2045) {
			errno = ERANGE;
			if (_lib_version == c_issue_4)
				return (sign ? -HUGE : HUGE);
			else
				return (sign ? -HUGE_VAL : HUGE_VAL);
		}
		/* The following code does the jamming.  Note that
		   we only need worry about rounding overflow if
		   the number becomes to large to represent.
		   Otherwise, the mantissa becomes 0, and instead
		   of the single hidden bit being added to the 
		   exponent, 2 will be added simply by the overflow
		   into the exponent portion of the double!
		*/
#if (i386)

		result.longs.lower = (sign << 31) + (t << 20) +
					(w1 >> 9);
		result.longs.upper = t = ((w1 & LOW(9)) << 23) +
				(w2 << 9) + ((w3 + 16) >> 5);
		/* test for rounding overflow (see if exp -> neg val) */
		if ((t == 0) && (w2 != 0) &&
		    ((++result.longs.lower >> 20) & LOW(11)) < 0) {
			errno = ERANGE;
			if (_lib_version == c_issue_4)
				return (sign ? -HUGE : HUGE);
			else
				return (sign ? -HUGE_VAL : HUGE_VAL);
		}
#endif
#if (vax)
		/* w4 used as scratch register. w5 is the carry bit */
		w4 = ((w3+15) >> 5); 	 /* w4 has upper 9 bits of w3 */
		w5 = (w4 == 0) && (w3 != 0);	/* w5 has round bit */
		w3 = w2 & LOW(7);	 /* w3 has low 7 bits of w2 */
		w5 = ((result.shorts.s4 = (w3 | w4) + w5) == 0)
			&& (w3 != 0);
		w4 = (w2 >> 7) & LOW(7); /* w4 has upper 7 bits of w2 */
		w3 = w1 & LOW(9);	  /* w3 has low 9 bits of w3 */
		w5 = ((result.shorts.s3 = (w3 | w4) + w5) == 0)
			&& (w3 != 0);
		w4 = (w1 >> 9) & LOW(16); /* w4 has 16 bits of w1 */
		w5 = ((result.shorts.s2 = w4 + w5) == 0)
			&& (w2 != 0);	
		w4 = (w1 >> 25) & LOW(7); /* w4 has high 7 bits of w1 */
		result.shorts.s1 = ((sign << 31) | (t << 7)) + w4 + w5;
		if ((t == DMAXEXP - EXP_BIAS - 1) && (w5 != 0) &&
		    (w4 == 127)) {
			/* if overflow */
			errno = ERANGE;
			if (_lib_version == c_issue_4)
				return (sign ? -HUGE : HUGE);
			else
				return (sign ? -HUGE_VAL : HUGE_VAL);
		}
#endif
		if (denormexp == 0)  {
			/* if not denormalized number */
			return(result.dvalue);
		}  else  {
			/* value was a denormalized number */
			return ldexp(result.dvalue, denormexp);
		}
	}
}
