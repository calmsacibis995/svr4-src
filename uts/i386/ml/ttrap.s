/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.ident	"@(#)kern-ml:ttrap.s	1.4.3.1"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

#include "sys/ipl.h"
#include "sys/pic.h"
#include "sys/reg.h"
#include "symvals.h"

	.set    IS_LDT_SEL, 0x04        / GDT/LDT bit in selector
	.set    F_OFF, 0x2		/ kernel mode flags, interrupts off
	.set	F_ON, 0x202		/ kernel mode flags, interrupts on
	.set	IE, 0x200
#if defined(VPIX) || defined(MERGE386)
	.set    VMFLAG, 0x00020000
	.set    ACCESS_BYTE, 5
	.set    NT_BIT, 0x4000
	.set    TASK_BUSY, 0x2
	.set    IOPL, 0x00000000
#endif /* VPIX || MERGE386 */

#ifdef  KPERF
        .set    KPT_INTR, 1     	/ interrupt entry point
        .set    KPT_TRAP_RET, 2 	/ return from trap to user mode
        .set    KPT_INT_KRET, 3 	/ return from interrupt to kernel mode
        .set    KPT_INT_URET, 4 	/ return from interrupt to user mode
#endif  /* KPERF */


	.globl	curproc
	.globl	runrun
	.globl	ivect
	.globl	queueflag
	.globl	qrunflag
	.globl	queuerun
	.globl  intpri
	.globl	dotimein

	.globl	u_trap
	.globl	k_trap
	.globl	systrap
	.globl	clock_int
	.globl	swtch
	.globl  addupc
	.globl	timein
	.globl  ipl             / current interrupt priority level
#ifdef  VPIX
	.globl  v86vint
#endif

#ifdef  KPERF
        .globl  kperf_write     / kperf.c: kperf_write(type, pc, kproc)
        .globl  kpftraceflg     / flag to enable Kernel PERFormance measurement
#endif  /* KPERF */


/
/ Kernel entry/exit hook arrays (null-terminated).
/
	.globl	io_kenter	/ extern int (*io_kenter[])();
	.globl	io_kexit	/ extern int (*io_kexit[])();

	.data
	.align	4

kentered:	.long	0	/ The kernel has been entered from user level
				/ This is actually a nesting count

intr_entered:	.long	0	/ Interrupts are being processed
				/ This is actually a nesting count

/
/ Hooks for debuggers.
/
	.text
	.align	4
nullhook:
	xorl	%eax,%eax
	ret

	.data
	.globl	nmi_hook
nmi_hook:	.long	nullhook

#ifndef NODEBUGGER
	.globl	nullsys
	.globl	debug_level
	.globl	cdebugger
debug_level:	.long	0
cdebugger:	.long	nullsys
user_owns_dbregs:	.long	0
save_kernel_db0:	.long	0
save_kernel_db1:	.long	0
save_kernel_db2:	.long	0
save_kernel_db3:	.long	0
save_kernel_db7:	.long	0
#endif /* NODEBUGGER */

	.text
#ifdef VPIX
	.globl  idt
	.globl  idt2
	.align	8
idtdsc1:
	.value	[8\*256-1]
	.long	idt
	.align	8
idtdsc2:
	.value	[8\*256-1]
	.long   idt2
	.align  8
#endif

/ Handle kernel entry hooks.
/ Note that these are called with interrupts disabled.

	.align	4
kentry_check:
	movl	kentered, %eax	/ Get kernel-entered nesting count
	incl	%eax		/ Increment nesting count
	movl	%eax, kentered
	cmpl	$1, %eax	/ If not first entry,
	jne	kentry_done	/    don't call hooks
	movl	$io_kenter, %esi

kentry_loop:
	lodsl			/ Get next function pointer
	orl	%eax, %eax
	jz	kentry_entered	/ Loop done if zero
	call	*%eax		/ Indirect call to hook function
	jmp	kentry_loop

kentry_entered:
#ifndef NODEBUGGER
	/ See if need to restore debug registers
	cmpl	%eax, user_owns_dbregs	/ Only needed if user owns debug regs
	jz	kentry_done
	movl	%eax, %db7		/ Disable debug traps for now
	movl	%db6, %eax		/ Save user's DR6
	movl	%eax, u+u_debugreg+24
	xorl	%eax, %eax		/ Clear DR6 for kernel
	movl	%eax, %db6
	movl	save_kernel_db0, %eax	/ Restore kernel's DR0
	movl	%eax, %db0
	movl	save_kernel_db1, %eax	/ Restore kernel's DR1
	movl	%eax, %db1
	movl	save_kernel_db2, %eax	/ Restore kernel's DR2
	movl	%eax, %db2
	movl	save_kernel_db3, %eax	/ Restore kernel's DR3
	movl	%eax, %db3
	movl	save_kernel_db7, %eax	/ Restore kernel's DR7
	movl	%eax, %db7
	xorl	%eax, %eax
	movl	%eax, user_owns_dbregs
