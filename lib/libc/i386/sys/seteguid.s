.ident	"@(#)libc-i386:sys/seteguid.s	1.2.1.1"


	.file	"seteguid.s"

	.text

	.globl	_cerror

_m4_ifdef_(`ABI',`
	.globl	setegid
_fgdef_(setegid):
',`
_m4_ifdef_(`DSHLIB',`
	.globl	setegid
_fgdef_(setegid):
',`
_fwdef_(`setegid'):
')
')
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SETEGID,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret

_m4_ifdef_(`ABI',`
	.globl	seteuid
_fgdef_(seteuid):
',`
_m4_ifdef_(`DSHLIB',`
	.globl	seteuid
_fgdef_(seteuid):
',`
_fwdef_(`seteuid'):
')
')
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SETEUID,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
