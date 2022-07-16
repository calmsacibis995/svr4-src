/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.


#ifdef VPIX

	.ident	"@(#)kern-ml:v86gptrap.s	1.3.1.1"

/ CGA Status Port Register Extension:
/ Copyright (c) 1989 Phoenix Technologies Ltd.
/ All Rights Reserved
/
/
/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

/ VP/ix General Protection Fault handler.  Emulate legal non-I/O
/ exceptions directly: int, cli, sti, pushf, popf, iret.
/ Make others (I/O and illegals) cause a task switch to the ECT at user
/ level.
/ NOTE: THIS CODE IS REACHED THROUGH AN INTERRUPT GATE.  Interrupts
/ are off when we enter here.  We leave them off so that device interrupts
/ do not occur here and so will always return to user mode through v86vint.
/ We can take page faults here, but they are rare enough for it not to
/ matter that v86vint is not called on return to user mode.
/ NOTE: User's segment registers remain in effect, except for CS,
/ throughout all this code.  Kernel DS/ES are never switched in.

#include <vm/faultcatch.h>

/ stack offsets based on GP fault stack frame plus 4 registers

	    .set    OLDIEFL, 12
	    .set    OLDEIP,  20
	    .set    OLDCS,   24
	    .set    OLDEFL,  28
	    .set    OLDESP,  32
	    .set    OLDSS,   36

	    .set    VMFLAG,  0x00020000
	    .set    CLNFLAGS,0x00007000         / Set iopl and nt to 0
	    .set    IFLAG,   0x00000200
	    .set    TFLAG,   0x00000100
	    .set    OPSAVED, 2

	    .set    OP_ARPL, 0x63
	    .set    OP_PUSHF,0x9C
	    .set    OP_POPF, 0x9D
	    .set    OP_INT,  0xCD
	    .set    OP_INTO, 0xCE
	    .set    OP_IRET, 0xCF
	    .set    OP_CLI,  0xFA
	    .set    OP_STI,  0xFB
	    .set    OFLOVEC, 4

	.set	SAVEDX,		4
	.set	SAVEAX,		12
	.set	OP_INB_R,	0xEC
	.set	PORT_VTR,	0x3DA
	.set	EN_VTR,		0x0001

	    .text

	    .globl gptrap

	    .align 4
	    .globl v86gptrap
v86gptrap:
	testl   $VMFLAG,OLDIEFL(%esp)   / Was Virtual 8086 the culprit?
	je      gotrap                  / No..go to regular gp trap logic
					/ (after turning on interrupts)

	pushl	%eax			/ NOTE: these determine
	pushl	%ebx			/ ... stack offset values
	pushl	%edx			/ ... in this code
	pushl   %ebp
	movl    %esp,%ebp               / Set up a stack frame

	movw	$KDSSEL, %ax
	movw	%ax, %ds

	/ CATCH_FAULTS(CATCH_UFAULT)
	movl	$CATCH_UFAULT, u+u_fault_catch+fc_flags

	movzwl  OLDCS(%ebp),%eax        / Get address of offending byte
	addl    %eax, %eax              / prepare for multiply of 16 for cs
	movzwl  OLDEIP(%ebp),%ebx
	leal    (%ebx, %eax, 8), %eax   / Compute %eax = cs * 16 + ip
	movl	$gpfetch_done, u+u_fault_catch+fc_func
	movb    (%eax), %bl             / Fetch the byte 
gpfetch_done:
	movl    XTSSADDR+xt_vflptr,%edx / EDX = Ptr to 8086 virtual flags

	cmpb    $OP_CLI, %bl
	je      op_cli
	cmpb    $OP_STI, %bl
	je      op_sti


	cmpb    $OP_INT, %bl
	je      op_int
	cmpb    $OP_IRET, %bl
	je      op_iret

	cmpb    $OP_PUSHF, %bl
	je      op_pushf
	cmpb    $OP_POPF, %bl
	je      op_popf
	cmpb    $OP_INTO, %bl
	je      op_into

	testl	$EN_VTR,XTSSADDR+xt_op_emul	/ is this opcode enabled?
	je	go_ECT				/ no - passthru to ECT.
	cmpb	$OP_INB_R,%bl			/ is this an inb()?
	jne	go_ECT				/ no - passthru to ECT.
	cmpw	$PORT_VTR,SAVEDX(%ebp)		/ yes - is it for PORT_VTR?
	je	op_inb_vtr			/ yes - go emulate

