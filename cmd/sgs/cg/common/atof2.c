/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/atof2.c	1.1"
/* originally:  #ident	"@(#)stincc:m32/atof2.c	1.2" */
/*LINTLIBRARY*/
/*
 *	C library - ascii to floating (atof) and string to double (strtod)
 *
 *	This version compiles both atof and strtod depending on the value
 *	of STRTOD, which is set in the file and may be overridden on the
 *	"cc" command line.  The only difference is the storage of a pointer
 *	to the character which terminated the conversion.
 */

/* Compile this file only for Amdahls */

/* The odd conditions
**	1 || (u3b || M32 || M32B)
** and	0 && (vax)
** preserve the outline of previous code but force this
** version only to do IEEE-style conversions.
*/

#if defined(uts) && defined(FP_EMULATE)

#ifndef STRTOD
#define STRTOD	0
#endif
#include <ctype.h>
#include "values.h"
#include <stdio.h>
#include "mfile1.h"

extern double _ptod();

#if STRTOD
#define STORE_PTR	(*ptr = p)
#define GOT_DIGIT	(got_digit++)
#define RET_ZERO(val)	if (!got_digit) return (0.0)

double
strtod(p, ptr)
register char *p;
char **ptr;
#else
#define STORE_PTR
#define GOT_DIGIT
#define RET_ZERO(val)	if (!val) return (0.0)

double
atof2(p)
register char *p;
#endif
{
	register int c, exp = 0;
	long high, low = 0;
	int lowlen = 0;
	int neg_val = 0;
#if STRTOD
	int got_digit = 0;
	char *dummy;
	if (ptr == (char **)0)
		ptr = &dummy; /* harmless dumping place */
	STORE_PTR;
#endif
	while (isspace(c = *p)) /* eat leading white space */
		p++;
	switch (c) { /* process sign */
	case '-':
		neg_val = 1;
	case '+': /* fall-through */
		p++;
	}
	{	/* accumulate value */
		register long temphigh = 0;
		register int decpt = 0;

		while (isdigit(c = *p++) || c == '.' && !decpt++ &&
							isdigit(c = *p++)) {
			GOT_DIGIT;
			exp -= decpt; /* decr exponent if decimal pt. seen */
			c -= '0';
			if (temphigh < MAXLONG/10) {
				temphigh *= 10;
				temphigh += c;
			} else if (++lowlen <= 9) {
				low = 10*low + c;
			} else {
				exp++;
				lowlen--;
			}
		}
		RET_ZERO(temphigh);
		high = temphigh;
	}
	STORE_PTR; /* in case there is no legitimate exponent */
	if (c == 'E' || c == 'e') { /* accumulate exponent */
		register int e_exp = 0, neg_exp = 0;

		switch (*p) { /* process sign */
		case '-':
			neg_exp++;
		case '+': /* fall-through */
		case ' ': /* many FORTRAN environments generate this! */
			p++;
		}
		if (isdigit(c = *p)) { /* found a legitimate exponent */
			do {
				/* limit outrageously large exponents */
				if (e_exp < DMAXEXP)
					e_exp = 10 * e_exp + c - '0';
			} while (isdigit(c = *++p));
			if (neg_exp)
				exp -= e_exp;
			else
				exp += e_exp;
			STORE_PTR;
		}
	}

	return(_ptod(high, low, lowlen, exp, neg_val));
}
#ident	"@(#)libc-m32:gen/biglitpow.c	1.3"
/*
  *	arrays used by both _dtop.c and _ptod.c
  *     seperated out for space conservation.
  */
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
struct simex _bigpow[] = {
{ { 0x40c2, 0x05e5, 0x0f96, 0x19c6, 0x3af5},  1063 },
{ { 0x6913, 0x1f83, 0x0ae3, 0x145f, 0x063b},   956 },
{ { 0x553f, 0x1d7f, 0x1cef, 0x33bd, 0x06f0},   850 },
{ { 0x4529, 0x12df, 0x3f19, 0x3992, 0x17c3},   744 },
{ { 0x7038, 0x1ef1, 0x29c9, 0x0eaa, 0x3544},   637 },
{ { 0x5b0b, 0x1425, 0x1bff, 0x0c2f, 0x152d},   531 },
{ { 0x49dd, 0x08f9, 0x0c07, 0x1319, 0x2fe5},   425 },
{ { 0x77d9, 0x3562, 0x362c, 0x3629, 0x1164},   318 },
{ { 0x613c, 0x03e9, 0x0ffe, 0x1f4d, 0x2aa0},   212 },
{ { 0x4ee2, 0x35b5, 0x015b, 0x216b, 0x0efa},   106 },
{ { 0x4000,   0000,   0000,   0000,   0000},     0 },
{ { 0x67d8, 0x23d5, 0x2a29, 0x3329, 0x1d35},  -107 },
{ { 0x543f, 0x3d44, 0x3d29, 0x33d3, 0x12a1},  -213 },
{ { 0x445a, 0x005e, 0x3feb, 0x2aa7, 0x0d8a},  -319 },
{ { 0x6ee8, 0x08cf, 0x2325, 0x39c9, 0x1058},  -426 },
{ { 0x59fa, 0x1c12, 0x1edb, 0x2741, 0x09ae},  -532 },
{ { 0x48ff, 0x306e, 0x3aa1, 0x0798, 0x033f},  -638 },
{ { 0x7672, 0x279d, 0x2251, 0x229d, 0x1ed4},  -745 },
{ { 0x6018, 0x2864, 0x2b1b, 0x3432, 0x1cff},  -851 },
{ { 0x4df6, 0x19cc, 0x141b, 0x158a, 0x3ba9},  -957 },
{ { 0x7e80, 0x1712, 0x3a3c, 0x0471, 0x2846}, -1064 },
{ { 0x66a1, 0x1422, 0x1a37, 0x269e, 0x3e82}, -1170 },
};

