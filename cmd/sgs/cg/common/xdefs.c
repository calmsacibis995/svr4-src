/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/xdefs.c	1.6"

# include "mfile1.h"
# include "mfile2.h"
# include <memory.h>		/* for memcpy() */
# include <malloc.h>

/*	communication between lexical routines	*/

int	lineno;		/* line number of the input file */

FILE * outfile = stdout;	/* place to write all output */
#ifdef	CG
FILE * textfile = stdout;	/* user-requested output file */
#endif

#ifndef CG
CONSZ lastcon;  /* the last constant read by the lexical analyzer */
FP_DOUBLE dcon;   /* the last double read by the lexical analyzer */
#endif

/*	symbol table maintainence */

int	curftn;  /* "current" function */
int	strftn;  /* 1 if current function returns struct or union */
int	ftnno;  /* "current" function number */
int	curloc;		  /* current location counter value */

#ifndef	CG			/* CG has no symbol table */

/* initial statically allocated table */
/* static */ struct symtab st_init[INI_SYMTSZ];

/* symbol table */
TD_INIT( td_stab, INI_SYMTSZ, sizeof(struct symtab),
		(TNULL == 0 ? TD_ZERO : 0), st_init, "symbol table");

int	curclass,	  /* current storage class */
instruct,	/* "in structure" flag */
stwart;		/* for accessing names which are structure members or names */

/* initial, statically allocated table */
/* static */ int dim_init[INI_DIMTABSZ];

/* dimension table */
TD_INIT( td_dimtab, INI_DIMTABSZ, sizeof(int), 0,  dim_init, "dimtab");

/* initial, statically allocated table */
/* static */ int param_init[INI_PARAMSZ];

/* parameter stack */
TD_INIT( td_paramstk, INI_PARAMSZ, sizeof(int), 0,  param_init, "parameter stack");

/* These three tables related to function arguments (incoming),
** and they are expanded at the same time.  They must start out
** with, and continue to have, the same size.
*/

/* static */ int astk_init[INI_ARGSZ];
/* static */ int aoff_init[INI_ARGSZ];
/* static */ TWORD aty_init[INI_ARGSZ];

/* argument symbol table offsets */
TD_INIT( td_argstk, INI_ARGSZ, sizeof(int),
		0,  astk_init, "arg. symtab offset table");
/* argument offsets */
TD_INIT( td_argsoff, INI_ARGSZ, sizeof(int), 0, aoff_init, "argument offset table");
/* argument types */
TD_INIT( td_argty, INI_ARGSZ, sizeof(TWORD), 0, aty_init, "argument type table");

int	autooff,	/* the next unused automatic offset */
argoff,	/* the next unused argument offset */
strucoff;	/*  the next structure offset position */
#endif	/* def CG */

#ifdef	REGSET
RST	regvar;		/* currently busy register variable bitmap */
#else
int	regvar;		/* the next free register for register variables */
#endif
int	nextrvar;	/* the next allocated reg (set by cisreg) */
OFFSZ	inoff;		/* offset of external element being initialized */
#ifndef CG
int	brkflag = 0;	/* complain about break statements not reached */
int 	sw_beg;  	/* index of beginning of cases for current switch */
int 	reached;	/* true if statement can be reached... */
int 	idname;		/* tunnel to buildtree for name id's */
int 	brklab;
int 	contlab;
int 	flostat;
int 	retstat;
int cflag = 0;  /* do we check for funny casts */
int hflag = 0;  /* do we check for various heuristics which may indicate errors */
int pflag = 0;  /* do we check for portable constructions */
#endif  /* not CG */

/* tables related to switches */
static struct sw swtab_init[INI_SWITSZ];

/* switch table */
TD_INIT( td_swtab, INI_SWITSZ, sizeof(struct sw), 0, swtab_init, "switch table");

/* debugging flag */
int xdebug = 0;
int idebug = 0;

int strflg;  /* if on, strings are to be treated as lists */

#ifdef IMPSWREG
	int swregno;
#endif
int retlab = NOLAB;

/* save array for break, continue labels, and flostat */

/* beware!!  If REGSET turned on, register vector bitmaps get
** stored here.  An "int" had better be large enough!
*/

