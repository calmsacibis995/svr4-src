	.file	"strcpy.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strcpy.s	1.4"

	.globl	strcpy
	.align	4

_fgdef_(strcpy):
	MCOUNT			/ profiling
	movl	%edi,%edx	/ save register variables
	pushl	%esi

	movl	12(%esp),%edi	/ %edi = source string address
	xorl	%eax,%eax	/ %al = 0 (search for 0)
	movl	$-1,%ecx	/ length to look: lots
	repnz ; scab

	notl	%ecx		/ %ecx = length to move
	movl	12(%esp),%esi	/ %esi = source string address
	movl	8(%esp),%edi	/ %edi = destination string address
	movl	%ecx,%eax	/ %eax = length to move
	shrl	$2,%ecx		/ %ecx = words to move
	rep ; smovl

	movl	%eax,%ecx	/ %ecx = length to move
	andl	$3,%ecx		/ %ecx = leftover bytes to move
	rep ; smovb

	movl	8(%esp),%eax	/ %eax = returned dest string addr
	popl	%esi		/ restore register variables
	movl	%edx,%edi
	ret
