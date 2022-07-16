.ident	"@(#)libc-i386:libc-i386/sys/gtty.s	1.4"

	.file	"gtty.s"

	.text

	.globl	_cerror

_fwdef_(`gtty'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GTTY,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
