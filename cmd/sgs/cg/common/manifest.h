/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/manifest.h	1.34"

/*To allow multiple includes:*/
#ifndef MANIFEST_H
#define	MANIFEST_H

#ifndef EOF
# include <stdio.h>
#endif

#ifndef PUTCHAR
#define PUTCHAR(c) ((void)(putc((c),outfile)))
#endif
#ifndef	PUTS
#define	PUTS(s) ((void) fputs((s), outfile))
#endif

	/* macro definitions for common cases of type collapsing */
# ifdef NOSHORT
# define SZSHORT SZINT
# define ALSHORT ALINT
# endif

# ifdef NOLONG
# define SZLONG SZINT
# define ALLONG ALINT
# endif

# ifdef NOFLOAT
# define SZFLOAT SZLONG
# define SZDOUBLE SZLONG
# define ALFLOAT ALLONG
# define ALDOUBLE ALLONG
# endif

# ifdef ONEFLOAT
# define SZFLOAT SZDOUBLE
# define ALFLOAT ALDOUBLE
# endif

/* define default assembly language comment starter */

# ifndef COMMENTSTR
# define COMMENTSTR	"#"
#endif

/*	manifest constant file for the lex/yacc interface */

# define ERROR 1
# define NAME 2
# define STRING 3
# define ICON 4
# define FCON 5
# define PLUS 6
# define MINUS 8
# define MUL 11
# define STAR (UNARY MUL)
# define AND 14
# define OR 17
# define ER 19
# define QUEST 21
# define COLON 22
# define ANDAND 23
# define OROR 24

/*	special interfaces for yacc alone */
/*	These serve as abbreviations of 2 or more ops:
	ASOP	=, = ops
	RELOP	LE,LT,GE,GT
	EQUOP	EQ,NE
	DIVOP	DIV,MOD
	SHIFTOP	LS,RS
	ICOP	INCR,DECR
	UNOP	NOT,COMPL
	STROP	DOT,STREF

	*/
# define ASOP 25
# define RELOP 26
# define EQUOP 27
# define DIVOP 28
# define SHIFTOP 29
# define INCOP 30
# define UNOP 31
# define STROP 32

/*	reserved words, etc */
# define TYPE 33
# define CLASS 34
# define STRUCT 35
# define RETURN 36
# define GOTO 37
# define IF 38
# define ELSE 39
# define SWITCH 40
# define BREAK 41
# define CONTINUE 42
# define WHILE 43
# define DO 44
# define FOR 45
# define DEFAULT 46
# define CASE 47
# define SIZEOF 48
# define ENUM 49

/*	little symbols, etc. */
/*	namely,

	LP	(
	RP	)

	LC	{
	RC	}

	LB	[
	RB	]

	CM	,
	SM	;

	*/

# define LP 50
# define RP 51
# define LC 52
# define RC 53
# define LB 54
# define RB 55
# define CM 56
# define SM 57
# define ASSIGN 58
	/* ASM returned only by yylex, and totally eaten by yyparse */
# define ASM 59

/*	END OF YACC */

/*	left over tree building operators */
# define COMOP 59
# define DIV 60
# define MOD 62
# define LS 64
# define RS 66
# define DOT 68
# define STREF 69
# define CALL 70
# define FORTCALL 73
# define NOT 76
# define COMPL 77
# define INCR 78
# define DECR 79
# define EQ 80
# define NE 81
# define LE 82
# define LT 83
# define GE 84
# define GT 85
# define ULE 86
# define ULT 87
# define UGE 88
# define UGT 89
/* # define SETBIT 90 */
/* # define TESTBIT 91 */
# define ARS 92
/* ASG ARS is 93 */
# define REG 94
# define TEMP 95
# define CCODES 96
# define FREE 97
# define STASG 98
# define STARG 99
# define STCALL 100
/* # define RESETBIT 101 */
/* UNARY STCALL is 102 */

/*	some conversion operators */
# define FLD 103
# define CONV 104
# define PMUL 105
# define PDIV 106

