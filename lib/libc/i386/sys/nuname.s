.ident	"@(#)libc-i386:sys/nuname.s	1.1"

/ gid = nuname();
/ returns effective gid

	.file	"nuname.s"

	.text

	.globl  _cerror

_fwdef_(`nuname'):
	MCOUNT
	movl	$NUNAME,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
