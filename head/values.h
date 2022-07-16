/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:values.h	1.33"

#ifndef BITSPERBYTE
/* These values work with any binary representation of integers
 * where the high-order bit contains the sign. */

/* a number used normally for size of a shift */
#if defined(__STDC__)

#if #machine(gcos)
#define BITSPERBYTE	9
#else
#define BITSPERBYTE	8
#endif

#else
#if gcos
#define BITSPERBYTE	9
#else
#define BITSPERBYTE	8
#endif

#endif 	/* __STDC__ */

#define BITS(type)	(BITSPERBYTE * (int)sizeof(type))

/* short, regular and long ints with only the high-order bit turned on */
#define HIBITS	((short)(1 << BITS(short) - 1))

#if defined(__STDC__)
#define HIBITI	(1U << BITS(int) - 1)
#define HIBITL	(1UL << BITS(long) - 1)

#else

#define HIBITI	((unsigned)1 << BITS(int) - 1)
#define HIBITL	(1L << BITS(long) - 1)
#endif

/* largest short, regular and long int */
#define MAXSHORT	((short)~HIBITS)
#define MAXINT	((int)(~HIBITI))
#define MAXLONG	((long)(~HIBITL))

/* various values that describe the binary floating-point representation
 * _EXPBASE	- the exponent base
 * DMAXEXP 	- the maximum exponent of a double (as returned by frexp())
 * FMAXEXP 	- the maximum exponent of a float  (as returned by frexp())
 * DMINEXP 	- the minimum exponent of a double (as returned by frexp())
 * FMINEXP 	- the minimum exponent of a float  (as returned by frexp())
 * MAXDOUBLE	- the largest double
			((_EXPBASE ** DMAXEXP) * (1 - (_EXPBASE ** -DSIGNIF)))
 * MAXFLOAT	- the largest float
			((_EXPBASE ** FMAXEXP) * (1 - (_EXPBASE ** -FSIGNIF)))
 * MINDOUBLE	- the smallest double (_EXPBASE ** (DMINEXP - 1))
 * MINFLOAT	- the smallest float (_EXPBASE ** (FMINEXP - 1))
 * DSIGNIF	- the number of significant bits in a double
 * FSIGNIF	- the number of significant bits in a float
 * DMAXPOWTWO	- the largest power of two exactly representable as a double
 * FMAXPOWTWO	- the largest power of two exactly representable as a float
 * _IEEE	- 1 if IEEE standard representation is used
 * _DEXPLEN	- the number of bits for the exponent of a double
 * _FEXPLEN	- the number of bits for the exponent of a float
 * _HIDDENBIT	- 1 if high-significance bit of mantissa is implicit
 * LN_MAXDOUBLE	- the natural log of the largest double  -- log(MAXDOUBLE)
 * LN_MINDOUBLE	- the natural log of the smallest double -- log(MINDOUBLE)
 * LN_MAXFLOAT	- the natural log of the largest float  -- log(MAXFLOAT)
 * LN_MINFLOAT	- the natural log of the smallest float -- log(MINFLOAT)
 */

#if defined(__STDC__)

#if #machine(u3b) || #machine(M32) || #machine(u3b15) || #machine(u3b5) || #machine(u3b2) || #machine(i386) || #machine(i286)
#define MAXDOUBLE	1.79769313486231570e+308
#define MAXFLOAT	((float)3.40282346638528860e+38)
#define MINDOUBLE	4.94065645841246544e-324
#define MINFLOAT	((float)1.40129846432481707e-45)
#define	_IEEE		1
#define _DEXPLEN	11
#define _HIDDENBIT	1
#define DMINEXP	(-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
#define FMINEXP	(-(FMAXEXP + FSIGNIF - _HIDDENBIT - 3))

#elif #machine(pdp11) || #machine(vax)
#define MAXDOUBLE	1.701411834604692293e+38
#define MAXFLOAT	((float)1.701411733192644299e+38)

/* The following is kludged because the PDP-11 compilers botch the simple form.
   The kludge causes the constant to be computed at run-time on the PDP-11,
   even though it is still "folded" at compile-time on the VAX. */

#define MINDOUBLE	(0.01 * 2.938735877055718770e-37)
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	1
#define DMINEXP	(-DMAXEXP)
#define FMINEXP	(-FMAXEXP)

