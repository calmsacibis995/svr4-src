.ident	"@(#)libc-i386:libc-i386/sys/stty.s	1.4"


	.file	"stty.s"

	.text

	.globl	_cerror

_fwdef_(`stty'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$STTY,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
