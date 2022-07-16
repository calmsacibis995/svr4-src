.ident	"@(#)libc-i386:libc-i386/sys/creat.s	1.4"

	.file	"creat.s"

	.text


	.globl	_cerror

_fwdef_(`creat'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$CREAT,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
