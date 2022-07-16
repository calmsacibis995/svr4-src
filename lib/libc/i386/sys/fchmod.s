
.ident	"@(#)libc-i386:sys/fchmod.s	1.1"

/ error = fchmod(fd)

	.file	"fchmod.s"

	.text

	.globl  _cerror

_fwdef_(`fchmod'):
	MCOUNT
	movl	$FCHMOD,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
