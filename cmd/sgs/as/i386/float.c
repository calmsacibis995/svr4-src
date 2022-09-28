/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:i386/float.c	1.3"
#include <ctype.h>
#include "systems.h"
#include <stdio.h>
#include "symbols.h"
#include "instab.h"
#include "instr.h"
#include "parse.h"

#define SINGLEBIAS 127
#define DOUBLEBIAS 1023


static int atofd();
static void fpdtos();

extern double
	lclatof(),
	floor(),
	pow(),
	log2();

static int
atofs(string,ret)
char *string;
unsigned long ret[];
{
	if (atofd(string,ret))
		return(1);
	else {
		fpdtos(ret);
		return(0);
	}
	/*NOTREACHED*/
}

static int
atofd(string,ret)
char *string;
unsigned long ret[];
{
	double	dbl, fexp;
	double	two16 = 65536.;
	long	sbit, exponent, mantissa;
	short	leadbit = 0;

/*
 *	Some of the cast operations, namely
 *	double -> float and double -> long  don't necessarily
 *	have the same implementation on various machines
 *	that is why the following two variables are defined.
 *	Any time there are used in this file it is to get
 *	around the cast operator.
 */
	double	dtmp;		/*  dummy var needed for casting */
	float	ftmp;		/*  dummy var needed for casting */
	long	i, nf;


	dbl = lclatof(string);

	if (dbl == 0.0) {
		ret[0] = 0x0L;
		ret[1] = 0x0L;
		return(0);
	}
	if (dbl > 0.0)
		sbit = 0L;
	else {
		sbit = 1L;
		dbl *= -1.0;
	}
/*
 *	Find out how many two's are in the floating point number ('dbl')
 *	then divide 'dbl' by 2**(number of 2's) (by multiplying
 *	by its reciprical) in order to scale it down within the range
 *	1.0 =< dbl =< 2.0
 *	Note: Due to rounding and imprecise results the resulting
 *	number may be slightly < 1.0 or slightly > 2.0
 *	which is why we must check for these cases.
 */
	fexp = floor(log2(dbl));
/*
 * The following line of code is commented out due to some problems
 * with the precision of the pow function. It is replaced by the
 * following while loop and if stmt.
 *
	dbl *= pow((double)2.0,-fexp);
 */
	i = (long)fexp;
	nf = 0;
	if (i < 0) { nf = 1; i = -i; }
	dtmp = (double)1.0;
	while (i-- > 0L) dtmp *= (double)2.0;
	if (nf == 0 )
		dbl *= ((double)1.0 / dtmp);
	else
		dbl *= dtmp;

	if (dbl >= 2.0) {
		dbl /= (double)2.0;
		++fexp;
	}
	else if (dbl < 1.0) {
		dbl *= (double)2.0;
		--fexp;
	}
#if	DEBUG
	(void) printf("ato3bs: fexp= %f   dbl= %.13e   %lx %lx\n",fexp,dbl,dbl);
#endif

	if (fexp > -(DOUBLEBIAS-1))  /* normalized numb */
		exponent = (long)fexp + (long)DOUBLEBIAS;
	else {		/* denormalized numb, must adjust mantissa
			 * e.g. a number like 1.0E-1025 will be coded
			 *	as 0.001E-1022 with some precision lost
			 */
		exponent = 0L;
/*
		fexp += (double)(DOUBLEBIAS-1);
		dbl *= pow((double)2.0, fexp);
*/
#if	DEBUG
	(void) printf("ato3bs: denorm numb - dbl= %.13e  fexp= %f\n",dbl,fexp);
#endif
	}
	if (exponent != 0L)
		leadbit = 1;
#if	DEBUG
	(void) printf("ato3bs: leadbit= %d\n", leadbit);
#endif

/*
 *	Multiply by 2**20 to get the first 20 bits of the fraction
 *	necessary to fill in the first word.
 *	Then subtract out this number and multiply by 2**32 to
 *	get the remaining fractional part for the second word of the double
 */

	dtmp = (dbl - (double)leadbit) * (double)(1L<<20); /* mult by 2**20 */
	ftmp = floor(dtmp);		/* takes the place of a cast */
	mantissa = ftmp;	/* takes the place of a cast */
	ret[0] = (sbit << 31) | (exponent << 20) | mantissa;
#if	DEBUG
	(void) printf("ato3bs: sign, (exp<<20), mantissa= %ld  %lx  %lx\n",
		sbit, (exponent<<20), mantissa);
	(void) printf("ato3bs: dtmp,ftmp,mantissa= %.13e  %.13e  %ld\n",
		dtmp,ftmp,mantissa);
#endif
	dtmp -= mantissa;
	dtmp *= two16;
	ftmp = floor(dtmp);		/* takes the place of a cast */
	ret[1] = ftmp;
	dtmp -= ret[1];
	dtmp *= two16;
	ftmp = dtmp;		/* takes the place of a cast */
	mantissa = ftmp;	/* takes the place of a cast */
	ret[1] = (ret[1]<<16) | mantissa;
#if	DEBUG
	(void) printf("ato3bs: result (val,val2)= %08lx  %08lx\n",ret[0],ret[1]);
#endif
	return(0);
} /* atofd */


