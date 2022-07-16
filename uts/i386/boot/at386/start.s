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

	.file	"start.s"

	.ident	"@(#)boot:boot/at386/start.s	1.1.3.2"

/ 	UNIX V.3/386 bootstrap

#include "bsymvals.h"


/	This is ground zero, the initial point where control is transfered
/	by the master boot loader.
/ 	Here we are running where we are originally loaded:
/	cs = 0x0, ip = 0x7c00.

	.text	

ZERO:
firststage:
	cli

/	A tremendous kludge, here; we hardcode "ljmp 0x7c0:restart" in
/	order to set us up in the new segment.

	.byte	0xea
	.value	0x6
	.value	0x7c0

/	Now we are running at 0x7c0:0 

restart:
	movw	%cs, %ax
	movw	%ax, %es
	movw	%ax, %ds
	movw	%ax, %ss	/ set up stack
	data16			/ this indicates a "true" 32 bit operation
	movl	$STACK, %esp
	sti

	addr16			
 	movw	destseg, %ss		/ set stack at top of memory
/	data16
/	mov	$STACK, %esp

	push	%ss			/ save for lret

	data16
	call	readboot		/ read/relocate boot

	.byte	0x8d
	.byte	0x06
	.byte	0x00
	.byte	0x02			/ lea secondstage, %ax

	push	%eax

	lret				/ jump to second stage

	/ does not return


/	----------------------------------------------------
readboot:

/ 	Get the hard drive parameters; use int 13 function 8 to do this

	movb	$8, %ah			/ Return Drive Parameters function
	movb	$0x80, %dl		/ for Hard Drive 0

	int	$0x13			/ BIOS disk support

	addr16
	mov	%ecx, hd0parm		/ Save parameters in table
	addr16
	mov	%edx, hd0parm+2

	/ As a result of the BIOS call, the following parameters are now loaded:
	/   CL = max_sect  CH = max_cyl  DL = n_drives  DH = max_head

#ifdef WINI
	data16
	movzbl	%cl, %eax		/ # sectors per track
	andb	$0x3F, %al

	addr16		
	data16	
	movl	%eax, spt

	data16
	movzbl	%dh, %eax		/ # tracks per cylinder
	incl	%eax

	addr16
	mul	spt			/   	* # sectors per track
	addr16				/ 32 bit moff
	data16				/ 32 bits of data
	movl	%eax, spc		/   	= # sectors per cylinder

/ 	Find the active partition

	data16
	movl	$1, %ecx		/ sector = partition table
	data16
	movl	$0x201, %eax		/ read 1 sector

	addr16				/ 32 bit moff
	movw	destseg, %es		/ destination segment

	addr16				/ 32 bit moff
	data16				/ move 32 bits of data
	movl	destoff, %ebx		/ destination offset

	data16
	movl	$BOOTDRIVE, %edx	/ from main wini drive 0x80

	int	$0x13			/ BIOS disk support

	jc	ioerr

	data16
	movl	$FD_NUMPART, %ecx
	data16
	addl	$BOOTSZ, %ebx

ostry:
	addr16
	movb    %es:BOOTIND(%ebx), %al
	cmpb	$ACTIVE, %al
	je	osfound

	data16
	addl	$16, %ebx

	loop	ostry

	data16
	movl	$nopart, %esi		/ no active partition found
	data16
	jmp	fatal

osfound:
	addr16
	data16
	movl	%es:RELSECT(%ebx), %eax	/ save relative sector number
	data16
	addr16
	movl	%eax, unix_start
#else	/* WINI */

/	if we are supporting 3.5 inch drives, we must go through
/	the diskette parameter table to discover the # of sectors/track.
/	Recall that, for a diskette, sectors/cyl = 2 * sectors/track.

	data16
	pusha

	data16
	xorl	%eax, %eax		/ clear out %eax
	movb	$0x8, %ah		/ subfunction 8
	movb	$0, %dl			/ drive 0

	int	$0x13
	jc	typecheck_failed	/ assume 15 sec per track if fails
	
	data16
	and	$0x3f, %ecx		/ low 6 bits of %ecx are spt

	addr16
	data16
	movl	%ecx, spt

typecheck_failed:
	addr16
	data16
	movl	spt, %ecx

	addr16
	data16
	movl	numsec, %eax		/ set numsec to do one track at a time

	addr16
	data16
	movl	%ecx, numsec

	data16
	subl	%ecx, %eax

	addr16
	data16
	movl	%eax, extrasec		/ leftover sectors on second track

	shl	$1, %ecx  		/ multiply spt by 2 to get spc

	addr16
	data16
	movl	%ecx, spc

	data16 
	popa
#endif /* WINI */

/ 	call the BIOS to read the remainder of the bootstrap from disk