/*	special node operators, used for special contexts */
/* # define FORCE 107 */
# define GENLAB 108
# define CBRANCH 109
# define GENBR 110
# define CMP 111
# define CMPE 112	/* if IEEE standard, used for exception raising fp cmps */
# define GENUBR 113
# define INIT 114
# define CAST 115
# define FUNARG 116
# define VAUTO 117
# define VPARAM 118
# define RNODE 119
# define SNODE 120
# define QNODE 121
/*	a whole bunch of ops, done with unary; I don't need to tackle prec */
# define UOP0  122
# define UOP1  123
# define UOP2  124
# define UOP3  125
# define UOP4  126
# define UOP5  127
# define UOP6  128
# define UOP7  129
# define UOP8  130
# define UOP9  131
#ifdef	IN_LINE
# define INCALL 132			/* beware of UNARY INCALL == 134! */
#endif
# define MANY  133
/*	UNARY INCALL 134 */

/* ops used for NAIL*/
#ifdef CG
# define SEMI	135
# define ENTRY	136
# define PROLOG	137
# define ENDF	138
# define LOCCTR	139
# define LXINFO	140
# define SWBEG	141
# define SWCASE	142
# define SWEND	143
# define ALLOC	144
# define DEFNAM	145
# define UNINIT	146
# define CURFRAME 147
# define FCHAIN	148
# define BMOVE	150
# define BMOVEO	151
# define EXSETV	152
# define EXGETV	153
# define EXCLEAR 154
# define EXRAISE 155
# define JUMP	156
# define SINIT	157
# define LET	158
# define CSE	159
# define CURCAP	160
# define ALIGN	162
# define NPRETURN 163
# define VLRETURN 164
# define FSELECT 165
# define FCONV	166
# define BCMP	167
# define EXTEST	168
# define COPYASM 169
# define NOP	170
# define VFLD	171
# define COPY	172
# define BEGF	173
# define CAPCALL 174
# define RSAVE	175
# define RREST	176
# define CAPRET 177
# define LABELOP 178
# define UPLUS	179	/* ANSI C unary + */
# define NAMEINFO 180
#endif	/* def CG */

	/* DSIZE is the size of the dope array:  highest OP # + 1 */
#ifdef	CG
#define DSIZE  NAMEINFO+1
#else
# define DSIZE MANY+1+1			/* room for UNARY INCALL */
#endif

/*	node types */
# define LTYPE 02
# define UTYPE 04
# define BITYPE 010


/*	type names, used in symbol table building */
# define TNULL 0
# define FARG 1
# define CHAR 2
# define SHORT 3
# define INT 4
# define LONG 5
# define FLOAT 6
# define DOUBLE 7
# define STRTY 8
# define UNIONTY 9
# define ENUMTY 10
# define MOETY 11
# define UCHAR 12
# define USHORT 13
# define UNSIGNED 14
# define ULONG 15
# define VOID 16
# define UNDEF 17

# define ASG 1+
# define UNARY 2+
# define NOASG (-1)+
# define NOUNARY (-2)+

/*	various flags */
# define NOLAB (-1)

/* type modifiers */

# define PTR  040
# define FTN  0100
# define ARY  0140

/* type packing constants */

# define MTMASK 03
# define BTMASK 037
# define BTSHIFT 5 
# define TSHIFT 2
# define TMASK (MTMASK<<BTSHIFT)
# define TMASK1 (MTMASK<<(BTSHIFT+TSHIFT))
# define TMASK2  (TMASK||MTMASK)

/*	macros	*/

# ifndef BITMASK
	/* beware 1's complement */
# define BITMASK(n) (((n)==SZLONG)?-1L:((1L<<(n))-1))
# endif
# define ONEBIT(n) (1L<<(n))
# define MODTYPE(x,y) x = (x&(~BTMASK))|y  /* set basic type of x to y */
# define BTYPE(x)  (x&BTMASK)   /* basic type of x */
# define ISUNSIGNED(x) ((x)<=ULONG&&(x)>=UCHAR)
# define UNSIGNABLE(x) ((x)<=LONG&&(x)>=CHAR)
# define ENUNSIGN(x) ((x)+(UNSIGNED-INT))
# define DEUNSIGN(x) ((x)+(INT-UNSIGNED))
# define ISPTR(x) ((x&TMASK)==PTR)
# define ISFTN(x)  ((x&TMASK)==FTN)  /* is x a function type */
# define ISARY(x)   ((x&TMASK)==ARY)   /* is x an array type */
# define INCREF(x) (((x&~BTMASK)<<TSHIFT)|PTR|(x&BTMASK))
# define DECREF(x) (((x>>TSHIFT)&~BTMASK)|(x&BTMASK))
# define SETOFF(x,y)   if( (x)%(y) != 0 ) x = ( ((x)/(y) + 1) * (y))
		/* advance x to a multiple of y */
