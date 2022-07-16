
.ident	"@(#)libc-i386:sys/adjtime.s	1.1"

/ adjtime


	.file	"adjtime.s"

	.text

	.globl  _cerror

_fwdef_(`adjtime'):
	MCOUNT
	movl	$ADJTIME,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
