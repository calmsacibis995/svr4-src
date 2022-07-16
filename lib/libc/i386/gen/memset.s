	.file	"memset.s"

	.ident	"@(#)libc-i386:libc-i386/gen/memset.s	1.3"

	.globl	memset
	.align	4

_fgdef_(memset):
	MCOUNT			/ profiling
	pushl	%edi		/ save register variable
	movl	8(%esp),%edi	/ %edi = string address
	movb	12(%esp),%al	/ %al = byte to duplicate
	movl	16(%esp),%ecx	/ %ecx = number of copies
	cmpl	$20,%ecx	/ strings with 20 or more chars should
	jle	.byteset	/ be set one longword at a time

	andl	$0xff,%eax	/ clear out top 3 bytes
	movb	%al,%ah		/ replicate the byte four times
	movl	%eax,%edx
	shll	$16,%edx
	orl	%edx,%eax
	movl	%ecx,%edx	/ %edx = number of bytes to set
	shrl	$2,%ecx		/ %ecx = number of words to set
	rep; sstol
	movl	%edx,%ecx	/ %ecx = number of bytes to set
	andl	$3,%ecx		/ %ecx = number of bytes left
.byteset:
	rep; sstob
	movl	8(%esp),%eax	/ return string address
	popl	%edi		/ restore register variable
	ret
