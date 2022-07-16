.ident	"@(#)libc-i386:sys/sysconfig.s	1.1"

	.file	"sysconfig.s"

	.text

	.globl	_cerror

_fwdef_(`sysconfig'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SYSCONFIG,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
