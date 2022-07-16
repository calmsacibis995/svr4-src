.ident	"@(#)libc-i386:libc-i386/sys/stime.s	1.4"


	.file	"stime.s"

	.text

	.globl	_cerror

_fwdef_(`stime'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	4(%esp),%eax	/ Move it to a safe location before
	movl	(%eax),%eax	/ getting privileged.
	movl	%eax,4(%esp)
	movl	$STIME,%eax
	lcall	$0x7,$0
	jc	_cerror
	xorl	%eax,%eax
	ret
