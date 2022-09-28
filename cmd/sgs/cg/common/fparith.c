/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/fparith.c	1.8"
/* originally:  #ident	"@(#)stincc:m32/fparith.c	1.3" */


/*  The following routines are floating point emulation routines performed  */
/*  on two IEEE double precision numbers.   These routines behave according */
/*  to the IEEE standard for floating point numbers.			    */

/*  This file required only by Amdahl UTS */

#if defined(uts) && defined(FP_EMULATE)

#include "fp.h"
#include <stdio.h>
#include <errno.h>


/* add two floating-point numbers */

double
fladd (arg1,arg2)
double arg1, arg2;
{
	fp x, y;
	double fp_addmag(), fp_submag();

	x.dval = arg1;
	y.dval = arg2;

	/* extract special cases */
	switch(dspecial(ADD,x.dval,y.dval)){
	case NONE:
		break;
	case ARG1:
		return(x.dval);
	case ARG2:
		return(y.dval);
	case NAN:
		x.ll = NOTANUMBER;
		return(x.dval);
	case DZERO:
		return(0.0);
	}

	if (x.bval.sign == y.bval.sign)
		return(fp_addmag(x.dval, y.dval));
	else
		return(fp_submag(x.dval, y.dval));
}


/*
 *	add magnitudes of values addressed by x and y.
 *	sign of y is ignored, result sign = sign of x.
 */


double
fp_addmag (arg1, arg2)
double arg1, arg2;
{
	fp result, x, y;
	register int lexp, sexp;
	fp lfrac, sfrac, rfrac;
	register int shift, sign, rs, norm=0;

	x.dval = arg1;
	y.dval = arg2;
	result.dval = 0;
	sign = x.bval.sign;

	/*
	 *	put large exponent, fraction into lexp, lfrac
	 *	and small exponent, fraction into sexp, sfrac
	 */

	if (x.bval.exp >= y.bval.exp) {
		lexp = x.bval.exp;
		lfrac.ll = x.ll & VALMASK;
		sexp = y.bval.exp;
		sfrac.ll = y.ll & VALMASK;
	} else {
		lexp = y.bval.exp;
		lfrac.ll = y.ll & VALMASK;
		sexp = x.bval.exp;
		sfrac.ll = x.ll & VALMASK;
	}

	/* normalize unnormalized numbers */
	if((lexp == 0) && (lfrac.ll & VALMASK))
		while ((HIDDENBIT & lfrac.ll) == 0){
			lexp--;
			lfrac.ll <<= 1;
		}
	if((sexp == 0) && (sfrac.ll & VALMASK))
		while ((HIDDENBIT & sfrac.ll) == 0){
			sexp--;
			sfrac.ll <<= 1;
		}

	/* install hidden bit in both fractions and create guard bits */
	lfrac.ll = (lfrac.ll | HIDDENBIT) << GBITS;
	sfrac.ll = (sfrac.ll | HIDDENBIT) << GBITS;

	/* difference between exponents is how many bits to shift */
	shift = lexp - sexp;

	/*
	 *	if the larger is way bigger
	 *	return the larger operand
	 */
	if (shift > FRACSIZE + 2) {
		result.ll = (lfrac.ll >> GBITS) & ~HIDDENBIT;
		result.bval.exp = lexp;
		result.bval.sign = sign;
		return(result.dval);
	}

	/* the actual addition */
	rfrac.ll = lfrac.ll + (sfrac.ll >> shift);

	/* check for a carry during addition */
	if (rfrac.ll & CARRYBIT){
		lexp++;
		rfrac.ll >>= 1;
	}

	/* rounding */
	rs = (0x6 & rfrac.ll) >> 1;	/* get round and sticky bits */
	if(rfrac.ll & RSBITS)		/* set sticky */
		rs |= 1;
	rfrac.ll = dround(&lexp,rfrac.ll >> GBITS, rs);

	/* check for underflow */
	if (lexp <= 0){
		if(lexp < -52){
			errno = ERANGE;
			return(0.0);
		}
		while(lexp++ < 0)
			rfrac.ll >>= 1;
		lexp = 0;
	}
			
	/* overflow check */
	if (lexp >= MAXEXP) {
		lexp = ~0;
		rfrac.ll = 0;
		errno = ERANGE;
	}

	/* store final result */
	if((rfrac.ll & HIDDENBIT) && (lexp == 0)) lexp++;
	result.ll = rfrac.ll;
	result.bval.exp = lexp;
	result.bval.sign = sign;
	return(result.dval);
}


