.ident	"@(#)libc-i386:libc-i386/sys/fstatfs.s	1.4"

	.file	"fstatfs.s"

	.text

	.globl	_cerror

_fwdef_(`fstatfs'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$FSTATFS,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
