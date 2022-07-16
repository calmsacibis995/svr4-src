	.file	"setjmp.s"

	.ident	"@(#)libc-i386:libc-i386/gen/setjmp.s	1.4"

/	longjmp(env, val)
/ will generate a "return(val)" from
/ the last call to
/	setjmp(env)
/ by restoring registers ip, sp, bp, bx, si, and di from 'env'
/ and doing a return.

/ entry    reg	offset from (%si)
/ env[0] = %ebx	 0	/ register variables
/ env[1] = %esi	 4
/ env[2] = %edi	 8
/ env[3] = %ebp	 12	/ stack frame
/ env[4] = %esp	 16
/ env[5] = %eip	 20

	.globl	setjmp
	.align	4

_fgdef_(setjmp):
	MCOUNT			/ subroutine entry counter if profiling
	movl	4(%esp),%eax	/ jmpbuf address
	movl	%ebx,0(%eax)	/ save ebx
	movl	%esi,4(%eax)	/ save esi
	movl	%edi,8(%eax)	/ save edi
	movl	%ebp,12(%eax)	/ save caller's ebp
	popl	%edx		/ return address
	movl	%esp,16(%eax)	/ save caller's esp
	movl	%edx,20(%eax)
	subl	%eax,%eax	/ return 0
	jmp	*%edx

	.globl	longjmp
	.align	4
_fgdef_(longjmp):
	MCOUNT			/ subroutine entry counter if profiling
	movl	4(%esp),%edx	/ first parameter after return addr
	movl	8(%esp),%eax	/ second parameter
	cmpl	12(%edx),%ebp	/ see if called from same procedure
	je	.sameproc	/ don't destroy the register variables
	movl	0(%edx),%ebx	/ restore ebx
	movl	4(%edx),%esi	/ restore esi
	movl	8(%edx),%edi	/ restore edi
	movl	12(%edx),%ebp	/ restore caller's ebp
	movl	16(%edx),%esp	/ restore caller's esp
	jmp	.lab1
.sameproc:
	addl	$8,%esp		/ remove two extra stack words. Arguments are
				/ popped after return from function call.
				/ Also, the return address is pushed by call
				/ and popped by ret.
				/ Since longjmp jumps to setjmp call and only
				/ one argument will be popped on return
				/ from setjmp, the other argument to longjmp
				/ and the return address are popped here. 
				/ If not called from same procedure, esp is 
				/ restored so the pops are not necessary.
.lab1:
	test	%eax,%eax	/ if val != 0
	jnz	.ret		/ 	return val
	incl	%eax		/ else return 1
.ret:
	jmp	*20(%edx)	/ return to caller
