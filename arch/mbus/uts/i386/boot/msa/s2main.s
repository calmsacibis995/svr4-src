/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/	Copyright (c) 1988  Intel Corporation	
/	All Rights Reserved	

/	INTEL CORPORATION PROPRIETARY INFORMATION	

/	This software is supplied to AT & T under the terms of a license  
/	agreement with Intel Corporation and may not be copied nor        
/	disclosed except in accordance with the terms of that agreement.   

	.file   "s2main.s"

	.ident	"@(#)mbus:uts/i386/boot/msa/s2main.s	1.3"

/ ********************************************************************
/
/	This is the Unix V/386 R3.1 second stage bootstrap loader for 
/	Intel's MBII system with the first stage executing in protected mode.
/
/ ********************************************************************

/ DIB types from include "dib.h"		

	.set	DISK_DIB,		0x5961		/ SCSI disk dib
	.set	TAPE_DIB,		0x5962		/ SCSI tape dib
	.set	BS_DIB,			0x5963		/ MBII Bootserver DIB 
	.set	PCI_DISK_DIB,		0x5964		/ PCI  disk DIB 
	.set	PCI_TAPE_DIB,		0x5965		/ PCI  tape DIB 
	.set	STAGE2,			2
	.set	E_DIB_TYPE,		0x1b

	.set	VLAB_START,		0x180
	.set	VLAB_FSDOFF,		56
	.set	VLABS_PL_FSDOFF,	0x1b8
	.set	BSDIBSZ, 		56	/ must match the Bootserver 
						/ DIB size
	.set	SCDISKDIBSZ,		24	/ must match the SCSI Disk 
						/ DIB size
	.set	SCTAPEDIBSZ,		36	/ must match the SCSI Tape 
						/ DIB size
	.set	PCIDISKDIBSZ, 		24	/ must match PCI disk DIB size
	.set	PCITAPEDIBSZ, 		36	/ must match PCI tape DIB size
	.set	GDTSEL,			8
	.set	DATASEL,		0x1b0	/ selector 54
	.set	MEMALIAS,		0x1e0	/ selector 60
	.set	FIRST_FREE_SEL,		0x200	/ selector 64
	.set	MAX_DEV_GRAN,		0x1000	/ max device granularity


	.bss

/ this is the local copy of the bootstrap linkage block passed from the 
/ first stage 

	.globl	bootparam
bootparam:
	. = . + 4		/ Bootstrap flags

	.globl	DIB_ptr
DIB_ptr:
	. = . + 4		/ DIB offset
	. = . + 2		/ DIB selector

	.globl	_err_handler
_err_handler:
	. = . + 4		/ _err_handler offset
	. = . + 2		/ _err_handler selector

	.globl	_file_open_ptr
_file_open_ptr:
	. = . + 4		/ file_open offset
	. = . + 2		/ file_open selector

	.globl	_file_read_ptr
_file_read_ptr:
	. = . + 4		/ file_read offset
	. = . + 2		/ file_read selector

	.globl	_file_close_ptr
_file_close_ptr:
	. = . + 4		/ file_close offset
	. = . + 2		/ file_close selector

mem_used_list:
	. = . + 4		/ mem_used_list offset
	. = . + 2		/ mem_used_list selector

#ifdef MB1
	.globl	path_off
path_off:
	. = . + 4		/ path offset
	.globl	path_sel
path_sel:
	. = . + 2		/ path selector
#endif
filler:
	. = . + 2

/ certain selective fields from the DIB which are used in the second stage

	.align 8
			
_init_ptr:
	. = . + 4		/ init offset
	. = . + 2		/ init selector
_disk_read_ptr:
	. = . + 4		/ disk_read offset
	. = . + 2		/ disk_read selector

	.align 8

_tape_read_ptr:
	. = . + 4		/ tape_read offset
	. = . + 2		/ tape_read selector

	.globl	_tape_rewind_ptr
_tape_rewind_ptr:
	. = . + 4		/ tape_rewind offset
	. = . + 2		/ tape_rewind selector

	.globl	_tape_seek_ptr
_tape_seek_ptr:
	. = . + 4		/ tape_seek offset
	. = . + 2		/ tape_seek selector

	.align 8

	.globl	_get_bps_ptr
_get_bps_ptr:
	. = . + 4		/ get_bps offset
	. = . + 2		/ get_bps selector

	.globl	bs_file_open_ptr
