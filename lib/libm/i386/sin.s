	.file	"sin.s"
	.ident	"@(#)libm:i386/sin.s	1.3"
/##################################################
/	double sin(x)
/	double cos(x)
/	double tan(x)
/	double x;
/ sin, cos and tan routines - algorithm from paper by Peter Teng, UCB
/ argument is expressed as (n * pi/2 + f ), where
/ -pi/4 <= f <= pi/4 and f is in extended precision
/ use table look-up for values less than 64*pi/2;
/ Payne and Hanek range reduction for greater values
/ sin(x) = _sin(f)* -1**(s), for n even
/      and _cos(f)* -1**(s), for n odd (s == floor(n/2))
/ for cos(x), simply increment n by 1 after range reduction
/ tanx(x) = _sin(f)/_cos(f), for n even
/      and -_cos(f)/_sin(f), for n odd
/ If 80387 is attached, machine sin and cos are 
/ used after range reductions is performed.
/-----------------------------------------------------
/ multiples of pi/2 expressed as sum of 2 extendeds:
/ leading, trailing
/ extended precision multiples of pi/2 (168 bits) were generated
/ using bc and dc
/ leading = (double)(n *pi/2 in extended)
/ trailing = (double)(n * pi/2 - leading in extended)
/######################################################
	.data
	.align	4
leading:			/ n *pi/2 in double extended
	.long	0x0,0x0,0x0			/leading[0]
	.long	0x2168c234,0xc90fdaa2,0x3fff	/leading[1]
	.long	0x2168c234,0xc90fdaa2,0x4000	/leading[2]
	.long	0x990e91a7,0x96cbe3f9,0x4001	/leading[3]
	.long	0x2168c234,0xc90fdaa2,0x4001	/leading[4]
	.long	0xa9c2f2c1,0xfb53d14a,0x4001	/leading[5]
	.long	0x990e91a7,0x96cbe3f9,0x4002	/leading[6]
	.long	0xdd3ba9ee,0xafeddf4d,0x4002	/leading[7]
	.long	0x2168c234,0xc90fdaa2,0x4002	/leading[8]
	.long	0x6595da7b,0xe231d5f6,0x4002	/leading[9]
	.long	0xa9c2f2c1,0xfb53d14a,0x4002	/leading[10]
	.long	0x76f80584,0x8a3ae64f,0x4003	/leading[11]
	.long	0x990e91a7,0x96cbe3f9,0x4003	/leading[12]
	.long	0xbb251dca,0xa35ce1a3,0x4003	/leading[13]
	.long	0xdd3ba9ee,0xafeddf4d,0x4003	/leading[14]
	.long	0xff523611,0xbc7edcf7,0x4003	/leading[15]
	.long	0x2168c234,0xc90fdaa2,0x4003	/leading[16]
	.long	0x437f4e58,0xd5a0d84c,0x4003	/leading[17]
	.long	0x6595da7b,0xe231d5f6,0x4003	/leading[18]
	.long	0x87ac669e,0xeec2d3a0,0x4003	/leading[19]
	.long	0xa9c2f2c1,0xfb53d14a,0x4003	/leading[20]
	.long	0x65ecbf72,0x83f2677a,0x4004	/leading[21]
	.long	0x76f80584,0x8a3ae64f,0x4004	/leading[22]
	.long	0x88034b95,0x90836524,0x4004	/leading[23]
	.long	0x990e91a7,0x96cbe3f9,0x4004	/leading[24]
	.long	0xaa19d7b9,0x9d1462ce,0x4004	/leading[25]
	.long	0xbb251dca,0xa35ce1a3,0x4004	/leading[26]
	.long	0xcc3063dc,0xa9a56078,0x4004	/leading[27]
	.long	0xdd3ba9ee,0xafeddf4d,0x4004	/leading[28]
	.long	0xee46efff,0xb6365e22,0x4004	/leading[29]
	.long	0xff523611,0xbc7edcf7,0x4004	/leading[30]
	.long	0x105d7c23,0xc2c75bcd,0x4004	/leading[31]
	.long	0x2168c234,0xc90fdaa2,0x4004	/leading[32]
	.long	0x32740846,0xcf585977,0x4004	/leading[33]
	.long	0x437f4e58,0xd5a0d84c,0x4004	/leading[34]
	.long	0x548a9469,0xdbe95721,0x4004	/leading[35]
	.long	0x6595da7b,0xe231d5f6,0x4004	/leading[36]
	.long	0x76a1208d,0xe87a54cb,0x4004	/leading[37]
	.long	0x87ac669e,0xeec2d3a0,0x4004	/leading[38]
	.long	0x98b7acb0,0xf50b5275,0x4004	/leading[39]
	.long	0xa9c2f2c1,0xfb53d14a,0x4004	/leading[40]
	.long	0xdd671c69,0x80ce280f,0x4005	/leading[41]
	.long	0x65ecbf72,0x83f2677a,0x4005	/leading[42]
	.long	0xee72627b,0x8716a6e4,0x4005	/leading[43]
	.long	0x76f80584,0x8a3ae64f,0x4005	/leading[44]
	.long	0xff7da88d,0x8d5f25b9,0x4005	/leading[45]
	.long	0x88034b95,0x90836524,0x4005	/leading[46]
	.long	0x1088ee9e,0x93a7a48f,0x4005	/leading[47]
	.long	0x990e91a7,0x96cbe3f9,0x4005	/leading[48]
	.long	0x219434b0,0x99f02364,0x4005	/leading[49]
	.long	0xaa19d7b9,0x9d1462ce,0x4005	/leading[50]
	.long	0x329f7ac2,0xa038a239,0x4005	/leading[51]
	.long	0xbb251dca,0xa35ce1a3,0x4005	/leading[52]
	.long	0x43aac0d3,0xa681210e,0x4005	/leading[53]
	.long	0xcc3063dc,0xa9a56078,0x4005	/leading[54]
	.long	0x54b606e5,0xacc99fe3,0x4005	/leading[55]
	.long	0xdd3ba9ee,0xafeddf4d,0x4005	/leading[56]
	.long	0x65c14cf6,0xb3121eb8,0x4005	/leading[57]
	.long	0xee46efff,0xb6365e22,0x4005	/leading[58]
	.long	0x76cc9308,0xb95a9d8d,0x4005	/leading[59]
	.long	0xff523611,0xbc7edcf7,0x4005	/leading[60]
	.long	0x87d7d91a,0xbfa31c62,0x4005	/leading[61]
	.long	0x105d7c23,0xc2c75bcd,0x4005	/leading[62]
	.long	0x98e31f2b,0xc5eb9b37,0x4005	/leading[63]
	.long	0x2168c234,0xc90fdaa2,0x4005	/leading[64]