#endif /* NODEBUGGER */

kentry_done:
	ret

/ Handle kernel exit hooks.
/ Note that these are called with interrupts disabled.

	.align	4
kexit_check:
	decl	kentered	/ Decrement kernel-entered nesting count
	js	kexit_neg
	jnz	kexit_done	/ If not last exit, don't call hooks
	movl	$io_kexit, %esi

kexit_loop:
	lodsl			/ Get next function pointer
	orl	%eax, %eax
	jz	kexit_done	/ All done if zero
	call	*%eax		/ Indirect call to hook function
	jmp	kexit_loop

	.align	4
kexit_done:
	ret

	.align	4
kexit_neg:
	movl	$0, kentered	/ Nesting count went negative, clear it
	jmp	kexit_done

/
/  servicing_interrupt -- returns non-zero if interrupt servicing in progress
/
	.align	4
	.globl	servicing_interrupt

servicing_interrupt:
	movl	intr_entered, %eax
	ret

/
/  The common entry point for traps from entry points in intr.s
/  For stack layout, see reg.h
/  When cmntrap gets called, the error code and trap number have been pushed.
/
/  **** Code in misc.s assumes %ds == %es. Initialize %ds and %es in the
/       various kernel entry points.

	.align	4
	.globl	cmntrap
cmntrap:
#ifdef VPIX
	lidt    %cs:idtdsc1     / Note: don't care about DS..pure linear
#endif
	pusha			/ save all registers
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs
	pushl	$F_OFF
	popfl

	movw	$KDSSEL, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movl	%esp, %ebp

/	We have to clear fs, gs to guarantee that they are not illegal
/	in the context of the descending process, otherwise the jmp
/	$JTSSSEL in swtch() results in an invalid tss exception.

	xorw	%ax, %ax
	movw	%ax, %fs
	movw	%ax, %gs

	call	kentry_check	/ check for kernel entry hooks

	testl	$IE, [EFL\*4](%ebp)	/ enable interrupts only if they were
	jz	trap_no_sti		/   enabled at the time of the trap
	sti
trap_no_sti:

	/ check for nmi trap
	cmpl	$2,[TRAPNO\*4](%ebp)
	jne	not_nmi

	pushl	%esp		/ argument to trap handler
	call	*nmi_hook	/ hook for NMI Debugging Board
	addl	$4, %esp	/ get argument off stack

	orl	%eax,%eax	/ if nmi_hook returns non-zero,
	jnz	ret_intr	/    we bypass normal handling
	
not_nmi:
	pushl	%esp		/ argument to trap handler
	/ see if trap was in kernel or user mode
#ifdef	MERGE386
	cmpl	$0, merge386enable
	je	notvm86trap
	testl	$VMFLAG, [EFL\*4](%ebp)	/ have we come from a V86 process ?
	jz	notvm86trap		/ if not, continue with the unix
	call	vm86_trap		/ call the vm86 trap handler
	or	%eax, %eax		/ if error is returned,
	jnz	notvm86trap		/ continue and call u_trap
	addl	$4, %esp		/ otherwise
	jmp	ret_user		/ return
notvm86trap:
#endif	/* MERGE386 */
	movl    [CS\*4](%ebp), %eax
#if defined(VPIX) || defined(MERGE386)
	testl   $VMFLAG, [EFL\*4](%ebp)  / if he was in V86 mode, the coming
	jnz     user_trap              / IS_LDT_SEL test is meaningless
#endif
	testw	$IS_LDT_SEL, %ax
	jz	kern_trap

user_trap:
	movb	$0, u+u_sigfault    / don't catch signal processing GP faults
	call	u_trap		/ u_trap() handles all user traps
	addl	$4, %esp	/ get argument off stack
	jmp     ret_user        / jump to common return to user code

	.align	4
kern_trap:
	cmpw	$FPESEL, %ax	/ are we running the fp emulator?
	je	user_trap
	call	k_trap		/ handle kernel trap
	movb	$0, u+u_sigfault    / don't catch signal processing GP faults
	addl	$4, %esp	/ get argument off stack
	jmp     do_ret


/
/  System call handler.  This is the destination of the call gate.
/
	.align	4
	.globl	sys_call
