.ident	"@(#)libc-i386:sys/statf.s	1.1"


	.file	"statf.s"

	.text

	.globl	_cerror

_fwdef_(`statf'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$STATF,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
