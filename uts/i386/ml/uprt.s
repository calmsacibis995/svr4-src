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

	.ident	"@(#)kern-ml:uprt.s	1.3.2.2"

#include "symvals.h"

	.globl  _start
	.globl	pstart
	.globl	gdt
	.globl  gdtend
	.globl	idt
#ifdef VPIX
	.globl	idt2
#endif
	.globl	kpt0
	.globl	kptn
	.globl	mon1sel
	.globl	mon3sel
	.globl	mon1off
	.globl	mon3off
	.globl	monidt
	.globl	scall_dscr
	.globl	sigret_dscr
	.globl	df_stack
	.globl	Rgdtdscr
	.globl	Ridtdscr
	.globl	egafontptr

	.set	PROTBIT, 0x0001
	.set	PAGEBIT, 0x8000
	.set    DFSTKSIZ,0x0FFE
	.set	IDTLIM, [8\*256-1]
	.set	MONIDTLIM, [8\*16-1]
	.set	JBTLOC, 0x0400
	.set	BM_BASE, 0
	.set	BM_EXTENT, 4
	.set	KPD_LOC, [KPTBL_LOC+0x1000]

	.text
/
/	*** NOTICE *** NOTICE *** NOTICE *** NOTICE *** NOTICE ***
/
/		The instructions in pstart are reversed 16 <--> 32
/		bits.  This is because we are running pstart in
/		REAL MODE.  By using long instructions, we generate
/		opcodes that are 16 bit instructions when run
/		in REAL MODE.

/	More nice information:
/		This code now only supports the BKI boot-kernel interface.
/		This passes the magic number 0xff1234ff in %edi.
/		All other info is passed in the bootinfo structure.

pstart:
_start:

	data16
	cmpl	$BKI_MAGIC, %edi
	data16
	je	BKI_ok

	/ Bad magic number from bootstrap.
	/ Print a message, then halt.
	/ Unfortunately, this will only work on an AT386.

	data16
	call	_rprint
	.string	"\r\nBootstrap too old.\r\n"
_halt:
	sti
	hlt
	jmp	_halt

_rprint:
	data16
	popl	%esi		/ get pointer to message

	movb	$1, %bl		/ foreground color
ploop:	addr16
	movb	%cs:(%esi), %al	/ get chr
	data16
	incl	%esi
	testb	%al, %al	/ test for end of string
	jz	pend
	movb	$14, %ah	/ setup call to bios
	int	$0x10		/ print chr
	jmp	ploop		/ repeat for next chr
pend:
	data16
	pushl	%esi
	data16
	ret


	.align	8
Rusermon:
	.byte   0               / If you set this byte to non-zero
				/ moninit will put the monitors vectors
				/ into idt(1) and idt(3), thus allowing
				/ user programs to be debugged with DMON.
	.string	"<-Here"

	.align	8	/ This is for ease of looking at memory.
Rgdtdscr:
	.value  [8\*GDTSZ-1]       / We will re-compute this, but just in case...
	.long	gdt

	.align	8

Ridtdscr:
	.value	IDTLIM
	.long	idt

	.align	8
RIgdtdscr:			/ This and the next entry are used to
	.value  [8\*GDTSZ-1]       / initialize DMON
	.long	gdt

	.align	8
RIidtdscr:
	.value	IDTLIM
	.long	idt

	.align	8
RMidtdscr:
	.value	MONIDTLIM
	.long	monidt

	.align	8
R0idtdscr:
	.value	0xffff
	.long	0

/	EGA font pointers (these start as real mode pointers)
/	The pointers point to the 8x8, 8x14, 9x14, 8x16 and 
/	the 9x16 fonts, respectively

