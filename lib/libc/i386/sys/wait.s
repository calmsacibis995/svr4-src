.ident	"@(#)libc-i386:libc-i386/sys/wait.s	1.5"


	.file	"wait.s"

	.text

	.set	ERESTART,91

_fwdef_(`wait'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$WAIT,%eax
	lcall	$0x7,$0
	jae	noerror
	cmpb	$ERESTART,%al
	je	wait
	jmp	_cerror

noerror:
	movl	4(%esp),%ecx
	testl	%ecx,%ecx
	jz	.return
	movl	%edx,(%ecx)
.return:
	ret
