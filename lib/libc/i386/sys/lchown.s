.ident	"@(#)libc-i386:sys/lchown.s	1.1"


	.file	"lchown.s"

	.text

	.globl  _cerror

_fwdef_(`lchown'):
	MCOUNT
	movl	$LCHOWN,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax, %eax
	ret