/*
 * This table contains 1e16, 1e15, 1e14, etc., down to 1e-15.
 */

struct simex _litpow[] = {
{ { 0x470d, 0x3937, 0x3820,   0000,   0000},    53 },
{ { 0x71af, 0x3526, 0x0d00,   0000,   0000},    49 },
{ { 0x5af3, 0x041e, 0x2400,   0000,   0000},    46 },
{ { 0x48c2, 0x1ce5, 0x1000,   0000,   0000},    43 },
{ { 0x746a, 0x14a2,   0000,   0000,   0000},    39 },
{ { 0x5d21, 0x36e8,   0000,   0000,   0000},    36 },
{ { 0x4a81, 0x1f20,   0000,   0000,   0000},    33 },
{ { 0x7735, 0x2500,   0000,   0000,   0000},    29 },
{ { 0x5f5e, 0x0400,   0000,   0000,   0000},    26 },
{ { 0x4c4b, 0x1000,   0000,   0000,   0000},    23 },
{ { 0x7a12,   0000,   0000,   0000,   0000},    19 },
{ { 0x61a8,   0000,   0000,   0000,   0000},    16 },
{ { 0x4e20,   0000,   0000,   0000,   0000},    13 },
{ { 0x7d00,   0000,   0000,   0000,   0000},     9 },
{ { 0x6400,   0000,   0000,   0000,   0000},     6 },
{ { 0x5000,   0000,   0000,   0000,   0000},     3 },
{ { 0x4000,   0000,   0000,   0000,   0000},     0 },
{ { 0x6666, 0x1999, 0x2666, 0x1999, 0x2668},    -4 },
{ { 0x51eb, 0x2147, 0x2b85, 0x07ae, 0x0520},    -7 },
{ { 0x4189, 0x0dd2, 0x3c6a, 0x1fbe, 0x1db4},   -10 },
{ { 0x68db, 0x22eb, 0x0710, 0x32ca, 0x15eb},   -14 },
{ { 0x53e2, 0x3588, 0x38da, 0x0f08, 0x1189},   -17 },
{ { 0x431b, 0x37a0, 0x2d7b, 0x18d3, 0x1ad5},   -20 },
{ { 0x6b5f, 0x329a, 0x2f2b, 0x3485, 0x1e20},   -24 },
{ { 0x55e6, 0x0ee2, 0x0c23, 0x039d, 0x3e80},   -27 },
{ { 0x44b8, 0x0be8, 0x09b5, 0x294b, 0x0b9a},   -30 },
{ { 0x6df3, 0x1fd9, 0x35ef, 0x1bab, 0x1f5c},   -34 },
{ { 0x57f5, 0x3fe1, 0x1e59, 0x0955, 0x3f7d},   -37 },
{ { 0x465e, 0x1981, 0x0b7a, 0x2111, 0x25fe},   -40 },
{ { 0x7097, 0x0268, 0x125d, 0x281c, 0x0996},   -44 },
{ { 0x5a12, 0x1b86, 0x284a, 0x39b0, 0x07ab},   -47 },
{ { 0x480e, 0x2f9e, 0x39d5, 0x2159, 0x2c89},   -50 },
};
#ident	"@(#)libc-m32:gen/dtop.c	1.4"
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

#include <errno.h>
#include <math.h>
#include "values.h"

