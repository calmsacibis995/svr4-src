	.file	"atanf.s"
	.ident	"@(#)libm:i386/atanf.s	1.4"
/###########################################################
/ 	atanf returns the arctangent of its single-precision argument,
/	in the range [-pi/2, pi/2].
/	There are no error returns.
/	atan2f(y, x) returns the arctangent of y/x,
/	in the range (-pi, pi].
/	atan2f discovers what quadrant the angle is in and calls atanf.
/	atan2f returns EDOM error and value 0 if both arguments are zero.
/
/###########################################################
	.data
	.align	4
.one:
	.long	0x3f800000
	.align	4
.BIG:
	.long	0x4cbebc20	/ 1.0e8
	.align	4
.PI_2:
	.long	0x3fc90fdb
	.align	4
.DPI_2:		/ double precision for accuracy
	.long	0x54442d18,0x3ff921fb
	.align	4
.NPI_2:
	.long	0xbfc90fdb
/ special value of PI 1 ulp less than M_PI rounded to float
/ to satisfy the range requirements. (float)M_PI is actually
/ greater than true PI
.FM_PI:
	.align	4
	.long	0x40490fda
	.text
	.align	4
	.globl	atanf
atanf:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp
	MCOUNT
	pushl	%edi
	xorl	%edi,%edi	/ neg_x = 0
	flds	8(%ebp)		/ if (x < 0.0) {
	ftst
	fstsw	%ax
	sahf	
	jae	.L51
	fchs			/ x = -x;
	movl	$1,%edi		/ neg_x = 1;
.L51:
	fcoms	.BIG		/ if (x >= 1e20)
	fstsw	%ax
	sahf	
	jb	.L53
	testl	%edi,%edi
	je	.L57		/ return(neg_x ? -M_PI_2 : M_PI_2)
	flds	.NPI_2
	ffree	%st(1)
.L47:
	popl	%edi
	leave	
	ret	
	.align	4
.L57:
	flds	.PI_2
	ffree	%st(1)
	popl	%edi
	leave	
	ret	
	.align	4
.L53:
	fcoms	.one		/ if (x < 1.0)
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
	fldl	.DPI_2
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
	.globl	atan2f
atan2f:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$40,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	xorl	%edi,%edi	/ neg_x = 0
	xorl	%esi,%esi	/ neg_y = 0
	flds	12(%ebp)
	ftst	
	fstsw	%ax
	sahf	
	jne	.L71		/ if !x
	flds	8(%ebp)
	ftst	
	fstsw	%ax
	sahf	
	jne	.L73		/ && !y
	movl	$1,-40(%ebp)	/ exc.type = DOMAIN
	movl	$.L72,-36(%ebp)	/ exc.name = "atan2f"
	fstpl	-32(%ebp)	/ exc.arg1 = (double)y
	fstpl	-24(%ebp)	/ exc.arg2 = (double)x
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
	pushl	$21		/ write(2,"atan2f: DOMAIN error\n",21);
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
	fldl	-16(%ebp)	/ return (float)exc.retval
.L68:
	popl	%esi
	popl	%edi
	leave	
	ret	
	.align	4
.L71:
	flds	8(%ebp)
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
	flds	.PI_2		/		return pi/2
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L81:
	flds	.NPI_2		/		else return -pi/2
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
	testl	%edi,%edi	/		if(!neg_x)
	jne	.L91
	fldz			/			return 0.0
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L91:
	flds	.FM_PI			/	else return pi
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
	flds	.PI_2		/		return pi/2
	popl	%esi
	popl	%edi
	ffree	%st(1)
	ffree	%st(2)
	leave	
	ret	
	.align	4
.L96:
	flds	.NPI_2		/		else return -pi/
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
	fldl	.DPI_2
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
	.byte	0x61,0x74,0x61,0x6e,0x32,0x66,0x00
.L79:
	.byte	0x61,0x74,0x61,0x6e,0x32,0x66,0x3a,0x20,0x44,0x4f,0x4d
	.byte	0x41,0x49,0x4e,0x20,0x65,0x72,0x72,0x6f,0x72,0x0a
	.byte	0x00
	.text
