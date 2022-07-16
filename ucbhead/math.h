/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:math.h	1.2.3.1"

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
 * Math library definitions for all the public functions implemented in libm.a.
 */

#ifndef _math_h
#define _math_h

#include <fp.h>				/* Contains definitions for types and 
					 * functions implemented in libc.a.
					 */

/* 	4.3 BSD functions: math.h	4.6	9/11/85	*/

extern int    finite();
extern double fabs(), floor(), ceil(), rint();
extern double hypot();
extern double copysign();
extern double sqrt();
extern double modf(), frexp();
extern double asinh(), acosh(), atanh();
extern double erf(), erfc();
extern double exp(), expm1(), log(), log10(), log1p(), pow();
extern double lgamma();
extern double j0(), j1(), jn(), y0(), y1(), yn();
extern double sin(), cos(), tan(), asin(), acos(), atan(), atan2();
extern double sinh(), cosh(), tanh();
extern double cbrt();

/*      Sun definitions.        */
 
/* Implemented precisions for trigonometric argument reduction. */
enum fp_pi_type {
	fp_pi_infinite	= 0,	/* Infinite-precision approximation to pi. */
	fp_pi_66	= 1,	/* 66-bit approximation to pi. */
	fp_pi_53	= 2	/* 53-bit approximation to pi. */
};

/* Pi precision to use for trigonometric argument reduction. */
extern enum fp_pi_type fp_pi;

/*	Functions callable from C, intended to support IEEE arithmetic.	*/

extern enum fp_class_type fp_class();
extern int ilogb(), irint(), signbit();
extern int isinf(), isnan(), isnormal(), issubnormal(), iszero();
extern void ieee_retrospective(), standard_arithmetic(), nonstandard_arithmetic();
extern double nextafter(), remainder();
extern double logb(), significand(), scalb(), scalbn();
extern double min_subnormal(), max_subnormal();
extern double min_normal(), max_normal();
extern double infinity(), quiet_nan(), signaling_nan();
extern int ieee_flags ();
extern int ieee_handler ();

/*	Other functions for C programmers.	*/

extern double log2(), exp10(), exp2(), aint(), anint();
extern int nint();
extern void sincos(), sincospi();
extern double sinpi(), cospi(), tanpi(), asinpi(), acospi(), atanpi(), atan2pi();
extern double compound(), annuity();

/* 	Constants, variables, and functions from System V */

#define _ABS(x) ((x) < 0 ? -(x) : (x))

#ifndef HUGE
#if #machine(gcos)
#define HUGE		((float)1.7014118219281863150e+38)
#else
#if #machine(pdp11) || #machine(vax)
#define HUGE		((float)1.701411733192644299e+38) /* From BSD4.3 */
#else
#define HUGE		((float)3.40282346638528860e+38)
#endif
#endif
#endif

#define DOMAIN          1
#define SING            2
#define OVERFLOW        3
#define UNDERFLOW       4
#define TLOSS           5
#define PLOSS           6

struct exception {
        int type;
        char *name;
        double arg1;
        double arg2;
        double retval;
};

extern int signgam;

extern double fmod(), ldexp();
extern int matherr();

/* First three have to be defined exactly as in values.h including spacing! */

#define M_LN2	0.69314718055994530942
#define M_PI	3.14159265358979323846
#define M_SQRT2	1.41421356237309504880

#define M_E		2.7182818284590452354
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN10		2.30258509299404568402
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT1_2	0.70710678118654752440
#define _POLY1(x, c)    ((c)[0] * (x) + (c)[1])
#define _POLY2(x, c)    (_POLY1((x), (c)) * (x) + (c)[2])
#define _POLY3(x, c)    (_POLY2((x), (c)) * (x) + (c)[3])
#define _POLY4(x, c)    (_POLY3((x), (c)) * (x) + (c)[4])
#define _POLY5(x, c)    (_POLY4((x), (c)) * (x) + (c)[5])
#define _POLY6(x, c)    (_POLY5((x), (c)) * (x) + (c)[6])
#define _POLY7(x, c)    (_POLY6((x), (c)) * (x) + (c)[7])
#define _POLY8(x, c)    (_POLY7((x), (c)) * (x) + (c)[8])
#define _POLY9(x, c)    (_POLY8((x), (c)) * (x) + (c)[9])

/* 	
 *	Deprecated functions for compatibility with past.  
 *	Changes planned for future.
 */

extern double cabs();	/* Use double hypot(x,y)
			 * Traditional cabs usage is confused - 
			 * is its argument two doubles or one struct?
			 */
extern double drem();	/* Use double remainder(x,y)
			 * drem will disappear in a future release.
			 */
extern double gamma();	/* Use double lgamma(x)
			 * to compute log of gamma function.
			 * Name gamma is reserved for true gamma function
			 * to appear in a future release.
			 */

#endif /*!_math_h*/
