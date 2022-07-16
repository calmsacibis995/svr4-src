.ident	"@(#)libc-i386:libc-i386/sys/utime.s	1.4"


	.file	"utime.s"

	.text

	.globl	_cerror

_fwdef_(`utime'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$UTIME,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
