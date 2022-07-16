
.ident	"@(#)libc-i386:sys/_nfssys.s	1.1"

/ _nfssys function


	.file	"_nfssys.s"

	.text

	.globl  _cerror
	.globl	_nfssys

_fgdef_(_nfssys):
	MCOUNT
	movl	$NFSSYS,%eax
	lcall	$0x7,$0
	jc 	_cerror
	xorl	%eax,%eax
	ret
