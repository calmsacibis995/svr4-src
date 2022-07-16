	.file	"semsys.s"

	.ident	"@(#)libc-i386:libc-i386/sys/semsys.s	1.4"


	.globl	semsys
	.globl	semctl
	.globl	semget
	.globl	semop
	.globl	_cerror
	.set	SEMCTL,0
	.set	SEMGET,1
	.set	SEMOP,2

semctl:
	MCOUNT
	popl	%eax
	pushl	$SEMCTL
	pushl	%eax
	jmp	semsys

semget:
	MCOUNT
	popl	%eax
	pushl	$SEMGET
	pushl	%eax
	jmp	semsys

semop:
	MCOUNT
	popl	%eax
	pushl	$SEMOP
	pushl	%eax

semsys:
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SEMSYS,%eax
	lcall	$0x7,$0
	popl	%edx
	movl	%edx,0(%esp)	/ Remove extra word
	jc	_cerror
	ret
