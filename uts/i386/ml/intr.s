/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

#ident	"@(#)kern-ml:intr.s	1.3.1.1"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

/ Dual-Mode floating point support:
/ Copyright (c) 1989 Phoenix Technologies Ltd.
/ All Rights Reserved

	.set	IS_LDT_SEL,	0x04
#ifdef MERGE386
	.set	VMFLAG, 0x20000
#endif /* MERGE386 */

	.data

#ifdef  VPIX
	.globl  v86procflag
#endif
	.globl  curproc

	.text

#ifdef  VPIX
	.globl  idtdsc1
#endif
	.globl	cmnint
	.globl	cmntrap

	.align	4
	.globl	div0trap
div0trap:
	pushl	$0
	pushl	$0
	jmp	cmntrap

	.globl	mon1sel
	.globl	mon1off

	.align	4
	.globl	dbgtrap
dbgtrap:
/ This dmon stuff causes a kernel page-fault
/ when a there is a single-step through an lcall.
/ Otherwise, it almost works.
/testw	$IS_LDT_SEL, 4(%esp)
/jnz	nodbgmon
/testw	$0xFFFF, %cs:mon1sel
/jz	nodbgmon

/ A little explaination of the following code:
/	The 2 pushes make space for the selector and offset of
/	the monitor entry point on the stack AND save eax.
/	Note that the second xchgl operation will restore eax
/	to its original value, thus when we do the retl, it will
/	look to the monitor like we did the "int" directly to it.
/cli
/pushl	%eax
/pushl	%eax
/movl	%cs:mon1sel, %eax
/xchgl	%eax, 4(%esp)
/movl	%cs:mon1off, %eax
/xchgl	%eax, (%esp)
/lret
	.align	4
nodbgmon:
	pushl	$0
	pushl	$1
	jmp	cmntrap

	.align	4
	.globl	nmiint
nmiint:
	pushl	$0
	pushl	$2
	jmp	cmntrap

	.globl	mon3sel
	.globl	mon3off

	.align	4
	.globl	brktrap
brktrap:
#ifdef MERGE386
	testl	$VMFLAG, 8(%esp)
	jnz	nobrkmon
#endif /* MERGE386 */
	testw	$IS_LDT_SEL, 4(%esp)
	jnz	nobrkmon
	testw	$0xFFFF, %cs:mon3sel
	jz	nobrkmon

/ See the comment in dbgtrap.
	cli
	pushl	%eax
	pushl	%eax
	movl	%cs:mon3sel, %eax
	xchgl	%eax, 4(%esp)
	movl	%cs:mon3off, %eax
	xchgl	%eax, (%esp)
	lret

	.align	4
nobrkmon:
	pushl	$0
	pushl	$3
	jmp	cmntrap

	.align	4
	.globl	ovflotrap
ovflotrap:
	pushl	$0
	pushl	$4
	jmp	cmntrap

	.align	4
	.globl	boundstrap
boundstrap:
	pushl	$0
	pushl	$5
	jmp	cmntrap

	.align	4
	.globl	invoptrap
invoptrap:
	pushl	$0
	pushl	$6
	jmp	cmntrap

	.align	4
	.globl	ndptrap
#ifdef VPIX
	.globl	ndptrap0
	.globl	ndptrap2
	.globl	ndptrap3
	.globl	ndptrap4
	.globl	EM80387
ndptrap0:
	pushl	%eax
	movl	$0, %eax
	jmp	ndptrap
	.align	4
ndptrap2:
	pushl	%eax
	movl	$1, %eax
	jmp	ndptrap
	.align	4
ndptrap3:
	pushl	%eax
	movl	$2, %eax
	jmp	ndptrap
	.align	4
ndptrap4:
	pushl	%eax
	movl	$3, %eax
	jmp	ndptrap
	.align	4
#endif

ndptrap:
#ifdef VPIX
	.globl	v86procflag
	.set	OLDFLAGS, 16
	.set 	VMFLAG,	0x00020000
	.set 	FPINIT, 0x1	

	cmpb    $0,%cs:v86procflag      / Is this a v86 process?
	je      procndp                 / No - then process trap as usual
	cmpb    $FP_NO,%cs:fp_kind      / Is there no fp unit or emuation?
	je      ndptrap1                / No unit or emulation - spurious intr
	cmpb	$FP_SW,%cs:fp_kind	/ Is there emulation?
	jne	procndp			/ No, there's a chip - usual process
	testl	$VMFLAG,OLDFLAGS(%esp)	/ Was it in V86 mode?
	jne	setemandemul		/ No, must set EM and emulate
	pushl   %eax                    / Save the user reg
	movl	%cr0,%eax
	andl	$-1!CR0_EM, %eax	/ Turn off EM bit
	movl	%eax,%cr0
	popl    %eax                    / Restore user register
