.ident	"@(#)libc-i386:sys/fstatf.s	1.1"

	.file	"fstatf.s"

	.text

	.globl	_cerror

_fwdef_(`fstatf'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$FSTATF,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
