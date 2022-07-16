/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FLOAT_H
#define _FLOAT_H

#ident	"@(#)head:float.h	1.7"

#if defined(__STDC__)

extern int __flt_rounds;
#define FLT_ROUNDS	__flt_rounds

#else

#define FLT_ROUNDS	1
#endif

#define FLT_RADIX       2
#define FLT_MANT_DIG    24
#define FLT_EPSILON     1.19209290E-07F
#define FLT_DIG         6
#define FLT_MIN_EXP     (-125)
#define FLT_MIN         1.17549435E-38F
#define FLT_MIN_10_EXP  (-37)
#define FLT_MAX_EXP     (+128)
#define FLT_MAX         3.40282347E+38F
#define FLT_MAX_10_EXP  (+38)

#define DBL_MANT_DIG    53
#define DBL_EPSILON     2.2204460492503131E-16
#define DBL_DIG         15
#define DBL_MIN_EXP     (-1021)
#define DBL_MIN         2.2250738585072014E-308
#define DBL_MIN_10_EXP  (-307)
#define DBL_MAX_EXP     (+1024)
#define DBL_MAX         1.7976931348623157E+308
#define DBL_MAX_10_EXP  (+308)

#define LDBL_MANT_DIG    53
#define LDBL_EPSILON     2.2204460492503131E-16
#define LDBL_DIG         15
#define LDBL_MIN_EXP     (-1021)
#define LDBL_MIN         2.2250738585072014E-308
#define LDBL_MIN_10_EXP  (-307)
#define LDBL_MAX_EXP     (+1024)
#define LDBL_MAX         1.7976931348623157E+308
#define LDBL_MAX_10_EXP  (+308)

#endif
