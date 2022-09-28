/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/types.c	52.50"
/* types.c */

#include "p1.h"

/* This module contains the code to support Pass 1 type
** processing.  Types consist, essentially of base types
** and linked lists of modifiers.  Types are represented as a
** number to the outside world.  Within the types package the
** number is the offset in the type table, which sizes itself
** dynamically.  The TT macro (TT(i,st)) references the i-th
** element in the table as a pointer to a struct st.
**
** Although the table is, conceptually, a linked list of type
** type nodes, the nodes are comprised, in fact, of consecutive
** scalar values.  Their particular meanings are context-sensitive.
** See types.h for the layout of the type "nodes".
*/

#ifndef	INI_TYPETAB
#define	INI_TYPETAB 250		/* initial size of variable type table */
#endif

/* This table contains the characteristics of the generic types.
** Obviously the entries must agree with the definitions of the
** respective type numbers.
*/

static ty_typetab in_typetab[TY_NBASE+INI_TYPETAB] = {
	0,	/* no type */
	TY_CHAR | TY_TPLAIN,
	TY_UCHAR | TY_TUNSIGNED,
	TY_SCHAR | TY_TSIGNED,
	TY_SHORT | TY_TPLAIN,
	TY_USHORT | TY_TUNSIGNED,
	TY_SSHORT | TY_TSIGNED,
	TY_INT | TY_TPLAIN,
	TY_UINT | TY_TUNSIGNED,
	TY_SINT | TY_TSIGNED,
	TY_INT | TY_TPLAIN,		/* TY_AINT */
	TY_UINT | TY_TUNSIGNED,		/* TY_AUINT */
	TY_LONG | TY_TPLAIN,
	TY_ULONG | TY_TUNSIGNED,
	TY_SLONG | TY_TSIGNED,
	TY_LLONG | TY_TPLAIN,
	TY_ULLONG | TY_TUNSIGNED,
	TY_SLLONG | TY_TSIGNED,
	TY_ENUM | TY_TENUM|TY_TPLAIN,
	TY_FLOAT | TY_TFP,
	TY_DOUBLE | TY_TFP,
	TY_LDOUBLE | TY_TFP,
	TY_VOID | 0,
	TY_STRUCT | TY_TSTRUCT,
	TY_UNION | TY_TUNION,
	TY_PTR | TY_TPTR,
	TY_ARY | TY_TARRAY,
	TY_FUN | TY_TFUNC,
};


/* Table descriptor is always external -- various macros diddle
** with the table.  Initial table has already been defined.  The
** second "extern" refers to it.
*/
TABLE(/*extern*/, typetab, extern, in_typetab,
		ty_typetab, (TY_NBASE+INI_TYPETAB), 0, "type table");

#define	TTNEED(n) TD_NEED(typetab, (n))

/* Sizes of "structs" in terms of typetab words. */
#define TT_PTLEN (sizeof(ts_ptr)/sizeof(ty_typetab))		/* pointer */
#define	TT_ARLEN (sizeof(ts_ary)/sizeof(ty_typetab))		/* array */
#define	TT_FNLEN (sizeof(ts_fun)/sizeof(ty_typetab))		/* function */
#define	TT_SUELEN (sizeof(ts_sue)/sizeof(ty_typetab))		/* s/u/e */
#define	TT_BASELEN (sizeof(ts_base)/sizeof(ty_typetab))		/* base type */
#define	TT_QUALLEN (sizeof(ts_qualtype)/sizeof(ty_typetab))	/* qualifiers */

/* These types are not simple:  they have no representation
** within the initial types array.
*/
#define	TY_NOTSIMPLE (TY_TPTR|TY_TSTRUCT|TY_TUNION|TY_TENUM|TY_TARRAY|TY_TFUNC)


/* This macro obtains the name string for a symbol.  Because of
** symbol scoping, sometimes the name (for a structure tag, for
** example) can go out of scope.  The name field gets zeroed.
** Convert a bad name string to "<FREED>".
*/
#define	TY_SYNAME(sid) \
	(SY_NAME((SX)(sid)) == SY_NOSYM ? "<FREED>" : SY_NAME((SX)(sid)))

/* Macros to get/set number of parameters for function. */
/* Set number of parameters from actual number + "..." flag */

#define	SETNP(np,isddd)	(isddd ? -(np)-1 : (np)+1);
/* Get just number of parameters. */
#define	GETNP(n) ((long)(n) > 0 ? (long)(n)-1 : (long) -(n)-1)
/* Check ... flag. */
#define	ISDDD(n) ((long) (n) < 0)

/* Skip any qualifiers */
#define	SKIPQUAL(t) if (TY_ISQUAL(t)) t = TT(t,ts_qualtype)->ts_unqual

T1WORD ty_voidstar;			/* static version of void * */
T1WORD ty_frint;			/* == function-returning-int */

/* TT_LASTBASE is the number of basic types. */
#define	TT_LASTBASE	28

static int ty_chkparam();
static BITOFF ty_sizesub();
static void ty_setasub();
static T1WORD ty_newtype();

/* This table contains the sizes for basic types.
** A zero entry means the basic type has no inherent size.
*/

#ifndef LINT
/* Lint needs to be able to change this table with the -p option. */
const
#endif
BITOFF _ty_size[TY_NBASE] = {
	0,		/* no type */
	SZCHAR,		/* TY_CHAR */
	SZCHAR,		/* TY_UCHAR */
	SZCHAR,		/* TY_SCHAR */
	SZSHORT,	/* TY_SHORT */
	SZSHORT,	/* TY_USHORT */
	SZSHORT,	/* TY_SSHORT */
	SZINT,		/* TY_INT */
	SZINT,		/* TY_UINT */
	SZINT,		/* TY_SINT */
	SZINT,		/* TY_AINT */
	SZINT,		/* TY_AUINT */
	SZLONG,		/* TY_LONG */
	SZLONG,		/* TY_ULONG */
	SZLONG,		/* TY_SLONG */
	0 /*SZLLONG*/,	/* TY_LLONG */
	0 /*SZLLONG*/,	/* TY_ULLONG */
	0 /*SZLLONG*/,	/* TY_SLLONG */
	0,		/* TY_ENUM */
	SZFLOAT,	/* TY_FLOAT */
	SZDOUBLE,	/* TY_DOUBLE */
	SZLDOUBLE,	/* TY_LDOUBLE */
	0,		/* TY_VOID */
	0,		/* TY_STRUCT */
	0,		/* TY_UNION */
	SZPOINT,	/* TY_PTR */
	0,		/* TY_ARY */
	0,		/* TY_FUN */
};

/* This table contains the alignments for basic types.  A value
** of zero means the type has no inherent alignment.
*/
#ifndef LINT
/* Lint also needs to be able to change this table with the -p option. */
static const
#else
static
#endif
BITOFF _ty_align[TY_NBASE] = {
	0,	/* no type */
	ALCHAR,	/* TY_CHAR */
	ALCHAR,	/* TY_UCHAR */
	ALCHAR,	/* TY_SCHAR */
	ALSHORT,	/* TY_SHORT */
	ALSHORT,	/* TY_USHORT */
	ALSHORT,	/* TY_SSHORT */
	ALINT,	/* TY_INT */
	ALINT,	/* TY_UINT */
	ALINT,	/* TY_SINT */
	ALINT,	/* TY_AINT */
	ALINT,	/* TY_AUINT */
	ALLONG,	/* TY_LONG */
	ALLONG,	/* TY_ULONG */
	ALLONG,	/* TY_SLONG */
	0 /*ALLLONG*/,	/* TY_LLONG */
	0 /*ALLLONG*/,	/* TY_ULLONG */
	0 /*ALLLONG*/,	/* TY_SLLONG */
	0,	/* TY_ENUM */
	ALFLOAT,	/* TY_FLOAT */
	ALDOUBLE,	/* TY_DOUBLE */
	ALLDOUBLE,	/* TY_LDOUBLE */
	0,	/* TY_VOID */
	0,	/* TY_STRUCT */
	0,	/* TY_UNION */
	ALPOINT,	/* TY_PTR */
	0,	/* TY_ARY */
	0,	/* TY_FUN */
};


