.ident	"@(#)libc-i386:libc-i386/sys/access.s	1.4"

/ access(file, request)
/ test ability to access file in all indicated ways
/ 1 - read
/ 2 - write
/ 4 - execute

	.file	"access.s"

	.text

/ access(file, request)
/ test ability to access file in all indicated ways
/ 1 - read
/ 2 - write
/ 4 - execute


	.globl  _cerror

_fwdef_(`access'):
	MCOUNT
	movl	$ACCESS,%eax
	lcall	$0x7,$0
	jc 	_cerror
	ret