ndptrap1:
	popl    %eax                    / Restore user register
	clts                            / Try to prevent further spurious ints
	iret                            / Exit trap interrupt routine

	.align	4
setemandemul:
	pushl	%eax			/ Must set EM mode back on
	movl	%cr0,%eax
	orl	$CR0_EM,%eax
	movl	%eax,%cr0
	popl	%eax

	andl	$FPINIT, %eax
	cmpl	$FPINIT, %eax
	je	procndp

	movl	8(%esp), %eax
	cmpw	$USER_CS, %ax
	jne	procndp
	
	movl	%esp, %eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	pushl	%fs
	pushl	%gs
	
	pushl	%esi
	pushl	%edi
	pushl	%ebp

	movl	%eax, %esi
	movl	%esp, %ebp

/	Fix the return address on the user stack from the fp emulator

	movw	20(%esi), %fs		/ USER SS SEGMENT
	movl	16(%esi), %edi		/ USER ESP
	movl	12(%esi), %ebx		/ USER FLAGS
	movl	 8(%esi), %ecx		/ USER CS SEGMENT
	movl	 4(%esi), %edx		/ USER EIP 

	pushl	%ebx			/ USER FLAGS
	pushl	%ecx			/ USER CS SEGMENT
	pushl	%edx			/ USER EIP 
	movl	%esp, %eax

	subl	$12, %edi	
	pushl	$12		/ 3 longs
	pushl	%edi		/ user stack
	pushl	%eax		/ kernel stack
	call	copyout

	movl	%ebp, %esp		/ restore %ESP

	movl	%edi, 16(%esi)
	movl	$FPESEL, 8(%esi)

/	store the %eip value (EM80387) so we can jump to after iret
	movl	%cs:EM80387, %eax
	movl	%eax, 4(%esi)

	popl	%ebp
	popl	%edi
	popl	%esi

	popl	%gs
	popl	%fs

	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%eax

	iret

	.align	4
procndp:
	popl	%eax
#endif /* VPIX */
	pushl	$0
	pushl	$7
	jmp	cmntrap

	.align	4
	.globl	syserrtrap
syserrtrap:
	movl    curproc,%ebx    / Get pointer to current proc struct
	movl    $kpd0-KVSBASE,%eax / Get kernel CR3
	orl	fp387cr3,%eax
	movl    %eax,%cr3       / Set the handler CR3 to fault process'
#ifdef  VPIX
	cmpb    $0,v86procflag  / Is the process a dual mode process?
	jz      cont_dbl        / No - process double fault
	lidt    %cs:idtdsc1     / Reset to original interrupt table
	addl    $4,%esp         / Get rid of error code on stack
	mov     $0,v86procflag  / Reset to normal mode
	iret                    / Task switch and refault (GP this time)
	jmp     syserrtrap      / Process double fault

	.align	4
cont_dbl:
#endif
	popl    %eax            / Get the error code
	pushl   $0              / Fake SS
	pushl   $0              / Fake ESP
	pushl   $0              / Fake interrupt stack (flags)
	pushl   $0              / Fake CS
	pushl   $0              / Fake EIP
	pushl   %eax            / Error code on stack
	pushl   $8              / Trap number for default handler
	jmp	cmntrap

	.align	4
	.globl	overrun
overrun:
	pushl	$0
	pushl	$9
	jmp	cmntrap

	.align	4
	.globl	invtsstrap
invtsstrap:
	pushl	$10		/ already have error code on stack
	jmp	cmntrap

	.align	4
	.globl	segnptrap
segnptrap:
	pushl	$11		/ already have error code on stack
	jmp	cmntrap

	.align	4
	.globl	stktrap
stktrap:
	pushl	$12		/ already have error code on stack
	jmp	cmntrap

	.align	4
	.globl	gptrap
gptrap:
	pushl	$13		/ already have error code on stack
	jmp	cmntrap

	.align	4
	.globl	pftrap
pftrap:
	pushl	$14		/ already have error code on stack
	jmp	cmntrap

	.align	4
	.globl	resvtrap
resvtrap:
	pushl	$0
	pushl	$15
	jmp	cmntrap

	.align	4
	.globl	ndperr
ndperr:
	pushl	$0
	pushl	$16
	jmp	cmntrap

	.align	4
	.globl	invaltrap
invaltrap:
	pushl	$0
	pushl	$17
	jmp	cmntrap

	.align	4
	.globl	invalint
invalint:
	pushl	$0
	pushl	$17
	jmp	cmnint

	.align	4
	.globl	ivctM0
ivctM0:
	pushl	$0
	pushl	$0
	jmp	cmnint

	.align	4
	.globl	ivctM1
ivctM1:
	pushl	$0
	pushl	$1
	jmp	cmnint

	.align	4
	.globl	ivctM2