#ifdef LINT
/*
** This is called when lint sees the -p option.
**
** Lint passes an array of max structs with the following members:
**      ty : generic type
**      size : size of ty
**      align : align of ty
**
** This method lets lint set up the portable size and alignment arrays
** in any order, without worrying about the types package changing.
*/
void
lntt_init(lnsizes, max)
PORTSIZE lnsizes[];
int max;
{
    int i;
    for (i=0;i<max;i++) {
        _ty_size[lnsizes[i].ty] = lnsizes[i].size;
        _ty_align[lnsizes[i].ty] = lnsizes[i].align;
    }
}
#endif


void
tt_init()
{
    TD_USED(typetab) = TY_NBASE;	/* set amount of table used so far */
    ty_frint = ty_mkfunc(TY_INT);	/* initialize type */
    ty_voidstar = ty_mkptrto(TY_VOID);
    return;
}

static T1WORD
ty_newtype(spec, n)
ty_typetab spec;
int n;
/* Allocate n type table words, set specifier word (presumed to be the
** first word) with spec.
*/
{
    T1WORD newt = TD_USED(typetab);
    TTNEED(n);
    TD_USED(typetab) += n;
    TD_CHKMAX(typetab);
    TT(newt, ts_base)->ts_spec = spec;
    return( newt );
}


#ifndef	NODBG

void
tt_print(from,to)
int from;
int to;
/* Debug printing:  examine specific type table entry words. */
{
    while (from <= to) {
	DPRINTF("%4d\t%#x\t%d\n", from, TE(from), TE(from));
	++from;
    }
    return;
}

void
ty_print(t)
T1WORD t;
/* Pretty-print a type name. */
{
    static void ty_pfun(), ty_psoru(), ty_penu();
    /* This table contains the print-names for basic types.
    ** Complicated types are handled separately.
    */
    static const char * const _ty_pname[TY_NBASE] = {
	0,		/* no type */
	"char",		/* TY_CHAR */
	"uchar",	/* TY_UCHAR */
	"schar",	/* TY_SCHAR */
	"short",	/* TY_SHORT */
	"ushort",	/* TY_USHORT */
	"sshort",	/* TY_SSHORT */
	"int",		/* TY_INT */
	"uint",		/* TY_UINT */
	"sint",		/* TY_SINT */
	"aint",		/* TY_AINT */
	"auint",	/* TY_AUINT */
	"long",		/* TY_LONG */
	"ulong",	/* TY_ULONG */
	"slong",	/* TY_SLONG */
	"llong",	/* TY_LLONG */
	"ullong",	/* TY_ULLONG */
	"sllong",	/* TY_SLLONG */
	0,		/* TY_ENUM */
	"float",	/* TY_FLOAT */
	"double",	/* TY_DOUBLE */
	"ldouble",	/* TY_LDOUBLE */
	"void",		/* TY_VOID */
	0,		/* TY_STRUCT */
	0,		/* TY_UNION */
	0,		/* TY_PTR */
	0,		/* TY_ARY */
	0,		/* TY_FUN */
};

    for (;;) {
	ts_base * tp = TT(t, ts_base);

	if (TY_ISQUAL(t)) {
	    if (tp->ts_spec & TY_CONST)
		DPRINTF("const ");
	    if (tp->ts_spec & TY_VOLATILE)
		DPRINTF("vol ");
	    if (tp->ts_spec & TY_NOALIAS)
		DPRINTF("noalias ");
	    if ((tp->ts_spec & TY_TPTR) == 0)	/* pointer has no modifier */
		SKIPQUAL(t);
	}

	/* Simple types will have type numbers within the initial
	** array, and we know their names already (or should).
	*/
	if (t < TY_NBASE) {
	    const char * s = _ty_pname[t];
	    DPRINTF(s ? s : (const char *) "BAD BASETYPE");
	    return;			/* done */
	}

	/* Do case analysis on generic type. */
	switch( tp->ts_spec & TY_BTMASK ) {
	case TY_PTR:
	    DPRINTF("ptr to ");
	    break;
	
	case TY_ARY:
	    DPRINTF("ary[%s%ld] of ",
		(tp->ts_spec & TY_THIDDEN) ? "#" : "",
		(long) ((ts_ary *) tp)->ts_nelem);
	    break;
	
	case TY_FUN:
	    ty_pfun(t);
	    break;
	
	case TY_STRUCT:
	    ty_psoru("struct", t); return;
	
	case TY_UNION:
	    ty_psoru("union", t); return;
	
	case TY_ENUM:
	    ty_penu(t); return;
	
	default:
	    DPRINTF("BAD TYPE %d, (%#x)", t, TE(t)); return;
	}

	/* Follow generic derived type of ptr/ary/fun/qual. */
	t = TT(t, ts_generic)->ts_type;
    }
#if 0
    cerror("shouldn't get here");
#endif
}


static void
ty_pfun(t)
T1WORD t;
/* Print function and parameters for function-returning type t. */
{
    ts_fun * tp = TT(t, ts_fun);
    int plist = tp->ts_prmlist;

    DPRINTF("ftn(");

    if ((plist = tp->ts_prmlist) != 0) {	/* number of params known */
	int nparam = TE(plist);			/* count word for prototype */
	int dotdotdot = ISDDD(nparam);		/* ... flag */

	if (tp->ts_spec & TY_THIDDEN)
	    DPRINTF("#");			/* mark hidden information */
	for (nparam = GETNP(nparam); nparam > 0; --nparam) {
	    ++plist;
	    ty_print((T1WORD) TE(plist));	/* print this param */
	    if (nparam > 1 || dotdotdot)
		DPRINTF(",");
	}
	if (dotdotdot)
	    DPRINTF("...");
	else if (plist == tp->ts_prmlist)
	    DPRINTF("void");
    }
    DPRINTF(") ret'g ");
    return;
}


static void
ty_psoru(s, t)
char * s;
T1WORD t;
/* Print struct/union innards:  types and names of members.
** To prevent infinite recursion on recursive structures,
** print only one level of member types.  That is, for higher
** order levels, just print s/u tag.
*/
{
    static int level = 0;
    ts_sue * tp = TT(t, ts_sue);
    ty_typetab mlist;

    ++level;
    DPRINTF("%s %s", s, TY_SYNAME((SX)tp->ts_tag));
    if ((tp->ts_spec & TY_TMQUAL) != 0) {
	DPRINTF(" (");
	if (tp->ts_spec & TY_TMCONST)
	    DPRINTF("c");
	if (tp->ts_spec & TY_TMVOLATILE)
	    DPRINTF("v");
	if (tp->ts_spec & TY_TMNOALIAS)
	    DPRINTF("n");
	DPRINTF(")");
    }
    DPRINTF(" {");
    if (level <= 1) {
	for (mlist = tp->ts_mbrlist; mlist < tp->ts_lastmbr; ++mlist) {
	    if (mlist != tp->ts_mbrlist)	/* no , on first mbr */
		DPRINTF(", ");
	    ty_print(SY_TYPE((SX)TE(mlist)));
	    DPRINTF(" %s", TY_SYNAME(TE(mlist)));
	}
    }
    DPRINTF("}");
    --level;
    return;
}


static void
ty_penu(t)
T1WORD t;
/* Print enum members. */
{
    ts_sue * tp = TT(t, ts_sue);
    ty_typetab mlist;

    DPRINTF("enum %s {", TY_SYNAME((SX)tp->ts_tag));
    for (mlist = tp->ts_mbrlist; mlist < tp->ts_lastmbr; ++mlist) {
	if (mlist != tp->ts_mbrlist)
	    DPRINTF(", ");			/* no , on first mbr */
	DPRINTF("%s(%d)", TY_SYNAME((SX)TE(mlist)), SY_OFFSET((SX)TE(mlist)));
    }
    DPRINTF("}");
    return;
}

