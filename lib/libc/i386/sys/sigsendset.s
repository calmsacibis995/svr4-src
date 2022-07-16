.ident	"@(#)libc-i386:sys/sigsendset.s	1.1"

	.file	"sigsendset.s"

	.globl	_cerror

_fwdef_(`sigsendset'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SIGSENDSET,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
