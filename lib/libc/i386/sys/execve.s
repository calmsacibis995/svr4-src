.ident	"@(#)libc-i386:libc-i386/sys/execve.s	1.4"

	.file	"execve.s"

	.text

	.globl	_cerror

_fwdef_(`execve'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$EXECE,%eax
	lcall	$0x7,$0
	jmp	_cerror
