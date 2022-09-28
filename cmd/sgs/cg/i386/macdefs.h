/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cg:i386/macdefs.h	1.12.1.1"
/*	macdefs - machine dependent macros and parameters
 *	i386 CG
 *		Intel iAPX386
 *
 */

/* initial values for stack offsets in bits */

#define ARGINIT 	(8*SZCHAR)
#define AUTOINIT	0
#define INTARGS

/* sizes of types in bits */

#define SZCHAR		8
#define SZSHORT		16
#define SZINT		32
#define SZPOINT		32

#define SZFLOAT		32
#define SZDOUBLE	64
#define SZFPTR		64

/* alignments of types in bits */

#define ALCHAR		8
#define ALSHORT		16
#define ALINT		32
#define ALPOINT		32
#define ALSTACK		32
#define ALSTRUCT	8
#define ALINIT		32

#define ALFLOAT		32
#define ALDOUBLE	32
#define ALFPTR		32

/* Put character arrays at least 4 bytes 
** long on word boundary for performance 
*/
#define ALCARRAY
#define ALCARRAY_LIM	32

/* structure sizes must be a multiple of 32 bits */

#define STMULT 32

#define NOLONG			/* map longs to ints */

/* format for labels (if anybody cares) */

#define LABFMT ".L%d"

/* feature definitions */

#ifndef STINCC
#define	STINCC			/* enable qcc version of compiler */
#endif
#ifndef IN_LINE
#define	IN_LINE			/* enable in-line asm's */
#endif
#ifndef REGSET
#define	REGSET			/* do register set version of compiler */
#endif

/* type size in number of regs */
/* all types take 1 regs: doubles/floats 1 287 reg, others 1 CPU reg + 5 */

typedef	unsigned long RST;	/* type to hold register bit maps */

#define szty(t) (1)

/* number of scratch registers:
** 3 are integer/pointer
** 1 are 287
*/

#define NRGS 4

/* total registers
** 8 are integer/pointer (3 scratch, 3 user register, esp/ebp)
** 1 are 287		 (1 scratch)
*/

#define TOTREGS	9
#define USRREGHI 7

/* Define symbol so we can allocate odd register pairs; in fact,
** no pairs are allocated
*/

#define ODDPAIR

/* params addressed with positive offsets from fp */

#undef BACKPARAM
#undef BACKARGS

/* temps and autos addressed with negative offsets from fp */

#define BACKTEMP
#define BACKAUTO

/* bytes run right to left */

#define RTOLBYTES

/* function calls and arguments */
/* don't nest function calls (where do the args go?) */

/* 	#define NONEST			  */

/* evaluate args right to left */

#undef LTORARGS

/* chars are signed */

#define CHSIGN

/* enable register usage suggestions to optimizer */

#define IMPREGAL

/* disable alternate switch register */

#undef  IMPSWREG

/* don't pass around structures as scalars */

#define NOSIMPSTR

/* structures returned in temporary location -- define magic string */

#ifdef	OLDTMPSRET
#define TMPSRET         "\tleal\tZP,%eax\n"
#define AUXREG		REG_EAX
#else
/* TMPSRET is a dummy.  We don't want the compiler to pass the
** address of the return value automatically, because it shows
** up on the stack.  But we need a temporary intermediate register,
** and this may interfere with addressing expressions in call's
** left operand.  Leave the setup to STASG templates.
*/
#define TMPSRET         ""
#define AUXREG		REG_EAX
#endif

/* In constant folding, truncate shift count to 5 bits first. */

#define SHIFT_MASK      (0x1f)

/* enable C source in output comments */

/* comment indicator */

#define COMMENTSTR "/"

/* string containing ident assembler directive for #ident feature */

#define IDENTSTR	".ident"

/* asm markers */

#define	ASM_COMMENT	"/ASM"
#define	ASM_END		"/ASMEND"

/* protection markers: no optimizations here*/

#define PROT_START	"/ASM"
#define PROT_END	"/ASMEND"

