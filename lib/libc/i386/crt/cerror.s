	.file	"cerror.s"

	.ident	"@(#)libc-i386:libc-i386/crt/cerror.s	1.3"

/ C return sequence which sets errno, returns -1.

	.globl	_cerror
	.globl	errno

_fgdef_(_cerror):
	_prologue_
_m4_ifdef_(`DSHLIB',
`	movl	_daref_(errno),%ecx
	movl	%eax,(%ecx)
',
`	movl	%eax,errno
')
	movl	$-1,%eax
	_epilogue_
	ret
