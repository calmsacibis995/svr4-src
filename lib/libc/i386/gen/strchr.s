	.file	"strchr.s"

	.ident	"@(#)libc-i386:libc-i386/gen/strchr.s	1.4"

	.globl	strchr
	.align	4

_fgdef_(strchr):
	MCOUNT			/ profiling
	movl	4(%esp),%eax	/ %eax = string address
	movb	8(%esp),%dh	/ %dh = byte sought
.loop:
	movb	(%eax),%dl	/ %dl = byte of string
	cmpb	%dh,%dl		/ find it?
	je	.found		/ yes
	testb	%dl,%dl		/ is it null?
	je	.notfound

	movb	1(%eax),%dl	/ %dl = byte of string
	cmpb	%dh,%dl		/ find it?
	je	.found1		/ yes
	testb	%dl,%dl		/ is it null?
	je	.notfound

	movb	2(%eax),%dl	/ %dl = byte of string
	cmpb	%dh,%dl		/ find it?
	je	.found2		/ yes
	testb	%dl,%dl		/ is it null?
	je	.notfound

	movb	3(%eax),%dl	/ %dl = byte of string
	cmpb	%dh,%dl		/ find it?
	je	.found3		/ yes
	addl	$4,%eax
	testb	%dl,%dl		/ is it null?
	jne	.loop

.notfound:
	xorl	%eax,%eax	/ %eax = NULL
	ret

.found3:
	incl	%eax
.found2:
	incl	%eax
.found1:
	incl	%eax
.found:
	ret