/
/  Can't emulate here.  Force trap to ECT.
/
go_ECT:
	movb    %bl,XTSSADDR+xt_magictrap       / Save trapped code in XTSS
	movb    $OPSAVED,XTSSADDR+xt_magicstat  / Set flag in XTSS
/ Page fault possible here on some branches.  Could separate them.
	movl    $go_ECT_done,u+u_fault_catch+fc_func
	movb    $OP_ARPL,(%eax)                 / Plant bad opcode

go_ECT_done:
	movl	$fc_jmpjmp, u+u_fault_catch+fc_func	/ Reset standard
							/  fault handler
	lidt    idtdsc2
	popl    %ebp
	popl	%edx
	popl    %ebx
	popl    %eax
	addl    $4,%esp                     / Get error code off the stack
	iret

	.align	4
go1_ECT:
	movzwl  OLDCS(%ebp),%eax            / Get address of offending byte
	addl    %eax, %eax                  / prepare for mult of 16 for cs
	movzwl  OLDEIP(%ebp),%ebx
	leal    (%ebx, %eax, 8), %eax       / Compute %eax = cs * 16 + ip
	movl    $go_ECT_done,u+u_fault_catch+fc_func
	movb    (%eax),%bl                  / Fetch the byte (fault possible)
	jmp     go_ECT                      / Force ECT execution

/
/  Emulate 'cli' opcode
/
	.align	4
op_cli:
	lidt    idtdsc1                 /  Set IDT for kernel operations
	incw    OLDEIP(%ebp)            / Pass opcode on return
	movl	$op_cli_done,u+u_fault_catch+fc_func
	movl	$0,(%edx)		/ Turn off ints in 8086 virtual flags
op_cli_done:
	movl	$fc_jmpjmp, u+u_fault_catch+fc_func	/ Reset standard
							/  fault handler
	lidt    idtdsc2
	popl    %ebp
	popl	%edx
	popl    %ebx
	popl    %eax
	addl    $4,%esp                 / Get error code off the stack
	iret

/
/  Emulate 'sti' opcode
/
	.align	4
op_sti:
	lidt    idtdsc1                 /  Set IDT for kernel operations
	incw    OLDEIP(%ebp)            / Pass opcode on return
	movl	$op_sti_done,u+u_fault_catch+fc_func
	movl	$IFLAG,(%edx)		/ Turn on ints in 8086 virtual flags
op_sti_done:
	testb   $0xFF,XTSSADDR+xt_intr_pin   / Interrupts for virtual machine?
	jnz     go1_ECT                      / Yes - force ECT to post it
	movl	$fc_jmpjmp, u+u_fault_catch+fc_func	/ Reset standard
							/  fault handler
	lidt    idtdsc2
	popl    %ebp
	popl	%edx
	popl    %ebx
	popl    %eax
	addl    $4,%esp                     / Get error code off the stack
	iret

/
/  Emulate 'pushf' opcode
/
	.align	4
op_pushf:
	lidt    idtdsc1                 /  Set IDT for kernel operations
	incw    OLDEIP(%ebp)            /  Pass opcode on return
	subw    $2,OLDESP(%ebp)         /  Pushing 1 16-bit word
	movzwl  OLDSS(%ebp),%eax        / Get address of stack
	addl    %eax, %eax              / prepare for multiply of 16 for ss
	movzwl  OLDESP(%ebp),%ebx
	leal    (%ebx, %eax, 8), %eax   / Compute %eax = ss * 16 + sp
	movw    OLDEFL(%ebp),%bx
	andw    $-1!IFLAG,%bx           /  Prepare to set correct IF bit
	movl	$pushf_done1,u+u_fault_catch+fc_func
	orw	(%edx),%bx		/  Include IF bit in 8086 virtual flags
