
.ident	"@(#)libc-i386:sys/fchown.s	1.1"

/ error = fchown(fd,owner,group)

	.file	"fchown.s"

	.text
	
	.globl  _cerror

_fwdef_(`fchown'):
	MCOUNT
	movl	$FCHOWN,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
