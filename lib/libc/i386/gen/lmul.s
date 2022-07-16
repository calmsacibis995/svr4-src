
/ Double long multiply routine.

	.ident	"@(#)libc-i386:gen/lmul.c	1.4"
	.file	"lmul.s"
	.text

	.set	lop,8
	.set	rop,16
	.set	ans,0

_fwdef_(`lmul'):
	popl	%eax
	xchgl	%eax,0(%esp)

	MCOUNT

	pushl	%eax

	movl	lop+4(%esp),%eax
	mull	rop(%esp)	/ high(lop) * low(rop)
	movl	%eax,%ecx	/ partial high(product)

	movl	rop+4(%esp),%eax
	mull	lop(%esp)	/ high(rop) * low(lop)
	addl	%eax,%ecx	/ partial sum of high(product)

	movl	rop(%esp),%eax
	mull	lop(%esp)	/ low(rop) * low(lop)
	addl	%edx,%ecx	/ final high(product)

	movl	%eax,%edx
	popl	%eax
	movl	%edx,ans(%eax)
	movl	%ecx,ans+4(%eax)

	ret
