	.file	"setuname.s"

	.ident	"@(#)libc-i386:libc-i386/sys/setuname.s	1.3"


	.globl	setuname
	.set	SETUNAME,3

setuname:
	MCOUNT			/ subroutine entry counter if profiling
	pushl	$SETUNAME
	pushl	$0
	pushl	12(%esp)	/ retaddr+$SETUNAME+$0
	subl	$4,%esp
	movl	$UTSSYS,%eax
	lcall	$0x7,$0
	jc	.cerror
	addl	$16,%esp
	xorl	%eax,%eax
	ret

.cerror:
	movl	%eax,errno
	movl	$-1,%eax
	addl	$16,%esp
	ret
