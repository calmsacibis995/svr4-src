/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/pow.c	1.11"
/*LINTLIBRARY*/
/* 
 */

/* POW(X,Y)  
 * RETURN X**Y 
 *
 * Method
 *	1. Compute and return log(x) in three pieces:
 *		log(x) = n*ln2 + hi + lo,
 *	   where n is an integer.
 *	2. Perform y*log(x) by simulating muti-precision arithmetic and 
 *	   return the answer in three pieces:
 *		y*log(x) = m*ln2 + hi + lo,
 *	   where m is an integer.
 *	3. Return x**y = exp(y*log(x))
 *		= 2^m * ( exp(hi+lo) ).
 *
 */

/*
 * Special case(s):
 * 1. if x < 0 and y is not an integer or x == 0 and y is <= 0
 *	0 is returned and a domain error signaled
 * 2. if the correct value would overflow or underflow, pow returns
 *    +- HUGE or 0 respectively and signals range error.
 */

#include "synonyms.h"
#include <math.h>
#include <values.h>
#include <errno.h>
#include "fpparts.h"

#if _IEEE
static double
ln2hi  =  6.9314718036912381649E-1,
ln2lo  =  1.9082149292705877000E-10,
invln2 =  1.4426950408889633870E0,
sqrt2  =  1.4142135623730951455E0;
#else
#if vax /* Vax D format */
static double
ln2hi  =  6.9314718055829871446E-1,
ln2lo  =  1.6465949582897081279E-12,
invln2 =  1.4426950408889634148E0,
sqrt2  =  1.4142135623730950622E0;
#endif
#endif

/* coefficients for polynomial expansion */
#if _IEEE
static double p[] = {
	1.4795612545334174692E-1,
	1.5314087275331442206E-1,
	1.8183562745289935658E-1, 
	2.2222198607186277597E-1,
	2.8571428742008753154E-1,   
	3.9999999999416702146E-1,    
	6.6666666666667340202E-1,
	0.0
};
#else 
#if vax /* Vax D format */
static double p[] = {
	1.2500000000000000000E-1,
	1.3338356561139403517E-1,
	1.5382888777946145467E-1,
	1.8181879517064680057E-1,
	2.2222221233634724402E-1,
	2.8571428579395698188E-1,
	3.9999999999970461961E-1,
	6.6666666666666703212E-1,
	0.0
};
#endif
#endif

/* codes for error types */
#define DOM 0
#define OVER 1
#define UNDER 2

double	pow(x, y)
double	x, y;
{
	static double pow_exc(),_exp__E();
	register double s, c, t;
	double	z, tx, ty;
	float	sx, sy;
	long	k = 0;
	int	n, m, r;
	int 	neg = 0;

	if (!x) {
		if (y > 0.0) 
			return(x);	/* (0 ** pos) == 0 */
		if ((y == 0.0) && (_lib_version != c_issue_4))
			return(1.0);
		return(pow_exc(x, y, DOM, 0));
	}
	if (y == 0.0)
		return 1.0;
	if (y == 1.0 || x == 1.0)
		return x;
	if (x < 0.0) {
		x = -x;
		if (y >= -MAXLONG && y <= MAXLONG) {
			k = (long) y;
			if ((double)k != y) /* y not integral */
				return(pow_exc(-x, y, DOM, 0));
			neg = k % 2;
		}
		else {
			if (!((t = fmod(y, 2.0)) == 0.0 || t == 1.0 || t == -1.0))
				return(pow_exc(-x, y, DOM, 0));
			if (t == 1.0 || t == -1.0)
				neg++; /* y is an odd integer */
		}
	}
	/* reduce x to z in [sqrt(1/2)-1, sqrt(2)-1] */
#if _IEEE
	/* inline expand logb() for efficiency */
	if ((n = EXPONENT(x)) == 0)
		n = -1022;
	else n -= 1023;
	if (n > -1022) {
	/* inline expand ldexp for efficiency */
		z = x;
		EXPONENT(z) -=n;
	}
	else {
		z = ldexp(x, -n);
		if ((m = EXPONENT(z)) == 0)
			m = -1022;
		else m -= 1023;
		n += m;
		z = ldexp(z, -m);
	}
#else
	z = ldexp(x, -(n = logb(x)));
#endif
	if (z >= sqrt2 ) {
		n += 1; 
		z *= 0.5;
	}  
	z -= 1.0;

	/* log(x) = nlog2+log(1+z) ~ nlog2 + t + tx */
	s = z / (2.0 + z); 
	c = z * z * 0.5; 
	t = s * s;
#if _IEEE
	tx = s * (c + _POLY7(t, p));
#else 
#if vax /* Vax D format */
	tx = s * (c + _POLY8(t, p));
#endif
#endif
	t = z - (c - tx); 
	tx += (z - t) - c;

	/* if y*log(x) is neither too big nor too small */
#if _IEEE
	/* expand logb inline for efficiency */
	z = n + t;
	if ((m = EXPONENT(z)) == 0)
		m = -1022;
	else m -= 1023;
	if ((r = EXPONENT(y)) == 0)
		r = -1022;
	else r -= 1023;
	if ((s = m + r) < 12.0)
#else
	if ((s = logb(y) + logb(n + t)) < 12.0)
#endif
		if (s > -60.0) {

			/* compute y*log(x) ~ mlog2 + t + c */
			s = y * (n + invln2 * t);
			m = s + (s < 0.0 ? -0.5 : 0.5);
			k = (long)y;
			/* m := nint(y*log(x)) */

			if ((double)k == y) {	/* if y is an integer */
				k = m - k * n;
				sx = t; 
				tx += (t - sx); 
			} else {		/* if y is not an integer */
				k = m;
				tx += n * ln2lo;
				sx = (c = n * ln2hi) + t; 
				tx += (c - sx) + t; 
			}
			/* end of checking whether k==y */
			sy = y; 
			ty = y - sy;          /* y ~ sy + ty */
			s = (double)sx * sy - k * ln2hi;        /* (sy+ty)*(sx+tx)-kln2 */
			z = (tx * ty - k * ln2lo);
			tx = tx * sy; 
			ty = sx * ty;
			t = ty + z; 
			t += tx; 
			t += s;
			c = -((((t - s) - tx) - ty) - z);

			/* return exp(y*log(x)) */
			t += _exp__E(t, c); 
			t = ldexp(1.0 + t, m);
			return(neg ? -t : t);
		}
	/* end of if log(y*log(x)) > -60.0 */

		else 
			return(neg ? -1.0 : 1.0);
	else if (((y < 0.0) ? -1.0 : 1.0) * (n + invln2 * t) < 0.0) {
		/* exp(-(big#)) underflows to zero */

		return(pow_exc(x, y, UNDER, 0));
	} else {
		/* exp(+(big#)) overflows to INF */
		return(pow_exc(x, y, OVER, neg));
	}

}

