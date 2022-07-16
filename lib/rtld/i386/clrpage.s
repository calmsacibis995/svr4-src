	.ident	"@(#)rtld:i386/clrpage.s	1.1"
	.file	"clrpage.s"

/ _clrpage(dst, cnt)
/	Fast assembly routine to zero page.
/	Sets cnt bytes to zero, starting at dst
/	Dst and cnt must be coordinated to give word alignment
/		(guaranteed if used to zero a page)

	.set	.dst,0x8	/ offset of arg0
	.set	.cnt,0xc	/ offset of arg1
	.globl	_clrpage
	.type	_clrpage,@function
	.text
	.align	4

_clrpage:
	pushl	%edi
	movl	.dst(%esp),%edi	/ dst
	movl	.cnt(%esp),%ecx	/ cnt
	movl	$0,%eax		/ value to store
	movl	%ecx,%edx
	andl	$3,%ecx
	repz; stosb		/ clear the memory
	movl	%edx,%ecx
	shrl	$2,%ecx
	repz; stosl		/ clear the memory
	popl	%edi
	ret