bs_file_open_ptr:
	. = . + 4		/ bs_file_open offset
	. = . + 2		/ bs_file_open selector

	.globl	bs_file_read_ptr
bs_file_read_ptr:
	. = . + 4		/ bs_file_read offset
	. = . + 2		/ bs_file_read selector

	.globl	bs_file_close_ptr
bs_file_close_ptr:
	. = . + 4		/ bs_file_close offset
	. = . + 2		/ bs_file_close selector

bs_disconnect_ptr:
	. = . + 4		/ bs_disconnect offset
	. = . + 2		/ bs_disconnect selector

	.align 8

	.globl	dev_gran
dev_gran:
	. = . + 4		/ Device granularity

int_tab_off:
	. = . + 8 		/ to load IDT on transfer

jmp_vector:
	. = . + 8 		/ place to store the jump vector for 32-16 xfer

	.globl	disk_flag
disk_flag:
	. = . + 1		/ flag for disk type device

	.globl	ourDS
ourDS:
	. = . + 2		/ save our Data Segment register

	.globl	mem_alias_sel
mem_alias_sel:
	. = . + 2		/ memory alias segment descriptor

cs_alias_seg:
	. = . + 2		/ code segment alias descriptor

	.globl	unix_start
unix_start:
	. = . + 4		/ start of unix area on disk

	.align 16

	.globl	dib_data
dib_data:
	. = . + 80		/ 80 bytes max for the dib 
			
	.globl	bmu_list_entry
bmu_list_entry:
	. = . + 80		/ 80 bytes max for the memory used list

	.globl	syment
syment:
	. = . + 18		/ size of SYMENT see syms.h

	.align	8

	.globl	status
status:
	. = . + 4		/ global status variable

	.globl	actual
actual:
	. = . + 4


	.text

/ parameter space

saved_ebp		= 0
ret_addr_off		= 4
ret_addr_sel		= 8
bs_link_block_offset	= 12
bs_link_block_sel	= 16

/ When the first stage transfers control, the .text section of the second
/ stage is loaded in the code segment and the data segment descriptor is
/ set to the size of the data section as given in the BOLT.  The 
/ first stage guarantees that the code and data segments are 
/ contiguous in memory and in increasing address order (code 
/ followed by data).
/
/ Because the second stage is in small model (cs=ds), we have to manipulate
/ the data segment descriptor base and limit to allow us to execute in 
/ small model.  The data segment descriptor base is set equal to 
/ code segment descriptor, and the limit is set to cslimit + dslimit.
/
/ Note three things about the second stage code: (1)  the .data 
/ section does not get loaded, (2) .bss is not necessarily zeroed,
/ and (3) all constants are coded in assembly language to be placed
/ in the .text section.
/
/ The second stage will continue to use the first stage stack.

	.globl	s2main
