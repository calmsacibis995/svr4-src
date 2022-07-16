/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file   "bboot.s"

/	Copyright (c) 1988  Intel Corporation
/	All Rights Reserved

/	INTEL CORPORATION PROPRIETARY INFORMATION

/	This software is supplied to AT & T under the terms of a license
/	agreement with Intel Corporation and may not be copied nor
/	disclosed except in accordance with the terms of that agreement.

	.ident	"@(#)mb1:uts/i386/boot/mb1/bboot.s	1.3"

/ ********************************************************************
/
/	This is the boot strap loader for the UNIX-386 system
/
/	The bootstrap loader is in two parts.  Part one begins at 0
/	and goes through byte 383.  Part two begins at 1024 and goes 
/	through a maximum of 6K.  The actual end of .text and .data 
/	is figured out at run-time from the label end generated
/	automatically by ld.
/
/	NOTE: The first stage bootstrap is executing in real mode
/	(16-bit values) and the second stage starts of in real-mode
/	later switches to protected mode.  The interface between
/	the second stage and first stage is STILL in real-mode which
/	means 16-bit values on the stack.
/
/	NOTE: The second stage bootstrap loader is generated using
/	386SGS, which means the assembler generates code for 32-bit
/	mode only.  Until we switch to protected mode, this code will
/	executed in real-mode so we have to use the address and/or data
/	override prefixes where appropriate.
/
/ ********************************************************************
#include "../sys/prot.h"

BOLT_LOC	=	512			/ Location of the BOLT
SZBSTACK	=	0x400		/ size of boot-stack
VLAB_START	=	0x180
VLAB_NCYL	=	48			/ ushort
VLAB_NFHEAD	=	50			/ unchar
VLAB_NRHEAD	=	51			/ unchar
VLAB_NSEC	=	52			/ unchar
VLAB_SECSIZ	=	53			/ ushort (not aligned)
VLAB_NALT	=	55			/ unchar
VLAB_FSDOFF	=	56
SZRMLAB		=	0x80
BOOT_BREAK	=	0x180
BOOT_RESUME	=	0x400		/ resume at 1024

	.bss

/ *********************************************************************
/ The amazing piece of code below is a carefully designed kludge
/ to take care of a problem introduced by a new 1st stage loader which
/ stores parameters in magic places in the stack.
/ To make it happy, we will force the .data to be empty by not declaring
/ any data, and then we will reserve the first 272 bytes of bss.
/ *********************************************************************

	. = . + 272

	.globl	text_addr	/ the C part needs to know this value
	.globl	text_len	/ the C part needs to know this value
	.globl	data_len	/ the C part needs to know this value
	.globl	cyls		/ the C part needs to know this value	
	.globl	heads		/ the C part needs to know this value	
	.globl	sectors		/ the C part needs to know this value	
	.globl	alternates	/ the C part needs to knoe this value
	.globl	fsdelta		/ the C part needs to know this value	
	.globl	dev_gran	/ the C part needs to know this value
	.globl	err_addr
	.globl	err_check
	.globl	unit
	.globl	debug_ck	/ Changed identifier name

dev_gran:	
	. = . + 4			/ saves dev-gran from first stage

drv_addr:
	. = . + 4			/ saves driver entry from first stage

err_addr:
	. = . + 4

err_check:
	. = . + 4

debug_ck:				/ Changed identifier name
	. = . + 4

text_len:
	. = . + 4

text_addr:
	. = . + 4

data_len:
	. = . + 4

cyls:
	. = . + 2

heads:
	. = . + 2

sectors:
	. = . + 2

alternates:
	. = . + 2

fsdelta:
	. = . + 2

unit:
	. = . + 2

fname:
	. = . + 128

bstack:
	. = . + SZBSTACK

	.text

/ *********************************************************************
/
/	bootstrap
/
/	main entry point
/
/ *********************************************************************

/	parameter space

sec_size	=	0
aunit		=	2
prom_driver	=	4
boot_dev_name	=	8
boot_file_name	=	12
boot_error	=	16
error_check	=	20
debug_check	=	24
 
/	local data space

block_count	=	-4
buffer		=	-8
copy_count	=	-12
bytes_read	=	-16

	.globl	bootstrap

bootstrap:
	xchg	%eax,%eax	/ May be changed to be int $1  (0xcd01)
	xchg	%eax,%eax	/ (using adb)  These are the first two
				/ bytes on the disk.  (These two instructions
				/ are also known as no-ops)

	movw	%cs,%ax		/ initialise ds = cs (small model)
	movw	%ax,%ds
	movw	%ax,%es
	data16
	and	$0xffff, %ebp	/ zero out upper 16-bits
	data16
	mov	%ebp,%esp	/ a little strange ! but we're never
				/ going to return to what called us
	data16
	sub	$16,%esp	/ create space for locals

/
/	read in rest of this stage
/

	addr16
	mov	sec_size(%ebp),%eax
	addr16
	mov	%eax,buffer(%ebp)

	addr16				/ Start reading from block number 1.
	movl $1, block_count(%ebp)	/ Firmware has read block num 0.

	data16				/ Figure out number of bytes to
	mov	$end, %ecx		/ be read from boot media.
	data16
	addr16
	sub	sec_size(%ebp), %ecx
	addr16
	mov	%ecx, copy_count(%ebp)

	xor	%edx,%edx
	addr16
	mov	%edx, bytes_read(%ebp)		/ init count

