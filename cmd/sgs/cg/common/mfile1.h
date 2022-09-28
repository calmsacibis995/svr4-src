/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/mfile1.h	1.13"


#ifndef	MACDEFS_H			/* guard against double include */
# include "macdefs.h"
#define	MACDEFS_H
#endif
# include "manifest.h"

/*	storage classes  */
# define SNULL 0
# define AUTO 1
# define EXTERN 2
# define STATIC 3
# define REGISTER 4
# define EXTDEF 5
# define LABEL 6
# define ULABEL 7
# define MOS 8
# define PARAM 9
# define STNAME 10
# define MOU 11
# define UNAME 12
# define TYPEDEF 13
# define FORTRAN 14
# define ENAME 15
# define MOE 16
# define UFORTRAN 17
# define USTATIC 18

#ifdef IN_LINE
#define INLINE 19
#endif

	/* field size is ORed in */
# define FIELD 0100
# define FLDSIZ 077
# ifndef NODBG
extern char *scnames();
# endif

/* symbol table flags */
# define SMOS 01
/* SHIDDEN 02 and SHIDES 04 were removed.
** Their codes have been reused.
*/
# define SLABEL 02			/* symbol is label */
# define SISREG 04			/* declared as REG, but not in REG */
# define SSET 010
# define SREF 020
# define SNONUNIQ 040
# define STAG 0100

/* flags for fn.flags field */
#define	FF_ISREG	01		/* user wanted this (leaf) node to be REG */
#define	FF_ISFLD	02		/* node started as bitfield */
/* three flags available to implementations */
#define	FF_MDP1		01000
#define	FF_MDP2		02000

typedef long OFFSZ;

struct symtab {
	char *sname;
	TWORD stype;		/* type word */

	char sclass;		/* storage class */
	char slevel;		/* scope level */
	char sflags;		/* flags for set, use, hidden, mos, etc. */
	int offset;		/* offset or value */
	short dimoff;		/* offset into the dimension table */
	short sizoff;		/* offset into the size table */
	short suse;		/* line number of last use of the variable */
	unsigned short st_scopelink;
				/* index of next symbol at current scope level */
	unsigned short st_next;	/* index of next symbol on hash chain */
	unsigned short * st_own;/* pointer to owning hash chain */
};


struct sw {
	CONSZ sval;
	int slab;
};

#ifdef CG
			/*For CG: cases can contain large ranges.
			  Need a "swtab" -like array that deals
			  in ranges rather than single values.*/

struct case_range {
	CONSZ lower_bound;
	CONSZ upper_bound;
	int goto_label;
};

extern struct td td_case_range;	/* structure describing switch table */
#define case_ranges ((struct case_range *)(td_case_range.td_start))
#define RNGSZ (td_case_range.td_allo)	/* size of switch case table */

#ifndef MAXSWIT		/*Max # of entries in swtab[]*/
#define MAXSWIT 200
#endif

#endif /*CG*/


#ifndef CG

extern int sw_beg;
extern int instruct, stwart;
extern int lineno, nerrors;
typedef union {
	int intval;
	NODE * nodep;
} YYSTYPE;
extern YYSTYPE yylval;

extern CONSZ lastcon;
extern CONSZ ccast();
extern FP_DOUBLE dcon;
extern int curclass;
extern int autooff, argoff, strucoff;
extern char yytext[];
extern int brkflag;
extern int curftn;

#endif

extern int ftnno;
extern char ftitle[];
extern int strftn;
extern int curloc;
#ifdef	REGSET
extern RST regvar;			/* bit vector of current reg. vars. */
#else
extern int regvar;			/* number of register variables */
#endif
extern int nextrvar;
extern int strflg;

extern OFFSZ inoff;

#ifndef CG
/*	tunnel to buildtree for name id's */

extern int reached;
extern int idname;
extern int cflag, hflag, pflag;

/* various labels */
extern int brklab;
extern int contlab;
extern int flostat;
extern int retstat;

#endif /* not CG */

#ifdef IMPSWREG
	extern int swregno;
#endif
extern int retlab;

/*	flags used in structures/unions */

# define SEENAME 01
# define INSTRUCT 02
# define INUNION 04
# define FUNNYNAME 010
# define TAGNAME 020

/*	flags used in the (elementary) flow analysis ... */

