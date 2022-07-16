.ident	"@(#)libc-i386:libc-i386/sys/close.s	1.4"
	
	.file	"close.s"

	.text

	.globl	_cerror

_fwdef_(`close'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$CLOSE,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