#endif	/* ndef NODBG */


T1WORD
ty_mkptrto(t)
T1WORD t;
/* Make a pointer to type (word) t.  We can guarantee that two
** pointers to the same type have the same type number by recording
** the new type number in the type's ty_ptr field.  For simple types,
** store the number in the table below.
*/
{
    static ty_typetab ty_ptrs[TY_NBASE]; /* pointers to simple types */
    T1WORD newt;
    
    /* Old type is in type if type not simple, else in local table. */
    newt = (t < TY_NBASE) ? ty_ptrs[t] : TT(t,ts_generic)->ts_ptr;

    if (newt == 0) {
	newt = ty_newtype(TE(TY_PTR), TT_PTLEN);
	TT(newt, ts_ptr)->ts_ptr = 0;	/* pointer to this type (none) */
	TT(newt, ts_ptr)->ts_type = t;	/* pointed-to type */
	if (t < TY_NBASE)
	    ty_ptrs[t] = newt;
	else
	    TT(t, ts_generic)->ts_ptr = newt;
    }
    return( newt );			/* return pointer type */
}


T1WORD
ty_mkaryof(t, n)
T1WORD t;
SIZE n;
/* Make a array of type (word) t with n elements.  Try to share
** null arrays of simple types.
*/
{
    static ty_typetab ty_nullary[TY_NBASE]; /* null arrays of simple types */
    int issimple = (n == TY_NULLDIM && t < TY_NBASE);
    T1WORD newt;
    ts_ary * tp;

    if (issimple && (newt = ty_nullary[t]) != TY_NONE)
	return(newt);

    newt = ty_newtype(TE(TY_ARY), TT_ARLEN);
    tp = TT(newt, ts_ary);
    
    tp->ts_type = t;			/* array-of type */
    tp->ts_ptr = 0;			/* no pointer to this type yet */
    ty_setasub(newt, n);		/* set number of elements, array size */

    /* Propagate qualifiers of embedded type. */
    tp->ts_spec |=
	(TT(t, ts_base)->ts_spec & TY_TMQUAL) |
    		TY_QUALSHIFT(TT(t, ts_base)->ts_spec & TY_TQUAL);

    if (issimple)
	ty_nullary[t] = newt;		/* save the type for later */
    return( newt );			/* return new type */
}


T1WORD
ty_mkfunc(t)
T1WORD t;
/* Create function-returning type t. */
{
    T1WORD newt = ty_newtype(TE(TY_FUN), TT_FNLEN);
    ts_fun * tp = TT(newt, ts_fun);

    tp->ts_rettype = t;			/* return type */
    tp->ts_ptr = 0;			/* no pointer to this type yet */
    tp->ts_prmlist = 0;			/* no params yet */

    /* Figure out the effective return type for the function.
    ** Functions that return smaller than int-sized integral types
    ** actually return int.  Functions that return float actually
    ** return double, except if there's a prototype that has a
    ** float (picked up by ty_mkparm()).
    */
    switch( TY_TYPE(t) ) {
    case TY_CHAR:  case TY_SCHAR:
    case TY_SHORT: case TY_SSHORT:
	t = TY_INT;
	break;
    
    /* Unsigned types promote differently in different versions. */
    case TY_USHORT:
#ifdef	NOSHORT				/* short same as int */
	t = TY_UINT;
	break;
#else
	/*FALLTHRU*/			/* short distinct from int */
#endif
    case TY_UCHAR:
	t = (version & V_CI4_1) ? TY_UINT : TY_INT;
	break;
    case TY_FLOAT:
	t = TY_DOUBLE;
	break;
    case TY_VOID:
	/* Check for qualifiers on void. */
	if (TY_ISQUAL(t))
	    WERROR("inappropriate qualifiers with \"void\"");
	break;
    }
    tp->ts_erettype = t;		 /* effective return type */
    return( newt );
}


void
ty_mkparm(func, paramt, hidden)
T1WORD func;
T1WORD paramt;
int hidden;
/* Add parameter of type paramt to function-returning type "func".
** If "hidden" is non-zero, this is hidden type information.
** A paramt == TY_DOTDOTDOT signifies start of a variable number of parameters.
** If an added parameter is of type "float", we may adjust the effective
** return type for this function to be float, rather than double.
** The additions are assumed to go left to right (first to
** last).  Function parameter types MUST be added to a
** function-returning type immediately after ty_mkfunc()
** has been called.  This routine updates the number-of-params
** field of the type.
*/
{
    int nparm;
    T1WORD prmlist;
    ts_fun * funcp = TT(func, ts_fun);

    /* Make sure func is a function-returning type, and that
    ** the next type-table word to be used follows the current
    ** parameter list.
    */
    if (! TY_ISFTN(func))
	cerror("ty_mkparm():  not a function");

    /* Check if there's already a parameter list.  If not, start one. */
    if ((prmlist = funcp->ts_prmlist) == 0) {
	prmlist = ty_newtype((ty_typetab) 0, 1); /* one word, fill with 0 */
	funcp = TT(func, ts_fun);	/* recompute pointer */
	funcp->ts_prmlist = prmlist;
	if (hidden)
	    funcp->ts_spec |= TY_THIDDEN; /* note hidden info */
    }

    nparm = TE(prmlist);

    /* Decode current number of parameters and check for errors. */
    if	    (nparm < -1) cerror("already have start of ... arguments");
    else if (nparm == 1) cerror("adding parameters after f(void)");
    else if (nparm != 0) nparm = GETNP(nparm);

    /* nparm is now actual number of parameters so far */
    
    /* Consider changing function effective return type.  Do so if
    ** the parameter type is float, the effective return type is
    ** double, the true return type is float, and this is for a
    ** true function prototype.
    */
    if (   !hidden
	&& TY_TYPE(paramt) == TY_FLOAT
	&& funcp->ts_erettype == TY_DOUBLE
	&& TY_TYPE((T1WORD) funcp->ts_rettype) == TY_FLOAT
    )
	funcp->ts_erettype = TY_FLOAT;

    if (paramt != TY_DOTDOTDOT) {	/* TY_DOTDOTDOT is flag for ... */
	/* Make sure new parameter is at end. */
	if (ty_newtype((ty_typetab) paramt, 1) != (prmlist + nparm + 1))
	    cerror("ty_mkfunc() and ty_mkparm() out of sync");
	/* (funcp may be invalid) */
    
	if (paramt == TY_VOID) {	/* special case for f(void) */
	    if (nparm != 0)
		cerror("f(...,void)");
	}
	else
	    ++nparm;			/* ordinary params need extra bump */
	nparm = SETNP(nparm,0);		/* new encoding */
					/* new number of parameters */
    }
    else				/* setting ...:  just change # */
	/* This actually handles f(...), if that's ever legal */
	nparm = SETNP(nparm,1);		/* new encoding */

    TE(prmlist) = nparm;
    return;
}


T1WORD
ty_erettype(t)
T1WORD t;
/* Return the effective return-type for the function whose type is
** t.
*/
{
    if (! TY_ISFTN(t))
	cerror("ty_erettype():  not a function");
    
    return( TT(t, ts_fun)->ts_erettype );
}

