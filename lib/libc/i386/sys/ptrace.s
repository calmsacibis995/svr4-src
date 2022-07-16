.ident	"@(#)libc-i386:libc-i386/sys/ptrace.s	1.4"

	.file	"ptrace.s"

	.text

	.globl	_cerror
	.globl	errno

_fwdef_(`ptrace'):
	_prologue_
	MCOUNT			/ subroutine entry counter if profiling
_m4_ifdef_(`DSHLIB',
`	movl	_daref_(errno),%ecx
	movl	$0,(%ecx)
',
`	movl	$0,errno
')
	movl	$PTRACE,%eax
	_epilogue_
	lcall	$0x7,$0
	jc	_cerror
	ret