sys_call:
	/ set up the stack to look as in reg.h
#ifdef VPIX
	lidt    %cs:idtdsc1     / Note: don't care about DS..pure linear
#endif
	subl	$8, %esp	/ pad the stack with dummy ERRCODE, TRAPNO
	pusha			/ save user registers
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	pushf
	popl	%eax
	movl    %eax, [EFL\*4](%esp)

	movw	$KDSSEL, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movl	%esp, %ebp
	pushl	$F_OFF
	popfl

/	We have to clear fs, gs to guarantee that they are not illegal
/	in the context of the descending process, otherwise the jmp
/	$JTSSSEL in swtch() results in an invalid tss exception.

	xorw	%ax, %ax
	movw	%ax, %fs
	movw	%ax, %gs

	call	kentry_check	/ check for kernel entry hooks
	sti

	movb	$0, u+u_sigfault    / don't catch signal processing GP faults
	pushl	%esp		/ argument to systrap
	call	systrap
	addl	$4, %esp
	jmp     ret_user        / jump to common return to user code


/
/ Handle external (I/O) interrupts.
/
/ This routine is passed an index into ivect[] by the code in intr.s.
/ It calls splint() to raises the interrupt priority level to the level
/ assigned to the interrupt by intpri[] and send EOI (end-of-interrupt)
/ to the master and (perhaps) slave PICs.
/ It then re-enables interrupts and calls the interrupt routine specified
/ in ivect[].
/ clock() is special-cased since it takes different arguments and runs with
/ all interrupts disabled.
/ When the interrupt routine returns, the interrupt priority level is
/ restored to what it was at the time of the interrupt.
/

	.align	4
	.globl	cmnint
cmnint:
#ifdef VPIX
	lidt    %cs:idtdsc1     / Note: don't care about DS..pure linear
#endif
	pusha
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	movw	$KDSSEL, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movl	%esp, %ebp
	pushl	$F_OFF
	popfl

/	We have to clear fs, gs to guarantee that they are not illegal
/	in the context of the descending process, otherwise the jmp
/	$JTSSSEL in swtch() results in an invalid tss exception.

	xorw	%ax, %ax
	movw	%ax, %fs
	movw	%ax, %gs

	incl	intr_entered		/ servicing a new interrupt

	call	kentry_check		/ check for kernel entry hooks

	movb	$0, u+u_sigfault    / don't catch signal processing GP faults

	/ raise interrupt priority level
	movl    [TRAPNO\*4](%ebp), %edi / keep i/o interrupt number in %edi

	call    splint                  / raise interrupt priority level

	/ check for spurious interrupt
	cmp	$-1, %eax		/ if return from splint() is -1 ..
	je	cmnintret		/   .. simply return

	pushl   %eax                    / push returned old priority level

	cmpl    $SPLHI, ipl             / if new level is SPLHI ..
	je      clock               	/   .. must be clock interrupt

	sti                             / enable interrupts

	/ call the interrupt routine
	pushl   %edi                    / push interrupt number arg
	movl	$ivect, %ebx
	call    *(%ebx, %edi, 4)        / (*ivect[intlev])()
	addl    $4, %esp                / pop arg

	/ NOTE: If the interrupt was in kernel mode, we leave interrupts
	/ disabled until the iret, to avoid stacking interrupts in
	/ the section of code from the splxint() call to the iret.
	/ Interrupts will be enabled when the old flags are popped
	/ by the iret.

	cli                             / disable interrupts

	/ restore old interrupt priority level, which is on stack
	call    splxint
	addl    $4, %esp        / pop old interrupt priority level

	jmp     cmnintret


/ clock interrupt
/	push args to clock() and call it with interrupts still disabled.
/	old ipl is already on stack

	.align	4
clock:
	/ push args to clock() and call it with interrupts still disabled.
	/ old ipl is already on stack
	movl    [EFL\*4](%ebp), %eax
	pushl	%eax
#ifdef VPIX
	testl   $VMFLAG, [EFL\*4](%ebp)
	jz      real_cs
	pushl   $USER_CS    / User CS ... don't push a v86 mode CS
	jmp     push_ip

	.align	4
real_cs:
#endif
	movl    [CS\*4](%ebp), %eax
	pushl	%eax
push_ip:
	movl    [EIP\*4](%ebp), %eax
	pushl	%eax
	call    clock_int       / clock(ip, cs, flags, oldipl)
				/ clock_int is in hrtimers.c
	addl    $12, %esp       / pop 3 args, leaving olp ipl
	movl    %eax, %edi      / save user profiling flag returned by clock

