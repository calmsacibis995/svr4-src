.ident	"@(#)libc-i386:libc-i386/sys/sync.s	1.4"


	.file	"sync.s"
	
	.text

_fwdef_(`sync'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SYNC,%eax
	lcall	$0x7,$0
	ret
