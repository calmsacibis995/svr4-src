.ident	"@(#)libc-i386:sys/utssys.s	1.1"


	.file	"utssys.s"

	.text

	.globl	_cerror

_fwdef_(`utssys'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$UTSSYS,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
