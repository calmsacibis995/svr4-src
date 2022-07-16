	.file	"atan.s"
	.ident	"@(#)libm:i386/atan.s	1.4"
/###########################################################
/ 	atan returns the arctangent of its double-precision argument,
/	in the range [-pi/2, pi/2].
/	There are no error returns.
/	atan2(y, x) returns the arctangent of y/x,
/	in the range (-pi, pi].
/	atan2 discovers what quadrant the angle is in and calls atan.
/	atan2 returns EDOM error and value 0 if both arguments are zero.
/
/###########################################################
	.data
	.align	4
.zero:
	.long	0x0,0x0
	.align	4
.one:
	.long	0x0,0x3ff00000
	.align	4
.BIG:
	.long	0x78b58c40,0x4415af1d / 1.0e20
	.align	4
.PI_2:
	.long	0x54442d18,0x3ff921fb
	.align	4
.XPI_2:		/ extended precision for accuracy
	.long	0x2168c235,0xc90fdaa2,0x3fff
	.align	4
.NPI_2:
	.long	0x54442d18,0xbff921fb
	.text
	.align	4
	.globl	atan
atan:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp
	MCOUNT
	pushl	%edi
	xorl	%edi,%edi
	fldl	8(%ebp)		/ if (x < 0.0) {
	ftst
	fstsw	%ax
	sahf	
	jae	.L51
	fchs			/ x = -x;
	movl	$1,%edi		/ neg_x = 1;
.L51:
	fcoml	.BIG		/ if (x >= 1e20)
	fstsw	%ax
	sahf	
	jb	.L53
	testl	%edi,%edi
	je	.L57		/ return(neg_x ? -M_PI_2 : M_PI_2)
	fldl	.NPI_2
	ffree	%st(1)
.L47:
	popl	%edi
	leave	
	ret	
	.align	4
.L57:
	fldl	.PI_2
	ffree	%st(1)
	popl	%edi
	leave	
	ret	
	.align	4
.L53:
	fcoml	.one		/ if (x < 1.0)
	fstsw	%ax
	sahf	
	jae	.L59
	ftst			/ if (!x)
	fstsw	%ax
	sahf	
	jne	.L60
	fldz			/ return(0.0)
	ffree	%st(1)
	popl	%edi
	leave	
	ret	
	.align	4
.L60:
/ASM
	fld1			/ else z = xatan(x,1.0)
	fpatan
/ASMEND0
	jmp	..0
	.align	4
.L59:
/ASM
	fld1			/ else z = xatan(1.0,x)
	fxch
	fpatan
/ASMEND1
	fldt	.XPI_2
	fsub	%st(1),%st		/ z = PI_2 -z
	ffree	%st(1)
..0:
	testl	%edi,%edi	/ return(neg_x ? -z : z);
	je	.L64
	fchs	
.L64:
	popl	%edi
	leave	
	ret	
//////////////////////////////////////////////////////////////////
	.text
	.align	4
	.globl	atan2
atan2:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$40,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	xorl	%edi,%edi	/ neg_x = 0
	xorl	%esi,%esi	/ neg_y = 0
	fldl	16(%ebp)
	ftst	
	fstsw	%ax
	sahf	
	jne	.L71		/ if !x
	fldl	8(%ebp)
	ftst	
	fstsw	%ax
	sahf	
	jne	.L73		/ && !y
	movl	$1,-40(%ebp)	/ exc.type = DOMAIN
	movl	$.L72,-36(%ebp)	/ exc.name = "atan2"
	fstpl	-32(%ebp)	/ exc.arg1 = y
	fstpl	-24(%ebp)	/ exc.arg2 = x
	xorl	%eax,%eax	/ exc.retval = 0.0
	movl	%eax,-12(%ebp)
	movl	%eax,-16(%ebp)
	movl	$2,%eax
	cmpl	%eax,_lib_version	/ if (_lib_version==strict_ansi) 
	jne	.L74
	jmp	.L77
	.align	4
..1:
	cmpl	$0,_lib_version		/ if (_lib_version== c_issue_4)
	jne	.L77
	pushl	$20		/ write(2,"atan2: DOMAIN error\n",20);
	pushl	$.L79
	pushl	$2
	call	write
	addl	$12,%esp
.L77:
	movl	$33,errno	/ errno = EDOM
	jmp	.L75
	.align	4
.L74:
	leal	-40(%ebp),%eax	/ if (!matherr(&exc))
	pushl	%eax
	call	matherr
	popl	%ecx
	testl	%eax,%eax
	je	..1
.L75:
	fldl	-16(%ebp)	/ return exc.retval
.L68:
	popl	%esi
	popl	%edi
	leave	
	ret	
	.align	4
.L71:
	fldl	8(%ebp)
.L73:
	fld	%st(0)
	fadd	%st(2),%st	/ if (y+x==y)
	fcomp
	fstsw	%ax
	sahf	
	jne	.L80
	ftst			/ 	if (y > 0.0)
	fstsw	%ax
	sahf	
	jbe	.L81
	fldl	.PI_2		/		return pi/2
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L81:
	fldl	.NPI_2		/		else return -pi/2
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L80:
	ftst			/ if (y < 0.0)
	fstsw	%ax
	sahf	
	jae	.L85
	movl	$1,%esi		/ neg_y = 1
	fchs			/ y = -y
.L85:
	fxch
	ftst			/ if (x < 0.0)
	fstsw	%ax
	sahf	
	jae	.L87
	movl	$1,%edi		/ neg_x = 1
	fchs			/ x = -x
.L87:
	fcom			/ if( ay < ax)
	fstsw	%ax
	sahf	
	jb	.L89
	fxch
	ftst			/		if (!y)
	fstsw	%ax
	sahf	
	jne	.L90
	testl	%edi,%edi	/			if(!neg_x)
	jne	.L91
	fldz			/				return 0.0
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L91:
	fldpi			/			else return pi
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L90:
/ASM
	fxch
	fpatan			/		else z = xatan(ay,ax)
/ASMEND2
	jmp	..2
	.align	4
.L89:
	ftst			/ else if (!x)
	fstsw	%ax
	sahf	
	jne	.L95
	testl	%esi,%esi	/ 	if (!neg_y)
	jne	.L96
	fldl	.PI_2		/		return pi/2
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L96:
	fldl	.NPI_2		/		else return -pi/
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L95:
/ASM
	fxch
	fpatan			/ else z = xatan(ax,ay)
/ASMEND3
	fldt	.XPI_2
	fsub	%st(1),%st		/ z = PI_2 -z
	ffree	%st(2)
..2:
	testl	%edi,%edi	/ if (neg_x)
	je	.L100
	fldpi
	fsub	%st(1),%st	/ z = pi-z
	ffree	%st(2)
.L100:
	testl	%esi,%esi	/ if (neg_y)
	je	.L102
	fchs			/ z = -z
.L102:
	popl	%esi
	popl	%edi
	ffree	%st(1)
	leave			/ return z
	ret	
	.align	4
	.data
.L72:
	.byte	0x61,0x74,0x61,0x6e,0x32,0x00
.L79:
	.byte	0x61,0x74,0x61,0x6e,0x32,0x3a,0x20,0x44,0x4f,0x4d
	.byte	0x41,0x49,0x4e,0x20,0x65,0x72,0x72,0x6f,0x72,0x0a
	.byte	0x00
	.text