/*
 *	subtract magnitude of y from magnitude of x.
 *	result has sign of x if |y| <= |x|, opposite
 *	sign otherwise.  sign of y is ignored.
 */

double
fp_submag (arg1,arg2)
double arg1, arg2;
{
	fp result, x, y;
	register int lexp, sexp;
	fp lfrac, sfrac, rfrac, t;
	register int shift, sign, rs, norm = 0;

	x.dval = arg1;
	y.dval = arg2;
	result.dval = 0;
	sign = x.bval.sign;
	/*
	 *	put exponent, fraction of large operand into lexp, lfrac
	 *	and exponent, fraction of small operand into sexp, sfrac
	 *	flip result sign if necessary
	 */
	if (x.bval.exp > y.bval.exp || x.bval.exp == y.bval.exp && (x.ll & VALMASK) >= (y.ll & VALMASK)) {
		lexp = x.bval.exp;
		lfrac.ll = x.ll & VALMASK;
		sexp = y.bval.exp;
		sfrac.ll = y.ll & VALMASK;
	} else {
		lexp = y.bval.exp;
		lfrac.ll = y.ll & VALMASK;
		sexp = x.bval.exp;
		sfrac.ll = x.ll & VALMASK;
		sign ^= 1;
	}

	/* normalize unnormalized numbers, set norm flag */
	if((lexp == 0) && (lfrac.ll & VALMASK)){
		norm++;
		while ((HIDDENBIT & lfrac.ll) == 0){
			lexp--;
			lfrac.ll <<= 1;
		}
	}
	if((sexp == 0) && (sfrac.ll & VALMASK)){
		norm++;
		while ((HIDDENBIT & sfrac.ll) == 0){
			sexp--;
			sfrac.ll <<= 1;
		}
	}

	/* install hidden bit in both fractions and create guard bits */
	lfrac.ll = (lfrac.ll | HIDDENBIT) << GBITS;
	sfrac.ll = (sfrac.ll | HIDDENBIT) << GBITS;

	/* difference between exponents is how many bits to shift */
	shift = lexp - sexp;
	/*
	 *	if exponent difference is too large,
	 *	return the larger operand
	 */
	if (shift > FRACSIZE + 2 ) {
		result.ll = (lfrac.ll >> GBITS) & ~HIDDENBIT;
		result.bval.exp = lexp;
		result.bval.sign = sign;
		return(result.dval);
	}

	/* the actual subtraction, with sticky right shift */
	t.ll = sfrac.ll >> shift;
	rfrac.ll = lfrac.ll - t.ll;
	if (sfrac.ll != t.ll << shift)
		rfrac.ll--; 

	/* if zero return true zero */
	if (rfrac.ll == 0)
		return(0.0);
	
	/* result nonzero, normalize */
	while ((rfrac.ll & NORMMASK) == 0){
		lexp--;
		rfrac.ll <<= 1;
	}
	/* round */
	rs = (rfrac.ll & (RSBITS << 1)) >> 1;	/* get round and sticky bits */
	if(rfrac.ll & RSBITS)		/* set sticky */
		rs |= 1;
	rfrac.ll = dround(&lexp,rfrac.ll >> GBITS,rs);
	
	/* check for underflow */
	if (lexp <= 0){
		if(norm) lexp++;
		if(lexp < -52){
			errno = ERANGE;
			return(0.0);
		}
		while(lexp <= 0){
			lexp++;
			rfrac.ll >>= 1;
		}
		lexp = 0;
	}
			
	/* store final result */
	result.ll = rfrac.ll & ~HIDDENBIT;
	result.bval.exp = lexp;
	result.bval.sign = sign;
	return(result.dval);
}



