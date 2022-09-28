/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/debug.c	55.1"
/* debug.c */

#ifndef ELF_OBJ

/* Define db_curline whether or not LINT is defined since
 * it is used in lex.c and aclex.l.
 */

int db_curline = 0;		/* start line of current statement */
#endif

#ifndef LINT

/* Code to support standard debugging output.  The various
** flavors of output that are supported are:
**
**	1)  Start and end of function.
**	2)  Start and end of block.
**	3)  Symbol.
**	4)  Line number.
**
** This version duplicates the output from PCC for COFF.  Among
** other things, this means that specifics about which program
** section the information must be in, what size gets printed,
** what the type of a label is, and other peculiarities are all
** replicated.
** The rules for program sections are these:
**	1) Labels are in .text.
**	2) Global symbols are in .data, except
**	3) Struct/union/enum declarations are in .text.,
**	4) and function definitions, args., autos, etc., are in .text.
*/


#include "p1.h"

#ifndef	ELF_OBJ				/* assume COFF-style debug info */

#include <stdio.h>

/*HACK:  Undefine the PCC versions of these macros and use the
** COFF versions.
*/
#undef BTYPE
#undef INCREF
#undef DECREF
#undef ISPTR
#undef ISARY
#undef ISFTN
#include "syms.h"
#include "storclass.h"

/* These two symbols might not be defined in all versions of syms.h. */
#ifndef	T_VOID
#define	T_VOID	0			/* Same as T_NULL */
#endif
#ifndef	T_LDOUBLE
#define	T_LDOUBLE 1			/* Overload unused T_ARG */
#endif

/* Define strings that introduce debugging pseudo-ops. */

#ifndef	DB_DEF
#define	DB_DEF	".def"
#endif
#ifndef	DB_ENDEF
#define	DB_ENDEF ".endef"
#endif
#ifndef	DB_BF
#define	DB_BF	".bf"
#endif
#ifndef	DB_EF
#define	DB_EF	".ef"
#endif
#ifndef DB_BB
#define	DB_BB	".bb"
#endif
#ifndef	DB_EB
#define	DB_EB	".eb"
#endif
#ifndef	DB_DIM
#define	DB_DIM	".dim"
#endif
#ifndef	DB_LINE
#define	DB_LINE	".line"
#endif
#ifndef	DB_SCL
#define	DB_SCL	".scl"
#endif
#ifndef DB_SIZE
#define	DB_SIZE	".size"
#endif
#ifndef	DB_TAG
#define	DB_TAG	".tag"
#endif
#ifndef	DB_TYPE
#define	DB_TYPE	".type"
#endif
#ifndef	DB_VAL
#define	DB_VAL	".val"
#endif
#ifndef	DB_EOS
#define	DB_EOS	".eos"
#endif
#ifndef	DB_LN
#define	DB_LN	".ln"
#endif

/* This macro gives the line number relative to the beginning of a function. */
#ifndef	DB_RELLINENO
#define	DB_RELLINENO()	(db_curline-db_funcline)
#endif

/* This macro converts the register number in a symbol table entry's
** offset field into the register number that the debugger expects.
*/
#ifndef	DB_OUTREGNO
#define	DB_OUTREGNO(sid) (SY_REGNO(sid))
#endif


int db_linelevel = DB_LEVEL2;	/* line number debugger info level */
int db_symlevel = DB_LEVEL2;	/* symbol debugger info level */
static SX db_func;		/* symbol ID of current function */
static int db_funcline;		/* real line number for start of function */
static int db_lastlineno;	/* line number at last line number output */

static void db_out();
static void db_ucsymbol();
static void db_uclineno();
static T1WORD db_gbtype();
extern int elineno;
extern int db_curline;		/* current statement line number */


/* Maximum number of type modifiers that COFF can support. */
#define	MAXMODS	((16-4)/2)	/* 16-bit specifier, 4-bit base type,
				** 2 bits per modifier.
				*/
#define MAXDIMS 4		/* COFF can't handle more than four
				** .dim's!
				*/

#ifndef	INI_BBFLAGS
#define	INI_BBFLAGS 10
#endif

/* This structure (array) holds flags that say whether a .bb
** has been generated for a particular block level.
*/
TABLE(Static, td_bbflags, static, bbflags_init,
		char, INI_BBFLAGS, 0, "block flags");