read_in_loop:

	addr16
	push	aunit(%ebp)
	xor	%eax,%eax
	push	%eax
	addr16
	push	block_count(%ebp)
	movw	%cs, %ax
	push	%eax
	addr16
	push	buffer(%ebp)
	addr16
	lcall	*prom_driver(%ebp)	/ PL/M style function call
	addr16
	inc	block_count(%ebp)
	addr16
	mov	sec_size(%ebp),%eax
	addr16
	add	%eax,buffer(%ebp)
	addr16
	mov	sec_size(%ebp),%eax
	addr16
	add	%eax,bytes_read(%ebp)
	addr16
	mov	copy_count(%ebp), %ebx
	addr16
	cmp	%ebx,bytes_read(%ebp)
	jb	read_in_loop

/
/	save stuff from the label. Note that we are saving it on the stack.
/
	addr16
	push	[VLAB_START+VLAB_FSDOFF]	/ v_fsdelta
	addr16
	movzbw	[VLAB_START+VLAB_NALT],%ax	/ alternate
	addr16
	push	%eax
	addr16
	movzbw	[VLAB_START+VLAB_NSEC],%ax	/ sectors per track
	addr16
	push	%eax
	addr16
	movzbw	[VLAB_START+VLAB_NFHEAD],%ax/ fixed heads
	addr16
	push	%eax
	addr16
	push	[VLAB_START+VLAB_NCYL]		/ cylinders

/
/	save the relevant stuff from the BOLT on the stack too.
/
	data16
	addr16
	push	[BOLT_LOC+28]	/ bolt.data_size
	data16
	addr16
	push	[BOLT_LOC+36]	/ bolt.tbl_entries.byte_offset
	data16
	addr16
	push	[BOLT_LOC+40]	/ bolt.tbl_entries.length

/
/	squash up code (over where label got loaded)
/

	movw	%ds,%ax
	movw	%ax,%es
	addr16
	mov	$BOOT_RESUME, %esi
	data16
	mov	$BOOT_BREAK,%edi
	addr16

	/ figure out number of bytes to copy

	mov	copy_count(%ebp), %ecx
	addr16
	add sec_size(%ebp), %ecx
	data16
	sub $BOOT_RESUME, %ecx
	inc	%ecx
	shr	%ecx
	cld
	rep
	smov

/
/	save a copy of the file name and terminate it with NULL
/	Note that this must be done before we do all of the pops
/	so that the stack will be valid.
/	We push DS here so that it will be easily retrievable after
/	we finish mucking with the filename.

	push	%ds
	addr16
	lds	boot_file_name(%ebp),%esi
	movw	%cs,%ax
	movw	%ax,%es
	data16
	mov	$fname,%edi

getname:

	slodb
	data16
	cmpb	$0xd, %al		/ NULL/CR/NL or anything else low terminates
	jle	gotname
	sstob
	jmp	getname

gotname:

	xorb	%al,%al
	sstob
	pop	%ds

/
/	save the relevant stuff from the BOLT on the stack too

	data16
	addr16

	data16
	addr16
	pop	text_len	/ bolt.tbl_entries.length
	data16
	addr16
	pop	text_addr	/ bolt.tbl_entries.byte_offset
	data16
	addr16
	pop	data_len	/ bolt.data_size

/	before we play with the stack pointer we must retrieve
/	the volume label stuff that we pushed on the stack.
	addr16
	pop	cyls		/ cylinders
	addr16
	pop	heads		/ fixed heads
	addr16
	pop	sectors		/ sectors per track
	addr16
	pop	alternates	/ alternates
	addr16
	pop	fsdelta

/	since we have not changed ss and ebp, they still point
/	to the place on the stack where the first stage has left
/	us gobs of interesting things.

	mov	%ebp, %esp
	addr16
	pop	dev_gran		/ save dev-gran
	addr16
	movl	$0,dev_gran+2
	addr16
	pop	unit			/ save unit #
	addr16
	pop	drv_addr+0		/ driver entry offset
	addr16
	pop	drv_addr+2		/ driver entry segment
	pop	%eax			/ ignore boot device name
	pop	%eax
	pop	%eax			/ ignore file name (we already
	pop	%eax			/ moved it into our area)
	addr16
	pop	err_addr+0		/ boot error routine
	addr16
	pop	err_addr+2
	addr16
	pop	err_check+0		/ error routine check word
	addr16
	pop	err_check+2		/ error routine check word
	addr16
	pop	debug_ck+0		/* debug check word */
	addr16
	pop	debug_ck+2		/* debug check word */

/
/	Move to lowcore (MY_PADDR)
/
	xor		%edx, %edx	/ Destination offset 0
	mov		%edx, %edi
	mov		%edx, %esi	/ Source offset 0
	data16
	movl	$end, %ecx	/ For the size of the program
	data16
	movl	$MY_PADDR,%eax
	data16
	shr		$4,%eax		/ Compute real mode selector
	movw	%ax, %es	/ Set up destination selector
	rep
	smov

/
/	Begin running on a new stack in the new copy
/
	movw	%ax,%ds		/ Move data to new segment
	movw	%ax,%ss		/ Make stack same as data
	data16				/ And run on new stack
	movl	$bstack+SZBSTACK,%esp
	data16
	push	%eax		/ Make new cs same as ds and ss
	data16
	push	$newcopy	/ Be here now
	data16
	lret
newcopy:

/
/	call the bootstrap program
/
	data16
	call	goprot			/ go protected mode	
	push	$fname
	call	bload			/ bload(fname); never returns
