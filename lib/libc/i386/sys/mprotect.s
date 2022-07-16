.ident	"@(#)libc-i386:sys/mprotect.s	1.1"

/ gid = mprotect();
/ returns effective gid

	.file	"mprotect.s"

	.text

	.globl  _cerror

_fwdef_(`mprotect'):
	MCOUNT
	movl	$MPROTECT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
