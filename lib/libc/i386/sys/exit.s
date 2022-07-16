.ident	"@(#)libc-i386:libc-i386/sys/exit.s	1.4"

	.file	"exit.s"

	.text

	.globl	_exit

_fgdef_(_exit):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$EXIT,%eax
	lcall	$0x7,$0
