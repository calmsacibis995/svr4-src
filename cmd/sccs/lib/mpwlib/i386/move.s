	.file	"move.s"

	.ident	"@(#)sccs:lib/mpwlib/i386/move.s	1.1"
	.ident	"@(#)move.s	1.1"

	.globl	move

move:
	pushl	%esi
	pushl	%edi
	movl	4(%ebp),%esi
	movl	8(%ebp),%edi
	movl	%edi,%eax
	movl	12(%ebp),%ecx
	rep ;  smovb
	popl	%edi
	popl	%esi
	ret