# define NOFIT(x,y,z)   ( ((x)%(z) + (y)) > (z) )
	/* can y bits be added to x without overflowing z */
	/* pack and unpack field descriptors (size and offset) */
# define PKFIELD(s,o) (((o)<<6)|(s))
# define UPKFSZ(v)  ((v)&077)
# define UPKFOFF(v) ((v)>>6)

/* Definitions for floating point emulation.  The current version
** only works on an Amdahl host.  This stuff appears in macdefs.h
** in QCC/RCC compilers, so make this version conditional on CG.
*/

#if defined(CG) && defined(uts) && defined(FP_EMULATE)

#define	FP_FLOAT long			/* prevent unfortunate conversions */
#define FP_CMPD(d1,d2)	flcmp(d1,d2)
#define FP_DTOFP(d)	dtofp(d)
#define	FP_DTOF(d)	dtof(d)
#define FP_DTOL(d)	dtol(d)
#define FP_DTOUL(d)	dtoul(d)
#define FP_LTOD(d)	ltod(d)
#define FP_ULTOD(d)	ultod(d)
#define FP_PLUS(d1,d2)	fladd(d1,d2)
#define FP_MINUS(d1,d2) fladd(d1,-d2)
#define FP_TIMES(d1,d2) flmul(d1,d2)
#define FP_DIVIDE(d1,d2) fldiv(d1,d2)
#define	FP_ISZERO(d)	fp_iszero(d)

extern double ltod(), ultod(), fladd(), flmul(), fldiv(), dtofp();
extern FP_FLOAT dtof();
extern long dtol();
extern int fp_iszero();

#define FP_ATOF(s)	atof2(s)
extern double atof2();
#endif /* uts && FP_EMULATE */

/* Default definitions in the absence of emulation. */

extern int errno;
#ifndef	FP_DOUBLE
#define	FP_DOUBLE	double	/* type containing doubles */
#endif
#ifndef	FP_FLOAT
#define	FP_FLOAT	float	/* type containing floats */
#endif
#ifndef	FP_DTOF
#define	FP_DTOF(d) ((float) d)	/* convert double to float */
#endif
#ifndef	FP_DTOFP		/* truncate double to float precision */
#define	FP_DTOFP(d) ((double) FP_DTOF(d))
#endif
#ifndef	FP_DTOL			/* convert double to long */
#define	FP_DTOL(d) ((long) (d))
#endif
#ifndef FP_DTOUL		/* convert double to unsigned long */
#define FP_DTOUL(d) ((unsigned long) (d))
#endif
#ifndef	FP_LTOD			/* convert long to double */
#define	FP_LTOD(l) ((double) (l))
#endif
#ifndef	FP_ULTOD		/* convert unsigned long to double */
#define	FP_ULTOD(ul) ((double) (ul))
#endif
#ifndef	FP_NEG			/* negate double */
#define	FP_NEG(d) (-(d))
#endif
#ifndef	FP_PLUS			/* add double */
#define	FP_PLUS(d1,d2) ((d1)+(d2))
#endif
#ifndef	FP_MINUS		/* subtract double */
#define	FP_MINUS(d1,d2) ((d1)-(d2))
#endif
#ifndef	FP_TIMES		/* multiply double */
#define	FP_TIMES(d1,d2) ((d1)*(d2))
#endif
#ifndef	FP_DIVIDE		/* divide double */
#define	FP_DIVIDE(d1,d2) ((d1)/(d2))
#endif
#ifndef	FP_ISZERO		/* is double value zero? */
#define	FP_ISZERO(d) (!(d))
#endif
#ifndef	FP_CMPD			/* compare two doubles */
#define	FP_CMPD(x,y) ((x)==(y)?0:((x)>(y)?1:-1))
#endif
#ifndef	FP_ATOF				/* convert string to FP_DOUBLE */
#ifndef	FLOATCVT			/* backward compatibility */
#define	FP_ATOF(s) atof(s)		/* use UNIX atof() */
			/*Note: for CG, we also need a C++ declaration
			  of atof(); */
