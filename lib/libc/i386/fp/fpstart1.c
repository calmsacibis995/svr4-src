/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:fp/fpstart1.c	1.2"

/* establish the default settings for the floating-point state
 * for a C language program:
 *	rounding mode		-- round to nearest default by OS,
 *	exceptions enabled	-- all masked
 *	sticky bits		-- all clear by default by OS.
 *      precision control       -- double extended
 * set the variable _fp_hw according to what floating-point hardware
 * is available.
 * set the variable __flt_rounds according to the rounding mode.
 */

#include	<sys/sysi86.h>	/* for SI86FPHW definition	*/
#include "synonyms.h"

long      			_fp_hw; /* default: bss: 0 == no hardware  */
int 				__flt_rounds;	/* ANSI rounding mode */

 void
_fpstart()
{
	extern int	sysi86(); /* avoid external refs */
	long	cw = 0;  /* coprocessor control word - used as -4(%ebp) */

	__flt_rounds = 1; /* ANSI rounding is rnd-to-near */

	(void)sysi86( SI86FPHW, &_fp_hw ); /* query OS for HW status*/
	_fp_hw &= 0xff;  /* mask off all but last byte */
	
	
	/* At this point the hardware environment (established by UNIX) is:
	 * round to nearest, all sticky bits clear,
    	 * divide-by-zero, overflow and invalid op exceptions enabled.
	 * Precision control is set to double.
	 * We will disable all exceptions and set precision control
 	 * to double extended.
	 */
	asm("	fstcw	-4(%ebp)");
	asm("	orl	$0x33f,-4(%ebp)");
	asm("	fldcw	-4(%ebp)");

	return;
}
