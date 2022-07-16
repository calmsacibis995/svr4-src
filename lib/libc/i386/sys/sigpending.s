.ident	"@(#)libc-i386:sys/sigpending.s	1.1"

/ C library -- setsid, setpgid, getsid, getpgid


	.file	"sigpending.s"

	.text

	.globl	__sigfillset
	.globl  _cerror

_fwdef_(`sigpending'):
	popl	%edx
	pushl	$1
	pushl	%edx
	jmp	sys

_fgdef_(`__sigfillset'):
	popl	%edx
	pushl	$2
	pushl	%edx
	jmp	sys

sys:
	movl	$SIGPENDING,%eax
	lcall	$7,$0
	popl	%edx
	movl	%edx,0(%esp)	/ Remove extra word
	jc	_cerror
	ret

