	.file	"reduce.s"
	.ident	"@(#)libm:i386/reduce.s	1.2"
/####################################################################
/	Range reduction for the trigonemetric functions
/
/       See Payne and Hanek, SIGNUM Jan '83 page 19
/	Reduction is quadrant.
/
/       This implementation assumes:
/       a) the range of double precision floating point
/          is no greater than IEEE double precision,
/          else one needs more bits of pi.
/       b) L is less than 32 
/       c) L > 3
/       d) longs are 32 bits
/       e) floating point add/subtract are done via round to nearest
/       f) probably doesn't work on non-binary machines without some
/          more work.
/------------------------------------------------------------------
/   Inputs:
/   x is the argument to be reduced.  x is assumed to be positive and
/       greater than M_PI/2.
/
/
/   Outputs:
/ 
/   I      0 <= I <= 3    (integer) == quadrant number
/
/   returns hr == reduced argument in double extended precision
/		h*r  (where 0 <= h < 1.0,  r = (pi/2) )
/			(or -0.5 <= h < 0.5, for i == 1)
/
/   relationship of variables:
/
/   x = hr + (I*pi/2)    (modulo 2*pi)
/
/####################################################################
	.data
	.align	4
/ extended precision constants
.x_four:
	.long	0x0,0x80000000,0x4001
.M_PI_2:
	.long	0x2168c235,0xc90fdaa2,0x3fff
.twom15:
	.long	0x0,0x80000000,0x3ff0
	.align	4
pihex:			/ bits of 1/ 2*pi
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	683565275
	.long	-1819212470
	.long	2131351028
	.long	2102212464
	.long	920167782
	.long	1326507024
	.long	2140428522
	.long	-139529896
	.long	1841896334
	.long	-1869384520
	.long	26364858
	.long	-2106301305
	.long	1065843399
	.long	743074255
	.long	-1172271747
	.long	1269748001
	.long	979835913
	.long	-1390944368
	.long	1315206542
	.long	1624559229
	.long	656480226
	.long	-276936178
	.long	-939645441
	.long	-142514685
	.long	-70531998
	.long	-696083641
	.long	-615669837
	.long	-906837395
	.long	-741240871
	.long	-1483212149
	.long	1565126321
	.long	-84312994
	.long	-817770883
	.long	-493574982
	.long	-1694574612
	.long	1206081346
	.long	360762385
	.long	-1088496918
	.long	-63705336
	.long	649923975
	.long	1786307672
	.long	1471776450
	.long	426139991
	.long	-987227632
	.long	595588640
	.long	324848076
	.long	1099007317
	.long	-1298339278
	.long	1480105712
	.long	588960241
	.long	108059123
	.long	2000290378
	.long	-1596559075
	.long	-1562015959
	.long	-1217670860
	.long	1439056207
	.long	-1451245119
	.long	1340050682
	.long	212907923
	.long	-435380370
	.long	-254850629
	.long	-624453698
	.long	667846605
	.long	1924964295
	.long	1643923556
	.long	522127642
	.long	-1097091747
	.long	-884207410
	.long	-982416083
	.long	1732669680
	.long	-1322561453
	.long	1259814915
	.long	295657867
	.long	1551427249
	.long	-1497360244
	.long	-550195931
	.long	-370330283
	.long	1308463029
	.long	-972805889
	.long	496192167
	.long	1495987676
	.long	1042225365
	.long	-333530683
	.long	1171624560
	.long	913673934
	.long	556397256
	.long	-920788299
	.long	-1999300946
	.long	1203766978
	.long	-1132989562
	.long	2144102362
	.long	-2019977131
	.long	-430833941
	.long	908058541
	.long	-649551238
	.text
	.align	4
	.globl	_reduce
_reduce:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$1256,%esp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	$0,-8(%ebp)	/ sign = 0
	movl	$0,-12(%ebp)	/ onemhflag = 0
	movl	12(%ebp),%eax	/ get unbiased exp of x
	movl	%eax,%ecx
	shrl	$20,%ecx
	andl	$2047,%ecx
	cmpl	$2047,%ecx
	je	.L52
	subl	$1022,%ecx	/ unbias if not inf or NaN
.L52:
	movl	%ecx,-4(%ebp)	/ k = EXPONENT(x)
				/ break mantissa of x into 15 bit chunks
	movl	%eax,%ecx
	shrl	$13,%ecx
	andl	$127,%ecx	/ hi 7 bits of mantissa
	orl	$128,%ecx	/ dont forget implied 1
	movl	%ecx,-44(%ebp)	/ F[3] = hi 8 bits
	movl	%eax,%ecx
	andl	$8191,%ecx
	leal	(,%ecx,4),%ecx	/ next 13 bits << 2
	movl	8(%ebp),%eax	/ lo word
	movl	%eax,%edx
	shrl	$30,%edx	/ hi 2 bits of low word
	andl	$3,%edx
	orl	%edx,%ecx
	movl	%ecx,-48(%ebp) 	/ F[2] = bits 30-44 
	movl	%eax,%edx
	shrl	$15,%edx
	andl	$32767,%edx
	movl	%edx,-52(%ebp)	/ F[1] = bits 29-15
	andl	$32767,%eax
	movl	%eax,-56(%ebp)	/ F[0] = bits 14-0
	movl	$8,-16(%ebp)	/ M = 8
	xorl	%ebx,%ebx	/ for(l=0;l<NUMPARTIAL;l++)
	.align	4