egafontptr:
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	
BKI_ok:
/	Here we have to set up the kernel symbol page table
/	according to the memused information passed by the bootstrap.
/	After we're done, R_Set_Addr will be able to convert virtual
/	addresses to physical addresses using this page table.

	data16
	xorl	%eax, %eax		/ Load 0 into segment registers
	movw	%ax, %ds		/   so we get absolute addresses
	movw	%ax, %es
	movw	%ax, %fs
	cld

	data16
	movl	$KPTBL_LOC, %edi	/ First zero out the page table & dir
	data16
	movl	$2048, %ecx
	addr16
	data16
	rep; sstol

	data16
	movl	$BOOTINFO_LOC, %ebx
	data16
	addr16
	movl	memusedcnt(%ebx), %edx	/ Get count of memused segments
	data16
	movl	%edx, %esi
	data16
	addl	$memused, %ebx		/ Get pointer to first segment

	data16
	movl	$KPTBL_LOC, %edi	/ "Reserved" segment maps at KVSBASE

kptbl_loop:
	data16
	addr16
	movl	BM_EXTENT(%ebx), %ecx	/ Compute # pages for this segment
	data16
	shrl	$12, %ecx

	data16
	addr16
	movl	BM_BASE(%ebx), %eax	/ Compute base pte for this segment
	data16
	andl	$0xfffff000, %eax
	incl	%eax			/ Set present bit

kptseg_loop:
	addr16
	data16
	sstol				/ Store the next page table entry
	data16
	addl	$0x1000, %eax		/ Advance to next physical page
	loop	kptseg_loop

	data16
	cmpl	%edx, %esi		/ If moving on to 2nd segment,
	jne	kptbl_next		/ Reset %esi for start of text

	data16
	movl	$stext, %edi		/ Compute addr of page table
	data16
	shrl	$12-2, %edi		/  entry for start of kernel text
	data16
	andl	$0xffc, %edi
	data16
	addl	$KPTBL_LOC, %edi

kptbl_next:
	data16
	addl	$12, %ebx		/ Advance to next segment
	decl	%edx
	data16
	jnz	kptbl_loop

/	At this point, we are running on the bootloaders stack.
/	We will now find our stack and switch to it.
	data16
	movl	$df_stack, %eax
	data16
	call	R_Set_Addr

	movw	%ds, %ax
	movw	%ax, %ss
	data16
	addl	$DFSTKSIZ, %ebx
	movw	%bx, %sp

	/ Now, find the GDT so that we can rearrange it.
	data16
	movl	$gdt, %eax

	data16
	movl    $gdtend, %ecx
	subw	%ax, %cx
	subw	$1, %cx

	data16
	movl	$Rgdtdscr, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%ecx, (%ebx)

	data16
	movl	$gdt, %eax
	data16
	call	munge_table

	/ Find the IDT so that we can rearrange it.
	data16
	movl	$idt, %eax

	data16
	movl	$IDTLIM, %ecx
	data16
	call	munge_table
#ifdef VPIX
	/ Find the IDT so that we can rearrange it.
	data16
	movl	$idt2, %eax

	data16
	movl	$IDTLIM, %ecx
	data16
	call	munge_table
#endif

	/ A couple of other interesting descriptors.  (scall_dscr)
	data16
	movl    $scall_dscr, %eax
	data16
	movl	$1, %ecx
	data16
	call	munge_table

	/ A couple of other interesting descriptors.  (sigret_dscr)
	data16
	movl    $sigret_dscr, %eax
	data16	
	movl	$1, %ecx
	data16
	call	munge_table

	/ Now, we need to fix up the first, 3gig, and last entries in the
	/ page directory.

	data16
	movl	$kpt0, %eax		/ First, Page table 0
	data16
	call	R_Virt_to_Phys
	incl	%eax			/ Set the present bit
	addr16
	movw	%ax, %fs:KPD_LOC
	addr16
	movw	%ax, %fs:[KPD_LOC+3072]

	data16				/ Also, kernel address page table
	movl	$KPTBL_LOC+1, %eax	/   (with present bit set)
	addr16
	movw	%ax, %fs:[KPD_LOC+3328]

	data16
	movl	$kptn, %eax		/ Now, the last Page table
	data16
	call	R_Virt_to_Phys
	incl	%eax			/ Set the present bit
	addr16
	movw	%ax, %fs:[KPD_LOC+4092]


