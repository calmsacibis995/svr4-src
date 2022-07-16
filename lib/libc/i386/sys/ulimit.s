.ident	"@(#)libc-i386:libc-i386/sys/ulimit.s	1.4"

	.file	"ulimit.s"

	.text

	.globl	_cerror

_fwdef_(`ulimit'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$ULIMIT,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