/*
 *
 *	round double fp number to single by:
 *		- get high 3 bits of second word, these will go into
 *			new single word since the exponent shrinks to 8 bits
 *		- get guard bits (next 2 bits after high 3 bits)
 *		- if both guard bits equal 1 then add 1 to hi3bits
 *			note: if this add produces a carry it should
 *				promulgate into the mantissa however for the
 *				first cut we won't promulgate it.
 *
 */
static void
fpdtos(fpval)
unsigned long fpval[];
{
	long	sign, exp, mantissa, hi3bits, guard;

	sign = fpval[0] & ((unsigned) 1L<<31);
	exp = (fpval[0]>>20) & 0x7ffL;
	if (exp != 0L)
		exp = (exp - (long)DOUBLEBIAS) + (long)SINGLEBIAS;
#if	DEBUG
	(void) printf("fpdtos: sign, old,new exp= %lx   %lx   %lx\n",sign,
		((fpval[0]>>20) & 0x7ffL), exp);
#endif

	mantissa = fpval[0] & 0xfffffL;
	hi3bits = (fpval[1]>>29) & 0x7L;
	guard = (fpval[1]>>27) & 0x3L;
	if (guard & 0x3L) {	/* round up */
		if (!(hi3bits & 0x7L))	/* add will not produce a carry */
			hi3bits += 1;
	}
#if	DEBUG
(void) printf("fpdtos: hi3bits,guard= %lx  %lx\n",hi3bits,guard);
(void) printf("fpdtos: old, new man= %lx    %lx\n",mantissa,(mantissa<<3) | hi3bits);
#endif
	mantissa = (mantissa<<3) | hi3bits;

	fpval[0] = 0L;
	fpval[0] = sign | (exp<<23) | mantissa;
#if	DEBUG
(void) printf("fpdtos: new expval= %lx\n",fpval[0]);
#endif
}

atob16f(line,lp)
char *line;
long *lp;
{
	double	dbl,fexp;
	long	exponent,sbit;
	long	mantissa;

/*
 *
 *	Some of the cast operations, namely
 *	double -> float and double -> long  don't necessarily
 *	have the same implementation on various machines
 *	that is why the following two variables are defined.
 *	Any time there are used in this file it is to get
 *	around the cast operator.
 *
 */
	double	dtmp;		/*  dummy var needed for casting */
	float	ftmp;		/*  dummy var needed for casting */

#ifdef	u370
	long	i, nf;	/* dummy var needed for maxi kludge
			it can be deleted when maxi libm is fixed */
#endif

	dbl = lclatof(line);
	if (dbl == 0.0) {
		*lp = 0x0L;
		return(0);
	}
	if (dbl > 0.0)
		sbit = 0L;
	else {
		sbit = 1L;
		dbl *= -1.0;
	}
	fexp = floor(log2(dbl));

/*
 *	The following ifdef is a maxi kludge.
 *	The error tolerence of the pow function
 *	is too small so it doesn't always
 *	return the correct value.
 *
 *	Note: 'dtmp' is NOT being used as a casting var in this ifdef.
 */
#ifdef	u370
	i = (long)fexp;
	if ( i >= 0 ) nf = 0;
	else {
		nf = 1;
		i = -i;
	}
	dtmp = (double)1.0;
	while (i-- > 0L) dtmp *= (double)2.0;
	if (nf == 0)
		dbl *= ((double)1.0 / dtmp);
	else
		dbl *= dtmp;
#else
	dbl *= pow((double)2.0,-fexp);
#endif

	ftmp = dbl;		/* this assgmt takes the place of a cast */
	if (ftmp >= 2.0) {
		dbl /= 2;
		++fexp;
	}
	else if (ftmp < 1.0) {
		dbl *= 2;
		--fexp;
	}
	exponent = (long)fexp + 127;
	dtmp = ((dbl - (double)1.0) * (double)0x00800000) ;  /* multiply by 2**23 */
	ftmp = dtmp;		/* this assgmt takes the place of a cast */
	mantissa = ftmp ;	/* this assgmt takes the place of a cast */
	*lp = (sbit << 31) | (exponent << 23) | mantissa;
	return(0);
}
double
lclatof(p)
register char *p;
{
	register char c;
	double fl, flexp, exp5;
	double big = 72057594037927936.;  /*2^56*/
	double ldexp();
	register int eexp, exp, bexp;
	register short	neg, negexp ;

	neg = 1;
	while((c = *p++) == ' ')
		;
	if (c == '-')
		neg = -1;
	else if (c=='+')
		;
	else
		--p;

	exp = 0;
	fl = 0;
	while ((c = *p++), isdigit(c)) {
		if (fl<big)
			fl = 10*fl + (c-'0');
		else
			exp++;
	}

	if (c == '.') {
		while ((c = *p++), isdigit(c)) {
			if (fl<big) {
				fl = 10*fl + (c-'0');
				exp--;
			}
		}
	}

	negexp = 1;
	eexp = 0;
	if ((c == 'E') || (c == 'e')) {
		if ((c= *p++) == '+')
			;
		else if (c=='-')
			negexp = -1;
		else
			--p;

		while ((c = *p++), isdigit(c)) {
			eexp = 10*eexp+(c-'0');
		}
		if (negexp<0)
			eexp = -eexp;
		exp = exp + eexp;
	}

	negexp = 1;
	if (exp<0) {
		negexp = -1;
		exp = -exp;
	}

	flexp = 1;
	exp5 = 5;
	bexp = exp;
	for (;;) {
		if (exp&01)
			flexp *= exp5;
		exp >>= 1;
		if (exp==0)
			break;
		exp5 *= exp5;
	}
	if (negexp<0)
		fl /= flexp;
	else
		fl *= flexp;
	fl = ldexp(fl, negexp*bexp);
	if (neg<0)
		fl = -fl;
	return(fl);
}


