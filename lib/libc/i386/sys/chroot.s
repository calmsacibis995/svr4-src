
.ident	"@(#)libc-i386:libc-i386/sys/chroot.s	1.4"

	.file	"chroot.s"

	.text

	.globl	_cerror

_fwdef_(`chroot'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$CHROOT,%eax
	lcall	$0x7,$0
	jc	_cerror
	ret
