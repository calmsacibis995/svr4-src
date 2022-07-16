.ident	"@(#)libc-i386:libc-i386/sys/getuid.s	1.4"

	.file	"getuid.s"

	.text

_fwdef_(`getuid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GETUID,%eax
	lcall	$0x7,$0
	ret
