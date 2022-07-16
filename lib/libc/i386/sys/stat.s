.ident	"@(#)libc-i386:libc-i386/sys/stat.s	1.4"


	.file	"stat.s"

	.text

	.globl	_cerror

_fwdef_(`stat'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$STAT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