.L56:
	movl	$0,-632(%ebp,%ebx,4) /	partial[l] = 0;
	incl	%ebx
	cmpl	$143,%ebx
	jl	.L56
				/ first approximation to x * 1/2*pi
	movl	$1,%esi		/ for(j=1;j<=M;j++){
	movl	-4(%ebp),%eax	/ k
	addl	$92,%eax	/ k - PRECISION-L+(PADDING*CHARSIZE)
	movl	%eax,-648(%ebp)	/ tmp = k-PRECISION-L
	.align	4
.L60:
	movl	-648(%ebp),%eax	/ tmp 
	imull	$15,%esi,%edx	/ L * j
	addl	%edx,%eax	
				/ get 15 bits of 1/2pi, starting at
				/ position k-PRECISION-L+L*j
				/ there are 2 possibilites:
				/ a) all 15 bits are in 1 long,
				/ b) the 15 bits span 2 longs
	movl	$32,%ecx
	cltd	
	idivl	%ecx		/ %eax = j/32; %edx = j % 32
	
	movl	pihex(,%eax,4),%ebx / val1 = pihex[jover32]
	movl	%edx,%ecx
	shll	%cl,%ebx	/ val1 <<== jmod32
	shrl	$17,%ebx	/ val1 >>= 17
	andl	$32767,%ebx	/ val1 &= LMASK
	cmpl	$17,%edx	/ if (jmod32 <= 17)
	jle	.L163
	leal	1(%eax),%eax
	movl	pihex(,%eax,4),%eax	/ val2 = pihex[jover32 +1]
	movl	$49,%ecx
	subl	%edx,%ecx
	shrl	%cl,%eax		/ val2 >>= (64 -L -jmod32)
	andl	$32767,%eax		/ val2 &= LMASK
	addl	%eax,%ebx		/ val1 += val2
.L163:
	movl	%ebx,-32(%ebp)		/ gval = val1
	xorl	%edi,%edi	/ for(n=0;(n<N)&&(n<=j);n++){
	jmp	.L63
	.align	4
.L64:
	leal	-56(%ebp),%eax	
	leal	(,%edi,4),%edx
	addl	%edx,%eax
	movl	(%eax),%eax
	imull	-32(%ebp),%eax
	movl	%eax,-36(%ebp)	/ prod = F[n] * gval
	movl	%esi,%ebx	/ for(l=j-n;l>=0&&prod!=0;l--)
	subl	%edi,%ebx
	jmp	.L67
	.align	4
.L68:
	movl	-36(%ebp),%eax	
	addl	-632(%ebp,%ebx,4),%eax
	movl	%eax,-40(%ebp)	/ tot = prod + partial[l]
	movl	$32768,%ecx
	cltd	
	idivl	%ecx
	movl	%edx,-632(%ebp,%ebx,4)	/ partial[l] = tot % TWOL
	movl	%eax,-36(%ebp)		/ prod = tot / TWOL
	decl	%ebx
.L67:
	cmpl	$0,%ebx
	jl	.L69
	cmpl	$0,-36(%ebp)
	jne	.L68			/		}
.L69:
	incl	%edi
.L63:
	cmpl	$4,%edi
	jge	.L70
	cmpl	%esi,%edi
	jle	.L64			/	}