/*	macros and constants describing internal format	*/
#define WSIZE 14
#define WMASK LOW(WSIZE)
/*	general purpose macros and constants	*/
#define LOW(x) ((1 << (x)) - 1)
#if 1 || (u3b || M32 || M32B)
#define EXP_BIAS	1024
#define EXP_SIZE	11	/* size of exponent	*/
#endif
#if 0 && (vax)
#define EXP_BIAS	128
#define EXP_SIZE	8	/* size of exponent	*/
#endif

extern double frexp2(), ldexp2();

/* Format of a simulated extended precision power of ten.  These 
 * numbers contain 71 bits; the first 70 bits are obtained by taking
 * the ceiling at the 70th bit.  The idea is roughly to compensate
 * for truncation errors during multiplication.  The 71st bit is whatever
 * actually belongs there.  For obtaining 64 bits of accuracy, exactly
 * how the 71st bit is handled is not critical.
 *
 * Each number comes with a binary exponent.
 */


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
/* IEEE-style only */
#if 1 || (u3b || M32 || M32B)
		*sign = ((w1 = dp->lg.l1) >> 31) & 1;
		binexp = ((w1 >> 20) & LOW(EXP_SIZE)) - EXP_BIAS + 2;
#endif
#if 0 && (vax)
		*sign  = ((w1 = dp->sh.s1) >> 15) & 1;
		binexp = ((w1 >> 7) & LOW(EXP_SIZE)) - EXP_BIAS + 2; 
#endif
		if (binexp == -EXP_BIAS + 2)  {
			/* handle denormalized number */
			dp->dbl = frexp2(dp->dbl, &binexp);
		}
		
#if 1 || (u3b || M32 || M32B)
	/* initialize w1-w3 */
		w2 = WMASK;
		w4 = ((w1 = dp->lg.l2) << 3) & w2;
		w1 >>= WSIZE - 3;
		w3 = w1 & w2;
		w1 >>= WSIZE;
		w1 &= LOW(7);
		w1 += dp->lg.l1 << 7;
		w2 &= w1;
		w1 >>= WSIZE;
		w1 &= LOW(13);
		w1 |= 1 << 13;
#endif
#if 0 && (vax)
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
			w2 = w3; w3 = w4;
		}

		if (prec <= 0)
			return(retval);

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

#ident	"@(#)libc-m32:gen/ptod.c	1.5"
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

#include <errno.h>
#include <math.h>
#include "values.h"

/*	macros and constants describing internal format	*/
#define WSIZE 14
#define WMASK LOW(WSIZE)
/*	general purpose macros and constants	*/
#define LOW(x) ((1 << (x)) - 1)
#if 1 || (u3b || M32 || M32B)
#define EXP_BIAS	1024
#define EXP_SIZE	11	/* size of exponent	*/
#endif
#if 0 && (vax)
#define EXP_BIAS	128
#define EXP_SIZE	8	/* size of exponent	*/
#endif

extern double frexp2(), ldexp2();

/* Format of a simulated extended precision power of ten.  These 
 * numbers contain 71 bits; the first 70 bits are obtained by taking
 * the ceiling at the 70th bit.  The idea is roughly to compensate
 * for truncation errors during multiplication.  The 71st bit is whatever
 * actually belongs there.  For obtaining 64 bits of accuracy, exactly
 * how the 71st bit is handled is not critical.
 *
 * Each number comes with a binary exponent.
 */


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
		return (HUGE);
	} else if (dec_exp < -350)
		return(0.0);

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
			return(HUGE);
		}
		/* The following code does the jamming.  Note that
		   we only need worry about rounding overflow if
		   the number becomes to large to represent.
		   Otherwise, the mantissa becomes 0, and instead
		   of the single hidden bit being added to the 
		   exponent, 2 will be added simply by the overflow
		   into the exponent portion of the double!
		*/
#if 1 || (u3b || M32 || M32B)

		result.longs.upper = (sign << 31) + (t << 20) +
					(w1 >> 9);
		result.longs.lower = t = ((w1 & LOW(9)) << 23) +
				(w2 << 9) + ((w3 + 16) >> 5);
		/* test for rounding overflow (see if exp -> neg val) */
		if ((t == 0) && (w2 != 0) &&
		    ((++result.longs.upper >> 20) & LOW(11)) < 0) {
			errno = ERANGE;
			return(HUGE); 
		}
#endif
#if 0 && (vax)
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
			return(HUGE); 
		}
#endif
		if (denormexp == 0)  {
			/* if not denormalized number */
			return(result.dvalue);
		}  else  {
			/* value was a denormalized number */
			return ldexp2(result.dvalue, denormexp);
		}
	}
}