#if defined (MB1) || defined (MB2)
/	The mon_init procedure will call into the monitor to allow it
/	to initialize its vectors in the IDT and GDT.  Due to some
/	'features' in DMON, gdtr and idtr will be handled in mon_init.
/	data16
/	call	mon_init
#endif /* MB1 */

	/ Load IDTR and GDTR
	data16
	movl    $Rgdtdscr, %eax
	data16
	call	R_Set_Addr
	addr16
	data16
	lgdt	(%ebx)

#ifdef AT386
/	Code to find font locations from the bios
/	and to put them in egafonptr[] where the kd driver can find them.

	movw	$0x1130, %ax	/ set up bios call
	.value	0
	movw	$0x0300, %bx	/ get pointer to 8x8 font
	.value	0
	int	$0x10

	data16
	movl	$egafontptr, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%ebp, (%ebx)
	addr16
	movw	%es, 2(%ebx)

	movw	$0x1130, %ax	/ set up bios call
	.value	0
	movw	$0x0200, %bx	/ get pointer to 8x14 font
	.value	0
	int	$0x10

	data16
	movl	$egafontptr+4, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%ebp, (%ebx)
	addr16
	movw	%es, 2(%ebx)

	movw	$0x1130, %ax	/ set up bios call
	.value	0
	movw	$0x0500, %bx	/ get pointer to 9x14 font
	.value	0
	int	$0x10

	data16
	movl	$egafontptr+8, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%ebp, (%ebx)
	addr16
	movw	%es, 2(%ebx)

	movw	$0x1130, %ax	/ set up bios call
	.value	0
	movw	$0x0600, %bx	/ get pointer to 8x16 font
	.value	0
	int	$0x10

	data16
	movl	$egafontptr+0xc, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%ebp, (%ebx)
	addr16
	movw	%es, 2(%ebx)

	movw	$0x1130, %ax	/ set up bios call
	.value	0
	movw	$0x0700, %bx	/ get pointer to 9x16 font
	.value	0
	int	$0x10

	data16
	movl	$egafontptr+0x10, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%ebp, (%ebx)
	addr16
	movw	%es, 2(%ebx)
#endif /* AT386 */

/	*** NOTICE *** NOTICE *** NOTICE *** NOTICE *** NOTICE ***
/
/		Do not try to single step past this point!!!!
/		use a 'go till' command!!!!
	data16
	movl    $Ridtdscr, %eax
	data16
	call	R_Set_Addr
	addr16
	data16
	lidt	(%ebx)
	addr16
	smsw	%eax		/ Get the MSW

	data16
	orl	$PROTBIT, %eax

	addr16
	lmsw	%eax		/ Kick us into protected mode
	jmp	qflush

qflush:			/ Note that this point we are still
			/ in 16 bit addressing mode.

	data16
	movl	$KPD_LOC, %eax
	addr16
	movl	%eax, %cr3

	addr16
	movl	%cr0, %eax
	orw	$0, %ax
	.value	PAGEBIT
	addr16
	movl	%eax, %cr0

	data16
	movl	$JTSSSEL, %eax

	ltr	%ax

/	This is a 16 bit long jump.
	.byte	0xEA
	.value	0
	.value	KTSSSEL

/ *********************************************************************
/
/	munge_table:
/		This procedure will 'munge' a descriptor table to
/		change it from initialized format to runtime format.
/
/		Assumes:
/			%eax -- contains the base address of table.
/			%ecx -- contains size of table.
/
/ *********************************************************************
munge_table:
	pushl	%ds

	data16
	andl	$0xFFFF, %ecx
	addw	%ax, %cx
	movw	%ax, %si