pushf_done1:
	movl    $pushf_done2,u+u_fault_catch+fc_func
	movw    %bx,(%eax)              /  Put new value on stack
pushf_done2:
	movw	%bx,%ax			/  Get the flags value
	andw	$IFLAG,%ax		/  Isolate the IF status
	jmp     comf                    /  %ax = Virtual IF status

/
/  Emulate 'popf' opcode
/
	.align	4
op_popf:
	lidt    idtdsc1                 /  Set IDT for kernel operations
	incw    OLDEIP(%ebp)            /  Pass opcode on return
	movzwl  OLDSS(%ebp),%eax        / Get address of stack
	addl    %eax, %eax              / prepare for multiply of 16 for ss
	movzwl  OLDESP(%ebp),%ebx
	leal    (%ebx, %eax, 8), %eax   / Compute %eax = ss * 16 + sp
	addw    $2,OLDESP(%ebp)         /  Pop 1 16-bit word
	movl    $popf_done,u+u_fault_catch+fc_func
	movw    (%eax),%bx              /  Get new value from stack
	movw    %bx,%ax
	andw    $IFLAG,%ax
	movl	$popf_done,u+u_fault_catch+fc_func
	movw	%ax,(%edx)		/  Save the interrupt flag status
popf_done:
	andw    $-1!CLNFLAGS,%bx        /  Make sure IOPL=0, NT=0
	orw     $IFLAG,%bx              /  Don't let real I bit be off
	movw    %bx,OLDEFL(%ebp)        /  Replace flags plus I-bit

comf:                                   /  NOTE: %ax = Virtual IF status
	cmpw    $0,%ax                       / Interrupt flag set?
	je      comf1                        / No - no interrupt processing
	testb   $0xFF,XTSSADDR+xt_intr_pin   / Interrupts for virtual machine?
	jnz     go1_ECT                      / Yes - force ECT to post it
comf1:
	movl	$fc_jmpjmp, u+u_fault_catch+fc_func	/ Reset standard
							/  fault handler
	lidt    idtdsc2
	popl    %ebp
	popl	%edx
	popl    %ebx
	popl    %eax
	addl    $4,%esp                 / Get error code off the stack
	iret

/
/  Emulate 'iret' opcode
/
	.align	4
op_iret:
	lidt    idtdsc1                 /  Set IDT for kernel operations
	incw    OLDEIP(%ebp)            /  Pass opcode on return
	movzwl  OLDSS(%ebp),%eax        / Get address of stack
	addl    %eax, %eax              / prepare for multiply of 16 for ss
	movzwl  OLDESP(%ebp),%ebx
	leal    (%ebx, %eax, 8), %eax   / Compute %eax = ss * 16 + sp
	addw    $6,OLDESP(%ebp)
	movl    $iret_done,u+u_fault_catch+fc_func
	movl    (%eax),%ebx             /  Get new CS & IP from user stack
	movw    %bx,OLDEIP(%ebp)        /  Fix up low EIP on curr stack
	roll    $16,%ebx                /  Exchange %bxh & %bxl
	movw    %bx,OLDCS(%ebp)         /  Fix up CS on curr stack
	movl    $iret_done,u+u_fault_catch+fc_func
	movw    4(%eax),%bx             /  Get flags
	andw    $-1!CLNFLAGS,%bx        /  Set IOPL=0, NT=0
	movw    %bx,%ax
	andw    $IFLAG,%ax
	movl	$iret_done1,u+u_fault_catch+fc_func
	movw	%ax,(%edx)
iret_done1:
	orw     $IFLAG,%bx              /  Don't let real I bit be off
	movw    %bx,OLDEFL(%ebp)
iret_done:
	jmp     comf                    / %ax = Virtual IF status

/
/  Emulate 'int'  and 'into' opcodes
/  Note: int3 opcode is emulated in the ECT
/
	.align	4
op_into:
	lidt    idtdsc1                     /  Set IDT for kernel operations
	pushl   %ecx
	movl    $OFLOVEC,%ecx               /  Point to vector
	btl     %ecx,XTSSADDR+xt_imaskbits  /  Does ECT want control for INTO?
	jc      int_go_ect                  /  Yes - force control to ECT
	decw    OLDEIP(%ebp)                /  Kludge for add of 2 below
	jmp     all_ints

	.align	4
