	.file	"exect.s"

	.ident	"@(#)libc-i386:libc-i386/sys/exect.s	1.5"

/ this is the same as execve described below.
/ It sets single step prior to exec call,
/ this will stop the user on the first instruction executed
/ and allow the parent to set break points as appropriate.
/ This is used by tracing mechanisms,such as sdb.
/ execve(path, argv, envp);
/ char	*path, *argv[], *envp[];

_m4_define_(`PS_T', 0x0100)

	.globl	_cerror

_fwdef_(`exect'):
	MCOUNT			/ subroutine entry counter if profiling
	movl	$EXECE,%eax

/ set the single step flag bit (trap flag)
	pushf
	popl	%edx
	orl	$PS_T,%edx
	pushl	%edx
	popf
/ this has now set single step which should be preserved by the system
	lcall	$0x7,$0		/ call gate into OS
	jmp	_cerror		/ came back
