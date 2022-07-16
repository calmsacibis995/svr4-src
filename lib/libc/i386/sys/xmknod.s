.ident	"@(#)libc-i386:sys/xmknod.s	1.1"

/ OS library -- _xmknod

/ error = _xmknod(version, string, mode, dev)

	.file	"xmknod.s"

	.text

	.globl	_cerror
	.globl	_xmknod

_fgdef_(`_xmknod'):
	movl	$XMKNOD,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
