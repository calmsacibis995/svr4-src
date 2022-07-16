/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:gen/dtop.c	1.4"
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
 * 	_dtop(dptr, scale, prec, frac_ptr, sign)
 *
 *	Convert a scaled double to a pair of longs.
 *
 *	Given a pointer dptr to a double precision floating value x,
 *	and an integer "scale", this function returns the integer part
 *	of  abs(x) * 10**scale.  Further, if "prec" is positive, _dtop
 *	computes y = the fractional part of (abs(x) * 10**scale), and
 *	stores the integer part of (y * 10**prec) in the value pointed
 *	to by "frac_ptr".  The sign of x (0 or 1) is stored in *sign.
 *
 *	It is the responsibility of the calling program, which is cvt,
 *	to choose "scale" in such a way that the integer part of
 *	abs(x) * 10**scale fits in a long.  Moreover, if "prec" is positive,
 *	_dtop assumes that "scale" has been chosen to make the return
 *	value of _dtop as large as possible (i.e. 8 to 10 decimal digits).
 */

long
_dtop(dptr, scale, prec, frac_ptr, sign)
long *dptr;
int scale, prec, *sign;
long *frac_ptr;
{
	register long w1, w2, w3, w4;
	long w5;
	long retval;
	int binexp;

	/* Put the given double precision value into the internal format */

	{	
		register union types {
			double dbl;
			struct {
				unsigned long l1, l2;
			} lg;
			struct {
				unsigned short s1, s2, s3, s4;
			} sh;
		} *dp;

		dp = (union types *)dptr;
#if (i386)
		*sign = ((w1 = dp->lg.l2) >> 31) & 1;
		binexp = ((w1 >> 20) & LOW(EXP_SIZE)) - EXP_BIAS + 2;
#endif
#if (vax)
		*sign  = ((w1 = dp->sh.s1) >> 15) & 1;
		binexp = ((w1 >> 7) & LOW(EXP_SIZE)) - EXP_BIAS + 2; 
#endif
		if (binexp == -EXP_BIAS + 2)  {
			/* handle denormalized number */
			dp->dbl = frexp(dp->dbl, &binexp);
		}
		
#if (i386)
	/* initialize w1-w3 */
		w2 = WMASK;
		w4 = ((w1 = dp->lg.l1) << 3) & w2;
		w1 >>= WSIZE - 3;
		w3 = w1 & w2;
		w1 >>= WSIZE;
		w1 &= LOW(7);
		w1 += dp->lg.l2 << 7;
		w2 &= w1;
		w1 >>= WSIZE;
		w1 &= LOW(13);
		w1 |= 1 << 13;
#endif
#if (vax)
	/* initialize w1-w3 */
		/* init w4 */
		w4 = dp->sh.s4 & WMASK;
		/* init w3 */
		w1 = (dp->sh.s4 >> WSIZE) & LOW(2);
		w3 = ((LOW(WSIZE - 2) & dp->sh.s3) << 2) | w1;
		/* init w2 */
		w1 = (dp->sh.s3 >> WSIZE - 2) & LOW(4);
		w2 = ((LOW(WSIZE - 4) & dp->sh.s2) << 4) | w1;
		/* init w1 */
		w1 = (dp->sh.s2 >> WSIZE - 4) & LOW(6);
		w1 = ((LOW(WSIZE - 6 - 1) & dp->sh.s1) << 6) | w1;
		/* add hidden bit */
		w1 |= 1 << 13;
#endif
	}

	/*
	 * Multiply x by 10**scale in two stages.  First multiply
	 * by a power of 1e32, then multiply by a power of ten in
	 * the range 1e-15 to 1e16.  In the following, we multiply
	 * by one of the big powers.  The result of the multiplication
	 * leaves the binary point preceding 28 bits of w1.
	 */

	if ((16 - scale) >> 5 != 0) {
		register short *m;

		m = _bigpow[(336 - scale) >> 5].signif;
		w4 = w4*m[0] + w3*m[1] + w2*m[2] + w1*m[3] +
		     ((w4*m[1] + w3*m[2] + w2*m[3] + w1*m[4]) >> WSIZE);
		w3 = w3*m[0] + w2*m[1] + w1*m[2] + (w4 >> WSIZE);
		w2 = w2*m[0] + w1*m[1] + (w3 >> WSIZE);
		w1 = w1*m[0] + (w2 >> WSIZE);
		binexp += ((struct simex *) m)->expo;
	} else {
		w1 <<= WSIZE; w1 += w2;
		w2 = w3; w3 = w4; w4 = 0;
	}

	/*
	 * Now multiply (if necessary) by a power from 1e-15 to 1e16.
	 * But first reorganize the number so it can be multiplied.
	 */

	if ((scale & 31) != 0) {
		{	register int t = WMASK;
			w5 = w4 & t; w4 = w3 & t; w3 = w2 & t; w2 = w1 & t;
			w1 >>= WSIZE;
		}

		{	register short *m;

			m = _litpow[(16 - scale) & 31].signif;
			w4 = w4*m[0] + w3*m[1] + w2*m[2] + w1*m[3] +
				((w5*m[0] + w4*m[1] + w3*m[2] + w2*m[3] 
				+ w1*m[4]) >> WSIZE);
			w3 = w3*m[0] + w2*m[1] + w1*m[2] + (w4 >> WSIZE);
			w2 = w2*m[0] + w1*m[1] + (w3 >> WSIZE);
			w1 = w1*m[0] + (w2 >> WSIZE);
			binexp += ((struct simex *) m)->expo;
		}
	}

	/*
	 * The implied binary point now lies before 28 bits of w1.  Use
	 * this fact and the value of the binary exponent to extract the
	 * integer part of the product and put it in retval.
	 * For example, if "binexp" equals 28, then we must extract
	 * everything up to 28 bits after the binary point, in other
	 * words, we must extract exactly what is in w1.  If more than
	 * 28 bits are required, we must dip into w2.
	 * After extracting the integer part we delete it and leave the
	 * fractional part--in case digits from behind the binary point
	 * are desired.
	 */

	{	register int t = WMASK;

		w2 &= t; w3 &= t; w4 &= t; /* delete left-over garbage */

		/* In the following, t will be the number of bits by which
		 * the resultant fraction must be shifted left in order
		 * to normalize it.
		 */

		if ((t = binexp - 2*WSIZE) <= 0) {
			retval = w1 >> -t;	/* extract int. part */
			w1 &= (1 << -t) - 1;	/* mask off int. part */
			t += WSIZE;
		} else {
			retval = (w1 << t) + ((w2 << t) >> WSIZE);
			w1 = w2 & (WMASK >> t);
			w2 = w3; w3 = w4; w4 = 0;
		}

           	/* for the accuracy of rounding result, if the last digit of 
		 * retval is 5, and the remaining fraction part is nonzero, 
		 * do the adjustment - change the last digit to 6 
		 */
			       
		if (prec <= 0)
		{
			if(retval%10 == 5 && (w1 || w2 || w3 || w4))  
				retval++;
			return(retval);
		}

		/* Normalize! */

		w1 <<= t; w2 <<= t; w3 <<= t; w4 <<= t;
		w1 += w2 >> WSIZE; w2 += w3 >> WSIZE; w3 += w4 >> WSIZE;
		t = WMASK;
		w1 &= t; w2 &= t; w3 &= t;
	}

	/*
	 * We now have a normalized fraction which is to be multiplied by
	 * the power 10**prec.  Three words (42 bits) of precision are
	 * adequate, since only up to 30 bits of the result will be used.
	 */

	{	register short *m;

		m = _litpow[16 - prec].signif;
		w2 = w2*m[0] + w1*m[1] + ((w3*m[0] + w2*m[1]) >> WSIZE);
		w1 = w1*m[0] + (w2 >> WSIZE);
		w4 = ((struct simex *) m)->expo;
	}

	/*
	 * As when extracting the integer part of the scaled number above,
	 * we again extract the integer part of the result of multiplying
	 * the fraction by 10**prec.  Here w4 is the binary exponent of
	 * 10**prec.
	 */

	{	register int t;

		if ((t = w4 - 2*WSIZE) <= 0)
			*frac_ptr = w1 >> -t;
		else
			*frac_ptr = (w1 << t) +
				(((w2 & WMASK) << t) >> WSIZE);
	}

	return(retval);
}