/*
#	Ldexp returns value * 2**exp, if that result is in range.
#	If underflow occurs, it returns zero.  If overflow occurs,
#	it returns a value of appropriate sign and largest possible single-
#	precision magnitude.  In case of underflow or overflow,
#	the external int "errno" is set to ERANGE. If value is
#	NAN or infinity errno is set to EDOM. Note that errno is
#	not modified if no error occurs, so if you intend to test it
#	after you use ldexp, you had better set it to something
#	other than ERANGE first (zero is a reasonable value to use).
#-----------------------------------------------------------------------------
*/

#define	EXP_BITS	0x7ff00000
#define SIGN_MASK	0x7fffffff
#define VALMASK		0x000fffff
#define TWOTO52		0x43300000
#define DEXP_OFFSET	20
#define MAX_EXP		0x7ff
#define ERANGE		34
#define EDOM		33
#define LASTBIT		0x00000001
#define FIRSTBIT	0x80000000

extern	int errno;

union fltrep {
	double dval;
	struct brep {
		unsigned long bsign:1;
		unsigned long bexp:11;
		unsigned long bdmod1:20;
		unsigned long bdmod2:32;
	} bval;
	struct {
		unsigned long word1, word2;
	} lg;
};


double ldexp2(val,exp)
double val;
int exp;
{

	register  long bias_exp;
	int	sign;
	union fltrep tempv, value;

	value.dval = val;
	if (value.dval == 0.0)
		return(0.0);

	if(exp == 0) return(value.dval);

	bias_exp = value.bval.bexp;
	if(bias_exp == MAX_EXP){
		errno = EDOM;
		return(value.dval);
	}

	if(!bias_exp){
		tempv.lg.word1 = TWOTO52;
		tempv.lg.word2 = 0;
		value.dval = FP_TIMES(value.dval,tempv.dval);
		bias_exp = value.bval.bexp - 52;
	}

	bias_exp += exp;

	if(bias_exp <= 0){
		if(bias_exp < -52){
			errno = ERANGE;
			return(0.0);
		}
		value.bval.bexp = 1;
		bias_exp += 1022;
		bias_exp <<= DEXP_OFFSET;
		tempv.lg.word1 = bias_exp;
		tempv.lg.word2 = 0;
		value.dval = FP_TIMES(value.dval,tempv.dval);
		return(value.dval);
	}
	
	if(bias_exp >= MAX_EXP){
		if(value.dval < 0)
			errno = ERANGE;
		return(value.dval < 0 ? -MAXFLOAT : MAXFLOAT);
	}
	
	value.bval.bexp = bias_exp;
	return(value.dval);
}

/*
#------------------------------------------------------------------------------
#	double frexp (value, eptr)
#		double value;
#		int *eptr;
#
#	Frexp breaks "value" up into a fraction and an exponent.
#	It stores the exponent indirectly through eptr, and
#	returns the fraction.  More specifically, after
#
#		double d, frexp();
#		int e;
#		d = frexp (x, &e);
#
#	then |d| will be less than 1, and x will be equal to d*(2**e).
#	Further, if x is not zero, d will be no less than 1/2, and if
#	x is zero, both d and e will be zero too.

# NOTE: For infinity and NAN, it returns the same number, and sets
#	e to 0x7ff, which is the reserved exponent.
#	IT DOES NOT TRAP ON Trapping NANs.
#------------------------------------------------------------------------------
*/


#define MAX_EXP		0x7ff
#define DEXP_BIAS	1023
#define IMPLIED_BIT	0x100000
#define NORM		0x00080000


double frexp2(val,eptr)
double val;
int *eptr;
{

	register unsigned long exp, sign, temp;
	union fltrep value;
	value.dval = val;
	if (value.dval == 0.0)
		return(0.0);

	exp = value.bval.bexp;
	if(exp == MAX_EXP){
		*eptr = MAX_EXP;
		return(value.dval);
	}

	if(exp == 0){
		sign = value.bval.bsign;
		exp++;
		while(!(IMPLIED_BIT & value.lg.word1)){
			temp = value.lg.word2 >> 31;
			value.lg.word1 <<= 1;
			value.lg.word1 |= temp;
			value.lg.word2 <<= 1;
			exp--;
		}
		value.bval.bsign = sign;
	}

	value.bval.bexp = DEXP_BIAS-1;
	*eptr = -1*((DEXP_BIAS-1) - exp);
	return(value.dval);
}

#endif	/* defined(uts) && defined(FP_EMULATE) */