T1WORD
ty_mkqual(t, qual)
T1WORD t;
int qual;
/* Make a qualified version of type t.  qual
** contains the modifier bits to apply.  If the selected
** qualifiers are already in effect, do nothing.  If the type is
** already qualified by other qualifiers, make a new qualifier
** that qualifies the underlying (unqualified) type.  Otherwise,
** create a newly qualified type.
** The standard requires that a qualifier applied
** to a function or array really applies to the underlying type.
** To get this right, walk the type recursively until a suitable
** type is found, insert the qualifier, and rebuild a new type
** on the way out.  This can happen when a qualifier
** is applied to a typedef.
*/
{
    T1WORD newt;
    ty_typetab hadqual;

    /* If we're looking at a qualified type with the same
    ** qualifier, return it.  Otherwise, create a new qualified type.
    */
    if ((hadqual = (TT(t, ts_base)->ts_spec & TY_TQUAL)) == qual)
	return( t );

    switch( TY_TYPE(t) ) {
    case TY_FUN:
	/* Build function returning modified type, share param list.
	** Copy effective return type, which is unchanged.
	*/
	newt = ty_mkqual((T1WORD) TT(t,ts_fun)->ts_rettype, qual);
	if (newt == TT(t,ts_fun)->ts_rettype)
	    return( t );		/* no change required */
	newt = ty_mkfunc(newt);
	TT(newt, ts_fun)->ts_prmlist = TT(t, ts_fun)->ts_prmlist;
	TT(newt, ts_fun)->ts_erettype = TT(t, ts_fun)->ts_erettype;
	break;
    case TY_ARY:
	/* Build array of modified type. */
	newt = ty_mkqual((T1WORD) TT(t, ts_ary)->ts_type, qual);
	if (newt == TT(t,ts_ary)->ts_type)
	    return( t );		/* no change required */
	newt = ty_mkaryof(newt, (SIZE) TT(t,ts_ary)->ts_nelem);
	break;

    case TY_PTR:
    {
	ts_ptr * tp;

	/* Make a copy of the existing pointer type, stick in the qualifier
	** bits.  This simplifies the TY_DECREF() operation, since the
	** pointer actually points at the right place.  However, we
	** MUST make a new copy, since we might get back an existing
	** copy of the thing pointed at, which we can't modify.
	*/
	newt = ty_newtype(TE(TY_PTR), TT_PTLEN);
	tp = TT(newt, ts_ptr);
	tp->ts_spec |= qual;
	tp->ts_ptr = 0;			/* no pointer to this type */
	tp->ts_type = TT(t,ts_ptr)->ts_type;
	break;
    }

    default:
	/* Build a modified type.  The new type consists of a qualifier
	** structure with the same specifier bits as the underlying
	** type, plus the selected qualifier bits.  And we point to the
	** underlying type.
	*/
	newt =
	    ty_newtype((TT(t, ts_base)->ts_spec | qual), TT_QUALLEN);
	TT(newt, ts_qualtype)->ts_unqual =
	    hadqual ? TT(t, ts_qualtype)->ts_unqual : t;
	TT(newt, ts_qualtype)->ts_ptr = 0;	/* no pointer to this type yet */
	break;
    }
    return( newt );
}


T1WORD
ty_mktag(type, tag)
T1WORD type;
SX tag;
/* Make initial piece of struct/union/enum:  establish
** its tag and whether it is a struct/union/enum.  "type"
** contains the generic type TY_STRUCT/TY_UNION/TY_ENUM
** as to whether the tag is a struct/union/enum tag.
** "tag" is the symbol table entry for the tag.
** The returned T1WORD becomes the handle for this struct/union/enum.
*/
{
    T1WORD newt = ty_newtype(TE(type), TT_SUELEN);
    static ts_sue newsue;		/* we'll fill in non-zero parts */

    if (! TY_ISSUE(newt))		/* make sure we've got s/u/e */
	cerror("bad type to ty_mktag()");

    /* Fill in new type. */
    newsue.ts_spec = TE(type);
    newsue.ts_tag = tag;
    *TT(newt, ts_sue) = newsue;		/* copy new s/u/e structure */

    return( newt );
}

void
ty_mkmbr(t, mbr)
T1WORD t;			/* add to this struct/union */
SX mbr;				/* symbol index of s/u member */
/* Add a member to a struct/union.  Members need not be added
** immediately after a struct/union has been started (ty_mktag()),
** but they must be added consecutively, in order, without
** intervening type creation operations.  Keep track of embedded
** member qualifiers.
*/
{
    T1WORD newt;
    ts_sue * tp = TT(t, ts_sue);

    if (mbr != SY_NOSYM) {
	T1WORD tmbr = SY_TYPE(mbr);	/* member's type */

	/* Propagate qualifiers of embedded type. */
	tp->ts_spec |= 
		TY_QUALSHIFT(TT(tmbr, ts_base)->ts_spec & TY_TQUAL) |
		    TT(tmbr, ts_base)->ts_spec & TY_TMQUAL;
    }
    else if (tp->ts_mbrlist != 0)	/* empty entry must be first */
	cerror("confused ty_mkmbr()");
    else {
	/* Special hack for no members:  set first, last to random
	** numbers such that the number of elements returned is 0.
	*/
	tp->ts_mbrlist = TY_STRUCT;
	tp->ts_lastmbr = TY_STRUCT;
	return;
    }
	
    newt = ty_newtype((ty_typetab) mbr, 1);
    /* Pointer may have changed; recompute. */
    tp = TT(t, ts_sue);

    /* If this is first member, start new list. */
    if (tp->ts_mbrlist == 0)
	tp->ts_mbrlist = newt;
    else {
	/* Make sure "last pointer" points at end of table. */
	if ( tp->ts_lastmbr != TD_USED(typetab)-1)
	    cerror("confused ty_mkmbr(2)");
    }
    tp->ts_lastmbr = newt+1;		/* number of next table entry */
    return;
}

void
ty_e_sue(t, size, align)
T1WORD t;			/* struct/union/enum being set */
BITOFF size;			/* final struct/union/enum size */
BITOFF align;			/* same for align */
/* End of s/u/e type creation.  Set the final size and alignment
** for a struct/union/enum.  If s/u/e didn't have a member list,
** force one now.
*/
{
    if (TT(t, ts_sue)->ts_mbrlist == 0)
	ty_mkmbr(t, SY_NOSYM);	/* Create a single null member so we can claim
				** to have had a member list; happens only when
				** all s/u members are unnamed.
				*/
    TT(t,ts_sue)->ts_size = size;
    TT(t,ts_sue)->ts_align = align;
    return;
}


BITOFF
ty_size(t)
T1WORD t;
/* Return size of type t, in bits.  Because of delayed definition:
**	typedef struct s sa[5];
**	struct s {int x;};
** it's necessary to recheck sizes if they're zero.
*/
{
    BITOFF size;
    int gentype = TY_TYPE(t);		/* type's generic type */

    if ((size = _ty_size[gentype]) == 0) {
	switch( gentype ){
	    static const char mesg[] = "compiler takes size of %s";
	case TY_FUN:	cerror(mesg, "function");	/*NOTREACHED*/
	case TY_VOID:	cerror(mesg, "void");		/*NOTREACHED*/
	case TY_STRUCT: case TY_UNION: case TY_ENUM:
	    SKIPQUAL(t);
	    size = TT(t, ts_sue)->ts_size;
	    break;
	case TY_ARY:
	    if ((size = TT(t, ts_ary)->ts_size) == 0)
		size = ty_sizesub(t);
	    break;
	}
    }
    return( size );
}


static BITOFF
ty_sizesub(t)
T1WORD t;
/* Recursively determine size of t. */
{
    BITOFF size;
    if (!TY_ISARY(t))
	cerror("non-array in ty_sizesub()");

    size = TT(t, ts_ary)->ts_size;

    if (size == 0) {
	switch( TY_NELEM(t) ){
	case TY_ERRDIM:
	case TY_NULLDIM:
	case 0:
	    break;			/* these are hopeless */
	default:
	    if (TY_SIZE(TY_DECREF(t))) { /* if have size of elem now... */
		ty_setasub(t, TY_NELEM(t)); /* compute size with new info */
		size = TT(t, ts_ary)->ts_size;
	    }
	    break;
	}
    }
    return( size );
}


