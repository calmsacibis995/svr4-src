.ident	"@(#)libc-i386:libc-i386/sys/lseek.s	1.4"

	.file	"lseek.s"

	.text

	.globl	_cerror

_fwdef_(`lseek'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$LSEEK,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
