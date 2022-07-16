.ident	"@(#)libc-i386:libc-i386/sys/execle.s	1.4"

	.file	"execle.s"

	.text

	.globl	execve

_fwdef_(`execle'):
	MCOUNT			/ subroutine entry counter if profiling
	leal    4(%esp),%edx    / argv (retaddr + file arg - 4)
	movl	%edx,%eax

.findzero:
	addl	$4,%eax
	cmpl	$0,(%eax)
	jne	.findzero

	pushl	4(%eax)		/ envp
	leal    12(%esp),%edx   / argv (push + retaddr + file arg)
	pushl   %edx
	pushl	12(%esp)	/ file (retaddr + 2 pushes off %esp)
	call	_fref_(execve)
	addl    $12,%esp
	ret