trailing:
	.long	0x0,0x0,0x0			/trailing[0]
	.long	0x80dc1cd1,0xc4c6628b,0x3fbf	/trailing[1]
	.long	0x80dc1cd1,0xc4c6628b,0x3fc0	/trailing[2]
	.long	0xa0a5159c,0x9394c9e8,0x3fc1	/trailing[3]
	.long	0x80dc1cd1,0xc4c6628b,0x3fc1	/trailing[4]
	.long	0x61132405,0xf5f7fb2e,0x3fc1	/trailing[5]
	.long	0xa0a5159c,0x9394c9e8,0x3fc2	/trailing[6]
	.long	0x430264dc,0xb0b658e8,0x3fc0	/trailing[7]
	.long	0x80dc1cd1,0xc4c6628b,0x3fc2	/trailing[8]
	.long	0xe1ef40d6,0xbabe5db9,0x3fc1	/trailing[9]
	.long	0x61132405,0xf5f7fb2e,0x3fc2	/trailing[10]
	.long	0xd12ea79f,0x8e90c77f,0x3fc2	/trailing[11]
	.long	0xa0a5159c,0x9394c9e8,0x3fc3	/trailing[12]
	.long	0x58b2d769,0xdfe13011,0x3fc3	/trailing[13]
	.long	0x430264dc,0xb0b658e8,0x3fc1	/trailing[14]
	.long	0x919cb608,0xf0f3f8c5,0x3fc2	/trailing[15]
	.long	0x80dc1cd1,0xc4c6628b,0x3fc3	/trailing[16]
	.long	0xc74ef4f1,0x889645a1,0x3fc0	/trailing[17]
	.long	0xe1ef40d6,0xbabe5db9,0x3fc2	/trailing[18]
	.long	0xa9056238,0xa9ab9505,0x3fc3	/trailing[19]
	.long	0x61132405,0xf5f7fb2e,0x3fc3	/trailing[20]
	.long	0x8c9072e9,0xa12230ab,0x3fc4	/trailing[21]
	.long	0xd12ea79f,0x8e90c77f,0x3fc3	/trailing[22]
	.long	0x449e34b6,0xed6e96d4,0x3fc4	/trailing[23]
	.long	0xa0a5159c,0x9394c9e8,0x3fc4	/trailing[24]
	.long	0xf2afda0d,0xe6ebf3f3,0x3fc2	/trailing[25]
	.long	0x58b2d769,0xdfe13011,0x3fc4	/trailing[26]
	.long	0xb4b9b850,0x86076325,0x3fc4	/trailing[27]
	.long	0x430264dc,0xb0b658e8,0x3fc2	/trailing[28]
	.long	0x6cc77a1d,0xd253c94e,0x3fc4	/trailing[29]
	.long	0x919cb608,0xf0f3f8c5,0x3fc3	/trailing[30]
	.long	0x26a9df54,0xf5017bb9,0x3fc1	/trailing[31]
	.long	0x80dc1cd1,0xc4c6628b,0x3fc4	/trailing[32]
	.long	0xb9c5fb6f,0xd5d92b3f,0x3fc3	/trailing[33]
	.long	0xc74ef4f1,0x889645a1,0x3fc1	/trailing[34]
	.long	0x94f0bf84,0xb738fbc8,0x3fc4	/trailing[35]
	.long	0xe1ef40d6,0xbabe5db9,0x3fc3	/trailing[36]
	.long	0x3fa05475,0xe1587c53,0x3fbe	/trailing[37]
	.long	0xa9056238,0xa9ab9505,0x3fc4	/trailing[38]
	.long	0xa18863d,0x9fa39034,0x3fc3	/trailing[39]
	.long	0x61132405,0xf5f7fb2e,0x3fc4	/trailing[40]
	.long	0x5e8d0275,0xce0f1721,0x3fc5	/trailing[41]
	.long	0x8c9072e9,0xa12230ab,0x3fc5	/trailing[42]
	.long	0x7527c6b9,0xe86a946b,0x3fc4	/trailing[43]
	.long	0xd12ea79f,0x8e90c77f,0x3fc4	/trailing[44]
	.long	0xb4d62218,0xd2dbea50,0x3fc2	/trailing[45]
	.long	0x449e34b6,0xed6e96d4,0x3fc5	/trailing[46]
	.long	0x72a1a529,0xc081b05e,0x3fc5	/trailing[47]
	.long	0xa0a5159c,0x9394c9e8,0x3fc5	/trailing[48]
	.long	0x9d510c20,0xcd4fc6e5,0x3fc4	/trailing[49]
	.long	0xf2afda0d,0xe6ebf3f3,0x3fc3	/trailing[50]
	.long	0xaaf66f6a,0xcce16872,0x3fc1	/trailing[51]
	.long	0x58b2d769,0xdfe13011,0x3fc5	/trailing[52]
	.long	0x86b647dd,0xb2f4499b,0x3fc5	/trailing[53]
	.long	0xb4b9b850,0x86076325,0x3fc5	/trailing[54]
	.long	0xc57a5187,0xb234f95f,0x3fc4	/trailing[55]
	.long	0x430264dc,0xb0b658e8,0x3fc3	/trailing[56]
	.long	0x3ec409aa,0xff40afc4,0x3fc5	/trailing[57]
	.long	0x6cc77a1d,0xd253c94e,0x3fc5	/trailing[58]
	.long	0x9acaea90,0xa566e2d8,0x3fc5	/trailing[59]
	.long	0x919cb608,0xf0f3f8c5,0x3fc4	/trailing[60]
	.long	0xeda396ee,0x971a2bd9,0x3fc4	/trailing[61]
	.long	0x26a9df54,0xf5017bb9,0x3fc2	/trailing[62]
	.long	0x52d8ac5d,0xf1b34901,0x3fc5	/trailing[63]
	.long	0x80dc1cd1,0xc4c6628b,0x3fc5	/trailing[64]
