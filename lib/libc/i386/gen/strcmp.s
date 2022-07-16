	.file	"strcmp.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strcmp.s	1.4"

	.globl	strcmp
	.align	4

_fgdef_(strcmp):
	MCOUNT			/ subroutine ertry counter if profiling
	pushl	%esi
	movl	8(%esp),%esi	/ %esi = pointer to string 1
	movl	12(%esp),%edx	/ %edx = pointer to string 2
	cmpl	%esi,%edx	/ s1 == s2 ?
	je	.equal		/ they are equal
	.align	4
.loop:				/ iterate for cache performance
	movl	(%esi),%eax	/ pick up 4-bytes from first string
	movl	(%edx),%ecx	/ pick up 4-bytes from second string
	cmpl	%ecx,%eax	/ see if they are equal
	jne	.notequal	/ if not, find out why and where
	subl	$0x01010101,%eax	/ see if we hit end of the string
	notl	%ecx
	andl	$0x80808080,%ecx
	andl	%ecx,%eax
	jnz	.equal		/ there was a 0 in the 4-bytes
	movl	4(%esi),%eax	/ pick up 4-bytes from first string
	movl	4(%edx),%ecx	/ pick up 4-bytes from second string
	cmpl	%ecx,%eax	/ see if they are equal
	jne	.notequal	/ if not, find out why and where
	subl	$0x01010101,%eax	/ see if we hit end of the string
	notl	%ecx
	andl	$0x80808080,%ecx
	andl	%ecx,%eax
	jnz	.equal		/ there was a 0 in the 4-bytes
	movl	8(%esi),%eax	/ pick up 4-bytes from first string
	movl	8(%edx),%ecx	/ pick up 4-bytes from second string
	cmpl	%ecx,%eax	/ see if they are equal
	jne	.notequal	/ if not, find out why and where
	subl	$0x01010101,%eax	/ see if we hit end of the string
	notl	%ecx
	andl	$0x80808080,%ecx
	andl	%ecx,%eax
	jnz	.equal		/ there was a 0 in the 4-bytes
	movl	12(%esi),%eax	/ pick up 4-bytes from first string
	movl	12(%edx),%ecx	/ pick up 4-bytes from second string
	cmpl	%ecx,%eax	/ see if they are equal
	jne	.notequal	/ if not, find out why and where
	addl	$16,%esi	/ increment first pointer
	addl	$16,%edx	/ increment second pointer
	subl	$0x01010101,%eax	/ see if we hit end of the string
	notl	%ecx
	andl	$0x80808080,%ecx
	andl	%ecx,%eax
	jz	.loop		/ not yet at end of string, try again
	.align	4
.equal:				/ strings are equal, return 0
	popl	%esi
	xorl	%eax,%eax
	ret
	.align	4
.notequal:			/ two words are not the same, find out why
	cmpb	%cl,%al		/ see if individual bytes are the same
	jne	.set_sign	/ if not the same, go set sign
	andb	%al,%al		/ see if we hit the end of string
	je	.equal		/ yes, they are equal
	cmpb	%ch,%ah		/ check next byte...
	jne	.set_sign
	andb	%ah,%ah
	je	.equal
	shrl	$16,%eax
	shrl	$16,%ecx
	cmpb	%cl,%al
	jne	.set_sign
	andb	%al,%al
	je	.equal
	cmpb	%ch,%ah		/ last byte guaranteed not equal
.set_sign:			/ set the sign of the result for unequal
	popl	%esi
	sbbl	%eax,%eax
	orl	$1,%eax
	ret
