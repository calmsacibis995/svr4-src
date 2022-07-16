/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/reducef.c	1.2"
/*
*********************************************************************
*	Range reduction for the trigonemtric functions
*	Single precision version
*       See Payne and Hanek, SIGNUM Jan '83 page 19
*       As much as possible, variable names match those in the
*       SIGNUM article.
*	Reduction is quadrant.
*
*       This implementation assumes:
*       a) the range of double precision floating point
*          is no greater than IEEE double precision,
*          else one needs more bits of pi.
*       b) L is less than 32 
*       c) L > 3
*       d) longs are 32 bits
*       e) floating point add/subtract are done via round to nearest
*       f) probably doesn't work on non-binary machines without some
*          more work.
*********************************************************************
*/

#include <math.h>
#include <values.h>

#define PADDING 20      /* Number of bytes of padding at start of pihex[] */
#define CHARSIZE 8      /* number of bits per character */

/******************* MACHINE DEPENDENT CONSTANTS ********************
*   PRECISION is the number of bits in mantissa
*   L is the largest number so that the product of two L-bit integers
*     fits into an integer.  We have chosen L a bit smaller than this
*     so that it isn't necessary to use unsigned integer arithmetic.
*   N is the smallest integer larger than PRECISION/L.
*   K is the number of guard digits for floating point operations
*       K       Maximum error in ulps for range reduction
*       0       1.0
*       1       0.75
*       2       0.63
*       3       0.56
*       4       0.53
*
*   NUMPARTIAL is the maximum size of the array of partial sums
*     and is given by:  (EMAX - EMIN + PRECISION + 3 + K) / L
*     EMAX and EMIN being the maximum and minimum exponents.
*   M1 = (2*PRECISION + K + L + 1)/L - initial number of partial sums
*   TWOL is 2**L
*   LMASK is TWOL-1
*   TWOPRECISION is 2**PRECISION.  If compiler doesn't do this right,
*     one should hardcode in the hex constant.
*   TWOLM2 is 2**(L-2)
*   TWOLM3 is 2**(L-3)
*   LM2MASK is TWOLM2 -1
*   if max1s is M1 * L -2 * PRECISION -K +2, then
*   MAX1S is max1s 1s followed by L-MAX1s 0's
*   MAX1S2 is max1s 1s followed by L-max1s-2 0's
*   MAX0S is 1 followed by L-max1s-1 0's
*   MAX0S2 is 1 followed by L-max1s-3 0's
*********************************************************************
*/

#if _IEEE
#define PRECISION 24
#define K 4	
#define L 15
#define LMASK 0x7fff
#define LM2MASK 0x1fff
#define	M1 4
#define N 2
#define TWOL 32768
#define TWOLM2 0x2000
#define TWOLM3 0x1000
#define TWOPRECISION 16777216.
#define NUMPARTIAL 22
#define BIAS 127
#define MAXEXP 255
#define MAX1S 0x7fe0
#define MAX1S2 0x1ff8
#define MAX0S 0x10
#define MAX0S2 0x4
#endif

/*************** END MACHINE DEPENDENT CONSTANTS *******************/

#define	FACT	4  /* indicates reduction - 4 for quadrant
		    * to reduce to other intervals, this
		    * can be changed to: 1 - full period,
		    *			 2 - half period,
		    *			 8 - octant
		    */
#define TWO_NEGL	1.0/(float)TWOL

/* pi/2 = 1.57... in hex */
static unsigned long piby2[] = {
	0x1,0x490f,0x6d51,0x85a,0x1846,0x4c4c,
	0x3314,0x2e03,0x3839,0x5129,0x127
};

/*
*********************************************************************
*   Inputs:
*   x is the argument to be reduced.  x is assumed to be positive and
*       greater than M_PI/2.
*
*   i = 0     -> reduced argument between 0 and pi/2
*       1     -> reduced argument between -pi/4 and pi/4
*
*   Outputs:
* 
*   I      0 <= I <= 3    (integer) == quadrant number
*
*   hr     reduced argument 
*		h*r  (where 0 <= h < 1.0,  r = (pi/2) )
*			(or -0.5 <= h < 0.5, for i == 1)
*   dhr    extra bits of reduced argument
*
*   relationship of variables:
*
*   x = hr + dhr + (I*pi/2)    (modulo 2*pi)
*
*********************************************************************
*/


