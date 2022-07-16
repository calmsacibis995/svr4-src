.ident	"@(#)libc-i386:libc-i386/sys/setgid.s	1.4"

	.file	"setgid.s"

	.text

	.globl	_cerror

_fwdef_(`setgid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SETGID,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
