/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/cgstuff.c	55.5"
/* cgstuff.c */

/* This module contains routines that serve as mediators between
** the compiler front-end and the CG back-end.  p1allo.c contains
** CG-related routines that allocate storage offsets.
*/

#include "p1.h"
#include "mfile2.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#ifdef	__STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define	t2alloc() ((ND2 *) talloc())
#define ND2NIL ((ND2 *) 0)

/* put this definition in mfile1.h eventually */
#define FF_ISVOL        020             /* flag to indicate volatile attribute */

#ifndef	IDENTSTR
#define	IDENTSTR ".ident"
#endif

#ifndef	MYLABFMT
#define	MYLABFMT	".X%d"
#endif

#define	MYLABTYPE	int
#ifndef LINT
static void cg_p2tree();

static int cg_lastlocctr = UNK;		/* last location counter used:
					** unknown initially
					*/
static int cg_savelc = UNK;		/* saved location counter (none) */
static char * cg_fstring = "";		/* current filename string */
static int cg_outhdr = 1;		/* need to output file header stuff
					** if non-0
					*/
#endif

#ifdef	FAT_ACOMP
static int cg_infunc = 0;		/* non-zero if within function */
#endif
static int cg_ldused = 0;		/* long double used somewhere */

/* Force out a "touch" of a symbol that must be present in start-up
** code to support long double as double.
*/
#ifndef	CG_LDUSED
#define	CG_LDUSED() cg_copyasm("\t.globl\t__longdouble_used")
#endif

static char * cg_statname();
#ifdef	FAT_ACOMP
static void cg_q_gen();
static void cg_q_nd2();
static void cg_q_nd2call();
static void cg_q_puts();
#endif
#ifndef LINT
static void cg_inflush();
static void cg_inaccum();
static void cg_outbytes();
static void cg_copyasm();
static ND1 * cg_strout();
static void cg_pushlc();
static void cg_poplc();

/* void cg_p2compile(ND2 *p) */
#ifdef FAT_ACOMP
#define CG_PUSHLC(lc) cg_q_int(cg_pushlc, lc)
#define CG_POPLC() cg_q_call(cg_poplc)
#define CG_SETLOCCTR(lc) cg_q_int(cg_setlocctr, lc)
#define	cg_p2compile(p) if (cg_outhdr) \
			    cg_begfile(); \
			cg_q_nd2(p);
#else	/* ! FAT_ACOMP */
#define CG_PUSHLC(lc) cg_pushlc(lc)
#define CG_POPLC() cg_poplc()
#define CG_SETLOCCTR(lc) cg_setlocctr(lc)
#define	cg_p2compile(p) if (cg_outhdr) \
			    cg_begfile(); \
			p2compile((NODE*) p);
#endif	/* def FAT_ACOMP */
#else
#define cg_p2compile(p) tfree((NODE*) p)
#define cg_p2tree(p)
#endif


#ifndef LINT
extern int tyreg();
/* Replaced with null macro for lint */
#endif

TWORD
cg_tconv(t, ldflag)
T1WORD t;
int ldflag;
/* Convert a pass 1 type to a pass 2 type.  If ldflag is non-zero,
** complain about long doubles, as appropriate for the compiler
** version.  Otherwise, treat long double like double.
*/
{
    switch( TY_TYPE(t) ) {
#ifdef C_CHSIGN
    case TY_CHAR:
#endif
    case TY_SCHAR:			return( T2_CHAR );

#ifndef C_CHSIGN
    case TY_CHAR:
#endif
    case TY_UCHAR:			return( T2_UCHAR );

    case TY_SHORT:	case TY_SSHORT:	return( T2_SHORT );
    case TY_USHORT:			return( T2_USHORT );

    case TY_INT:	case TY_SINT:	return( T2_INT );
    case TY_UINT:			return( T2_UNSIGNED );

    case TY_LONG:	case TY_SLONG:	return( T2_LONG );
    case TY_ULONG:			return( T2_ULONG );

    case TY_FLOAT:			return( T2_FLOAT );
    case TY_DOUBLE:			return( T2_DOUBLE );
    case TY_LDOUBLE:
	/* Do check for error message, then treat as double. */
	if (ldflag)
	    cg_ldcheck();
	return( T2_DOUBLE );

    case TY_STRUCT:	case TY_UNION:	return( T2_STRUCT );

    case TY_VOID:			return( T2_VOID );
    case TY_ENUM:
    {
	BITOFF tsize = TY_SIZE(t);

	if      (tsize == TY_SIZE(TY_INT))	return( T2_INT );
#if	0
	/* small enums not supported */
	else if (tsize == TY_SIZE(TY_SHORT))	return( T2_SHORT );
	else if (tsize == TY_SIZE(TY_CHAR))	return( T2_CHAR );
	return( T2_INT );			/* to keep us going */
#endif
	cerror("unknown size");
	/*NOTREACHED*/
    }

    case TY_PTR:
	return ( T2_ADDPTR(cg_tconv(TY_DECREF(t), ldflag)) );
    case TY_FUN:
	return ( T2_ADDFTN(cg_tconv(TY_DECREF(t), ldflag)) );
    case TY_ARY:
	return ( T2_ADDARY(cg_tconv(TY_DECREF(t), ldflag)) );

    default:
	cerror("confused type %d in cg_tconv()", t);
    }
    /*NOTREACHED*/
}

#ifndef LINT
/* lint doesn't need below routines - replaced with stubs */

static 
cg_align(t)
T1WORD t;
/* Produce CG alignment type for pass 1 type t. */
{
    TWORD altype;

    for (;;) {					/* for array case */
	switch( TY_TYPE(t) ) {
	    int align;
	case TY_STRUCT:
	case TY_UNION:
	    /* Choose some other type that has the alignment of the s/u,
	    ** because that information is missing from TSTRUCT.
	    */
	    align = TY_ALIGN(t);
	    if (align < ALSTRUCT)
		altype = ALSTRUCT;
	    else {				/* only bother if necessary */
		if (align == ALINT)
		    altype = T2_INT;
		else if (align == ALSHORT)
		    altype = T2_SHORT;
#if 0
		else if (align == ALLDOUBLE)
		    altype = T2_LDOUBLE
#endif
		else if (align == ALDOUBLE)
		    altype = T2_DOUBLE;
		else if (align == ALFLOAT)
		    altype = T2_FLOAT;
		else if (align == ALPOINT)
		    altype = T2_ADDPTR(T2_VOID); /* arbitrary pointer type */
		else
		    altype = T2_CHAR;
	    }
	    break;
	case TY_ARY:
	    t = TY_DECREF(t);
	    continue;
	case TY_PTR:
	    altype = T2_ADDPTR(T2_VOID); break;
	/* align functions like ints */
	case TY_FUN:				altype = T2_INT; break;
	default:				altype = cg_tconv(t,0); break;
	} /* end switch */
	break;					/* out of loop */
    } /* end for */
    return( altype );
}


void
cg_setlocctr(lc)
int lc;
/* Generate code to set location counter lc.  If there's no
** change, just exit.
*/
{
    ND2 * new;

    if (lc == cg_lastlocctr) return;

    cg_lastlocctr = lc;

    new = t2alloc();
    new->op = LOCCTR;
    new->lval = lc;
    cg_p2compile(new);
}

static void
cg_pushlc(lc)
int lc;
/* Push current location counter on (1-level) stack,
** set new location counter, lc.
*/
{
    if (cg_savelc != UNK)
	cerror("cg_pushlc:  lc already saved");
    cg_savelc = cg_lastlocctr;
    cg_setlocctr(lc);
    return;
}

static void
cg_poplc()
/* Restore saved (on 1-level stack) location counter. */
{
    if (cg_savelc == UNK)
	cerror("cg_poplc:  no saved lc");
    cg_setlocctr(cg_savelc);
    cg_savelc = UNK;
    return;
}


