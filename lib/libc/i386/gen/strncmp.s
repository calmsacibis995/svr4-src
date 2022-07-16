	.file	"strncmp.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strncmp.s	1.5"

	.globl	strncmp
	.align	4

_fgdef_(strncmp):
	MCOUNT
	movl	%edi,%edx	/ save register variables
	pushl	%esi

	movl	8(%esp),%esi	/ %esi = first string
	movl	12(%esp),%edi	/ %edi = second string
	cmpl	%esi,%edi	/ same string?
	je	.equal
	movl	16(%esp),%ecx	/ %ecx = length
	incl	%ecx		/ will later predecrement this uint
.loop:
	decl	%ecx
	je	.equal		/ Used all n chars?
	slodb ; scab
	jne	.notequal	/ Are the bytes equal?
	testb	%al,%al
	je	.equal		/ End of string?

	decl	%ecx
	je	.equal		/ Used all n chars?
	slodb ; scab
	jne	.notequal	/ Are the bytes equal?
	testb	%al,%al
	je	.equal		/ End of string?

	decl	%ecx
	je	.equal		/ Used all n chars?
	slodb ; scab
	jne	.notequal	/ Are the bytes equal?
	testb	%al,%al
	je	.equal		/ End of string?

	decl	%ecx
	je	.equal		/ Used all n chars?
	slodb ; scab
	jne	.notequal	/ Are the bytes equal?
	testb	%al,%al
	jne	.loop		/ End of string?

.equal:
	xorl	%eax,%eax	/ return 0
	popl	%esi		/ restore registers
	movl	%edx,%edi
	ret

	.align	4
.notequal:
	jnc	.ret2
	movl	$-1,%eax	
	jmp	.ret1
.ret2:
	movl	$1,%eax
.ret1:
	popl	%esi		/ restore registers
	movl	%edx,%edi
	ret
