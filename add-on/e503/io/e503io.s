	.ident	"@(#)e503io.s	1.1	92/09/30	JPB"
	.ident	"@(#)e503io.s	3.2 Lachman System V STREAMS TCP source"
/
/	System V STREAMS TCP - Release 3.0
/
/	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
/
/	All Rights Reserved.
/
/	The copyright above and this notice must be preserved in all
/	copies of this source code.  The copyright above does not
/	evidence any actual or intended publication of this source
/	code.
/
/	This is unpublished proprietary trade secret source code of
/	Lachman Associates.  This source code may not be copied,
/	disclosed, distributed, demonstrated or licensed except as
/	expressly authorized by Lachman Associates.
/
/	System V STREAMS TCP was jointly developed by Lachman
/	Associates and Convergent Technologies.
/
	.set	PTR,    8
	.set	LEN,   12
	.set	PORT,  16
	.set	STREG, 20
	.set	DPRDY, 0x80

	.globl	e503ioout
e503ioout:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edx
	pushl	%esi
	pushl	%ecx
	cld
	movl	PTR(%ebp),%esi
otop:
	cmpl	$0,LEN(%ebp)
	jg	odprdy
	popl	%ecx
	popl	%esi
	popl	%edx
	leave
	ret
odprdy:
	movw	STREG(%ebp),%dx
oloop:
	inb	(%dx)
	testb	$DPRDY,%al
	je	oloop

	movl	$8,%ecx
	subl	%ecx,LEN(%ebp)
	jge	output
	movl	LEN(%ebp),%ecx
	addl	$8,%ecx
output:
	movw	PORT(%ebp),%dx
	rep	
	outsb
	jmp	otop

	.globl	e503ioin
e503ioin:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edx
	pushl	%edi
	pushl	%ecx
	cld
	movl	PTR(%ebp),%edi
itop:
	cmpl	$0,LEN(%ebp)
	jg	idprdy
	popl	%ecx
	popl	%edi
	popl	%edx
	leave
	ret
idprdy:
	movw	STREG(%ebp),%dx
iloop:
	inb	(%dx)
	testb	$DPRDY,%al
	je	iloop

	movl	$8,%ecx
	subl	%ecx,LEN(%ebp)
	jge	input
	movl	LEN(%ebp),%ecx
	addl	$8,%ecx
input:
	movw	PORT(%ebp),%dx
	rep	
	insb
	jmp	itop
