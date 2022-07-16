	.file	"memchr.s"

	.ident	"@(#)libc-i386:libc-i386/gen/memchr.s	1.5"

	.globl	memchr
	.align	4

_fgdef_(memchr):
	MCOUNT			/ profiling
	movl	%edi,%edx	/ save register variable
	movl	4(%esp),%edi	/ %edi = string address
	movb	8(%esp),%al	/ %al = byte that is sought
	movl	12(%esp),%ecx	/ %ecx = number of bytes
	jcxz	.notfound	/ check if number of bytes is 0
	repnz ; scab		/ look for %al
	jne	.notfound	/ search failed

	leal	-1(%edi),%eax	/ search increments after finding
	movl	%edx,%edi	/ restore register variable
	ret

	.align	4
.notfound:
	xorl	%eax,%eax
	movl	%edx,%edi	/ restore register variable
	ret
