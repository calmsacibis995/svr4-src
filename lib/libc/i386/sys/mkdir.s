.ident	"@(#)libc-i386:libc-i386/sys/mkdir.s	1.4"

	.file	"mkdir.s"

	.text

	.globl	_cerror

_fwdef_(`mkdir'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$MKDIR,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
