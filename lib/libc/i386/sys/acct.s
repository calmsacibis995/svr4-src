
.ident	"@(#)libc-i386:libc-i386/sys/acct.s	1.4"

/ acct function


	.file	"acct.s"

	.text

	.globl  _cerror

_fwdef_(`acct'):
	MCOUNT
	movl	$ACCT,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
