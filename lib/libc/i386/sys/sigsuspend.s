.ident	"@(#)libc-i386:sys/sigsuspend.s	1.1"

	.file	"sigsuspend.s"

	.globl	_cerror

_fwdef_(`sigsuspend'):
	_prologue_
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SIGSUSPEND,%eax
	movl	_daref_(_sigreturn),%edx
	_epilogue_
	lcall	$0x7,$0
	jb	_cerror
	ret

_sigreturn:
	addl	$4,%esp		/ return args to user interrupt routine
	lcall	$0xF,$0		/ return to kernel to return to user