BITOFF
ty_align(t)
T1WORD t;
/* Return alignment for type t, in bits. */
{
    BITOFF align = _ty_align[TY_TYPE(t)];

    if (align == 0) {
	while (TY_ISARY(t))
	    t = TT(t, ts_ary)->ts_type;	/* follow to underlying type */
	if (TY_ISSUE(t)) {
	    SKIPQUAL(t);
	    align = TT(t, ts_sue)->ts_align;
	}
	else if ((align = _ty_align[TY_TYPE(t)]) == 0)
	    cerror("compiler takes alignment of function or void");
    }
    return( align );
}

#ifndef	NODBG

T1WORD
ty_type(t)
T1WORD t;
/* Return the top level type for t.  That means for base types
** we return the base type; for qualifiers, we return the
** unqualified type.
*/
{
    t = TT(t, ts_base)->ts_spec & TY_BTMASK;
    return( t );
}


T1WORD
ty_decref(t)
T1WORD t;
/* Return next level type for type t.  This only applies
** to arrays, functions, pointers.
*/
{
    if (TT(t, ts_base)->ts_spec & (TY_TARRAY|TY_TPTR|TY_TFUNC))
	return( TT(t, ts_generic)->ts_type );

    /* Bad type for ty_decref() */
    ty_print(t);
    cerror("can't de-reference type");
    /*NOTREACHED*/
}

#endif


int
ty_haslist(t)
T1WORD t;
/* Return 1 if type t (which must be struct/union/enum) has
** an associated member list (yet), 0 if not.  Strip off
** qualifiers before checking.
*/
{
    if (! TY_ISSUE(t))
	cerror("ty_haslist() called with bad type");

    SKIPQUAL(t);
    return( TT(t, ts_sue)->ts_mbrlist );
}

int
ty_eqtype(tl, tr)
T1WORD tl;
T1WORD tr;
/* Return 1 if types tl and tr are identical.
** Return -1 if types tl and tr would be considered
** identical in C Issue 4.1.  Otherwise return 0.
** Return -2 (TY_HIDNONCOMPAT) if types are identical
** except for a mismatch due to hidden information.
*/
{
    for (;;) {
	ty_typetab specl;
	ty_typetab specr;

	if (tl == tr) return 1;		/* same type */

	specl = TT(tl, ts_base)->ts_spec;
	specr = TT(tr, ts_base)->ts_spec;

	/* Member qualifier bits may differ if the first declaration of a
	** tag did not define members and a subsequent one declared qualified
	** members.
	*/
	if ((specl & ~(TY_THIDDEN|TY_TMQUAL)) != (specr & ~(TY_THIDDEN|TY_TMQUAL))) {
	    /* Do compatibility checks on integral types:  UNIX C allowed
	    ** int to match long if they were the same size.  ANSI C allows
	    ** int and signed int to be equivalent, but they're different
	    ** types.
	    */

	    /* If either type is non-integral, fail.  Also, if either type
	    ** is qualified, fail:  not a compatibility concern.
	    */
	    if (   (specl & TY_TINTTYPE) == 0
		|| (specr & TY_TINTTYPE) == 0
	        || ((specl|specr) & TY_TQUAL) != 0
	       )
		return( 0 );

	    /* Weed out other unprofitable cases. */
	    specl &= TY_BTMASK;
	    specr &= TY_BTMASK;
	    if (_ty_size[specl] != _ty_size[specr])
		return( 0 );			/* sizes must match */

#define XX(l,r) (l*TT_LASTBASE+r)
	    switch( XX(specl,specr) ) {
	    case XX(	TY_INT,		TY_SINT	):
	    case XX(	TY_SINT,	TY_INT	):
	    case XX(	TY_SHORT,	TY_SSHORT ):
	    case XX(	TY_SSHORT,	TY_SHORT ):
	    case XX(	TY_LONG,	TY_SLONG ):
	    case XX(	TY_SLONG,	TY_LONG ):
		return( 1 );

	    /* These mismatched cases can arise for ambiguous function
	    ** parameter types.
	    */
	    case XX(	TY_INT,		TY_UINT	):
		if (tl == TY_AINT || tr == TY_AUINT)
		    return( -1 );
		return( 0 );
	    case XX(	TY_UINT,	TY_INT	):
		if (tl == TY_AUINT || tr == TY_AINT)
		    return( -1 );
		return( 0 );

	    /* These were considered the same in C Issue 4.1.
	    ** (No need to consider "signed" versions, because
	    ** they didn't exist.)
	    */
#ifdef NOLONG
	    case XX(	TY_LONG,	TY_INT		):
	    case XX(	TY_INT,		TY_LONG 	):
	    case XX(	TY_ULONG,	TY_UINT		):
	    case XX(	TY_UINT,	TY_ULONG	):
#endif
#ifdef NOSHORT
	    case XX(	TY_SHORT,	TY_INT 		):
	    case XX(	TY_INT,		TY_SHORT 	):
	    case XX(	TY_USHORT,	TY_UINT		):
	    case XX(	TY_UINT,	TY_USHORT	):
#endif
		if (version & V_CI4_1)
		    return( -1 );
		/*FALLTHRU*/
	    default:
		/* Other type combinations mismatch. */
		return( 0 );
	    } /* end switch */
	    /*NOTREACHED*/
	} /* end if on different kinds */

	/* Same generic types. */
	switch( specl&TY_BTMASK ) {
	case TY_PTR:
	    /* Both are pointers with same (or no) qualifier bits. */
	    tl = TT(tl, ts_ptr)->ts_type;
	    tr = TT(tr, ts_ptr)->ts_type;
	    /* For transition mode, allow char * and void * to be
	    ** equivalent.
	    */
	    if (version & V_CI4_1) {
		if (   (tl == TY_CHAR && tr == TY_VOID)
		    || (tl == TY_VOID && tr == TY_CHAR)
		)
		    return( -1 );
	    }
	    continue;
	
	case TY_ARY:
	    /* Assume, for this test, that at least one of the sizes may
	    ** be null and still be considered the same.  The assumption
	    ** is that only the outermost array may be null legally.
	    */
	    if (   TT(tl, ts_ary)->ts_nelem == TT(tr, ts_ary)->ts_nelem
		|| TT(tl, ts_ary)->ts_nelem == TY_NULLDIM
		|| TT(tr, ts_ary)->ts_nelem == TY_NULLDIM
	    ) {
		/* Arrays are equivalent. */
		tl = TT(tl, ts_ary)->ts_type;
		tr = TT(tr, ts_ary)->ts_type;
		continue;
	    }
	    if ((specl & TY_THIDDEN) != 0 || (specr & TY_THIDDEN) != 0) {
		/* One side has a hidden dimension.  Call them
		** the same if they have the same underlying type.
		*/
		if (TY_EQTYPE(TY_DECREF(tl), TY_DECREF(tr)))
		    return( TY_HIDNONCOMPAT );
	    }
	    return( 0 );
	    
	case TY_FUN:
	{
	    /* Must have same return type, and parameters must be
	    ** equivalent.  If parameter types don't match exactly,
	    ** mismatch (except for ...).  If return types match
	    ** according to CI4.1 rules, indicate same.
	    **
	    ** A special dispensation is granted for function return-type
	    ** mismatches where one is "unsigned" and the other is "int".
	    ** Say the return types are compatibly matched.  This copes
	    ** with changes due to header files for ANSI C that changed
	    ** types of functions like strlen().
	    */
	    int eqtype = ty_eqtype(
		    (T1WORD) TT(tl, ts_fun)->ts_rettype,
		    (T1WORD) TT(tr, ts_fun)->ts_rettype
		);
	    int eqptype = ty_chkparam(tl, tr);

	    if (eqtype == 0) {
		T1WORD retl = TT(tl, ts_fun)->ts_rettype;
		T1WORD retr = TT(tr, ts_fun)->ts_rettype;
		if (   (TY_EQTYPE(retl, TY_UINT) > 0 && TY_EQTYPE(retr, TY_INT) >0)
		    || (TY_EQTYPE(retl, TY_INT) > 0 && TY_EQTYPE(retr, TY_UINT) >0)
		)
		    eqtype = -1;
	    }
		
	    if (eqtype == 0 || eqptype == 0)
		return( 0 );		/* complete mismatch */
	    /* Return minimum non-zero value:  1, -1, -2. */
	    return ( eqtype < eqptype ? eqtype : eqptype );
	}
	
	case TY_STRUCT:
	case TY_UNION:
	case TY_ENUM:
	    /* Comparing different struct/union/enum.  The types
	    ** are the same only if they have the same tag, in
	    ** which case the type numbers are identical.
	    ** If they are qualified types, need to strip qualifiers
	    ** and check further.  The qualifiers have already been
	    ** checked.
	    */
	    if (! TY_ISQUAL(tl))	/* not qualified:  not same */
		return 0;
	    tl = TY_UNQUAL(tl);
	    tr = TY_UNQUAL(tr);
	    continue;

	default:
	    /* All that remains here are qualified simple types.
	    ** (Other simple types would have been caught by tl == tr.)
	    ** It must be the case that the qualifiers match, but the
	    ** type numbers are different.
	    */
	    return( 1 );
	} /* end switch */
    } /* end for */
    /*NOTREACHED*/
}


