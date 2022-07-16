	.file	"strrchr.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strrchr.s	1.4"

	.globl	strrchr
	.align	4

_fgdef_(strrchr):
	MCOUNT
	movl	%edi,%edx	/ save register variable

	movl	4(%esp),%edi	/ %edi = address of string
	xorl	%eax,%eax	/ %eax = 0 (search for null)
	movl	$-1,%ecx	/ %ecx = a very long string
	repnz ; scab

	decl	%edi		/ post-incremented past null
	notl	%ecx		/ %ecx = length of string
	movb	8(%esp),%al	/ %esi = char sought
	std ; repnz ; scab ; cld / search backward at least one char

	jnz	.notfound
	leal	1(%edi),%eax	/ postdecremented.
	movl	%edx,%edi	/ restore register variable
	ret

	.align	4
.notfound:
	xorl	%eax,%eax
	movl	%edx,%edi	/ restore register variable
	ret
