.ident	"@(#)libc-i386:libc-i386/sys/read.s	1.5"

	.file	"read.s"

	.text

	.set	ERESTART,91

	.globl	_cerror

_fwdef_(`read'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$READ,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	read
	jmp	_cerror

noerror:
	ret
