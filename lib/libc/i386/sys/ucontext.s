.ident	"@(#)libc-i386:sys/ucontext.s	1.1"


	.file	"ucontext.s"

	.globl	__getcontext
	.globl	_cerror

_fgdef_(`__getcontext'):
	popl	%edx
	pushl	$0
	pushl	%edx
	jmp 	sys

_fwdef_(`setcontext'):
	popl	%edx
	pushl	$1
	pushl	%edx
	jmp 	sys

sys:
	MCOUNT	
	movl	$UCONTEXT,%eax
	lcall	$0x7,$0
	popl	%edx
	movl	%edx,0(%esp)
	jc	_cerror
	ret

	
