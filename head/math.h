/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _MATH_H
#define _MATH_H

#ident	"@(#)head:math.h	2.11.1.27"

#if __STDC__ - 0 == 0 && !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
struct exception {
	int type;
	char *name;
	double arg1;
	double arg2;
	double retval;
};

enum version { c_issue_4, ansi_1, strict_ansi };

#endif

#if defined(__STDC__)

extern double acos(double); 
extern double asin(double); 
extern double atan(double); 
extern double atan2(double, double); 
extern double cos(double); 
extern double sin(double); 
extern double tan(double); 

extern double cosh(double); 
extern double sinh(double); 
extern double tanh(double); 

extern double exp(double); 
extern double frexp(double, int *); 
extern double ldexp(double, int); 
extern double log(double); 
extern double log10(double); 
extern double modf(double, double *); 

extern double pow(double, double); 
extern double sqrt(double);

extern double ceil(double); 
extern double fabs(double); 
extern double floor(double); 
extern double fmod(double, double); 

extern float acosf(float); 
extern float asinf(float); 
extern float atanf(float); 
extern float atan2f(float, float); 
extern float cosf(float); 
extern float sinf(float); 
extern float tanf(float); 

extern float coshf(float); 
extern float sinhf(float); 
extern float tanhf(float); 

extern float expf(float); 
extern float logf(float); 
extern float log10f(float); 

extern float powf(float, float); 
extern float sqrtf(float);

extern float ceilf(float); 
extern float fabsf(float); 
extern float floorf(float); 
extern float fmodf(float, float); 
extern float modff(float, float *); 

#ifndef HUGE_VAL
#if #machine(pdp11) || #machine(vax)
#define HUGE_VAL	1.701411733192644299e+38

#elif #machine(gcos)
#define HUGE_VAL        1.7014118219281863150e+38

#else
typedef union _h_val {
  	unsigned long i[2];
	double d;
} _h_val;

extern const _h_val __huge_val;
#define HUGE_VAL __huge_val.d
#endif
#endif	/* HUGE_VAL */

#if (__STDC__ == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)

extern double erf(double); 
extern double erfc(double); 
extern double gamma(double);
extern double hypot(double, double);
extern double j0(double); 
extern double j1(double); 
extern double jn(int, double); 
extern double y0(double); 
extern double y1(double); 
extern double yn(int, double); 
extern double lgamma(double);
extern int isnan(double);

#if #machine(gcos)
#define MAXFLOAT        ((float)1.7014118219281863150e+38)

#else
#if #machine(pdp11) || #machine(vax)
#define MAXFLOAT        ((float)1.701411733192644299e+38)

#else   
#define MAXFLOAT        ((float)3.40282346638528860e+38)
#endif
#endif

#endif

#if __STDC__ == 0 && !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)

extern const enum version _lib_version;

#define HUGE		MAXFLOAT

extern double atof(const char *); 

/* new functions */
extern double scalb(double, double);
extern double logb(double);
extern double nextafter(double, double);
extern double acosh(double);
extern double asinh(double);
extern double atanh(double);
extern double cbrt(double);
extern double copysign(double, double);
extern double rint(double);
extern double remainder(double, double);
extern int unordered(double, double);
extern int finite(double);

extern int matherr(struct exception *);

#endif

#else	/* !defined(__STDC__) */

extern int errno;
extern enum version _lib_version;

extern double atof(), frexp(), ldexp(), modf();
extern double j0(), j1(), jn(), y0(), y1(), yn();
extern double erf(), erfc();
extern double exp(), log(), log10(), pow(), sqrt();
extern double floor(), ceil(), fmod(), fabs();
extern double gamma();
extern double hypot();
extern int matherr();
extern double sinh(), cosh(), tanh();
extern double sin(), cos(), tan(), asin(), acos(), atan(), atan2();
extern double scalb(), logb(), nextafter(), acosh(), asinh(), atanh();
extern double cbrt(), copysign(), lgamma(), rint(), remainder();
extern int unordered(), finite();
extern int isnan();

#if pdp11 || vax
#define MAXFLOAT        ((float)1.701411733192644299e+38)
#else
#if gcos
#define MAXFLOAT        ((float)1.7014118219281863150e+38)
#else
#define MAXFLOAT        ((float)3.40282346638528860e+38)
#endif
#endif

#define HUGE    MAXFLOAT

#ifndef HUGE_VAL
#if pdp11 || vax
#define HUGE_VAL	1.701411733192644299e+38

#elif gcos
#define HUGE_VAL        1.7014118219281863150e+38

#else
typedef union _h_val {
  	unsigned long i[2];
	double d;
} _h_val;

extern _h_val __huge_val;
#define HUGE_VAL __huge_val.d
#endif
#endif	/* HUGE_VAL */

#endif	/* __STDC__ */	

#if (__STDC__ - 0 == 0 && !defined(_POSIX_SOURCE)) || defined(_XOPEN_SOURCE)

extern int signgam;

/* some useful constants */
#define M_E		2.7182818284590452354
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		0.69314718055994530942
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440

#endif

#if __STDC__ - 0 == 0 && !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)

#define _ABS(x)		((x) < 0 ? -(x) : (x))

#define _REDUCE(TYPE, X, XN, C1, C2)	{ \
	double x1 = (double)(TYPE)X, x2 = X - x1; \
	X = x1 - (XN) * (C1); X += x2; X -= (XN) * (C2); }

#define DOMAIN		1
#define	SING		2
#define	OVERFLOW	3
#define	UNDERFLOW	4
#define	TLOSS		5
#define	PLOSS		6

#define _POLY1(x, c)	((c)[0] * (x) + (c)[1])
#define _POLY2(x, c)	(_POLY1((x), (c)) * (x) + (c)[2])
#define _POLY3(x, c)	(_POLY2((x), (c)) * (x) + (c)[3])
#define _POLY4(x, c)	(_POLY3((x), (c)) * (x) + (c)[4])
#define _POLY5(x, c)	(_POLY4((x), (c)) * (x) + (c)[5])
#define _POLY6(x, c)	(_POLY5((x), (c)) * (x) + (c)[6])
#define _POLY7(x, c)	(_POLY6((x), (c)) * (x) + (c)[7])
#define _POLY8(x, c)	(_POLY7((x), (c)) * (x) + (c)[8])
#define _POLY9(x, c)	(_POLY8((x), (c)) * (x) + (c)[9])

#endif

#endif /* _MATH_H */
