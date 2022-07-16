	.file	"msgsys.s"

	.ident	"@(#)libc-i386:libc-i386/sys/msgsys.s	1.4"


	.globl	msgget
	.globl	msgctl
	.globl	msgrcv
	.globl	msgsnd
	.globl	msgsys
	.globl	_cerror
	.set	MSGGET,0
	.set	MSGCTL,1
	.set	MSGRCV,2
	.set	MSGSND,3

msgget:
	MCOUNT
	popl	%edx		/ return address
	pushl	$MSGGET		/ call type
	pushl	%edx
	jmp	msgsys

msgctl:
	MCOUNT
	popl	%edx		/ return address
	pushl	$MSGCTL		/ call type
	pushl	%edx
	jmp	msgsys

msgrcv:
	MCOUNT
	popl	%edx		/ return address
	pushl	$MSGRCV		/ call type
	pushl	%edx
	jmp	msgsys

msgsnd:
	MCOUNT
	popl	%edx		/ return address
	pushl	$MSGSND		/ call type
	pushl	%edx

msgsys:
	MCOUNT			/ subroutine entry counter if profiling
	movl	$MSGSYS,%eax
	lcall	$0x7,$0
	popl	%edx
	movl	%edx,0(%esp)	/ Remove extra word
	jc	_cerror
	ret
