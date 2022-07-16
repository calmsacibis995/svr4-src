.ident	"@(#)libc-i386:libc-i386/sys/chmod.s	1.4"

	.file	"chmod.s"

	.text

	.globl	_cerror

_fwdef_(`chmod'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$CHMOD,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