#ifdef	MERGE386
	cmpl	$0, merge386enable
	je	novm86_clock
	call	vm86_clock	/ call various merge functions to deliver
				/ virtual interrupts and check for dirty screens
novm86_clock:
#endif	/* MERGE386 */
	/ restore old interrupt priority level, which is on stack,
	/ leaving interrupts disabled and %edi preserved
	call    splxint
	addl    $4, %esp        / pop old interrupt priority level

	orl     %edi, %edi      / test user profiling flag from clock()
	jz      cmnintret

	/ Call addupc() to do user profiling.
	/ It must run with interrupts enabled, because it may
	/ cause a page fault.
	/ This code is executed only if the clock interrupt occurred
	/ in user mode, so we must be at ipl zero.
	cmpl    $0, ipl         / check that current ipl is zero
	jne     ipl0panic

	sti                     / enable interrupts
	pushl	$1		/ one tick
	pushl	[EIP\*4](%ebp)
	call	addupc		/ addupc(userip, 1)
	addl	$8, %esp	/ pop 2 args
	cli                     / disable interrupts for consistency below

cmnintret:
	cmpl    $0, ipl         / if not at ipl zero ..
	jnz     notimein        /   .. don't run timeouts
	cmpl    $0, dotimein    / check for timeouts
	jz      notimein
	movl    $0, dotimein    / clear the flag
	call    timein          / run timeouts
notimein:

	decl	intr_entered	/ un-nest interrupt indicator

ret_intr:
#if defined(VPIX) || defined(MERGE386)
	testl   $VMFLAG, [EFL\*4](%ebp) / if we interrupted V86 mode,
	jnz     ret_user
#endif
	movl    [CS\*4](%ebp), %eax
	testw	$IS_LDT_SEL, %ax
	jz      do_ret          / if interrupt was in kernel mode, return

/ all traps, interrupts and system calls return to user mode here

	.globl  ret_user        / debug
ret_user:
	cmpl    $0, ipl                 / check that current ipl is zero
	jne     ipl0panic


	/ run streams queue service routines
	/
	/ The function of the qrunflag is not clear, since the service
	/ routines are not supposed to sleep, and thus another process
	/ should not be able to also run them.
	/
	cli			/ enter critical section
	cmpb    $0, qrunflag    / skip if nothing to run
	je      not_qrun
	cmpb    $0, queueflag   / skip if already in queuerun()
	jne     not_qrun
	movb	$1, queueflag
	sti			/ leave critical section
	call	queuerun
	movb	$0, queueflag
	jmp     do_s_trap

	.align	4
not_qrun:
	sti			/ leave critical section

do_s_trap:
#ifdef  VPIX
	pushl   %ebp            / Pointer to registers (parm)
#endif
	call    s_trap          / do signal processing and rescheduling
#ifdef  VPIX
	addl    $4, %esp        / Clean parm off the stack
#endif
#ifdef	MERGE386
	cmpl	$0, merge386enable
	je	dst_000
	testl	$VMFLAG, [EFL\*4](%ebp)	/ if we came from a vm86 process...
	jz	dst_000		
	pushl	%esp
	call	chkvm86ints		/ check for pending virtual interrupts
	addl	$4, %esp
dst_000:
#endif /* MERGE386 */

	cli     		/ disable interrrupts

	/ load debug registers for user mode
	cmpb	$0,u+u_debugon
	jz      user_ret
#ifndef NODEBUGGER
	cmpb	$0, user_owns_dbregs
	jnz	skip_dbreg_save
	movl	%db0, %eax		/ Save kernel's debug registers
	movl	%eax, save_kernel_db0
	movl	%db1, %eax
	movl	%eax, save_kernel_db1
	movl	%db2, %eax
	movl	%eax, save_kernel_db2
	movl	%db3, %eax
	movl	%eax, save_kernel_db3
	movl	%db7, %eax
	movl	%eax, save_kernel_db7
	incl	user_owns_dbregs
skip_dbreg_save:
#endif /* NODEBUGGER */
	movl	u+u_debugreg,%eax
	movl	%eax,%db0
	movl	u+u_debugreg+4,%eax
	movl	%eax,%db1
	movl	u+u_debugreg+8,%eax
	movl	%eax,%db2
	movl	u+u_debugreg+12,%eax
	movl	%eax,%db3
	subl	%eax,%eax
	movl	%eax,%db6
	movl	u+u_debugreg+28,%eax
	movl	%eax,%db7

user_ret:
#ifdef VPIX
	.globl  v86procflag
	cmpb    $0, v86procflag         / If v86 process flag set
	jne     itis_v86                / Process v86 process return