.MAXLOOKUP:
	.long	0x2168c235,0xc90fdaa2,0x4005	/ 64 *pi/2
.twoopi:
	.long	0x4e44152a,0xa2f9836e,0x3ffe	/ 2/pi in extended
.half:
	.long	0x0,0x80000000,0x3ffe
/-----------------------------------------------------------------
	.text
	.align	4
	.globl	sin
/-----------------------------------------------------------------
sin:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$20,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	fldl	8(%ebp)		
	fabs			/ y = |x|
	xorl	%ebx,%ebx	/ mode = 0, indicates sin
	
/-----------------------------------------------------------------
	.text
	.align	4

/-----------------------------------------------------------------
.trig:
	fldt	.MAXLOOKUP
	fcomp			/ if (y > MAXLOOKUP)
	fstsw	%ax
	sahf	
	ja	.lookup		/ use Payne-Hanek reduction
	leal	-4(%ebp),%eax	/ n - quadrant
	pushl	%eax
	fstpl	-12(%ebp)	/ y
	pushl	-8(%ebp)
	pushl	-12(%ebp)
	call	_reduce
	addl	$12,%esp
	movl	-4(%ebp),%esi	/ register int n
	jmp	.compx		/ reduced argument in %st(0)
.lookup:
	fldt	.twoopi
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
	fistpl	-4(%ebp)	/ n = (int)tmp: quadrant
	movl	-4(%ebp),%esi
	fstcw	-8(%ebp)	/ restore old rounding mode
	jmp	.sub
