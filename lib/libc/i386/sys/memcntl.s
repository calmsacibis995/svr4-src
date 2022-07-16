.ident	"@(#)libc-i386:sys/memcntl.s	1.1"

/ gid = memcntl();
/ returns effective gid

	.file	"memcntl.s"

	.text

	.globl  _cerror

_fwdef_(`memcntl'):
	MCOUNT
	movl	$MEMCNTL,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
