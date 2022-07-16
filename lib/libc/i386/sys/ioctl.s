.ident	"@(#)libc-i386:libc-i386/sys/ioctl.s	1.5"

	.file	"ioctl.s"

	.text

	.set	ERESTART,91

	.globl	_cerror

_fwdef_(`ioctl'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$IOCTL,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je 	ioctl	
	jmp	_cerror

noerror:
	ret