#elif #machine(gcos)
#define MAXDOUBLE	1.7014118346046923171e+38
#define MAXFLOAT	((float)1.7014118219281863150e+38)
#define MINDOUBLE	2.9387358770557187699e-39
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	0
#define DMINEXP	(-(DMAXEXP + 1))
#define FMINEXP	(-(FMAXEXP + 1))
#endif

#if #machine(u370)
#define _LENBASE	4

#else
#define _LENBASE	1
#endif

#else

#if u3b || M32 || u3b15 || u3b5 || u3b2 || i286 || i386
#define MAXDOUBLE	1.79769313486231570e+308
#define MAXFLOAT	((float)3.40282346638528860e+38)
#define MINDOUBLE	4.94065645841246544e-324
#define MINFLOAT	((float)1.40129846432481707e-45)
#define	_IEEE		1
#define _DEXPLEN	11
#define _HIDDENBIT	1
#define DMINEXP	(-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
#define FMINEXP	(-(FMAXEXP + FSIGNIF - _HIDDENBIT - 3))
#endif

#if pdp11 || vax
#define MAXDOUBLE	1.701411834604692293e+38
#define MAXFLOAT	((float)1.701411733192644299e+38)

/* The following is kludged because the PDP-11 compilers botch the simple form.
   The kludge causes the constant to be computed at run-time on the PDP-11,
   even though it is still "folded" at compile-time on the VAX. */

#define MINDOUBLE	(0.01 * 2.938735877055718770e-37)
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	1
#define DMINEXP	(-DMAXEXP)
#define FMINEXP	(-FMAXEXP)
#endif

#if gcos
#define MAXDOUBLE	1.7014118346046923171e+38
#define MAXFLOAT	((float)1.7014118219281863150e+38)
#define MINDOUBLE	2.9387358770557187699e-39
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	0
#define DMINEXP	(-(DMAXEXP + 1))
#define FMINEXP	(-(FMAXEXP + 1))
#endif

#if u370
#define _LENBASE	4

#else
#define _LENBASE	1
#endif

#endif	/* __STDC__ */

#define _EXPBASE	(1 << _LENBASE)
#define _FEXPLEN	8
#define DSIGNIF	(BITS(double) - _DEXPLEN + _HIDDENBIT - 1)
#define FSIGNIF	(BITS(float)  - _FEXPLEN + _HIDDENBIT - 1)
#define DMAXPOWTWO	((double)(1L << BITS(long) - 2) * \
				(1L << DSIGNIF - BITS(long) + 1))
#define FMAXPOWTWO	((float)(1L << FSIGNIF - 1))
#define DMAXEXP	((1 << _DEXPLEN - 1) - 1 + _IEEE)
#define FMAXEXP	((1 << _FEXPLEN - 1) - 1 + _IEEE)
#define LN_MAXDOUBLE	(M_LN2 * DMAXEXP)
#define LN_MAXFLOAT	(float)(M_LN2 * FMAXEXP)
#define LN_MINDOUBLE	(M_LN2 * (DMINEXP - 1))
#define LN_MINFLOAT	(float)(M_LN2 * (FMINEXP - 1))
#define H_PREC	(DSIGNIF % 2 ? (1L << DSIGNIF/2) * M_SQRT2 : 1L << DSIGNIF/2)
#define FH_PREC	(float)(FSIGNIF % 2 ? (1L << FSIGNIF/2) * M_SQRT2 : 1L << FSIGNIF/2)
#define X_EPS	(1.0/H_PREC)
#define FX_EPS	(float)((float)1.0/FH_PREC)
#define X_PLOSS	((double)(long)(M_PI * H_PREC))
#define FX_PLOSS ((float)(long)(M_PI * FH_PREC))
#define X_TLOSS	(M_PI * DMAXPOWTWO)
#define FX_TLOSS (float)(M_PI * FMAXPOWTWO)
#define M_LN2	0.69314718055994530942
#define M_PI	3.14159265358979323846
#define M_SQRT2	1.41421356237309504880
#define MAXBEXP	DMAXEXP /* for backward compatibility */
#define MINBEXP	DMINEXP /* for backward compatibility */
#define MAXPOWTWO	DMAXPOWTWO /* for backward compatibility */

#endif	/* BITSPERBYTE */
