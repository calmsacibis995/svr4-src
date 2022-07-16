.ident	"@(#)libc-i386:sys/fsync.s	1.1"

/ error = fsync(fd);


	.file	"fsync.s"

	.text

	.globl  _cerror

_fwdef_(`fsync'):
	MCOUNT
	movl	$FSYNC,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