.L70:
	incl	%esi
	cmpl	-16(%ebp),%esi
	jle	.L60			/ }
					/ find I (quadrant)
	movl	-628(%ebp),%eax		/ tmp = ((partial[1]*FACT)/TWOL
	leal	(,%eax,4),%eax
	movl	$32768,%ecx
	cltd	
	idivl	%ecx
	movl	-632(%ebp),%edx
	leal	(,%edx,4),%edx
	addl	%edx,%eax		/ tmp += partial[0] * FACT
	movl	$4,%ecx
	cltd	
	idivl	%ecx
	movl	16(%ebp),%eax
	movl	%edx,(%eax)		/ *I = tmp % FACT

					/ determine whether to return
					/ h or 1-h
	movl	-628(%ebp),%eax
	andl	$8191,%eax
	cmpl	$4096,%eax		/ if (partial[1] & LM2MASK > TWOLM3){
	jle	.loop1
	movl	$1,-8(%ebp)		/ return 1-h - sign=1
	movl	$1,-12(%ebp)		/ onemhflag = 1 
	movl	16(%ebp),%eax
	incl	(%eax)			/ *I += 1 }
	.align	4

.loop1:
					/ loop begin - test for loss of
					/ significance and generate more
					/ bits if necessary
					/ calculate h or 1-h
	cmpl	$0,-12(%ebp)		/ if (!onemhflag)
	jne	.onemh
	fldz				/ h = 0.0
	jmp	.h
.onemh:
	fld1				/ else onemh = 1.0
.h:
	fldt	.x_four			/ factor = 4.0;
	movl	$1,%ebx			/ for(l=1;l <=M;l++)
.top:
	fldt	.twom15
	fmulp	%st,%st(1)		/ factor *= 2^-15
	fild	-632(%ebp,%ebx,4)	/ (long double)partial[l]
	fmul	%st(1),%st		/ tmp = factor * partial[l]
	fld1
	fcomp				/ if (tmp>=1.0)
	fstsw	%ax
	sahf
	ja	.less1
	fld	%st(0)
	fstcw	-1252(%ebp)		/ c truncates - set round to 0
	movl	-1252(%ebp),%eax
	orl	$0xc00,-1252(%ebp)
	fldcw	-1252(%ebp)
	frndint				/ tmp = (int)tmp
	movl	%eax,-1252(%ebp)	/ restore old control word
	fldcw	-1252(%ebp)
	fsubrp	%st,%st(1)		/ tmp = tmp - (int)tmp
.less1:
	cmpl	$0,-12(%ebp)		/ if (!onemhflag)
	jne	.onemh2
	faddp	%st,%st(2)		/ h += tmp
	jmp	.h2
.onemh2:
	fsubrp	%st,%st(2)		/ onemh -= tmp
.h2:
	incl	%ebx
	cmpl	-16(%ebp),%ebx
	jbe	.top
					/ get unbiased exponent of h == nbits
	fxch
	ffree	%st(1)


	fstpt	-644(%ebp)
	movl	-636(%ebp),%eax
	andl	$0x7fff,%eax
	subl	$16382,%eax
	negl	%eax			 / nbits = -nbits
	imull	$15,-16(%ebp),%ecx	/ if (nbits <= M*L-2*PRECISION-K)
	subl	$110,%ecx
	cmpl	%ecx,%eax
	jle	.noloss			/ no loss of significance
					/ jump out of loop
					/ else calculate more bits
	movl	-4(%ebp),%eax
	addl	$107,%eax		/ k - PRECISION+(PADDING*CHARSIZE)
	imull	$15,-16(%ebp),%edx	/ L * M
	addl	%edx,%eax		/ j
				/ get 15 bits of 1/2pi, starting at
				/ position k-PRECISION-L+L*j
				/ there are 2 possibilites:
				/ a) all 15 bits are in 1 long,
				/ b) the 15 bits span 2 longs
	movl	$32,%ecx
	cltd	
	idivl	%ecx		/ %eax = j/32; %edx = j % 32
	
	movl	pihex(,%eax,4),%ebx / val1 = pihex[jover32]
	movl	%edx,%ecx
	shll	%cl,%ebx	/ val1 <<== jmod32
	shrl	$17,%ebx	/ val1 >>= 17
	andl	$32767,%ebx	/ val1 &= LMASK
	cmpl	$17,%edx	/ if (jmod32 <= 17)
	jle	.L165
	leal	1(%eax),%eax
	movl	pihex(,%eax,4),%eax	/ val2 = pihex[jover32 +1]
	movl	$49,%ecx
	subl	%edx,%ecx
	shrl	%cl,%eax		/ val2 >>= (64 -L -jmod32)
	andl	$32767,%eax		/ val2 &= LMASK
	addl	%eax,%ebx		/ val1 += val2
.L165:
	movl	%ebx,-32(%ebp)		/ gval = val1
	incl	-16(%ebp)		/ M+= 1
	xorl	%edi,%edi		/ for(n=0;n<N;n++){
	.align	4
.L114:
	leal	-56(%ebp),%eax		/ prod = F[n] * gval;
	leal	(,%edi,4),%edx
	addl	%edx,%eax
	movl	(%eax),%eax
	imull	-32(%ebp),%eax
	movl	%eax,-36(%ebp)
	movl	-16(%ebp),%ebx		/ for(l=M-n;(l>=0)&&prod;l--){
	subl	%edi,%ebx
	jmp	.L117
	.align	4
.L118:
	movl	-36(%ebp),%eax		/ tot = prod + partial[l]
	addl	-632(%ebp,%ebx,4),%eax
	movl	%eax,-40(%ebp)
	movl	$32768,%ecx	
	cltd	
	idivl	%ecx
	movl	%edx,-632(%ebp,%ebx,4)	/ partial[l] = tot % TWOL;
	movl	%eax,-36(%ebp)		/ prod = tot / TWOL;
	decl	%ebx
.L117:
	testl	%ebx,%ebx
	jl	.L119			
	cmpl	$0,-36(%ebp)
	jne	.L118			/	}
.L119:
	incl	%edi
	cmpl	$4,%edi
	jl	.L114			/ }
	jmp	.loop1
.noloss:				/ no further loss of significance
					/ h or 1-h is in -644(%ebp)
					/ now multiply by pi/2
	fldt	-644(%ebp)
	fldt	.M_PI_2
	fmulp	%st,%st(1)
	cmpl	$0,-8(%ebp)		/ if (sign) negate result
	je	.done
	fchs
.done:
	popl	%ebx
	popl	%esi
	popl	%edi
	leave	
	ret	
	.align	4
	.text
