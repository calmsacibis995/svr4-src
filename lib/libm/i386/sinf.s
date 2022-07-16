	.file	"sinf.s"
	.ident	"@(#)libm:i386/sinf.s	1.3"
/##################################################
/	float sinf(float x)
/	float cosf(float x)
/	float tanf(float x)
/ sinf, cosf and tanf routines - algorithm from paper by Peter Teng, UCB
/ argument is expressed as (n * pi/2 + f ), where
/ -pi/4 <= f <= pi/4 and f is in extended precision
/ use table look-up for values less than 64*pi/2;
/ Payne and Hanek range reduction for greater values
/ sin(x) = _sin(f)* -1**(s), for n even
/      and _cos(f)* -1**(s), for n odd (s == floor(n/2))
/ for cos(x), simply increment n by 1 after range reduction
/ tanx(x) = _sin(f)/_cos(f), for n even
/      and -_cos(f)/_sin(f), for n odd
/ If 80387 is attached, machine sin and cos are used
/ after range reduction is performed.
/-----------------------------------------------------
/ multiples of pi/2 expressed as sum of 2 doubles:
/ leading, trailing
/ extended precision multiples of pi/2 (168 bits) were generated
/ using bc and dc
/ leading = (float)(n *pi/2 in double)
/ trailing = (float)(n * pi/2 - leading in double)
/######################################################
	.data
	.align	4
leading:			/ n *pi/2 in double extended
	.long	0x0,0x0			/ leading[0]
	.long	0x54442d18,0x3ff921fb	/ leading[1]
	.long	0x54442d18,0x400921fb	/ leading[2]
	.long	0x7f3321d2,0x4012d97c	/ leading[3]
	.long	0x54442d18,0x401921fb	/ leading[4]
	.long	0x2955385e,0x401f6a7a	/ leading[5]
	.long	0x7f3321d2,0x4022d97c	/ leading[6]
	.long	0xe9bba775,0x4025fdbb	/ leading[7]
	.long	0x54442d18,0x402921fb	/ leading[8]
	.long	0xbeccb2bb,0x402c463a	/ leading[9]
	.long	0x2955385e,0x402f6a7a	/ leading[10]
	.long	0xc9eedf01,0x4031475c	/ leading[11]
	.long	0x7f3321d2,0x4032d97c	/ leading[12]
	.long	0x347764a4,0x40346b9c	/ leading[13]
	.long	0xe9bba775,0x4035fdbb	/ leading[14]
	.long	0x9effea47,0x40378fdb	/ leading[15]
	.long	0x54442d18,0x403921fb	/ leading[16]
	.long	0x9886fea,0x403ab41b	/ leading[17]
	.long	0xbeccb2bb,0x403c463a	/ leading[18]
	.long	0x7410f58d,0x403dd85a	/ leading[19]
	.long	0x2955385e,0x403f6a7a	/ leading[20]
	.long	0xef4cbd98,0x40407e4c	/ leading[21]
	.long	0xc9eedf01,0x4041475c	/ leading[22]
	.long	0xa4910069,0x4042106c	/ leading[23]
	.long	0x7f3321d2,0x4042d97c	/ leading[24]
	.long	0x59d5433b,0x4043a28c	/ leading[25]
	.long	0x347764a4,0x40446b9c	/ leading[26]
	.long	0xf19860c,0x404534ac	/ leading[27]
	.long	0xe9bba775,0x4045fdbb	/ leading[28]
	.long	0xc45dc8de,0x4046c6cb	/ leading[29]
	.long	0x9effea47,0x40478fdb	/ leading[30]
	.long	0x79a20bb0,0x404858eb	/ leading[31]
	.long	0x54442d18,0x404921fb	/ leading[32]
	.long	0x2ee64e81,0x4049eb0b	/ leading[33]
	.long	0x9886fea,0x404ab41b	/ leading[34]
	.long	0xe42a9153,0x404b7d2a	/ leading[35]
	.long	0xbeccb2bb,0x404c463a	/ leading[36]
	.long	0x996ed424,0x404d0f4a	/ leading[37]
	.long	0x7410f58d,0x404dd85a	/ leading[38]
	.long	0x4eb316f6,0x404ea16a	/ leading[39]
	.long	0x2955385e,0x404f6a7a	/ leading[40]
	.long	0x1fbace4,0x405019c5	/ leading[41]
	.long	0xef4cbd98,0x40507e4c	/ leading[42]
	.long	0xdc9dce4c,0x4050e2d4	/ leading[43]
	.long	0xc9eedf01,0x4051475c	/ leading[44]
	.long	0xb73fefb5,0x4051abe4	/ leading[45]
	.long	0xa4910069,0x4052106c	/ leading[46]
	.long	0x91e2111e,0x405274f4	/ leading[47]
	.long	0x7f3321d2,0x4052d97c	/ leading[48]
	.long	0x6c843287,0x40533e04	/ leading[49]
	.long	0x59d5433b,0x4053a28c	/ leading[50]
	.long	0x472653ef,0x40540714	/ leading[51]
	.long	0x347764a4,0x40546b9c	/ leading[52]
	.long	0x21c87558,0x4054d024	/ leading[53]
	.long	0xf19860c,0x405534ac	/ leading[53]
	.long	0xfc6a96c1,0x40559933	/ leading[54]
	.long	0xe9bba775,0x4055fdbb	/ leading[55]
	.long	0xd70cb82a,0x40566243	/ leading[57]
	.long	0xc45dc8de,0x4056c6cb	/ leading[58]
	.long	0xb1aed992,0x40572b53	/ leading[59]
	.long	0x9effea47,0x40578fdb	/ leading[60]
	.long	0x8c50fafb,0x4057f463	/ leading[61]
	.long	0x79a20bb0,0x405858eb	/ leading[62]
	.long	0x66f31c64,0x4058bd73	/ leading[63]
	.long	0x54442d18,0x405921fb	/ leading[64]
