.ident	"@(#)libc-i386:libc-i386/sys/pause.s	1.4"

	.file	"pause.s"

	.text

	.globl	_cerror

_fwdef_(`pause'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$PAUSE,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
