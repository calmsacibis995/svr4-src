	.file	"strncpy.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strncpy.s	1.3"

	.globl	strncpy
	.align	4

/ Assume relatively long strings and small numbers of nulls at end.

_fgdef_(strncpy):
	MCOUNT
	pushl	%edi		/ save register variables
	pushl	%esi

	movl	16(%esp),%edi	/ %edi = address of source
	movl	20(%esp),%ecx	/ %ecx = length of string
	xorl	%eax,%eax	/ %al = 0 (search for 0)
	repnz ; scab
	movl	%ecx,%edx	/ %edx = bytes to zero out later

	movl	%edi,%ecx	/ %ecx = address after source
	movl	16(%esp),%esi	/ %esi = address of source
	subl	%esi,%ecx	/ %ecx = length to move
	movl	12(%esp),%edi	/ %edi = destination string address
	movl	%ecx,%eax	/ %eax = length to move
	shrl	$2,%ecx		/ %ecx = words to move
	rep ; smovl

	movl	%eax,%ecx	/ %ecx = length to move
	andl	$3,%ecx		/ %ecx = leftover bytes to move
	rep ; smovb

	movl	%edx,%ecx	/ %edx = leftover bytes to null
	xorl	%eax,%eax	/ %eax = null
	rep ; sstob		/ store nulls at end

.done:
	movl	12(%esp),%eax	/ return first argument
	popl	%esi		/ restore register variables
	popl	%edi
	ret
