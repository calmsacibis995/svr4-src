	.file	"rtmemcpy.s"

	.ident	"@(#)rtld:i386/rtmemcpy.s	1.1"

	.globl	_rt_memcpy
	.align	4

_rt_memcpy:
	pushl	%edi
	pushl	%esi
	movl	12(%esp),%edi	/ %edi = dest address
	movl	16(%esp),%esi	/ %esi = source address
	movl	20(%esp),%ecx	/ %ecx = length of string
	movl	%edi,%eax	/ return value from the call

	movl	%ecx,%edx	/ %edx = number of bytes to move
	shrl	$2,%ecx		/ %ecx = number of words to move
	rep ; smovl		/ move the words

	movl	%edx,%ecx	/ %ecx = number of bytes to move
	andl	$0x3,%ecx	/ %ecx = number of bytes left to move
	rep ; smovb		/ move the bytes

	popl	%esi
	popl	%edi
	ret
