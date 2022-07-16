.ident	"@(#)libc-i386:libc-i386/sys/poll.s	1.6"

	.file	"poll.s"

	.text

	.set ERESTART,91

	.globl	_cerror

_fwdef_(`poll'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$POLL,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	poll
	jmp	_cerror
noerror:
	ret
