.ident	"@(#)libc-i386:libc-i386/sys/umount.s	1.4"


	.file	"umount.s"

	.text

	.globl	_cerror

_fwdef_(`umount'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$UMOUNT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
