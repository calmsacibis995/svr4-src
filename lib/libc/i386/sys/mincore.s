.ident	"@(#)libc-i386:sys/mincore.s	1.1"

/ gid = mincore();
/ returns effective gid

	.file	"mincore.s"

	.text

	.globl  _cerror

_fwdef_(`mincore'):
	MCOUNT
	movl	$MINCORE,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
