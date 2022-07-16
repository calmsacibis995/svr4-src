.ident	"@(#)libc-i386:libc-i386/sys/chdir.s	1.4"

/ chdir

	.file	"chdir.s"

	.text

	.globl	_cerror

_fwdef_(`chdir'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$CHDIR,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