void
cg_defnam(sid)
SX sid;
/* Output symbol definition stuff for static/extern's first
** tentative definition, or for actual definition.
** The location counter must be set appropriately for the symbol.
** Only produce output for these cases:
**	1) Function definition.
**	2) New tentative definition.
**	3) Defining instance (has initializer).
*/
{
    ND2 * new;
    T1WORD t;
    BITOFF sz = 0;			/* size of COMMON */
    TWORD altype;			/* alignment type for CG */
    int class = SY_CLASS(sid);		/* symbol class */
    SY_FLAGS_t hasinit = (SY_FLAGS_t) (SY_FLAGS(sid) & SY_DEFINED);
    int isfunc = 0;
    
    /* Determine the proper alignment type for CG. */
    t = SY_TYPE(sid);
    if (TY_TYPE(t) == TY_FUN) {
	if (!hasinit) return;		/* function decl is unimportant */
	isfunc = 1;
    }
    altype = cg_align(t);

    if (!hasinit) sz = TY_SIZE(t);	/* size for common */

    new = t2alloc();

    new->op = DEFNAM;
    new->name = cg_extname(sid);
    new->type = altype;
    new->rval = sz;
    new->lval = 0;
    if (hasinit) {
	new->lval |= DEFINE;
	if (class == SC_EXTERN)
	    new->lval |= EXTNAM;
    }
    else
	new->lval |= (class == SC_EXTERN ? COMMON : LCOMMON);
    if (isfunc)
	new->lval |= FUNCT;

    cg_p2compile(new);
    return;
}

char *
cg_extname(sid)
SX sid;
/* Return the character string that corresponds to the (presumed)
** external representation of a symbol's name.  Functions always
** use the symbol's name.  Take into account the alternate naming
** for block statics.
*/
{
    switch( SY_CLASS(sid) ){
    case SC_STATIC:
	if (SY_LEVEL(sid) != SL_EXTERN && !TY_ISFTN(SY_TYPE(sid)))
	    return( cg_statname((CONVAL) SY_OFFSET(sid)) );
	/*FALLTHRU*/
    case SC_EXTERN:
    case SC_ASM:
	return( SY_NAME(sid) );
    }
    return( (char *) 0 );		/* has no name */
}


void
cg_nameinfo(sid)
SX sid;
{
#ifdef	ELF_OBJ
    SY_CLASS_t class = SY_CLASS(sid);

    /* Only produce information for function definitions and external
    ** or static objects.
    */
    switch( class ){
	ND2 * nameinfo;
    case SC_STATIC:
    case SC_EXTERN:
	nameinfo = t2alloc();

	nameinfo->op = NAMEINFO;
	nameinfo->type = T2_VOID;
	nameinfo->name = cg_extname(sid);
	if (TY_ISFTN(SY_TYPE(sid))) {
	    CG_SETLOCCTR(PROG);
	    nameinfo->lval = 0;
	    nameinfo->rval = NI_FUNCT;
	}
	else {
	    nameinfo->lval = TY_SIZE(SY_TYPE(sid));
	    /* Select appropriate symbol kind. */
	    if (class == SC_EXTERN)
		nameinfo->rval = NI_GLOBAL;
	    else if (SY_LEVEL(sid) == SL_EXTERN)
		nameinfo->rval = NI_FLSTAT;
	    else
		nameinfo->rval = NI_BKSTAT;
	    nameinfo->rval |= NI_OBJCT;
	}
	cg_p2compile(nameinfo);
    }
#endif
    return;
}


/* These routines interface to CG's initialization handling.
** Because CG doesn't support bitfield initialization, we
** must accumulate field words ourselves, if necessary, and
** spit them out at the end.  All initializers get accumulated
** in an object of size INITSIZE.
** This code maintains state information:  current bit number,
** current accumulated bits.
*/

static CONVAL cg_inval;			/* current accumulated initializer */
static BITOFF cg_gotbits;		/* number of bits accum. therein */
static BITOFF cg_curoff;		/* current bit number */
static int cg_initsize;			/* maximum-sized unit to accumulate */
static int cg_inlocctr = UNK;		/* location counter being used for
					** current initialization.
					*/


void
cg_instart(t, readonly)
T1WORD t;
int readonly;
/* Set location counter for data initialization for type t.
** The type dictates whether the data go in read-only or
** read/write storage.  Non-volatile const stuff goes into
** read-only storage.  If readonly == C_READONLY, force stuff
** into read-only section.
** Begin accumulating initializers.  Zero the current
** bit and value.  t is the type being initialized.
** That type's alignment affects how big a hunk we can
** accumulate at one time.
*/
{
    cg_inlocctr = CDATA;		/* assume read-only */
    if (readonly != C_READONLY) {
	while (TY_ISARY(t))
	    t = TY_DECREF(t);		/* get to non-array type */
	if (!TY_ISCONST(t) || TY_ISMBRVOLATILE(t))
	    cg_inlocctr = DATA;
    }
    CG_SETLOCCTR(cg_inlocctr);

    cg_inval = 0;
    cg_curoff = 0;
    cg_gotbits = 0;
    /* Limit size to SZLONG:  that's the size of the biggest
    ** bitfield, which is the maximum we must (or can)
    ** accumulate.
    */
    if ((cg_initsize = TY_ALIGN(t)) > SZLONG)
	cg_initsize = SZLONG;
    return;
}

static TWORD
cg_intype(size)
BITOFF size;
/* Return the pass 2 integral type that corresponds in size
** to "size".  If none corresponds, return 0.  (This assumes,
** of course, that none of those types have a zero value.)
*/
{
    /* Can't do switch because of possibly duplicate values. */
    if (size == TY_SIZE(TY_INT))
	return( T2_INT );
#ifndef	NOLONG
    if (size == TY_SIZE(TY_LONG))
	return( T2_LONG );
#endif
#ifndef	NOSHORT
    if (size == TY_SIZE(TY_SHORT))
	return( T2_SHORT );
#endif
    if (size == TY_SIZE(TY_CHAR))
	return( T2_CHAR );
    return( 0 );
}


void
cg_incode(p, len)
ND1 * p;
BITOFF len;
/* Do static initialization with an integral value or address.
** p points to an ICON or FCON node; len is the length, in bits,
** of the constant.
**
** Assume alignment is correct already and type of initializer is
** suitable for thing to be initialized.
*/
{
    ND2 * new;
    T1WORD t = p->type;			/* initialization type */
    TWORD p2t;				/* back-end type */
    BITOFF morebits = 0;		/* extra bits needed at end */

    /* CG can't handle named ICONs (the result of ( & NAME )) or
    ** STRINGs in bitfields, nor in smaller than int regular initializers.
    ** NOTE:  Assumes all pointers are the same size!
    */
    if (    ((p->op == ICON && p->rval != ND_NOSYMBOL) || p->op == STRING)
	&&  len < TY_SIZE(TY_VOIDSTAR)
    )
	UERROR("invalid initializer");

    if (p->op == ICON && p->rval == ND_NOSYMBOL && len < cg_initsize) {
	/* Accumulate integral value. */
	p->op = FREE;
	cg_inaccum((UCONVAL) p->lval, len);
	return;
    } /* end small int case */
	
    if (cg_gotbits)
	cg_inflush();			/* flush accumulated bits */

    /* For pointer types, make the CG type bit big enough to handle
    ** the number of bits we want to output.  For floating types,
    ** just choose the right one.
    */
    if (TY_ISFPTYPE(t)) {
	p2t = cg_tconv(p->type, 1);
	/* Under -Xc we may produce too few bits if we output a double. */
	if (TY_TYPE(p->type) == TY_LDOUBLE && p2t == T2_DOUBLE)
	    morebits = TY_SIZE(TY_LDOUBLE) - TY_SIZE(TY_DOUBLE);
    }
    else if ((p2t = cg_intype(len)) == 0) {
	cg_inaccum((UCONVAL) 0, len);	/* this is error case; do this to
					** keep going
					*/
	p->op = FREE;
	return;
    }
    
    CG_SETLOCCTR(cg_inlocctr);		/* set location counter to current for
					** initialization
					*/

    new = t2alloc();
    new->op = INIT;

    cg_p2tree((ND2 *) p);
    new->left = (ND2 *) p;
    /* (CG bug forces us to change types of both nodes.) */
    new->type = p->type = p2t;		/* Pass 2 type */
    
    cg_p2compile(new);
    cg_curoff += len;
    if (morebits)
	cg_zecode(morebits);
    return;
}


