.ident	"@(#)libc-i386:libc-i386/sys/open.s	1.4"

	.file	"open.s"
	
	.text

	.globl	_cerror

_fwdef_(`open'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$OPEN,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
