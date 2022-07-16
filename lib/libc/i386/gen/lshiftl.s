
/ Shift a double long value.

	.ident	"@(#)libc-i386:gen/lshiftl.s	1.2"
	.file	"lshiftl.s"
	.text

	.set	arg,8
	.set	cnt,16
	.set	ans,0

_fwdef_(`lshiftl'):
	popl	%eax
	xchgl	%eax,0(%esp)

	MCOUNT

	pushl	%eax
	movl	arg(%esp),%eax
	movl	arg+4(%esp),%edx
	movl	cnt(%esp),%ecx
	orl	%ecx,%ecx
	jz	.lshiftld
	jns	.lshiftlp

/ We are doing a negative (right) shift

	negl	%ecx

.lshiftln:
	sarl	$1,%edx
	rcrl	$1,%eax
	loop	.lshiftln
	jmp	.lshiftld

/ We are doing a positive (left) shift

.lshiftlp:
	shll	$1,%eax
	rcll	$1,%edx
	loop	.lshiftlp

/ We are done.

.lshiftld:
	movl	%eax,%ecx
	popl	%eax
	movl	%ecx,ans(%eax)
	movl	%edx,ans+4(%eax)

	ret
