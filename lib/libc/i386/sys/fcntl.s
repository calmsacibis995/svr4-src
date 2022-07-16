.ident	"@(#)libc-i386:libc-i386/sys/fcntl.s	1.4"

	.file	"fcntl.s"

	.text

	.set	ERESTART,91

	.globl	_cerror

_fwdef_(`fcntl'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$FCNTL,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je 	fcntl
	jmp	_cerror

noerror:
	ret
