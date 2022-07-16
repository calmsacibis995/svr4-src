.ident	"@(#)libc-i386:libc-i386/sys/fork.s	1.4"

/ pid = fork();

/ %edx == 0 in parent process, %edx = 1 in child process.
/ %eax == pid of child in parent, %eax == pid of parent in child.

	.file	"fork.s"
	
	.text

	.globl	_cerror

_fwdef_(`fork'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$FORK,%eax
	lcall	$0x7,$0
	jc	_cerror
	testl	%edx,%edx
	jz	.parent
	xorl	%eax,%eax
.parent:
	ret
