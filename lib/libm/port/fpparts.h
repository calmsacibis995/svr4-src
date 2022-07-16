/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/fpparts.h	1.2"

/* Macros to pull apart parts of single and  double precision
 * floating point numbers in IEEE format
 * Be sure to include /usr/include/values.h before including
 * this file to get the required definition of _IEEE
 */

#if _IEEE
#if M32 || u3b || u3b2 || u3b5 || u3b15
/* byte order with high order bits at lowest address */

/* double precision */
typedef  union {
	struct {
		unsigned  sign	:1;
		unsigned  exp	:11;
		unsigned  hi	:20;
		unsigned  lo	:32;
	} fparts;
	struct {
		unsigned  sign	:1;
		unsigned  exp	:11;
		unsigned  qnan_bit	:1;
		unsigned  hi	:19;
		unsigned  lo	:32;
	} nparts;
	struct {
		unsigned hi;
		unsigned lo;
	} fwords;
	double	d;
} _dval;

/* single precision */
typedef  union {
	struct {
		unsigned sign	:1;
		unsigned exp	:8;
		unsigned fract	:23;
	} fparts;
	struct {
		unsigned sign	:1;
		unsigned exp	:8;
		unsigned qnan_bit	:1;
		unsigned fract	:22;
	} nparts;
	unsigned long	fword;
	float	f;
} _fval;


#else
#if i386
/* byte order with low order bits at lowest address */

/* double precision */
typedef  union {
	struct {
		unsigned  lo	:32;
		unsigned  hi	:20;
		unsigned  exp	:11;
		unsigned  sign	:1;
	} fparts;
	struct {
		unsigned  lo	:32;
		unsigned  hi	:19;
		unsigned  qnan_bit	:1;
		unsigned  exp	:11;
		unsigned  sign	:1;
	} nparts;
	struct {
		unsigned  lo	:32;
		unsigned  hi	:32;
	} fwords;
	double	d;
} _dval;

/* single precision */
typedef  union {
	struct {
		unsigned fract	:23;
		unsigned exp	:8;
		unsigned sign	:1;
	} fparts;
	struct {
		unsigned fract	:22;
		unsigned qnan_bit	:1;
		unsigned exp	:8;
		unsigned sign	:1;
	} nparts;
	unsigned long	fword;
	float	f;
} _fval;
#else
#if i286
/* byte order with low order bits at lowest address
 * machines which cannot address 32 bits at a time
 */

/* double precision */
typedef  union {
	struct {
		unsigned  long lo;
		unsigned  hi1   :16;
		unsigned  hi2	:4;
		unsigned  exp	:11;
		unsigned  sign	:1;
	} fparts;
	struct {
		unsigned  long lo;
		unsigned  hi1	:16;
		unsigned  hi2	:3;
		unsigned  qnan_bit	:1;
		unsigned  exp	:11;
		unsigned  sign	:1;
	} nparts;
	struct {
		unsigned  long lo;
		unsigned  long hi;
	} fwords;
	double	d;
} _dval;

/* single precision */
typedef  union {
	struct {
		unsigned fract1	:16;
		unsigned fract2	:7;
		unsigned exp	:8;
		unsigned sign	:1;
	} fparts;
	struct {
		unsigned fract1	:16;
		unsigned fract2	:6;
		unsigned qnan_bit	:1;
		unsigned exp	:8;
		unsigned sign	:1;
	} nparts;
	unsigned long	fword;
	float	f;
} _fval;
#endif /* i286 */
#endif /* i386 */
#endif /* 3B  */

/* parts of a double precision floating point number */
#define	SIGNBIT(X)	(((_dval *)&(X))->fparts.sign)
#define EXPONENT(X)	(((_dval *)&(X))->fparts.exp)

#if M32 || u3b || u3b2 || u3b5 || u3b15 || i386
#define HIFRACTION(X)	(((_dval *)&(X))->fparts.hi)
#endif

#define LOFRACTION(X)	(((_dval *)&(X))->fparts.lo)
#define QNANBIT(X)	(((_dval *)&(X))->nparts.qnan_bit)
#define HIWORD(X)	(((_dval *)&(X))->fwords.hi)
#define LOWORD(X)	(((_dval *)&(X))->fwords.lo)

#define MAXEXP	0x7ff /* maximum exponent of double*/
#define ISMAXEXP(X)	((EXPONENT(X)) == MAXEXP)

/* macros used to create quiet NaNs as return values */
#define SETQNAN(X)	((((_dval *)&(X))->nparts.qnan_bit) = 0x1)
#define HIQNAN(X)	((HIWORD(X)) = 0x7ff80000)
#define LOQNAN(X)	((((_dval *)&(X))->fwords.lo) = 0x0)

/* macros used to extract parts of single precision values */
#define	FSIGNBIT(X)	(((_fval *)&(X))->fparts.sign)
#define FEXPONENT(X)	(((_fval *)&(X))->fparts.exp)

#if M32 || u3b || u3b2 || u3b5 || u3b15 || i386
#define FFRACTION(X)	(((_fval *)&(X))->fparts.fract)
#endif

#define FWORD(X)	(((_fval *)&(X))->fword)
#define FQNANBIT(X)	(((_fval *)&(X))->nparts.qnan_bit)
#define MAXEXPF	255 /* maximum exponent of single*/
#define FISMAXEXP(X)	((FEXPONENT(X)) == MAXEXPF)

#endif  /*IEEE*/
