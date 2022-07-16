.ident	"@(#)libc-i386:sys/readlink.s	1.1"


	.file	"readlink.s"

	.text

	.globl  _cerror

_fwdef_(`readlink'):
	MCOUNT
	movl	$READLINK,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
