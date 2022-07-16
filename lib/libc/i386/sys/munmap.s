.ident	"@(#)libc-i386:sys/munmap.s	1.1"

/ gid = munmap();
/ returns effective gid

	.file	"munmap.s"

	.text

	.globl  _cerror

_fwdef_(`munmap'):
	MCOUNT
	movl	$MUNMAP,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
