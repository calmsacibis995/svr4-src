.ident	"@(#)libc-i386:libc-i386/sys/unlink.s	1.4"


	.file	"unlink.s"

	.text

	.globl	_cerror

_fwdef_(`unlink'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$UNLINK,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
