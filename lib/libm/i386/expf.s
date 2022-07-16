	.file	"expf.s"
	.ident	"@(#)libm:i386/expf.s	1.3"
//////////////////////////////////////////////////////////////
/
/	float expf(float x)
/	
/	algorithm and coefficients from Cody and Waite
/
//////////////////////////////////////////////////////////////
	.data
	.align	4
.p:
	.long	0x3b885308
	.long	0x3e800000
	.align	4
.q:
	.long	0x3d4cbf5b
	.long	0x3f000000
	.align	4
.xeps:
	.long	0x323504f3
	.align	4
.LNMINFLOAT:
	.long	0xc2ce8ed0
	.align	4
.MINFLOAT:
	.long	0x1
	.align	4
.fzero:
	.long	0x0
	.align	4
.LNMAXFLOAT:
	.long	0x42b17218
	.align	4
.dzero:
	.long	0x0,0x0
.MAXFLOAT:
	.long	0x7f7fffff
	.align	4
.HUGE:
	.long	0xe0000000,0x47efffff
	.align	4
.huge_val:
	.long	0x0,0x7ff00000
.half:
	.long	0x3f000000
	.align	4
.C1:
	.long	0x3f318000
	.align	4
.C2:
	.long	0xb95e8083
	.text
	.align	4
	.globl	expf
expf:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$68,%esp
	MCOUNT
	pushl	%edi
	flds	8(%ebp)		/ |x|
	fld	%st(0)
	fabs			
	fcomps	.xeps		/ if (|x| < xeps)
	fstsw	%ax
	sahf	
	jae	.L56
	fld1
	fadd			/ return 1.0 + x
.L49:
	popl	%edi
	leave	
	ret	
	.align	4
.L56:
	fcoms	.LNMINFLOAT	/ if (x<= LN_MINFLOAT)
	fstsw	%ax
	sahf	
	ja	.L60
	jne	.L62		/ if (exc.arg1 == LN_MINFLOAT)
	flds	.MINFLOAT	/ return MINFLOAT
	ffree	%st(1)
	popl	%edi
	leave	
	ret	
	.align	4
.L62:
	fstpl	-32(%ebp)	/ exc.arg1 = x
	movl	$.L59,-36(%ebp)	/ exc.name = "expf";
	movl	$4,-40(%ebp)	/ exc.type = UNDERFLOW;
	movl	.dzero+4,%eax	/ exc.retval = 0.0;
	movl	%eax,-12(%ebp)
	movl	.dzero,%eax
	movl	%eax,-16(%ebp)
	movl	$2,%eax
	cmpl	%eax,_lib_version / if (_lib_version == strict_ansi)
	jne	.L66
..1:
	movl	$34,errno	/ errno =ERANGE;
	jmp	.L67
	.align	4
.L60:
	fcoms	.LNMAXFLOAT	 / if (x>= LN_MAXFLOAT)
	fstsw	%ax
	sahf	
	jb	.L69
	jne	.L71		/ if (exc.arg1 == LN_MAXFLOAT)
	flds	.MAXFLOAT	/ return MAXFLOAT
	ffree	%st(1)
	popl	%edi
	leave	
	ret	
	.align	4
.L71:
	fstpl	-32(%ebp)	/ exc.arg1 = x
	movl	$.L59,-36(%ebp)	/ exc.name = "expf";
	movl	$3,-40(%ebp)	/ exc.type = OVERFLOW
	cmpl	$0,_lib_version	/ if (_lib_version == c_issue_4)
	jne	.L74
	movl	.HUGE+4,%eax 	/ return HUGE
	movl	%eax,-12(%ebp)
	movl	.HUGE,%eax
	jmp	..0
	.align	4
.L74:
	movl	.huge_val+4,%eax	/ else return HUGE_VAL
	movl	%eax,-12(%ebp)
	movl	.huge_val,%eax
..0:
	movl	%eax,-16(%ebp)
	movl	$2,%eax			/ if (_lib_version - strict_ansi)
	cmpl	%eax,_lib_version
	je	..1
.L66:
	leal	-40(%ebp),%eax		/ else
	pushl	%eax
	call	matherr			/ if (!matherr(&exc))
	popl	%ecx
	testl	%eax,%eax
	je	..1
.L67:
	fldl	-16(%ebp)		/ return exc.retval
	popl	%edi
	leave	
	ret	
	.align	4
.L69:
	fldl2e			/ log2e
	fmul	%st(1),%st		/ x * log2e
	frndint			/ y = rnd(x * log2e)
	fistl	-48(%ebp)	/ n = (int)y
	fld	%st(0)		/ tmp = y
	fmuls	.C1		/ tmp *= C1
	fsubr	%st,%st(2)	/ x = x - Y * C1
	fxch
	fmuls	.C2		/ y *= C2
	fsubr	%st,%st(2)	/ x = (x - y * C1) - y * C2
	fld	%st(2)
	fmul	%st(0),%st	/ y = x * x
	fst	%st(1)
	fmuls	.p		/ y * p[0]
	fadds	.p+4		/   + p[1]
	fmul	%st,%st(3)	/ x *= ..
	fld	%st(1)
	fmuls	.q		/ y * q[0]
	fadds	.q+4		/   + q[1]
	fsub 	%st(4),%st		/   - x
	fdivr	%st(4),%st	/ x/(_POLY1(y,q) -x)
	fadds	.half		/ + 0.5
	addl	$1,-48(%ebp)	/ n+= 1
	fild	-48(%ebp)	/ return ldexp(x,n+1)
	fxch
	fscale
	ffree	%st(1)
	ffree	%st(2)
	ffree	%st(3)
	ffree	%st(4)
	ffree	%st(5)
	popl	%edi
	leave	
	ret	
	.align	4
	.data
.L59:
	.byte	0x65,0x78,0x70,0x66,0x00
	.text
