.ident	"@(#)libc-i386:sys/statvfs.s	1.1"


	.file	"statvfs.s"

	.text

	.globl	_cerror

_fwdef_(`statvfs'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$STATVFS,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
