.ident	"@(#)libc-i386:sys/priocntlset.s	1.1"

/ gid = priocntlset();
/ returns effective gid

	.file	"priocntl.s"

	.text

	.globl  _cerror
	.globl	__priocntlset

_fgdef_(`__priocntlset'):
	MCOUNT
	movl	$PRIOCNTLSET,%eax
	lcall	$0x7,$0	
	jc	_cerror
	ret
