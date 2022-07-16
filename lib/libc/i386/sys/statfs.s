.ident	"@(#)libc-i386:libc-i386/sys/statfs.s	1.4"


	.file	"statfs.s"

	.globl	_cerror

_fwdef_(`statfs'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$STATFS,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
