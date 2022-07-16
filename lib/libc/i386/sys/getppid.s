.ident	"@(#)libc-i386:libc-i386/sys/getppid.s	1.4"

	.file	"getppid.s"

	.text

_fwdef_(`getppid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GETPID,%eax
	lcall	$0x7,$0
	movl	%edx,%eax
	ret
