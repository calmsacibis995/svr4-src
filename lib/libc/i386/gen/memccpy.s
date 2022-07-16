	.file	"memccpy.s"

	.ident	"@(#)libc-i386:libc-i386/gen/memccpy.s	1.4"

	.align	4

_fwdef_(`memccpy'):
	MCOUNT			/ profiling
	pushl	%esi		/ save register variable
	movl	8(%esp),%eax	/ %eax = address of dest string
	movl	12(%esp),%esi	/ %esi = address of source string
	movb	16(%esp),%dh	/ %dh = character to search for
	movl	20(%esp),%ecx	/ %ecx = length to go still
.loop:
	decl	%ecx		/ decrement bytes to go
	jl	.notfound
	movb	(%esi),%dl
	movb	%dl,(%eax)	/ move byte
	cmpb	%dh,%dl		/ is it the byte sought?
	je	.found		/ yes

	decl	%ecx		/ decrement bytes to go
	jl	.notfound
	movb	1(%esi),%dl
	movb	%dl,1(%eax)	/ move byte
	cmpb	%dh,%dl		/ is it the byte sought?
	je	.found1		/ yes

	decl	%ecx		/ decrement bytes to go
	jl	.notfound
	movb	2(%esi),%dl
	movb	%dl,2(%eax)	/ move byte
	cmpb	%dh,%dl		/ is it the byte sought?
	je	.found2		/ yes

	decl	%ecx		/ decrement bytes to go
	jl	.notfound
	movb	3(%esi),%dl
	movb	%dl,3(%eax)	/ move byte
	addl	$4,%esi
	addl	$4,%eax
	cmpb	%dh,%dl		/ is it the byte sought?
	jne	.loop		/ no
	decl	%eax

.found:
	popl	%esi		/ restore register variable
	incl	%eax		/ return pointer to next byte in dest
	ret

	.align	4
.found2:
	incl	%eax
.found1:
	popl	%esi		/ restore register variable
	addl	$2,%eax		/ return pointer to next byte in dest
	ret

	.align	4
.notfound:
	popl	%esi		/ restore register variable
	xorl	%eax,%eax	/ search fails
	ret
