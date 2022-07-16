.ident	"@(#)libc-i386:libc-i386/sys/umask.s	1.4"


	.file	"umask.s"

	.text

	.globl	_cerror

_fwdef_(`umask'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$UMASK,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