doio:
	addr16
	mov	numsec, %ebx
	push	%ebx			/ sector count: 16 bits

	addr16
	mov	destoff, %ebx
	push	%ebx			/ destination offset: 16 bits

	addr16
	data16
	mov	destseg, %ebx
	push	%ebx			/ destination segment: 16 bits

	addr16
	data16
	mov	unix_start, %ebx
	data16
	push	%ebx			/ relative sector number: 32 bits

	data16
	call	_disk			/ do the i/o

	data16
	addl	$10, %esp		/ restore the stack

#ifndef WINI
	addr16
	push	extrasec		/ extra sectors on second track

	data16
	xorl	%edx, %edx		/ clear %edx for multiplies

	addr16
	data16
	mov	numsec, %eax		/ calculate new dest. offset

	data16
	mov	$SECSIZE, %ebx		/ multiply by dev_gran

	data16
	mul	%ebx	

	addr16
	data16
	add	destoff, %eax		/ add to original offset

	push	%eax			/ destination offset: 16 bits

	addr16
	data16
	mov	destseg, %ebx
	push	%ebx			/ destination segment: 16 bits

	addr16
	data16
	mov	unix_start, %ebx

	addr16
	data16
	addl	numsec, %ebx

	data16
	push	%ebx			/ relative sector number: 32 bits

	data16
	call	_disk			/ do the i/o

	data16
	addl	$10, %esp		/ restore the stack
#endif

	data16
	ret				/ return to caller

#ifndef WINI

/	Leave space at offset %ss:256, since the floppy BIOS
/	uses it as scratch.
/
fd_zero:
/	. = ZERO + 256
	.align	256
	.byte 0, 0, 0, 0
#endif

ioerr:
	data16
	movl	$readerr, %esi
	data16
	jmp	fatal



/	----------------------------------------------------
/ 	_puts:		put null-terminated string at si to console
/
	.globl	_puts
_puts:
	data16
	pushl	%esi

	movb	$1, %bl		/ normal attribute
ploop:
	cld

	addr16
	lodsb			/ get next msg byte

	addr16			/ chip bug workaround
	nop			/ errata 7

	orb	%al,%al
	jz	pend		/ end of msg if NUL

	movb	$14, %ah	/ teletype putchar
	int	$0x10		/ issue request

	jmp	ploop

pend:
	data16
	popl	%esi

	data16
	ret			

/	----------------------------------------------------
/	_disk(secno, seg, offset, count)
/
/	Makes bios calls to read in one sector at a time to 
/	the segment:offset destination.  secno is 0 for the
/	first sector on the disk.  count is the number of
/	sectors to read.
/
	.globl	_disk
_disk:	
	data16
	push	%ebp			
	data16
	mov	%esp, %ebp		/ C-entry save stack frame

	push	%es

	data16
	push	%ebx
	data16
	push	%esi			/ save registers
	data16
	push	%edi

retry:
	addr16
	mov	8(%ebp),%eax		/ secno (%ax,dx) = rel sector number
	addr16
	mov	10(%ebp),%edx
	addr16
	div	spc			/ cyl (%cx) = secno / spc
	mov	%eax, %ecx			/   temp (%dx) = secno % spc
	mov	%edx, %eax			/ head (%al) = temp / spt
	addr16
	divb	spt			/   sector (%ah) = temp % spt
	xchgb	%ch,%cl			/ low cylinder bits in %ch
	rorb	$1,%cl			/ high cyl bits in top 2 bits of %cl
	rorb	$1,%cl
	orb	%ah,%cl			/ sector in rest of %cl
	incb	%cl			/ convert to 1-based
	movb	%al,%dh			/ head

	addr16
	movw	12(%ebp), %es		/ segment
	addr16
	mov	14(%ebp), %ebx		/ starting offset
	addr16
	movb	16(%ebp), %al		/ number of sectors to read
	movb	$2, %ah			/ function code for reading sectors
	movb	$BOOTDRIVE, %dl		/ from which drive 

	int	$0x13			/ BIOS disk support

	jnb	okread			/ retry if error

	movb	$0, %ah			/ reset controller
	int	$0x13
	jmp	retry

okread:
	data16
	pop	%edi			/ restore registers
	data16
	pop	%esi
	data16
	pop	%ebx

	pop	%es

	data16
	pop	%ebp

	data16
	ret				/ return

/	----------------------------------------------------
/	Routine to read the requested byte (in %al) from the CMOS ram, 
/	return result in %al

rdcmos:
	xorb	%ah, %ah
 	outb	$0x70
 	inb	$0x71

	data16
	ret

/	----------------------------------------------------
/	halt()
/
/	Stop everything, an error occured.
/

fatal:
	data16
	call	_puts			/ print error message
					/ fall through to...
	.align	4
	.globl	halt

halt:	cli				/ allow int's
/	hlt				/ stop.
	jmp	halt			/ STOP.

