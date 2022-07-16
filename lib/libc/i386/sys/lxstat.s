.ident	"@(#)libc-i386:sys/lxstat.s	1.1"

/ gid = _lxstat();
/ returns effective gid

	.file	"lxstat.s"

	.text

	.globl  _cerror
	.globl  _lxstat

_fgdef_(`_lxstat'):
	MCOUNT
	movl	$LXSTAT,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