#  ifdef c_plusplus
extern double atof(const char *);
#  else
extern double atof();
#  endif
#else
#define	FP_ATOF(s) FLOATCVT(s)
#endif
#endif

/*	operator information */

# define TYFLG 016
# define ASGFLG 01
# define LOGFLG 020

# define SIMPFLG 040
# define COMMFLG 0100
# define DIVFLG 0200
# define FLOFLG 0400
# define LTYFLG 01000
# define CALLFLG 02000
# define MULFLG 04000
# define SHFFLG 010000
# define ASGOPFLG 020000

# define SPFLG 040000
# define STRFLG 0100000		/* for structure ops */
# define AMBFLG	0200000		/* ops that cause ambiguity */

#ifdef	REGSET
#	define RS_BIT(n) (((RST) 1) << (n)) /* bit corresponding to reg n */
#	define RS_PAIR(n) ((RST)(n) << 1) /* reg n's pair companion */
/* Choose a register from a register set bit vector.  Used to pick a
** scratch register.  Always choose right-most bit.
*/
#	define RS_CHOOSE(vec) ((vec) & ~((vec)-1))
/* Choose left-most bit.  This one's a function. */
#ifndef CG
extern RST RS_RCHOOSE();
#endif

#	define RS_NONE	((RST) 0)	/* no register bits */
/* these definitions are slight perversions of the use of RS_BIT */
#	define RS_NRGS	(RS_BIT(NRGS)-1) /* bits for all scratch registers */
#	define RS_TOT	(RS_BIT(TOTREGS)-1) /* bits for all registers */
#endif

#define optype(o) (dope[o]&TYFLG)
#define asgop(o) (dope[o]&ASGFLG)
#define asgbinop(o) (dope[o]&ASGOPFLG)
#define logop(o) (dope[o]&LOGFLG)
#define mulop(o) (dope[o]&MULFLG)
#define shfop(o) (dope[o]&SHFFLG)
#define callop(o) (dope[o]&CALLFLG)
#define structop(o) (dope[o]&STRFLG)
#define ambop(o) (dope[o]&AMBFLG)

#ifdef	CG
/*	for CG: two basic exceptions*/
#define NUMERIC 1
#define STACKOV 2
/* 	numeric exception types*/
#define EXLSHIFT	01	/*dropped bit on ls*/
#define EXCAST		02	/*dropped bit on convert*/
#define EXINT		04	/*integer overflow*/
#define EXFLOAT		010	/*floating overflow*/
#define EXDBY0		020	/*div by zero*/
#endif

/*	table sizes	*/

/* The following undef-initions will help flag places where
** older machine-dependent code depends on fixed size tables.
*/
#undef BCSZ
#undef MAXNEST
#undef SYMTSZ
#undef DIMTABSZ
#undef PARAMSZ
#undef ARGSZ
#undef TREESZ
/* keep this one for non-users of MAKEHEAP */
# ifndef SWITSZ
# define SWITSZ 250 /* size of switch table */
# endif
# ifndef YYMAXDEPTH
# define YYMAXDEPTH 300 /* yacc parse stack */
# endif

/* The following define initial sizes for dynamic tables. */

#ifndef INI_NINS		/* inst[]:  table of generated instructions */
#define INI_NINS 50
#endif

#ifndef	INI_SWITSZ		/* swtab[]:  switch case table */
#define	INI_SWITSZ 100
#endif

#ifndef	INI_HSWITSZ		/* heapsw[]:  heap-sorted switch table */
#define	INI_HSWITSZ 25
#endif

#ifndef INI_BBFSZ		/* bb_flags[]:  .bb debug flag table */
#define INI_BBFSZ 10
#endif

#ifndef INI_FAKENM		/* mystrtab[]:  unnamed struct fake names */
#define INI_FAKENM 15
#endif

#ifndef	INI_N_MAC_ARGS		/* macarg_tab[]:  enhanced asm arg. names */
#define INI_N_MAC_ARGS 5
#endif

#ifndef	INI_SZINLARGS		/* inlargs[]:  buffer for enhanced asm. formals */
#define	INI_SZINLARGS 50
#endif