s2main:
	xchg	%eax, %eax		/ save space for an int 1 instruction

	movw	$GDTSEL, %ax
	movw	%ax, %es		/ load es:di to point to CS selector
	xor	%edi, %edi
	xor	%esi, %esi
	movw	%cs, %di
	movw	%es:(%edi), %bx		/ get limit of CS selector 
	movw	$DATASEL, %esi
	movw	%es:(%esi), %ax		/ get limit of DS selector

	/ assume small model (cslimit + dslimit) < 64K

	addw 	%bx, %ax
	orb	$0xf, %al		/ paragraph align
	movw	%ax, %es:(%esi)		/ put it back in DS limit

	movb 	%es:7(%edi), %al	/ now set dsbas = csbas
	movb 	%es:7(%edi), %al	/ now set dsbas = csbas
	movb 	%al, %es:7(%esi)
	movb 	%es:4(%edi), %al
	movb 	%al, %es:4(%esi)
	movw 	%es:2(%edi), %ax
	movw 	%ax, %es:2(%esi)
	
	movw	$DATASEL, %ax
	movw	%ax, %ds		/ reload DS
	movw	%ax, ourDS
	movw	$MEMALIAS, mem_alias_sel
	
	/ make local copy of parameters

	push	%ebp
	movl	%esp, %ebp
	mov 	bs_link_block_sel(%ebp), %eax
	movw 	%ax, %es
	mov 	bs_link_block_offset(%ebp), %esi
	movl	$bootparam, %edi
	movl	$[filler - bootparam], %ecx

	push	%ds
	push	%es
	pop	%ds			/ switch ds and es to do the transfer
	pop	%es			/ ds:esi -> es:edi
	repnz
	smovb

	push	%es
	pop	%ds			/ restore ds back

	/ now make local copy of the bootstrap memory used list

	movw	[4+mem_used_list], %es
	movl	mem_used_list, %esi
	movl	$bmu_list_entry, %edi
	movl	$80, %ecx		/ size of bmu_list_entry
	push	%ds
	push	%es
	pop	%ds			/ switch ds and es to do the transfer
	pop	%es			/ ds:esi -> es:edi

	repnz
 	smovb

	push	%es
	pop	%ds			/ restore ds back

	/ reset flag for disk_flag

	movb	$0, disk_flag
	
	/ now copy the DIB  for posterity

	movw	[4 + DIB_ptr], %es
	movl	DIB_ptr, %esi
	movl	$dib_data, %edi
	movw	%es:4(%esi), %bx	/ save DIB type in bx for use later
	movl	%es:(%esi), %ecx	/ DIB size

	push	%ds
	push	%es
	pop	%ds			/ switch ds and es to do the transfer
	pop	%es			/ ds:esi -> es:edi

	repnz 	
	smovb

	push	%ds
	push	%es
	pop	%ds			/ restore ds back
	pop	%es			/ restore es back

	movl	DIB_ptr, %edi
	cmpw	$PCI_DISK_DIB, %bx	/ booting from a PCI disk ?
	jne	is_it_pci_tape_device
	movl	%es:8(%edi), %eax
	movl	%eax, _init_ptr
	movw	%es:12(%edi), %ax
	movw	%ax, [_init_ptr + 4]
	movl	%es:14(%edi), %eax
	movl	%eax, _disk_read_ptr
	movw	%es:18(%edi), %ax
	movw	%ax, [_disk_read_ptr + 4]
	movzwl	%es:20(%edi), %eax
	movl	%eax, dev_gran
	movb	$1, disk_flag
	jmp	dib_done

is_it_pci_tape_device:
	cmpw	$PCI_TAPE_DIB, %bx 	/ booting from a PCI tape ?
	jne	is_it_sdisk
	movl	%es:8(%edi), %eax
	movl	%eax, _init_ptr
	movw	%es:12(%edi), %ax
	movw	%ax, [_init_ptr + 4]
	movl	%es:14(%edi), %eax
	movl	%eax, _tape_read_ptr
	movw	%es:18(%edi), %ax
	movw	%ax, [_tape_read_ptr + 4]
	movl	%es:20(%edi), %eax
	movl	%eax, _tape_rewind_ptr
	movw	%es:24(%edi), %ax
	movw	%ax, [_tape_rewind_ptr + 4]
	movl	%es:26(%edi), %eax
	movl	%eax, _tape_seek_ptr
	movw	%es:30(%edi), %ax
	movw	%ax, [_tape_seek_ptr + 4]
	movzwl	%es:32(%edi), %eax
	movl	%eax, dev_gran
	jmp	dib_done
	
is_it_sdisk:
	cmpw	$DISK_DIB, %bx		/ booting from on-board SCSI disk ?
	jne	is_it_stape
	movl	%es:14(%edi), %eax
	movl	%eax, _disk_read_ptr
	movw	%es:18(%edi), %ax
	movw	%ax, [_disk_read_ptr + 4]
	movzwl	%es:20(%edi), %eax
	movl	%eax, dev_gran
	movb	$1, disk_flag
	jmp	dib_done
	
is_it_stape:
	cmpw	$TAPE_DIB, %bx		/ booting from on-board SCSI tape ?
	jne	is_it_bs
	movl	%es:14(%edi), %eax
	movl	%eax, _tape_read_ptr
	movw	%es:18(%edi), %ax
	movw	%ax, [_tape_read_ptr + 4]
	movl	%es:26(%edi), %eax
	movl	%eax, _tape_seek_ptr
	movw	%es:30(%edi), %ax
	movw	%ax, [_tape_seek_ptr + 4]
	movzwl	%es:32(%edi), %eax
	movl	%eax, dev_gran
	jmp	dib_done
		
