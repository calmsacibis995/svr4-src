	.file	"strcat.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strcat.s	1.3"

	.globl	strcat
	.align	4

_fgdef_(strcat):
	MCOUNT			/ profiling
	movl	%edi,%edx	/ save register variables
	pushl	%esi

/ find end of destination string
	movl	8(%esp),%edi	/ %edi = destination string
	xorl	%eax,%eax	/ %eax = 0 (search for a null)
	movl	$-1,%ecx	/ %ecx = a very long search
	repnz ; scab

	decl	%edi		/ result was already postincremented
	movl	%edi,%esi	/ save for later

/ search for end of source string
	movl	12(%esp),%edi	/ %edi = address of source string
	movl	$-1,%ecx	/ %ecx = a very long search
	repnz ; scab

/ Most suffixes are not very long; not worth setup for longword moves
	notl	%ecx		/ %ecx = length of move
	movl	%esi,%edi	/ %edi = destination
	movl	12(%esp),%esi	/ %esi = source string
	rep ; smovb

	movl	8(%esp),%eax	/ %eax = returned dest string addr
	popl	%esi		/ restore register variables
	movl	%edx,%edi
	ret