/*
 *	floating-point multiply
 */

double
flmul (arg1, arg2)
double arg1, arg2;
{
	fp result, x, y;
	int exp, xexp, yexp, sign, rs, round, sticky=0, norm=0;
	fp zh, zm, zl, xfrac, yfrac;
	long xh, xl, yh, yl;

	x.dval = arg1;
	y.dval = arg2;
	sign = x.bval.sign ^ y.bval.sign;

	/* extract special cases */
	switch(dspecial(MUL,x.dval,y.dval)){
	case NONE:
		break;
	case ARG1:
		return(x.dval);
	case ARG2:
		return(y.dval);
	case NAN:
		x.ll = NOTANUMBER;
		return(x.dval);
	case NEGNAN:
		x.ll = NEGNOTANUMBER;
		return(x.dval);
	case DZERO:
		return(0.0);
	case DNEGZERO:
		x.ll = NEGZERO;
		return(x.dval);
	}

	/* if either operand is infinity, return infinity */
	if (x.bval.exp == INFEXP || y.bval.exp == INFEXP) {
		result.ll = INFINITY;
		result.bval.sign = sign;
		errno = EDOM;
		return(result.dval);
	}
	
	/* if either operand is zero, return zero */
	if(((x.ll & ~SIGNMASK) == 0) || ((y.ll & ~SIGNMASK) == 0)){
		result.ll = 0;
		result.bval.sign = sign;
		return(result.dval); 
	}

	xfrac.ll = x.ll & VALMASK;
	yfrac.ll = y.ll & VALMASK;
	xexp = x.bval.exp;
	yexp = y.bval.exp;

	/* normalize denormal numbers */
	if((xexp == 0) && (xfrac.ll & VALMASK)){
		norm++;
		while ((HIDDENBIT & xfrac.ll) == 0){
			xexp--;
			xfrac.ll <<= 1;
		}
	}
	if((yexp == 0) && (yfrac.ll & VALMASK)){
		norm++;
		while ((HIDDENBIT & yfrac.ll) == 0){
			yexp--;
			yfrac.ll <<= 1;
		}
	}

	/* install hidden bit in both fractions and create guard bits */
	xfrac.ll = xfrac.ll | HIDDENBIT;
	yfrac.ll = yfrac.ll | HIDDENBIT;

	/* calculate result exponent */
	exp = xexp + yexp - EXPOFFSET;

	/* split the fractions into pieces */
	xh = hi (xfrac.ll);
	xl = lo (xfrac.ll);
	yh = hi (yfrac.ll);
	yl = lo (yfrac.ll);

	/* multiply the pieces, but don't merge carries yet */
	zh.ll = lmul (xh, yh);
	zm.ll = lmul (xh, yl) + lmul (xl, yh);
	zl.ll = lmul (xl, yl);

	/* propagate carries */
	zm.ll += hi (zl.ll);
	zh.ll += hi (zm.ll);

	/* check for overflow during carry propagation */
	if ( zh.lg.word1 & 0x2000000){
		exp++;
		zm.ll >>= 1;
		zm.ll = zm.ll | lowbit(zh.ll);
		zh.ll >>= 1;
	}
	if(zm.ll || zl.ll || (0x7 & zh.ll))	   /* set sticky */
		sticky = 1;
	zh.ll >>= 2;
	round = RSBITS & zh.ll;
	rs = round | sticky;
	zh.ll >>= 2;		/* 52 bits of fraction in zh.ll */

	/* normalize if necessary */
	if(zh.ll & VALMASK){
		while ((zh.ll & HIDDENBIT) == 0) {
			zh.ll = (zh.ll << 1) | hibit (zm.ll);
			zm.ll <<= 1;
			exp--;
		}
	}
	/* underflow? */
	if (exp < 1){
		if(norm) exp++;
		if(exp < -FRACSIZE){
			errno = ERANGE;
			result.ll = 0;
			result.bval.sign = sign;
			return(result.dval);
		}
		while(exp++ <= 0){
			sticky = round;
			round = (zh.ll << 63) >> 63;
			zh.ll >>= 1;
		}
		rs = (round << 1) | sticky;
		exp = 0;
	}
	/* round */
	zh.ll = dround(&exp,zh.ll,rs);

	/* store result or overflow indication */
	if (exp >= MAXEXP){
		errno = ERANGE;
		result.ll = INFINITY;
	}
	else {
		if((!exp) && (zh.ll & HIDDENBIT)) exp++;
		result.ll = zh.ll & ~HIDDENBIT;
		result.bval.exp = exp;
	}

	/* result sign */
	result.bval.sign = sign;
	return(result.dval);
}



