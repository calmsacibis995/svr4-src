.ident	"@(#)libc-i386:libc-i386/sys/link.s	1.4"

	.file	"link.s"

	.text

	.globl	_cerror

_fwdef_(`link'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$LINK,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
