	.file	"shmsys.s"

	.ident	"@(#)libc-i386:libc-i386/sys/shmsys.s	1.4"


	.globl	shmsys
	.globl	shmat
	.globl	shmctl
	.globl	shmdt
	.globl	shmget
	.globl	_cerror
	.set	SHMAT,0
	.set	SHMCTL,1
	.set	SHMDT,2
	.set	SHMGET,3

shmat:
	MCOUNT
	popl	%eax
	pushl	$SHMAT
	pushl	%eax
	jmp	shmsys

shmctl:
	MCOUNT
	popl	%eax
	pushl	$SHMCTL
	pushl	%eax
	jmp	shmsys

shmdt:
	MCOUNT
	popl	%eax
	pushl	$SHMDT
	pushl	%eax
	jmp	shmsys

shmget:
	MCOUNT
	popl	%eax
	pushl	$SHMGET
	pushl	%eax

shmsys:
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SHMSYS,%eax
	lcall	$0x7,$0
	popl	%edx
	movl	%edx,0(%esp)	/ Remove extra word
	jc	_cerror
	ret