#define bblevel (TD_USED(td_bbflags))
#define BBFLAG (TD_ELEM(td_bbflags,char,bblevel))


/*ARGSUSED*/
void
db_s_file(s)
char * s;				/* filename string */
/* Do what needs to be done for debug information at the start of a file. */
{
    return;
}

void
db_e_file()
/* Clean up debugging at end of source file. */
{
    return;
}


void
db_begf(sid)
SX sid;
/* Do debug stuff at start of function definition. */
{
    db_func = sid;

    /* Always do start of function code. */
    db_ucsymbol(sid);
    return;
}


void
db_s_fcode()
/* Output debug information after function prologue, before real
** function code.
*/
{
    if (db_symlevel == DB_LEVEL2) {
	db_uclineno(1);			/* now at relative line 1 in function;
					** ALWAYS generated, even if line
					** numbers disabled.
					*/

	db_funcline = 0;		/* HACK:  make .line in symbol output
					** be the true line number
					*/
	db_out(SY_NOSYM, DB_BF, TY_NONE, C_FCN, (OFFSET) 0, PROG);
	db_funcline = db_curline-1;	/* set relative line number */ 
    }

    db_funcline = db_curline-1;		/* current line is relative 1 */ 
    db_lastlineno = db_curline;

    return;
}


void
db_e_fcode()
/* Generate debug output at end of function, before function epilogue. */
{
    if (db_symlevel == DB_LEVEL2)
	db_out(SY_NOSYM, DB_EF, TY_NONE, C_FCN, (OFFSET) 0, PROG);
    db_lineno();
    return;
}


void
db_endf()
/* Generate debug output at end of entire function definition. */
{
    db_out(db_func, SY_NAME(db_func), TY_NONE, C_EFCN, (OFFSET) 0, PROG);
    return;
}



void
db_s_block()
/* Starting a new block.  Make sure there is enough space in the
** block begin flags.
*/
{
    if (++bblevel >= td_bbflags.td_allo) {
	TD_NEED1(td_bbflags);
	TD_CHKMAX(td_bbflags);
    }
    BBFLAG = 0;				/* zero flag at current level */
    return;
}


void
db_e_block()
/* Exiting a block.  Output .eb if we've already done a .bb.
** Drop the current block level.
*/
{
    if (db_symlevel == DB_LEVEL2 && BBFLAG)
	db_out(SY_NOSYM, DB_EB, TY_NONE, C_BLOCK, (OFFSET) 0, PROG);
    if (--bblevel < 0)
	cerror("confused block level in db_e_block()");
    return;
}


void
db_symbol(sid)
SX sid;
/* Produce debug information for sid, conditional on global flag. */
{
    if (db_symlevel == DB_LEVEL2)
	db_ucsymbol(sid);
    return;
}


static void
db_ucsymbol(sid)
SX sid;
/* Do unconditional symbol output for the symbol whose table
** entry is sid.
*/
{
    int coffclass;			/* COFF class for internal class */
    int locctr;

    /* Avoid output a second time. */
    if (SY_FLAGS(sid) & SY_DBOUT)
	return;

    /* No debugging for asm functions. */
    if (SY_CLASS(sid) == SC_ASM)
	return;

    /* ... or if the symbol is external and is neither defined nor
    ** tentatively defined.
    */
    if (   SY_CLASS(sid) == SC_EXTERN
	&& (SY_FLAGS(sid) & (SY_DEFINED|SY_TENTATIVE)) == 0
	)
	return;


    /* Determine appropriate COFF class for internal symbol class. */
    switch( SY_CLASS(sid) ) {
    case SC_AUTO:
	coffclass = (SY_REGNO(sid) == SY_NOREG) ? C_AUTO : C_REG;
	break;
    case SC_EXTERN:	coffclass = C_EXT; break;
    case SC_STATIC:	coffclass = C_STAT; break;
    case SC_TYPEDEF:	coffclass = C_TPDEF; break;
    case SC_PARAM:
	coffclass = (SY_REGNO(sid) == SY_NOREG) ? C_ARG : C_REGPARM;
	break;
    case SC_LABEL:	coffclass = C_LABEL; break;
    default:
	cerror("bad class in db_ucsymbol()");
    }
    /* Signify entry of new block with .bb if we haven't done so
    ** already.  No .bb for labels, though.
    */
    if (BBFLAG == 0 && SY_LEVEL(sid) > SL_INFUNC && coffclass != C_LABEL ) {
	db_out(SY_NOSYM, DB_BB, TY_NONE, C_BLOCK, (OFFSET) 0, PROG);
	BBFLAG = 1;
    }

    /* If current symbol level is within a function, debug information
    ** is in .text.  Otherwise it is in .data.
    */
    if (   SY_LEVEL(sid) != SL_EXTERN
	|| (TY_ISFTN(SY_TYPE(sid)) && (SY_FLAGS(sid) & SY_DEFINED))
       )
	locctr = PROG;
    else
	locctr = DATA;
    db_out(sid, SY_NAME(sid), SY_TYPE(sid), coffclass, (OFFSET) SY_OFFSET(sid),
		locctr);

    SY_FLAGS(sid) |= SY_DBOUT;		/* debugging has been produced */
    return;
}


