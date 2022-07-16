.ident	"@(#)libc-i386:sys/sysinfo.s	1.3"


	.file	"sysinfo.s"

	.text

	.globl	_cerror

_m4_ifdef_(`ABI',`
	.globl	sysinfo
_fgdef_(sysinfo):
',`
_m4_ifdef_(`DSHLIB',`
	.globl	sysinfo
_fgdef_(sysinfo):
',`
_fwdef_(`sysinfo'):
')
')
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SYSINFO,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