is_it_bs:
	cmpw	$BS_DIB, %bx		/ booting from a MBII Bootserver ?
	jne	unknown_dib
	movl	%es:14(%edi), %eax
	movl	%eax, _get_bps_ptr
	movl	%es:18(%edi), %eax
	movw	%ax, [_get_bps_ptr + 4]
	movl	%es:26(%edi), %eax
	movl	%eax, bs_file_open_ptr
	movl	%es:30(%edi), %eax
	movw	%ax, [bs_file_open_ptr + 4]
	movl	%es:32(%edi), %eax
	movl	%eax, bs_file_read_ptr
	movw	%es:36(%edi), %ax
	movw	%ax, [bs_file_read_ptr + 4]
	movl	%es:44(%edi), %eax
	movl	%eax, bs_file_close_ptr
	movw	%es:48(%edi), %ax
	movw	%ax, [bs_file_close_ptr + 4]
	movl	%es:50(%edi), %eax
	movl	%eax, bs_disconnect_ptr
	movw	%es:54(%edi), %ax
	movw	%ax, [bs_disconnect_ptr + 4]
	movl	$MAX_DEV_GRAN, dev_gran
	jmp	dib_done

unknown_dib:
	push	$0
	push	$E_DIB_TYPE
	push	$STAGE2
	call	error			/ does not return
	
dib_done:
#ifndef MB1
	/ first indicate that stage 2 has commenced - status in i/c
	push	$0
	push	$0
	push	$STAGE2
	call	error
#endif

	/ update file open, read, close entries

	movw	%cs, [_file_open_ptr + 4]
	movl	$BL_file_open, _file_open_ptr
	movw	%cs, [_file_read_ptr + 4]
	movl	$BL_file_read, _file_read_ptr
	movw	%cs, [_file_close_ptr + 4]
	movl	$BL_file_close, _file_close_ptr

/ setup memory alias segment with base=0, limit = 0xfffff, access 
/ rights as rw and expand up.  Memory alias segment will be used to 
/ load the target file.

	movw	$GDTSEL, %ax
	movw	%ax, %es
	movl	$MEMALIAS, %ebx
	movw	$0xffff, %es:(%ebx)
	movw	$0, %es:2(%ebx)
	movb	$0, %es:4(%ebx)
	movb	$0x92, %es:5(%ebx)
	movb	$0xcf, %es:6(%ebx)
	movb	$0, %es:7(%ebx)

	movw	%ds, %ax
	movw	%ax, %es

	movl	$0, unix_start
	call	pboot		/ pass the baton -- start of the relay race 

/ *********************************************************************
/
/	iomove
/
/ *********************************************************************

	.globl	iomove

src_off	=	32
src_sel	=	36
dst_off	=	40
dst_sel	=	44
count	=	48

iomove:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	movw	dst_sel(%ebp),%es
	movl	dst_off(%ebp),%edi
	movw	src_sel(%ebp),%ds
	movl	src_off(%ebp),%esi
	movl	count(%ebp),%ecx
	jcxz	iomdone			/ 0 ==> done
	rep					/ mult-times...
	smovb				/ move it.
iomdone:				/ return C-style
	jmp	cret

/ *********************************************************************
/
/ error
/	calls the first stage error routine
/
/ *********************************************************************

	.globl	error

stage		= 32
err_code	= 36
err_msg_off	= 40

error:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	movl	stage(%ebp), %eax
	push	%eax
	movl	err_code(%ebp), %eax
	push	%eax
	movw	%ds, %bx		/ all error messages are in code segment
	movl	err_msg_off(%ebp),%eax
	cmpl	$0, %eax
	jne	null_p
	movl	$0, %ebx
null_p:
	push	%ebx
	push	%eax
	lcall	*_err_handler

	jmp	cret

/ *********************************************************************
/
/ get_host_bps
/	This is the C to PL/M assembly language interface for the get_host_bps
/	procedure from the bootserver device driver.  The reason it is 
/	included here and not with the bootserver driver module is that
/	this functionality is not symmetrical with other file driver modules
/	and hence the special treatise.
/
/ *********************************************************************

	.globl	get_host_bps

host_bps_off	= 32
status_off	= 36

get_host_bps:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	push	%ds
	movl	host_bps_off(%ebp), %eax
	push	%eax
	push	%ds
	movl	status_off(%ebp), %eax
	push	%eax
	lcall	*_get_bps_ptr
	jmp	cret
	
/ *********************************************************************
/
/ disk_read
/	This is the C to PL/M assembly language interface for the disk
/	read routine
/
/ *********************************************************************

	.globl	disk_read

sector_num		= 32
num_blocks		= 36
buff_offset		= 40
buff_sel		= 44
t_actual		= 48
status_off		= 52