trailing:
	.long	0x0,0x0			/ trailing[0]
	.long	0x33145c07,0x3c91a626	/ trailing[1]
	.long	0x33145c07,0x3ca1a626	/ trailing[2]
	.long	0x4c9e8a0a,0x3caa7939	/ trailing[3]
	.long	0x33145c07,0x3cb1a626	/ trailing[4]
	.long	0xbfd97309,0x3cb60faf	/ trailing[5]
	.long	0x4c9e8a0a,0x3cba7939	/ trailing[6]
	.long	0xd963a10c,0x3cbee2c2	/ trailing[7]
	.long	0x33145c07,0x3cc1a626	/ trailing[8]
	.long	0xf976e788,0x3cc3daea	/ trailing[9]
	.long	0xbfd97309,0x3cc60faf	/ trailing[10]
	.long	0xbce200bb,0xbcd3ddc5	/ trailing[11]
	.long	0x4c9e8a0a,0x3cca7939	/ trailing[12]
	.long	0xf67f753a,0xbcd1a900	/ trailing[13]
	.long	0xd963a10c,0x3ccee2c2	/ trailing[14]
	.long	0x6039d373,0xbccee878	/ trailing[15]
	.long	0x33145c07,0x3cd1a626	/ trailing[16]
	.long	0xd374bc71,0xbcca7eee	/ trailing[17]
	.long	0xf976e788,0x3cd3daea	/ trailing[18]
	.long	0x46afa570,0xbcc61565	/ trailing[19]
	.long	0xbfd97309,0x3cd60faf	/ trailing[20]
	.long	0xb9ea8e6e,0xbcc1abdb	/ trailing[21]
	.long	0xbce200bb,0xbce3ddc5	/ trailing[22]
	.long	0x74b6a225,0x3cecaf6b	/ trailing[23]
	.long	0x4c9e8a0a,0x3cda7939	/ trailing[24]
	.long	0x40c0c0d5,0xbcb1b191	/ trailing[25]
	.long	0xf67f753a,0xbce1a900	/ trailing[26]
	.long	0x3b192da6,0x3ceee430	/ trailing[27]
	.long	0xd963a10c,0x3cdee2c2	/ trailing[28]
	.long	0x58c99c43,0xbc26d61b	/ trailing[29]
	.long	0x6039d373,0xbcdee878	/ trailing[30]
	.long	0xfe8446d9,0xbceee70a	/ trailing[31]
	.long	0x33145c07,0x3ce1a626	/ trailing[32]
	.long	0x2567f739,0x3cb19abb	/ trailing[33]
	.long	0xd374bc71,0xbcda7eee	/ trailing[34]
	.long	0x3821bb58,0xbcecb246	/ trailing[35]
	.long	0xf976e788,0x3ce3daea	/ trailing[36]
	.long	0xac3e29a0,0x3cc1a070	/ trailing[37]
	.long	0x46afa570,0xbcd61565	/ trailing[38]
	.long	0x71bf2fd8,0xbcea7d81	/ trailing[39]
	.long	0xbfd97309,0x3ce60faf	/ trailing[40]
	.long	0x8746f50c,0xbcfcb18f	/ trailing[41]
	.long	0xb9ea8e6e,0xbcd1abdb	/ trailing[42]
	.long	0xaa51add5,0x3cf3dba1	/ trailing[43]
	.long	0xbce200bb,0xbcf3ddc5	/ trailing[44]
	.long	0x6fa942d3,0x3cd1a34b	/ trailing[45]
	.long	0x74b6a225,0x3cfcaf6b	/ trailing[46]
	.long	0xe4fa18d6,0xbce613f7	/ trailing[47]
	.long	0x4c9e8a0a,0x3cea7939	/ trailing[48]
	.long	0xc0e4698b,0xbcfa7cca	/ trailing[49]
	.long	0x40c0c0d5,0xbcc1b191	/ trailing[50]
	.long	0x70b43955,0x3cf61066	/ trailing[51]
	.long	0xf67f753a,0xbcf1a900	/ trailing[52]
	.long	0x893370d7,0x3cda765e	/ trailing[53]
	.long	0x3b192da6,0x3cfee430	/ trailing[54]
	.long	0x583501d4,0xbce1aa6e	/ trailing[55]
	.long	0xd963a10c,0x3ceee2c2	/ trailing[56]
	.long	0xfa81de0a,0xbcf84805	/ trailing[57]
	.long	0x58c99c43,0xbc36d61b	/ trailing[58]
	.long	0x3716c4d6,0x3cf8452b	/ trailing[59]
	.long	0x6039d373,0xbceee878	/ trailing[60]
	.long	0xd15ecf6d,0x3ce1a4b8	/ trailing[61]
	.long	0xfe8446d9,0xbcfee70a	/ trailing[62]
	.long	0x96dfd5a5,0xbcda81c9	/ trailing[63]
	.long	0x33145c07,0x3cf1a626	/ trailing[64]
