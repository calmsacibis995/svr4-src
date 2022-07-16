.ident	"@(#)libc-i386:libc-i386/sys/fstat.s	1.4"

	.file	"fstat.s"

	.text

	.globl	_cerror

_fwdef_(`fstat'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$FSTAT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
