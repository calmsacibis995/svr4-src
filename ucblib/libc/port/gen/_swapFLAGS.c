/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/_swapFLAGS.c	1.1.3.1"

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
 * _swapRD(rd) 	exchanges rd with the current rounding direction.
 * _swapRP(rp) 	exchanges rp with the current rounding precision.
 * _swapTE(ex)  exchanges ex with the current exception trap enable bits.
 * _swapEX(ex) 	exchanges ex with the current accrued exception. 
 *		Here ex is a five bit value where each bit (cf. enum 
 *		fp_exception_type in <sys/ieeefp.h>) corresponds 
 *		to either an exception-occurred accrued status flag or an
 *		exception trap enable flag (0 off,1 on):
 * 			bit 0 :  inexact flag
 * 			bit 1 :  division by zero flag
 * 			bit 2 :  underflow flag
 * 			bit 3 :  overflow flag
 * 			bit 4 :  invalid flag
 */

#include <math.h>

enum fp_direction_type _swapRD(rd)
enum fp_direction_type rd;
{
	/* 
	   swap rd with the current rounding direction
	 */
	return (enum fp_direction_type) -1;	/* not available */
}

enum fp_precision_type _swapRP(rp)
enum fp_precision_type rp;
{
	/* 
	   swap rp with the current rounding precision
	 */
	return (enum fp_precision_type) -1;	/* not available */
}

int _swapEX(ex)
int ex;
{
	/* 
	   swap ex with the current accrued exception mode
	 */
	return -1;	/* not available */
}

int _swapTE(ex)
int ex;
{
	/* 
	   swap ex with the current exception trap enable bits
	 */
	return -1;	/* not available */
}
