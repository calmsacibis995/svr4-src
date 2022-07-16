.ident	"@(#)libc-i386:libc-i386/sys/rmdir.s	1.4"

	.file	"rmdir.s"

	.text

	.globl	_cerror

_fwdef_(`rmdir'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$RMDIR,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
