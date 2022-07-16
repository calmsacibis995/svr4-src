.ident	"@(#)libc-i386:libc-i386/sys/getdents.s	1.5"

	.file	"`getdents.s'"

	.text

	.globl	_cerror

_m4_ifdef_(`ABI',`
	.globl	getdents
_fgdef_(getdents):
',`
_m4_ifdef_(`DSHLIB',`
	.globl	getdents
_fgdef_(getdents):
',`
_fwdef_(`getdents'):
')
')
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GETDENTS,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
