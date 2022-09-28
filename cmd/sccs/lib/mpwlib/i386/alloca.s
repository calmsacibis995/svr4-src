	.file	"alloca.s"

	.ident	"@(#)sccs:lib/mpwlib/i386/alloca.s	1.2"

	.globl  alloca

alloca:
	popl	%edx		/ %edx = Return address.
	popl	%eax		/ %eax = # bytes to allocate.
	movl	%esp,%ecx	/ %ecx points to area for saved regs.
	subl	%eax,%esp	/ Allocate the request.
	andl	$-4,%esp	/ Round down to word boundary.
	movl	%esp,%eax	/ Return beginning of allocation.
	pushl	8(%ecx)		/ Copy possible saved regs to top of
	pushl	4(%ecx)		/ stack so that they can be restored
	pushl	0(%ecx)		/ with pops when caller returns.
	pushl	%ecx		/ Caller will add 4 to stack when
				/this function returns.
	jmp	*%edx		/ Return
