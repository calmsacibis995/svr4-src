	.file	"exp.s"
	.ident	"@(#)libm:i386/exp.s	1.3"
//////////////////////////////////////////////////////////////
/
/	double exp(x)
/	double x;
/	
/	algorithm and coefficients from Cody and Waite
/
//////////////////////////////////////////////////////////////
	.data
	.align	4
.p:
	.long	0x2ae6921e,0x3f008b44	/ 3.1555192765684646400000e-05
	.long	0xf22a12a6,0x3f7f074b   / 7.5753180159422777100000e-03
	.long	0x0,0x3fd00000	        / 2.5000000000000000000000e-01
	.align	4
.q:
	.long	0xce50455,0x3ea93363   / 7.5104028399870046400000e-07
	.long	0x5c28d4df,0x3f44af0c  / 6.3121894374398502800000e-04
	.long	0x51dfd9ff,0x3fad1728  / 5.6817302698551221900000e-02
	.long	0x0,0x3fe00000	       / 5.0000000000000000000000e-01
	.align	4
.L55:
	.long	0x0,0x0
	.align	4
.xeps:
	.long	0x667f3bcd,0x3e46a09e  / 1.0536712127723507900000e-08
	.align	4
.LNMINDOUBLE:
	.long	0x446d71c3,0xc0874385  / -7.4444007192138126000000e+02
	.align	4
.MINDOUBLE:
	.long	0x1,0x0		       / 4.9406564584124654e-324
	.align	4
.dzero:
	.long	0x0,0x0
	.align	4
.LNMAXDOUBLE:
	.long	0xfefa39ef,0x40862e42  / 7.0978271289338400100000e+02
	.align	4
.MAXDOUBLE:
	.long	0xffffffff,0x7fefffff  / 1.79769313486231470e308
	.align	4
.HUGE:
	.long	0xe0000000,0x47efffff  / 3.4028234663852886e38
	.align	4
.huge_val:
	.long	0x0,0x7ff00000 	       / infinity
.half:
	.long	0x0,0x3fe00000	       / 1/2
	.align	4
.L81:
	.long	0x652b82fe,0x3ff71547  / 1.4426950408889634200000e+00
	.align	4
.C1:
	.long	0x0,0x3fe63000	       / 6.9335937500000000000000e-01
	.align	4
.C2:
	.long	0x5c610ca8,0xbf2bd010  / -2.1219444005469058000000e-04
	.text
	.align	4
	.globl	exp
exp:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$68,%esp
	MCOUNT
	pushl	%edi
	fldl	8(%ebp)		/ |x|
	fld	%st(0)
	fabs			
	fcompl	.xeps		/ if (|x| < xeps)
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
	fcoml	.LNMINDOUBLE	/ if (x<= LN_MINDOUBLE)
	fstsw	%ax
	sahf	
	ja	.L60
	jne	.L62		/ if (exc.arg1 == LN_MINDOUBLE)
	fldl	.MINDOUBLE	/ return MINDOUBLE
	ffree	%st(1)
	popl	%edi
	leave	
	ret	
	.align	4
.L62:
	fstpl	-32(%ebp)	/ exc.arg1 = x
	movl	$.L59,-36(%ebp)	/ exc.name = "exp";
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
	fcoml	.LNMAXDOUBLE	 / if (x>= LN_MAXDOUBLE)
	fstsw	%ax
	sahf	
	jb	.L69
	jne	.L71		/ if (exc.arg1 == LN_MAXDOUBLE)
	fldl	.MAXDOUBLE	/ return MAXDOUBLE
	ffree	%st(1)
	popl	%edi
	leave	
	ret	
	.align	4
.L71:
	fstpl	-32(%ebp)	/ exc.arg1 = x
	movl	$.L59,-36(%ebp)	/ exc.name = "exp";
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
	fmull	.C1		/ tmp *= C1
	fsubr	%st,%st(2)	/ x = x - Y * C1
	fxch
	fmull	.C2		/ y *= C2
	fsubr	%st,%st(2)	/ x = (x - y * C1) - y * C2
	fld	%st(2)
	fmul	%st(0),%st	/ y = x * x
	fst	%st(1)
	fmull	.p		/ y * p[0]
	faddl	.p+8		/   + p[1]
	fmul	%st(1),%st		/   * y
	faddl	.p+16		/   + p[2]
	fmul	%st,%st(3)	/ x *= ..
	fld	%st(1)
	fmull	.q		/ y * q[0]
	faddl	.q+8		/   + q[1]
	fmul	%st(2),%st		/   * y
	faddl	.q+16		/   + q[2]
	fmul	%st(2),%st	/   * y
	faddl	.q+24		/   + q[3]
	fsub 	%st(4),%st		/   - x
	fdivr	%st(4),%st	/ x/(_POLY3(y,q) -x)
	faddl	.half		/ + 0.5
	addl	$1,-48(%ebp)	/ n+= 1
	fildl	-48(%ebp)	/ return ldexp(x,n+1)
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
	.byte	0x65,0x78,0x70,0x00
	.text
