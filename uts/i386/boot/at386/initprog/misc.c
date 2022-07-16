/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)boot:boot/at386/initprog/misc.c	1.1.4.1"

/* #include <sys/inline.h> */
#include "../bsymvals.h"


/*
 * attempt to turn on the 20th bit of address.
 * This is necessary in order to be able to access the
 * extended memory boards, if they exist.
 * The Gate A20 signal is an output of the 8042 slave processor.
 * return true if successful, 0 otherwise
 */

a20()
{
	flush8742();
	outb(KB_STAT, 0xd1);	/* 8042 command to write output port */
	flush8742();
	outb(KB_IDAT, 0xdf);	/* address line 20 gate on */
	flush8742();
	return 1;
}

/* 
 * Disable shadow RAM 
 */

shadow()
{
	unsigned	i;

	flush8742();
	clearout();
	outb(0x64, 0xa8);
	clearout();
	outb(0x64, 0x81);
	clearout();
	outb(0x60, 0x2);
	i = (in60() & ~0x40);
	flush8742();
	clearout();
	outb(0x64, 0xa8);
	clearout();
	outb(0x64, 0x80);
	clearout();
	outb(0x60, 0x2);
	clearout();
	outb(0x60, i);
}

flush8742()
{
	register int i, j;

	for (i = 0; i < 20; i++) {
		if (inb(0x64) & 0x1)
			inb(0x60);
		for (j = 0x5a; j > 0; j--)
			;
	}
}

clearout()
{
	register int i;

	for (i = 0xffff; i > 0; i--) {
		if (!(inb(0x64) & 0x2))
			break;
	}
}

in60(val)
unsigned val;
{
	register int i;

	for (i = 0xffff; i > 0; i--) {
		if (inb(0x64) & 0x1)
			break;
	}
	return(inb(0x60));
}
