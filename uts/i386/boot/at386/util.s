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

	.file	"util.s"

	.ident	"@(#)boot:boot/at386/util.s	1.1.3.1"

#include "bsymvals.h"


/	----------------------------------------------------
/
/ 	In this wonderful bit of code, we change from protected to real
/  	mode, do the requested I/O into a buffer, return to real mode,
/	and finally copy the data to where it belongs in memory. 
/
/	Arguments are disk(int sector, char *paddr, short count )
/
/	The layout of the boot segment is as follows:
/			0x0000 to 0x5fff code, data, bss
/			0x6000 to 0x6fff disk buffer
/			0x7000 to 0x7fff stack
/	Thus, the maximum disk transfer size is 4k (0x1000) bytes.
/

	.globl	disk
disk:	
	push	%ebp
	mov	%esp, %ebp

	push	%es
	push	%esi
	push	%edi
	push	%ebx

	call	goreal
	sti

	addr16
	pushl	16(%ebp)		/ count of sectors to read
	data16
	mov	$DISKBUF, %eax
	push	%eax			/ offset of disk buffer
	push	%ds			/ segment of disk buffer
	addr16
	push	10(%ebp)		/ high word of sector number
	addr16
	push	8(%ebp)			/ low word of sector number

	data16
	call	_disk

	data16
	addl	$10, %esp		/ pop stack

	cli
	data16
	call	goprot

	xor	%eax, %eax		/ clear out %eax

	movw	16(%ebp), %ax		/ get sector count

	movl	$SECSIZE, %ebx
	mull	%ebx			/ calculate # bytes to move
	movl	%eax, %ecx

	movl	$DISKBUF, %esi		/ get source address

	movl	12(%ebp), %edi		/ get destination offset

	movl	$0x08, %ebx		/ 'flat' descriptor for physical
	movw	%bx, %es		/ addressing

	cld				/ set direction flag
	rep				
	smovb				/ copy %ecx bytes - %ds:%esi to %es:%edi

	pop	%ebx
	pop	%edi
	pop	%esi
	pop	%es

	pop	%ebp			/ restore frame pointer

	ret

/	----------------------------------------------------
/	protected mode putchar; character on stack.

	.globl	putchar
putchar:
	push	%ebp			/ save stack
	mov	%esp,%ebp
	pushl	%ebx

	call	goreal
	sti

	movb	$1, %bl
	addr16
	movb	8(%ebp), %al		
	movb	$14, %ah		/ teletype putchar
	int	$0x10			/ issue request

	cli
	data16
	call	goprot

	popl	%ebx
	pop	%ebp			/ restore frame pointer

	ret

/	----------------------------------------------------
/	protected mode getchar; C entry stack, character returned in %ax
	
	.globl	getchar

getchar:
	pushl	%ebp			/ save stack
	movl	%esp, %ebp
	pushl	%ebx

	call	goreal
	sti

	movb	$0, %ah			/ setup for bios read a char
	int	$0x16
	movb	$0, %ah			/ clear scancode
	movl	%eax, %edx		/ goprot trashes %ax, %bx

	cli
	data16
	call	goprot			

	movl	%edx, %eax		/ put result in %ax

	pop	%ebx
	pop	%ebp			/ restore frame pointer

	ret


/	----------------------------------------------------
/ Return TRUE if a character is waiting to be read.
/
	.globl	ischar

ischar:
	push	%ebp			/ C entry
	mov	%esp,%ebp

	push	%ebx
	push	%edi
	push	%esi

	call	goreal
	sti

	data16
	mov	$0, %edx		/ clear %ecx for result

	movb	$1, %ah			/ setup for bios test for a char
	int	$0x16			/ sets the zero flag if char is waiting

	jz	nochar			/ no char waiting

	data16
	mov	$1, %edx		/ char waiting: return TRUE

nochar:
	cli
	data16
	call	goprot	

	pop	%esi			/ C exit
	pop	%edi
	pop	%ebx

	pop	%ebp

	mov	%edx, %eax		/ setup return; goprot trashes %eax

	ret


/	----------------------------------------------------
/	return the value of DS, for use by physaddr().
/

	.globl	getDS
getDS:
	xorl	%eax, %eax		/ clean out %eax
	movw	%ds, %ax		/ return %ds
	ret

/	----------------------------------------------------
/	read the cmos database, entry from protected mode
/
	.globl	prdcmos
prdcmos:
	mov	4(%esp), %eax
	outb	$0x70
	inb	$0x71
	andl	$0xff, %eax
	ret

/	----------------------------------------------------
/	bhdparam(a, p)
/	Fill the hdparam struct pointed to by p, using the information
/	pointed to by the pointer at address real-mode address a.
/
	.globl	bhdparam

bhdparam:
	push	%ebp
	mov	%esp, %ebp

	push	%esi
	push	%es

	mov	$0x8, %ecx		/ set es to point to flat address space
	movw	%cx, %es

	mov	8(%ebp), %esi		/ physaddr of pointer to BIOS param blk.

	xorl	%eax, %eax		/ clean out %eax, %edx
	xorl	%edx, %edx

	movw	%es:(%esi), %dx		/ 16 bit real mode offset
	movw	%es:2(%esi), %ax	/ 16 bit real mode selector

	shl	$4, %eax
	addl	%edx, %eax		/ physaddr of the disk param block

	mov	12(%ebp), %esi		/ %ds relative addr. of binfo.hdparam

	movw	%es:HDBIOS_NCYL(%eax), %cx
	movw	%cx, hdp_ncyl(%esi)		/ number of cylinders

	movb	%es:HDBIOS_NHEAD(%eax), %cl
	movb	%cl, hdp_nhead(%esi)		/ number of heads

	movw	%es:HDBIOS_PRECOMP(%eax), %cx
	movw	%cx, hdp_precomp(%esi) 		/ precompression

	movw	%es:HDBIOS_LZ(%eax), %cx
	movw	%cx, hdp_lz(%esi)		/ landing zone

	movb	%es:HDBIOS_SPT(%eax), %cl
	movb	%cl, hdp_nsect(%esi)		/ sectors/track

	pop	%es
	pop	%esi

	leave
	ret


/	----------------------------------------------------
/	read the rom bios bits F000:ED00, entry from protected mode
/
	.globl  prdrom
prdrom:
	push    %ebp
	mov     %esp, %ebp

	push    %esi
	push    %es

	movl    $0x8, %eax              / set es to point to a 'flat' descriptor
	movw    %ax, %es

	movl    8(%ebp), %esi		/ Where in the ROM?
	movl    12(%ebp), %edi		/ Where to put the data
	movl    16(%ebp), %ecx		/ How many bytes?

jbu_dmm:
	movb	%es:(%esi), %al
	movb	%al, %es:(%edi)
	inc	%esi
	inc	%edi
	dec	%ecx
	cmpl	$0x0, %ecx
	jne	jbu_dmm

	pop     %es
	pop     %esi

	leave
	ret
