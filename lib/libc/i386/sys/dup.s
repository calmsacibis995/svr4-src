.ident	"@(#)libc-i386:libc-i386/sys/dup.s	1.4"

	.file	"dup.s"

	.text

	.globl	_cerror

_fwdef_(`dup'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$DUP,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
