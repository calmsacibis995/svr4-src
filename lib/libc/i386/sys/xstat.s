.ident	"@(#)libc-i386:sys/xstat.s	1.1"

/ OS library -- _xstat

/ error = _xstat(version, string, statbuf)

	.file	"xstat.s"

	.text

	.globl	_cerror
	.globl	_xstat

_fgdef_(_xstat):
	movl	$XSTAT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
