.ident	"@(#)libc-i386:sys/lstat.s	1.1"


	.file	"lstat.s"

	.text

	.globl  _cerror

	.align 4
_fwdef_(`lstat'):
	MCOUNT
	movl	$LSTAT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
