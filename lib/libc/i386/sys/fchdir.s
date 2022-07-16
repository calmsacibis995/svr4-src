
.ident	"@(#)libc-i386:sys/fchdir.s	1.1"

/ error = fchdir(fd)

	.file	"fchdir.s"

	.text

	.globl  _cerror

_fwdef_(`fchdir'):
	MCOUNT
	movl	$FCHDIR,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
