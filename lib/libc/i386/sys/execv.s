.ident	"@(#)libc-i386:libc-i386/sys/execv.s	1.4"

	.file	"execv.s"

	.text

	.globl	execve
	.globl	environ

_fwdef_(`execv'):
	_prologue_
	MCOUNT
_m4_ifdef_(`DSHLIB',
`	movl	_daref_(environ),%ecx
	pushl	(%ecx)
',
`	pushl	environ		/ default environment
')
	pushl	_esp_(12)	/ argv ptr (1 arg + retaddr+ push)
	pushl	_esp_(12)	/ file name (0 args + retaddr + 2 push)
	call	_fref_(execve)
	addl	$12,%esp
	_epilogue_
	ret
