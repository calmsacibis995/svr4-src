/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.file	"compaq.s"

	.ident	"@(#)boot:boot/at386/initprog/compaq.s	1.1.2.1"

/ 	Initialization program for Compaq Deskpro386's.
/	Turn on the 20th addressing bit.
/ 	Turns on double mapped memory.
/
/	This is run in protected mode; by convention, the caller is
/	returned to through an lret.
/	
	.text

#include "../bsymvals.h"

	.globl	initprog

initprog:

/	Turn on A20 addressing

	mov	$0x100, %ecx		/ 'timeout' count for keyboard buffer

waitclear0:				/ wait for the keyboard buffer to clear
	inb	$KB_STAT
	testb 	$KB_INBF, %al
	loopne 	waitclear0

	cmpl	$0, %ecx
	je	a20done

	movb	$KB_WOP, %al		
	outb	$KB_STAT

	mov	$0x100, %ecx

waitclear1:
	inb	$KB_STAT
	testb 	$KB_INBF, %al
	loopne 	waitclear1

	cmpl	$0, %ecx
	je	a20done

	movb	$0xdf, %al		/ turn on the addressing
	outb	$KB_OUT

	mov	$0x100, %ecx

waitclear2:
	inb	$KB_STAT
	testb 	$KB_INBF, %al
	loopne 	waitclear2

a20done:

/ 	turn off Compaq double mapped memory

	push	%es			/ save %es

	mov	$0x8, %eax		/ set es to point to a 'flat' descriptor
	movw	%ax, %es
	mov 	$0x80c00000, %eax	/ touch the magic memory location
	movb	$0xff, %es:(%eax)

/	if we are running on a SuperVu or an EGA, we have to 
/	re-initialize the display BIOS, since, by default, it 
/	depends on the presence of shadow RAM

/	test for EGA/SuperVu

	mov	$0x14, %eax	/ get the CMOS equipment byte
	outb	$0x70
	inb	$0x71

	shr	$4, %eax
	and	$3, %eax
	cmp	$0, %eax	/ if this is zero, we have a EGA/SuperVu
	jne	done

	/ if the fonts are at c000, don't change anything
	mov	$0x10e, %eax
	cmpw	$0xc000, %es:(%eax)
	je	done

	/ Point int 0x10 at c000:0cd7
	mov	$0x42, %eax
	movw	$0xc000, %es:(%eax)	/ 0:42 <- 0xC000	

	/ Point ??? at c000:3560
	mov	$0x7e, %eax
	movw	$0xc000, %es:(%eax)	/ 0:7e <- 0xC000	

	/ Fonts are at c000:3160
	mov	$0x10e, %eax
	movw	$0xc000, %es:(%eax)	/ 0:10e <- 0xC000	
done:
	pop	%es			/ restore %es

	lret