void
cg_zecode(len)
BITOFF len;
/* Output len zero bits starting at the current bit position.
** If necessary, flush accumulated bits.
*/
{
    BITOFF bitsleft = cg_initsize - (cg_curoff % cg_initsize);
				/* what's currently accumulated */

#if 0
    printf("in zecode, len = %d, curoff = %d, inval = %d, gotbits = %d\n",
	len, cg_curoff, cg_inval, cg_gotbits);
#endif

    if (bitsleft < cg_initsize) {
	if (len >= bitsleft) {
	    cg_inaccum((UCONVAL) 0, bitsleft);
	    if (cg_gotbits)		/* flush now if not already flushed */
		cg_inflush();
	    len -= bitsleft;
	}
	else {
	    cg_inaccum((UCONVAL) 0, len);
	    len = 0;
	}
    }

    /* len is now number of remaining bits to zero.  Move to next
    ** cg_initsize boundary.
    */
    if (len != 0) {
	BITOFF upto = ((cg_curoff + len) / cg_initsize) * cg_initsize;

	if (upto >= cg_curoff + TY_SIZE(TY_CHAR)) {
	    ND2 * new = t2alloc();

	    new->op = UNINIT;		/* uninitialized stuff */
	    new->type = T2_CHAR;	/* always push out chars */
	    new->lval = (upto-cg_curoff) / TY_SIZE(TY_CHAR);
	    cg_p2compile(new);
	}
	cg_curoff += len;
	cg_gotbits = cg_curoff - upto;
    }
    return;
}

static void
cg_inaccum(v, len)
UCONVAL v;
BITOFF len;
/* Accumulate integer bits, when cg_initsize word is full, flush.
** Keep things shifted so right-most cg_gotbits of cg_inval
** represent value.
*/
{
    static UCONVAL masks[] = {
	0x00000000, 0x00000001, 0x00000003, 0x00000007,
	0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
	0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
	0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
	0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
	0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
	0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
	0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
	(UCONVAL) 0xffffffff
    };
#if SZLONG > 32
#include "can't handle bitfields with mask"	/* internal error */
#endif

    if (cg_gotbits + len > cg_initsize)
	cg_inflush();			/* flush what we've got so far */

    v &= masks[len];			/* truncate value */

#ifdef	RTOLBYTES
    cg_inval = (v << cg_gotbits) | cg_inval;
#else
    cg_inval = (cg_inval << len) | v;
#endif
    cg_gotbits += len;
    cg_curoff += len;

    if (cg_gotbits == cg_initsize || cg_curoff % cg_initsize == 0)
	cg_inflush();
    return;
}

static void
cg_inflush()
/* Force out any accumulated bits.  If the current bit offset
** is a multiple of cg_initsize, assume there are cg_initsize bits.
** cg_gotbits is presumedly non-zero and a multiple of TY_SIZE(TY_CHAR).
*/
{
    TWORD t;

#if 0
    printf("in inflush, curoff = %d, inval = %d, gotbits = %d\n",
	cg_curoff, cg_inval, cg_gotbits);
#endif

    /* Set correct location counter. */
    CG_SETLOCCTR(cg_inlocctr);

    /* Push out uninitialized stuff if value is zero. */
    if (cg_inval == 0) {
	ND2 * uninit = t2alloc();

	uninit->op = UNINIT;
	uninit->type = T2_CHAR;
	uninit->lval = cg_gotbits/TY_SIZE(TY_CHAR);
	cg_p2compile(uninit);
    }
    /* Figure out an appropriate type, based on bits. */
    else if ((t = cg_intype(cg_gotbits)) != 0) {
	ND2 * icon = t2alloc();
	ND2 * init;

	icon->op = ICON;
	icon->type = t;
	icon->lval = cg_inval;
	icon->name = (char *) 0;

	init = t2alloc();
	init->op = INIT;
	init->type = t;
	init->left = icon;
	init->right = ND2NIL;
	cg_p2compile(init);
    }
    else if (cg_gotbits % TY_SIZE(TY_CHAR) == 0) {
	/* We have straggler bytes because of a string init.:
	** struct { char ca[1], c; } x = { "a", 1 };
	*/
	cg_outbytes();
    }
    else
	cerror("confused cg_inflush()");

    cg_gotbits = 0;
    cg_inval = 0;
    return;
}

