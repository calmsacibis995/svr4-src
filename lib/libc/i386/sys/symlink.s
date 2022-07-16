.ident	"@(#)libc-i386:sys/symlink.s	1.1"


	.file	"symlink.s"

	.text

	.globl	_cerror

_fwdef_(`symlink'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SYMLINK,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