/*
 *	floating-point divide
 */

double
fldiv (arg1, arg2)
double arg1, arg2;
{
	fp result, x, y;
	register int exp, xexp, yexp, norm=0, round=0, rs=0, sticky=0;
	fp z, q, r, xfrac, yfrac;

	x.dval = arg1;
	y.dval = arg2;

	/* extract special cases */
	switch(dspecial(DIV,x.dval,y.dval)){
	case NONE:
		break;
	case NAN:
		x.ll = NOTANUMBER;
		return(x.dval);
	case NEGNAN:
		x.ll = NEGNOTANUMBER;
		return(x.dval);
	case DZERO:
		return(0.0);
	case DNEGZERO:
		x.ll = NEGZERO;
		return(x.dval);
	case INF:
		x.ll = INFINITY;
		return(x.dval);
	case NEGINF:
		x.ll = NEGINFINITY;
		return(x.dval);
		break;
	}


	/* if dividend is infinity, return infinity */
	if (x.bval.exp == INFEXP) {
		result.dval = x.dval;
		result.bval.sign ^= y.bval.sign;
		return(result.dval);
	}
	xexp = x.bval.exp;
	yexp = y.bval.exp;
	xfrac.ll = x.ll & VALMASK;
	yfrac.ll = y.ll & VALMASK;

	/* normalize denormal numbers */
	if((xexp == 0) && (xfrac.ll & VALMASK)){
		norm++;
		while ((HIDDENBIT & xfrac.ll) == 0){
			xexp--;
			xfrac.ll <<= 1;
		}
	}
	if((yexp == 0) && (yfrac.ll & VALMASK)){
		norm++;
		while ((HIDDENBIT & yfrac.ll) == 0){
			yexp--;
			yfrac.ll <<= 1;
		}
	}


	/* install hidden bit in both fractions and create guard bits */
	xfrac.ll = xfrac.ll | HIDDENBIT;
	yfrac.ll = yfrac.ll | HIDDENBIT;
	
	/* calculate result exponent */
	exp = xexp - yexp + EXPOFFSET + FRACSIZE + 1;


	/* divide, by repeated subtraction (ugh) */
	q.ll = 0;
	do {
		q.ll <<= 1;
		if (xfrac.ll >= yfrac.ll) {
			q.ll++;
			xfrac.ll -= yfrac.ll;
		}
		xfrac.ll <<= 1;
		exp--;
	} while ((q.ll & HIDDENBIT) == 0);

	/* round, perhaps renormalize */
	if (xfrac.ll >= yfrac.ll){
		round++;
		q.ll++;
		if((q.ll & HIDDENBIT) == 0) {
			q.ll >>= 1;
			exp++;
		}
	}

	/* underflow? */
	if (exp < 1) {
		if(norm) exp++;
		if(exp < -FRACSIZE){
			errno = ERANGE;
			result.ll = 0;
			result.bval.sign = x.bval.sign ^ y.bval.sign;
			return(result.dval);
		}
		while(exp++ <= 0){
			sticky = round;
			round = (q.ll << 63) >> 62;
			rs = round | sticky;
			q.ll >>= 1;
		}
		exp = 0;
	}

	q.ll = dround(&exp,q.ll,rs);

	/* store result or overflow indication */
	if (exp > MAXEXP){
		errno = ERANGE;
		result.ll = INFINITY;
	}
	else {
		if((q.ll & HIDDENBIT) && norm) exp++;
		if(!exp && (q.ll & ~VALMASK))
				exp++;
		result.ll = q.ll & ~HIDDENBIT;
		result.bval.exp = exp;
	}

	/* result sign */
	result.bval.sign = x.bval.sign ^ y.bval.sign;
	return(result.dval);
}