#ifndef INI_ASMBUF		/* asmbuf[]:  buffer for asm() lines */
#define INI_ASMBUF 20
#endif

#ifndef	INI_SYMTSZ		/* stab[]:  symbol table */
#define	INI_SYMTSZ 500
#endif

#ifndef INI_RNGSZ		/*case_ranges[]: ranges for big cases (CG) */
#define INI_RNGSZ INI_SWITSZ
#endif

/* dimtab must be at least 16:  see mainp1() */
#if !defined(INI_DIMTABSZ) || INI_DIMTABSZ < 20
#undef INI_DIMTABSZ		/* dimtab[]:  dimension (and misc.) table */
#define INI_DIMTABSZ 400
#endif

#ifndef	INI_PARAMSZ		/* paramstk[]:  struct parameter stack */
#define INI_PARAMSZ 100
#endif

#ifndef	INI_BCSZ		/* asavbc[]:  block information */
#define INI_BCSZ 50
#endif

/* scopestack should be at least 3 (indices 0-2) to handle simple fct. */
#if !defined(INI_MAXNEST) || INI_MAXNEST < 3
#undef INI_MAXNEST		/* scopestack[]:  sym. tab. scope stack */
#define INI_MAXNEST 20
#endif

#ifndef INI_ARGSZ		/* argstk[], argsoff[], argty[]:  incoming
				** arg information
				*/
#define INI_ARGSZ 15
#endif

#ifndef INI_INSTK		/* instack[]:  initialization stack */
#define INI_INSTK 10
#endif

#ifndef	INI_TREESZ		/* number of nodes per cluster */
#define	INI_TREESZ 100
#endif

#ifndef	INI_MAXHASH		/* number of segmented scanner hash tables */
#define	INI_MAXHASH 20
#endif

#ifndef	INI_NTSTRBUF		/* number of temp string buffers */
#define	INI_NTSTRBUF 20
#endif


#ifdef	CG
/*	turn on NONEST if can't nest calls or allocs */
#  if defined(NOFNEST) || defined(NOANEST)
#    define NONEST
#  endif
#endif	/* def CG */
#ifndef CG
	char		*hash();
	char		*savestr();
	void		freestr();
#endif
	char		*tstr();
	extern FILE *	outfile;

#	define NCHNAM 8  /* number of characters in a truncated name */

/*	common defined variables */

extern int nerrors;  /* number of errors seen so far */

typedef unsigned long BITOFF;
typedef unsigned int TWORD;
typedef long CONSZ;  /* size in which constants are converted */
typedef long OFFSET; /* offset of memory addresses */

	/* default is byte addressing */
	/* BITOOR(x) converts bit width x into address offset */
# ifndef BITOOR
# define BITOOR(x) ((x)/SZCHAR)
# endif

#define OKTYPE  ( TINT | TUNSIGNED | TLONG | TULONG | TPOINT | \
                  TPOINT2 | TSHORT | TUSHORT | TCHAR | TUCHAR)

/* Many architectures can't handle literal double/float constants.
** For those that do, define LITDCON to return 1 in situations where
** a literal is permitted.  p is the FCON node.  Otherwise return 0,
** and a named constant will be defined.
*/

#ifndef	LITDCON
#define	LITDCON(p)	(0)
#endif

# ifdef SDB
# define STABS
# endif

#if defined(TMPSRET) && !defined(AUXREG)
#define AUXREG (NRGS - 1)
#endif

# define NIL (NODE *)0

extern long dope[];  /* a vector containing operator information */
extern char *opst[];  /* a vector containing names for ops */

# define NCOSTS (NRGS+4)

/* The idea of all of the machinations below for nodes is to create
** compatible definitions of NODE, ND1, and ND2.  The existing code
** that deals with NODEs must continue to do so, and yet the code
** for ND1s and ND2s must be self-consistent as well.  Moreover,
** it's important that the proper corresponding fields be accessed
** with the appropriate names.
*/

/* This #define gives those items that must be in both first
** and second pass nodes.
*/

#define ND12(name) \
    int op;		/* opcode */		\
    TWORD type;		/* type encoding */	\
    name * left;	/* left operand */	\
    name * right;/* right operand */	\
    CONSZ lval;		/* constant value */	\
    int rval;		/* usually stab entry */ \
    FP_DOUBLE dval	/* double constant */


