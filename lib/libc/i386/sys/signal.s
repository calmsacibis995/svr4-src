	.file	"signal.s"
.ident	"@(#)libc-i386:libc-i386/sys/signal.s	1.9"

	.globl	signal
	.align	4
_fgdef_(signal):
	jmp	__signal	/ Do the work.