/* _exp__E(x,c)
 * ASSUMPTION: c << x  SO THAT  fl(x+c)=x.
 * (c is the correction term for x)
 * exp__E RETURNS
 *
 *			 /  exp(x+c) - 1 - x ,  1E-19 < |x| < .3465736
 *       exp__E(x,c) = 	| 		     
 *			 \  0 ,  |x| < 1E-19.
 *
 * Method:
 *	1. Rational approximation. Let r=x+c.
 *	   Based on
 *                                   2 * sinh(r/2)     
 *                exp(r) - 1 =   ----------------------   ,
 *                               cosh(r/2) - sinh(r/2)
 *	   exp__E(r) is computed using
 *                   x*x            (x/2)*W - ( Q - ( 2*P  + x*P ) )
 *                   --- + (c + x*[---------------------------------- + c ])
 *                    2                          1 - W
 * 	   where  P := _POLY2(x^2, p1)
 *	          Q := _POLY2(x^2, q1)
 *	          W := x/2-(Q-x*P),
 *
 *	   (See the listing below for the values of p1,q1. The poly-
 *	    nomials P and Q may be regarded as the approximations to sinh
 *	    and cosh :
 *		sinh(r/2) =  r/2 + r * P  ,  cosh(r/2) =  1 + Q . )
 *
 *         The coefficients were obtained by a special Remez algorithm.
 */

#define SMALL	1.0E-19

#if _IEEE
static double 
p1 = 1.3887401997267371720E-2,
p2 = 3.3044019718331897649E-5, 
q1 = 1.1110813732786649355E-1,
q2 = 9.9176615021572857300E-4;
#else  
#if vax /* Vax D format */
static double 
p1 =  1.5150724356786683059E-2, 
p2 =  6.3112487873718332688E-5,
q1 =  1.1363478204690669916E-1, 
q2 =  1.2624568129896839182E-3,
q3 =  1.5021856115869022674E-6;
#endif
#endif

static
double _exp__E(x,c)
double x, c;
{
	register double z, p3, q;
	double xp, xh, w;

	if (_ABS(x) > SMALL) {
           z = x * x;
	   p3 = z * (p1 + z * p2);
#if _IEEE
           q = z * (q1 + z * q2);
#else 
#if vax /* Vax D format */
           q = z * ( q1 + z * ( q2 + z * q3 ));
#endif
#endif
           xp= x * p3; 
	   xh= x * 0.5;
           w = xh - (q - xp);
	   p3 += p3;
	   c += x * ((xh * w - (q - (p3 + xp)))/(1.0 - w) + c);
	   return(z * 0.5 + c);
	}
	/* end of |x| > small */

	else 
	    return 0.0;
}

static double
pow_exc(x, y, etype, neg)
register double x, y;
register int etype, neg;
{
	struct exception exc;

	exc.arg1 = x;
	exc.arg2 = y;
	exc.name = "pow";
	switch(etype) {
	case DOM:	
			if ((x == 0.0) && (_lib_version != c_issue_4))
				exc.retval = -HUGE_VAL;
			else
				exc.retval = 0.0;
			exc.type = DOMAIN;
			break;
	case UNDER:	exc.type = UNDERFLOW;
			exc.retval = 0.0;
			break;
	case OVER:	exc.type = OVERFLOW;
			if (_lib_version == c_issue_4)
				exc.retval = (neg ? -HUGE: HUGE);
			else
				exc.retval = (neg ? -HUGE_VAL: HUGE_VAL);
			break;
	}
	if (_lib_version == strict_ansi) {
		if (etype == DOM) 
			errno = EDOM;
		else 
			errno = ERANGE;
	}
	else if (!matherr(&exc)) {
		if (etype == DOM) {
			errno = EDOM;
			if (_lib_version == c_issue_4)
				(void)write(2,"pow: DOMAIN error\n",18);
		}
		else errno = ERANGE;
	}
	return exc.retval;
}
