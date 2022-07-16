	
.ident	"@(#)libc-i386:libc-i386/sys/getpid.s	1.4"

	.file	"getpid.s"

	.text


_fwdef_(`getpid'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$GETPID,%eax
	lcall	$0x7,$0
	ret
