	.file	"memcpy.s"

	.ident	"@(#)libc-i386:libc-i386/gen/memcpy.s	1.4"

	.globl	memcpy
	.align	4

_fgdef_(memcpy):
	MCOUNT			/ profiling
	movl	%edi,%edx	/ save register variables
	pushl	%esi
	movl	8(%esp),%edi	/ %edi = dest address
	movl	12(%esp),%esi	/ %esi = source address
	movl	16(%esp),%ecx	/ %ecx = length of string
	movl	%edi,%eax	/ return value from the call

	shrl	$2,%ecx		/ %ecx = number of words to move
	rep ; smovl		/ move the words

	movl	16(%esp),%ecx	/ %ecx = number of bytes to move
	andl	$0x3,%ecx	/ %ecx = number of bytes left to move
	rep ; smovb		/ move the bytes

	popl	%esi		/ restore register variables
	movl	%edx,%edi
	ret
