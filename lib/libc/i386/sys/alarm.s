.ident	"@(#)libc-i386:libc-i386/sys/alarm.s	1.4"

/ alarm 

	.file	"alarm.s"

	.text

	.globl	_cerror

_fwdef_(`alarm'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$ALARM,%eax
	lcall	$0x7,$0
	ret