/* Pass 1 node */
#define DEF_P1NODE(sname,pname) struct sname {		\
    ND12(pname);	/* the above, and ... */	\
    int flags;		/* special pass 1 flags */	\
    int cdim;		/* dimoff */			\
    int csiz;		/* sizoff */			\
    TWORD sttype;	/* actual struct/union type for s/u ops */ \
    char * string;	/* ptr. to STRING string */	\
};

/* Pass 2 node */
#define DEF_P2NODE(sname,pname) struct sname {		\
    ND12(pname);	/* the basics, plus ... */	\
							\
    int goal;		/* code generation goal */	\
    int label;		/* branch target label # */	\
    int lop;		/* GENBR branch condition */	\
    char * name;	/* pointer to name string */	\
    int stsize;		/* structure size */		\
    short stalign;	/* structure alignment */	\
    unsigned short argsize;	/* size of CALL argument list */ \
    ND_VAR		/* variable information, depending... */ \
};

/* This ugly chain of #if/#else/#endif is necessitated by the
** Amdahl's not understanding #elif.
*/

#if !defined(CG) && !defined(STINCC)
/* PCC2 case */
#  define ND_VAR \
    int cst[NCOSTS];	/* PCC2 costs */
#else
#  if  defined(STINCC) && !defined(REGSET) && !defined(CG)
/* QCC case */
#  define ND_VAR \
    /* empty */
#  else
#    if defined(STINCC) && defined(REGSET) && !defined(CG)
/* RCC case */
#  define ND_VAR \
    int scratch;	/* index to scratch register vectors */
#    else
#      if defined(CG)
/* CG case */
#  define ND_VAR \
    int scratch;	/* index to scratch register vectors */ \
    int strat;		/* code generation strategy */ \
    int id;		/* name of CSE destination */
#      else
#  include "don't know compiler type"
#endif /* CG case */
#endif /* RCC case */
#endif /* QCC case */
#endif /* PCC2 case */

typedef union ndu NODE;		/* old-style node */
typedef struct nND1 ND1;	/* new first pass node */
typedef struct nND2 ND2;	/* new second pass node */

/* Define several different flavors of structures that
** depend on the type of the pointer contained within.
*/
DEF_P1NODE(nND1,ND1)
DEF_P2NODE(nND2,ND2)
DEF_P1NODE(oND1,NODE)
DEF_P2NODE(oND2,NODE)

union ndu {
    struct oND1 fn;	/* front end node */
    struct oND1 fpn;	/* floating point node */

    struct oND2 in;	/* interior (binary) node */
    struct oND2 tn;	/* terminal (leaf) node */
    struct oND2 bn;	/* branch node */
    struct oND2 stn;	/* structure node */
#ifdef CG
    struct oND2 csn;	/* CSE node */
#endif
};



typedef struct nndu {
    NODE node;
    int _node_no;
} nNODE;			/* numbered node */

#define node_no(p) /* NODE * p */ (((nNODE *) (p))->_node_no)


#ifdef	CG
/* For CG, file pointers for output and debugging*/
extern FILE *textfile;		/* user-requested output file.
				** Normally same as outfile.
				*/
extern FILE *debugfile; 	/*output file for debugging*/
extern char 	costing;	/*1 if just costing*/
extern int strftn, proflag;
extern int str_spot;		/*place to hold struct return*/
NODE	*firstl(), *lastl();	/*CG functions*/
#endif	/* def CG */

/* Location counters.  Formerly in mfile1.h, moved here for nail's
** benefit.
*/
# define PROG 0
# define ADATA 1
# define DATA 2
# define ISTRNG 3
# define STRNG 4
#ifdef	CG
# define CDATA 5	/* constants */
# define CSTRNG 6	/* constant strings */
# define CURRENT 7	/* Return current locctr*/
# define UNK 8		/* Unknown (inital) state */
#endif

/* Guarantee that const and myVOID are defined.  myVOID used for pointers. */
#ifndef	__STDC__
# ifndef const
# define const				/* this vanishes in old-style C */
# endif
typedef char myVOID;
#else
typedef void myVOID;
#endif

/* Define table descriptor structure for dynamically
** managed tables.
*/

