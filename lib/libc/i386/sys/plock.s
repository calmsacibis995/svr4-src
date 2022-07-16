.ident	"@(#)libc-i386:libc-i386/sys/plock.s	1.4"

	.file	"plock.s"

	.text

	.globl	_cerror

_fwdef_(`plock'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$PLOCK,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
