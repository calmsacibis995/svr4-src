/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)boot:boot/sys/farcall.h	1.1.2.1"

#include "../sys/dib.h"		/* for POINTER type */
#include "../sys/prot.h"	/* for GDTSEL value */

#ifdef lint
/* ARGSUSED */
static int
inb(port)	/* Should be in inline.h */
ushort port;
{	return(0);
}

/* ARGSUSED */
static void
outb(port, byte)	/* Should be in inline.h */
ushort port;
char byte;
{
}

/* ARGSUSED */
static paddr_t
vtophys(sel, off)
ushort sel;
caddr_t off;
{	return(0);
}

/* ARGSUSED */
static void
far_store(sel, off, value)
ushort sel;
caddr_t off;
ulong value;
{
}

/* ARGSUSED */
static char
far_byte(sel, off)
ushort sel;
caddr_t off;
{	return(0);
}

/* ARGSUSED */
static long
far_long(sel, off)
ushort sel;
caddr_t off;
{	return(0);
}

static void
far_enter()
{
}

/* ARGSUSED */
static void
far_return(argbytes)
int argbytes;
{
}

/* ARGSUSED */
static ushort
get_ss()
{	return(0);
}
#else
asm ulong vtophys(sel, off)
{
%mem sel, off;
	push	%es				/ Save es
	push	%edi			/ Save edi
	movw	$GDTSEL, %ax	/ Get GDT alias
	movw	%ax, %es
	movzwl	sel, %edi		/ Get selector
	movl	%es:2(%edi),%eax/ Get base (24 bits)
	andl	$0xffffff, %eax
	addl	off, %eax		/ Add offset
	pop		%edi			/ Restore edi
	pop		%es				/ Restore es
}

asm void far_store(sel, off, value)
{
%mem sel, off, value;
	push	%es				/ Save es and di
	push	%edi
	movw	sel, %es		/ copy sel into es
	movl	off, %edi		/ get offset
	movl	value, %eax		/ get value
	movl	%eax,%es:(%edi)	/ store
	pop		%edi			/ Restore es and di
	pop		%es
}

asm char far_byte(sel, off)
{
%mem sel, off;
	push	%es				/ Save es and di
	push	%edi
	movw	sel, %es		/ copy sel into es
	movl	off, %edi		/ get offset
	subl	%eax, %eax		/ zero high bytes
	movb	%es:(%edi),%al	/ get low byte
	pop		%edi			/ Restore es and di
	pop		%es
}

asm long far_long(sel, off)
{
%mem sel, off;
	push	%es				/ Save es and di
	push	%edi
	movw	sel, %es		/ copy sel into es
	movl	off, %edi		/ get offset
	movl	%es:(%edi),%eax	/ get far long
	pop		%edi			/ Restore es and di
	pop		%es
}

asm void far_enter()
{
	push	%ds
	movw	$DATADESC, %ax
	movw	%ax, %ds
}

asm void far_return(argbytes)
{
%con argbytes;
	pop	%ds
	leave
	lret	argbytes
}

asm ushort get_ss()
{
	push	%ss
	popl	%eax
}
#endif
