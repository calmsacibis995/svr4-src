.ident	"@(#)libc-i386:libc-i386/sys/kill.s	1.4"

	.file	"kill.s"

	.text

	.globl	_cerror

_fwdef_(`kill'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$KILL,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
