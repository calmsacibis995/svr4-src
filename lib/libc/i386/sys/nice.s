.ident	"@(#)libc-i386:libc-i386/sys/nice.s	1.4"


	.file	"nice.s"

	.text

	.globl	_cerror

_fwdef_(`nice'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$NICE,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
