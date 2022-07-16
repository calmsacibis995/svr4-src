.ident	"@(#)libc-i386:sys/fstatvfs.s	1.1"

/ error = fstatvfsf(file, statbuf, len);
/ char statbuf[34]

	.file	"fstatvfs.s"

	.text

	.globl  _cerror

_fwdef_(`fstatvfs'):
	MCOUNT
	movl	$FSTATVFS,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