.round:
	fldt	.half
	fcomp			/ if tmp <= 0.5 tmp = 0.0
	fstsw	%ax		/ this is a workaround for an
				/ 80387 chip bug
	sahf	
	jbe	.round2
	fstp	%st(0)		/ pop stack
	fldz	
.round2:
	fistpl	-4(%ebp)	/ n = (int)tmp:  quadrant
	movl	-4(%ebp),%esi
.sub:
	imull	$12,%esi,%eax
	movl	$leading,%edx
	addl	%eax,%edx
	fldt	(%edx)
	fsubrp	%st,%st(1)		/ y = y - leading[n]
	movl	$trailing,%edx
	addl	%eax,%edx
	fldt	(%edx)
	fsubrp	%st,%st(1)		/ y = y - trailing[n]
.compx:
	testl	$0x80000000,12(%ebp) / if (x < 0.0)
	je	.pos
	fchs			/ y = -y
	subl	$4,%esi		
	negl	%esi		/ n = 4 -n
.pos:
	cmpl	$3,_fp_hw	/ 80387 attached?
	jne	.FP287
	cmpl	$2,%ebx		/ tan ?
	je	.dotan2
	addl	%ebx,%esi	/ if cos n+=1
	movl	%esi,%edi	/ sign = n
	shr	$1,%edi		/ sign = n/2
	testl	$0x1,%esi	/ if (n % 2)
	je	.dosin2
	fcos
	jmp	.comp2	
.dosin2:
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
.dotan2:
	fsincos			/sin in st(1), cos in st
	testl	$0x1,%esi	/ if (n %2)
	je	.even2
	fdiv	%st(1),%st
	fchs			/ return -cos/sin
	ffree	%st(1)
	jmp	.return2
.even2:
	fdivr	%st(1),%st	/ return sin/cos
	ffree	%st(1)
	jmp	.return2
.FP287:
	cmpl	$2,%ebx		/ tan ?
	je	.dotan
	addl	%ebx,%esi	/ if cos n+=1
	movl	%esi,%edi	/ sign = n
	shr	$1,%edi		/ sign = n/2
	fstpt	-12(%ebp)
	pushl	-4(%ebp)
	pushl	-8(%ebp)
	pushl	-12(%ebp)
	testl	$0x1,%esi	/ if (n % 2)
	je	.dosin
	call	_cos
	jmp	.comp	
.dosin:
	call	_sin
.comp:
	addl	$12,%esp
	testl	$1,%edi		/ if (sign %2)
	je	.return
	fchs			/ return -y
.return:
	popl	%ebx
	popl	%esi
	popl	%edi
	leave
	ret