op_int:
	lidt    idtdsc1                 /  Set IDT for kernel operations
	pushl   %ecx                    /  Opcode ptr still in %eax
	movl    $int_done,u+u_fault_catch+fc_func  /  Next byte could be
						   /   in next page
	movzbl  1(%eax),%ecx            /  Get interrupt number
	btl     %ecx,XTSSADDR+xt_imaskbits  /  Does ECT want control for INT?
	jc      int_go_ect              /  Yes - force control to ECT
all_ints:
	shll    $2,%ecx                 /  Multiply by 4 for vector ptr
	subw    $6,OLDESP(%ebp)         /  Adjust stack for user CS:IP, flags
	movzwl  OLDSS(%ebp),%eax        /  Get address of stack
	addl    %eax, %eax              /  prepare for multiply of 16 for ss
	movzwl  OLDESP(%ebp),%ebx
	leal    (%ebx, %eax, 8), %eax   /  Compute %eax = ss * 16 + sp
	movw    OLDCS(%ebp),%bx         /  Get old CS
	shll    $16,%ebx                /  Move up the old CS to top
	movw    OLDEIP(%ebp),%bx        /  Get old IP from curr stack
	addw    $2,%bx                  /  Skip INT inst. when done
	movl    $int_done,u+u_fault_catch+fc_func
	movl    %ebx,(%eax)             /  Put new CS & IP on user stack
	movw    OLDEFL(%ebp),%bx        /  Get current flags
	andw    $-1![IFLAG|TFLAG],%bx   /  Prepare to set correct IF,TF bits
	movl	$int_done1,u+u_fault_catch+fc_func
	orw	(%edx),%bx		/  Include virtual flags I-bit
int_done1:
	movl    $int_done,u+u_fault_catch+fc_func
	movw    %bx,4(%eax)             /  Push flags on user stack
	movl	$int_done2,u+u_fault_catch+fc_func
	movl	$0,(%edx)
int_done2:
	movl    $int_done,u+u_fault_catch+fc_func
	movl    (%ecx),%ebx             /  Get 8086 interrupt vector
	movw    %bx,OLDEIP(%ebp)        /  Set him up to go back to isr
	shrl    $16,%ebx
	movw    %bx,OLDCS(%ebp)
int_done:
	popl    %ecx
	movl	$fc_jmpjmp, u+u_fault_catch+fc_func	/ Reset standard
							/  fault handler
	lidt    idtdsc2
	popl    %ebp
	popl	%edx
	popl    %ebx
	popl    %eax
	addl    $4,%esp                     / Get error code off the stack
	iret

	.align	4
int_go_ect:
	popl    %ecx
	movl    $go_ECT_done,u+u_fault_catch+fc_func
	jmp     go_ECT

	.align	4
gotrap:
	sti                            / Because ints were off on entry
	jmp     gptrap

/
/ Extension for CGA Status Port Read
/
	.align	4
	.globl	cs_table_beg
	.globl	cs_table_ptr
	.globl	cs_table_end
op_inb_vtr:
	movl	cs_table_ptr,%edx	/ get address of current status
	movl	SAVEAX(%ebp),%eax	/ get original eax (ah) contents
	movb	(%edx),%al		/ put new status in AL
	incl	%edx
	cmpl	%edx,cs_table_end	/ have we overflowed?
	ja	vtr_cs_ok		/ no
	movl	cs_table_beg,%edx	/ yes - reset the ptr
vtr_cs_ok:
	movl	%edx,cs_table_ptr
	incw	OLDEIP(%ebp)		/ skip OPcode on return
	movl	$fc_jmpjmp, u+u_fault_catch+fc_func	/ Reset standard
							/  fault handler
	popl	%ebp
	popl	%edx
	popl	%ebx
	addl	$8,%esp			/ %eax (%al) has new value
					/ and get error code off stack
	iret

#endif /* VPIX */
