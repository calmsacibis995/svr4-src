.ident	"@(#)libc-i386:libc-i386/sys/mknod.s	1.4"

	.file	"mknod.s"
	
	.text

	.globl	_cerror

_fwdef_(`mknod'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$MKNOD,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
