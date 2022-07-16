/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:sys/ieeefp.h	1.3.3.1"

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
 * Definitions for constants and types for IEEE floating point.
 */

#ifndef _sys_ieeefp_h
#define _sys_ieeefp_h


/*	Sun TYPES for IEEE floating point.	*/

#ifdef u3b2
enum fp_direction_type 		/* rounding direction */
	{
	fp_nearest	= 0,
	fp_tozero	= 1,
	fp_positive	= 2,
	fp_negative	= 3
	} ;
#endif
#ifdef sparc
enum fp_direction_type 		/* rounding direction */
	{
	fp_nearest	= 0,
	fp_tozero	= 1,
	fp_positive	= 2,
	fp_negative	= 3
	} ;
#endif
#ifdef i386
enum fp_direction_type 		/* rounding direction */
	{
	fp_nearest	= 0,
	fp_negative	= 1,
	fp_positive	= 2,
	fp_tozero	= 3
	} ;
#endif
#ifdef mc68000
enum fp_direction_type 		/* rounding direction */
	{
	fp_nearest	= 0,
	fp_tozero	= 1,
	fp_negative	= 2,
	fp_positive	= 3
	} ;
#endif

#ifdef i386
enum fp_precision_type		/* extended rounding precision */
	{
	fp_single	= 0,
	fp_precision_3	= 1,
	fp_double	= 2,
	fp_extended	= 3
	} ;
#else
enum fp_precision_type		/* extended rounding precision */
	{
	fp_extended	= 0,
	fp_single	= 1,
	fp_double	= 2,
	fp_precision_3	= 3
	} ;
#endif

#ifdef i386
enum fp_exception_type		/* exceptions according to bit number */
	{
	fp_invalid	= 0,
	fp_denormalized	= 1,
	fp_division	= 2,
	fp_overflow	= 3,
	fp_underflow	= 4,
	fp_inexact	= 5
	} ;
#else
enum fp_exception_type		/* exceptions according to bit number */
	{
	fp_inexact	= 0,
	fp_division	= 1,
	fp_underflow	= 2,
	fp_overflow	= 3,
	fp_invalid	= 4
	} ;
#endif

enum fp_class_type		/* floating-point classes */
	{
	fp_zero		= 0,
	fp_subnormal	= 1,
	fp_normal	= 2,
	fp_infinity   	= 3,
	fp_quiet	= 4,
	fp_signaling	= 5
	} ;

#endif /*!_sys_ieeefp_h*/