.MAXLOOKUP:
	.long	0x54442d18,0x405921fb	/ 64 * p/2
.twoopi:
	.long	0x6dc9c883,0x3fe45f30	/ 2/pi in double
.half:
	.long	0x0,0x3fe00000
/-----------------------------------------------------------------
	.text
	.align	4
	.globl	sinf
/-----------------------------------------------------------------
sinf:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$20,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	flds	8(%ebp)		
	fabs			/ y = |x|
	xorl	%ebx,%ebx	/ mode = 0, indicates sinf
	
/-----------------------------------------------------------------
	.text
	.align	4

/-----------------------------------------------------------------
.trig:
	fldl	.MAXLOOKUP
	fcomp			/ if (y > MAXLOOKUP)
	fstsw	%ax
	sahf	
	ja	.lookup		
				/ use Payne-Hanek reduction
	leal	-4(%ebp),%eax	/ n - quadrant
	pushl	%eax
	fstps	-8(%ebp)	/ y
	pushl	-8(%ebp)
	call	_reducef
	addl	$8,%esp
	movl	-4(%ebp),%esi	/ register int n
	jmp	.compx		/ reduced argument in %st(0)
.lookup:
	fldl	.twoopi
	fmul	%st(1),%st	/ tmp = y * 2/pi
	fstcw	-8(%ebp)	/ the folowing sets the rounding mode
				/ to round to nearest
				/ before converting float to int
				/ then restores the rounding mode.
	testl	$0xc00,-8(%ebp)
	je	.round		/ already round to nearest
	movl	-8(%ebp),%eax
	movl	%eax,-12(%ebp)
	andl	$0xfffff3ff,-12(%ebp) / set round to nearest
	fstcw	-12(%ebp)
	fistpl	-4(%ebp)	/ n = (int)tmp - quadrant
	movl	-4(%ebp),%esi
	fstcw	-8(%ebp)	/ restore old rounding mode
	jmp	.sub
