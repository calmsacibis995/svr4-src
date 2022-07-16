.ident	"@(#)libc-i386:sys/setgroups.s	1.1"

	.file	"setgroups.s"

	.globl	_cerror

_fwdef_(`setgroups'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SETGROUPS,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
