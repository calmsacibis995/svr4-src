.ident	"@(#)libc-i386:libc-i386/sys/getegid.s	1.4"

	.file	"getegid.s"

	.text

_fwdef_(`getegid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GETGID,%eax
	lcall	$0x7,$0
	movl	%edx,%eax
	ret