struct td {
    int td_allo;		/* array elements allocated */
    int td_used;		/* array elements in use */
    int td_size;		/* sizeof(array element) */
    int td_flags;		/* flags word */
    myVOID * td_start;		/* (really void *) pointer to array start */
    myVOID * td_name;		/* table name for error */
#ifdef STATSOUT
    int td_max;			/* maximum value reached */
#endif
};

/* flags for td_flags */
#define TD_MALLOC 	1	/* array was malloc'ed */
#define TD_ZERO		2	/* zero expansion area */

int td_enlarge();		/* enlarges a table so described, returns
				** old size
				*/

#define TD_INIT(tab, allo, size, flags, start, name) \
struct td tab = { \
	allo,	/* allocation */ \
	0,	/* used always 0 */ \
	size,	/* entry size */ \
	flags,	/* flags */ \
	(myVOID *)start, /* pointer */ \
	name	/* table name */ \
}

#ifdef	OPTIM_SUPPORT

/* Definitions for HALO optimizer interface. */

/* "Storage classes" */
#define	OI_AUTO		1
#define	OI_PARAM	2
#define	OI_EXTERN	3
#define	OI_EXTDEF	4
#define	OI_STATEXT	5
#define	OI_STATLOC	6

/* Loop codes */
#define	OI_LSTART	10
#define	OI_LBODY	11
#define	OI_LCOND	12
#define	OI_LEND		13

/* Routines: */
extern char * oi_loop();
extern char * oi_symbol();
extern char * oi_alias();

#endif	/* def OPTIM_SUPPORT */

#ifdef VOL_SUPPORT
	extern void	vol_instr_end();
#endif

extern NODE * talloc();

#ifdef CG

/* C++ compatible declarations */
#ifdef c_plusplus

extern OFFSET next_temp(TWORD, BITOFF, int);
extern OFFSET next_arg(TWORD, BITOFF, int);
extern OFFSET off_conv(int, OFFSET, TWORD, TWORD);
extern OFFSET off_bigger(int, OFFSET, OFFSET);
extern OFFSET off_incr(int, OFFSET, long);
extern OFFSET max_temp(void), max_arg(void);
extern int    
	p2done(void),
	cisreg(TWORD, char*);
	rgsave(char*),
	tyreg(TWORD),
	gtalgin(TWORD),
	gtsize(TWORD),
	tchech(void),
	off_is_err(int, OFFSET);

extern void   
	p2init(void),
	p2abort(void),
	p2compile(NODE *),
	p2flags(char *),
        bind_param(TWORD, TWORD, int, OFFSET, int *, char *),
	tfree(NODE *),
	ret_type(TWORD),
	ofile(FILE *),
	profile(int),
	tshow(void),
	set_next_temp(OFFSET),
	set_next_arg(OFFSET),
	off_init(int);

#else

extern OFFSET next_temp();
extern OFFSET next_arg();
extern OFFSET off_conv();
extern OFFSET off_bigger();
extern OFFSET off_incr();
extern OFFSET max_temp(), max_arg();
extern int
	cisreg(),
	tcheck(),
	p2done(),
	rgsave(),
	tyreg(),
	gtalign(),
	gtsize(),
	off_is_err();

extern void
	p2compile(),
	p2init(),
	p2abort(),
	p2flags(),
	bind_param(),
	ret_type(),
	ofile(),
	profile(),
	set_next_temp(),
	set_next_arg(),
	off_init(),
	tshow(),
	tfree();
#endif

#endif  /* CG */

#ifdef	IN_LINE
/* Declarations for enhanced asm support. */
#if defined(CG) && defined(__STDC__)
extern void as_gencode(ND2 *, FILE *);
extern void as_start(char *);
extern void as_param(char *);
extern void as_e_param(void);
extern void as_putc(int);
extern void as_end(void);
#else	/* no prototypes */
extern void as_gencode();
extern void as_start();
extern void as_param();
extern void as_e_param();
extern void as_putc();
extern void as_end();
#endif
#endif	/* def IN_LINE */

/* Error reporting routines. */
#if defined(CG) && defined(__STDC__)
extern void uerror(const char *, ...);
extern void werror(const char *, ...);
extern void cerror(const char *, ...);
#else
extern void uerror();
extern void werror();
extern void cerror();
#endif	/* def __STDC__ */

#endif	/* def MANIFEST_H:  from top of file */