ivctM2:
	pushl	$0
	pushl	$2
	jmp	cmnint

	.align	4
	.globl	ivctM3
ivctM3:
	pushl	$0
	pushl	$3
	jmp	cmnint

	.align	4
	.globl	ivctM4
ivctM4:
	pushl	$0
	pushl	$4
	jmp	cmnint

	.align	4
	.globl	ivctM5
ivctM5:
	pushl	$0
	pushl	$5
	jmp	cmnint

	.align	4
	.globl	ivctM6
ivctM6:
	pushl	$0
	pushl	$6
	jmp	cmnint

	.align	4
	.globl	ivctM7
ivctM7:
	pushl	$0
	pushl	$7
	jmp	cmnint

	.align	4
	.globl	ivctM0S0
ivctM0S0:
	pushl	$0
	pushl	$8
	jmp	cmnint

	.align	4
	.globl	ivctM0S1
ivctM0S1:
	pushl	$0
	pushl	$9
	jmp	cmnint

	.align	4
	.globl	ivctM0S2
ivctM0S2:
	pushl	$0
	pushl	$10
	jmp	cmnint

	.align	4
	.globl	ivctM0S3
ivctM0S3:
	pushl	$0
	pushl	$11
	jmp	cmnint

	.align	4
	.globl	ivctM0S4
ivctM0S4:
	pushl	$0
	pushl	$12
	jmp	cmnint

	.align	4
	.globl	ivctM0S5
ivctM0S5:
	pushl	$0
	pushl	$13
	jmp	cmnint

	.align	4
	.globl	ivctM0S6
ivctM0S6:
	pushl	$0
	pushl	$14
	jmp	cmnint

	.align	4
	.globl	ivctM0S7
ivctM0S7:
	pushl	$0
	pushl	$15
	jmp	cmnint

	.align	4
	.globl	ivctM1S0
ivctM1S0:
	pushl	$0
	pushl	$16
	jmp	cmnint

	.align	4
	.globl	ivctM1S1
ivctM1S1:
	pushl	$0
	pushl	$17
	jmp	cmnint

	.align	4
	.globl	ivctM1S2
ivctM1S2:
	pushl	$0
	pushl	$18
	jmp	cmnint

	.align	4
	.globl	ivctM1S3
ivctM1S3:
	pushl	$0
	pushl	$19
	jmp	cmnint

	.align	4
	.globl	ivctM1S4
ivctM1S4:
	pushl	$0
	pushl	$20
	jmp	cmnint

	.align	4
	.globl	ivctM1S5
ivctM1S5:
	pushl	$0
	pushl	$21
	jmp	cmnint

	.align	4
	.globl	ivctM1S6
ivctM1S6:
	pushl	$0
	pushl	$22
	jmp	cmnint

	.align	4
	.globl	ivctM1S7
ivctM1S7:
	pushl	$0
	pushl	$23
	jmp	cmnint

	.align	4
	.globl	ivctM2S0
ivctM2S0:
	pushl	$0
	pushl	$24
	jmp	cmnint

	.align	4
	.globl	ivctM2S1
ivctM2S1:
	pushl	$0
	pushl	$25
	jmp	cmnint

	.align	4
	.globl	ivctM2S2
ivctM2S2:
	pushl	$0
	pushl	$26
	jmp	cmnint

	.align	4
	.globl	ivctM2S3
ivctM2S3:
	pushl	$0
	pushl	$27
	jmp	cmnint

	.align	4
	.globl	ivctM2S4
ivctM2S4:
	pushl	$0
	pushl	$28
	jmp	cmnint

	.align	4
	.globl	ivctM2S5
ivctM2S5:
	pushl	$0
	pushl	$29
	jmp	cmnint

	.align	4
	.globl	ivctM2S6
ivctM2S6:
	pushl	$0
	pushl	$30
	jmp	cmnint

	.align	4
	.globl	ivctM2S7
ivctM2S7:
	pushl	$0
	pushl	$31
	jmp	cmnint

	.align	4
	.globl	ivctM3S0
ivctM3S0:
	pushl	$0
	pushl	$32
	jmp	cmnint

	.align	4
	.globl	ivctM3S1
ivctM3S1:
	pushl	$0
	pushl	$33
	jmp	cmnint

	.align	4
	.globl	ivctM3S2
ivctM3S2:
	pushl	$0
	pushl	$34
	jmp	cmnint

	.align	4
	.globl	ivctM3S3
ivctM3S3:
	pushl	$0
	pushl	$35
	jmp	cmnint

	.align	4
	.globl	ivctM3S4
ivctM3S4:
	pushl	$0
	pushl	$36
	jmp	cmnint

	.align	4
	.globl	ivctM3S5
ivctM3S5:
	pushl	$0
	pushl	$37
	jmp	cmnint

	.align	4
	.globl	ivctM3S6
