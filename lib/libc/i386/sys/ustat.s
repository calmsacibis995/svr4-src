.ident	"@(#)libc-i386:libc-i386/sys/ustat.s	1.4"


	.file	"ustat.s"

	.globl	errno
	.set	USTAT,2

_fwdef_(`ustat'):
	MCOUNT			/ subroutine entry counter if profiling
	pushl	$USTAT
	pushl	8(%esp)		/ dev (retaddr + $USTAT off %esp)
	pushl	16(%esp)	/ buf (retaddr + $USTAT + dev + 1 arg off %esp)
	subl	$4,%esp
	movl	$UTSSYS,%eax
	lcall	$0x7,$0
	jc	.cerror
	addl	$16,%esp
	xorl	%eax,%eax
	ret

.cerror:
	_prologue_
_m4_ifdef_(`DSHLIB',
`	movl	_daref_(errno),%ecx
	movl	%eax,(%ecx)
',
`	movl	%eax,errno
')
	movl	$-1,%eax
	_epilogue_
	addl	$16,%esp
	ret
