/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:libc-i386/fp/fpsetround.c	1.6"

#include <ieeefp.h>
#include "fp.h"
#include "synonyms.h"

#ifdef __STDC__
	#pragma weak fpsetround = _fpsetround
#endif

fp_rnd
fpsetround(newrnd)
fp_rnd newrnd;
{
	struct _cw87 cw;
	fp_rnd oldrnd;
	extern int __flt_rounds;  /* ANSI value for rounding */

	newrnd &= 0x3;	/* mask off all ubt last 2 bits */
	switch(newrnd) {	/* set ANSI rounding mode */
	case FP_RN:	__flt_rounds = 1;
			break;
	case FP_RM:	__flt_rounds = 3;
			break;
	case FP_RP:	__flt_rounds = 2;
			break;
	case FP_RZ:	__flt_rounds = 0;
			break;
	};
	_getcw(&cw);
	oldrnd = (fp_rnd)cw.rnd;
	cw.rnd = newrnd;
	_putcw(cw);
	return oldrnd;
}
