/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"wdbcopy.s"
	.ident	"@(#)wd:io/wdbcopy.s	1.1"
/
/
/
/	A bcopy routine to make sure that the source or the
/	destination is word alligned.
/
/	direction = 0 implies source must be word aligned	
/
/	wdbcopy (from,to,bytes,direction)
/	caddr_t	from,to;
/	int	bytes,direction;
/
/
/
	.set	FROMADR, 8
	.set	TOADR, 12
	.set	BCOUNT, 16
	.set	DIR, 20
	.set	TEMP_A, -4 
	.set	NBPW, 4
	.set	MASK, 0xFFFFFFFC


	.text
	.align	4
	.globl	wdbcopy

wdbcopy:
	pushl	%ebp
	movl	%esp, %ebp	/ setup stack frame
	subl	$4, %esp	/ save space for temp_a
	pushl	%ebx
	pushl	%esi
	pushl	%edi		/ save registers

allcopy:
	movl	BCOUNT(%ebp), %ebx	/ get count
	orl	%ebx, %ebx
	jz	bcpdone			/ count is zero
	movl	FROMADR(%ebp), %esi	/ get source address
	movl	TOADR(%ebp), %edi	/ get destination address
	movl	DIR(%ebp), %eax		/ get direction
	orl	%eax, %eax
	jz	salign

dalign:
	/ %esi = source address
	/ %edi = dest address

	testl	$NBPW-1, %edi	/ is destination aligned??
	/ do copy to work aligned destination
	jz	bcpalign

	/
	/ copy bytes until destination address is word aligned
	/
	movl	%edi, %ecx	/ get destination address
	andl	$NBPW-1, %ecx	/ get offset from word boundary

	movl	%edi, %eax	/ get destination address
	andl	$MASK, %eax	/ get word aligned address
	pushl	%eax		/ save destination address for later
	movl	(%eax), %eax	/ get word at this 
	movl	%eax, TEMP_A(%ebp) / address into memory
	movl	%ebp, %edi        / get address of word aligned memory
	addl	$TEMP_A, %edi
	addl	%ecx, %edi	/ add offset
	movl	$NBPW, %eax	/ figure number of bytes to move
	subl	%ecx, %eax
	movl	%eax, %ecx 
	cmpl	%ebx, %ecx	/ do we want to move this many bytes?
	jl	dmove
	movl	%ebx, %ecx	/ no, only copy original count

dmove:
	subl	%ecx, %ebx	/ decrement count of bytes to copy
	repz
	smovb			/ move odd bytes to TEMP_A(%ebp)

	popl	%edi
	movl	TEMP_A(%ebp), %eax	/ move aligned word
	movl	%eax, (%edi)		/ 	back to board
	addl	$4, %edi	/ set edi back up to the next word boundary

	orl	%ebx, %ebx	/ no bytes left?
	jz	bcpdone

	cmpl	$NBPW, %ebx	/ no words left?
	jl	dtail

	jmp	bcpalign

salign:
	/ %esi = source address
	/ %edi = dest address

	testl	$NBPW-1, %esi	/ is source aligned??
	/ do copy to work aligned source
	jz	bcpalign

	/
	/ copy bytes until source address is word aligned
	/
	movl	%esi, %ecx	/ get source address
	andl	$NBPW-1, %ecx	/ get offset from word boundary

	movl	%esi, %eax	/ get source address
	andl	$MASK, %eax	/ get word aligned address
	pushl	%eax		/ save source address for later
	movl	(%eax), %eax		/ get word at this
	movl	%eax, TEMP_A(%ebp)	/ address into memory
	movl	%ebp, %esi	/ get address of word aligned memory
	addl	$TEMP_A, %esi
	addl	%ecx, %esi	/ add offset
	movl	$NBPW, %eax	/ figure number of bytes to move
	subl	%ecx, %eax
	movl	%eax, %ecx 
	cmpl	%ebx, %ecx	/ do we want to move this many bytes?
	jl	smove
	movl	%ebx, %ecx	/ no, only copy original count

smove:
	subl	%ecx, %ebx	/ decrement count of bytes to copy
	repz
	smovb			/ move odd bytes to TEMP_A(%ebp)

	popl	%esi		/ set esi back up
	addl	$4, %esi	/ to the next word boundary

	orl	%ebx, %ebx	/ no bytes left?
	jz	bcpdone

	cmpl	$NBPW, %ebx	/ no words left?
	jl	stail

bcpalign:
	/ %esi = source address
	/ %edi = dest address
	/ %ebx = count of bytes to copy

	movl	%ebx, %ecx
	shrl	$2, %ecx	/ convert byte count to words
	andl	$NBPW-1, %ebx	/ %ebx gets remainder bytes to copy
	repz
	smovl			/copy words

	orl	%ebx, %ebx	/ no bytes left?
	jz	bcpdone

	movl	DIR(%ebp), %eax		/ get direction
	orl	%eax, %eax
	jz	stail

dtail:
	/ %esi = source address
	/ %edi = word aligned dest address
	/ %ebx = count of bytes to copy

	pushl	%edi
	movl	(%edi), %eax		/ get word at this
	movl	%eax, TEMP_A(%ebp)	/ 	address into memory
	movl	%ebp, %edi	/ get address of word aligned memory
	addl	$TEMP_A, %edi	/ get address of word aligned memory
	movl	%ebx, %ecx
	repz
	smovb			/ move leftover bytes to TEMP_A(%ebp)
	popl	%edi
	movl	TEMP_A(%ebp), %eax	/ move last word
	movl	%eax, (%edi)	/ 	back to board
	jmp	bcpdone

stail:
	/ %esi = word aligned source address
	/ %edi = dest address
	/ %ebx = count of bytes to copy

	movl	(%esi), %eax		/ get word at this
	movl	%eax, TEMP_A(%ebp)	/ 	address into memory
	movl	%ebp, %esi		/ get address of word aligned memory
	addl	$TEMP_A, %esi		/ get address of word aligned memory
	movl	%ebx, %ecx
	repz
	smovb			/ move leftover bytes from TEMP_A(%ebp)

bcpdone:
	popl	%edi
	popl	%esi
	popl	%ebx
	addl	$4, %esp	/ space for local temp_a
	popl	%ebp
	subl	%eax, %eax	/ return zero
	ret
