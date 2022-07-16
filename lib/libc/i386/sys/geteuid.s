.ident	"@(#)libc-i386:libc-i386/sys/geteuid.s	1.4"

	.file	"geteuid.s"

	.text

_fwdef_(`geteuid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GETUID,%eax
	lcall	$0x7,$0
	movl	%edx,%eax
	ret