not_v86:
#endif

/ all traps, interrupts and system calls return here

	.globl  do_ret          / debug
do_ret:
	cli
	call	kexit_check		/ check for kernel exit hooks

	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popa
	addl    $8, %esp        / get TRAPNO and ERROR off the stack

	/ Code in k_trap assumes that this is the ONLY point where
	/ there is an iret to user mode.
	.globl	common_iret
common_iret:
	iret

#ifdef VPIX
/ If we are going to user mode of a dual mode process (V86 or 386
/ mode) then the virtual interrupts are processed by a call to
/ v86vint(pointer_to_regs_on_stack, in_v86_not_386_flag) which
/ simulates the virtual interrupts.

	.align	4
itis_v86:
	mov     $1, %eax                / Assume V86 mode and not 386 mode
	testl   $VMFLAG, [EFL\*4](%ebp) / if we interrupted V86 mode,
	jnz     was_v86                 / we must be returning to user mode now
	testw   $IS_LDT_SEL, [CS\*4](%ebp) / Did we interrupt user mode?
	jz      not_v86                 / lidt only if entering user mode now
					/ in a dual-mode process
/ If dual-mode task in 386 mode, then the NT bit is expected to be on.
/ Force it on, because the system call has a race condition by which
/ an interrupt before the "pushf" instruction in "sys_call" can reset
/ the NT bit.
	orl     $NT_BIT, [EFL\*4](%ebp) / Set the NT bit of 386 mode task
	orl     $IOPL, [EFL\*4](%ebp)   / Set IOPL = 0 (nop)
					/ The previous line should be
					/ removed, as it does nothing.
					/ It is left in case it
					/ needs to be patched.

/ The busy bit in the XTSS descriptor (of the V86 task) has to be
/ set when in 386 mode in a dual mode process so that an IRET can
/ nest correctly when the NT bit is on.
	orb     $TASK_BUSY, gdt+XTSSSEL+ACCESS_BYTE     / Set XTSS busy bit
	xor     %eax, %eax              / Flag 386 user mode (EAX = 0)
was_v86:
	pushl   %eax                    / 1->V86mode, 0->386mode
	pushl   %ebp                    / Pointer to registers on stack
	call    v86vint                 / Process virtual interrupts
	addl    $8, %esp                / Throw away parms on stack

	cli                             / No interrupts til iret
					/ or we might get un-lidt'ed

	call	kexit_check		/ check for kernel exit hooks

	pushfl
	andl    $0x1bfff, (%esp)        / Stack NT can be on, but turn off
					/ CURRENT NT flag so next IRET
	popfl                           / won't switch to V86 task
			    / OK to clear VM since it can't be on anyway!
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popa
	addl	$8, %esp		/ get intlevel and padding off the stack
	lidt    %cs:idtdsc2             / use the v86 idt for this one
	iret
#endif

	.align	4
ipl0panic:
	call    ipl0panic2


/ Entry point for cleanup after user signal handling

	.align	4
	.globl	sig_clean
sig_clean:
	/ set up the stack to look as in reg.h
	subl    $8, %esp        / pad the stack with ERRCODE and TRAPNO
	pusha			/ save user registers
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	pushfl
	popl	%eax
	movl    %eax, [EFL\*4](%esp)

	movw	$KDSSEL, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movl	%esp, %ebp
	pushl	$F_OFF
	popfl

	call	kentry_check	/ check for kernel entry hooks
	sti

	pushl	%esp		/ argument to sigclean
	.globl  sigclean
	call    sigclean        / restore pre-signal state
	addl    $4, %esp        / pop arg

	cli                     / Don't allow interrupts that could clear the
				/ u.u_sigfault flag before a possible kernel
				/ GP violation on the iret.
	movb	$1, u+u_sigfault    / catch signal processing GP faults
				/ This is processed in k_trap (trap.c)

	jmp     user_ret        / Can't do regular return-to-user processing


/ return to computer firmware

	.globl	inrtnfirm

	.align	4
	.globl	rtnfirm
rtnfirm:
#ifdef AT386
	movl	$1, inrtnfirm	/ set flag indicating we are in rtnfirm
	call	spl0		/ ensure interrupt from kbd gets through
loop_forever:
	jmp	loop_forever	/ wait for ctl-alt-del forever
#else
/ There is no monitor.  Put a complete halt to things.
rtn_forever:
	cli	/ No interrupts
	hlt	/ There is no monitor to return to, just halt
	jmp	rtn_forever
#endif


	.data
inrtnfirm:
	.long	0
