.ident	"@(#)libc-i386:libc-i386/sys/syscall.s	1.6"


	.file	"syscall.s"

	.text

	.globl	_cerror

_m4_ifdef_(`ABI',`
	.globl	syscall
_fgdef_(syscall):
',`
_m4_ifdef_(`DSHLIB',`
	.globl	syscall
_fgdef_(syscall):
',`
_fwdef_(`syscall'):
')
')
	MCOUNT			/ subroutine entry counter if profiling
	pop	%edx		/ return address.
	pop	%eax		/ system call number
	pushl	%edx
	lcall	$0x7,$0
	movl	0(%esp),%edx
	pushl	%edx		/ Add an extra entry to the stack
	jc	_cerror
	ret