readerr:	.string		"boot: Error reading bootstrap\r\n"
#ifdef WINI
nopart:		.string		"boot: No active partition on hard disk\r\n"
#endif

	.globl	destseg			/ for use by goreal()

destseg:	.long	0x100		/ put bootstrap at 4K
destoff:	.long	0

numsec:		.long	29
#ifndef WINI
extrasec:	.long	0
#endif

/	Raw disk parameters from BIOS

	.globl	hd0parm
	.globl	hd0maxsec
	.globl	hd0maxcyl
	.globl	hd0ndisk
	.globl	hd0maxhd
	.globl	hd1parm
	.globl	hd1maxsec
	.globl	hd1maxcyl
	.globl	hd1ndisk
	.globl	hd1maxhd

hd0parm:
hd0maxsec:	.byte	0
hd0maxcyl:	.byte	0
hd0ndisk:	.byte	0
hd0maxhd:	.byte	0

hd1parm:
hd1maxsec:	.byte	0
hd1maxcyl:	.byte	0
hd1ndisk:	.byte	0
hd1maxhd:	.byte	0

/ 	The following are used in the high level disk driver

	.globl	spt
	.globl	spc
	.globl	dev_gran
	.globl	unix_start

spt:		.long	15
spc:		.long	15\*2
dev_gran:	.long	SECSIZE
unix_start:	.long	0

/ 	The code immediately below tags this as a "boot block" for DOS
/ 	master boot block loader

/  ------------------------------------------------------------------
/
/	The following '.align' directives do not work with the
/	i386 assembler, load i6.
/
/	As a short-term workaround (i.e., until we get a fixed
/	assembler from CPLU) we will use the code that follows.
/
/  ----------------- <begin commented-out section> -------------------
/#ifndef WINI
/	.align	510
/#else
/	.align	506
/#endif
/  ----------------- < end  commented-out section> -------------------
/
/  ----------------- < begin replacement code > ----------------------
#ifndef	WINI
	. = ZERO + 510
#else
	. = ZERO + 506
#endif
/  ----------------- < end  replacement code > -----------------------
	.byte 0x55
	.byte 0xaa

/	----------------------------------------------------

/	This is the second stage of the bootstrap, where we jump once
/	the bootstrap has been complete read into memory and relocated.
/	The code above guarentees that 'secondstage' is at offset 0x200.

secondstage:

/	need to copy variables from first stage that were changed

	int	$0x12			/ BIOS memory size call
	addr16
	data16
	mov	%eax, %cs:memsz		/ already relocated, do not add cs
	addr16
	data16				/ move 32 bits of data
	mov	hd0parm, %eax		/ copy hd0parm into relocated data
	addr16
	data16				/ move 32 bits of data
	mov	%eax, %cs:hd0parm
	addr16
	data16				/ move 32 bits of data
	mov	spt, %eax		/ copy spt into relocated data
	addr16
	data16				/ move 32 bits of data
	mov	%eax, %cs:spt
	addr16
	data16				/ move 32 bits of data
	mov	spc, %eax		/ copy spc into relocated data
	addr16
	data16				/ move 32 bits of data
	mov	%eax, %cs:spc
	addr16
	data16				/ move 32 bits of data
	mov	dev_gran, %eax		/ copy dev_gran into relocated data
	addr16
	data16				/ move 32 bits of data
	mov	%eax, %cs:dev_gran	
	addr16
	data16				/ move 32 bits of data
	mov	destseg, %eax		/ copy spt into relocated data
	addr16
	data16				/ move 32 bits of data
	mov	%eax, %cs:destseg

	addr16
	data16				/ move 32 bits of data
	mov	unix_start, %eax	/ copy unix_start into relocated data
	addr16
	data16				/ move 32 bits of data
	mov	%eax, %cs:unix_start

/	set up the segment registers

	movw	%cs, %ax			/ Want CS = DS = ES = SS
	movw	%ax, %ds			
	movw	%ax, %es			
 	movw	%ax, %ss

	data16
	mov	$STACK, %esp

#ifdef DEBUG
	data16
	mov	$banner2, %esi
	data16
	call	_puts
#endif

/	get hard disk parameters for drive 1 to pass to the kernel

	movb	$8, %ah			/ Return Drive Parameters function
	movb	$0x81, %dl		/ for Hard Drive 1

	int	$0x13			/ BIOS disk support

	addr16
	mov	%ecx, hd1parm		/ Save parameters in table
	addr16
	mov	%edx, hd1parm+2

/	start up the C code

	data16
	call	goprot			/ enter protected mode
	cli

	call	main			/ jump to the C code; shouldn't return

	sti
	call	goreal			/ we should never reach this point

	jmp	halt

#ifdef DEBUG
banner2:	.string	"boot [second stage]:\r\n"
#endif
	.globl  memsz
memsz:		.long   0