/*
 *	signed long to double
 */

double
ltod (x)
long x;
{
	fp result;
	double ultod();

	if (x >= 0)
		return ultod ((unsigned) x);
	
	result.dval = ultod ((unsigned) -x);
	result.bval.sign = 1;

	return(result.dval);
}

/*
 *	unsigned long to double 
 */


double
ultod (x)
unsigned long x;
{
	fp result, frac;
	register int exp;
	
	/* converting zero? */
	if (x == 0)
		return(0.0);

	/* create an unnormalized fraction and exponent */
	frac.ll = x;
	exp = EXPOFFSET + FRACSIZE;

	/* normalize */
	while ((frac.ll & HIDDENBIT) == 0) {
		frac.ll <<= 1;
		exp--;
	}

	/* store the result */
	result.ll = frac.ll & ~HIDDENBIT;
	result.bval.sign = 0;
	result.bval.exp = exp;
	return(result.dval);
}


/*
 *	floating-point to unsigned long conversion
 */


unsigned long
dtoul (val)
double val;
{
	register int exp, shift;
	fp x, ll;
	unsigned long l;

	x.dval = val;
	/* extract exponent, test for zero */
	exp = x.bval.exp;
	if (exp == 0)
		return 0L;
	
	/* extract fraction, restore hidden bit */
	ll.ll = (x.ll & VALMASK) | HIDDENBIT;

	/* calculate how far to shift */
	shift = exp - EXPOFFSET - FRACSIZE;

	/* Check for value too big according to shift count. */
	if (shift >= 0) {
	    /* With the hidden bit in place, this value will never
	    ** fit in an unsigned long.
	    */
	    errno = ERANGE;
	    return( T_ULONG_MAX );
	}
	ll.ll >>= -shift;

	if (ll.ll > T_ULONG_MAX){
		errno = ERANGE;
		return T_ULONG_MAX;
	}

	l = ll.lg.word2;
	return(l);
}

/*
 *	floating-point to long conversion
 */

long
dtol (val)
double val;
{
	register int exp, shift;
	fp x, ll;
	long l;

	x.dval = val;
	/* extract exponent, test for zero */
	exp = x.bval.exp;
	if (exp == 0)
		return 0L;
	
	/* extract fraction, restore hidden bit */
	ll.ll = (x.ll & VALMASK) | HIDDENBIT;

	/* calculate how far to shift */
	shift = exp - EXPOFFSET - FRACSIZE;

	/* Check for value too big according to shift count. */
	if (shift >= 0) {
	    /* With the hidden bit in place, this value will never
	    ** fit in a long.
	    */
	    errno = ERANGE;
	    return( x.bval.sign ? T_LONG_MIN : T_LONG_MAX );
	}
	ll.ll >>= -shift;

	/* compensate for sign */
	if (x.bval.sign){
		ll.ll = -ll.ll;
		if(ll.ll < T_LONG_MIN){
			errno = ERANGE;
			return T_LONG_MIN;
		}
	}
	else if (ll.ll > T_LONG_MAX){
		errno = ERANGE;
		return T_LONG_MAX;
	}

	l = ll.lg.word2;

	return(l);
}

/*  dround rounds double precision floating point numbers to the nearest. */

double
dround(exp, mant, rs)
int *exp, rs;
unsigned long long mant;
{
	register int norm=0;

	if(exp)
		norm++;

	if((R_BIT & rs) && ((S_BIT & rs) || (L_BIT & mant))){
		mant++;
		if((mant & V_BIT) && norm){
			mant >>= 1;
			(*exp)++;
		}
	}
	return(mant);
}

