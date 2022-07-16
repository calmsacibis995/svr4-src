.ident	"@(#)libc-i386:libc-i386/sys/mount.s	1.4"

	.file	"mount.s"

	.globl	_cerror

_fwdef_(`mount'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$MOUNT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