static int
ty_chkparam(tl, tr)
T1WORD tl;
T1WORD tr;
/* Check the parameters of two functions for equivalence.
** Section 3.5.5:
**	Two functions that return the same type are treated as having
**	the same type if one function declarator has no parameter
**	specification and the other specifies a fixed number of
**	parameters, none of which is affected by the default argument
**	promotions.
** This rule is relaxed for compatibility reasons if one of
** the types has ... and the other is an old-style type.
** Of course, if both have prototypes, the prototypes must match.
** Following results of 9/87 ANSI meeting, qualifiers don't matter
** when matching prototype parameter types.
**
** Another check is made if either function type has hidden
** type information.  If there is a mismatch that would be
** invisible if the hidden information weren't there, return -2.
** In that case a 0 return is impossible.
** Return 0 on mismatch, -1 on compatibility match, 1 on match.
*/
{
    int lplist = TT(tl, ts_fun)->ts_prmlist;	/* left-side param list */
    int rplist = TT(tr, ts_fun)->ts_prmlist;
    int nl;					/* number of params on left */
    int nr;
    int nparams;
    int plist;
    int retval = 1;			/* assume final success */

    /* Check for which case. */
    if (lplist == rplist)
	return( 1 );			/* both have same (possibly none)
					** parameter list
					*/

    nl = lplist ? TE(lplist) : 0;	/* number of params on left */
    nr = rplist ? TE(rplist) : 0;	/* number of params on right */
    if (lplist && rplist) {		/* both have parameter lists */
	if (nl != nr)
	    goto mismatch;		/* parameter lists are diff. length */

	/* Both have prototypes.  Types must be identical, unless
	** we find one involving ambiguous types.
	*/
	nparams = GETNP(nl);

	++lplist; ++rplist;		/* bump index of both lists */
	retval = 1;			/* assume success */
	for (; nparams > 0; --nparams) {
	    int eqtype = ty_eqtype(
				TY_UNQUAL((T1WORD) TE(lplist)),
				TY_UNQUAL((T1WORD) TE(rplist))
				);
	    
	    if (eqtype == 0)
		goto mismatch;
 
	    /* Look for mismatch because of ambiguous type (only). */
	    if (eqtype < 0) {
		switch( XX(
			TY_UNQUAL((T1WORD) TE(lplist)),
			TY_UNQUAL((T1WORD) TE(rplist))
			)
		){
		case XX(	TY_AINT,	TY_UINT  ):
		case XX(	TY_AUINT,	TY_INT   ):
		case XX(	TY_INT,		TY_AUINT ):
		case XX(	TY_UINT,	TY_AINT  ):
		    /* These combinations incompatible because of
		    ** change in type promotion rules.  Call them
		    ** compatibly equivalent.
		    */
		    retval = -1;
		    break;
		default:
		    goto mismatch;
		}
	    }
	    ++lplist;
	    ++rplist;
	}
	return( retval );		/* prototypes match */
    }

    /* (3.5.5) Two functions that return the same type are treated as
    ** having the same type if one function declarator has no parameter
    ** specification and the other specifies a fixed number of parameters,
    ** none of which is affected by the default argument promotions.
    */
    if (nl == 0) {
	if (ISDDD(nr))
	    retval = -1;		/* right is variable length */
	plist = rplist+1;		/* check right side */
	nparams = GETNP(nr);
    }
    else if (nr == 0) {
	if (ISDDD(nl))
	    retval = -1;		/* left is variable length */
	plist = lplist+1;		/* check left side */
	nparams = GETNP(nl);
    }
    
    /* One of the types included a prototype.  The list of types is at
    ** offset plist in the type table.  Check them for default argument
    ** promotions.  Make ones that are smaller than int return a compatible
    ** failure.  (The compiler still passes them as int-size, whether they're
    ** declared smaller or not.)
    */
    {
	int incompat = 0;

	for (; nparams > 0; --nparams) {
	    switch( TY_TYPE((T1WORD) TE(plist)) ) {
	    case TY_CHAR:
	    case TY_UCHAR:
	    case TY_SCHAR:
#ifndef	NOSHORT
	    case TY_SHORT:
	    case TY_USHORT:
	    case TY_SSHORT:
#endif
		incompat = 1;
		break;
	    case TY_FLOAT:
		goto mismatch;		/* this causes non-equivalence */
	    }
	    ++plist;
	}
	if (! incompat)
	    return( retval );		/* parameters are equivalent */
	retval = -1;			/* compatible mismatch */
    }
    /*FALLTHRU*/
mismatch:;
    /* Come here if there's a mismatch.  Check for hidden type
    ** information.  Return 0 if it's all visible, -2 otherwise.
    */
    if (   (TT(tl, ts_fun)->ts_spec & TY_THIDDEN) != 0
	|| (TT(tr, ts_fun)->ts_spec & TY_THIDDEN) != 0
	)
	return( TY_HIDNONCOMPAT );	/* hidden info mismatch */
    /* If current return value is unchanged from the start, there is a
    ** complete mismatch.
    */
    if (retval == 1)
	retval = 0;
    return( retval );
}


int
ty_hashidden(t)
T1WORD t;
/* Returns 1 if function type t has hidden argument type
** information, 0 otherwise.
*/
{
    if (TY_ISFTN(t))
	return(   (TT(t, ts_fun)->ts_spec & TY_THIDDEN) != 0
	       && TT(t, ts_fun)->ts_prmlist != 0
	);

    cerror("non-function in ty_hasproto()");
    /*NOTREACHED*/
}

int
ty_hasproto(t)
T1WORD t;
/* Returns 1 if function type t has prototype, 0 otherwise. */
{
    if (TY_ISFTN(t))
	return(   (TT(t, ts_fun)->ts_spec & TY_THIDDEN) == 0
	       && TT(t, ts_fun)->ts_prmlist != 0
	);

    cerror("non-function in ty_hasproto()");
    /*NOTREACHED*/
}


T1WORD
ty_proprm(t, pn)
T1WORD t;
int pn;
/* Return type of pn-th parameter for function type t.
** Parameters range from 0 to n-1.
** If pn is beyond number of formals, but there is a ...,
** return TY_DOTDOTDOT.
*/
{
    int plist;

    if (!TY_ISFTN(t))
	cerror("non-function in ty_proprm()");
    
    plist = TT(t, ts_fun)->ts_prmlist;
    /* Check for too-large parameter number. */
    if (!plist || pn >= GETNP(TE(plist)))
	cerror("invalid parameter number %d in ty_proprm()", pn);

    return( TE(plist+1+pn) );
}