/* dtof converts a IEEE double to an IEEE single floating point */

long				/* type pun for float */
dtof(val)
double val;
{
	fp value;
	unsigned long mant;
	register exp, rs, sign;

	value.dval = val;
	exp = value.bval.exp;
	sign = value.bval.sign;
	value.ll = (value.ll & VALMASK) | HIDDENBIT;
	exp -= DEXP_BIAS;
	exp += SEXP_BIAS;
	value.ll <<= 3;		/* 23 fract bits in word1 */
	rs = value.lg.word2 >> 30; /* bits 24 and 25 are r and s */
	if(value.lg.word2 | 0x7fffffff)
		rs |= S_BIT;
	mant = value.lg.word1;	/* 23 fract bits plus hiddenbit */

	/* rounding */
	if((R_BIT & rs) && ((S_BIT & rs) || (L_BIT & mant))){
		mant++;
		if(mant & 0x1000000){
			mant >>= 1;
			exp++;
		}
	} 
	
	/* check for overflow */
	if(exp >= SMAXEXP){
		value.lg.word1 = SINFINITY;
		errno = EDOM;
		return(value.lg.word1);
	}

	/* check for underflow */
	if(exp < 1){
		if(exp < -23){
			errno = ERANGE;
			return(0);
		}
		while(exp++ <= 0)
			mant >>= 1;
		exp = 0;
	}
	value.fbval.frac1 = mant;
	value.fbval.sign = sign;
	value.fbval.exp = exp;
	return(value.lg.word1);
}


/* dtofp converts a IEEE double to an IEEE single floating point
   and then back to a double again.				*/

double			
dtofp(val)
double val;
{
	fp value;
	unsigned long mant;
	register exp, rs, sign;
	value.dval = val;
	exp = value.bval.exp;
	sign = value.bval.sign;

	/* If both words of the double are all zeros, we
	** have 0.0, so return it now.
	*/
	if (value.lg.word1 == 0x0 && value.lg.word2 == 0x0)
		return (0.0);		/* return true 0 */

	/* Result nonzero, but is a sub-normal number.
	** Sub-normal numbers are too small for floats.
	*/
	if((exp == 0) && (value.ll & VALMASK))
		return (0.0);

	value.ll = (value.ll & VALMASK) | HIDDENBIT;
	exp -= DEXP_BIAS;
	exp += SEXP_BIAS;
	value.ll <<= 3;		/* 23 fract bits in word1 */
	rs = value.lg.word2 >> 30; /* bits 24 and 25 are r and s */
	if(value.lg.word2 | 0x7fffffff)
		rs |= S_BIT;
	mant = value.lg.word1;	/* 23 fract bits plus hiddenbit */

	/* rounding */
	if((R_BIT & rs) && ((S_BIT & rs) || (L_BIT & mant))){
		mant++;
		if(mant & 0x1000000){
			mant >>= 1;
			exp++;
		}
	} 
	
	/* check for overflow */
	if(exp >= SMAXEXP){
		value.lg.word1 = SINFINITY;
		value.lg.word2 = 0;
		errno = EDOM;
		return(value.dval);
	}

	/* check for underflow */
	if(exp < 1){
		if(exp < -23){
			errno = ERANGE;
			return(0.0);
		}
		while(exp++ <= 0)
			mant >>= 1;
		exp = 0;
	}
	value.lg.word2 = 0x7 & mant;
	value.lg.word2 <<= 29;		/* has lower 3 bits in position */
	value.lg.word1 = mant >> 3;	/* has higher 20 bits */
	value.bval.sign = sign;
	exp -= SEXP_BIAS;
	exp += DEXP_BIAS;
	value.bval.exp = exp;
	return(value.dval);
}

/* flcmp compares two IEEE double floating point numbers returning:
	-1	if arg1 < arg2
	0	if arg1 == arg2
	1	if arg1 > arg2
*/

