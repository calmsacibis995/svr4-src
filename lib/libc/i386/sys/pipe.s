.ident	"@(#)libc-i386:libc-i386/sys/pipe.s	1.4"

	.file	"pipe.s"

	.text

	.globl	_cerror

_fwdef_(`pipe'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$PIPE,%eax
	lcall	$0x7,$0
	jc	_cerror
	movl	4(%esp),%ecx
	movl	%eax,(%ecx)
	movl	%edx,4(%ecx)
	xorl	%eax,%eax
	ret