extern double log();

double
log2(x)
double x;
{
	return( log(x) / log((double)2.0) );
}

/*
**	80287 conversion routines 
**
*/


/*
 *	conv287
 *
 *	main conversion routine
 *
 *	type	-	is type of constant expected
 *	asc	-	ascii representation of that value
 *	value	-	pointer to union containg various types for 
 *			return of converted answer
 *
 *	returns 
 *		0	success
 *
 */

conv287 ( type , asc , value )
char *asc ;
floatval *value ;
{

	switch( type )  {

	case PSLONG:
			return dolongs ( 0 , asc , value ) ;
	case PSFLOAT:
			return doreals ( 0 , asc , value ) ;
	case PSDOUBLE:
			return doreals ( 1 , asc , value ) ;
	case PSEXT:
			return doreals ( 2 , asc , value ) ;
	case PSBCD:
			return dobcd ( asc , value ) ;
	}

}

dobcd( asc , value )
char *asc ;
floatval *value ;
{
	char buff[20] ;
	int i , j ;
	char *p ;

	p = asc ;
	while ( *p ) 
		p++ ;

	p-- ;

	for(i=0;i<20;i++)
		if (p < asc )
			buff[i] = '0' ;
		else
		{
			if( (*p < '0') || (*p > '9') )
				return 1 ;
			buff[i] = *p-- ;
		}

	for ( i = 0 ;i < 5 ; i++ )
	{
		value->fvala[i] = 0 ;
		for(j=0;j<4;j++)
		{
			value->fvala[i] <<= 4 ;
			value->fvala[i] |= (buff[i*4+j] & 0xf) ;
		}
	}

	return 0 ;
}

/*
 *	doreals - does floating point conversions
 */