ivctM3S6:
	pushl	$0
	pushl	$38
	jmp	cmnint

	.align	4
	.globl	ivctM3S7
ivctM3S7:
	pushl	$0
	pushl	$39
	jmp	cmnint

	.align	4
	.globl	ivctM4S0
ivctM4S0:
	pushl	$0
	pushl	$40
	jmp	cmnint

	.align	4
	.globl	ivctM4S1
ivctM4S1:
	pushl	$0
	pushl	$41
	jmp	cmnint

	.align	4
	.globl	ivctM4S2
ivctM4S2:
	pushl	$0
	pushl	$42
	jmp	cmnint

	.align	4
	.globl	ivctM4S3
ivctM4S3:
	pushl	$0
	pushl	$43
	jmp	cmnint

	.align	4
	.globl	ivctM4S4
ivctM4S4:
	pushl	$0
	pushl	$44
	jmp	cmnint

	.align	4
	.globl	ivctM4S5
ivctM4S5:
	pushl	$0
	pushl	$45
	jmp	cmnint

	.align	4
	.globl	ivctM4S6
ivctM4S6:
	pushl	$0
	pushl	$46
	jmp	cmnint

	.align	4
	.globl	ivctM4S7
ivctM4S7:
	pushl	$0
	pushl	$47
	jmp	cmnint

	.align	4
	.globl	ivctM5S0
ivctM5S0:
	pushl	$0
	pushl	$48
	jmp	cmnint

	.align	4
	.globl	ivctM5S1
ivctM5S1:
	pushl	$0
	pushl	$49
	jmp	cmnint

	.align	4
	.globl	ivctM5S2
ivctM5S2:
	pushl	$0
	pushl	$50
	jmp	cmnint

	.align	4
	.globl	ivctM5S3
ivctM5S3:
	pushl	$0
	pushl	$51
	jmp	cmnint

	.align	4
	.globl	ivctM5S4
ivctM5S4:
	pushl	$0
	pushl	$52
	jmp	cmnint

	.align	4
	.globl	ivctM5S5
ivctM5S5:
	pushl	$0
	pushl	$53
	jmp	cmnint

	.align	4
	.globl	ivctM5S6
ivctM5S6:
	pushl	$0
	pushl	$54
	jmp	cmnint

	.align	4
	.globl	ivctM5S7
ivctM5S7:
	pushl	$0
	pushl	$55
	jmp	cmnint

	.align	4
	.globl	ivctM6S0
ivctM6S0:
	pushl	$0
	pushl	$56
	jmp	cmnint

	.align	4
	.globl	ivctM6S1
ivctM6S1:
	pushl	$0
	pushl	$57
	jmp	cmnint

	.align	4
	.globl	ivctM6S2
ivctM6S2:
	pushl	$0
	pushl	$58
	jmp	cmnint

	.align	4
	.globl	ivctM6S3
ivctM6S3:
	pushl	$0
	pushl	$59
	jmp	cmnint

	.align	4
	.globl	ivctM6S4
ivctM6S4:
	pushl	$0
	pushl	$60
	jmp	cmnint

	.align	4
	.globl	ivctM6S5
ivctM6S5:
	pushl	$0
	pushl	$61
	jmp	cmnint

	.align	4
	.globl	ivctM6S6
ivctM6S6:
	pushl	$0
	pushl	$62
	jmp	cmnint

	.align	4
	.globl	ivctM6S7
ivctM6S7:
	pushl	$0
	pushl	$63
	jmp	cmnint

	.align	4
	.globl	ivctM7S0
ivctM7S0:
	pushl	$0
	pushl	$64
	jmp	cmnint

	.align	4
	.globl	ivctM7S1
ivctM7S1:
	pushl	$0
	pushl	$65
	jmp	cmnint

	.align	4
	.globl	ivctM7S2
ivctM7S2:
	pushl	$0
	pushl	$66
	jmp	cmnint

	.align	4
	.globl	ivctM7S3
ivctM7S3:
	pushl	$0
	pushl	$67
	jmp	cmnint

	.align	4
	.globl	ivctM7S4
ivctM7S4:
	pushl	$0
	pushl	$68
	jmp	cmnint

	.align	4
	.globl	ivctM7S5
ivctM7S5:
	pushl	$0
	pushl	$69
	jmp	cmnint

	.align	4
	.globl	ivctM7S6
ivctM7S6:
	pushl	$0
	pushl	$70
	jmp	cmnint

	.align	4
	.globl	ivctM7S7
ivctM7S7:
	pushl	$0
	pushl	$71
	jmp	cmnint

#ifdef VPIX
	.align	4
	.globl	setem
setem:
	pushl	%eax
	movl	%cr0,%eax
	orl	$CR0_EM,%eax
	movl	%eax,%cr0
	popl	%eax
	ret
#endif
