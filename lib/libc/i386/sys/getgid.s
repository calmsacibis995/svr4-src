.ident	"@(#)libc-i386:libc-i386/sys/getgid.s	1.4"

	.file	"getgid.s"

	.text

_fwdef_(`getgid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GETGID,%eax
	lcall	$0x7,$0
	ret
