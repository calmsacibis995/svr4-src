	.file	"strncat.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strncat.s	1.3.1.1"

	.globl	strncat
	.align	4

_fgdef_(strncat):
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
	movl	16(%esp),%ecx	/ %ecx = max length want of source
	repnz ; scab

	jnz	.move
	decl	%edi		/ point to null after string

.move:
/ Most suffixes are not very long; not worth setup for longword moves
	movl	%edi,%ecx	/ %ecx = after source string
	movl	%esi,%edi	/ %edi = destination
	movl	12(%esp),%esi	/ %esi = source string
	subl	%esi,%ecx	/ %ecx = length of source string
	rep ; smovb
	movb	%cl,(%edi)	/ put null after all strings

	movl	8(%esp),%eax	/ %eax = returned dest string addr
	popl	%esi		/ restore register variables
	movl	%edx,%edi
	ret
