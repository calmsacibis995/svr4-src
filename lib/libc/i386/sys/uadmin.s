.ident	"@(#)libc-i386:libc-i386/sys/uadmin.s	1.4"


	.file	"uadmin.s"

	.text

	.globl	_cerror

_fwdef_(`uadmin'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$UADMIN,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
