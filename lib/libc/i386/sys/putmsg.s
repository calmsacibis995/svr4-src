.ident	"@(#)libc-i386:libc-i386/sys/putmsg.s	1.6"

	.file	"putmsg.s"

	.text

	.set	ERESTART,91

	.globl	_cerror

_fwdef_(`putmsg'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$PUTMSG,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	putmsg
	jmp	_cerror
noerror:
	ret