doreals(type , asc , value )
char *asc ;
floatval *value ;
{
#if vax
	struct cheat { unsigned short a,b,c,d,e;} ;
	union {
		struct cheat cheats ;
		double cheatd ;
		} ch ;
	unsigned short exp ;
	sscanf ( asc , "%le" , &ch.cheatd ) ;
	/*
	 * special case for 0.0
	 */

	if ( ch.cheatd == 0.0 )
	{
		value->fvala[0] = 0 ;
		value->fvala[1] = 0 ;
		value->fvala[2] = 0 ;
		value->fvala[3] = 0 ;
		value->fvala[4] = 0 ;
		return 0 ;
	}

	switch( type ) {

	/* float */

	case 0: exp = ch.cheats.a >> 7 ;
		exp -= 0x81 ;
		exp += 0x7f ;
		value->fvala[1] = (ch.cheats.a & 0x007f) | ( exp << 7 ) ;
		value->fvala[0] = (ch.cheats.b);
		break ;

	/* double */

	case 1:	exp = ch.cheats.a >> 7 ;
		exp -= 0x81 ;
		exp += 0x3ff ;
		value->fvala[3] = (exp << 4 ) + ((ch.cheats.a >> 3) & 0xf) ;
		value->fvala[2] = (ch.cheats.a << 13) | (ch.cheats.b >> 3) ;
		value->fvala[1] = (ch.cheats.b << 13) | (ch.cheats.c >> 3) ;
		value->fvala[0] = (ch.cheats.c << 13) | (ch.cheats.d >> 3) ;
		break ;

	/* temp */

	case 2:	exp = ch.cheats.a >> 7 ;
		exp -= 0x81 ;
		exp += 0x3fff ;
		ch.cheats.e = 0 ;
		value->fvala[4] = exp ;
		value->fvala[3] = 0x8000 | ( ( ch.cheats.a << 8) & 0x7f00) | 
				( ch.cheats.b >> 8 ) ;
		value->fvala[2] = (ch.cheats.b << 8) | (ch.cheats.c >> 8) ;
		value->fvala[1] = (ch.cheats.c << 8) | (ch.cheats.d >> 8) ;
		value->fvala[0] = (ch.cheats.d << 8) | (ch.cheats.e >> 8) ;
		break ;

	}
#else /* 80286 and 3b's */
	switch( type ) {

	/* float */

	case 0: atofs(asc,value->fvalla);
#if uts || AR32W
		{
			unsigned short tmp;

			tmp = value->fvala[0];
			value->fvala[0] = value->fvala[1];
			value->fvala[1] = tmp;
		}
#endif
		break ;

	/* double */

	case 1:	atofd(asc,value->fvalla);


#if uts || AR32W
		{
			unsigned short tmp;

                        tmp = value->fvala[0];
                        value->fvala[0] = value->fvala[3];
                        value->fvala[3] = tmp;
                        tmp = value->fvala[1];
                        value->fvala[1] = value->fvala[2];
                        value->fvala[2] = tmp;


		}
#else	/* 386 */
		{
			unsigned long tmp;

			tmp = value->fvalla[0];
			value->fvalla[0] = value->fvalla[1];
			value->fvalla[1] = tmp;
		}

#endif

		break ;

	/* temp */

	case 2:	/* NOT YET IMPLEMENTED */
		break ;

	}
#endif /* vax */

	
	return 0 ;
}

dolongs(type , asc , value )
char *asc ;
floatval *value ;
{
#if vax
	struct cheat { unsigned short a,b ; } ;
	union  {
			struct cheat cheats ;
			unsigned long cheatl ;
		} ch ;
#endif
	unsigned long val ;
	unsigned short base ;
	char *p = asc ;

	
	val = (*p) - '0';
	if ((*p) == '0') {
		p++ ;
		if ((*p) == 'x' || (*p) == 'X') {
			base = 16;
		} else if (((*p) & ~' ') == 'B') { 
			base = 2;
		} else {
			p++ ;
			base = 8;
		}
	} else
		base = 10;
	while ( (( *(++p) >= '0') && (*p <= '9') )
	    || ((base == 16) &&
		((('a'<=(*p)) && ((*p)<='f'))||(('A'<=(*p)) && ((*p)<='F')))))
	{
		if (base == 8)
			val <<= 3;
		else if (base == 10)
			val *= 10;
		else if (base == 2)
			val <<= 1;
		else
			val <<= 4;
		if ('a' <= (*p) && (*p) <= 'f')
			val += 10 + (*p) - 'a';
		else if ('A' <= (*p) && (*p) <= 'F')
			val += 10 + (*p) - 'A';
		else	val += (*p) - '0';
	}

#if vax
	if(type)
	{
		ch.cheatl = val ;
		value->fvala[0] = 0 ;
		value->fvala[1] = 0 ;
		value->fvala[2] = ch.cheats.b ;
		value->fvala[3] = ch.cheats.a ;
		return 0 ;
	}
	else
	{
		ch.cheatl = val ;
		value->fvala[0] = (ch.cheats.a) ;
		value->fvala[1] = (ch.cheats.b) ;
		return 0 ;
	}
#else /* 80286 and 3b's */
	if(type)
	{
		/* NOT YET IMPLEMENTED */

		return 0 ;
	}
	else
	{
		value->fvall = val ;
#if AR32W /* target byte order is mirror image of host */
		{
			unsigned short tmp;

			tmp = value->fvala[0];
			value->fvala[0] = value->fvala[1];
			value->fvala[1] = tmp;
		}
#endif /* AR32W */
		return 0 ;
	}
#endif /* vax */
}