disk_read:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	movl	sector_num(%ebp), %eax	
	push	%eax
	movl	num_blocks(%ebp), %eax
	push	%eax
	movw	buff_sel(%ebp), %ax
	push	%eax
	movl	buff_offset(%ebp), %eax
	push	%eax
	movl	t_actual(%ebp), %eax
	push	%ds
	push	%eax
	movl	status_off(%ebp), %eax
	push	%ds
	push	%eax
	lcall	*_disk_read_ptr
	jmp	cret

/ *********************************************************************
/
/ tape_read
/	This is the C to PL/M assembly language interface for the tape
/	read routine
/
/ *********************************************************************

	.globl	tape_read

num_blocks		= 32
buff_offset		= 36
buff_sel		= 40
t_actual		= 44
status_off		= 48

tape_read:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	movl	num_blocks(%ebp), %eax
	push	%eax
	movw	buff_sel(%ebp), %ax
	push	%eax
	movl	buff_offset(%ebp), %eax
	push	%eax
	movl	t_actual(%ebp), %eax
	push	%ds
	push	%eax
	movl	status_off(%ebp), %eax
	push	%ds
	push	%eax
	lcall	*_tape_read_ptr
	jmp	cret

/ *********************************************************************
/
/ tape_seek
/	This is the C to PL/M assembly language interface for the tape
/	seek routine
/
/ *********************************************************************

	.globl	tape_seek

status_off		= 32

tape_seek:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	movl	status_off(%ebp), %eax
	push	%ds
	push	%eax
	lcall	*_tape_seek_ptr
	jmp	cret

/ *********************************************************************
/
/ bs_disconnect
/	This is the C to PL/M assembly language interface for the bootserver
/	disconnect routine
/
/ *********************************************************************

status_off		= 32

	.globl	bs_disconnect
bs_disconnect:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	movl	status_off(%ebp), %eax
	push	%ds
	push	%eax
	lcall	*bs_disconnect_ptr
	jmp	cret

/ *********************************************************************
/
/ cret
/
/ *********************************************************************

cret:
	leave
	pop		%edi
	pop		%esi
	pop		%ebx
	pop		%ecx
	pop		%ds
	pop		%es
	ret

/ *********************************************************************
/
/ startunix
/	This is the final step in the second stage bootstrap loader for
/	loading Unix.  The kernel start address and the debug flag is 
/	saved in registers, the processor is switched to real mode,
/	and, a temporary stack is setup. If the debug flag is set, 
/	an INT 3 is simulated else, a long return  is simulated to the 
/	kernel start address
/
/ *********************************************************************

	.globl	startunix

save_bp			= 0
ret_address		= 4
kernel_address		= 8
debug_flag		= 12

startunix:
	push 	%ebp
	movl	%esp, %ebp
	mov 	kernel_address(%ebp), %edx	/ kernel addr in seg:offset 
	movl	$0xff1234ff, %edi		/ BKI interface requirement
	movw	$0x3ff, int_tab_off		/ idtbas = 0, idtlim = 3ff
	movl	$0, [int_tab_off + 2]

	/ search in the GDT for a null descriptor and use it to make an 
	/ alias to our code segment

	xor	%eax, %eax
	xor	%ebx, %ebx
	movw	$GDTSEL, %ax
	movw	%ax, %es
	movw	$FIRST_FREE_SEL, %bx
	movzwl	%es:(%eax), %ecx

check_this:
	cmpl	$0, %es:(%ebx)
	jne	keep_searching
	cmpl	$0, %es:4(%ebx)
	jne	keep_searching
	movw	%bx, cs_alias_seg		/ found it. save in cs_alias_seg
	movw	%cs, %ax
	movl	%es:(%eax), %ecx
	movl	%ecx, %es:(%ebx)
	movl	%es:4(%eax), %ecx
	movl	%ecx, %es:4(%ebx)
	jmp	found_null_des
keep_searching:
	add	$8, %ebx
	cmpl	%ecx, %ebx
	jl		check_this
	movw	%cs, cs_alias_seg		/ if GDT is full, default to cs

found_null_des:
	movl	debug_flag(%ebp), %ecx 
	movl	%ecx, %esi
	cmpl	$0, %ecx
	je	skip_int_check

	/ ecx will contain the int vect.

	movw	$0xffff, %esi			/ set the debug flag
	movw	mem_alias_sel, %ax
	movw	%ax, %es
	
	movw	%es:12, %cx			/ first  the int 3 vector
	shl	$16, %ecx
	movw	%es:14, %cx

	cmpl	$0, %ecx
	jne	skip_int_check

	movw	%es:4, %cx			/ use the int 1 vector
	shl	$16, %ecx
	movw	%es:6, %cx
