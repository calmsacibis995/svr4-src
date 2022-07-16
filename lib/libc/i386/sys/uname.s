.ident	"@(#)libc-i386:libc-i386/sys/uname.s	1.4"


	.file	"uname.s"

	.text

	.globl	errno
	.set	UNAME,0

_fwdef_(`uname'):
	MCOUNT			/ subroutine entry counter if profiling
	pushl	$UNAME		/ type
	pushl	$0		/ mv flag
	pushl	12(%esp)	/ utsname address (retaddr+$UNAME+0)
	subl	$4,%esp		/ where return address would be.
	movl	$UTSSYS,%eax
	lcall	$0x7,$0
	jc	.cerror
	addl	$16,%esp
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