flcmp(arg1,arg2)
double arg1,arg2;
{
	fp x,y;
	int retval;
	int xsign;

	x.dval = arg1;
	y.dval = arg2;

	if (x.ll == y.ll)
	    return( 0 );
	
	/* Values are different. */
	xsign = x.bval.sign;
	if(xsign != y.bval.sign)
	    return( xsign ? -1 : 1 );

	/* Signs are the same.  Check magnitudes. */
	if(x.bval.exp > y.bval.exp)
	    retval = 1;
	else if (x.bval.exp < y.bval.exp)
	    retval = -1;
	/* Same exponents. */
	/* NOTE:  the correct check here would appear to be
	** x.bval.frac > y.bval.frac, but the Amdahl compiler
	** seems to have problems with the bitfields.  Since
	** the high-order parts are known to be the same, this
	** is equivalent (and probably cheaper).
	*/
	else if ((unsigned long long) x.ll > (unsigned long long) y.ll)
	    retval = 1;
	else if ((unsigned long long) x.ll < (unsigned long long) y.ll)
	    retval = -1;
	else
	    cerror("confused flcmp()");
 
	/* Reverse sense of compare if negative values. */
	return( xsign ? -retval : retval );
}

/* discerns which numbers create special conditions */
dspecial(op,arg1,arg2)
int	op;
double arg1,arg2;
{
	register int xtype, ytype;

	xtype = argtype(arg1,op);
	ytype = argtype(arg2,op);
	switch(op){
	case ADD:
	if((xtype == NONE) && (ytype == NONE)) return(NONE);
	if((xtype == DZERO) && (ytype == NONE)) return(ARG2);
	if((xtype == NONE) && (ytype == DZERO)) return(ARG1);
	if((xtype == DNEGZERO) && (ytype == NONE)) return(ARG2);
	if((xtype == NONE) && (ytype == DNEGZERO)) return(ARG1);
	if((xtype == DNEGZERO) && (ytype == DZERO)) return(DZERO);
	if((xtype == DZERO) && (ytype == DNEGZERO)) return(DZERO);
	if((xtype == DZERO) && (ytype == DZERO)) return(DZERO);
	if((xtype == DNEGZERO) && (ytype == DNEGZERO)) return(ARG1);
	if((xtype == INF) && ((ytype == NONE) || (ytype == DZERO))) return(ARG1);
	if((xtype == INF) && ((ytype == NONE) || (ytype == DNEGZERO))) return(ARG1);
	if(((xtype == NONE) || (ytype == DZERO)) && (ytype == INF)) return(ARG2);
	if((xtype == NEGINF) && ((ytype == NONE) || (ytype == DZERO))) return(ARG1);
	if((xtype == NEGINF) && ((ytype == NONE) || (ytype == DNEGZERO))) return(ARG1);
	if(((xtype == NONE) || (xtype == DZERO)) && (ytype == NEGINF)) return(ARG2);
	if((xtype == DZERO) || (xtype == DNEGZERO) && (ytype == INF)) return(INF);
	if((xtype == DZERO) || (xtype == DNEGZERO) && (ytype == NEGINF)) return(NEGINF);
	if((xtype == INF) && (ytype == NEGINF)) return(NAN);
	if((xtype == NEGINF) && (ytype == INF)) return(NAN);
	if((xtype == NAN) || (ytype == NAN)) return(NAN);
	break;
	case MUL:
	if((xtype == NONE) && (ytype == NONE)) return(NONE);
	if((xtype == INF) && (ytype == DNEGZERO)) return(NEGNAN);
	if((xtype == DNEGZERO) && (ytype == INF)) return(NEGNAN);
	if((xtype == NAN) || (ytype == NAN)) return(NAN);
	if((xtype == NEGINF) && (ytype == DNEGZERO)) return(NAN);
	if((xtype == DNEGZERO) && (ytype == NEGINF)) return(NAN);
	if((xtype == DZERO) && (ytype == DZERO)) return(DZERO);
	if((xtype == DNEGZERO) && (ytype == DNEGZERO)) return(DZERO);
	if((xtype == DNEGZERO) && (ytype == DZERO)) return(DNEGZERO);
	if((xtype == DZERO) && (ytype == DNEGZERO)) return(DNEGZERO);
	if((xtype == INF) && (ytype == DZERO)) return(NAN);
	if((xtype == DZERO) && (ytype == INF)) return(NAN);
	
	break;
	case DIV:
	if((xtype == NONE) && (ytype == NONE)) return(NONE);
	if((xtype == NEGNONE) && (ytype == NEGNONE)) return(NONE);
	if((xtype == NEGNONE) && (ytype == NONE)) return(NONE);
	if((xtype == NONE) && (ytype == NEGNONE)) return(NONE);
	if((xtype == DZERO) && (ytype == DZERO)) return(NAN);
	if((xtype == DNEGZERO) && (ytype == DNEGZERO)) return(NAN);
	if((xtype == DZERO) && (ytype == DNEGZERO)) return(NEGNAN);
	if((xtype == DNEGZERO) && (ytype == DZERO)) return(NEGNAN);
	if((xtype == NAN) || (ytype == NAN)) return(NAN);
	if((xtype == INF) && (ytype == INF)) return(NAN);
	if((xtype == NEGINF) && (ytype == NEGINF)) return(NAN);
	if((xtype == INF) && (ytype == NEGINF)) return(NEGNAN);
	if((xtype == NEGINF) && (ytype == INF)) return(NEGNAN);
	if((xtype == NEGINF) && (ytype == DNEGZERO)) return(INF);
	if((xtype == INF) && (ytype == DNEGZERO)) return(NEGINF);
	if((xtype == DNEGZERO) && (ytype == INF)) return(DNEGZERO); 
	if((xtype == DZERO) && (ytype == INF)) return(DZERO);
	if((xtype == DNEGZERO) && (ytype == NEGINF)) return(DZERO);
	if((xtype == DZERO) && (ytype == NEGINF)) return(DNEGZERO);
	if((xtype == NEGNONE) && (ytype == INF)) return(DNEGZERO);
	if((xtype == NONE) && (ytype == NEGINF)) return(DNEGZERO);
	if((xtype == NEGNONE) && (ytype == NEGINF)) return(DZERO);
	if((xtype == NONE) && (ytype == INF)) return(DZERO);
	if((xtype == DNEGZERO) && (ytype == NONE)) return(DNEGZERO);
	if((xtype == DZERO) && (ytype == NEGNONE)) return(DNEGZERO);
	if((xtype == NONE) && (ytype == DNEGZERO)) return(NEGINF);
	if((xtype == NONE) && (ytype == DZERO)) return(INF);
	if((xtype == DNEGZERO) && (ytype == NEGNONE)) return(DZERO);
	if((xtype == DZERO) && (ytype == NONE)) return(DZERO);
	if((xtype == NEGNONE) && (ytype == DZERO)) return(NEGINF);
	if((xtype == NEGNONE) && (ytype == DNEGZERO)) return(INF);
	return(NONE);
	break;
	}
}

/* return type of arg */
argtype(arg,op)
double arg;
int op;
{
	fp x;
	x.dval = arg;
	if(x.bval.exp == MAXEXP){
		errno = ERANGE;
		if(x.ll & VALMASK) return(NAN);
		if(x.bval.sign) return(NEGINF);
		return(INF);
	}
	if(x.bval.exp == 0){
		if((x.ll & VALMASK) == 0){
			if(x.bval.sign == 1)
				return(DNEGZERO);
			else return(DZERO);
		}
	}
	if(op == DIV)
		if(x.bval.sign) return(NEGNONE);
	return(NONE);
}


/* return 1 if argument is zero floating point value, else 0 */

int
fp_iszero(arg)
fp arg;
{
    return( !arg.ll );
}

#endif	/* defined(uts) && defined(FP_EMULATE) */