moretable:
	cmpw	%si, %cx
	jl	donetable		/ Have we done every descriptor??

	movw	%si, %ax
	data16
	call	R_Set_Addr

	addr16
	movb	7(%ebx), %al	/ Find the byte containing the type field
	testb	$0x10, %al	/ See if this descriptor is a segment
	jne	notagate
	testb	$0x04, %al	/ See if this destriptor is a gate
	je	notagate
				/ Rearrange a gate descriptor.
	addr16
	movl	6(%ebx), %eax	/ Type (etc.) lifted out
	addr16
	movl	4(%ebx), %edx	/ Selector lifted out.
	addr16
	movl	%eax, 4(%ebx)	/ Type (etc.) put back
	addr16
	movl	2(%ebx), %eax	/ Grab Offset 16..31
	addr16
	movl	%edx, 2(%ebx)	/ Put back Selector
	addr16
	movl	%eax, 6(%ebx)	/ Offset 16..31 now in right place
	jmp	descdone

notagate:			/ Rearrange a non gate descriptor.
	addr16
	movl	4(%ebx), %edx	/ Limit 0..15 lifted out
	addr16
	movb	%al, 5(%ebx)	/ type (etc.) put back
	addr16
	movl	2(%ebx), %eax	/ Grab Base 16..31
	addr16
	movb	%al, 4(%ebx)	/ put back Base 16..23
	addr16
	movb	%ah, 7(%ebx)	/ put back Base 24..32
	addr16
	movl	(%ebx), %eax	/ Get Base 0..15
	addr16
	movl	%eax, 2(%ebx)	/ Base 0..15 now in right place
	addr16
	movl	%edx, (%ebx)	/ Limit 0..15 in its proper place

descdone:
	data16
	addl	$8, %esi	/ Go for the next descriptor
	jmp	moretable

donetable:
	popl	%ds
	data16
	ret

#if defined (MB1) || defined (MB2)
/ *********************************************************************
/
/	mon_init:
/               This procedure will check a DMON flag in memory to
/		find the entry point to sq$init_monitor and call it.
/
/		The steps performed are:
/		1) Look at the start of the table to see if
/		   the characters "JBT" are there.  If so,
/		   this is DMON and we can call sq$mon_init.
/		   If not, exit, we cannot use the monitor.
/
/		2) Load gdtr and idtr with the physical addresses
/		   of the descriptor tables.  These addresses are not
/		   the same as the linear addresses we will use, but
/		   must be the physical addresses so that DMON can
/		   find the tables.
/ *********************************************************************
mon_init:
	data16
	movl	$JBTLOC, %eax
	data16
	call	R_Set_Addr

	addr16
	movw	(%ebx), %ax
	andw	$0xffff, %ax
	.value	0x00ff

	data16
	cmpl    $0x0054424a, %eax
	jne	no_monitor

	pushl   %ds             / Save the address of the DMON flag
	popl    %es
	movw	%bx, %cx

	/ Load IDTR and GDTR with the physical addresses of the tables.
	data16
	movl	$RIgdtdscr, %eax
	data16
	call	R_Set_Addr

	addr16
	movw	2(%ebx), %ax
	data16
	call	R_Virt_to_Phys
	addr16
	movw	%ax, 2(%ebx)

	addr16
	data16
	lgdt	(%ebx)

/	*** NOTICE *** NOTICE *** NOTICE *** NOTICE *** NOTICE ***
/
/		Do not try to single step past this point!!!!
/		use a 'go till' command!!!!
	/ See if the user wants to use DMON on user processes.
	data16
	movl	$Rusermon, %eax
	data16
	call	R_Set_Addr

	addr16
	testb	$0xff, (%ebx)
	jz	normal_mon

	data16
	movl	$RIidtdscr, %eax
	data16
	movl	$idt, %edi
	jmp	chose_mon

normal_mon:
	data16
	movl	$RMidtdscr, %eax
	data16
	movl	$monidt, %edi

chose_mon:
	data16
	call	R_Set_Addr

	addr16
	movw	2(%ebx), %ax
	data16
	call	R_Virt_to_Phys
	addr16
	movw	%ax, 2(%ebx)

	addr16
	data16
	lidt	(%ebx)

/	Let the monitor initialize its vectors.
/	.byte	0x9A
/	.value	0x341
/	.value	0x80
	movl	%ecx, %ebx
	.byte	0x26		/ Use es
	.byte	0xff
	.byte	0x5f
	.byte	0x18