void
db_lineno()
/* Output the current line, relative to the beginning of the
** current function.
*/
{
    if (db_linelevel == DB_LEVEL2 && db_curline > db_lastlineno) {
	int outline = DB_RELLINENO();
	if (outline > 0)
	    db_uclineno(outline);
	db_lastlineno = db_curline;
    }
    return;
}

static void
db_uclineno(ln)
int ln;
/* Unconditional output line number ln.  Make sure it's in .text. */
{
    cg_setlocctr(PROG);
    fprintf(outfile, "	%s	%d\n", DB_LN, ln);
    return;
}


void
db_sue(t)
T1WORD t;
/* Output member information for s/u/e t after doing so for all
** recursively referenced s/u/e's.
*/
{
    SX tag = TY_SUETAG(t);		/* symbol ID of s/u tag */
    SIZE mbrno;
    SX mbr;
    int sutag;
    int sumbr;

    if (db_symlevel != DB_LEVEL2)
	return;				/* output disabled */

    if (!TY_HASLIST(t))
	return;				/* s/u/e has no members, is incomplete */

    if ((SY_FLAGS(tag) & SY_DBOUT) == 0) {
	switch(TY_TYPE(t)) {
	case TY_STRUCT:	sutag = C_STRTAG; sumbr = C_MOS; break;
	case TY_UNION:	sutag = C_UNTAG; sumbr = C_MOU; break;
	case TY_ENUM:	sutag = C_ENTAG; sumbr = C_MOE; break;
	default:  cerror("confused db_sue()");
	}
	db_out(tag, SY_NAME(tag), t, sutag, (OFFSET) TY_SIZE(t), PROG);

	SY_FLAGS(tag) |= SY_DBOUT;	/* tag info has been output */

	for (mbrno = 0; (mbr = TY_G_MBR(t,mbrno)) != SY_NOSYM; ++mbrno ) {
	    int class = sumbr;

	    if (SY_FLAGS(mbr) & SY_ISFIELD)
		class = C_FIELD;
	    /* For enum, make type look like int. */
	    db_out(mbr, SY_NAME(mbr), (sumbr == C_MOE ? TY_INT : SY_TYPE(mbr)),
			class, (OFFSET) SY_OFFSET(mbr), PROG);
	}

	db_out(tag, DB_EOS, TY_NONE, C_EOS, (OFFSET) TY_SIZE(t), PROG);
    }
    return;
}


/* db_gtword() changes these for db_type() */
static char dims[10*MAXMODS+1];		/* will hold dimension strings:  assume
					** no more than MAXMODS dimensions, max.
					** of 9 digits each, plus ',', plus
					** trailing NUL
					*/
static char * dimend;			/* current end of string */
static int modifiers;			/* number of COFF type modifiers */
static int dimensions;

