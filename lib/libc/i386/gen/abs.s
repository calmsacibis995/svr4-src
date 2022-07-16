	.file	"abs.s"

	.ident	"@(#)libc-i386:libc-i386/gen/abs.s	1.4"

/	/* Assembler program to implement the following C program */
/	int
/	abs(arg)
/	int	arg;
/	{
/		return((arg < 0)? -arg: arg);
/	}


	.text
	.globl	abs
	.globl	labs
	.align	4

_fgdef_(abs):
_fgdef_(labs):
	MCOUNT			/ subroutine entry counter if profiling

	movl	4(%esp),%eax	/ arg < 0?
	testl	%eax,%eax
	jns	.posit
	negl	%eax		/ yes, return -arg
.posit:
	ret
