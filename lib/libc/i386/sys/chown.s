.ident	"@(#)libc-i386:libc-i386/sys/chown.s	1.4"

	.file	"chown.s"

	.text

	.globl	_cerror

_fwdef_(`chown'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$CHOWN,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
