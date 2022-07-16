.ident	"@(#)libc-i386:libc-i386/sys/sysfs.s	1.4"


	.file	"sysfs.s"

	.text

	.globl	_cerror

_fwdef_(`sysfs'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SYSFS,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