# define FBRK 02
# define FCONT 04
# define FDEF 010
# define FLOOP 020

/*
* These defines control "while" and "for" loop code generation.
* wloop_level and floop_level each must be set to one of these values.
*/
#define LL_TOP	0	/* test at loop top */
#define LL_BOT	1	/* test at loop bottom */
#define LL_DUP	2	/* duplicate loop test at top and bottom */

#ifndef CG

#ifdef IMPREGAL
	/* for register allocation optimization */
	extern int fordepth;
	extern int whdepth;
	extern int brdepth;
	extern void rainit(), radbl(), raname(), raua(), raftn();
#endif

#endif /* CG */

/*	flags used for return status */

# define RETVAL 1
# define NRETVAL 2

/*	used to mark a constant with no name field */

# define NONAME 040000

	/* mark an offset which is undefined */

# define NOOFFSET (-10201)

/*	declarations of various functions */

#ifndef CG
extern NODE
	*aadjust(),
	*bcon(),
	*bdty(),
	*block(),
	*bpsize(),
	*buildtree(),
#ifdef IMPREGAL
	*myand(),
#endif
#ifdef	IMPSWREG
	*setswreg(),
#endif
	*conval(),
	*convert(),
	*dclstruct(),
	*doszof(),
	*getstr(),
	*makety(),
	*mkty(),
	*oconvert(),
	*offcon(),
	*optim(),
	*pconvert(),
	*ptmatch(),
	*pvconvert(),
	*rstruct(),
	*sconvert(),
	*strargs(),
	*stref(),
	*treecpy(),
	*tymatch(),
	*tymerge(),
	*unconvert(),
	*xicolon();

OFFSZ	tsize(),
	psize();

TWORD	types(),
	ctype();

char *exdcon();

#endif /* not CG */

char *exname(); 
extern void rbusy();
extern NODE * clocal();

# define checkst(x) 		/* turn off symbol table checking */

# ifdef SDB
# include "sdb.h"
# endif

/* type that is equivalent to pointers in size */
# ifndef PTRTYPE
# define PTRTYPE INT
# endif

/* size of hash bucket table */
#ifndef HASHTSZ
#define HASHTSZ 511
#endif

#ifndef CG
extern struct td td_stab;	/* structure describing symbol table */
#define SYMTSZ	(td_stab.td_allo)	/* size is now dynamic */
#define stab	((struct symtab *)(td_stab.td_start))
				/* address is now dynamic */

extern struct td td_dimtab;	/* structure describing dimension table */
#define DIMTABSZ (td_dimtab.td_allo)
#define dimtab ((int *) (td_dimtab.td_start))
#define curdim (td_dimtab.td_used)

extern struct td td_paramstk;	/* structure describing parameter stack */
#define PARAMSZ (td_paramstk.td_allo)
#define paramstk ((int *) (td_paramstk.td_start))
#define paramno (td_paramstk.td_used)

extern struct td td_argstk;	/* structure describing symtab indices of args */
#define ARGSZ (td_argstk.td_allo)
#define argstk ((int *)(td_argstk.td_start))
#define argno (td_argstk.td_used)

extern struct td td_argsoff;	/* structure describing offsets of arguments */
#define argsoff ((int *)(td_argsoff.td_start))

extern struct td td_argty;	/* structure describing types of arguments */
#define argty ((TWORD *)(td_argty.td_start))

extern struct td td_asavbc;	/* structure describing block save area */
#define BCSZ (td_asavbc.td_allo)
#define asavbc ((int *) (td_asavbc.td_start))
#define psavbc (td_asavbc.td_used)

extern struct td td_scopestack;	/* structure describing scope links for symtab */
#define MAXNEST (td_scopestack.td_allo)
#define scopestack ((int *)(td_scopestack.td_start))
#define blevel (td_scopestack.td_used)

#endif /* not CG */

extern struct td td_swtab;	/* structure describing switch table */
#define swtab ((struct sw *)(td_swtab.td_start))
#define CSWITSZ (td_swtab.td_allo)	/* size of switch case table */
#define swidx (td_swtab.td_used)	/* number used so far */

#ifdef	MAKEHEAP
#undef SWITSZ
extern void makeheap();
extern struct td td_heapsw;
#define heapsw ((struct sw *)(td_heapsw.td_start))
#define HSWITSZ (td_heapsw.td_allo)
#endif
