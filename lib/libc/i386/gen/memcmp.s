	.file	"memcmp.s"

	.ident	"@(#)libc-i386:libc-i386/gen/memcmp.s	1.5"

	.globl	memcmp
	.align	4

/ This implementation conforms to SVID but does not implement
/ the same algorithm as the portable version because it is
/ inconvenient to get the difference of the differing characters.
/ Cannot compare word at a time because of byte ordering and
/ the comparison of chars is signed.

_fgdef_(memcmp):
	MCOUNT			/ profiling
	movl	%edi,%edx	/ save register variables
	pushl	%esi
	movl	8(%esp),%esi	/ %esi = address of string 1
	movl	12(%esp),%edi	/ %edi = address of string 2
	cmpl	%esi,%edi	/ The same string?
	je	.equal

	movl	16(%esp),%ecx	/ %ecx = length in bytes
	testl	%ecx,%ecx	/ If a zero length string, set zero cc

	repz ; 	scmpb		/ compare the bytes
	je	.equal

.notequal:
	jnc	.ret2
	movl	$-1,%eax
	jmp	.ret1
.ret2:
	movl	$1,%eax
.ret1:
	popl	%esi
	movl	%edx,%edi
	ret

	.align	4
.equal:
	popl	%esi
	movl	%edx,%edi
	xorl	%eax,%eax
	ret
