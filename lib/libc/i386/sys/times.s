.ident	"@(#)libc-i386:libc-i386/sys/times.s	1.4"


	.file	"times.s"

	.text

	.globl	_cerror

_fwdef_(`times'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$TIMES,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
