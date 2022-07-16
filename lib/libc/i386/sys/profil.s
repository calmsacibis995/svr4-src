.ident	"@(#)libc-i386:libc-i386/sys/profil.s	1.4"

	.file	"profil.s"

	.text

	.globl	_cerror

_fwdef_(`profil'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$PROF,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
