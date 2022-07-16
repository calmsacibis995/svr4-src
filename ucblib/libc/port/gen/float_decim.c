/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/float_decim.c	1.1.3.1"

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
 * Conversion between single, and extended binary and decimal
 * floating point - separated from double_to_decimal to minimize impact on
 * main(){printf("Hello");}
 */

#include "base_conv.h"

void
single_to_decimal(px, pm, pd, ps)
	single         *px;
	decimal_mode   *pm;
	decimal_record *pd;
	fp_exception_field_type *ps;
{
	single_equivalence kluge;
	unpacked        u;

	*ps = 0;		/* Initialize *ps - no exceptions. */
	kluge.x = *px;
	pd->sign = kluge.f.msw.sign;
	pd->fpclass = _class_single(px);
	switch (pd->fpclass) {
	case fp_zero:
		break;
	case fp_infinity:
		break;
	case fp_quiet:
		break;
	case fp_signaling:
		break;
	default:
		_unpack_single(&u, &kluge.x);
		_unpacked_to_decimal(&u, pm, pd, ps);
	}
}

void
extended_to_decimal(px, pm, pd, ps)
	extended       *px;
	decimal_mode   *pm;
	decimal_record *pd;
	fp_exception_field_type *ps;
{
	extended_equivalence kluge;
	unpacked        u;

	*ps = 0;		/* Initialize *ps - no exceptions. */
	kluge.x[0] = (*px)[0];
	kluge.x[1] = (*px)[1];
	kluge.x[2] = (*px)[2];
	pd->sign = kluge.f.msw.sign;
	pd->fpclass = _class_extended(px);
	switch (pd->fpclass) {
	case fp_zero:
		break;
	case fp_infinity:
		break;
	case fp_quiet:
		break;
	case fp_signaling:
		break;
	default:
		_unpack_extended(&u, px);
		_unpacked_to_decimal(&u, pm, pd, ps);
	}
}