#ifndef CG
/* static */ int asavbc_init[INI_BCSZ];

/* block info stack */
TD_INIT( td_asavbc, INI_BCSZ, sizeof(int), 0, asavbc_init, "block info table");

/* stack of scope chains */
/* static */ int scst_init[INI_MAXNEST];

/* symbol table scope stack */
TD_INIT( td_scopestack, INI_MAXNEST, sizeof(int),
		TD_ZERO, scst_init, "scope stack");
/* blevel (td_scopestack.td_used) is block level: 0 for extern,
** 1 for ftn args, >= 2 inside function
*/

static char *ccnames[] = 
{
	 /* names of storage classes */
	"SNULL",
	"AUTO",
	"EXTERN",
	"STATIC",
	"REGISTER",
	"EXTDEF",
	"LABEL",
	"ULABEL",
	"MOS",
	"PARAM",
	"STNAME",
	"MOU",
	"UNAME",
	"TYPEDEF",
	"FORTRAN",
	"ENAME",
	"MOE",
	"UFORTRAN",
	"USTATIC",
#ifdef IN_LINE
	"INLINE",
#endif
};

char *
scnames( c )
register c; 
{
	/* return the name for storage class c */
	static char buf[12];
	if( c&FIELD )
	{
		sprintf( buf, "FIELD[%d]", c&FLDSIZ );
		return( buf );
	}
	return( ccnames[c] );
}
#endif

#ifdef IMPREGAL
	/* for register allocation optimizations */

	int fordepth;	/* nest depth of 'for' loops */
	int whdepth;	/* nest depth of 'while' and 'do-while' loops */
	int brdepth;	/* nest depth of 'if', 'if-else', and 'switch' */
#endif

#ifndef CG
/* default settings for loop test positioning */
int wloop_level = LL_BOT;	/* place "while" test at loop end */
int floop_level = LL_DUP;	/* place "for" test at top and bot */
#endif

#ifdef	CG
char costing = 0;	/* 1 if we are costing an expression */
int str_spot;		/* place for structure return */
#endif


/* function to enlarge a table described by a table descriptor */

int
td_enlarge(tp,minsize)
register struct td * tp;
int minsize;				/* minimum size needed:  0 means 1 more
					** than current
					*/
{
    int oldsize = tp->td_allo;		/* old size (for return) */
    unsigned int ocharsize = tp->td_allo * tp->td_size; /* old size in bytes */
    int newsize;			/* new size in storage units */
    unsigned int ncharsize;		/* size of new array in bytes */

    /* Realloc() previously malloc'ed tables, malloc() new one.
    ** If "end" were part of the C library, a check would have been
    ** done on the current value of the pointer, instead of having a
    ** bit in the td flags.
    */

/*    printf("%s changes from	%#lx - %#lx\n", tp->td_name,
			tp->td_start, tp->td_start+ocharsize); */

    /* determine new size:  must be "large enough" */
    newsize = tp->td_allo;		/* start at old size */
    do {
	newsize *= 2;
    } while (newsize < minsize);	/* note:  always false for minsize==0 */
    ncharsize = newsize * tp->td_size;	/* size of new array in bytes */

    if (tp->td_flags & TD_MALLOC)
	tp->td_start = realloc(tp->td_start, ncharsize);
    else {
	myVOID * oldptr = tp->td_start;

	/* copy old static array */
	if ((tp->td_start = malloc(ncharsize)) != 0)  /*lint*/
	    (void)memcpy(tp->td_start, oldptr, ocharsize);
    }
    tp->td_flags |= TD_MALLOC;		/* array now unconditionally malloc'ed */

    if (!tp->td_start)
	cerror("can't get more room for %s", tp->td_name);
    
/*    printf("		to	%#lx - %#lx\n",
			tp->td_start, tp->td_start+ncharsize); */
    /* zero out new part of array:  node and symbol tables expect this */
    if (tp->td_flags & TD_ZERO)
	(void)memset((char *) tp->td_start + ocharsize,0,(ncharsize-ocharsize));

    tp->td_allo = newsize;
    return oldsize;
}