static void
db_type(t, sid)
T1WORD t;
SX sid;
/* Generate type info output for type t for symbol sid.
** If a s/u/e and the output for the item has been generated,
** produce a .tag.  If there are array dimensions, produce them.
** Output size information if there's an array or s/u/e involved:
** The size of an array is the array's size; otherwise, if an
** s/u/e is involved (such as pointer-to), the size is the size of
** the s/u/e. (This is probably silly, but it's what PCC did.)
** The "type" for a label is array of int (!) to be compatible
** with the past.
**
** Warn if we try to generate too many modifiers for COFF format.
** The limit is MAXMODS type modifiers.
*/
{
    unsigned int otword;
    T1WORD base;
    static unsigned int db_gtword();
    unsigned long outsize = 0;

    dimend = dims;			/* dimension string currently empty */
    modifiers = 0;			/* no type modifiers yet */
    dimensions = 0;			/* no dimensions yet, either */
    if (t == TY_NONE) {
	if (sid == SY_NOSYM || SY_CLASS(sid) != SC_LABEL)
	    return;			/* no type output */
	else {
	    base = TY_INT;		/* dummy value */
	    /* Fake array of int to be consistent with earlier compilers. */
	    otword = (INCREF(T_INT) + ((DT_ARY-DT_PTR)<<N_BTSHFT));
	}
    }
    else {
	base = db_gbtype(t);
	if (SY_CLASS(sid) == SC_MOE)
	    otword = T_MOE;		/* hack for special "type" */
	else
	    otword = db_gtword(t);
    }

    /* otword and base are now both set. */
    fprintf(outfile, "	%s	%#o;", DB_TYPE, otword);
    /* If top-level is a function, COFF can't deal with size/dim
    ** information AND the function information.
    */
    if (!TY_ISFTN(t) && dimend != dims) {
	outsize = TY_SIZE(t);		/* remember size of non-function */
	fprintf(outfile, "	%s	%s;", DB_DIM, &dims[1]);
					/* skip leading , */
    }
    if (TY_ISSUE(base) && (SY_FLAGS(TY_SUETAG(base)) & SY_DBOUT))
	fprintf(outfile,"	%s	%s;", DB_TAG, SY_NAME(TY_SUETAG(base)));
    if (outsize == 0L && TY_ISSUE(base))
	outsize = TY_SIZE(base);
    if (outsize != 0L)
	fprintf(outfile, "	%s	%lu;",
				DB_SIZE, (unsigned long) BITOOR(outsize));

    /* Check whether we overflowed COFF's modifier or dimension limit. */
    if (dimensions > MAXDIMS)
	WERROR("debug output inaccurate, too many dimensions: %s",
		SY_NAME(sid));
    else if (modifiers > MAXMODS)
	WERROR("debug output inaccurate, too many modifiers: %s",
		SY_NAME(sid));
    return;
}


static unsigned int
db_gtword(t)
T1WORD t;
/* Recursively build an old-style type word from Pass 1 type t.
** Store dimension information at dimend.  Bump number of type
** modifiers seen, if any.
*/
{
    unsigned int newtype;
    SIZE nelem;

    switch( TY_TYPE(t) ) {
/* There are only two codes for three char types.  Treat T_CHAR as
** signed char; treat ``plain'' char as either signed or unsigned,
** depending on implementation.
*/
#ifdef	C_CHSIGN			/* plain chars are signed */
    case TY_CHAR:	newtype = T_CHAR; break;
#else
    case TY_CHAR:	newtype = T_UCHAR; break;
#endif
    case TY_SCHAR:	newtype = T_CHAR; break;
    case TY_UCHAR:	newtype = T_UCHAR; break;

    case TY_SSHORT:
    case TY_SHORT:	newtype = T_SHORT; break;
    case TY_USHORT:	newtype = T_USHORT; break;

    case TY_SINT:
    case TY_INT:	newtype = T_INT; break;
    case TY_UINT:	newtype = T_UINT; break;

    case TY_SLONG:
    case TY_LONG:	newtype = T_LONG; break;
    case TY_ULONG:	newtype = T_ULONG; break;

    case TY_FLOAT:	newtype = T_FLOAT; break;
    case TY_DOUBLE:	newtype = T_DOUBLE; break;
    case TY_LDOUBLE:	newtype = T_LDOUBLE; break;

    case TY_ENUM:	newtype = T_ENUM; break;
    case TY_STRUCT:	newtype = T_STRUCT; break;
    case TY_UNION:	newtype = T_UNION; break;

    case TY_VOID:	newtype = T_VOID; break;

    /* Modified types.  Beware of double evaluation in INCREF! */
    case TY_ARY:
	nelem = TY_NELEM(t);
	if (nelem == TY_NULLDIM || nelem == TY_ERRDIM)
	    nelem = 0;
	/* Don't collect more than MAXMODS dimensions. */
	if (dimensions < MAXDIMS) {
	    dimend += sprintf(dimend, ",%ld", (long) nelem);
	    if (dimend >= dims + sizeof(dims))
		cerror("dimension overflow");
	}
	++dimensions;
	newtype = db_gtword(TY_DECREF(t));
	newtype = INCREF(newtype);
	newtype += (DT_ARY-DT_PTR) << N_BTSHFT;
	++modifiers;
	break;
    
    case TY_PTR:
	newtype = db_gtword(TY_DECREF(t));
	newtype = INCREF(newtype);
	++modifiers;
	break;
    
    case TY_FUN:
	newtype = db_gtword(TY_DECREF(t));
	newtype = INCREF(newtype);
	newtype += (DT_FCN-DT_PTR) << N_BTSHFT;
	++modifiers;
	break;
    
    default:
	cerror("confused db_gtword()");
    }
    return( newtype );
}


