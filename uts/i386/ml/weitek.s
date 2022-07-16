/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.


	.ident	"@(#)kern-ml:weitek.s	1.3"

#ifdef WEITEK
	.set	ARG1,		8
	.set	WEITEK_VADDR,	0xffc00000	/ Virtual address of Weitek
	.set	WEITEK_STOR,	0xffc00c04	/ store r1
	.set	WEITEK_LOAD,	0xffc00404	/ load r1
	.set	WEITEK_LDCTX,	0xffc0c000	/ load context register
	.set	WEITEK_STCTX,	0xffc0c400	/ store context register
	.set	WEITEK_LDCTX_O,	0x0000c000
	.set	WEITEK_STCTX_O,	0x0000c400
	.set	CLEAR_AE,	0xFFFFFF00	/ Accumulated Exception Byte
	.set	WFPAE,		0x000000FD	/ Accum. Exception bits mask
	.set	WFPAEEM_SHFT,	16		/ shift to move accum.
						/ exception byte to exception
						/ mask
	.set	NOT_PCR_INTR,	0xFFFFFFFD	/ Everything except interrupt
	.set	WEITEK_20MHz,	0x00008000	/ Bit which is set to indicate
						/ 20 MHz 1163 chip
	.set	NUMREG,		31		/ Number of registers to save

/ Save the Weitek context register, and the general registers in the specified
/ register save area.

	.align	4
	.globl	save_weitek
save_weitek:
	pushl	%ebp				/ Save the registers.
	movl	%esp, %ebp
	pushl	%esi
	pushl	%edi

	movl	ARG1(%ebp), %edi		/ Get register save area.

	movl	WEITEK_STCTX, %eax		/ Save the context register
	movl	%eax, (%edi)			/ first, to quiesce the 
	addl	$4, %edi			/ Weitek chip.

	movl	$WEITEK_STOR, %esi		/ Save the Weitek registers.
	movl	$NUMREG, %ecx
	cld
	rep
	smovl

	popl	%edi				/ Restore the registers.
	popl	%esi
	popl	%ebp
	ret

/ Restore the context register, and the general registers from the specified 
/ register save area.

	.align	4
	.globl	restore_weitek
restore_weitek:
	pushl	%ebp				/ Save the registers.
	movl	%esp, %ebp
	pushl	%esi
	pushl	%edi

	call	init_wtimers			/ Re-initialize timers

	movl	ARG1(%ebp),	%esi		/ Get the register save area.

	movl	(%esi),	%eax			/ Restore the context register.
	movl	%eax, WEITEK_LDCTX
	addl	$4,	%esi

	movl	$WEITEK_LOAD,	%edi		/ Restore the Weitek registers.
	movl	$NUMREG,	%ecx
	cld
	rep
	smovl

	popl	%edi				/ Restore the registers.
	popl	%esi
	popl	%ebp
	ret

/ Initialize the Weitek chip.

	.align	4
	.globl	weitek_init
weitek_init:
	call	init_wtimers

	movl 	weitek_cfg, %eax		/ Rounding modes and Exception
	movl 	%eax, WEITEK_LDCTX		/ enables.
	ret

/ Reset the Accumulated Exception Byte, by reading the context register (to
/ quiesce the Weitek processor) and setting the exception mask bits to be
/ the bits set in the accumulated exception byte.  This turns off all of the
/ pending interrupts.

	.align	4
	.globl	reset_weitek_intr
reset_weitek_intr:
	pushl	%edi
	movl	$WEITEK_VADDR, %edi
	movl	WEITEK_STCTX_O(%edi), %eax	/ Read context register to
						/ quiesce the chip.
	andl	$WFPAE, %eax			/ Select the accumulated
						/ exception bits only
	shll	$WFPAEEM_SHFT, %eax		/ Move this bits to the
						/ exception mask.
	orl	WEITEK_STCTX_O(%edi), %eax	/ Disable the interrupts
	andl	$NOT_PCR_INTR, %eax		/ Turn off interrupt bit

	movl	%eax, WEITEK_LDCTX_O(%edi)	/ Set context register
	popl	%edi
	ret

/ This resets the Weitek processor timers, which is necessary for both
/ power-up and restore context operations.

	.align	4
init_wtimers:
	movl	$0xB8000000, WEITEK_LDCTX
	movl	WEITEK_STCTX, %eax		/ Check for 20 MHz 1163
	andl	$WEITEK_20MHz, %eax
	jnz	init_20MHz
	movl 	$0x16000000, WEITEK_LDCTX	/ 16 MHz 1164/1165 flowthrough
						/ timer
	jmp	init_wt1

	.align	4
init_20MHz:
	movl	$0x56000000, WEITEK_LDCTX	/ 20 MHz 1164/1165 flowthrough
	movl	$0x98000000, WEITEK_LDCTX	/ timer
	
init_wt1:
	movl 	$0x64000000, WEITEK_LDCTX	/ 1164 accumulate timer
	movl 	$0xA0000000, WEITEK_LDCTX	/ 1165 accumulate timer
	movl 	$0x30000000, WEITEK_LDCTX	/ Reserved mode bits (set to 0).
	ret
#endif