skip_int_check:

	/ now setup all segment registers with limit = 0xffff, G=0, 
	/ D/B = 0, E=0, W=1, and P=1.  Refer to the iAPX 386 programmer's
	/ reference manual chap 14. on requirements to xfer control to real
	/ mode.

	movw	$GDTSEL, %ax
	movw	%ax, %es
	movzwl	mem_alias_sel, %ebx
	movb	%es:6(%ebx), %al
	andb	$0x10, %al			/ turnoff G, B bits and force
						/ limit (19 - 16) to 0
	movb	%al, %es:6(%ebx)
	movw	$0xffff, %es:(%ebx)		/ set limit to 64K

	movw	%bx, %gs			/ re-load gs and fs
	movw	%bx, %fs

	movw	cs_alias_seg, %bx		/ now the code segment
	movb	%es:6(%ebx), %al
	andb	$0x10, %al			/ turn off G and D bit
	movb	%al, %es:6(%ebx)
	movw	$0xffff, %es:(%ebx)		/ set limit to 64K
 	
	movw	%ds, %bx			/ the data segment
	movb	%es:6(%ebx), %al
	andb	$0x1f, %al			/ turn off G and D bit
	movb	%al, %es:6(%ebx)
	movw	$0xffff, %es:(%ebx)		/ set limit to 64K

	movw	%ss, %bx			/ the stack segent next
	movb	%es:6(%ebx), %al
	andb	$0x10, %al			/ turn off G and D bit
	movb	%al, %es:6(%ebx)
	movw	$0xffff, %es:(%ebx)		/ set limit to 64K
	movl	%esp, %eax
	movw	%bx, %ss			/ re-load ss
	movl	%eax, %esp

	movw	%ds, %ax			/ re-load ds 
	movw	%ax, %ds
	movw	%ax, %es			/ re-load es 

	/ a kludge for a bug in DMON

	mov	%fs:0x400, %eax
	cmpl	$0x2e54424a, %eax		/ JBT. ? - test for itsdmon
	jne	skip_kludge

	mov	$0x8b6, %eax			/ csar field in dump area
	movb	$0x0f, %fs:(%eax)
	mov	$0x898, %eax			/ fslim field in dump area
	mov	$0xffff, %fs:(%eax)
	mov	$0x891, %eax			/ fslim field in dump area
	mov	$0x0f93, %fs:(%eax)

skip_kludge:

	movl	$in_16bit_mode, jmp_vector 
	movw	cs_alias_seg, %ax
	movw	%ax, [jmp_vector + 4]
	ljmp	*jmp_vector		/ this will re-load cs
	nop
	nop

in_16bit_mode:
	movw	%dx, %bx			/ save the offset
	data16
	shr	$16, %edx			/ segment in dx	
	
	cli					/ Unix will enable them
	data16
	movl	%cr0, %eax
	data16
	and	$0x7FFFFFFE, %eax		/ reset PG, PE  bit
	data16
	movl	%eax, %cr0			/ into real mode
	data16
	movl	$0, %eax
	data16
	movl	%eax, %cr3			/ flush TLB

	addr16
	lidt	int_tab_off
	
	/ the processor is in a special mode until the code segment register
	/ gets reloaded again.  We continue in this mode till we transfer
	/ control.  Be advised it is not a good idea to do memory accesses
	/ at this time, only register transfers using immediate operands.

	/ NOTICE: the stack segment has not been re-loaded because we
	/	  want to use the stack for transfer of control.  
	/	  The 386 CPU will allow access to memory > 1M in real
	/	  mode (after switching from PVAM) if the segment register
	/	  has not been loaded.
t1:
	xor	%eax, %eax
 
	movw	%ax, %ds

	pushf 					/ save it on the stack
	pop	%eax
	data16
	andl	$0xffff, %eax
	push	%eax	
	push	%edx				/ push the segment first
	push	%ebx				/ push the offset next
	xor	%eax, %eax

	cmpl	%esi, %eax			 / check the debug flag
	je	no_debug

	pushf 					/ simulate an int x
	pop	%eax
	data16
	andl	$0xffff, %eax
	push	%eax	
	push	%ecx				/ segment of int vector
	data16
	shr	$16, %ecx
	push	%ecx				/ offset of int vector
	iret

no_debug:
	lret					/ head out into the wild 
							/ blue yonder
