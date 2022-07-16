.ident	"@(#)libc-i386:sys/writev.s	1.4"

/ OS library -- writev 

/ error = writev(fd, iovp, iovcnt)

	.file	"writev.s"

	.text

	.set	ERESTART,91

	.globl	_cerror

_fwdef_(`writev'):
	/ MCOUNT
	movl	$WRITEV,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	writev
	jmp 	_cerror
noerror:
	ret
