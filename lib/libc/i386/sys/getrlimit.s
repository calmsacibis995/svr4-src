.ident	"@(#)libc-i386:sys/getrlimit.s	1.1"


	
	.file	"getrlimit.s"

	.text

	.globl  _cerror

_fwdef_(`getrlimit'):
	MCOUNT
	movl	$GETRLIMIT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax, %eax
	ret
