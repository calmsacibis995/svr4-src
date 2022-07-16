.ident	"@(#)libc-i386:libc-i386/sys/sysi86.s	1.5"

	.file	"sysi86.s"
	
	.text

	.globl	_cerror

_m4_ifdef_(`ABI',`
	.globl	sysi86
_fgdef_(sysi86):
',`
_m4_ifdef_(`DSHLIB',`
	.globl	sysi86
_fgdef_(sysi86):
',`
_fwdef_(`sysi86'):
')
')
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SYSI86,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