void
_reducef(float x, int i, int *I,float *hr,float *dhr)
{
	int k;   		/* exponent of x */
	int sign = 0;		/* negate reduced argument? */
	int onemh = 0;		/* return 1-h? */
	register int n, j, l;   /* indices */
	register int M;         /* M*L bits are needed for Q' */
	int needmoreiter;       /* Flag.  Are more iterations needed? */
	int nbits;              /* Number of leading bits of 0s or 1s */
	int loss;		/* flag - too many leading 0's or 1's */
	long gval;              /* saved bits of 1/2pi */
	static long g();	/* return correct bits of 1/2pi */
	register long prod;     /* Used in summing the partial[l]'s */
	register long tot;      /* Used in summing the partial[l]'s */
	long F[N];              /* f stored in L bit chunks */
	long temp;
	long partial[NUMPARTIAL]; /* partial x * 1/2pi */
	long tpart[NUMPARTIAL+5]; /* temporary storage for mult by pi*/

	register float factor; /* for storing powers of two */
	float temp1, temp2;  /* temporary float precision */

#if _IEEE /* break up mantissa of float into 15 bit chunks */
#if M32 || u3b || u3b2 || u3b5 || u3b15
/* 3B byte order */
	union	rfval {
		float	d;
		struct parts {
			unsigned int	sgn : 1;
			unsigned int 	exp : 8; 
			unsigned int	p1 : 8;
			unsigned int	p2 : 15;
		} dp;
	} f;
#else  /* reverse byte order */
	union	rfval {
		float	d;
		struct parts {
			unsigned int	p2 : 15;
			unsigned int	p1 : 8;
			unsigned int 	exp : 8; 
			unsigned int	sgn : 1;
		} dp;
	} f;
#endif  /* byte order */
#else
	float f;               /* normalized fractional part of x */
#endif 	/* IEEE */


#if _IEEE /* use union above to store argument in F[] array in 15
	   * bit chunks -  if not IEEE use frexp and modf to do 
	   * same thing - unbiased exponent of x is stored in k 
	   */
	f.d = x;
	if ((k = f.dp.exp) != MAXEXP)  /* not NaN or inf */
		k -= (BIAS-1); /* un-biased exponent */
	F[1] = 0x100 | f.dp.p1;  /* first part-don't forget implied 1 */
	F[0] = f.dp.p2;
#else
	f = (float)frexp((double)x, &k);
	f *= TWOPRECISION;
	for(j = 0; j < N; j++) {
		f = f / TWOL;
		F[j] = TWOL * modff(f, &f);
	}
#endif
	M = M1;
	for(l = 0;l < NUMPARTIAL; l++) 
		partial[l] = 0;

	/*
	**************************************************
	* F * D0 - first approximation to x * 1/2*pi 
	*   The product of F and D0 is stored as
	*   partial[0]   * implied 2^0
	*   partial[1]   * implied 2^-L
	*   ....
	*   partial[M-1] * implied 2^(1-M)L
	*   partial[M]   * implied 2^-ML
	*   Other terms are totally ignored!
	**************************************************
	*/
	for(j = 1; j <= M; j++) {
		gval = g(k - PRECISION - L + L * j);
		/* If n-j >=1,  it won't contribute */
		for(n = 0;(n < N)&&(n <= j); n++) {
			prod = F[n] * gval;
			/* Propagate carries */
			for(l = j-n; (l >= 0)&&(prod != 0); l--) {
				tot = prod + partial[l];
				partial[l] = tot % TWOL;
				prod = tot / TWOL;
			}
		}
	}
	/*
	**************************************************
	*   Find I.  Assuming that L is greater than 2,
	*   only two of the partial[]'s contribute.
	**************************************************
	*/
	temp = ((partial[1] * FACT)/TWOL) + partial[0] * FACT;
	*I = temp % FACT;
	/*
	**************************************************
	* Now check whether we should return h or 1-h
	* If i - want result to be between -pi/4 and pi/4
	* if the fraction  is >= 0.5, replace with 1 - fraction,
	* set sign flag and increment quadrant
	* Else if !i and quadrant is odd return 1-h rather than h;
	* again, replace fraction with 1 - fraction
	**************************************************
	*/

	if (i) {
		if ((partial[1] & LM2MASK) > TWOLM3) {
			sign = 1;
			onemh = 1;
			*I += 1;
		}
	}
	else {
		if ((*I) % 2)
			onemh = 1;
	}
	needmoreiter = 1;
	while (needmoreiter) {

	/*
	**************************************************
	*   Now check for loss of significance (by counting
	*   leading zeros or ones) and calculate extra
	*   bits if needed.
	**************************************************
	*/


		nbits = M * L-2 * PRECISION-K +2;
		
		j = 1;
		loss = 0;
		/* Check for loss of significance here. */
		if (onemh) { /* check number of leading 1's */
			while (nbits > 0 && !loss) {
				if (nbits < L) {
					if (j == 1)
						temp = MAX1S2;
					else
						temp = MAX1S;
					if ((partial[j] & temp) == temp)
						loss = 1;
					else nbits = 0;
				} else { 
					if ((j == 1 && ((partial[1] 
					& LM2MASK) < LM2MASK)) ||
					(j > 1 && partial[j] < LMASK))
						nbits = 0;
					else nbits -= L;
				}
				j++;
			}
		}
		else  {	/* !onemh - check number of leading 0's */
			while (nbits > 0 && !loss) {
				if (nbits < L) {
					if (j == 1)
						temp = MAX0S2;
					else
						temp = MAX0S;
					if ((j == 1 && ((partial[1]
					& LM2MASK) < temp)) ||
					(j > 1 && partial[j] < temp))
						loss = 1;
					else nbits = 0;
				}
				else {
					if ((j == 1 && ((partial[1]
					& LM2MASK) > 0)) ||
					(j > 1 && partial[j] > 0))
						nbits = 0;
					else nbits -= L;
				}
				j++;
			}
		}
		if (loss) {
			/*
			***********************************************
			*   Need more bits!  Calculate D.
			*   Code is nearly same as prior loop.
			***********************************************
			*/
			gval = g(k - PRECISION + L * M); /* j=M+1 */
			M++;
			for(n = 0; (n < N) && (n <= M); n++) {
				prod = F[n] * gval;
				/* Propagate carries. */
				for(l = M-n;(l >= 0)&&(prod !=0);l--){
					tot = prod + partial[l];
					partial[l] =  tot % TWOL;
					prod = tot / TWOL;
				}
			}
		} else
			/* no further loss of significance */
			needmoreiter = 0;
	} /* end while (needmoreiter) */
/*
*******************************************************
*   Assume that h, dhr are right.  Let's multiply
*   by PI/2 very carefully.  
*******************************************************
*/
	/* REMEMBER TO MASK OFF THE RIGHT BITS of partial */
	partial[0] = 0;
	partial[1] &= LM2MASK;
	/*
	 **********************************************************
	 * If onemh, replace h with 1-h.
	 * 1 - h == 1. - partial[1] *2^-(L-2) - partial[2] *
	 *  2^-(2L-2) - partial[3] *2^-(3L-2) ....  to get this
	 * effect, replace partial[1] with 2^(L-2) -partial[1]
	 * and negate other terms
	 **********************************************************
	 */
	 if (onemh) {
		partial[1] = TWOLM2 - partial[1];
		for(n = 2; n <= M; n++)
			partial[n] = -partial[n];
	}
	for(n=0; n <= M+5 ;n++)
		tpart[n]=0;
	/*
	**************************************************************
	*   The simplistic technique is to do the multiplication as
	*   sum j=0 to M sum n=0 to 5 partial[j]*pi[n] 2^-(j+n)L.
	**************************************************************
	*/
	for(j = M; j >= 0; j--) {   /* Loop over partials */
		for(n = 5; n >= 0; n--) {  /* use 1+(5*15) bits of pi */
			/* This term starts to contribute at l=j+n */
			prod = piby2[n] * partial[j];
			for(l = j+n; (l >= 0)&&(prod != 0) ;l--) {
				/* propagate carries */
				tot = prod + tpart[l];
				tpart[l] = tot % TWOL;
				prod = tot/TWOL;
			}
		}
	}
	/* Up to here, we were right. */
	/* Done with multiplications, extract hr, dhr */
	factor = FACT;
	*hr = 0.0f;
	for(l = 1; l <= M; l++) {
		factor *= TWO_NEGL;
		*hr += factor * tpart[l];
	}	
	factor = FACT;
	*dhr = *hr;
	for(l = 1; l <= M; l++) {
		factor *= TWO_NEGL;
		*dhr -= factor * tpart[l];
	}
	*dhr = - *dhr;

	/*
	**********************************************************
	*    hr and dhr may not be rounded exactly right yet,
 	*    let H = h + dh  be the "exact" answer,
	*    and define   h'+dh' = h+dh
	*    then   h' = h+dh	round to nearest,
	*    and    dh' = (h-h') + dh.   (need two steps)
	**********************************************************
	*/	
	temp1= *hr + *dhr;
	temp2= *hr - temp1;
	temp2= temp2 + *dhr;
	if (sign) { /* result is between -pi/4 and 0 */
		temp1 = - temp1;
		temp2 = - temp2;
	}
	*hr = temp1;
	*dhr = temp2;
	return;
}
/*
******************************************************************
*   1/(2*pi) in hex.  The binary point is just before the first
*   bit of second line (after the 5 longs of all zeros.) 
*   Current table size is big enough for IEEE double precision.
******************************************************************
*/
static unsigned long pihex[] = {
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x28be60db,0x9391054a,0x7f09d5f4,0x7d4d3770,0x36d8a566,
	0x4f10e410,0x7f9458ea,0xf7aef158,0x6dc91b8e,0x909374b8,
	0x01924bba,0x82746487,0x3f877ac7,0x2c4a69cf,0xba208d7d,
	0x4baed121,0x3a671c09,0xad17df90,0x4e64758e,0x60d4ce7d,
	0x272117e2,0xef7e4a0e,0xc7fe25ff,0xf7816603,0xfbcbc462,
	0xd6829b47,0xdb4d9fb3,0xc9f2c26d,0xd3d18fd9,0xa797fa8b,
	0x5d49eeb1,0xfaf97c5e,0xcf41ce7d,0xe294a4ba,0x9afed7ec,
	0x47e35742,0x1580cc11,0xbf1edaea,0xfc33ef08,0x26bd0d87,
	0x6a78e458,0x57b986c2,0x19666157,0xc5281a10,0x237ff620,
	0x135cc9cc,0x41818555,0xb29cea32,0x58389ef0,0x231ad1f1,
	0x0670d9f3,0x773a024a,0xa0d6711d,0xa2e58729,0xb76bd134,
	0x55c6414f,0xa97fc1c1,0x4fdf8cfa,0x0cb0b793,0xe60c9f6e,
	0xf0cf49bb,0xdac797be,0x27ce87cd,0x72bc9fc7,0x61fc4864,
	0x1f1f091a,0xbe9bb55d,0xcb4c10ce,0xc571852d,0x674670f0,
	0xb12b5053,0x4b174003,0x119f618b,0x5c78e6b1,0xa6c0188c,
	0xdf34ad25,0xe9ed3555,0x4dfd8fb5,0xc60428ff,0x1d934aa7,
	0x592af5dc,0x3e1f18d5,0xec1eb9c5,0x45d59270,0x36758ece,
	0x2129f2c8,0xc91de2b5,0x88d516ae,0x47c006c2,0xbc77f386,
	0x7fcc67da,0x87999855,0xe651feeb,0x361fdfad,0xd948a27a 
};

/*
******************************************************************
*     g is the function to return L bits of 1/(2*pi)
*     starting at position j.  L defined previously.
*     We explicitly assume that L is less than 32 to
*     make the shifting easier.  Note that 32 bits
*     longs are assumed in this support routine.
******************************************************************
*/

static long g(j)
register int j;	
{
	register int jmod32,jover32;
	register unsigned long val1, val2; 
	/*
	***************************************************
	*  Two possibilities.
	*  a) all L bits are in one long
	*  b) The L bits span two longs
	***************************************************
	*/
	j = j + PADDING*CHARSIZE;  /*Compensate for leading zeros*/
	jmod32 = j % 32;
	jover32 = j / 32;
	val1 = pihex[jover32] << jmod32;
	val1 = val1 >> (32-L);
	val1 &= LMASK;

	if (jmod32 <= (32-L) ) {
		/*case a)*/
		return val1;
	} else {
		/*case b)*/
		val2 = pihex[jover32 + 1] >> (64 - L - jmod32);
		val2 &= LMASK;
		return val1 + val2;
	}
}