.dotan:				/ tan
	fstpt	-12(%ebp)
	pushl	-4(%ebp)
	pushl	-8(%ebp)
	pushl	-12(%ebp)
	call 	_cos
	addl	$12,%esp
	fstpl	-20(%ebp)	/ _cos
	pushl	-4(%ebp)
	pushl	-8(%ebp)
	pushl	-12(%ebp)
	call	_sin
	addl	$12,%esp
	fldl	-20(%ebp)	/_cos
	testl	$1,%esi		/ if ( n % 2)
	je	.even
	fdiv	%st(1),%st
	fchs			/ return -cos/sin
	ffree	%st(1)
	jmp	.return
.even:
	fdivr	%st(1),%st	/ return sin/cos
	ffree	%st(1)
	jmp	.return
/-----------------------------------------------------------------

	.text
	.align	4
	.globl	cos
cos:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$20,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	fldl	8(%ebp)		
	fabs			/ y = |x|
	movl	$1,%ebx		/ mode = 1, indicates cos
	jmp	.trig
/-----------------------------------------------------------------
	.text
	.align	4
	.globl	tan
/-----------------------------------------------------------------
tan:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$20,%esp
	MCOUNT
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	fldl	8(%ebp)		
	fabs			/ y = |x|
	movl	$2,%ebx		/ mode = 2, indicates tan
	jmp	.trig
/#################################################################
/	double _sin(x)
/	double _cos(x)
/	long double x;
/	-pi/4 <= x <= pi/4
/	Algorithm and coefficients from paper by Peter Teng
/###################################################################
	.data
	.align	4
.p:

	.long	0xad2495f2,0x3de5de23
	.long	0x33569b98,0xbe5ae5f9
	.long	0x7262ecf8,0x3ec71de3
	.long	0x19e4a9ac,0xbf2a01a0
	.long	0x11110d97,0x3f811111
	.long	0x5555555a,0xbfc55555
	.align	4
.q:
	.long	0xbaf59d4b,0xbda8fb12
	.long	0x14cdc590,0x3e21ee9f
	.long	0x812b495b,0xbe927e4f
	.long	0x19cbf671,0x3efa01a0
	.long	0x16c1521f,0xbf56c16c
	.long	0x5555554c,0x3fa55555
	.align	4
.x_two:
	.long	0x0,0x80000000,0x4000
/-------------------------------------------------------------------
	.text
	.align	4
/--------------------------------------------------------------------
_sin:	
	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp
	MCOUNT
	fldt	8(%ebp)
	fld	%st(0)
	fmul	%st(1),%st	/ xsq = x= * x
	fstpl	-8(%ebp)	/ Xsq = xsq rounded to double
	fldl	-8(%ebp)
	fld	%st(0)
	fmull	.p		/ Xsq * p[0]
	faddl	.p+8		/     + p[1]
	fmul	%st(1),%st	/     * Xsq
	faddl	.p+16		/     + p[2]
	fmul	%st(1),%st	/     * Xsq
	faddl	.p+24		/     + p[3]
	fmul	%st(1),%st	/     * Xsq
	faddl	.p+32		/     + p[4]
	fmul	%st(1),%st	/     * Xsq
	faddl	.p+40		/     + p[5]
	fmul	%st(1),%st	/ Q(x) =  Xsq * _POLY5(Xsq,p)
	fmul	%st(2),%st	/ Q(x) *= x
	fadd	%st(2),%st	/ Q(x) += x
	ffree	%st(2)
	ffree	%st(1)
	leave
	ret
/--------------------------------------------------------------------
	.text
	.align	4
/--------------------------------------------------------------------
_cos:	
	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp
	MCOUNT
	fldt	8(%ebp)
	fmul	%st(0),%st	/ xsq = x= * x
	fstl	-8(%ebp)	/ Xsq = xsq rounded to double
	fldl	-8(%ebp)
	fld	%st(0)
	fmull	.q		/ Xsq * q[0]
	faddl	.q+8		/     + q[1]
	fmul	%st(1),%st	/     * Xsq
	faddl	.q+16		/     + q[2]
	fmul	%st(1),%st	/     * Xsq
	faddl	.q+24		/     + q[3]
	fmul	%st(1),%st	/     * Xsq
	faddl	.q+32		/     + q[4]
	fmul	%st(1),%st	/     * Xsq
	faddl	.q+40		/     + q[5]
	fmul	%st(1),%st	/ Q(x) =  Xsq * _POLY5(Xsq,p)
	fmul	%st(1),%st	/ Q(x) *=  Xsq 
	fxch	%st(2)		/ xsq
	fldt	.x_two
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