static void
cg_outbytes()
/* This routine flushes bytes that are offset from the alignment
** of cg_initsize.  Because of the off-alignment, they must be
** produced a byte at a time.  cg_gotbits is assumed to be a
** multiple of a character-size.
*/
{
    unsigned int charmask = ~(~0 << TY_SIZE(TY_CHAR));
    int shiftsize;

#ifdef RTOLBYTES
    for (shiftsize = 0; shiftsize < cg_gotbits; shiftsize += TY_SIZE(TY_CHAR)) {
#else
    shiftsize = cg_gotbits;
    while ((shiftsize -= TY_SIZE(TY_CHAR)) >= 0) {
#endif
	ND2 * icon = t2alloc();
	ND2 * init;

	icon->op = ICON;
	icon->lval = (cg_inval >> shiftsize) & charmask;
	icon->name = (char *) 0;
	icon->type = T2_UCHAR;

	init = t2alloc();
	init->op = INIT;
	init->left = icon;
	init->right = ND2NIL;
	init->type = T2_UCHAR;
	cg_p2compile(init);
    }
    return;
}

void
cg_inend()
/* Flush any accumulated bits. */
{
    if (cg_gotbits != 0)
	cg_inflush();
    cg_curoff = 0;
    cg_gotbits = 0;
    return;
}

ND1 *
cg_strinit(p, len, wantname, readonly)
ND1 * p;
SIZE len;
int wantname;
int readonly;
/* Build static string from STRING node with length "len".  If "len"
** is 0, use the whole string, including trailing null.  If wantname
** is non-0, put out a label first, and return a NAME node that
** corresponds to the initializer.  Generate the string in read-only
** space if readonly is C_READONLY.
** If the rval field of the STRING node is non-zero, the string contains
** wide characters (still as multi-byte string).  len is then the
** number of multibyte characters.  Do the appropriate thing, one
** multibyte character at a time.
*/
{
    ND1 * retval = ND1NIL;
    int iswide = p->rval & TR_ST_WIDE;	/* wide character flag */

    /* Put named strings in "initialized string data", unnamed in data. */

    if (wantname) {
	/* Build NAME node to return.  Type is array of char of appropriate
	** length, local symbol (label newlab).
	*/
	CG_PUSHLC(readonly == C_READONLY ? CSTRNG : ISTRNG);
	/* Having strings aligned on int boundaries frequently allows
	** string functions to work faster.
	*/
	retval = cg_defstat((T1WORD) (iswide ? T_wchar_t : TY_INT));
	/* Change type if restricted length. */
	retval->type = ty_mkaryof((T1WORD) (iswide ? T_wchar_t : TY_CHAR),
				   (len ? len : TY_NULLDIM));
    }
    else if (cg_gotbits)
	cg_inflush();			/* purge any accumulated stuff first */

    if (len == 0)
	len = p->lval+1;		/* use full length, NUL included */

    if (iswide) {
	char *sp = p->string;
	int wlen = len;			/* number of characters to generate */
	
	for (; wlen > 0; --wlen) {
	    ND1 * init = tr_newnode(INIT);
	    ND1 * icon = tr_newnode(ICON);
	    wchar_t wchar = lx_mbtowc(sp);

	    init->type = T_wchar_t;
	    init->left = icon;

	    icon->type = T_wchar_t;
	    icon->lval = wchar;
	    icon->rval = ND_NOSYMBOL;

	    cg_p2tree((ND2 *) init);
	    cg_p2compile((ND2 *) init);

	    sp += sizeof(wchar_t);
	}
    }
    else {
	ND2 * new = t2alloc();
	new->op = SINIT;
	new->type = T2_CHAR;
	new->lval = len;
	new->name = p->string;
	cg_p2compile(new);
    }

    p->op = FREE;			/* won't need STRING anymore */

    if (wantname)
	CG_POPLC();
    else
	/* Adjust initialization offset according to size. */
	cg_curoff += len * TY_SIZE((T1WORD) (iswide ? T_wchar_t : TY_CHAR));
    return( retval );
}

static ND1 *
cg_strout(p)
ND1 * p;
{
    /* Turn STRING into an ICON or NAME node.
    ** If type is ptr to const, put in const data section.
    ** Type might not be pointer/array if cast got pushed
    ** onto STRING node.
    */
    T1WORD t = p->type;
    int newop = (p->rval & TR_ST_ICON) ? ICON : NAME;

    if ( (TY_ISPTR(t) || TY_ISARY(t)) && TY_ISCONST(TY_DECREF(t)) )
        p = cg_strinit(p, 0, 1, C_READONLY);
    else
        p = cg_strinit(p, 0, 1, C_READWRITE);
    p->op = newop;
    return (p);
}

void
cg_treeok()
/* Tell the tree allocator that we're not caught in an
** infinite loop, to forestall messages about "out of
** tree space".
*/
{
    extern int watchdog;
    watchdog = 0;			/* reset watchdog check */
    return;
}


void
cg_ecode( p )
register ND1 *p;
{
    extern int watchdog;

#ifndef NODBG
    if (b1debug > 1)
	tr_eprint(p);			/* print tree before conversion */
#endif

    /* standard version of writing the tree nodes */
    if( nerrors ) {
	t1free(p);			/* discard the tree */
	return;
    }

    if (p) {
	/* Suppress code with an explicit access to a void value
	** via STAR.  CG has problem with access through volatile
	** void *.
	*/
	if (p->op == STAR && TY_TYPE(p->type) == TY_VOID) {
	    WERROR("access through \"void\" pointer ignored");
	    p->op = FREE;		/* just avoid the access */
	    p = p->left;
	}
	CG_SETLOCCTR(PROG);
	cg_p2tree( (ND2 *) p );
	cg_p2compile( (ND2 *) p );

	al_e_expr();
    }
    watchdog = 0;			/* Persuade CG that node allocation is
					** still sane.
					*/
    return;
}


# ifndef RNODNAME
# define RNODNAME MYLABFMT
# endif

static void
cg_p2tree(p)
ND2 * p;
{
    register ty;
    register o;
    T1WORD otype = p->type;		/* original Pass 1 type */
    T1WORD ftype;			/* type of left side of FLD node */
    int flags = ((ND1 *)p)->flags;	/* flags of pass 1 node */
#ifdef VA_ARG
    ND2 * va_arg_node;			/* ICON node:  2nd arg of special func */
    T1WORD va_arg_type;			/* ICON's Pass 1 type (pointed-at) */
#endif

# ifdef MYP2TREE
    MYP2TREE(p);  /* local action can be taken here; then return... */
# endif

    /* this routine sits painfully between pass1 and pass2 */
    ty = optype(o=p->op);
    p->goal = 0;			/* an illegal goal, just to clear it out */
    p->type = cg_tconv( otype, 1 );	/* type gets second-pass (bits) */

    /* strategy field marks a volatile operand */
    if ((flags & FF_ISVOL) != 0  && p->op != ICON)
	p->strat |= VOLATILE;

    switch( o )
    {
        case STRING:
        {
            ND2 * temp;
            T1WORD ntype = p->type;     /* save pass 2 type */

            p->type = otype;            /* reassign old type (pass 1) */
	    /* Output string, p becomes ICON. */
            temp = (ND2 *)cg_strout((ND1 *) p);
            *p = *temp;                 /* copy struct to p */
            temp->op = FREE;            /* discard temp now */
            p->type = ntype;            /* restore pass2 type */
        }
        /*FALLTHRU:  p is now ICON*/
	case ICON:
	case NAME:
	    if( p->rval == ND_NOSYMBOL )
		    p->name = (char *) 0;
	    else if( p->rval > 0 )
	    {
		 SX sid = p->rval;		/* embedded symbol */
		 /* copy name from exname */
#if 0
/* This stuff SHOULD call exname(); leave out call for now. */
			register char *cp;
			cp = exname( SY_NAME(p->rval) );
			p->name = st_lookup( cp );
#else
		p->name = cg_extname(sid);
#endif

		/* Convert op, offset, etc., based on storage class. */
		switch (SY_CLASS(sid)){
		    int regno;
		case SC_AUTO:
		    p->op = VAUTO;
		    p->rval = ND_NOSYMBOL;
		    p->lval += (SY_OFFSET(sid));
		    if ((regno = SY_REGNO(sid)) != SY_NOREG) {
			p->op = REG;
			p->rval = regno;
			p->lval = 0;
		    }
		    break;
		case SC_PARAM:
		    p->op = VPARAM;
		    p->rval = ND_NOSYMBOL;
		    p->lval += (SY_OFFSET(sid));
		    if ((regno = SY_REGNO(sid)) != SY_NOREG) {
			p->op = REG;
			p->rval = regno;
			p->lval = 0;
		    }
		    break;
		case SC_STATIC:
		    if (SY_LEVEL(sid) == SL_EXTERN)
			p->rval = NI_FLSTAT;
		    else
			p->rval = NI_BKSTAT;
		    break;
		case SC_EXTERN:
		    p->rval = NI_GLOBAL;
		    break;
		case SC_ASM:
		case SC_MOS:
		case SC_MOU:
		    break;
		default:
		    cerror("strange storage class in cg_p2tree()");
		    /*NOTREACHED*/
		}
	    }
#if	0
	    /* STATSRET not supported */
	    else if( p->rval == - strftn )
	    {
		char temp[32];			/* place to dump label stuff */
		sprintf( temp, RNODNAME, -p->rval );
		p->name = st_lookup( temp );
	    }
#endif
	    else
	    {
		/* p->rval is negative, is a label number already. */
		p->name = cg_statname((CONVAL) -p->rval);
		p->rval = NI_FLSTAT;
	    }
	    break;


	/* Find the structure size and alignment of structure ops. */
	case STARG:
	case STASG:
	case STCALL:
	case UNARY STCALL:
	{
	    /* Front-end hides true s/u type in p->sttype.  Grab it
	    ** before it possibly gets clobbered by union-reference
	    ** of another field (name).
	    */
	    T1WORD sttype = ((ND1 *)p)->sttype;

	    if (!TY_ISPTR(otype) || !TY_ISSU(sttype))
		cerror("confused cg_p2tree()");
	    /* CG can't figure arg. size unless type is T2_STRUCT. */
	    if (o == STARG)
		p->type = T2_STRUCT;
	    else if (callop(o))
		p->name = ((ND1 *)p)->string;
	    p->stalign = (short) TY_ALIGN(sttype);
	    p->stsize = TY_SIZE(sttype);
	}
	    break;

	case CALL:
#ifdef VA_ARG
	{
	    static char * va_arg_name;
	    ND2 * argp;

	    if (va_arg_name == 0)
		va_arg_name = st_lookup(VA_ARG);
	    
	    va_arg_node = ND2NIL;

	    /* Look for special name, remember type of second argument.
	    ** Later (below), doctor the second argument node.
	    ** Have to do this now, before child types have been changed.
	    */
	    if (   p->left->op == ICON
		&& p->left->rval != ND_NOSYMBOL
		&& SY_NAME(p->left->rval) == va_arg_name
		&& (argp = p->right)->op == CM
		&& (argp = argp->right)->op == FUNARG
		&& (argp = argp->left)->op == ICON
		&& TY_ISPTR(argp->type)
	    ) {
		va_arg_node = argp;
		va_arg_type = TY_DECREF(argp->type);
	    }
	}
	/*FALLTHRU*/
#endif
	case UNARY CALL:
	    /* Turn these into INCALL/UNARY INCALL if the class of the
	    ** left operand is SC_ASM.
	    */
#ifdef	IN_LINE
	    if (p->left->op == ICON && p->left->rval) {
		SX sid = p->left->rval;
		for ( ; SY_FLAGS(sid) & SY_TOMOVE; sid = SY_SAMEAS(sid)) {
		    if (SY_SAMEAS(sid) == SY_NOSYM)
			break;
		}
		if (SY_CLASS(sid) == SC_ASM)
		    p->op = p->op == CALL ? INCALL : UNARY INCALL;
	    }
#endif
	    p->name = ((ND1 *)p)->string;
	    break;

	case FLD:
	    ftype = p->left->type;	/* remember type of left child */
	    break;

#if 0
	    /* This should do something only if temporary regs are
	    /* built into the tree by machine-dependent actions.
	    ** (Since there aren't any-such in the ANSI C compiler,
	    ** don't bother.)
	    */
	case REG:
	    rbusy( p->rval, p->type );
#endif
	default:
	    p->name = (char *) 0;
    }

    if( ty != LTYPE ) cg_p2tree( p->left );
    if( ty == BITYPE ) cg_p2tree( p->right );

    /* Take care of random stuff or some special nodes. */
    switch( o ) {
	ND2 * l;
	TWORD ntype;

    case UPLUS:
	/* Remove UPLUS node and mark child as parenthesized. */
	l = p->left;
	*p = *l;
	p->strat |= PAREN;
	l->op = FREE;
	break;
    case FLD:
	/* All fields must be explicitly signed or unsigned.  Which
	** way a "plain"-typed field behaves is implementation defined.
	** Select an explicit type for them.  The actual type of the
	** field (as opposed to the type it promotes to) is in the
	** node below FLD.  Now that its type has been converted, do
	** the appropriate thing for "natural" types.
	*/
	ntype = p->left->type;		/* Usually use current type. */

	switch (TY_TYPE(ftype)) {
#ifdef	SIGNEDFIELDS			/* signed by default */
	case TY_CHAR:	ntype = T2_CHAR; break;
	case TY_SHORT:	ntype = T2_SHORT; break;
	case TY_ENUM:			/* behaves like int */
	case TY_INT:	ntype = T2_INT; break;
	case TY_LONG:	ntype = T2_LONG; break;
#else					/* unsigned by default */
	case TY_CHAR:	ntype = T2_UCHAR; break;
	case TY_SHORT:	ntype = T2_USHORT; break;
	case TY_ENUM:			/* behaves like int */
	case TY_INT:	ntype = T2_UNSIGNED; break;
	case TY_LONG:	ntype = T2_ULONG; break;
#endif
	}
	p->left->type = ntype;		/* Set type of addressing expr. to
					** reflect signed-ness correctly.
					*/
	break;
#ifdef VA_ARG
    case CALL:
	/* For varargs function, va_arg_node points to an ICON with
	** pointer type.  Change its type to the Pass 2 version of
	** the pointed-at type, save the size in lval and the
	** alignment in rval.
	*/
	if (va_arg_node) {
	    va_arg_node->type = cg_tconv(va_arg_type, 0);
	    va_arg_node->lval = TY_SIZE(va_arg_type);
	    va_arg_node->rval = TY_ALIGN(va_arg_type);
	}
	break;
#endif
    }
    return;
}


static char *
cg_statname(o)
CONVAL o;
/* Create print name for static variable # o. */
{
    char buf[20];

    (void) sprintf(buf, MYLABFMT, (MYLABTYPE) o);
    return( st_lookup(buf) );		/* squirrel string away and return */
}


void
cg_begf(sid)
SX sid;
/* Do function prologue for function whose symbol table entry is sid. */
{
    ND2 * new;
    TWORD rettype = cg_tconv(SY_TYPE(sid), 1);
					/* converted CG type for function */

#ifdef	FAT_ACOMP
    cg_infunc = 1;
#endif
    CG_SETLOCCTR(PROG);

    new = t2alloc();
    new->op = BEGF;
    new->name = (char *) 0;		/* no register information */
    cg_p2compile(new);
#ifdef FAT_ACOMP
    cg_q_type(ret_type, rettype);
#else
    ret_type(rettype);			/* tell CG the return type for the func. */
#endif

    cg_defnam(sid);

    new = t2alloc();
    new->op = ENTRY;
    new->type = rettype;
    cg_p2compile(new);

    /* Function entry may establish new starting point for autos,
    ** and current upper bound.
    */
#ifdef FAT_ACOMP
    cg_q_call(al_s_fcode);
#else
    al_s_fcode();
#endif
    return;
}


void
cg_endf(maxauto)
OFFSET maxauto;
/* Do function epilogue code.  For function-at-a-time compilation,
** generate code for the entire function, up to this point.
** maxauto is maximum extent of automatic variables.
*/
{
    ND2 * new;

#ifdef	FAT_ACOMP
    set_next_temp(maxauto);		/* put expression TEMPs after autos */
    cg_infunc = 0;			/* avoid queuing secondary calls */
    cg_q_gen();				/* generate accumulated stuff */
    sy_discard();			/* throw away out-of-scope symbols */
    maxauto = al_g_maxtemp();		/* new high-water mark */
#endif

    CG_SETLOCCTR(PROG);

    new = t2alloc();
    new->op = ENDF;
    new->lval = maxauto;
    new->name = al_g_maxreg();		/* list of registers used in func. */

    cg_p2compile(new);
    return;
}


void
cg_copyprm(sid)
SX sid;
/* Do semantic work required for a parameter.  sid is the symbol
** ID for the parameter.  There are two things to do:
**	1) For float arguments to functions without a prototype,
**		the actual is a double.  Need to copy, either in
**		place or elsewhere, possibly to a register.
**	2) For other parameters declared to be in registers,
**		try to allocate a register and move the actual.
**	3) For parameters that are smaller than int, the conceptual
**		truncation is handled magically by the offset
**		assignment calculations in CG.
** For any argument that actually DOES end up in a register,
** change the symbol's register number.
*/
{
    int regno = -1;			/* No register allocated. */
    T1WORD t = SY_TYPE(sid);
    TWORD cgtype = cg_tconv(t, 0);	/* Pass 2 equivalent type */
    TWORD cgetype = cgtype;		/* Effective parameter type (Pass 2) */
    int flags = 0;
    static char * name_dotdotdot = (char *) 0;
    static char * name_varargs = "";	/* different from any symbol string */

    if (name_dotdotdot == 0) {
	/* Look up name strings for "..." and alternate varargs name. */
	name_dotdotdot = st_lookup("...");
#ifdef	VA_ALIST
	name_varargs = st_lookup(VA_ALIST);
#endif
    }

    /* Prepare arguments for bindparam(). */
    if (SY_FLAGS(sid) & SY_ISDOUBLE)
	cgetype = T2_DOUBLE;

#ifndef FAT_ACOMP
    if (SY_FLAGS(sid) & SY_ISREG) {
	regno = al_reg(VPARAM, (OFFSET) SY_OFFSET(sid), t);
	if (regno >= 0)
	    SY_REGNO(sid) = regno;
    }
#endif

    if (SY_REGNO(sid) != SY_NOREG) {
	regno = SY_REGNO(sid);
	flags |= REGPARAM;
    }

#ifdef FAT_ACOMP
    if ((SY_FLAGS(sid) & SY_UAND) == 0)
	flags |= REGPARAM;
#endif

    if (TY_ISVOLATILE(t))
	flags |= VOLPARAM;
    if (SY_NAME(sid) == name_dotdotdot || SY_NAME(sid) == name_varargs)
	flags |= VARPARAM;

    bind_param(cgtype, cgetype, flags,
		(long) SY_OFFSET(sid), &regno, al_g_regs());
    
    /* If a register was allocated, update our register information. */
    if (regno >= 0) {
	al_regupdate(regno, tyreg(cgtype));
	SY_REGNO(sid) = regno;		/* remember parameter is in reg */
    }

    return;
}



ND1 *
cg_defstat(t)
T1WORD t;
/* Generate a static name for an object of type t,
** return a suitable NAME node corresponding to it.
** Location counter must be set suitably already.
*/
{
    ND2 * new;
    ND1 * p;
    int newlab = sm_genlab();

    new = t2alloc();
    new->op = DEFNAM;
    new->type = cg_align(t);		/* use type's alignment */
    new->lval = DEFINE;
    new->name = cg_statname((CONVAL) newlab);
    cg_p2compile(new);

    /* Build NAME node to return. */
    p = tr_newnode(NAME);
    p->type = t;
    p->lval = 0;
    p->rval = -newlab;
    return( p );
}

void
cg_bmove(sid, p)
SX sid;
ND1 * p;
/* Generate block move code to copy data starting at NAME node p
** to the location designated by sid.  CG requires UNARY & on
** both sides.
*/
{
    T1WORD t = SY_TYPE(sid);
    T1WORD ptrtype = ty_mkptrto(t);
    ND1 * bmovenode;
    ND1 * cmnode;
    ND1 * iconnode;
    unsigned int align = TY_ALIGN(t);

    /* Start by building , node for source, destination. */
    cmnode = tr_newnode(CM);
    cmnode->type = t;
    /* Need address of both sides.  Left is destination, right
    ** is source.
    */
    cmnode->left = tr_generic(UNARY AND, tr_symbol(sid), ptrtype);
    cmnode->right = tr_generic(UNARY AND, p, ptrtype);

    iconnode = tr_newnode(ICON);
    iconnode->type = TY_INT;
    iconnode->lval = TY_SIZE(t)/TY_SIZE(TY_CHAR);
    iconnode->rval = ND_NOSYMBOL;

    bmovenode = tr_newnode(BMOVE);
    if (TY_ISMBRVOLATILE(t))		/* if members are volatile, set flags */
	bmovenode->flags |= FF_ISVOL;
    bmovenode->left = iconnode;
    bmovenode->right = cmnode;
    /* Choose a type for the bmove node that has the same alignment
    ** as the type being moved.
    */
    if (align == ALINT)
	t = TY_INT;
    else if (align == ALSHORT)
	t = TY_SHORT;
    else if (align == ALLONG)
	t = TY_LONG;
#if 0
    else if (align == ALLDOUBLE)
	t = TY_LDOUBLE;
#endif
    else if (align == ALDOUBLE)
	t = TY_DOUBLE;
    else if (align == ALFLOAT)
	    t = TY_FLOAT;
    else
	t = TY_CHAR;

    bmovenode->type = t;

    sm_expr(bmovenode);
    return;
}


void
cg_deflab(l)
int l;
/* Generate code for text label l. */
{
    ND2 * lab;
    
    CG_SETLOCCTR(PROG);

    lab = t2alloc();
    lab->op = LABELOP;
    lab->label = l;

    cg_p2compile(lab);
    return;
}

void
cg_goto(l)
int l;
/* Generate an unconditional branch to text label l. */
{
    ND2 * jump;

    CG_SETLOCCTR(PROG);

    jump = t2alloc();
    jump->op = JUMP;
    jump->type = T2_INT;
    jump->label = l;

    cg_p2compile(jump);
    return;
}
#endif /* ifndef LINT */

/* Switch handling.
** For now, switch nodes are collected together on a singly-linked
** list.  Since it is LIFO, the BEGF node is buried in the list and
** must be located by walking the list.  However, the linked list
** simplifies handling nested switches, since we always generate
** code for the last one in, the most deeply nested.
**
** --- HACK ---
** Considerable cheating goes on here to use ->right pointers
** in UNARY or STANDALONE nodes to thread the list.
*/

static ND2 * swlist;			/* switch list header */

void
cg_swbeg(p)
ND1 * p;
{
    ND2 * swbeg = t2alloc();

    swbeg->op = SWBEG;
#ifdef FAT_ACOMP
    cg_q_nd2call(cg_p2tree, (ND2 *) p);	/* must be done after offsets have
					** been allocated for objects in tree
					*/
#else
    cg_p2tree((ND2 *) p);		/* tree will be a Pass 2 tree */
#endif
    swbeg->left = (ND2 *) p;
    swbeg->type = ((ND2 *) p)->type;
    /* CHEAT */
    swbeg->right = swlist;
    swlist = swbeg;
    return;
}


void
cg_swcase(val, l)
CONVAL val;
int l;
/* Add case to current switch.  Check for duplicates.  Keep cases sorted
** in reverse order (largest first).  Thus a search for duplicates can be
** cut short.
*/
{
    ND2 * swcase;
    register ND2 ** prev;		/* pointer to pointer to current node */

    for (prev = &swlist; (*prev)->op == SWCASE; prev = &((*prev)->right) ) {
	if (val < (*prev)->lval)
	    continue;			/* haven't found right place yet */

	if (val == (*prev)->lval) {
	    UERROR("duplicate case in switch: %d", val);
	    return;
	}
	break;				/* > case:  put case ahead of here */
    }

    /* Not a duplicate.  Allocate new node.  Put ahead of one at *prev. */
    swcase = t2alloc();
    swcase->op = SWCASE;
    swcase->lval = swcase->rval = val;
    swcase->label = l;
    /* CHEAT */
    swcase->right = *prev;
    *prev = swcase;

    return;
}


void
cg_swend(l)
int l;
/* Generate end-of-switch code.  l is the default label.  0 means
** no default (fall through).
** Walk the list, reversing pointers, until we get to the BEGF
** node, then walk the reversed pointers, generating code.
*/
{
    ND2 * swp;
    ND2 * prev;
    ND2 * next;

    ND2 * swend = t2alloc();

    swend->op = SWEND;
    swend->lval = (l == 0 ? -1 : l);	/* CG expects -1 for no default */

    /* Reverse the pointers up to the SWBEG node, update the
    ** switch linked list.
    */
    prev = ND2NIL;
    swp = swlist;
    while (swp->op != SWBEG) {
	ND2 * temp = swp->right;
	swp->right = prev;
	prev = swp;
	swp = temp;
    }
    /* swp points at SWBEG node */
    swlist = swp->right;		/* switch list is what remains */
    swp->right = prev;			/* link on SWCASE nodes */
    do {				/* generate code for SWBEG, SWCASEs */
	next = swp->right;
	cg_p2compile(swp);
    } while( (swp = next) != ND2NIL );

    cg_p2compile(swend);		/* now generate end-of-switch */
    return;
}

/* back to ignoring cgstuff for lint */
#ifndef LINT
void
cg_filename(fname)
char * fname;
/* Set new filename fname for CG. */
{
    /* Save new filename string if there's any likelihood we'll
    ** use it.  For our purposes, get the file's basename.
    */
    if (cg_outhdr) {
	char * start = strrchr(fname, '/');
	if (start++ == NULL)		/* Want portion after / */
	    start = fname;
	cg_fstring = st_lookup(start);
    }
    return;
}


void
cg_ident(i)
char * i;
/* Output #ident string.  Force out filename string, if any, first. */
{
    if (cg_outhdr)
	cg_begfile();
    fprintf(outfile, "	%s	%s\n", IDENTSTR, i);
    return;
}


void
cg_begfile()
/* Do start of file stuff, namely, produce .file pseudo-op.
** If a version string is defined (for the assembler) print
** it, too.
*/
{
    if (cg_outhdr) {
#ifdef	NO_DOT_FILE
	fprintf(outfile, "	%s	\"%s\"\n", NO_DOT_FILE, cg_fstring);
#else
	fprintf(outfile, "	.file	\"%s\"\n", cg_fstring);
#endif
#ifdef	COMPVERS
	fprintf(outfile, "	.version	\"%s\"\n", COMPVERS);
#endif
	cg_outhdr = 0;
    }
    return;
}


void
cg_asmold(p)
ND1 * p;
/* Write string node to output file as asm string.  Since CG doesn't
** supply trailing \n, we must.  Bracket output, if necessary, with
** implementation-specific bookends.
*/
{
    if (p->op != STRING)
	cerror("confused cg_asmold()");
    
    /* Must be regular string literal. */
    if ((p->rval & TR_ST_WIDE) != 0)
	UERROR("asm() argument must be normal string literal");

    /* CG can't handle string with embedded NUL. */
    else if (strlen(p->string) != p->lval)
	UERROR("embedded NUL not permitted in asm()");

#ifdef	ASM_COMMENT
    cg_copyasm(ASM_COMMENT);
#endif

    cg_copyasm(p->string);
    t1free(p);

#ifdef	ASM_END
    cg_copyasm(ASM_END);
#endif

    return;
}


static void
cg_copyasm(s)
char * s;
/* Generate COPYASM node for string s, follow with newline. */
{
    ND2 * new = t2alloc();
    new->op = COPYASM;
    new->type = T2_VOID;
    new->name = s;
    cg_p2compile(new);

    new = t2alloc();
    new->op = COPYASM;
    new->type = T2_VOID;
    new->name = "\n";
    cg_p2compile(new);
    return;
}


void
cg_eof(flag)
int flag;
/* Do whatever needs to be done at end of file.  flag is
** C_TOKEN if there were any tokens, C_NOTOKEN otherwise.
** That means issuing a warning if no tokens were seen
** (an empty input file).  Also, we must be sure that all
** assembly files have the beginning of file stuff, so
** if we haven't done so already, do what needs to be
** done at the beginning of an output file.
*/
{
    if (flag == C_NOTOKEN)
	WERROR("empty translation unit");
    if (cg_outhdr)
	cg_begfile();
    if (cg_ldused) {
	/* Do something to force error at link time if
	** long double mismatch.
	*/
	CG_LDUSED();
    }
    return;
}

void
cg_profile()
/* Tell CG to generate functions with profiling information. */
{
    profile(1);
    return;
}
#endif
/* endif ifdef LINT */


void
cg_ldcheck()
/* Check for validity of long double type and issue error
** message as appropriate:  for strictly conforming version
** we can only issue a warning that things are treated as
** double.  For other versions, issue an error message to
** prevent further compilation.  In either case, just issue
** one message.
** Preserve errno, since this routine gets called from places
** that may expect errno to have been set (or clear).
*/
{
    if (!cg_ldused) {
	int olderrno = errno;

	cg_ldused = 1;		/* we've complained once */
	/* Hard error if CI4.1-compatible or ANSI compatible
	** modes; warning if "strictly conforming".
	*/
	if (version & V_STD_C)
	    WERROR("\"long double\" not yet supported; using \"double\"");
	else
	    UERROR("\"long double\" not yet supported");
	errno = olderrno;
    }
    return;
}

OFFSET
cg_off_conv(space, off, from, to)
int space;
OFFSET off;
T1WORD from;
T1WORD to;
/* Mirror CG's off_conv(), but with pass 1 types.  Convert
** offset "off" for address space "space" from type "from"
** to type "to".
*/
{
    TWORD p2from = cg_tconv(from, 1);
    TWORD p2to = cg_tconv(to,1);
    off = off_conv(space, off, p2from, p2to);
    return( off );
}


BITOFF
al_struct(poffset, t, isfield, fsize)
BITOFF * poffset;
T1WORD t;
int isfield;
BITOFF fsize;
/* Allocate space for a structure member and return the bit
** number of the first bit for it.  t is the type of the
** member; isfield says whether to allocate a bitfield;
** fsize is the bitfield size if isfield is non-zero;
** poffset points to the starting bit offset, which gets
** updated to reflect the allocation of the new member.
*/
{
    int size = TY_SIZE( t );
    int align = TY_ALIGN( t );
    BITOFF noffset = *poffset;

#ifdef	PACK_PRAGMA
    /* Constrain alignment boundary of non-bitfield members. */
    if (!isfield && Pack_align != 0 && align > Pack_align)
	align = Pack_align;
#endif

    /* Update alignment for normal member or if zero-length
    ** field, or if not enough room for new field in current
    ** storage unit.
    */
    if (!isfield || fsize == 0 || (fsize + (noffset % align)) > size)
	noffset = AL_ALIGN(noffset, align);
    
    /* noffset contains the beginning of new member; we'll
    ** write back the update end bit number
    */
    *poffset = noffset + (isfield ? fsize : size);
    return( noffset );
}


#ifdef	OPTIM_SUPPORT

/* The following routines provide support for peephole
** optimizers.  More specifically the routines here pass information
** to CG, and CG generates information in the assembly code output.
**
** The interface contains two routines:
**	os_loop()	to generate information about loops
**	os_symbol()	to generate information about a symbol
**	os_uand()	to denote that the address of an object was taken
**
** --------	NOTE	--------
** This version assumes statement at a time compilation:  the calls
** here result in direct output (more or less).  For function at a
** time compilation we would have to store the string in temporary
** string space, create COPY nodes, and call cg_ecode().
**
** Another assumption (for speed right now) is that only parameters and
** top level automatics are of interest.
*/

void
os_loop(code)
int code;
/* Generate the appropriate optimizer information for the code.
** Typical codes are OI_LBODY, OI_LSTART, etc.
*/
{
    char * s = oi_loop(code);
    (void) fputs(s, outfile);
    return;
}

/* Do common setup for os_symbol, os_uand:  determine
** the correct interface storage class and set os_class.
** Discard symbols of no interest.  Build a node that's
** suitable to pass to one of CG's interface routines, or ND1NIL.
** If foralias is non-zero, enable some cases that don't apply
** for os_symbol():  defined struct/union parameters, "...".
*/

static int os_class;

static ND2 *
os_setup(sid, foralias)
SX sid;
int foralias;
{
    T1WORD t = SY_TYPE(sid);
    ND2 * name;

    /*TEMPORARY:  avoid triggering long double error */
    if (TY_EQTYPE(t, TY_LDOUBLE))
	return( ND2NIL );

    /* Suppress checks here if foralias and name is parameter. */
    
    if (! (foralias && SY_CLASS(sid) == SC_PARAM)) {
	if (!TY_ISSCALAR(t) || TY_ISVOLATILE(t))
	    return( ND2NIL );
    }

    /* Avoid incomplete enum -- we don't know its size. */
    if (TY_TYPE(t) == TY_ENUM && ! TY_HASLIST(t))
	return( ND2NIL );

    /* Discard things already in registers. */
    if (SY_REGNO(sid) != SY_NOREG)
	return( ND2NIL );

    /* Choose optimizer interface storage class for symbol. */
    switch( SY_CLASS(sid)) {
    case SC_PARAM:
	/* Parameters are special case.
	** If not for aliasing, checks for volatile, scalar have
	** already been done.  Generate symbol information.
	** For aliasing information, parameter need not be scalar,
	** but it must have been defined.  (Otherwise we'd generate
	** information for nested prototype declarations.)
	** "..." is a special case parameter:  it's not SY_DEFINED,
	** but we do generate aliasing information.
	*/
	if (   (SY_FLAGS(sid) & SY_DEFINED) != 0
	    || (foralias && SY_NAME(sid)[0] == '.')	/* "..." */
	) {
	    os_class = OI_PARAM;
	    break;
	}
	return( ND2NIL );
    case SC_AUTO:
	if (SY_LEVEL(sid) > SL_INFUNC)
	    return( ND2NIL );		/* ignore non-top-level autos */
	os_class = OI_AUTO;
	break;
    case SC_EXTERN:
	os_class =
	    (SY_FLAGS(sid) & (SY_DEFINED|SY_TENTATIVE)) ? OI_EXTDEF : OI_EXTERN;
	break;
    case SC_STATIC:
	os_class = SY_LEVEL(sid) == SL_EXTERN ? OI_STATEXT : OI_STATLOC;
	break;
    default:
	return( ND2NIL );		/* other classes are of no interest */
    }

    name = (ND2 *) tr_symbol(sid);	/* Build a name node. */
    cg_p2tree(name);			/* convert type for CG */
    return( name );
}


void
os_symbol(sid)
SX sid;
/* Generate appropriate optimizer information for symbol whose
** index is sid.  Pass out a tree node and the symbol's storage
** class, size (mostly for struct/union) in bits, and level.
*/
{
    ND2 * name = os_setup(sid, 0);	/* not for aliasing information */

    if (name) {
	char * s = oi_symbol((NODE *) name, os_class);
	(void) fputs(s, outfile);
	name->op = FREE;
    }

    /* Take conservative approach on parameter struct/unions:
    ** mark them as having their address taken.  Only worry about
    ** parameters that we actually output information for.
    */
    if (SY_CLASS(sid) == SC_PARAM && TY_ISSU(SY_TYPE(sid))) {
	char * s;
	name = os_setup(sid, 1);	/* for aliasing information */
	if (name) {
	    s = oi_alias((NODE *) name);
	    (void) fputs(s, outfile);
	    name->op = FREE;
	}
    }
    return;
}


void
os_uand(sid)
SX sid;
/* Report that the address has been taken of a symbol. */
{
    ND2 * name = os_setup(sid, 1);	/* this is for aliasing */

    if (name) {
	char * s = oi_alias((NODE *) name);
	(void) fputs(s, outfile);
	name->op = FREE;
    }
    return;
}

#endif	/* OPTIM_SUPPORT */

/* Support for function-at-a-time compilation. */

#ifdef	FAT_ACOMP

/* Operations in CG queue. */

#define	CGQ_EXPR_ND1	1	/* ND1 expression to optimize and compile */
#define	CGQ_EXPR_ND2	2	/* ND2 expression to compile */
#define	CGQ_PUTS	3	/* string to output */
#define	CGQ_CALL	4	/* f(void) */
#define	CGQ_CALL_INT	5	/* f(int) */
#define	CGQ_CALL_SID	6	/* f(SX) */
#define	CGQ_CALL_STR	7	/* f(char *) */
#define	CGQ_CALL_TYPE	8	/* f(T1WORD) */
#define	CGQ_CALL_ND2	9	/* f(ND2 *) */

#ifndef	INI_CGQ
#define	INI_CGQ	200
#endif

typedef union {
    int cgq_int;
    ND1 * cgq_nd1;
    ND2 * cgq_nd2;
    SX cgq_sid;
    char * cgq_str;
    T1WORD cgq_type;
} cgq_arg_t;

typedef struct {
    int cgq_op;		/* queued operation (one of above) */
    void (*cgq_func)();	/* function to call */
    cgq_arg_t cgq_arg;	/* argument to function */
    int cgq_ln;		/* parsing line number */
    int cgq_dbln;	/* debug info line number */
} cgq_t;

static void cg_q_do();

#define	CGQ_NOFUNC ((void (*)()) 0)

TABLE(Static, td_cgq, static, cgq_init,
		cgq_t, INI_CGQ, 0, "cgstuff queued operations");

#ifndef	INI_CGPRINTBUF
#define	INI_CGPRINTBUF 100
#endif

TABLE(Static, cg_printbuf, static, cg_printbuf_init,
		char, INI_CGPRINTBUF, 0, "cg printf buffer");

static void
cg_append(s,len)
const char * s;
int len;
/* Append len characters to cg_printbuf. */
{
    if (cg_printbuf.td_used + len >= cg_printbuf.td_allo)
	td_enlarge(&cg_printbuf, len);
    
    memcpy((myVOID *) &TD_ELEM(cg_printbuf, char, cg_printbuf.td_used),
		(myVOID *) s, len);
    TD_USED(cg_printbuf) += len;
    TD_CHKMAX(cg_printbuf);
    return;
}

/* This routine is a VERY partial simulation of printf,
** and it doesn't pretend to be very smart about what
** it does.  It can handle the following formats:
**	%s
**	%.ns
**	%.*s
**	%d
*/

void
#ifdef __STDC__
cg_printf(const char *fmt, ...) {
    va_list args;
    va_start(args,fmt);
#else	/* non-ANSI case */
cg_printf(fmt, va_alist)
char *fmt;
va_dcl
{
    va_list args;
    va_start(args);
#endif

    TD_USED(cg_printbuf) = 0;

    for (;;) {
	char *fmtpos = strchr(fmt, '%');
	int len = (fmtpos ? fmtpos-fmt : strlen(fmt));
	int hadprec;
	int prec;

	cg_append(fmt,len);
	if (fmtpos == 0)
	    break;
	
	/* Had a format character. */
	fmt = fmtpos+1;
	hadprec = 0;
	if (*fmt == '.') {
	    hadprec = 1;
	    ++fmt;
	    if (*fmt == '*') {
		++fmt;
		prec = va_arg(args, int);
	    }
	    else {
		prec = 0;
		while (isdigit(*fmt)) {
		    prec = prec*10 + (*fmt - '0');
		    ++fmt;
		}
	    }
	}
	switch (*fmt) {
	    char * s;
	    char intbuf[20];
	    int intval;
	case 's':
	    s = va_arg(args, char *);
	    len = strlen(s);
	    if (hadprec && len > prec)
		len = prec;
	    cg_append(s,len);
	    break;
	case 'd':
	    intval = va_arg(args, int);
	    len = sprintf(intbuf, "%d", intval);
	    cg_append(intbuf,len);
	    break;
	default:
	    cerror("cg_printf():  bad fmt char %c", *fmt);
	}
	++fmt;
    }

    cg_append("", 1);
    cg_q_puts(
	st_nlookup(&TD_ELEM(cg_printbuf, char, 0),
		  (unsigned int) TD_USED(cg_printbuf)
		  )
	  );
    return;
}


static void
cg_q(op, func, arg)
int op;
void (*func)();
cgq_arg_t arg;
/* Queue operation for later if within a function. */
{
    cgq_t *qp;
    extern int elineno;

    if (cg_infunc) {
	TD_NEED1(td_cgq);
	qp = &TD_ELEM(td_cgq, cgq_t, TD_USED(td_cgq));
	qp->cgq_op = op;
	qp->cgq_func = func;
	qp->cgq_arg = arg;
	qp->cgq_ln = elineno;
	qp->cgq_dbln = db_curline;
	++TD_USED(td_cgq);
	TD_CHKMAX(td_cgq);
    }
    else
	cg_q_do(op, func, arg);
    return;
}


static void
cg_q_gen()
/* Flush the CG queue of pending operations. */
{
    int i;
    extern int elineno;
    int save_ln = elineno;
    int save_dbln = db_curline;

    for (i = 0; i < TD_USED(td_cgq); ++i) {
	cgq_t *qp = &TD_ELEM(td_cgq, cgq_t, i);

	elineno = qp->cgq_ln;
	db_curline = qp->cgq_dbln;
	cg_q_do(qp->cgq_op, qp->cgq_func, qp->cgq_arg);
    }
    TD_USED(td_cgq) = 0;
    elineno = save_ln;
    db_curline = save_dbln;
    return;
}


static void
cg_q_do(op, func, arg)
int op;
void (*func)();
cgq_arg_t arg;
/* Do a queued CG operation. */
{
    switch( op ){
    case CGQ_CALL:	(*func)(); break;
    case CGQ_CALL_SID:	(*func)(arg.cgq_sid); break;
    case CGQ_CALL_INT:	(*func)(arg.cgq_int); break;
    case CGQ_CALL_ND2:	(*func)(arg.cgq_nd2); break;
    case CGQ_CALL_STR:	(*func)(arg.cgq_str); break;
    case CGQ_CALL_TYPE:	(*func)(arg.cgq_type); break;
    case CGQ_PUTS:	fputs(arg.cgq_str, stdout); break;
    case CGQ_EXPR_ND1:	cg_ecode(arg.cgq_nd1); break;
    case CGQ_EXPR_ND2:	p2compile((NODE *) arg.cgq_nd2); break;
    default:
	cerror("cg_q_do():  bad op %d\n", op);
    }
    return;
}


void
cg_q_int(f, i)
void (*f)();
int i;
/* Queue function call with int argument. */
{
    cgq_arg_t u;
    u.cgq_int = i;
    cg_q(CGQ_CALL_INT, f, u);
    return;
}


void
cg_q_sid(f, sid)
void (*f)();
SX sid;
/* Queue function call with symbol ID argument. */
{
    cgq_arg_t u;
    u.cgq_sid = sid;
    cg_q(CGQ_CALL_SID, f, u);
    return;
}


void
cg_q_str(f, s)
void (*f)();
char *s;
/* Queue function call with string argument. */
{
    cgq_arg_t u;
    u.cgq_str = s;
    cg_q(CGQ_CALL_STR, f, u);
    return;
}


void
cg_q_type(f, t)
void (*f)();
T1WORD t;
/* Queue function call with type word argument. */
{
    cgq_arg_t u;
    u.cgq_type = t;
    cg_q(CGQ_CALL_TYPE, f, u);
    return;
}


void
cg_q_call(f)
void (*f)();
/* Queue function call with no argument. */
{
    cgq_arg_t u;			/* dummy argument */
    cg_q(CGQ_CALL, f, u);
    return;
}


static void
cg_q_puts(s)
char * s;
/* Queue request to output string. */
{
    cgq_arg_t u;
    u.cgq_str = s;
    cg_q(CGQ_PUTS, CGQ_NOFUNC, u);
    return;
}

void
cg_q_nd1(p)
ND1 * p;
/* Queue Pass 1 node for later cg_ecode(). */
{
    cgq_arg_t u;
    u.cgq_nd1 = p;
    cg_q(CGQ_EXPR_ND1, CGQ_NOFUNC, u);
    return;
}


static void
cg_q_nd2(p)
ND2 * p;
/* Queue Pass 2 node for later p2compile(). */
{
    cgq_arg_t u;
    u.cgq_nd2 = p;
    cg_q(CGQ_EXPR_ND2, CGQ_NOFUNC, u);
    return;
}

static void
cg_q_nd2call(f, p)
void (*f)();
ND2 *p;
/* Queue call to routine with Pass 2 node as argument. */
{
    cgq_arg_t u;
    u.cgq_nd2 = p;
    cg_q(CGQ_CALL_ND2, f, u);
    return;
}

#endif	/* def FAT_ACOMP */
