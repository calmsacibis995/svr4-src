.ident	"@(#)libc-i386:sys/setrlimit.s	1.1"

	.file	"setrlimit.s"
	
	.text

	.globl	_cerror

_fwdef_(`setrlimit'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SETRLIMIT,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
