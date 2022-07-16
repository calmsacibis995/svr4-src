	.file	"cuexit.s"

	.ident	"@(#)libc-i386:libc-i386/gen/cuexit.s	1.4"

/ C library -- exit
/ exit(code)
/ code is return in %edx to system


	.globl	exit
	.align	4

_fgdef_(exit):
	_prologue_
	MCOUNT
	call	_fref_(_exithandle)
	movl	_esp_(4),%edx
	movl	$EXIT,%eax
	_epilogue_
	lcall	$0x7,$0
	jc	_cerror
	ret