static void
db_out(sid, s, t, class, val, section)
SX sid;
char * s;
T1WORD t;
int class;
OFFSET val;
int section;
/* Generate debugging information for symbol s of type t,
** with storage class "class", and value val.  Exactly
** what gets generated depends on the "class", which is
** the COFF class.  The information gets generated in
** program section "section".
*/
{
    static int firstout = 1;

    /* Before any output, make sure to have CG do what gets done at
    ** beginning of file.
    */
    if (firstout) {
	cg_begfile();
	firstout = 0;
    }

    /* Some errors lead to a null s.  Make it printable. */
    if (!s)
	s = "<NULLSTRING>";

    cg_setlocctr(section);

    fprintf(outfile, "	%s	%s;	%s	%d;", DB_DEF, s, DB_SCL, class);
    db_type(t, sid);			/* output type information */

    /* Other stuff depends on the storage class. */
    switch( class ) {
    case C_AUTO:
    case C_ARG:
	fprintf(outfile, "	%s	%ld;", DB_VAL, (long) val);
	break;
    case C_MOS:
    case C_MOU:
	fprintf(outfile, "	%s	%ld;", DB_VAL, (long) BITOOR(val));
	break;
    case C_REGPARM:
    case C_REG:
	val = DB_OUTREGNO(sid);		/* convert register number */
	/*FALLTHRU*/
    case C_MOE:
	fprintf(outfile, "	%s	%ld;", DB_VAL, (long) val);
	break;
    case C_STAT:
    case C_EXT:
	fprintf(outfile, "	%s	%s;", DB_VAL, cg_extname(sid));
	break;
    case C_LABEL:
	fprintf(outfile, "	%s	", DB_VAL);
	fprintf(outfile, LABFMT, (int) val);
	putc(';', outfile);
	break;
    case C_FIELD:
    {
	int size;
	BITOFF offset;

	SY_FLDUPACK(sid, size, offset);
	fprintf(outfile, "	%s	%lu;	%s	%lu;",
			DB_VAL, (unsigned long) offset,
			DB_SIZE, (unsigned long) size);
	break;
    }
    case C_EOS:
	fprintf(outfile, "	%s	%lu;	%s	%s;	%s	%lu;",
			DB_VAL, (unsigned long) BITOOR(val),
			DB_TAG, SY_NAME(sid),
			DB_SIZE, (unsigned long) BITOOR(val));
	break;
    case C_BLOCK:
    case C_FCN:
	fprintf(outfile, "	%s	%d;", DB_LINE, DB_RELLINENO());
	/*FALLTHRU*/
    case C_EFCN:
	fprintf(outfile, "	%s	.;", DB_VAL);
	break;
    case C_TPDEF:
    case C_STRTAG:
    case C_UNTAG:
    case C_ENTAG:
	/* Don't need to do anything for these.  (Handled by type printing.) */
	break;
    default:
	cerror("confused class %d in db_out()", class);
    }
	
    if (t != TY_NONE && SY_LEVEL(sid) > SL_INFUNC)
	fprintf(outfile, "	%s	%d;", DB_LINE, DB_RELLINENO());
    fprintf(outfile, "	%s\n", DB_ENDEF);
    return;
}

static T1WORD
db_gbtype(t)
T1WORD t;
/* Get base type for type word t.  Walk the list of modifiers
** until we reach a basic type or struct/union/enum.
*/
{
    for (;;){
	switch( TY_TYPE(t) ){
	case TY_PTR:
	case TY_FUN:
	case TY_ARY:
	    t = TY_DECREF(t);
	    continue;			/* continue looking */
	default:
	    return( t );
	}
    }
    /*NOTREACHED*/
}

#endif /* ifndef ELF_OBJ */

#endif /* ifndef LINT */
