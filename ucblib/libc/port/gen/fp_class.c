/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/fp_class.c	1.1.3.1"

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

/* IEEE functions
 * 	fp_class()
 */

#include <math.h>
#include "libm.h"

enum fp_class_type fp_class(x)
double x;
{
static	double	_m_	= 1.0;
register	n0, n1;

	double w=x;
	long *pw = (long *) &w, k;

	if ((* (int *) &_m_) != 0) { n0=0;n1=1; }  /* not a i386 */
	else { n0=1;n1=0; }			   /* is a i386  */

	k = pw[n0]&0x7ff00000;
	if(k==0) { 
		k = (pw[n0]&0x7fffffff)|pw[n1];
		if (k==0) 	return fp_zero; 	/* 0 */
		else 		return fp_subnormal; 	/* 1 */
	}
	else if(k!=0x7ff00000) 	return fp_normal;	/* 2 */
	else {
		k=(pw[n0]&0x000fffff)|pw[n1];
		if(k==0) 	return fp_infinity; 	/* 3 */
		else if((pw[n0]&0x00080000)!=0) 
				return fp_quiet; 	/* 4 */
		else 		return fp_signaling;	/* 5 */
	}
}
