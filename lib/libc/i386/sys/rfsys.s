.ident	"@(#)libc-i386:libc-i386/sys/rfsys.s	1.4"

	.file	"rfsys.s"

	.text

	.globl	_cerror

_fwdef_(`rfsys'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$RFSYS,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