T1WORD
ty_unqual(t, qual)
T1WORD t;
int qual;
/* Remove qualifier qual from type t, return unmodified type.
** If modifier doesn't match, return original type.
** Pointers are special:  since we hoisted the qualifier bits into the pointer
** modifier, we have to get the unmodified pointer type for them.
** That should be cheap, since we will have already built such a type
** once, before we modified it with a qualifier.
*/
{
    ty_typetab mod;			/* remaining qualifier bits */
    ty_typetab spec;
    T1WORD unqualtype;

    if (((spec = TT(t, ts_base)->ts_spec) & qual) == 0)
	return( t );			/* specified modifier is off */
    
    mod = spec & TY_TQUAL & ~qual;

    /* Choose correct un-qualified type. */
    unqualtype =
	  (spec & TY_TPTR)
	? ty_mkptrto((T1WORD) TT(t,ts_ptr)->ts_type)
	: TT(t, ts_qualtype)->ts_unqual;
    return( (mod == 0) ? unqualtype : ty_mkqual(unqualtype, (int) mod) );
}

int
ty_nparam(t)
T1WORD t;
/* Return the number of parameters associated with (presumed)
** function-returning-type t.  If t does not have an associated
** prototype, return -1.  Otherwise return the number of
** parameters for which types are known.  (ty_isvararg() tells
** whether more arguments are allowed.)
*/
{
    ty_typetab plist;

    if (!TY_ISFTN(t))
	cerror("bad type in ty_nparam()");
    
    plist = TT(t, ts_fun)->ts_prmlist;
    return( plist ? GETNP(TE(plist)) : -1 );
}

int
ty_isvararg(t)
T1WORD t;
/* Return 1 if function-returning-type t has a variable
** number of arguments, else 0.
*/
{
    ty_typetab plist;

    if (!TY_ISFTN(t))
	cerror("bad type in ty_isvararg()");
    
    plist = TT(t, ts_fun)->ts_prmlist;
    return( plist ? ISDDD(TE(plist)) : 0 );
}


SIZE
ty_nelem(t)
T1WORD t;
/* Return number of elements for type t.  For an array, it's the
** number of array elements.  For a struct, union, or enum,
** it's the number of members.
*/
{
    switch( TY_TYPE(t) ) {
    case TY_ARY:	return( TT(t, ts_ary)->ts_nelem );
    case TY_UNION:
    case TY_STRUCT:
    case TY_ENUM:
	SKIPQUAL(t);
	return( TT(t, ts_sue)->ts_lastmbr - TT(t, ts_sue)->ts_mbrlist );
    default:
	cerror("bad type %d in ty_nelem()", t);
    }
    /*NOTREACHED*/
}


static void
ty_setasub(t, nelem)
T1WORD t;
SIZE nelem;
/* Set number of elements for array t, adjust final array size. */
{
    ts_ary * tp = TT(t, ts_ary);
    unsigned long elemsize = TY_SIZE((T1WORD) tp->ts_type );
					/* size of an element */

    tp->ts_nelem = nelem;
    if (nelem == TY_NULLDIM || nelem == TY_ERRDIM)
	nelem = 0;
    /* Check for overflow in array size calculation. */
    if ( elemsize != 0 && (~((unsigned long) 0) / elemsize) < nelem) {
	UERROR("array too big");
	elemsize = 1;
    }
    tp->ts_size = nelem * elemsize;
    return;
}


SX
ty_g_mbr(t, mbrno)
T1WORD t;
SIZE mbrno;
/* Return symbol index corresponding to mbrno-th member of
** struct/union/enum t, starting with 0.  If at end, return SY_NOSYM.
*/
{
    ty_typetab plist;

    if (! TY_ISSUE(t) || (plist = TT(t, ts_sue)->ts_mbrlist) == 0)
	cerror("bad type %d in ty_g_mbr()", t);
    
    plist += mbrno;
    /* Return proper member or SY_NOSYM. */
    return( plist >= TT(t,ts_sue)->ts_lastmbr ? SY_NOSYM : TE(plist) );
}


SX
ty_suembr(t, mbr)
T1WORD t;
char * mbr;
/* Try to find member mbr of struct/union/enum t and return the
** corresponding symbol table entry index.  Return SY_NOSYM if not found.
** Strip qualifiers from the struct/union type.
*/
{
    ty_typetab plist;
    ts_sue * tp;
 
    if (! TY_ISSUE(t) )
	cerror("not s/u/e in ty_suembr()");

    SKIPQUAL(t);
    tp = TT(t, ts_sue);

    for (plist = tp->ts_mbrlist; plist < tp->ts_lastmbr; ++plist) {
	SX sid = TE(plist);
	if (SY_NAME(sid) == mbr)
	    return( sid );
    }
    /* not found */
    return( SY_NOSYM );
}


SX
ty_suetag(t)
T1WORD t;
/* Return symbol ID of tag for s/u/e t. */
{
    if (! TY_ISSUE(t) )
	cerror("not s/u/e in ty_suetag()");
    SKIPQUAL(t);
    return( TT(t, ts_sue)->ts_tag );
}

#ifndef	NODBG

int
ty_istype(t,whatsok)
T1WORD t;
long whatsok;
/* Return non-0 if t is of a suitable type, else 0.
** whatsok consists of type specifier bits.
*/
{
    return ( TT(t, ts_base)->ts_spec & whatsok );
}

#endif

int
ty_isptrobj(t)
T1WORD t;
/* Return 1 if t is a pointer to a (non-zero sized) object,
** 0 otherwise.
*/
{
    if (TY_ISPTR(t)) {
	t = TT(t,ts_ptr)->ts_type;	/* get pointed-at type */

	switch( TY_TYPE(t) ) {
	case TY_FUN:
	case TY_VOID:
	    break;			/* not objects */
	default:
	    return( TY_SIZE(t) != 0 );
	}
    }
    return( 0 );			/* not pointer to object */
}

int
ty_isunsigned(t)
T1WORD t;
/* Return 1 if type t is an unsigned type, 0 otherwise. */
{
    return( (TT(t, ts_base)->ts_spec & TY_TSGMASK) == TY_TUNSIGNED );
}

int
ty_issigned(t)
T1WORD t;
/* Return 1 if type t is explicitly "signed", -1 if
** "natural", 0 if neither.  enums behave like plain int.
*/
{
    ty_typetab spec = TT(t,ts_base)->ts_spec & TY_TSGMASK;
    if (spec)
	spec -= TY_TUNSIGNED;		/* magic:  makes signed +, plain - */
    return( spec );
}


T1WORD
ty_chksize(t, name, flags, lineno)
T1WORD t;
char * name;
int flags;
int lineno;
/* Reached a state where something with a size is required, or
** a function definition needs to know the size of the object returned.
** The incoming type is t; the name of the object/function is name.
** Flags is some combination of
**	TY_CVOID	error for void
**	TY_CTOPNULL	warn about top-level null array
**	TY_CSUE		error for incomplete struct/union/enum
**
** lineno is a the line number for a diagnostic, or -1 for "current".
** Return possibly different substitute type.
*/
{
    T1WORD rettype = t;			/* presumed return-type */

    switch( TY_TYPE(t) ){
    case TY_VOID:
	if (flags & TY_CVOID) {
	    ULERROR(lineno, "cannot have void object: %s", name);
	    rettype = TY_INT;		/* substitute type */
	}
	break;
    case TY_FUN:
	t = TY_DECREF(t);		/* look at type returned */
	if (TY_TYPE(t) == TY_VOID)
	    break;			/* function returning void */
	/*FALLTHRU*/
    default:
	if (TY_SIZE(t) == 0) {
	    if (   (flags & TY_CTOPNULL)
		&& (TY_ISARY(t) && TY_NELEM(t) == TY_NULLDIM)
		)
		WLERROR(lineno, "null dimension: %s", name);

	    if (flags & TY_CSUE) {
		while (TY_TYPE(t) == TY_ARY)
		    t = TY_DECREF(t);
		if (TY_ISSUE(t) && !TY_HASLIST(t)) {
		    ULERROR(lineno, "incomplete struct/union/enum %s: %s",
			SY_NAME(TY_SUETAG(t)), name);

		    /* Make struct/union/enum types look complete.  For
		    ** enums, give them the size and alignment of ints.
		    ** However, do not complete an incomplete type if the
		    ** current scope level is different from the level of
		    ** the tag.
		    */
		    if (sy_getlev() == SY_LEVEL(ty_suetag(t))) {
			if (TY_ISSU(t))
			    ty_e_sue(t, (BITOFF) 0, (BITOFF) ALSTRUCT);
			else
			    ty_e_sue(t, TY_SIZE(TY_INT), (BITOFF) ALINT);
		    }
		}
	    }
	}
	break;
    } /* end switch */
    return( rettype );			/* return new type */
}