.round:
	fldl	.half
	fcomp			/ if tmp <= 0.5 tmp = 0.0
	fstsw	%ax		/ this is a workaround for an
				/ 80387 chip bug
	sahf	
	jbe	.round2
	fstp	%st(0)		/ pop stack
	fldz	
.round2:
	fistpl	-4(%ebp)	/ n = (int)tmp - quadrant
	movl	-4(%ebp),%esi
.sub:
	leal	leading(,%esi,8),%edx
	fldl	(%edx)
	fsubrp	%st,%st(1)		/ y = y - leading[n]
	leal	trailing(,%esi,8),%edx
	fldl	(%edx)
	fsubrp	%st,%st(1)		/ y = y - trailing[n]
.compx:
	testl	$0x80000000,8(%ebp) / if (x < 0.0)
	je	.pos
	fchs			/ y = -y
	subl	$4,%esi		
	negl	%esi		/ n = 4 -n
.pos:
	cmpl	$3,_fp_hw	/ 80387 attached?
	jne	.FP287
	cmpl	$2,%ebx		/ tanf ?
	je	.dotanf2
	addl	%ebx,%esi	/ if cosf n+=1
	movl	%esi,%edi	/ sign = n
	shr	$1,%edi		/ sign = n/2
	testl	$0x1,%esi	/ if (n % 2)
	je	.dosinf2
	fcos
	jmp	.comp2	
.dosinf2:
	fsin
.comp2:
	testl	$0x1,%edi	/ if (sign %2)
	je	.return2
	fchs
.return2:
	popl	%ebx
	popl	%esi
	popl	%edi
	leave
	ret
.dotanf2:
	fsincos			/sinf in st(1), cosf in st
	testl	$0x1,%esi	/ if (n %2)
	je	.even2
	fdiv	%st(1),%st
	fchs			/ return -cosf/sinf
	ffree	%st(1)
	jmp	.return2
.even2:
	fdivr	%st(1),%st	/ return sinf/cosf
	ffree	%st(1)
	jmp	.return2
.FP287:
	cmpl	$2,%ebx		/ tanf ?
	je	.dotanf
	addl	%ebx,%esi	/ if cosf n+=1
	movl	%esi,%edi	/ sign = n
	shr	$1,%edi		/ sign = n/2
	fstpl	-8(%ebp)
	pushl	-4(%ebp)
	pushl	-8(%ebp)
	testl	$0x1,%esi	/ if (n % 2)
	je	.dosinf
	call	_cosf
	jmp	.comp	
.dosinf:
	call	_sinf
.comp:
	addl	$8,%esp
	testl	$1,%edi		/ if (sign %2)
	je	.return
	fchs			/ return -y
.return:
	popl	%ebx
	popl	%esi
	popl	%edi
	leave
	ret