/	call	%es:24(%cx)

	data16
	movl	$R0idtdscr, %eax
	data16
	call	R_Set_Addr

	addr16
	data16
	lidt	(%ebx)

/ Since we do not want DMON to use 0 linear for its data segment,
/ we must reset the base of gdt[3] to the linear area we have mapped
/ to 0 physical.
	data16
	movl	$gdt, %eax
	data16
	call	R_Set_Addr

	addr16
	movw	26(%ebx), %ax
	data16
	call	R_Virt_to_Phys
	movw	%ax, %cx
	xorw	%ax, %ax
	addr16
	movb	31(%ebx), %al
	shlw	$24, %ax
	orw	%cx, %ax
	addw	$0x0000, %ax
	.value	0xFFF7
	addr16
	movl	%eax, 26(%ebx)
	shrw	$16, %ax
	addr16
	movb	%ah, 31(%ebx)
	addr16
	movb	%al, 28(%ebx)

/ Now, grab the selector and offset out of interrupt 1 and 3
/ We will store them so the interrupt handlers can access them
/ at will.
	movw	%di, %ax	/ We loaded edi before the first lidt
	data16
	call	R_Set_Addr
	pushl	%ds
	popl	%es
	movw	%bx, %di

	data16
	movl	$mon1sel, %eax
	data16
	call	R_Set_Addr

	addr16
	movl	%es:10(%edi), %eax
	addr16
	movl	%eax, (%ebx)

	data16
	movl	$mon1off, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%es:8(%edi), %eax
	addr16
	movl	%eax, (%ebx)
	addr16
	movl	%es:14(%edi), %eax
	addr16
	movl	%eax, 2(%ebx)

	data16
	movl	$mon3sel, %eax
	data16
	call	R_Set_Addr

	addr16
	movl	%es:26(%edi), %eax
	addr16
	movl	%eax, (%ebx)

	data16
	movl	$mon3off, %eax
	data16
	call	R_Set_Addr
	addr16
	movl	%es:24(%edi), %eax
	addr16
	movl	%eax, (%ebx)
	addr16
	movl	%es:30(%edi), %eax
	addr16
	movl	%eax, 2(%ebx)

no_monitor:
	data16
	ret
#endif /* MB1 */

/ *********************************************************************
/
/	R_Virt_to_Phys:
/		This procedure takes a 32 bit virtual address and
/		converts it to a linear physical address by looking
/		it up in the kernel page table.
/
/		Input:
/			%eax -- virtual address.
/
/		Output:
/			%eax -- physical address.
/
/ *********************************************************************
R_Virt_to_Phys:
	data16
	pushl	%ebx
	data16
	movl	%eax, %ebx		/ Save virtual address in %ebx
	data16
	subl	$KVSBASE, %eax		/ If address is below KVSBASE,
	jb	vtop_done		/   assume it's physical

	data16
	shrl	$12-2, %eax		/ Convert to page table offset
	data16
	andl	$0xffc, %eax

	data16
	addr16
	movl	%fs:KPTBL_LOC(%eax), %eax  / Get physical page address
	data16
	andl	$0xfffff000, %eax

	data16
	andl	$0xfff, %ebx		/ Get virtual page offset

	data16
	addl	%eax, %ebx		/ Compute final virtual address

vtop_done:
	data16
	movl	%ebx, %eax
	data16
	popl	%ebx
	data16
	ret

/ *********************************************************************
/
/	R_Set_Addr:
/		This procedure takes a 32 bit address and sets up ds:bx
/		from it.  In the process, it will cut it down into
/		the first megabyte.
/
/		Input:
/			%eax -- 32-bit physical address.
/
/		Output:
/			%ds:%ebx -- real-mode address.
/			[ %eax not preserved ]
/
/ *********************************************************************
R_Set_Addr:
	data16
	call	R_Virt_to_Phys	/ Convert to a physical (linear) address.
	movw	%ax, %bx	/ Remember the addr for the offset portion.
	shrw	$4, %ax		/ Turn the address into a real mode selector.
	movw	%ax, %ds

	data16
	andl	$0xF, %ebx	/ Now the offset portion

	data16
	ret
