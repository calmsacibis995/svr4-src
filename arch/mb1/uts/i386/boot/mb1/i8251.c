/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ident	"@(#)mb1:uts/i386/boot/mb1/i8251.c	1.3"

/*
 *
 * This set of procedures implements the console device driver.
 * Procedures provided:
 *      i8251co     --  prints directly onto the console for kernel putchar
 *      i8251ci     --  reads a character directly from the console
 *
 *  These routine implement the 8251 USART/PIT device driver.  This is
 *  intended to provide full support for the 8251
 *  console driver.
 */

#include "../sys/boot.h"
#include "../sys/farcall.h"
#include "sys/i8251.h"

/*
 * This list gives the board-base I/O address for the 8251 controller.
 */
#define I8251_DATA 0xD8
#define I8251_CTRL 0xDA

/*
 *	Kernel look-alikes
*/
asm char inb(port)
{
%mem port;
	subl	%eax, %eax
	movw	port, %dx
	inb		(%dx)
}

asm void outb(port, value)
{
%mem port, value;
	movw	port, %edx
	movb	value, %al
	outb	(%dx)
}


/*
 * This procedure provides the console out routine for the
 * kernel's putchar (printf) routine.  It merely polls the
 * USART until the TX Ready bit is set and then outputs the character.
 *
 * Note that for debugging purposes, there is an initialization
 * sequence hardcoded into this routine.  This will have to remain
 * until the true driver runs correctly.
 */
i8251co(c)
char	c;
{
	/* Wait until the USART thinks that it is ready.        */
	while(!(inb(I8251_CTRL) & S_TXRDY))
		tenmicrosec();    /* Don't hit the USART too hard */
	/* We got it, output the character. */
	outb(I8251_DATA, c);
	/* Add carriage return to newline */
	if (c == '\n')
		i8251co('\r');
}

/*
 * This procedure reads a character from the console.
 * It returns -1 if there is no character in the USART receive buffer.
 * Note that interrupts must be disabled so that i8251intr doesn't
 * get the character first.
 *
 */
i8251ci()
{
	/* Return -1 if we don't have receive ready */
	if (!(inb(I8251_CTRL) & S_RXRDY)) {
		tenmicrosec();    /* Don't hit the USART too hard */
		return -1;
	}
	/* Read the character */
	return inb(I8251_DATA);
}

/* ARGSUSED */
void
cmessage(msg, sev, stage)
POINTER msg;
ulong sev;
ulong stage;
{	char c;

	while (1) {
		c = far_byte(msg.sel, msg.offset);
		if (c == 0)
			break;
		i8251co(c);
		msg.offset++;
	}
	i8251co('\n');
	if (sev == 0)
		return;
	while (1)
		asm("int $3");
}

#ifdef lint
/*
 *	Reference each routine that the boot loader does, to keep lint happy.
*/
main()
{
	i8251co('x');
	return(0);
}
#endif /*lint*/