.dotanf:				/ tanf
	fstpl	-8(%ebp)
	pushl	-4(%ebp)
	pushl	-8(%ebp)
	call 	_cosf
	addl	$8,%esp
	fstps	-16(%ebp)	/ _cosf
	pushl	-4(%ebp)
	pushl	-8(%ebp)
	call	_sinf
	addl	$8,%esp
	flds	-16(%ebp)	/_cosf
	testl	$1,%esi		/ if ( n % 2)
	je	.even
	fdiv	%st(1),%st
	fchs			/ return -cosf/sinf
	ffree	%st(1)
	jmp	.return
.even:
	fdivr	%st(1),%st	/ return sinf/cosf
	ffree	%st(1)
	jmp	.return
/-----------------------------------------------------------------

	.text
	.align	4
	.globl	cosf
cosf:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$20,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	flds	8(%ebp)		
	fabs			/ y = |x|
	movl	$1,%ebx		/ mode = 1, indicates cosf
	jmp	.trig
/-----------------------------------------------------------------
	.text
	.align	4
	.globl	tanf
/-----------------------------------------------------------------
tanf:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$20,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	flds	8(%ebp)		
	fabs			/ y = |x|
	movl	$2,%ebx		/ mode = 2, indicates tanf
	jmp	.trig
/#################################################################
/	float _sinf(x)
/	float _cosf(x)
/	double x;
/	-pi/4 <= x <= pi/4
/	Algorithm and coefficients from paper by Peter Teng
/###################################################################
	.data
	.align	4
.p:

	.long	0xb94cab5b
	.long	0x3c0883bf
	.long	0xbe2aaaa3
	.align	4
.q:
	.long	0x37ccf127
	.long	0xbab6060f
	.long	0x3d2aaaa5
	.align	4
.d_two:
	.long	0x0,0x40000000	/2.0
/-------------------------------------------------------------------
	.text
	.align	4
/--------------------------------------------------------------------
_sinf:	
	pushl	%ebp
	movl	%esp,%ebp
	subl	$4,%esp
	MCOUNT
	fldl	8(%ebp)
	fld	%st(0)
	fmul	%st(1),%st	/ xsq = x= * x
	fstps	-4(%ebp)	/ Xsq = xsq rounded to float
	flds	-4(%ebp)
	fld	%st(0)
	fmuls	.p		/ Xsq * p[0]
	fadds	.p+4		/     + p[1]
	fmul	%st(1),%st	/     * Xsq
	fadds	.p+8		/     + p[2]
	fmul	%st(1),%st	/ Q(x) =  Xsq * _POLY5(Xsq,p)
	fmul	%st(2),%st	/ Q(x) *= x
	fadd	%st(2),%st	/ Q(x) += x
	ffree	%st(1)
	ffree	%st(2)
	leave
	ret
/--------------------------------------------------------------------
	.text
	.align	4
/--------------------------------------------------------------------
_cosf:	
	pushl	%ebp
	movl	%esp,%ebp
	subl	$4,%esp
	MCOUNT
	fldl	8(%ebp)
	fmul	%st(0),%st	/ xsq = x= * x
	fsts	-4(%ebp)	/ Xsq = xsq rounded to double
	flds	-4(%ebp)
	fld	%st(0)
	fmuls	.q		/ Xsq * q[0]
	fadds	.q+4		/     + q[1]
	fmul	%st(1),%st	/     * Xsq
	fadds	.q+8		/     + q[2]
	fmul	%st(1),%st	/ Q(x) =  Xsq * _POLY5(Xsq,p)
	fmul	%st(1),%st	/ Q(x) *=  Xsq 
	fxch	%st(2)		/ xsq
	fldl	.d_two
	fdivrp	%st,%st(1)	/ xsq /= 2.0
	fsub	%st(2),%st	/ xsq -= Q(x)
	fld1
	fsubp	%st,%st(1)	/ return 1.0 - xsq
	ffree	%st(1)
	ffree	%st(2)
	ffree	%st(3)
	leave
	ret
	.text