/* volatile operand end */

#ifdef VOL_SUPPORT
#	define VOL_OPND_END	','
	extern int cur_opnd;
#       define vol_opnd_end()	{cur_opnd <<= 1;}
#endif

/* Register number for debugging */

	/* Temps int/pointer */
#define REG_EAX		0
#define REG_EDX		1
#define REG_ECX		2

	/* Temps floating point */
#define REG_FP0		3

	/* User Regs int/pointer */
#define REG_EBX		4
#define REG_ESI		5
#define REG_EDI		6

	/* Stack Pointer/Frame Pointer */
#define REG_ESP		7
#define REG_EBP		8

/* Turn on debug information */

#define	SDB

/* Enable assembly language comments */

#define	ASSYCOMMENT

/* user-level fix-up at symbol definition time */

#define FIXDEF(p)	fixdef(p)

/* support for structure debug info */

#define FIXSTRUCT(p,q)	strend(p)

/* Map register number to external register number for debugger */

#define OUTREGNO(p)     (outreg[(p)->offset])
extern int outreg[];

/* To turn on proper IEEE floating point standard comparison of non-trapping NaN's.
/* Two floating point comparisons:  CMPE for exception raising on all NaN's.
/* CMP for no exception raising for non-trapping NaN's, used for fp == and !=
*/
#define IEEE

/* All arithmetic done in extended precision */
#define FP_EXTENDED

/* expand string space in sty */

#define	NSTRING		60000

/* expand shape space in sty */

#define	NSTYSHPS	10000

/* expand template space in sty */

#define	NOPTB		1000
#define	NTMPLTS		1000

/* expand refine shape space in sty */

#define	NREFSH		1000

/* bypass initialization of INTs through templates */

#define	MYINIT	sincode

/* We supply locctr */

#define MYLOCCTR

/* We supply pickst 

#define MYPICKST        pickst
*/

/* We supply sconvert 

#define MYCONVERT
*/

#ifdef	IMPREGAL
/* Check for doubles in the tree when using IMPREGAL */

/* #define MYP2TREE	myp2tree */
#endif

/* Do local optimizations for the 386 tree (rewptr) 

#define MYP2OPTIM	myp2optim
*/

/* Do extra work for stack model compiler here 

#define MYECODE
*/

/* Our routine to move to a different register */

#define RS_MOVE         rs_move
extern void rs_move();

#define EXIT(x) myexit(x)

/* extend DIMTAB and SYMTAB size for kernel and friends */
/*
#define	DIMTABSZ	4500
#define	SYMTSZ		3000
*/

#define MYRET_TYPE(t)	myret_type(t)
/* use standard switch handling code */

#define	BCSZ		 200
#define	PARAMSZ		 300
#define TREESZ		1000
#define NTSTRBUF	 120 

extern void
	acon(),
	conput(),
	adrput(),
	upput(),
	zzzcode(),
	insput(),
	bycode(),
	fincode(),
	sincode(),
	lineid(),
	ex_before(),
	genswitch(),
	genladder(),
	bfcode(),
	efcode(),
	bfdata(),
	stasg(),
	starg(),
	protect(),
	unprot(),
	defalign(),
	defnam(),
	definfo(),
	deflab(),
	begf();

extern int
	locctr(),
	getlab();

#define	emit_str(s)	fputs(s,outfile)
#define	ELF_OBJ
#define	MYREADER	myreader
#define	BASEREG		REG_EBX

/* gotflag is a bit vector initialized to NOGOTREF in bfcode().  GOTREF
 * or GOTSWREF may be set independently of each other.
 * NOGOTREF:	no references to GOT in function
 * GOTREF:	non-switch reference to GOT
 * GOTSWREF:	reference to GOT via switch
 */

#define NOGOTREF 0
#define GOTREF	 01
#define GOTSWREF 02
extern int gotflag;

extern int picflag;
extern void myreader();

#define	WATCHDOG	200
#define FORCE_LC(lc)	(-(lc))		/*forcing location counter*/
