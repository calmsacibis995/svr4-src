.ident	"@(#)libc-i386:libc-i386/sys/setuid.s	1.4"

	.file	"setuid.s"

	.text

	.globl	_cerror

_fwdef_(`setuid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SETUID,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
