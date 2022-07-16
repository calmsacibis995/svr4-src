.ident	"@(#)libc-i386:libc-i386/sys/execl.s	1.4"

	.file	"execl.s"

	.text

	.globl	execv

_fwdef_(`execl'):
	MCOUNT			/ subroutine entry counter if profiling
	leal	8(%esp),%eax	/ address of args (retaddr + 1 arg)
	pushl	%eax
	pushl	8(%esp)		/ filename (push + retaddr)
	call	_fref_(execv)
	addl	$8,%esp
	ret
