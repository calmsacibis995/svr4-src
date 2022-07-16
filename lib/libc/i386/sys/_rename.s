.ident	"@(#)libc-i386:sys/_rename.s	1.1"

/ _rename is the system call version of rename()


	.file	"_rename.s"

	.text

	.globl	_rename
	.globl	_cerror

_fgdef_(_rename):
	MCOUNT
	movl	$RENAME,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