static T1WORD
ty_compsub(tl,tr,hiding)
T1WORD tl;
T1WORD tr;
int hiding;
/*
** Common code for ty_mkcomposite() and ty_mkhidden().
** Both routines do nearly the same thing, except that
** the type information that results from ty_mkhidden()
** is marked as "hidden" when the type information is
** derived from tl.
**
** A composite is built recursively.
**	1) For simple types, return either.
**	2) For pointers, form a pointer to the composite of the
**		types pointed to.  If the pointer is qualified,
**		be sure to propagate the qualifiers.
**	3) For an array, if one has a size and the other doesn't,
**		choose the one with the size.
**	4) For functions, use the same return type (as both); form
**		a parameter list from the respective composite types.
**
** For hidden information, the type qualifiers in a function
** prototype are immaterial.
**
** It's assumed that these routines are called only when it makes
** sense to with respect to the language:  when a function or array
** is redeclared.
*/
{
    T1WORD tc;				/* low-level composite; returned
					** composite
					*/
    T1WORD tdecl, tdecr;		/* DECREF'ed versions of tl, tr */
    SIZE nelem;

    if (tl == tr)
	return( tl );			/* identical types */

    switch( TY_TYPE(tl) ){
    default:
	tc = tl;			/* simple types; arbitrarily choose
					** left
					*/
	break;
    case TY_PTR:
	tdecl = TY_DECREF(tl);
	tdecr = TY_DECREF(tr);
	tc = ty_compsub(tdecl, tdecr, hiding);
	/* Use existing pointer, if possible; otherwise create suitable one. */
	if (tc == tdecl)
	    tc = tl;
	else if (tc == tdecr)
	    tc = tr;
	else {
	    /* Created new type. */
	    ty_typetab qual = TT(tl, ts_base)->ts_spec & TY_TQUAL;
	    tc = ty_mkptrto(tc);
	    if (qual && !hiding)
		tc = ty_mkqual(tc, (int) qual);
	}
	break;
    case TY_ARY:
	/* Choose N or ERRDIM or NULLDIM, in that order. */
	if ((nelem = TY_NELEM(tl)) == TY_NULLDIM)
		nelem = TY_NELEM(tr);
	else if (nelem == TY_ERRDIM) {
	    if ((nelem = TY_NELEM(tr)) == TY_NULLDIM)
		nelem = TY_ERRDIM;
	}
	tdecl = TY_DECREF(tl);
	tdecr = TY_DECREF(tr);
	tc = ty_compsub(tdecl, tdecr, hiding);

	/* Try to reuse types. */
	if (tc == tdecl && nelem == TY_NELEM(tl))
	    tc = tl;
	else if (tc == tdecr && nelem == TY_NELEM(tr))
	    tc = tr;
	else
	    tc = ty_mkaryof(tc, nelem);
	/* Mark information as hidden if appropriate. */
	if (hiding && tc != tr)
	    TT(tc, ts_ary)->ts_spec |= TY_THIDDEN;
	break;
    case TY_FUN:
    {
	ty_typetab newlist;
	ty_typetab savelist;
	ty_typetab llist;		/* left-side parameter list */
	ty_typetab rlist;		/* right-side parameter list */
	int nparm;
	int savenparm;
	int ishidden = 0;		/* note hidden information */

	tc = ty_mkfunc( ty_compsub(TY_DECREF(tl), TY_DECREF(tr), hiding) );

	/* If one function type lacks parameter list, use the other. */
	llist = TT(tl, ts_fun)->ts_prmlist;
	rlist = TT(tr, ts_fun)->ts_prmlist;
	if (llist == 0) {
	    /* Use right-side params (even if none).  If right side is
	    ** non-empty list and is hidden, new information is hidden
	    ** too.
	    */
	    savelist = rlist;
	    if (hiding && rlist && (TT(tr,ts_fun)->ts_spec & TY_THIDDEN) != 0)
		ishidden = 1;
	}
	else if (rlist == 0) {
	    savelist = llist;		/* use left-side params */
	    if (hiding && llist)
		ishidden = 1;
	}
	/* Otherwise, both have parameter lists. */
	else if (hiding) {
	    /* The right side must have a list.  */
	    savelist = rlist;
	    if (hiding && (TT(tr,ts_fun)->ts_spec & TY_THIDDEN) != 0)
		ishidden = 1;
	    /* If not really making hidden information, propagate
	    ** effective return type.
	    */
	    else if (TT(tr, ts_fun)->ts_erettype == TY_FLOAT)
		TT(tc, ts_fun)->ts_erettype = TY_FLOAT;
	}
	else {
	    /* Must form composite.  First take care of effective
	    ** return type.  The effective return type can only
	    ** change when both function types have parameter lists
	    ** that are compatible.  Therefore it is enough to copy
	    ** the return type of either side if it is float.
	    */
	    if (TT(tl, ts_fun)->ts_erettype == TY_FLOAT)
		TT(tc, ts_fun)->ts_erettype = TY_FLOAT;
	    
	    /* Allocate enough space for all parameters now.  */
	    nparm = TE(llist);
	    savenparm = GETNP(nparm);

	    /* First word is count, followed by GETNP() words. */
	    newlist = savelist = ty_newtype((ty_typetab) nparm, savenparm+1);
	    for (nparm = savenparm; nparm > 0; --nparm) {
		++llist; ++rlist; ++newlist;
		TE(newlist) =
		    ty_compsub((T1WORD) TE(llist),(T1WORD) TE(rlist),0);
	    }
	}

	TT(tc, ts_fun)->ts_prmlist = savelist;
	if (ishidden)
	    TT(tc, ts_fun)->ts_spec |= TY_THIDDEN;
	break;
    } /* end TY_FUN case */
    } /* end switch */
    return( tc );
}

T1WORD
ty_mkcomposite(tl,tr)
T1WORD tl;
T1WORD tr;
/* Make a composite type from tl and tr.  The two types are
** assumed to be equivalent according to ANSI C rules.
** Bias the type building toward the left type.  That is,
** prefer the left type to the right when a choice must be
** made.
*/
{
    /* Verify that the types are equivalent. */
    if (!TY_EQTYPE(tl,tr))
	cerror("non-equivalent types in ty_mkcomposite");
    return( ty_compsub(tl,tr,0) );
}

T1WORD
ty_mkhidden(tl,tr)
T1WORD tl;
T1WORD tr;
/* Make hidden type information from tl and tr.  The two types are
** assumed to be equivalent according to ANSI C rules.
** The information in type tr is used to provide hidden information
** for type tl.  If tr is TY_NONE, make hidden version of tl.
*/
{
    if (tr == TY_NONE) {
	/* Create a type with which we can compute hidden information. */
	switch(TY_TYPE(tl)){
	case TY_FUN:	tr = ty_mkfunc(TY_DECREF(tl)); break;
	case TY_ARY:	tr = ty_mkaryof(TY_DECREF(tl),TY_NELEM(tl)); break;
	case TY_PTR:	tr = ty_mkptrto(TY_DECREF(tl)); break;
	default:
	    return( tl );
	}
    }
    /* Verify that the types are equivalent. */
    else if (!TY_EQTYPE(tl,tr))
	cerror("non-equivalent types in ty_mkhidden");
    return( ty_compsub(tl,tr,1) );
}
