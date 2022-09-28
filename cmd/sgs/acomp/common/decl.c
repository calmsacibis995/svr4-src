/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/decl.c	55.5"
/* decl.c */

#include "p1.h"

/* This module contains routines to handle declarations.
** The declaration mechanism is divided into bite-size
** routines that mark the beginning and end of key events.
*/

#define	NOTYBIT	0			/* no explicit type bits */

/* These symbols define declaration contexts. */

#define	DCL_TOP		0		/* top-level:  start of declarator:
					**	expect to see type and
					**	storage class specifiers
					*/
#define	DCL_STRUCT	1		/* in struct declarator */
#define	DCL_UNION	2		/* in union declarator */
#define	DCL_FUNC	3		/* in function param. decl */
#define	DCL_FORMAL	5		/* in decl. list after function decl.
					** for function def.
					*/
#define	DCL_ABS		6		/* top of abstract declarator */


/* Because declarations are recursive, we need a stack to
** keep track of nested stuff.
*/

static struct decl {
    TYCL dcl_tybit;		/* current type bits */
    SY_CLASS_t dcl_class;	/* current storage class */
    I16 dcl_context;		/* current context */
    I16 dcl_typeok;		/* typename ok in current context */
    SY_FLAGS_t dcl_table;	/* current table to use for symbol lookup */
    SX  dcl_sid;		/* current name's symbol index, or 0 */
    T1WORD dcl_btype;		/* current base type, or 0 */
    CONVAL dcl_enumval;		/* current default value for current enumerator */
    SX dcl_savetag;		/* symbol entry of s/u/e tag that may have to
				** be hidden (see dcl_tag(), dcl_chkend())
				*/
} dcl_state =
{
    NOTYBIT,			/* initial type/class word */
    SC_NONE,			/* initial storage class */
    DCL_TOP,			/* initial context */
    1,				/* typedef ok */
    SY_NORMAL,			/* initial table to use for lookups */
    SY_NOSYM,			/* no initial symbol */
    TY_NONE,			/* no base type */
    0,				/* enum value */
    SY_NOSYM,			/* current saved s/u/e tag */
};

#ifndef	INI_DCLSTK
#define INI_DCLSTK 10		/* initial depth of decl stack */
#endif

TABLE(Static, dclstk, static, in_dclstk,
		struct decl, INI_DCLSTK, 0, "decl stack");
#define ty_dclstk struct decl

/* Declaration tree nodes (declnodes) are used exclusively to
** build the trees that describe types.  declnodes are referred
** to by their index number, which is called a DN.
*/

/* declnode operators */

#define	DN_PTR	1	/* pointer to */
#define	DN_ARY	2	/* array of */
#define	DN_FTN	4	/* function returning */
#define	DN_PARAM 5	/* parameter node */
#define	DN_VPARAM 6	/* equivalent to ... */
#define	DN_QUAL	7	/* type qualifiers */
#define	DN_MBR	8	/* struct/union member */
#define	DN_ENUM	9	/* enumeration constant */

struct declnode {
    I16 dn_op;			/* declaration operator */
    DN dn_next;			/* successor node */
    union {
	DN dn_parmlist;		/* parameter list for DN_FTN node */
	SIZE dn_arysize;	/* size of array */
	int dn_qualbits;	/* TY_CONST, TY_VOLATILE, TY_NOALIAS bits */
	struct {
	    T1WORD dn_ptype;	/* parameter type word */
	    SY_CLASS_t dn_pclass; /* storage class for parameter */
	    SX dn_psid;		/* parameter symbol index */
	} dn_param;		/* parameter information */
	struct {
	    T1WORD dn_sutype;	/* struct/union member type */
	    SX dn_susym;	/* struct/union member symbol */
	    int dn_suisfield;	/* struct/union is field */
	    CONVAL dn_sufldsize; /* struct/union field size */
	} dn_soru;
	struct {
	    SX dn_esym;		/* enumeration symbol ID */
	} dn_enum;
    } dn_var;			/* variable information for node */
};

#if !defined(INI_DCLNODE) || INI_DCLNODE < 2
#undef	INI_DCLNODE
#define	INI_DCLNODE 50		/* need to have at least index 1 avail. */
#endif

TABLE(Static, dclnode, static, in_dclnode,
		struct declnode, INI_DCLNODE, 0, "decl nodes");
#define	ty_dclnode struct declnode
#define	DD(i) (TD_ELEM(dclnode, ty_dclnode, (i)))
#define DP(i) (DD(i).dn_var.dn_param)
#define DSU(i) (DD(i).dn_var.dn_soru)
#define	DE(i) (DD(i).dn_var.dn_enum)

#ifdef	PACK_PRAGMA
BITOFF	Pack_align;		/* current s/u alignment constraint */
BITOFF	Pack_default;		/* default value of same */
#endif

static T1WORD dcl_end();
static char * dcl_sue_tname();
static int dcl_newold();
#ifndef LINT
static T1WORD dcl_efftype();
#endif
static void dcl_argoff();
static void dcl_formal();
static void dcl_push();
static void dcl_pop();

#ifdef	NODBG
#define	TY_PRINT(t, lev)
#else
static void dcl_print();
static char * dcl_pclass();

#define	TY_PRINT(t, lev) if (d1debug > lev) ty_print(t)
#endif

#ifdef	NODBG
#define	DCL_PRINT(sid, s)
#else
#define	DCL_PRINT(sid, s)	if (d1debug) dcl_print(sid, s)
#endif

static DN dcl_dnparam;		/* decl tree for function's parameter list */
static int dcl_infunc = 0;	/* Level within func's arg list.  This is used
				** to detect a struct/union/enum tag that is
				** defined in a formal parameter declaration:
				** such a tag would be, strictly speaking,
				** inaccessible outside the function definition.
				*/
static int dcl_complain;	/* complain about ANSI 3.5 constraint if non-0 */
static SX  dcl_curfunc;		/* symbol index of current function def. */
static int dcl_oldstyle;	/* old-style function definition if non-0 */
static SX  dcl_sameas;		/* used to specify an SID that a new SID
				** should be marked "same as"
				*/
typedef I32 action_t;		/* for passing actions in dcl_redecl() */

/* This is an array of symbol indices for declared arguments
** to the function that is currently being declared.
*/
#ifndef	INI_ARGSZ
#define	INI_ARGSZ	15
#endif

TABLE(Static, td_arginfo, static, arginfo_init,
		SX, INI_ARGSZ, 0, "arg symbol indices");
#define ARG(i) (TD_ELEM(td_arginfo,SX,(i)))
/* Number of args declared for current func. */
#define	dcl_nargs (TD_USED(td_arginfo))


/* This is a character that is illegal in normal C identifiers,
** but that the assembler will accept for identifiers.
*/
#ifndef	DCL_ILL_ID_CHAR
#define	DCL_ILL_ID_CHAR		'.'
#endif


#define	TB(x) (1 << (x))		/* select bit for type */
/* These make life easier:  */
#define	TB2(x,y) (TB(x)|TB(y))
#define	TB3(x,y,z) (TB(x)|TB(y)|TB(z))

/* This array contains bits that are valid if already set when
** we are presented with a new type.
*/

/* First, make sure we really know what the values of the symbols
** are.
*/

#if KT_CHAR	!= 1 \
 || KT_CONST	!= 2 \
 || KT_DOUBLE	!= 3 \
 || KT_ENUM	!= 4 \
 || KT_FLOAT	!= 5 \
 || KT_INT	!= 6 \
 || KT_LONG	!= 7 \
 || KT_NOALIAS	!= 8 \
 || KT_SHORT	!= 9 \
 || KT_SIGNED	!= 10 \
 || KT_STRUCT	!= 11 \
 || KT_UNION	!= 12 \
 || KT_UNSIGNED	!= 13 \
 || KT_VOID	!= 14 \
 || KT_VOLATILE	!= 15 \
 || KT_TYPENAME	!= 16
#include "array does not correspond to definitions"
#endif

#define KT_ALLBIT	(	/* All type keyword bits */ \
		  TB(KT_CHAR)		\
		| TB(KT_CONST)		\
		| TB(KT_DOUBLE)		\
		| TB(KT_ENUM)		\
		| TB(KT_FLOAT)		\
		| TB(KT_INT)		\
		| TB(KT_LONG)		\
		| TB(KT_NOALIAS)	\
		| TB(KT_SHORT)		\
		| TB(KT_SIGNED)		\
		| TB(KT_STRUCT)		\
		| TB(KT_UNION)		\
		| TB(KT_UNSIGNED)	\
		| TB(KT_VOID)		\
		| TB(KT_VOLATILE)	\
		| TB(KT_TYPENAME)	\
			)

#define	KT_QUAL	TB3(KT_CONST,KT_NOALIAS,KT_VOLATILE)
#define	KT_SUN	TB2(KT_SIGNED,KT_UNSIGNED)


void
dcl_start()
/* Start of declaration.  Initialize for a new one.
** Called from the grammar just before we see declaration
** (type and/or class) specifiers.
** If this is a declaration at top level, we must set up
** a few things:
**	1.  Reset the table of declaration nodes to reuse them.
**	2.  Prepare to complain about useless declarations.
**	3.  Indicate there's no s/u/e tag to complain about later.
** Some of this work anticipates the fact that there might be no
** declaration specifiers.
*/
{
    DEBUG(d1debug > 1, ("start of declaration\n"));
    dcl_state.dcl_tybit = NOTYBIT;	/* no type bits */
    dcl_state.dcl_class = SC_NONE;	/* no storage class */
    dcl_state.dcl_btype = TY_INT;	/* default base type */
    dcl_state.dcl_sid = SY_NOSYM;	/* no symbol ID yet */
    dcl_state.dcl_savetag = SY_NOSYM;	/* no saved s/u/e tag */
    dcl_set_type();			/* type name ok */
    /* need this for very first decl! */
    switch( dcl_state.dcl_context ) {
    case DCL_TOP:
	TD_USED(dclnode) = 1;		/* Reset declaration nodes */
	/*FALLTHRU*/
    case DCL_FORMAL:
	dcl_complain = 1;		/* assume complaint for useless */
    }
    return;
}

void
dcl_tycl(tycl)
/* Add a type/class declaration to the current default type/class.
** Also, check for mismatches.
*/
int tycl;
{
    static int dcl_chktype();

    /* look for types first */
    if (tycl >= KT_FIRST && tycl <= KT_LAST) {
	/* Can't have repeat, must be one of allowed types. */
	if (   dcl_state.dcl_tybit != NOTYBIT
	    && dcl_chktype(tycl, dcl_state.dcl_tybit, NOTYBIT)
	)
	    UERROR("invalid type combination");
	/* Catch non type qualifiers in pointer later. */
	dcl_state.dcl_tybit |= TB(tycl);
    }
    /* look for storage class next */
    else if ((SY_CLASS_t) tycl >= SC_FIRST && (SY_CLASS_t) tycl <= SC_LAST) {
	if (dcl_state.dcl_tybit != NOTYBIT && verbose)
	    WERROR("storage class after type is obsolescent");
	if (dcl_state.dcl_class != SC_NONE)
	    UERROR("only one storage class allowed");
	/* Check for storage classes in wrong places:
	**	auto, typedef, static, extern permitted only at DCL_TOP;
	**	asm permitted at DCL_TOP only at symbol level SL_EXTERN;
	**	only register (and none) permitted at DCL_FUNC;
	**	register not permitted at symbol level SL_EXTERN
	*/
	else {
	    switch( dcl_state.dcl_context ) {
	    case DCL_TOP:
		if (    (   (tycl == SC_AUTO || tycl == SC_REGISTER)
			 && sy_getlev() == SL_EXTERN
			)
		    || (tycl == SC_ASM && sy_getlev() != SL_EXTERN)
		) {
		    UERROR("auto/register/asm inappropriate here");
		    tycl = SC_NONE;
		}
		break;
	    
	    case DCL_FUNC:
	    case DCL_FORMAL:
	    {
		static const char mesg[] =
		    "only \"register\" valid as formal parameter storage class"; /*ERROR*/

		/* Only "register" or no storage class are allowed.
		** UNIX C allowed typedef!  Warn about typedef.
		*/
		if (tycl != SC_NONE && tycl != SC_REGISTER) {
		    if (tycl == SC_TYPEDEF && dcl_state.dcl_context == DCL_FORMAL)
			WERROR(mesg);
		    else {
			UERROR(mesg);
			tycl = SC_NONE;
		    }
		}
		break;
	    }

	    default:
		cerror("dcl_tycl() confused about class %d, context %d",
			tycl, dcl_state.dcl_context);
	    }
	}
	dcl_state.dcl_class = (SY_CLASS_t) tycl;
    }
    /* otherwise, bad value */
    else
	cerror("illegal type/class value");
    
    return;
}


static int
dcl_chktype(kt, tybit, tdbit)
int kt;
TYCL tybit;
TYCL tdbit;
/* Check whether keyword type kt conflicts with existing type
** bits tybit.  If kt is KT_TYPENAME, tdbit contains the typedef's
** type bits.  UNIX C allowed typedefs with unsigned/long/short
** modifiers, while ANSI C does not.  Allow for suitable
** compatibility.
** In function prototype declarations, compatibility is not an issue.
** Return 1 on conflict, else 0.
*/
{
    int i;
    int quals;

    /* This table contains the COMPLEMENT of bits that are allowed
    ** in combination with the indicated type.  That is, it contains
    ** the bits that must NOT be set already.
    */
    static const struct { TYCL bits; const char * name; } badtybit[KT_LAST+1] = {
		0, "",				/* no zero-th entry */
/* char 1 */	~( KT_QUAL | KT_SUN ), "char",
/* const 2 */	~( KT_ALLBIT & ~TB(KT_CONST) ), "const", /* all but const */
/* double 3 */	~( TB(KT_LONG) | KT_QUAL ), "double",
/* enum 4 */	~( KT_QUAL ), "enum",
/* float 5 */	~( TB(KT_LONG) | KT_QUAL ), "float", /* long, for compatibility */
/* int 6 */	~( TB(KT_LONG) | TB(KT_SHORT) | KT_QUAL | KT_SUN ), "int",
/* long 7 */	~( TB(KT_DOUBLE) | TB(KT_FLOAT) | TB(KT_INT) | KT_QUAL | KT_SUN ),
			"long",
/* noalias 8 */	~( KT_ALLBIT & ~TB(KT_NOALIAS) ), "noalias", /* all but noalias */
/* short 9 */	~( TB(KT_INT) | KT_QUAL | KT_SUN ), "short",
/* signed 10 */	~( TB(KT_CHAR) | TB(KT_INT) | TB(KT_SHORT) | TB(KT_LONG) | KT_QUAL ),
			"signed",
/* struct 11 */ ~( KT_QUAL ), "struct",
/* union 12 */	~( KT_QUAL ), "union",
/* unsigned 13 */ ~( TB(KT_CHAR) | TB(KT_INT) | TB(KT_SHORT) | TB(KT_LONG) | KT_QUAL ),
			"unsigned",
/* void 14 */	~( KT_QUAL ), "void",		/* allow const void *, etc. */
/* volatile 15 */ ~( KT_ALLBIT & ~TB(KT_VOLATILE) ),	/* all but volatile */
			"volatile",
/* type-name 16 */ ~( 0 ), "typedef",	/* handle typedef+qualifiers special */
};

    if ( (badtybit[kt].bits & tybit) == 0)
	return(0);			/* no conflict */

    /* Type mismatch.  Interesting cases are when the old or
    ** new type is a typedef.  That is, we could have
    ** "typedef int INT;" and then see one of these cases:
    **		unsigned INT foo;	(modifier, then typedef)
    **		INT unsigned foo;	(typedef, then modifier)
    **
    */
    if ((tybit & TB(KT_TYPENAME)) == 0 && kt != KT_TYPENAME)
	return(1);			/* neither was typedef */

    /*
    ** Flag combinations of qualifiers and typedefs if the
    ** typedef was already so-qualified.
    */
    if (kt != KT_TYPENAME)
	tdbit = TB(kt);
    if ((quals = tdbit & tybit & KT_QUAL) != 0) {
	/* Have conflicting qualifiers. */
	static const char mesg[] =
		"typedef already qualified with \"%s\""; /*ERROR*/

	if (quals & TB(KT_CONST))
	    WERROR(mesg, "const");
	if (quals & TB(KT_VOLATILE))
	    WERROR(mesg, "volatile");
	if (quals & TB(KT_NOALIAS))
	    WERROR(mesg, "noalias");
	return( 0 );
    }
    if (kt == KT_TYPENAME && (tybit & ~KT_QUAL) == 0)
	return( 0 );			/* qualifiers (only) precede typedef */

    /* Check new type's bits to see if they are compatible
    ** with the existing type.  That is, see if the modifiers
    ** would be compatible if they weren't part of a typedef.
    ** Need to try each of the typedef's bits individually.
    */

    tybit &= ~TB(KT_TYPENAME);
    for (i = KT_FIRST; i <= KT_LAST; ++i) {
	if ((TB(i) & tdbit) && ((badtybit[i].bits & tybit) != 0))
	    return(1);			/* new conflicts with old */
    }

    /* Reaching here, we had a combination of a typedef and type
    ** modifiers that would have been okay if the types in the
    ** typedef had been expressed explicitly, rather than as part
    ** of a typedef.  CI4.1 allowed this behavior and treated the
    ** combination as if the modifiers had been applied to the
    ** typedef.  ANSI C only allows type qualifiers.
    */

    /* Check for use within function prototype which, being new,
    ** is not a compatibility issue.
    */
    if (dcl_infunc && ! dcl_oldstyle)
	return( 1 );

    /* Put out message(s) that mention the inappropriate modifiers. */
    if (kt != KT_TYPENAME)
	tybit = tdbit;
    for (i = KT_FIRST; i <= KT_LAST; ++i) {
	if (TB(i) & tybit) {
	    static const char mesgold[] =
	    "modifying typedef with \"%s\"; only qualifiers allowed"; /*ERROR*/
	    static const char mesgnew[] =
	    "ANSI C behavior differs; not modifying typedef with \"%s\""; /*ERROR*/
	    WERROR((version & V_CI4_1) ? mesgold : mesgnew, badtybit[i].name);
	}
    }
    return((version & V_CI4_1) == 0);
}



char *
dcl_type(id)
/* Recognize a typedef.  If the typedef would conflict with
** types that are already present, return a pointer to the
** identifier's name string.  Otherwise, set the current type
** and return (char *) 0.
** The context can only be before any * modifier and is not
** a parenthesized identifier, because dcl_oktype() prevents
** other cases.
** Detect an s/u/e typedef that is used in a prototype declaration
** and the tag symbol is not at file scope.
*/
SX id;
{
    T1WORD t;

    if (   dcl_state.dcl_tybit != NOTYBIT
	&& dcl_chktype(KT_TYPENAME, dcl_state.dcl_tybit, SY_OFFSET(id))
       )
	return( SY_NAME(id) );		/* typedef conflicts with other types */

    dcl_state.dcl_tybit |= SY_OFFSET(id) | TB(KT_TYPENAME);
    t  = SY_TYPE(id);
    dcl_state.dcl_btype = t;
    SY_FLAGS(id) |= SY_REF;		/* symbol had reference */
    if (dcl_infunc && TY_ISSUE(t) && SY_LEVEL(TY_SUETAG(t)) != SL_EXTERN)
	WERROR("dubious reference to %s typedef: %s",
		dcl_sue_tname(TY_TYPE(t)), SY_NAME(id));
    DEBUG(d1debug > 1, ("dcl_type:  saw typedef, type %d, bits %#o\n",
	dcl_state.dcl_btype, dcl_state.dcl_tybit));
    return( (char *) 0 );
}


int
dcl_oktype()
/* Return 1 if it's okay for the scanner to return a typedef
** name, based on the current declaration context, 0 otherwise.
** Usually it IS okay, but when the current state is parenthesized,
** or when we're in the context of modifiers to a pointer,
** we MUST be looking for the declarator.
*/
{
    int retval =  dcl_state.dcl_typeok;
    DEBUG(d1debug,("dcl_oktype() returns %d\n", retval));
    return( retval );
}


char *
dcl_name(sid)
SX sid;
/* Return name part of type name. */
{
    if (SY_CLASS(sid) != SC_TYPEDEF)
	cerror("confused dcl_name()");
    return( SY_NAME(sid) );
}


static void
dcl_push(newstate)
I16 newstate;
/* This routine pushes the current declaration context and enters
** a new context.
*/
{
    DEBUG(d1debug > 1, ("dcl_push():  pushing context %d, starting context %d\n",
		dcl_state.dcl_context, newstate));

    TD_NEED1(dclstk);			/* need at least one stack element */
    
    TD_ELEM(dclstk, ty_dclstk, TD_USED(dclstk)) = dcl_state;
    TD_USED(dclstk)++;

    TD_CHKMAX(dclstk);

    dcl_state.dcl_tybit = NOTYBIT;
    dcl_state.dcl_class = SC_NONE;
    dcl_state.dcl_context = newstate;
    dcl_state.dcl_sid = SY_NOSYM;
    dcl_state.dcl_savetag = SY_NOSYM;
    /* Leave base type alone.  It gets changed when necessary. */
    /* Also leave current state of dcl_typeok alone. */
    return;
}

static void
dcl_pop()
/* dcl_pop() restores the previous declaration context.
** Bubble a name outward from ()'s
*/
{
    if (TD_USED(dclstk) <= 0)
	cerror("declaration stack underflow");
    
    TD_USED(dclstk)--;

    dcl_state = TD_ELEM(dclstk, ty_dclstk, TD_USED(dclstk));
    DEBUG(d1debug > 1,
	("dcl_pop():  restoring context %d\n", dcl_state.dcl_context));
    return;
}

void
dcl_e_ds()
/* End of declaration specifier.  Record base type if it's an
** arithmetic type.  Set current storage class.
*/
{
    register TYCL tycl = dcl_state.dcl_tybit;
    T1WORD btype;			/* calculated base type */

    DEBUG(d1debug > 1, ("end declaration_specifiers, value = %#o\n",
		dcl_state.dcl_tybit));

    /* If there was a typedef, struct, union, or enum, use it
    ** for base type.  (For CI 4.1 compatibility, disregard
    ** typedef type for numeric types, since we may also have
    ** other bits.)  (dcl_state.dcl_btype is already set for typedefs.)
    ** Otherwise figure out what the base type is from the other
    ** stuff supplied.  Update tycl in case we're creating a
    ** typedef and its type must be saved for later.
    */

    if (   (   (tycl & TB(KT_TYPENAME))
	   && !((version & V_CI4_1) && TY_ISNUMTYPE(dcl_state.dcl_btype))
	   )
	|| (tycl & TB3(KT_STRUCT, KT_UNION, KT_ENUM)) != 0
	)
	btype = dcl_state.dcl_btype;
    else {
    /* Form T1WORD for base type, presumably a numeric scalar or void.
    ** Strip qualifiers, typedef, int bits.  (int is default.)
    ** "short int" (e.g.) collapses to "short".
    */
	switch( tycl & ~(KT_QUAL|TB(KT_TYPENAME)|TB(KT_INT)) ) {
	case TB(  KT_VOID		):	btype = TY_VOID; break;

	case TB(  KT_FLOAT		):	btype = TY_FLOAT; break;
	case TB2( KT_FLOAT, KT_LONG	):
		/*STRICT*/
		WERROR("use \"double\" instead of \"long float\"");
		/* Update in case of typedef. */
		tycl |= TB(KT_DOUBLE);
		tycl &= ~(TB(KT_FLOAT)|TB(KT_LONG));
		/*FALLTHRU*/
	case TB(  KT_DOUBLE		):	btype = TY_DOUBLE; break;
	case TB2( KT_DOUBLE, KT_LONG	):	btype = TY_LDOUBLE; break;

	case TB(  KT_CHAR		):	btype = TY_CHAR; break;
	case TB2( KT_CHAR, KT_UNSIGNED	):	btype = TY_UCHAR; break;
	case TB2( KT_CHAR, KT_SIGNED	):	btype = TY_SCHAR; break;

	case TB(  KT_SHORT		):	btype = TY_SHORT; break;
	case TB2( KT_SHORT, KT_UNSIGNED	):	btype = TY_USHORT; break;
	case TB2( KT_SHORT, KT_SIGNED	):	btype = TY_SSHORT; break;

	case TB(  KT_LONG		):	btype = TY_LONG; break;
	case TB2( KT_LONG, KT_UNSIGNED	):	btype = TY_ULONG; break;
	case TB2( KT_LONG, KT_SIGNED	):	btype = TY_SLONG; break;

	case TB(  KT_SIGNED		):	btype = TY_SINT; break;
	case TB(  KT_UNSIGNED		):	btype = TY_UINT; break;
	/* "int", or no explicit type. */
	default:				btype = TY_INT; break;
	} /* end switch on types */
    } /* end if */

    /* To handle UNIX C compatibility of typedefs, we have to strip
    ** the "int" bit for types that had an explicit "int" plus other
    ** modifiers.
    */
    if ((tycl & TB(KT_INT)) && tycl != TB(KT_INT))
	tycl &= ~TB(KT_INT);

    /* Do qualifiers.  Base type may already have been qualified
    ** if it was a typedef.  May not use same qualifier again.
    */
    if (tycl & KT_QUAL) {
	int qual = 0;
	if (tycl & TB(KT_CONST)) qual |= TY_CONST;
	if (tycl & TB(KT_VOLATILE)) qual |= TY_VOLATILE;
	if (tycl & TB(KT_NOALIAS)) qual |= TY_NOALIAS;
	btype = ty_mkqual(btype, qual);
    }

    dcl_state.dcl_btype = btype;
    dcl_state.dcl_tybit = tycl;		/* Save updated type bits. */
    DEBUG(d1debug > 1,
	("base type is:  (%d)  ", btype));
    TY_PRINT(btype, 1);
    DEBUG(d1debug > 1, ("\n"));
    return;
}


void
dcl_s_dcor()
/* Get ready for a new declaration.  Finished with type and
** class specifiers, ready for type modifiers and (possible)
** declarator name.
** Reset the declaration nodes if at top level.  This is a
** space optimization.
*/
{
    DEBUG(d1debug > 1, ("start of declarator\n"));
    dcl_state.dcl_sid = SY_NOSYM;
    if (dcl_state.dcl_context == DCL_TOP)
	TD_USED(dclnode) = 1;
}


DN
dcl_dcor(s)
char * s;
/* Set declarator name, meaning set dcl_sid.  Name may be 0
** for abstract declarator.  Create a symbol table entry.
** Function parameters always hide names at lower scopes.
*/
{
    /* Create possibly new symbol entry for current name in
    ** proper table.
    */
    if (s) {
	SX sid = sy_lookup(s, dcl_state.dcl_table, SY_CREATE);

	if (dcl_state.dcl_sid != SY_NOSYM)
	    cerror("overwriting previous symbol id %d for %s",
		dcl_state.dcl_sid, s);

	/* Function parameters always hide previous names. */
	if (   ! SY_ISNEW(sid)
	    && dcl_state.dcl_context == DCL_FUNC
	    && SY_LEVEL(sid) < sy_getlev()
        )
	    sid = sy_hide(sid);

	/* New name is not in scope. */
	dcl_state.dcl_sid = sid;
    }
    DEBUG(d1debug > 1, ("saw declarator name %s, id %d\n", s ? s : "<none>",
		s ? dcl_state.dcl_sid : 0));
    return( DN_NULL );
}

void
dcl_set_type()
/* Signify that it is ok to return a type name.
*/
{
    DEBUG(d1debug > 1, ("dcl_set_type()\n"));
    dcl_state.dcl_typeok = 1;
    return;
}

void
dcl_clr_type()
/* Signify that it is not ok to return a type name.
*/
{
    DEBUG(d1debug > 1, ("dcl_clr_type()\n"));
    dcl_state.dcl_typeok = 0;
    return;
}

void 
dcl_s_ptrtsl()
/* Start a new state.  It is not ok to see type names in the declarator */
{
    dcl_push(dcl_state.dcl_context);
    dcl_clr_type();
    return;
}
 
void
dcl_s_absdcl()
/* Start abstract declarator:  for type name for casts, sizeof.
** Set new context.
*/
{
    dcl_push(DCL_ABS);
    return;
}


T1WORD
dcl_e_absdcl(dn)
DN dn;
/* Return type corresponding to abstract declarator dn.
** Restore declaration stack.
*/
{
    T1WORD retval = dcl_norm(dn, DS_ABS);
    dcl_pop();
    return( retval );
}


static DN
newdn()
/* Obtain a new declnode. */
{
    DN new;				/* new node number */

    TD_NEED1(dclnode);

    new = TD_USED(dclnode)++;

    TD_CHKMAX(dclnode);

    return( new );
}


DN
dcl_ofptr(ptr, at)
DN ptr;
DN at;
/* ptr is presumed to be a pointer declaration tree that ends in
** a DN_NULL.  at is the thing that should have been pointed at.
** Happens after a declarator that is modified by a pointer list.
** Return the resulting declaration tree.
*/
{
    DN tptr;

    if (ptr == DN_NULL) cerror("null pointer tree?");
    /* Find end of list. */
    for (tptr = ptr; DD(tptr).dn_next != DN_NULL; tptr = DD(tptr).dn_next) {
	if (DD(tptr).dn_op != DN_PTR) cerror("non-PTR declnode");
    }
    DD(tptr).dn_next = at;
    return( ptr );
}

    
DN
dcl_ptrto(to)
DN to;					/* what's pointed to */
/* Create a pointer to the pointed-to declnode:  add to the list
** of * modifiers.  But first, validate the type-specifier to be
** sure only qualifiers are present.
*/
{
    DN new = newdn();
    TYCL qualbits = dcl_state.dcl_tybit; /* grab qualifier bits before
					** restoring context
					*/
    int qual = 0;			/* qualifier bits to remember */

    dcl_pop();				/* restore previous context */

    /* only type_specifiers allowed are type qualifiers */
    DEBUG(d1debug > 1, ("%#o pointer to\n", qual));

    if (qualbits & ~KT_QUAL)
	UERROR("only qualifiers allowed after *");
    else {
	if (qualbits & TB(KT_CONST))
	    qual |= TY_CONST;
	if (qualbits & TB(KT_VOLATILE))
	    qual |= TY_VOLATILE;
	if (qualbits & TB(KT_NOALIAS))
	    qual |= TY_NOALIAS;
    }
    
    DD(new).dn_op = DN_PTR;
    DD(new).dn_next = to;
    DD(new).dn_var.dn_qualbits = qual;
    return( new );
}

DN
dcl_arrof(dn, sizep)
DN dn;
ND1 * sizep;
/* Add an "array-of" node to the declaration tree.  Happens
** after seeing declarator[expr].  "sizep" points to (we hope)
** a tree that can resolve to an ICON, or is NIL if there is a
** null size (e.g., int i[];).
*/
{
    DN new = newdn();
    CONVAL size = (CONVAL) TY_NULLDIM;

    DD(new).dn_op = DN_ARY;
    DD(new).dn_next = dn;
    if (sizep) {
	T1WORD contype = sizep->type;	/* type of integral expression */
	CONVAL tsize = tr_inconex(sizep); /* get expression value */
	static const char mesg[] = "zero or negative subscript"; /*ERROR*/

	/* Zero size is arguably allowable.  Negative is not. */
	if (tsize == 0) {
	    WERROR(mesg);
	    size = (CONVAL) TY_ERRDIM;
	}
	else if (TY_ISSIGNED(contype) && tsize < 0) {
	    UERROR(mesg);
	    size = (CONVAL) TY_ERRDIM;
	}
	else if (tsize == TY_NULLDIM || tsize == TY_ERRDIM) {
	    /* This is a HACK.  Assume these two values are very large,
	    ** such that specifying them would lead to a too-big array.
	    */
	    UERROR("array too big");
	    size = (CONVAL) TY_ERRDIM;
	}
	else
	    size = tsize;
    }

    DEBUG(d1debug > 1, ("array[%ld] of\n", (long) size));
    DD(new).dn_var.dn_arysize = size;
    return( new );
}


/* Function handling works something like this:
** 	1) See the function declarator, possibly with ANSI-style
**	parameters.
**	2) Possibly see old-style parameter declarations.
**	3) See the { that denotes the beginning of a function
**	definition.
**
** During step 1, each ( that is part of a function declarator
** introduces a new scope (bumps the symbol table level).  At
** the ), the scope is exited EXCEPT if the scope that's about
** to be exited is the level corresponding to parameters to a
** top-level function declarator.  The reason is that we want
** to retain the parameters in case we're about to do a function
** definition (and it's too early to tell when we see the ')').
**
** New- and old-style parameters are strung together by declaration
** nodes.  When we see what looks like it could be a top-level
** function definition, the declaration node tree that corresponds
** to the parameters gets saved in dcl_dnparam.  That way, new- and
** old-style parameters can be checked in a uniform way.
**
** These functions participate:
**	dcl_func	create a function-returning node
**	dcl_doparam	create a node for a function parameter
**	dcl_param	create a node for a new-style parameter
**	dcl_ident	create a node for an old-style parameter (name)
**	dcl_vparam	handle ",..."
**
** Nodes for parameters are collected left to right.
*/


DN
dcl_func(fun, parms)
DN fun;
DN parms;
/* Create a function-returning declaration node.  "fun" is he declaration
** tree for the return-type and "parms" is the list of declaration trees
** for the parameters.  dcl_func() gets called at the ')' after all of
** the parameters have been seen.
*/
{
    DN new = newdn();

    DEBUG(d1debug > 1, ("function returning\n"));
    DD(new).dn_op = DN_FTN;
    DD(new).dn_next = fun;
    DD(new).dn_var.dn_parmlist = parms;
    return( new );
}


/* The following few functions are related to building a function
** parameter declaration node tree.  Part of the fun here is to
** catch various user errors.
*/

static DN
dcl_doparam(idtype, pclass)
T1WORD idtype;
SY_CLASS_t pclass;
/* Create a parameter declaration node.  The identifier's ID is
** assumed to be in dcl_state.dcl_sid.  Its type will be idtype.
** If idtype is TY_NONE, this is an old-style parameter whose
** type is presumed to be int.  In either case the presumed
** storage class is pclass.
*/
{
    DN new = newdn();			/* get another node */
    SX sid = dcl_state.dcl_sid;		/* symbol index, if any */

    DD(new).dn_op = DN_PARAM;
    DD(new).dn_next = DN_NULL;
    DP(new).dn_pclass = pclass;

    DP(new).dn_ptype = idtype;		/* TY_NONE flags old-style */

    /* Parameters always hide earlier definitions. */
    if (sid != SY_NOSYM) {
	if (!SY_ISNEW(sid)) {
	    if (SY_LEVEL(sid) == sy_getlev())
		UERROR("parameter redeclared: %s", SY_NAME(sid));
	    sid = sy_hide(sid);
	}
	SY_TYPE(sid) = (idtype == TY_NONE ? TY_INT : idtype);
	SY_CLASS(sid) = SC_PARAM;
	if (pclass == SC_REGISTER)
	    SY_FLAGS(sid) |= SY_ISREG;
	SY_FLAGS(sid) |= SY_INSCOPE;	/* Parameter name now in scope. */
#ifdef FAT_ACOMP
	SM_WT_INIT(sid);		/* Set initial variable weight. */
#endif
#ifdef LINT
	/* Write out the parameter for cxref. */
	if (LN_FLAG('R'))
	    cx_ident(sid, 1);
#endif
    }
    DCL_PRINT(sid, "parameter");
    DP(new).dn_psid = sid;
    DEBUG(d1debug > 1, ("\n"));
    return( new );
}


DN
dcl_param(dn)
DN dn;
/* Build DN_PARAM node from current name, declaration tree represented
** by dn.  Called at the end of a prototype parameter declaration.
*/
{
    T1WORD t;

    /* Compatibility:  check for proper use of s/u/e tag. */
    dcl_chkend(dcl_state.dcl_sid == SY_NOSYM ? D_ABSDECL : D_DECL);

    /* End of declaration stuff:  build parameter type. */
    t = dcl_end(dn, 0);

    /* Build parameter with idtype.  Class is whatever user supplied. */
    return( dcl_doparam(t, dcl_state.dcl_class) );
}


DN
dcl_vparam()
/* Build DN_VPARAM (variable parameter) node, corresponding to ... .
** Create a symbol table entry for "symbol" "...".
*/
{
    DN new;
    char * s = st_nlookup("...", 4);	/* symbol name */
    SX sid;

    DEBUG(d1debug > 1, ("parameter ...\n"));

    /* Simulate declarator stuff.  Pretend the type is int
    ** initially.  (It makes life easier.)  After we get a
    ** symbol table entry, change the type to (void *).
    */
    dcl_start();
    dcl_s_dcor();
    new = dcl_param(dcl_dcor(s));	/* make a DN_PARAM node */
    DD(new).dn_op = DN_VPARAM;		/* make it into a DN_VPARAM */
    sid = DP(new).dn_psid;		/* get symbol index */
    SY_TYPE(sid) = TY_VOIDSTAR;
    return( new );
}


DN
dcl_ident(str)
char * str;
/* Build parameter node from just identifier (for old-style
** function definition.  str is the identifier string.
*/
{
    dcl_state.dcl_sid = sy_lookup(str, SY_NORMAL, SY_CREATE);

    /* Detect when we are in the midst of a potential old-style
    ** function definition.  Can only be if current scope level
    ** is function argument level.
    */
    if (sy_getlev() == SL_FUNARG)
	dcl_oldstyle = 1;

    /* Indicate old-style parameter.  Parameter storage class
    ** is unknown (NONE), type is unknown.
    */
    return( dcl_doparam(TY_NONE, SC_NONE) );
}


DN
dcl_plist(left, right)
DN left;
DN right;
/* Build decl node list of function parameters.  left is assumed
** to be the current parameter list, right is assumed to be a
** single parameter node.  Check for valid parameter types.  In
** particular, disallow void if not first param.  The left side
** is DN_NULL when the list is first created.
** Check for ... as first parameter if strictly conforming.
**
** The list reads right to left.
*/
{
    DN dn;
    static const char mesg[] = "\"void\" must be sole parameter"; /*ERROR*/

    if (left == DN_NULL) {
	if (   DD(right).dn_op == DN_VPARAM
	    && ((version & V_STD_C) != 0 || verbose)
	)
	    WERROR("ANSI C requires formal parameter before \"...\"");
	return( right );
    }

    if (TY_TYPE(DP(left).dn_ptype) == TY_VOID) {
	UERROR(mesg);
	DP(left).dn_ptype = TY_INT;	/* make it int to carry on */
    }
    if (TY_TYPE(DP(right).dn_ptype) == TY_VOID) {
	UERROR(mesg);
	DP(right).dn_ptype = TY_INT; /* make it int to carry on */
    }

    /* Find end of left's list, add right.  This is assumed to be
    ** relatively cheap because few functions have many formals.
    */
    for (dn = left; DD(dn).dn_next != DN_NULL; dn = DD(dn).dn_next)
	;
    DD(dn).dn_next = right;
    return( left );
}



static T1WORD
dcl_end(dn, isfuncdef)
DN dn;
int isfuncdef;
/* Do end-of-declaration stuff.  Build a type from the declaration nodes.
** If the declarator is at top level but isn't a function definition,
** and if the symbol level looks like it's at SL_FUNARG, reset it to
** flush any parameters.
**
** dcl_end() handles the transformation of types for parameters.  It
** must be done in two places:
**	1) The set of declaration nodes is changed, to avoid spurious
**		messages for types that would otherwise be illegal:
**			int ia[][];
**	2) The resulting type must also be checked, in case the parameter
**		type is the result of a typedef:
**			typedef int jmpbuf[6];
**			int setjmp(jmpbuf);
**
** We are processing a parameter if the current context is either
** DCL_FUNC (prototype parameter) or DCL_FORMAL (old-style formal parameter).
**
** The procedure works by assuming the base type at the top and
** descending the tree in the forward direction.  This has the
** effect of building the type inside-out.
** When building a function-returning, check whether identifier-list
** is appropriate, which is only true on top level function-returning
** and isfuncdef.
*/
{
    static void dcl_bdparms();
    T1WORD rettype = dcl_state.dcl_btype;
    int isparam = 0;			/* assume not parameter */
    DN parms;
    int qual;

    /* Take care of symbol level. */
    if (   dcl_state.dcl_context == DCL_TOP
	&& !isfuncdef
	&& sy_getlev() == SL_FUNARG
	)
	sy_declev();
    
    /* Set flag if processing type for parameter. */
    switch( dcl_state.dcl_context ){
    case DCL_FUNC:
    case DCL_FORMAL:
	isparam = 1;
	break;
    }
    /* Adjust the last modifier if necessary. */
    if (dn && isparam) {
	DN tptr;
	DN new;

	tptr = dn;
	while (DD(tptr).dn_next != DN_NULL)
	    tptr = DD(tptr).dn_next;

	switch( DD(tptr).dn_op ){
	case DN_FTN:
	    /* Add pointer node following function node. */
	    new = newdn();
	    DD(tptr).dn_next = new;
	    tptr = new;
	    DD(tptr).dn_next = DN_NULL;
	    /*FALLTHRU*/
	case DN_ARY:
	    /* Change array node to pointer node. */
	    DD(tptr).dn_op = DN_PTR;
	    DD(tptr).dn_var.dn_qualbits = 0;
	    break;
	}
    }

    /* Now build the type. */
    while (dn != DN_NULL) {
	switch( DD(dn).dn_op ) {
	case DN_PTR:
	    rettype = ty_mkptrto( rettype );
	    if ((qual = DD(dn).dn_var.dn_qualbits) != 0)
		rettype = ty_mkqual(rettype, qual);
	    break;
	case DN_ARY:
	    switch( TY_TYPE(rettype) ){
		/* Pick up zero-sized arrays and incomplete s/u/e
		** when we know we need a size.
		*/
	    case TY_ARY:
		/* int ia[4][] */
		if (TY_ISARY(rettype) && TY_NELEM(rettype) == TY_NULLDIM) {
		    char * s =
			dcl_state.dcl_sid == SY_NOSYM ?
				"<noname>" : SY_NAME(dcl_state.dcl_sid);
		    UERROR("null dimension: %s", s);
		}
		break;
	    case TY_VOID:
	    case TY_FUN:
		UERROR("cannot declare array of functions or void");
		rettype = TY_INT;
		break;
	    }
	    rettype = ty_mkaryof( rettype, DD(dn).dn_var.dn_arysize );
	    break;
	case DN_FTN:
	    if (TY_ISARY(rettype) || TY_ISFTN(rettype)) {
		UERROR("function cannot return function or array");
		rettype = TY_INT;
	    }
	    rettype = ty_mkfunc( rettype );
	    /* Build parameter list, if appropriate. */
	    parms = DD(dn).dn_var.dn_parmlist;
	    DD(dn).dn_var.dn_parmlist = parms;
	    /* Only build parameter list if not identifier-list */
	    if (parms != DN_NULL) {
		if (DD(parms).dn_op == DN_PARAM && DP(parms).dn_ptype == TY_NONE) {
		    /* have identifier list */
		    if (!isfuncdef || DD(dn).dn_next != DN_NULL)
			WERROR("function prototype parameters must have types");
		}
		else
		    dcl_bdparms(rettype, parms);
	    }
	    break;
	default:	cerror("can't handle DN op %d\n", DD(dn).dn_op);
	}
	dn = DD(dn).dn_next;
    }

    /* Change parameter types, if necessary, to change array-of and
    ** function-returning.
    */
    if (isparam) {
	switch( TY_TYPE(rettype) ){
	case TY_ARY:
	    rettype = TY_DECREF(rettype);
	    /*FALLTHRU*/
	case TY_FUN:
	    rettype = ty_mkptrto(rettype);
	    break;
	}
    }

    /* Check for improper uses of void. */
    if (TY_TYPE(rettype) == TY_VOID) {
	if (TY_ISQUAL(rettype) && dcl_state.dcl_class != SC_TYPEDEF)
	    WERROR("inappropriate qualifiers with \"void\"");
	/* Check for named void parameter. */
	if (isparam && dcl_state.dcl_sid != SY_NOSYM) {
	    UERROR("void parameter cannot have name: %s",
			SY_NAME(dcl_state.dcl_sid));
	    rettype = TY_INT;		/* force type to proceed */
	}
    }

    return( rettype );
}


static void
dcl_bdparms(fun, parms)
T1WORD fun;				/* function to add parameter to */
DN parms;				/* parameter list tree */
/* Add parameters to a function-returning type.  The declaration tree
** is assumed to be in left-to-right order.  Reverse them first before
** calling ty_mkparm() to add types to the function-returning type.
** This type information is not hidden.
*/
{
    for ( ; parms != DN_NULL; parms = DD(parms).dn_next) {
	if (DD(parms).dn_op == DN_VPARAM)
	    ty_mkparm(fun, TY_DOTDOTDOT, 0); /* add ... to function */
	else
	    ty_mkparm(fun, DP(parms).dn_ptype, 0);
    }
    return;
}


void
dcl_f_lp()
/* Do what needs to be done when the ( that marks the beginning
** of a function declarator is seen.  Initialize for old-style
** definition check.
*/
{
    ++dcl_infunc;		/* note we're within the scope of a func. */
    if (dcl_state.dcl_context == DCL_TOP)
	dcl_oldstyle = 0;	/* assume non old-style func. for now */
    dcl_push(DCL_FUNC);		/* change context */
    dcl_set_type();		/* can see a type name */
    dcl_state.dcl_table = SY_NORMAL;
				/* function parameters are in normal table */
    sy_inclev();		/* enter new symbol level */
    
    return;
}


void
dcl_f_rp()
/* Do what needs to be done when the ) that marks the end of a
** function declarator is seen.
*/
{
    if (dcl_state.dcl_context != DCL_FUNC)
	cerror("confused dcl_f_rp()");

    --dcl_infunc;		/* departing one level of function */
    dcl_pop();
    /* Decrement the symbol scope, but not below SL_FUNARG,
    ** in case we're in a function definition.  That could
    ** only be so at top level, with scope level SL_FUNARG.
    ** Context could be non-DCL_TOP if a function prototype
    ** appears in a s/u/e decl. or as a parameter in an outer
    ** function prototype.
    */
    if (dcl_state.dcl_context != DCL_TOP || sy_getlev() > SL_FUNARG)
	sy_declev();
    return;
}

void
dcl_lp()
/* Note a left paren in a normal declarator.  Mostly we
** must start a new context.  Actually, we continue the
** current context, but at a new level.
*/
{
    dcl_push( dcl_state.dcl_context );
    dcl_clr_type();	/* cannot return a typename */
    DEBUG(d1debug > 1,("dcl_lp()\n"));
    return;
}

void
dcl_rp()
/* Exit a ()'ed declarator.  We return to the previous
** context, but we copy any declarator name we found inside.
*/
{
    SX sid = dcl_state.dcl_sid;

    dcl_pop();
    if (dcl_state.dcl_sid != SY_NOSYM)
	cerror("dcl_rp() overwrites symbol ID %d", dcl_state.dcl_sid);
    dcl_state.dcl_sid = sid;
    return;
}


void
dcl_s_formal()
/* Start of formal declarations that follow a function
** declarator in a function definition.  dcl_s_formal()
** marks the beginning of a possible declaration-list;
** dcl_e_formal() marks the end of same.  The declaration-
** list may be empty.
*/
{
    dcl_push(DCL_FORMAL);
    ++dcl_infunc;		/* similar to function prototype */
    dcl_state.dcl_table = SY_NORMAL;
				/* function parameters are in normal table */
    return;
}


static void
dcl_formal(t)
T1WORD t;
/* Declare current identifier as formal with type t.  The
** parameter list decl tree is assumed to be in dcl_dnparam.
** This gets called for old-style formal declaration:
**	f(i) int i;{}		handle i
**
** UNIX C allowed typedef storage class here.  Don't complain
** about missing argument for typedef, but define the symbol.
** (We will have already warned about the bogus storage class.)
** If the typedef name matches a formal, just set the type and
** class, and remember the type bits, like a normal typedef..
*/
{
    DN dn;
    SX sid = dcl_state.dcl_sid;
    char * name = SY_NAME(sid);

    /* Find the parameter in the parameter list. */
    for (dn = dcl_dnparam; dn != DN_NULL; dn = DD(dn).dn_next) {
	if (DD(dn).dn_op != DN_PARAM) continue;
	if (DP(dn).dn_psid == sid) break;
    }


    if (dn == DN_NULL) {
	SY_CLASS_t class;

	/* Name not in list.  Declare the identifier anyway. */
	if ((class = dcl_state.dcl_class) != SC_TYPEDEF) {
	    UERROR("parameter not in identifier list: %s", name);
	    class = SC_AUTO;
	}
	(void) dcl_defid(sid, t, DS_NORM, class, SL_FUNARG);
	return;
    }

    if (DP(dn).dn_ptype != TY_NONE) {
	UERROR("parameter redeclared: %s", name);
	return;
    }

    if (dcl_state.dcl_class == SC_TYPEDEF) {
	SY_TYPE(sid) = DP(dn).dn_ptype = t;
	SY_CLASS(sid) = DP(dn).dn_pclass = SC_TYPEDEF;
	SY_OFFSET(sid) = dcl_state.dcl_tybit;
	return;
    }

    /* Function can't be asm. */
    if (SY_CLASS(dcl_curfunc) == SC_ASM)
	UERROR("asm definition cannot have old-style parameters");

    SY_TYPE(sid) = DP(dn).dn_ptype = t;
    SY_CLASS(sid) = DP(dn).dn_pclass = SC_PARAM;
    if (dcl_state.dcl_class == SC_REGISTER)
	SY_FLAGS(sid) |= SY_ISREG;

    /* Flag arguments that are declared as float but actually arrive
    ** as doubles.  True only for old-style definitions, since floats
    ** in prototype definitions arrive as float.
    ** Warn -v about small unsigned arguments -- they promote
    ** differently, making it tricky to write prototype declarations
    ** that match under -Xt and -Xa.
    */
    switch( TY_TYPE(t) ){
#ifndef LINT
    /*
    ** lint doesn't want to see this - it will generate an expression
    ** that will set the SY_SET bit on sid (which is undesirable at
    ** this point)
    */
    case TY_FLOAT:
	SY_FLAGS(sid) |= SY_ISDOUBLE;
	break;
#endif
#if 0				/* This was a bad idea. */
    case TY_UCHAR:
    case TY_USHORT:
	if (verbose && TY_SIZE(t) < TY_SIZE(TY_INT))
	    WERROR("parameter promotes differently in ANSI C: %s", name);
	break;
#endif
    }
    DCL_PRINT(sid, "formal");
    DEBUG(d1debug > 1, ("\n"));
    return;
}



void
dcl_e_formal()
/* End of formal declarations (if any) that follow a
** function definition.  Go about getting the parameters
** defined for real, now that we have their types and
** storage classes.  Assign parameter offsets.  Mark the
** parameters as SY_DEFINED so we can distinguish parameters
** in prototype declarations from those in function definitions.
**
** This code is reached after either old-style formal
** parameters have been seen, which is also after new-style
** function prototype parameters have been seen.
** dcl_curfunc is the symbol ID for the function that's
** being defined.  dcl_dnparam is the declaration tree for
** the parameter list for that function.  It has been filled
** in by dcl_formal() and dcl_param() via dcl_doparam().
**
** For a function definition with old-style parameters, two
** special cases arise:
**	1) There was no prior declaration.  We must create hidden
**		parameter type information.
**	2) There was a prior prototype or hidden type information.
**		We must compare the old-style parameters against
**		the remembered types, then keep the defined
**		parameter types as hidden information.
*/
{
    DN dn;
    int paramno;
    T1WORD functype = SY_TYPE(dcl_curfunc);
    T1WORD proto_type = TY_NONE;	/* type to use for prototype checks */
    int nparam = TY_NPARAM(functype);
    SX dddsid = SY_NOSYM;		/* ID of ... , if any */
    int isasm = SY_CLASS(dcl_curfunc) == SC_ASM;
    SX sid;
    int mismatch = 0;
    T1WORD t;
#ifdef LINT
    int knownargs = LN_DIR(VARARGS);	

    /*
    ** # of known args, or -1 if not variable.
    ** If the function has VARARGSn, then the programmer is saying that
    ** there must be at least n args.
    ** If the function has PRINTFLIKEn or SCANFLIKEn, then the programmer
    ** is saying there must be at least n args, and n is a char *.
    **
    ** If none of these is specified, then the value of knownargs will be -1
    */
    if (LN_DIR(VARARGS) != -1)
	knownargs = LN_DIR(VARARGS);
    else if (LN_DIR(PRINTFLIKE) != -1)
	knownargs = LN_DIR(PRINTFLIKE);
    else knownargs = LN_DIR(SCANFLIKE);
#endif

    TD_USED(td_arginfo) = 0;		/* pun for dcl_nargs */

    /* Remember type to use for old-style parameter checks. */
    if (TY_HASPROTO(functype) || TY_HASHIDDEN(functype))
	proto_type = functype;

    /* If there is no parameter information, this is an old-style
    ** definition.  However, dcl_oldstyle only got set if there
    ** was an identifier list in declaration processing.  Take care
    ** of the no parameter case here.
    */
    if (dcl_dnparam == DN_NULL)
	dcl_oldstyle = 1;

    /* Assign default type/storage class to any formal
    ** that needs it, make sure each has a name, build
    ** argument list.
    */
    paramno = 0;
    for (dn = dcl_dnparam; dn != DN_NULL; dn = DD(dn).dn_next, ++paramno) {
	switch( DD(dn).dn_op ) {
	case DN_PARAM:
	    break;			/* expected case */
	case DN_VPARAM:
	    dddsid = DP(dn).dn_psid;	/* remember ID of ... */
	    if (DD(dn).dn_next == DN_NULL) continue; /* to get out of loop */
	    /*FALLTHRU*/
	default:
	    cerror("confused dcl_e_formal(1)");
	}

	t = DP(dn).dn_ptype;		/* this parameter's type */
	if (TY_TYPE(t) == TY_VOID) {
	    if (paramno != 0) cerror("confused dcl_e_formal(2)");
	    break;			/* function def. has no formals */
	}

	sid = DP(dn).dn_psid;
	if (sid == SY_NOSYM) {
	    char pbuf[20];
	    UERROR("formal parameter lacks name: param #%d", paramno+1);
	    /* build an illegal dummy name for later use */
	    sprintf(pbuf, "parameter%c%d", DCL_ILL_ID_CHAR, paramno+1);
	    sid = sy_lookup(st_lookup(pbuf), SY_NORMAL, SY_CREATE);
	    SY_TYPE(sid) = t;
	    if ((SY_CLASS(sid) = DP(dn).dn_pclass) == SC_NONE)
		SY_CLASS(sid) = SC_PARAM;
	}
	if (t == TY_NONE)
	    SY_TYPE(sid) = TY_INT;

	/* Do this here because paramno could be > nparam. */
	TD_NEED1(td_arginfo);
	++dcl_nargs;			/* pun for TD_USED(td_arginfo) */
	TD_CHKMAX(td_arginfo);
	ARG(paramno) = sid;		/* save current parameter */
    }

    /* Now do another pass to check the types of each of the
    ** arguments, check old-style vs. new-style and vice versa.
    */
    for (paramno = 0; paramno < dcl_nargs; ++paramno) {
	char * name;

	sid = ARG(paramno);
	t = SY_TYPE(sid);
	name = SY_NAME(sid);

	/* Check for mismatch between existing prototype and old-style
	** definition.  Build hidden argument information for the function.
	** Do not provide such information for old-style asm definitions,
	** because there is no way to provide parameter types.  Consequently
	** the type information will be wrong.
	*/
	if (dcl_oldstyle && !isasm) {
	    if (proto_type != TY_NONE) {
		if (   paramno < nparam
		    && !dcl_newold(TY_PROPRM(proto_type, paramno), t)
		    ) {
		    WERROR("type does not match prototype: %s", name);
		    mismatch = 1;
		}
	    }
#ifdef LINT
	    /* For lint, only record known good arguments, including
	    ** up to n for VARARGSn
	    */
	    if (knownargs < 0 || paramno < knownargs)
#endif
	    {
		/* Now create hidden old-style parameter information, but
		 * only if we don't have a prototype. 
		 */
	 	if (proto_type == TY_NONE)
		    ty_mkparm(functype, dcl_efftype(t), 1);
	    }
	}

	/* Make sure parameter type has a bonafide size. */
	(void) ty_chksize(t, name, (TY_CVOID | TY_CSUE), -1);
    }
    /* Check for parameter number mismatch.  nparam < 0 means
    ** old-style function-type.
    */
    if (nparam >= 0 && dcl_nargs != nparam) {
	WERROR("parameter mismatch: %d declared, %d defined",
		nparam, paramno);
	mismatch = 1;
    }
    if (mismatch && !isasm) {

	/* We have an old-style declaration conflicting with a
	 * prototype.  Create a new type for this function.  An
	 * error message has already been issued.
	 */
	functype = ty_mkfunc(TY_DECREF(functype));
	SY_TYPE(dcl_curfunc) = functype;
	for (paramno = 0; paramno < dcl_nargs; ++paramno) {
	    sid = ARG(paramno);
	    t = SY_TYPE(sid);

#ifdef LINT
	    /* For lint, only record known good arguments, including
	    ** up to n for VARARGSn
	    */
	    if (knownargs < 0 || paramno < knownargs)
#endif
	    {
	        /* Now create hidden old-style parameter information. */
	        ty_mkparm(functype, dcl_efftype(t), 1);
	    }
	}
    }

    if (dcl_oldstyle && (proto_type == TY_NONE || mismatch)) {
#ifdef LINT
	/* Add varargs information to hidden type information for lint,
	** if appropriate.
	*/
	if (knownargs >= 0)
	    ty_mkparm(functype, TY_DOTDOTDOT, 1);
	else
#endif
	{
	    /* If old-style and no formals, create hidden info showing
	    ** no formals.
	    */
	    if (dcl_nargs == 0)
		ty_mkparm(functype, TY_VOID, 1);
	}
    }

#ifndef	NODBG
    if (dcl_oldstyle)
	DCL_PRINT(dcl_curfunc, "old-style function, after args");
#endif

    /* Allocate argument offsets if not an asm function. */
    if (!isasm)
	dcl_argoff(dddsid);		/* allocate arg. offsets */


    /* Now add ... symbol, if any, to list of args. */
    if (dddsid != SY_NOSYM) {
	TD_NEED1(td_arginfo);
	++dcl_nargs;			/* pun for TD_USED(td_arginfo) */
	TD_CHKMAX(td_arginfo);
	ARG(paramno) = dddsid;		/* save current parameter */
    }

    --dcl_infunc;			/* back out of "in function" */
    dcl_pop();
    return;
}


static int
dcl_newold(tnew, told)
T1WORD tnew;
T1WORD told;
/* Check whether tnew, the existing type in a function prototype,
** is compatible with told, the type in an old-style definition.
** Obviously the prototype declaration appeared before the old-style
** definition.
**
** These are the rules:
**	1)  If both are integral types:
**		a) both are long, or
**		b) prototype is int/uint and old-style promotes to
**		same.
**	    Beware of int/long sizes and enums.
**	2)  If both are floating types:
**		a) both are long double, or
**		b) old-style is float/double, and prototype says
**			double.
**	3)  Otherwise, the types must match.
**
** Return 1 if compatible, 0 if not.
** The verbose flag and -Xc invoke a stricter sense of compatibility.
** For example, long in prototype does not match short in old-style
** definition, even if int and long are same size.
** Ignore qualifiers.
*/
{
    int retval;				/* return value */
    T1WORD etold = dcl_efftype(told);	/* effective type for old-style */

    tnew = TY_UNQUAL(tnew);		/* remove qualifiers */

    if (TY_ISINTTYPE(etold) && TY_ISINTTYPE(tnew)) {
	/* integral/integral case:
	** Types are compatible if the effective type is an exact
	** match for the prototype type, or if they're the same
	** size and verbose is not set.
	*/
	retval = (verbose || (version & V_STD_C) != 0)
			? TY_EQTYPE(etold, tnew) > 0
			: (TY_SIZE(etold) == TY_SIZE(tnew));
    }
    else if (TY_ISFPTYPE(etold) && TY_ISFPTYPE(tnew))
	/* floating/floating case */
	retval = TY_EQTYPE(etold, tnew) > 0;
    else {
	/* all other cases:  require exact match */
	retval = TY_EQTYPE(etold, tnew);
    }
    return( retval );
}


#ifdef LINT
T1WORD
#else
static T1WORD
#endif
dcl_efftype(t)
T1WORD t;
/* Return the effective unqualified argument type for an old-style
** function definition, given a declared type t.  If the given
** type is an unsigned type smaller than int, return an ambiguous
** type, either TY_AUINT or TY_AINT, for UNIX C/ANSI C respectively.
*/
{
    t = TY_UNQUAL(t);

    if (TY_ISINTTYPE(t)) {
	if (TY_SIZE(t) < TY_SIZE(TY_INT)) {
	    /* The type is ambiguous.  Return the correct version. */
	    if (TY_ISUNSIGNED(t) && (version & V_CI4_1) != 0)
		t = TY_AUINT;
	    else
		t = TY_AINT;
	}
    }
    else if (TY_TYPE(t) == TY_FLOAT)
	t = TY_DOUBLE;
    /* Everything else remains as is. */
    return( t );
}

static void
dcl_argoff(dddsid)
SX dddsid;
/* Assign offsets to each parameter in a function definition.
** If "dddsid" is not SY_NOSYM, the function takes a
** variable number of arguments, and the sid is of symbol ...,
** to which an appropriate offset must be assigned.
*/
{
    int i;

    al_s_param();			/* prepare to assign param offsets */

    for (i = 0; i < dcl_nargs; ++i) {
	SX sid = ARG(i);

	al_param(sid);

	SY_FLAGS(sid) |= SY_DEFINED;	/* so we can recognize genuine
					** params in a definition
					*/
	DCL_PRINT(sid, "parameter");
    }
    if (dddsid != SY_NOSYM) {
	SY_OFFSET(dddsid) = al_g_param();
	SY_REGNO(dddsid) = SY_NOREG;	/* not a register variable */
	DCL_PRINT(dddsid, "parameter");
    }

    al_e_param();			/* done allocating */
    return;
}


void
dcl_s_func()
/* Start of function body.  For an ASM function, pass the formal
** names and the text of the ASM macro to CG.  Otherwise call CG
** to generate start-of-function stuff.
*/
{
#ifdef LINT
    /* check number of arguments to main (for strictly conforming - only 0 or 2)
    ** output information on definition of function for lint2
    */
    ln_funcdef(dcl_curfunc, dcl_nargs);
#endif
    if (SY_CLASS(dcl_curfunc) != SC_ASM)
	sm_begf(dcl_curfunc);		/* normal function */
#ifdef	IN_LINE
    else {				/* ASM function */
	int argno;
	int i;
	int curlies = 1;		/* count the { that got us here */

	as_start(SY_NAME(dcl_curfunc));
	for (argno = 0; argno < dcl_nargs; ++argno)
	    as_param( SY_NAME(ARG(argno)) );

	/* ==== HACK ====
	** This code assumes that yacc has eaten the { that starts
	** a function and has remembered it as the lookahead token.
	** Eat a character at a time until we find the appropriate
	** closing }, passing each to the asm handler.  Then, push
	** back the } so the next token (after the {) that yacc
	** will see is }.
	**
	** Look for a } that matches the one that opened the asm
	** macro as its end.
	*/

	as_e_param();

	lx_s_getc();			/* prepare to get lex chars */
	for (;;) {
	    switch( i = lx_getc() ) {
		static const char mesg[] =
			"%s in asm function definition";	/*ERROR*/
	    case EOF:
		UERROR(mesg, "EOF");
		goto done;
	    case 0:
		WERROR(mesg, "NUL");
		continue;		/* ignore NUL */
	    case '{':
		++curlies;
		break;
	    case '}':
		if (--curlies == 0)
		    goto done;
		break;
	    default:
		break;
	    }
	    as_putc((char) i);
	}
done:;

	lx_e_getc();			/* done fetching characters */
	lx_ungetc('}');			/* push back closing } */
	as_end();
    }
#endif	/* def IN_LINE */
    return;
}


void
dcl_e_func()
/* End of function body. */
{
    if (sy_getlev() != SL_FUNARG)
	cerror("confused scope level %d at function exit", sy_getlev());
#ifdef LINT
    ln_params(dcl_nargs);
#endif
    sy_declev();			/* flush params before epilogue */

    if (SY_CLASS(dcl_curfunc) != SC_ASM)
	sm_endf();			/* generate function epilogue */

    dcl_curfunc = SY_NOSYM;		/* forget current function symbol */
    return;
}


T1WORD
dcl_norm(dn,state)
DN dn;
int state;
/* Done with a normal declarator.  Build a type from the decl node dn.
** state is:
**	DS_NORM	for normal declarator
**	DS_ABS	for abstract declarator
**	DS_FUNC	for declarator that is part of function def.
**	DS_INIT for declarator that has following initializer
**
** Calls dcl_defid() for most "normal" declarations, dcl_formal()
** to declare formals in old-style function definitions.
*/
{
    T1WORD t;
    SX sid = dcl_state.dcl_sid;		/* symbol ID for declared symbol */
    char * name = sid ? SY_NAME(sid) : "<noname>";
    SY_LEVEL_t lev;

    DEBUG(d1debug > 1, ("end of declaration, state %d\n", state));

    /* sid should only be zero for DS_ABS */
    if (sid == SY_NOSYM) {
	if (state != DS_ABS)
	    cerror("no symbol for declarator");
    }
    else {
	/* Check up on strange misuse of s/u/e tag (a compatibility issue).
	** MUST do this before building the type, because the current base
	** type could change.
	*/
	dcl_complain = 0;		/* not a useless declaration,
					** because there's a symbol
					*/
	dcl_chkend(D_DECL);		/* check end for declarator */
    }

    t = dcl_end(dn, state == DS_FUNC);
    DEBUG(d1debug > 1, ("name \"%s\", declared type is: (%d) ", name, t));
    TY_PRINT(t, 1);
    DEBUG(d1debug > 1, ("\n"));

    switch( dcl_state.dcl_context ) {
    case DCL_TOP:
    {
	SY_CLASS_t class = dcl_state.dcl_class;

	if (state == DS_ABS)
	    cerror("DS_ABS in DCL_TOP");

	if (state == DS_FUNC) {
	    /* This can only be something that looks like a
	    ** function definition.
	    */
	    if (!TY_ISFTN(t)) {
		UERROR("syntax error, probably missing \",\", \";\" or \"=\"");
		/* Fix symbol table level, since really at top level:
		**	int (*f)() {}
		** Make type function returning int.
		*/
		while (sy_getlev() != SL_EXTERN)
		    sy_declev();
		t = ty_frint;
	    }
	    /* Check for typedef F(); F f {...} */
	    else {
		/* Find last modifier, which must be DN_FTN */
		DN d;
		for(d = dn; d != DN_NULL; d = DD(d).dn_next)
		    if (DD(d).dn_next == DN_NULL) break;

		if (d == DN_NULL || DD(d).dn_op != DN_FTN)
		    UERROR("()-less function definition");
		else
		    /* Remember its parameter list tree for later. */
		    dcl_dnparam = DD(d).dn_var.dn_parmlist;
	    }
	}

	/* Check for old-style declarations:  no explicit type or
	** class.  Assumes that the only place this can happen
	** is for function definition, declarator, init-declarator.
	*/
/*STRICT*/
	if (   dcl_state.dcl_tybit == NOTYBIT
	    && class == SC_NONE
	    && (state == DS_NORM || state == DS_INIT)
	    )
	    WERROR("old-style declaration; add \"int\"");

	/* Figure the level we want the symbol declared at.  If
	** we're in a function definition, the actual level is
	** one too high for the function's name.
	*/
	lev = sy_getlev();
	if (state == DS_FUNC) --lev;

	/* Try to declare the symbol */
	sid = dcl_defid(sid, t, state, class, lev);

	/* Remember symbol index for function being defined. */
	if (state == DS_FUNC)
	    dcl_curfunc = sid;
	else if (state == DS_INIT)
	    in_decl(sid);		/* remember identifier being init'ed */
	break;
    } /* end DCL_TOP case */

    case DCL_FORMAL:
	dcl_formal(t);

	if (state == DS_INIT) {
	    UERROR("cannot initialize parameter: %s", name);
	    in_decl(sid);		/* so we can keep going */
	}
	break;

    case DCL_ABS:			/* abstract declarator */
	if (state != DS_ABS)
	    cerror("non DS_ABS state for DCL_ABS context");
	break;				/* return type */

    default:
	cerror("bad context %d in dcl_norm()", dcl_state.dcl_context);

    } /* end switch */

    return( t );
}


void
dcl_topnull()
/* Grammar saw a null statement at top level.  UNIX C allowed
** this, but ANSI C doesn't.  Warn.
*/
{
    WERROR("syntax error:  empty declaration");
    return;
}

/* The code for testing redeclarations of symbols got so messy, it
** was broken into the following special cases:
**	1) Both objects in same scope, with same type, scope == SL_EXTERN.
**	2) Both objects in same scope, scope != SL_EXTERN.
**	3) Objects in disjoint scopes.
**	4) Functions, both in scope SL_EXTERN.
**	5) Other functions.
**
** Each of these functions returns an action to the calling routine.
** The action may include an error message number and other things to
** do.
**	DR_ERR		message number is an error message
**
**	DR_ARYCHK	check for array that has dimension now
**	DR_CHKEXT	check for out of scope symbol
**	DR_CHKOLD	warn about compatibly equivalent types
**	DR_COMPOS	make composite type
**	DR_HIDE		make hiding symbol
**	DR_HIDE0	make hiding symbol at level 0
**	DR_MKDEF	make defining instance
**	DR_MKSTAT	change storage class to static
**	DR_MKTENT	make tentative instance
**	DR_NOMOVE	suppress usual move of externs, functions
**	DR_REDECL	issue redeclaration message, clean up
**
** Error/Warning Codes:
**	DR_ARE		automatic redeclares extern
**	DR_HIDEP	declaration hides parameter
**	DR_HPROTO	old-style declaration hides prototype declaration
**	DR_INCON	inconsistent storage class for function
**	DR_INRE		inconsistent redeclaration of extern
**	DR_INRS		inconsistent redeclaration of static
**	DR_MREDECL	identifier redeclared
**	DR_MREDEF	identifier redefined
**	DR_OOSRED	out of scope extern and prior uses redeclared as static
**	DR_REQSTAT	identifier redeclared; ANSI C requires "static"
**	DR_SRE		static redeclares extern
**	DR_SRED		extern and prior uses redeclared as static
**	DR_TRE		typedef redeclares extern
**	DR_TREDECL	typedef redeclared
*/

/* Leave 5 bits for error message numbers. */
#define	DR_ERRBITS	(0x1f)
#define	DR_ERR		(1L<<5)
#define	DR_ARYCHK	(1L<<6)
#define	DR_CHKEXT	(1L<<7)
#define	DR_CHKOLD	(1L<<8)
#define	DR_COMPOS	(1L<<9)
#define DR_HIDE         (1L<<10)
#define DR_HIDE0        (1L<<11)
#define DR_MKDEF        (1L<<12)
#define	DR_MKSTAT	(1L<<13)
#define DR_MKTENT       (1L<<14)
#define	DR_NOMOVE	(1L<<15)
#define	DR_REDECL	(1L<<16)

#define EREDECL(n) ((n)|DR_ERR)
#define WREDECL(n) (n)

SX
dcl_defid(sid, t, state, eclass, lev)
SX sid;
T1WORD t;
int state;
SY_CLASS_t eclass;
SY_LEVEL_t lev;
/* Declare identifier whose symbol table entry is "sid" with type "t"
** in parse state "state", with lexical storage class "eclass" and
** lexical level "lev".
*/
{
    static action_t dcl_redecl();
    SY_FLAGS_t tentative = 0;
    SY_FLAGS_t defining = 0;		/* SY_DEFINED if defining instance,
					** else 0
					*/
    SY_CLASS_t class = eclass;		/* copy the explicit class */
    SX oldsid = sid;			/* remember starting sid */
    int hadchange;			/* flag:  something changed that
					** requires code generation
					*/
    int nomove = 0;			/* non-zero to suppress moving
					** functions, externals
					*/
#ifdef LINT
    T1WORD oldtype = SY_TYPE(sid);
#endif

    DEBUG(d1debug, ("dcl_defid():  initial class %s\n", dcl_pclass(class)));

    /* Assume no "same as" declarator initially. */
    dcl_sameas = SY_NOSYM;

    /* Is defining instance if defining function or have initializer. */
    if (state == DS_FUNC || state == DS_INIT)
	defining = SY_DEFINED;

    if (! SY_NAME(sid))			/* happens for f(i) i; ... */
	cerror("bad name in dcl_defid()");

    /* asm function can't have ... */
    if (class == SC_ASM && TY_ISFTN(t) && TY_ISVARARG(t))
	WERROR("cannot have \"...\" in asm function");

    /* Check for reasonable storage class for function declaration. */
    if (TY_ISFTN(t)) {
	int mesgno = 0;
	static const char * const mesgs[] = {
	    0,				/* not used */
	/*1*/ "\"asm\" valid only for function definition",	/* ERROR */
	/*2*/"\"typedef\" valid only for function declaration",	/* ERROR */
	/*3*/"storage class for function must be static or extern", /* ERROR */
	};
	switch( class ) {
	case SC_ASM:
	    /* asm okay only for definition */
	    if (state != DS_FUNC)
		mesgno = 1;
	    break;
	case SC_TYPEDEF:
	    /* typedef okay if not defining function */
	    if (state == DS_FUNC) mesgno = 2;
	    break;
	default:
	    mesgno = 3;
	    break;
	case SC_STATIC:
	    if ((version & V_STD_C) != 0 && lev >= SL_INFUNC)
		WERROR("dubious static function at block level");
	    /* don't change effective class */
	    break;
	case SC_EXTERN:
	case SC_NONE:
	    break;
	}
	if (mesgno) {
	    WERROR(mesgs[mesgno]);
	    class = SC_NONE;
	}
    }

    /* Choose appropriate class for new symbol.
    ** May get altered by dcl_redecl().
    */
    switch ( class ) {
    case SC_ASM:		/* only for function def. */
	if (state == DS_FUNC)
	    break;
	UERROR("\"asm\" valid only for function definition");
	class = SC_NONE;		/* pretend no class */
	break;
    case SC_NONE:
	/* Function definitions may look like they're at SL_FUNARG,
	** but they're not really.
	*/
	if (! TY_ISFTN(t)) {
	    if (lev == SL_EXTERN) {
		/* leave class SC_NONE */
		if (! defining) tentative = SY_TENTATIVE;
	    }
	    else {
		class = SC_AUTO;
		if (lev == SL_FUNARG)
		    cerror("expect dcl_param()");
	    }
	}
	break;
    
    case SC_EXTERN:
	if (state == DS_INIT && lev != SL_EXTERN) {
	    UERROR("cannot initialize \"extern\" declaration: %s", SY_NAME(sid));
	    defining = 0;
	}
	break;

    case SC_STATIC:
	/* Objects are either tentative, only at level 0, or defined. */
	if (!TY_ISFTN(t)) {
	    if (lev == SL_EXTERN) {
		if (! defining)
		    tentative = SY_TENTATIVE;
	    }
	    else
		defining = SY_DEFINED;
	}
	break;

    case SC_REGISTER:
	if (lev == SL_FUNARG) {
	    /* Function parameters get handled by dcl_param()
	    ** and dcl_formal().
	    */
	    cerror("saw SC_REGISTER in dcl_defid()");
	    /* class = SC_PARAM; */
	}
	else
	    class = SC_AUTO;
	/*FALLTHRU*/
    case SC_AUTO:
    case SC_PARAM:
    case SC_TYPEDEF:
	break;
    default:
	cerror("unknown storage class %d", class);
    }

    /* Check for conditions where a size must be known and other
    ** anomalies.
    */
    {
	int flags = 0;
	switch( state ){
	case DS_INIT:
	    if (eclass == SC_TYPEDEF)
		UERROR("cannot initialize typedef: %s", SY_NAME(sid));
	    else if (TY_ISFTN(t))
		UERROR("cannot initialize function: %s", SY_NAME(sid));

	    flags = TY_CVOID | TY_CSUE;
	    break;
	case DS_FUNC:
	    flags = TY_CSUE;
	    break;
	case DS_NORM:
	    if (TY_ISFTN(t))
		break;			/* function declaration */
	    switch( class ){
	    case SC_STATIC:
	    case SC_NONE:
		/* Only check void for tentative definition cases. */
		flags = TY_CVOID;
		if (lev == SL_EXTERN)
		    break;
		/*FALLTHRU*/
	    case SC_AUTO:
	    case SC_PARAM:
		flags = TY_CVOID | TY_CTOPNULL | TY_CSUE;
		break;
	    case SC_EXTERN:
	    case SC_TYPEDEF:
		break;
	    default:
		cerror("confused class %d", class);
	    }
	    break;
	case DS_ABS:
	    break;
	default:
	    cerror("confused state %d", state);
	}
	if (flags)
	    t = ty_chksize(t, SY_NAME(sid), flags, -1);
    }
    if (SY_ISNEW(sid)) {		/* symbol is new */
	hadchange = 1;			/* will need to do code gen. */
	oldsid = SY_NOSYM;		/* pretend there was no previous sym. */
    }
    else {				/* symbol is redeclaration */
	action_t action = dcl_redecl(sid, t, class, defining, lev);
	int newtype = 0;		/* set to 1 if new type selected */

	hadchange = 0;			/* assume no new code gen. */

	/* Do the mandated semantic actions. */

	/* If array type present and the new type has a size and the old
	** one did not, set size, do appropriate debug output.
	*/
	if (action & DR_ARYCHK) {
	    if (   TY_ISARY(t)
		&& TY_NELEM(t) != TY_NULLDIM
		&& TY_NELEM(SY_TYPE(sid)) == TY_NULLDIM
	    ) {
		SY_FLAGS(sid) &= (SY_FLAGS_t) ~SY_DBOUT;
		action |= DR_COMPOS;	/* make composite */
		hadchange = 1;
	    }
	}
	/* Figure out the type to set for current/new symbol. */
	if (action & DR_COMPOS) {
	    /* If the levels are the same, form a composite.  Otherwise,
	    ** form hidden information.
	    */
	    t = (SY_LEVEL(sid) == lev) ? 
				ty_mkcomposite(t, SY_TYPE(sid))
			    :   ty_mkhidden(SY_TYPE(sid), t);
	    newtype = 1;		/* have (possibly) new type */
	}

	/* Make hiding instance of symbol. */
	if (action & DR_HIDE)
	    sid = sy_hide(sid);
	else if (action & DR_HIDE0) {
	    sid = sy_hidelev0(sid);
#ifdef LINT
	    SY_FLAGS(sid) |= (SY_FLAGS(oldsid) & (SY_REF|SY_SET));
#endif
	}

	/* Make defining instance. */
	if (action & DR_MKDEF)
	    SY_FLAGS(sid) = (SY_FLAGS(sid) & ~SY_TENTATIVE) | SY_DEFINED;

	/* Make tentative instance. */
	if (action & DR_MKTENT) {
	    SY_FLAGS(sid) |= SY_TENTATIVE;
	    SY_LINENO(sid) = er_getline();	/* remember changed position */
	}

	/* Change storage class to "static". */
	if (action & DR_MKSTAT)
	    SY_CLASS(sid) = SC_STATIC;

	/* Need to do code generation for any of the following: */
	if (action & (DR_HIDE|DR_HIDE0|DR_MKDEF|DR_MKSTAT|DR_MKTENT))
	    hadchange = 1;

	/* If we're still using the same sid, the type won't get
	** stuffed into the entry below.  If there's a revised
	** type, set it now.
	*/
	if (newtype && oldsid == sid)
	    SY_TYPE(sid) = t;

	/* Symbol cannot be MOVED now, if redeclaration. */
	SY_FLAGS(sid) &= (SY_FLAGS_t) ~SY_MOVED;

	if (action & DR_NOMOVE)
	    nomove = 1;
    }

    /* Set SAMEAS to SY_NOSYM or the identifier passed in from dcl_redecl() */
    SY_SAMEAS(sid) = dcl_sameas;

    /* Set TOMOVE for externs, functions, as long as nomove is zero. */
    if (!nomove && (lev != SL_EXTERN && (TY_ISFTN(t) || class == SC_EXTERN)))
	SY_FLAGS(sid) |= SY_TOMOVE;	/* will want to move information */

    if (sid != oldsid) {		/* This is new symbol instance. */
	/* The only time class could be SC_NONE here (see switch above)
	** is for a symbol at level SL_EXTERN, a function definition, or
	** a function declaration.  In all such cases the implicit class
	** is extern.
	*/
	if (class == SC_NONE)
	    class = SC_EXTERN;
	SY_TYPE(sid) = t;
	SY_CLASS(sid) = class;
	SY_FLAGS(sid) |= (defining | tentative);
	if (eclass == SC_REGISTER) {
	    SY_FLAGS(sid) |= SY_ISREG;	/* user specified register here */
	}

	/* Allocate offsets. */

	SY_REGNO(sid) = SY_NOREG;	/* No register offset for these yet. */
#ifdef FAT_ACOMP
	SM_WT_INIT(sid);		/* Set initial variable weight. */
#endif

	switch( class ) {
	case SC_AUTO:
	    if ( !(TY_ISARY(t) && defining && TY_NELEM(t) == TY_NULLDIM)) {
		/* Don't allocate space for auto that's an array with an
		** initializer if the array has indefinite size.  Defer
		** that until we know how big the array is.
		*/
		al_auto(sid);		/* allocate space for auto */
	    }
	    break;
	case SC_STATIC:
	case SC_EXTERN:
	    SY_OFFSET(sid) = sm_genlab(); /* get new label # */
	    break;
	case SC_TYPEDEF:
	    /* Remember type bits to check for UNIX C-compatible
	    ** type non-conflicts.
	    */
	    SY_OFFSET(sid) = dcl_state.dcl_tybit;
	    break;
	case SC_ASM:
	    /* Do nothing. */
	    break;
	default:
	    cerror("confused class %d in dcl_defid()", class);
	}
    }


    /* Check for missing initializer of const at block scope.
    ** File scope tentative definitions handled later.
    */
    if (   verbose
	&& TY_ISMBRCONST(t)
	&& state != DS_INIT
	&& !TY_ISFTN(t)
	&& (eclass == SC_NONE || eclass == SC_STATIC || eclass == SC_AUTO)
	&& lev >= SL_INFUNC
    )
	WERROR("const object should have initializer: %s", SY_NAME(sid));

    /* Warn about function that may return double unexpectedly. */
    if (   verbose
	&& TY_ISFTN(t)
	&& TY_TYPE(TY_ERETTYPE(t)) == TY_DOUBLE
	&& TY_TYPE(TY_DECREF(t)) == TY_FLOAT
    )
	WERROR("function actually returns double: %s", SY_NAME(sid));

    SY_FLAGS(sid) |= SY_INSCOPE;	/* symbol now in scope */

    DCL_PRINT(sid, "identifier");	/* print internal debug info */
#ifdef LINT
    /* write info for lint2 about this identifier */
    if (   hadchange
	|| (   TY_ISFTN(oldtype)
            && !TY_HASPROTO(oldtype)
            && TY_HASPROTO(SY_TYPE(sid))
           )
       )
	ln2_ident(sid);

    /*
    ** If running with -R (for cxRef), then output additional information
    ** about this identifier.
    */
    if (LN_FLAG('R'))
        cx_ident(sid, (defining == SY_DEFINED) ? 1 : 0);
#endif

    /* Generate code for objects:  newly tentative definitions and
    ** uninitialized static objects in non-file scope.
    */
    if (   ! TY_ISFTN(t)
	&& hadchange
	&& class == SC_STATIC
	&& lev != SL_EXTERN
	&& state == DS_NORM
    ) {
	/* Treat block-level static as if it were a tentative definition.
	** This is a bit of a hack, but the advantage is that these statics
	** go into .bss instead of .data, which reduces a.out sizes.
	*/
	SY_FLAGS(sid) ^= (SY_DEFINED|SY_TENTATIVE);

	cg_instart(t, C_READWRITE);	/* set proper location counter for t */
	cg_defnam(sid);

	SY_FLAGS(sid) ^= (SY_DEFINED|SY_TENTATIVE);
    }

    /* Produce debugging information for typedefs, scalars.  Also for
    ** arrays with a fixed size.  Wait for functions and arrays whose
    ** size isn't known yet, but that are being initialized.
    ** Also wait for function parameters until we know whether they
    ** end up in registers or not and for tentative definitions (which
    ** are always at file scope).  Be sure to get typedefs of function
    ** type.
    */
    if (   class == SC_TYPEDEF
	|| !(   (SY_FLAGS(sid) & SY_TENTATIVE) != 0
	     || TY_ISFTN(t)
	     || (TY_ISARY(t) && TY_NELEM(t) == TY_NULLDIM && state == DS_INIT)
	    )
	)
	DB_SYMBOL(sid);
    return( sid );
}


void
dcl_tentative(sid)
SX sid;
/* Generate code necessary to represent a tentative definition.
** Check for incomplete type here.  Also generate debugger
** information.  Presumably we're called as the symbol is
** being removed from the symbol table at level SL_EXTERN.
** Do some checks:
**	1) For const object (implicitly has no initializer)
**	2) For incomplete type
** Must force null array to have size 1.
*/
{
    T1WORD t = SY_TYPE(sid);		/* object's type */

    if (verbose && TY_ISMBRCONST(t))
	WLERROR(SY_LINENO(sid), "const object should have initializer: %s",
				SY_NAME(sid));

    if (TY_SIZE(t) == 0) {
	/* Only interested in non-arrays or arrays with null dimension. */
	if (!TY_ISARY(t) || TY_NELEM(t) != TY_ERRDIM) {
	    char * name = SY_NAME(sid);
	    (void) ty_chksize(t, name, (TY_CTOPNULL | TY_CSUE), SY_LINENO(sid));

	    if (TY_ISARY(t) && TY_NELEM(t) == TY_NULLDIM)
		SY_TYPE(sid) = ty_mkaryof(TY_DECREF(t), 1);
	}
    }

    cg_instart(t, C_READWRITE);		/* set proper location counter for t */
    cg_defnam(sid);

    DB_SYMBOL(sid);

    return;
}


static action_t dcl_red_obj0();
static action_t dcl_red_objsl();
static action_t dcl_red_objdl();
static action_t dcl_red_flev0();
static action_t dcl_red_fsl();
static action_t dcl_red_fdl();

static action_t
dcl_redecl(sid, t, class, def, newlevel)
SX sid;
T1WORD t;
SY_CLASS_t class;
SY_FLAGS_t def;
SY_LEVEL_t newlevel;
/* Check validity of redefinition of symbol whose symbol index
** is sid.  The new symbol has type t, storage class "class",
** and flags "def" (SY_DEFINED if defining).
** The desired level for the new symbol is "newlevel".
** If "class" is SC_NONE, the declaration has no explicit storage
** class at top level; the appropriate class would be SC_EXTERN.
** Return a set of actions to the caller.
** The rule of thumb is:  messages are issued here.  Action bits
** are passed to the caller, who actually makes changes to the
** symbol table.
*/
{
    int mesgno;				/* returned error message # */
    action_t action;			/* what to do */
    int index;
    static const char * const mesgs[] = {
	/* The potential errors from this routine. */
	/* 0 */	"",
	/* 1 */ "identifier redeclared: %s",			/* ERROR */
#define	DR_MREDECL	1
	/* 2 */	"identifier redefined: %s",			/* ERROR */
#define DR_MREDEF	2
	/* 3 */	"declaration hides parameter: %s",		/* ERROR */
#define	DR_HPARAM	3
	/* 4 */	"identifier redeclared; ANSI C requires \"static\": %s", /* ERROR */
#define	DR_REQSTAT	4
	/* 5 */	"out of scope extern and prior uses redeclared as static: %s",	/* ERROR */
#define	DR_OOSRED	5
	/* 6 */	"extern and prior uses redeclared as static: %s",/* ERROR */
#define	DR_SRED		6
	/* 7 */	"inconsistent storage class for function: %s",	/* ERROR */
#define	DR_INCON	7
	/* 8 */	"static redeclares external: %s",		/* ERROR */
#define	DR_SRE		8
	/* 9 */	"automatic redeclares external: %s",		/* ERROR */
#define	DR_ARE		9
	/* 10 */"typedef redeclares external: %s",		/* ERROR */
#define	DR_TRE		10
	/* 11 */"typedef redeclared: %s",			/* ERROR */
#define	DR_TREDECL	11
	/* 12 */"inconsistent redeclaration of extern: %s",	/* ERROR */
#define	DR_INRE		12
	/* 13 */"inconsistent redeclaration of static: %s",	/* ERROR */
#define	DR_INRS		13
	/* 14 */"old-style declaration hides prototype declaration: %s", /* ERROR */
#define	DR_HPROTO	14
    };

    /* Actually triply dimensioned:
    ** 1st dimension is TY_ISFTN(t)
    ** 2nd dimension is same levels
    ** 3rd dimension is newlevel == 0
    */
    static action_t (*redfuns[8])() = {
	dcl_red_objdl,	dcl_red_objdl,	dcl_red_objsl,	dcl_red_obj0,
	dcl_red_fdl,	dcl_red_fdl,	dcl_red_fsl,	dcl_red_flev0,
    };

    /* Most of the hair here is to determine whether the old and
    ** new declaration/definition are compatible.
    **
    ** For the most part, then there are three outcomes:
    **	1) return the original entry, possibly modifying some bits
    **	2) produce error message and then
    **  3) produce hiding table entry
    */

    DEBUG(d1debug, ("redeclaring %s\n", SY_NAME(sid)));
    DEBUG(d1debug > 1, ("class %s, def %d\n",
		dcl_pclass(class), def));

    index = 0;
    if (TY_ISFTN(t))
	index |= 4;
    if (SY_LEVEL(sid) == newlevel)
	index |= 2;
    if (newlevel == SL_EXTERN)
	index |= 1;
    /* Call selected function, figure out what to do next. */
    action = (*redfuns[index])(sid, t, class, def);

    DEBUG( d1debug > 1 , ("action = %#x\n", action));

    mesgno = action & (DR_ERRBITS|DR_ERR); /* remember current msg number */

    /* If redeclaration, set up other processing. */
    if (action & DR_REDECL) {
	/* use redefinition message if appropriate, else redeclaration */
	mesgno =
	    (def & SY_FLAGS(sid)) ? EREDECL(DR_MREDEF) : EREDECL(DR_MREDECL);
	if (def)
	    action |= (TY_ISFTN(t) ? DR_HIDE0 : DR_HIDE);
	else
	    action |= DR_HIDE;
    }

    /* Begin further semantic checks. */

    if (def)
	action |= DR_MKDEF;		/* do defining instance if def set */

    /* Warn on backwardly compatible type. */
    if (mesgno == 0 && (action & DR_CHKOLD) && TY_EQTYPE(t, SY_TYPE(sid)) < 0)
	/* identifier redeclared */
	mesgno = WREDECL(DR_MREDECL);


    /* Check for similar, matching out-of-scope declarations.
    ** Don't let them propagate if there's an existing definition.
    ** Also, set dcl_sameas if we can find a symbol with the same
    ** name and type that is out of scope or hidden by another decl.
    */
    if (action & DR_CHKEXT) {
	SX oldsym = sy_chkfunext(sid);

	if (oldsym != SY_NOSYM) {
	    int sametype = TY_EQTYPE(SY_TYPE(oldsym), t);

	    if (sametype != 0 && sametype != TY_HIDNONCOMPAT)
		/* The new symbol will be same as this one. */
		dcl_sameas = oldsym;
	    else {
		if (mesgno == 0)
		    mesgno = (SY_CLASS(oldsym) == SC_STATIC) ?
			    WREDECL(DR_INRS) : WREDECL(DR_INRE);
		if (SY_FLAGS(oldsym) & SY_DEFINED)
		    action |= DR_NOMOVE;
	    }
	}
    }

    /* Issue any pending error message. */
    if (mesgno) {
	char * name = SY_NAME(sid);

	if (mesgno & DR_ERR)
	    UERROR(mesgs[mesgno&DR_ERRBITS], name);
	else
	    WERROR(mesgs[mesgno&DR_ERRBITS], name);
    }
    return( action );
}


/*ARGSUSED*/
static action_t
dcl_red_objdl(sid, t, class, def)
SX sid;
T1WORD t;
int class;
SY_FLAGS_t def;
/* Service routine for dcl_redecl():  redeclaration of two
** non-functions at different scope levels.
*/
{
    action_t actions = DR_HIDE;

    DEBUG( d1debug > 1, ("in dcl_red_objdl\n"));

    /* The redeclaration of a parameter must get flagged if it's
    ** at level SL_INFUNC:  according to ANSI, that's the same
    ** scope as what we call SL_FUNARG.
    ** An extern must match a prior extern or static declaration.
    */
    switch( SY_CLASS(sid) ){
    default:
	if (SY_LEVEL(sid) == SL_FUNARG && sy_getlev() == SL_INFUNC)
	    actions = WREDECL(DR_HPARAM) | DR_HIDE;
	else if (class == SC_EXTERN)
	    actions = DR_HIDE|DR_CHKEXT|DR_CHKOLD;
	break;
    case SC_STATIC:
	if (class != SC_EXTERN || SY_LEVEL(sid) != SL_EXTERN)
	    break;			/* just hide */
	/*FALLTHRU*/
    case SC_NONE:			/* could only be at top level */
    case SC_EXTERN:
	/* Cases of true linkage:  new decl with "extern", old with
	** extern/none, or static at file scope.
	*/
	if (class == SC_EXTERN) {
	    actions = DR_CHKEXT|DR_CHKOLD|DR_HIDE;
	    if (   TY_EQTYPE(t, SY_TYPE(sid)) == 0
		&& (SY_FLAGS(sid) & (SY_DEFINED|SY_TENTATIVE)) != 0
		)
		/* Be harsher if we know there's a serious mismatch. */
		actions = DR_REDECL;
	}
	break;
    }
    return( actions );
}


static action_t
dcl_red_obj0(sid, t, class, def)
SX sid;
T1WORD t;
int class;
SY_FLAGS_t def;
/* Service routine for dcl_redecl():  redeclaration of two
** non-functions at level 0.
*/
{
    action_t actions = 0;		/* assume redeclaration */
    int sametype = TY_EQTYPE(t, SY_TYPE(sid));

    DEBUG( d1debug > 1, ("in dcl_red_obj0\n"));

    /* If wrong type or redefinition, flag. */
    if (sametype && (SY_FLAGS(sid) & def) == 0) {
	/* Check various combinations of storage classes. */
	switch( SY_CLASS(sid) ){
	case SC_STATIC:			/* old is static */
	    switch( class ) {
	    case SC_NONE:
		actions = WREDECL(DR_REQSTAT)|DR_ARYCHK;
		break;
	    case SC_EXTERN:
	    case SC_STATIC:
		actions = DR_ARYCHK;
		break;			/* okay */
	    default:
		break;			/* flag as redeclaration */
	    }
	    break;
	case SC_EXTERN:			/* old was extern or none */
	case SC_NONE:
	    switch( class ){
	    case SC_EXTERN:
	    case SC_NONE:
		/* Check whether we just got array dimension. */
		actions = DR_ARYCHK;
		break;
	    case SC_STATIC:
		if (SY_FLAGS(sid) & SY_DEFINED)
		    break;		/* old symbol was definition */
		/* Turn existing entry into static, check arrays. */
		actions = DR_ARYCHK | DR_MKSTAT;
		/* Remark about mismatched storage classes:  extern/none
		** followed by static.  The earlier versions are treated
		** as forward references to the static.  Warn if verbose
		** or strictly conforming.
		*/
		if (verbose || (version & V_STD_C) != 0)
		    actions |= (SY_FLAGS(sid) & SY_MOVED) ?
					WREDECL(DR_OOSRED)
				    :	WREDECL(DR_SRED);
		break;
	    }
	    break;
	case SC_TYPEDEF:
	    /* Special case for transition, where several headers might
	    ** have unguarded redeclarations of the same typedef.
	    */
	    if (class == SC_TYPEDEF && sametype > 0)
		actions = WREDECL(DR_TREDECL);
	    break;
	}
    }
    if (actions == 0)
	actions = DR_REDECL;		/* do redeclaration processing */
    else {
	actions |= DR_CHKOLD|DR_CHKEXT;	/* check for old mismatched type */
	/* Make a tentative definition if at top level, class is "none" or
	** "static", and the old symbol has no previous definition and this
	** one is not a definition.
	*/
	if (   (class == SC_NONE || class == SC_STATIC)
	    && !def
	    && (SY_FLAGS(sid) & (SY_DEFINED|SY_TENTATIVE)) == 0
	)
	    actions |= DR_MKTENT;
    }
    return( actions );
}

static action_t
dcl_red_objsl(sid, t, class, def)
SX sid;
T1WORD t;
int class;
SY_FLAGS_t def;
/* Service routine for dcl_redecl():  redeclaration of non-function
** at same, non-top-level scope.
*/
{
    action_t actions = 0;		/* assume redeclaration */
    int sametype = TY_EQTYPE(t, SY_TYPE(sid));

    DEBUG( d1debug > 1, ("in dcl_red_objsl\n"));

    /* There are several interesting cases here:
    ** 1.  an external declaration is followed by another external
    ** declaration of the same type.  We may want to adjust an array
    ** dimension.  But the types must be the same, and there can't
    ** be a redefinition.
    ** 2, 3, 4.  UNIX C silently allowed an auto, typedef, or static
    **	to hide an extern at the same level.  ANSI C requires a remark:
    **		f() { extern int i; int i; }
    **		f() { extern int i; typedef int i; }
    **		f() { extern int i; static int i; }
    ** 5.  The redeclaration of a parameter must be noted.
    */
    switch( SY_CLASS(sid) ){
    case SC_NONE:
    case SC_EXTERN:
	switch( class ) {
	case SC_EXTERN:
	    if (sametype && (SY_FLAGS(sid) & def) == 0)
		actions = DR_ARYCHK|DR_CHKOLD|DR_CHKEXT;
	    break;
	case SC_AUTO:
	    actions = WREDECL(DR_ARE) | DR_HIDE; break;
	case SC_TYPEDEF:
	    actions = WREDECL(DR_TRE) | DR_HIDE; break;
	case SC_STATIC:
	    actions = WREDECL(DR_SRE) | DR_HIDE; break;
	}
	break;
    }
    if (actions == 0)
	actions = DR_REDECL;
    return( actions );
}


static action_t
dcl_red_flev0(sid, t, class, def)
SX sid;
T1WORD t;
int class;
SY_FLAGS_t def;
/* Service routine for dcl_redecl():  redeclaration of function
** at level 0.  That is, t is a function type.  Both declarations
** must be at level 0.
*/
{
    action_t actions = 0;		/* default action is redeclaration */
    T1WORD oldtype = SY_TYPE(sid);
    int sametype = TY_EQTYPE(t, oldtype);
    SY_FLAGS_t flags = SY_FLAGS(sid);

    DEBUG( d1debug > 1, ("in dcl_red_flev0\n"));

    /* Mismatch if the types would have been incompatible
    ** due to hidden information and the original type was
    ** for an old-style function definition.
    */
    if (   sametype == TY_HIDNONCOMPAT
	&& (flags & SY_DEFINED) != 0
	&& TY_ISFTN(oldtype) && TY_HASHIDDEN(oldtype)
	)
	actions = DR_REDECL;
    /* Otherwise, the types must be compatible, must not be redefinition. */
    else if (sametype && (flags & def) == 0) {
	/* If current declaration is a definition, do special "hide". */
	action_t hide = def ? DR_HIDE0 : DR_HIDE;

	/* Make sure storage classes are reasonable. */
	switch( SY_CLASS(sid) ){
	case SC_NONE:
	case SC_EXTERN:
	    switch( class ){
	    case SC_NONE:
	    case SC_EXTERN:
		/* Just check some stuff, make composite type. */
		actions = DR_CHKOLD|DR_COMPOS;
		break;
	    case SC_STATIC:
		/* Error if function already defined or this is not a
		** definition.
		*/
		if ((flags & SY_DEFINED) != 0 || ! def)
		    break;

		/* UNIX C allowed extern followed by static as a way to
		** forward declare static functions.  For compatibility,
		** allow the same, possibly with a warning.
		*/
		if (verbose || (version & V_STD_C) != 0)
		    actions |= (flags & SY_MOVED)
				    ? WREDECL(DR_OOSRED) : WREDECL(DR_SRED);
		actions |= DR_CHKOLD | hide;
		break;
	    case SC_ASM:
		/* Function must not be previously defined.
		** Types must match exactly.
		*/

		if (flags & SY_DEFINED)
		    actions = DR_REDECL;
		else if (sametype < 0)
		    actions = WREDECL(DR_MREDECL) | hide;
		else
		    actions = DR_CHKOLD|DR_COMPOS | hide;
		break;
	    }
	    break;
	case SC_STATIC:

	/* UNIX C:  a declaration of a function becomes external if
	** previous (static) was at top level, but was moved there.
	** Behaved as if the static declaration was never seen.
	**
	** ANSI C:  a declaration of a function with no explicit
	** storage class behaves as if the storage class extern
	** had been explicitly written.
	*/
	    switch( class ) {
		SY_FLAGS_t moved;
	    case SC_NONE:
	    case SC_EXTERN:
		if ((moved = (flags & SY_MOVED)) != 0)
		    actions = WREDECL(DR_REQSTAT);
		actions |= moved ? hide : (DR_CHKOLD|DR_COMPOS);
		break;
	    case SC_STATIC:
		actions = DR_CHKOLD|DR_COMPOS;
		break;
	    }
	    break;
	case SC_ASM:
	    /* Can't have redefinition in same file, but allow
	    ** forward references.
	    */
	    if ((class == SC_NONE || class == SC_EXTERN) && !def)
		actions = DR_CHKOLD|DR_COMPOS;
	    break;
	}
    }
    if (actions == 0)
	actions = DR_REDECL;
    return( actions );
}


/*ARGSUSED*/
static action_t
dcl_red_fsl(sid, t, class, def)
SX sid;
T1WORD t;
int class;
SY_FLAGS_t def;
/* Service routine for dcl_redecl():  redeclaration of identifier,
** new one is function at same level as original symbol, and both
** are NOT at top level.
*/
{
    int actions;
    int sametype = TY_EQTYPE(t, SY_TYPE(sid));
    int oldclass = SY_CLASS(sid);

    DEBUG(d1debug > 1, ("in dcl_red_fsl()\n"));
    /* Same type and class okay.
    ** extern followed by static:
    **	if same type:	inconsistent storage class for function
    **  else:		static redeclares external
    ** Everything else:  redeclaration
    */
    if (   sametype
	&& (oldclass == class || (oldclass == SC_EXTERN && class == SC_NONE))
    )
	actions = DR_COMPOS|DR_CHKOLD;
    else if (oldclass == SC_EXTERN && class == SC_STATIC)
	actions = WREDECL(sametype ? DR_INCON : DR_SRE) | DR_CHKOLD|DR_HIDE;
    else
	actions = DR_REDECL;
    return( actions );
}


/*ARGSUSED*/
static action_t
dcl_red_fdl(sid, t, class, def)
SX sid;
T1WORD t;
int class;
SY_FLAGS_t def;
/* Service routine for dcl_redecl():  redeclaration of identifier,
** new one is function at different level from original symbol.
*/
{
    action_t actions = DR_REDECL;	/* assume redeclaration */
    int oldclass = SY_CLASS(sid);
    int sametype = TY_EQTYPE(t, SY_TYPE(sid));


    DEBUG( d1debug > 1, ("in dcl_red_fdl\n"));

    /* Because we're at different levels, the second declaration
    ** cannot be a definition.  There are many messy special cases
    ** here.  Consider them based on the level at which the first
    ** symbol lives.
    **
    */

    switch( SY_LEVEL(sid) ){
    case SL_EXTERN:			/* old is at top level */
	if (SY_FLAGS(sid) & SY_MOVED) {
	    /* old class must be either extern or static */
	    actions = DR_HIDE|DR_CHKOLD|DR_CHKEXT;
	    if (sametype)
		actions |= DR_COMPOS;
	}
	else {
	    /* First instance is a genuine top level declaration. */
	    switch( oldclass ){
	    case SC_NONE:
	    case SC_EXTERN:
	    case SC_STATIC:
		if (sametype){
		    /* types same; check classes */
		    actions = DR_HIDE|DR_CHKOLD|DR_CHKEXT|DR_COMPOS;
		    if (class == SC_STATIC && oldclass != SC_STATIC)
			actions |= WREDECL(DR_INCON);
		    /* Warn when an old-style declaration hides a prototype:
		    ** the prototype is (according to the standard) hidden.
		    */
		    else if (   verbose &&
			     TY_HASPROTO(SY_TYPE(sid)) && !TY_HASPROTO(t)
			    )
			actions |= WREDECL(DR_HPROTO);
		}
		break;
	    case SC_ASM:
		if (sametype) {
		    actions = DR_HIDE|DR_CHKOLD|DR_CHKEXT|DR_COMPOS;
		    if (class == SC_STATIC)
			actions |= EREDECL(DR_INCON);
		}
		break;
	    default:			/* such as typedef */
		actions = DR_HIDE|DR_CHKEXT;
		break;
	    }
	}
	break;
    case SL_FUNARG:
	/* Parameter could never have function type -- it must have
	** been changed to pointer to function.
	*/
	if (sy_getlev() == SL_INFUNC)
	    actions = WREDECL(DR_HPARAM) | DR_HIDE;
	else
	    actions = DR_HIDE|DR_CHKEXT;
	break;
    default:				/* higher level decl. */
	if (sametype) {
	    actions = DR_HIDE|DR_CHKOLD|DR_COMPOS;
	    /* Check for matching classes. */
	    if (oldclass != class) {
		if (oldclass == SC_STATIC || class == SC_STATIC)
		    actions |= WREDECL(DR_INCON);
	    }
	}
	/* If previous is not function, there still might be a
	** function decl. that's hidden at a lower level.
	** Hide currently visible symbol, but look for 
	** earlier edition.
	*/
	else if (oldclass != SC_EXTERN && !TY_ISFTN(SY_TYPE(sid)))
	    actions = DR_HIDE|DR_CHKEXT;
	break;
    }
    return( actions );
}

/* Service routines for dcl_chkend(), dcl_tag(). */

static T1WORD
dcl_sue_type()
/* Return a type-word that corresponds to the current
** type bits if s/u/e.
*/
{
    if      (dcl_state.dcl_tybit & TB(KT_STRUCT))
	return( TY_STRUCT );
    else if (dcl_state.dcl_tybit & TB(KT_UNION))
	return( TY_UNION );
    else if (dcl_state.dcl_tybit & TB(KT_ENUM))
	return( TY_ENUM );
    cerror("confused dcl_sue_type(), %#o", dcl_state.dcl_tybit);
    /*NOTREACHED*/
}


static char *
dcl_sue_tname(t)
T1WORD t;
/* Return print-name for TY_STRUCT/TY_UNION/TY_ENUM. */
{
    switch( t ) {
    case TY_STRUCT:	return( "struct" );
    case TY_UNION:	return( "union" );
    case TY_ENUM:	return( "enum" );
    }
    cerror("confused dcl_sue_tname()");
    /*NOTREACHED*/
}


void
dcl_chkend(flag)
int flag;
/* This routine gets called at the end of a declaration to
** clean up niggling details.  Several things must be delayed
** until this point, because when we see a type (such as a
** struct declaration), we don't yet know if there are going to
** be any symbols declared that use the type.  flag is D_DECL
** to check up on declarator, D_ABSDECL for abstract declarator.
** Check for:
**	1) Useless declarations:
**		simple type with no declarator
**		s/u decl with member list but no explicit tag and no declarator
**		reuse of existing s/u/e tag with no declarator
**	2) Occurrence of hiding declaration of s/u/e tag.
**	3) Typedef declaration with no typename.
**
** There is a close interaction here with dcl_tag(), since
** most of the cases above relate to s/u/e tag declarations.
**
** The tag cases are:
**
**	f(){
**	    struct s1 { int x; };
**	    {
**		struct s1 q;		use outer s1
**		struct s1;		create hiding s1
**		...
**
** (Discriminate between presence and absence of declarator after tag.)
**
**	f(){
**	    struct s1 { int x; };
**	    {
**		union s1 q;		UNIX C allowed, meant struct s1!
**		...
**
** For this messy compatibility case, if "q" isn't there, s1 is a new
** s/u/e tag.  For compatibility (since UNIX C ignored it), we must
** avoid introducing a new tag, so warn.  For ANSI-mode, create the
** new tag.  If "q" is there, it would be an error (type mismatch)
** for ANSI C, but UNIX C allowed the loose use of struct/union/enum
** with a tag.
*/
{
    if (dcl_complain || dcl_state.dcl_savetag != SY_NOSYM) {
	if (dcl_state.dcl_savetag != SY_NOSYM) {
	    T1WORD tagtype = TY_TYPE(dcl_state.dcl_btype);
	    int newtag =    (dcl_state.dcl_sid == SY_NOSYM && flag == D_DECL)
			 || tagtype != dcl_sue_type();
					/* newtag non-0 if ANSI C would create a
					** new tag.
					*/

	    /* Have saved tag.  Figure out which case. */
	    if (! (version & V_CI4_1)) {
		/* For ANSI C mode, always create new tag if there's no
		** declarator or if there's a s/u/e type mismatch.
		*/
		if (newtag) {
		    /* Always create new tag of desired new type, no decl list. */
		    dcl_tag(SY_NAME(dcl_state.dcl_savetag), D_NOLIST, D_FORCE);
		    /* May clear dcl_complain. */
		}
	    }
	    else {
		/* Compatibility cases.  Reach here if there's a saved
		** tag and we're doing CI 4.1 compatibility behavior.
		**
		** If a symbol was declared, it has picked up the existing
		** tag's type, even if that was different from the declared
		** type:
		**	struct s {...}; f(){ union s foo; ...
		*/
		if (dcl_state.dcl_sid != SY_NOSYM && tagtype != dcl_sue_type()) {
		    WERROR("base type is really \"%s %s\": %s",
			dcl_sue_tname(tagtype),
			SY_NAME(TY_SUETAG(dcl_state.dcl_btype)),
			SY_NAME(dcl_state.dcl_sid));
		    /* Suppress further complaints. */
		    dcl_complain = 0;
		}
		/* If ANSI C would have created new tag type, let user know. */
		if (newtag) {
		    WERROR("declaration introduces new type in ANSI C: %s %s",
			dcl_sue_tname(dcl_sue_type()),
			SY_NAME(dcl_state.dcl_savetag));
		    /* Suppress further complaints. */
		    dcl_complain = 0;
		}
	    }
	}
	/* Retest dcl_complain, because it may still be set after
	** calling dcl_tag() for "enum e;".
	*/
	if (dcl_state.dcl_sid == SY_NOSYM  && flag == D_DECL && dcl_complain)
	    WERROR("useless declaration");
    }
    /* Check for silly typedef. */
    if (dcl_state.dcl_sid == SY_NOSYM && dcl_state.dcl_class == SC_TYPEDEF)
	WERROR("typedef declares no type name");
    return;
}


void
dcl_tag(s, haslist, forcenew)
char * s;
int haslist;
int forcenew;
/* Declare struct/union/enum tag s.  If s is NULL, declare a fake
** name for reference purposes.  If "haslist" is 0, there is
** no struct_decl_list associated with the declaration; else
** there will be.  New enumeration tags without enumerator lists
** are dubious.
**
** There is one messy special case, exemplified by
**	f(){
**	    struct s1 { int x; };
**	    {
**		struct s1 q;		use outer s1
**		struct s1;		create hiding s1
**		...
** Note that we can't tell which of the two inner cases we're
** dealing with here.  We can only tell after we have or have
** not seen an init_declarator.  Therefore, assume the first
** case, but remember the old tag symbol index (dcl_state.dcl_savetag).
** If it turns out that there is no init_declarator, rather
** than issue a "useless declaration" warning, hide the old
** tag and create a new one, and a new s/u/e type.
** 
** There is a related messy case that must be handled for UNIX
** C compatibility:
**	struct s {int x; };
**	f(){
**		union s v;		UNIX C interpreted as struct s!
**		...
** Since the appropriate behavior here depends on whether there's a
** declarator, which we don't know yet, delay warnings until dcl_chkend().
** (See further comments there.)
**
** If "forcenew" is non-zero, create a new tag if one already
** exists, to handle the special case.
** The SY_OFFSET field of the tag is set to 1 if there's a list
** following the tag.  That guards against confusion on
** (erroneous) declarations like:
**	struct s { struct s foo; int i; };
** that use the tag again before it's fully defined.
*/
{
    static int fakeno = 0;		/* current fake name number */
    T1WORD type;
    SY_CLASS_t class;
    SX tag;				/* symbol entry for tag */
    T1WORD newt;

    if (! s) {				/* no tag name; create dummy */
	char fakebuf[10];		/* fake name buffer */
	sprintf(fakebuf, "%c%dfake", DCL_ILL_ID_CHAR, fakeno++);
	s = st_lookup(fakebuf);		/* hash name for safe keeping */
    }

    type = TY_STRUCT; class = SC_STRUCT;
    if (dcl_state.dcl_tybit & TB(KT_UNION)) {
	type = TY_UNION; class = SC_UNION;
    }
    else if (dcl_state.dcl_tybit & TB(KT_ENUM)) {
	type = TY_ENUM; class = SC_ENUM;
    }

    DEBUG(d1debug > 1, ("declaring %s tag %s, force = %d\n",
			dcl_sue_tname(type), s, forcenew));

    tag = sy_lookup(s, SY_TAG, SY_CREATE); /* enter new entry if necessary */
    if (forcenew == D_FORCE && !SY_ISNEW(tag))
	tag = sy_hide(tag);		/* force new copy if necessary */

/* The logic for an existing entry requires a real messy decision table:
**	 8	 4	 2	 1
**	same	same	old	new		action
**	scope?	type?	haslist	haslist	line
**	 N	 -	 -	 Y	1	declare new tag, type
**	 N	 -	 -	 N	2	special cases:  may create new tag
**	 Y	 Y	 Y	 Y	3	redecl. error
**	 Y	 Y	 - 	 N	4	use old tag, type
**	 Y	 Y	 N	 Y	5	completing decl.
**	 Y	 N	 -	 -	6	redecl. error
**
** When using a new tag, clear dcl_complain, since the declaration
** does something.  (Must be at top level.)
*/
    if (!SY_ISNEW(tag)) {		/* comparable tag already in scope? */
	int switval = 0;

	if (haslist == D_LIST) 
	    switval += 1;
	if (TY_HASLIST(SY_TYPE(tag)) || SY_OFFSET(tag) != 0)
	    switval += 2;
	if (SY_CLASS(tag) == class) 
	    switval += 4;
	if (sy_getlev() == SY_LEVEL(tag))
	    switval += 8;
	switch( switval ) {
	case 8+4+2+1:	/* line 3 */
	case 8+0+0+0:	/* line 6 */
	case 8+0+0+1:	/* line 6 */
	case 8+0+2+0:	/* line 6 */
	case 8+0+2+1:	/* line 6 */
	{
	    char *tname = dcl_sue_tname(TY_TYPE(SY_TYPE(tag)));
	    UERROR("(%s) tag redeclared: %s", tname, s);
	    dcl_complain = 0;		/* avoid secondary complaints */
	    break;			/* create new entry */
	}
	case 0+0+0+0:	/* line 2 */
	case 0+4+0+0:	/* line 2 */
	case 0+0+2+0:	/* line 2 */
	case 0+4+2+0:	/* line 2 */
	    /* For the special cases, save the tag, and use the existing
	    ** type.
	    */
	    dcl_state.dcl_savetag = tag;
	    goto settype;
	case 8+4+0+1:	/* line 5 */
	    if (dcl_state.dcl_context == DCL_TOP)
		dcl_complain = 0;	/* bypass complaints:  completing def. */
	    goto settype;
	case 8+4+0+0:	/* line 4 */
	case 8+4+2+0:	/* line 4 */
	    /* Redeclaration of an existing tag.  Treat this as useful,
	    ** except for enumerations.
	    */
	    if (type != TY_ENUM)
		dcl_complain = 0;
settype:;
	    dcl_state.dcl_btype = SY_TYPE(tag);
	    DEBUG(d1debug > 1, ("using existing type %d for tag %s\n",
		dcl_state.dcl_btype, s));
	    /* Use previous type.  If in prototype declaration,
	    ** s/u/e tag had better be at top level.
	    */
	    if (dcl_infunc && SY_LEVEL(tag) != SL_EXTERN)
		WERROR("dubious tag in function prototype: %s %s",
		    dcl_sue_tname(type), s);
	    return;
	default:
	    /* For other cases, since we're defining a new tag,
	    ** declaration can't be useless.  Disable warning.
	    */
	    dcl_complain = 0;
	    break;
	}
	/* Default behavior is to create hiding declaration. */
	tag = sy_hide(tag);
    } /* end if on old tag */
    /* New tag.
    ** Warnings for useless declaration still appropriate if this is a
    ** list-less enumeration, or if this is at top level and is a
    ** struct/union with list and fake name.  (That is, disable the
    ** warning otherwise.)
    */
    else if (type == TY_ENUM && haslist == D_NOLIST) {
#if 0	/* erroneous flags "extern enum e x;" */
	/* Note:  s not fake because there was no list. */
	WERROR("no enumeration constants declared: enum %s", s);
#endif
	/*EMPTY*/
    }
    else if (   (   dcl_state.dcl_context == DCL_TOP
		 || dcl_state.dcl_context == DCL_FORMAL
		)
	     && !( haslist == D_LIST && s[0] == DCL_ILL_ID_CHAR)
	    )
	    dcl_complain = 0;

    /* If we're in a function prototype, almost anything except a
    ** reference to an existing tag is dubious, because a new tag
    ** goes out of scope at the end of the prototype declaration.
    */
    if (dcl_infunc) {
	WERROR(	haslist == D_LIST
	    ? "dubious %s declaration; use tag only: %s"	/*ERROR*/
	    : "dubious tag declaration: %s %s", 		/*ERROR*/
		dcl_sue_tname(type), s[0] == DCL_ILL_ID_CHAR ? "<unnamed>" : s
	);
    }

    newt = ty_mktag(type, tag);
    SY_TYPE(tag) = newt;
    SY_CLASS(tag) = class;
    SY_FLAGS(tag) |= SY_INSCOPE;	/* tag is now in scope */
    /* If there are members coming, use a dummy value to mark
    ** other uses of this tag until we actually add the members.
    */
    SY_OFFSET(tag) = haslist;
    dcl_state.dcl_btype = newt;
    DEBUG(d1debug, ("declaring %s tag %s%s, ID %d, type %d\n",
	dcl_sue_tname(type), SY_NAME(tag),
	haslist == D_NOLIST ? " (only)" : "", tag, newt));
    DCL_PRINT(tag, "s/u/e tag");
    return;
}


void
dcl_s_soru()
/* Start struct-declaration-list.  Push current context, choose
** new one:  DCL_STRUCT or DCL_UNION.
*/
{
    dcl_push( (dcl_state.dcl_tybit & TB(KT_STRUCT)) ? DCL_STRUCT : DCL_UNION );
    dcl_state.dcl_table = SY_MOSU;	/* look up struct/union members */
    return;
}


DN
dcl_mbr(dn, isfield, exprp)
DN dn;
int isfield;
ND1 * exprp;
/* Declare a struct/union member.  The decl tree for the member type
** is in dn.  If isfield is 0, the member is not a field.  Otherwise
** exprp points to the size expression, which is a tree that must
** represent an integral constant expression.
** This routine does the symbol checking now, but strings together
** a new set of decl nodes that are later processed for struct/union
** alignment and offsets.
*/
{
    T1WORD t;				/* member type */
    SX sid = dcl_state.dcl_sid;		/* symbol index for member */
    DN new = newdn();			/* resulting decl node */
    char * s = (sid == SY_NOSYM) ? "<unnamed>" : SY_NAME(sid);
					/* Name string, in case of error. */
    CONVAL fldsize;

    DEBUG(d1debug > 1, ("end of member %s declaration:\n", s));

    t = dcl_end(dn, 0);			/* do end-of-decl stuff */

    /* An anonymous union member always seems silly.  An anonymous
    ** non-field struct member is for UNIX C backward compatibility.
    */
    if (   sid == SY_NOSYM
	&& (dcl_state.dcl_context == DCL_UNION || isfield == D_NOFIELD))
	WERROR("unnamed %s member",
		dcl_state.dcl_context == DCL_UNION ? "union" : "struct");

    if (isfield == D_FIELD) {
	fldsize = tr_inconex(exprp);	/* get size expression */
	if (   fldsize < 0
	    || (    fldsize == 0
		 && (sid != SY_NOSYM || dcl_state.dcl_context == DCL_UNION)
	       )
	) {
	    UERROR("bit-field size <= 0: %s", s);
	    fldsize = 1;
	}
	/* check valid types for field */
	switch( TY_TYPE( t ) ) {
	case TY_ENUM:
	    t = ty_chksize(t, s, TY_CSUE, -1);
	    /*FALLTHRU*/
	case TY_CHAR:	case TY_UCHAR:	case TY_SCHAR:
	case TY_SHORT:	case TY_USHORT:	case TY_SSHORT:
	case TY_INT:	case TY_LONG:	case TY_ULONG:	
	case TY_SLONG:
#ifdef LINT
	    if (LN_FLAG('p'))
		WERROR("nonportable bit-field type");
	    /* FALLTHRU */
#endif
	case TY_UINT:	case TY_SINT:
	    /* These types are all okay. */
	    if (fldsize > TY_SIZE(t)) {
		UERROR("bit-field too big: %s", s);
		fldsize = TY_SIZE(t);
	    }
	    break;
	
	default:
	    UERROR("invalid type for bit-field: %s", s);
	    t = TY_INT;
	    break;
	}
    }
    else {				/* non-field member */
    /* Check up on member type. */
	if (TY_ISFTN(t)) {
	    UERROR("member cannot be function: %s", s);
	    t = TY_INT;
	}
	else
	    t = ty_chksize(t, s, (TY_CVOID | TY_CTOPNULL | TY_CSUE), -1);
    }  /* end if (isfield) */
	    
    if (sid != SY_NOSYM) {
	/* Create a hiding instance and use that if there's already
	** an equivalent member name in scope.
	*/
	if ((SY_FLAGS(sid) & SY_INSCOPE) != 0)
	    sid = sy_hide(sid);
	SY_TYPE(sid) = t;
	SY_CLASS(sid) =
		(dcl_state.dcl_context == DCL_STRUCT) ? SC_MOS : SC_MOU;
    }

    DD(new).dn_op = DN_MBR;
    DD(new).dn_next = DN_NULL;

    DSU(new).dn_sutype = t;
    DSU(new).dn_susym = sid;
    DSU(new).dn_suisfield = isfield;
    DSU(new).dn_sufldsize = fldsize;

    /* Mark identifier as in-scope. */
    if (sid != SY_NOSYM)
	SY_FLAGS(sid) |= SY_INSCOPE;

    DCL_PRINT(sid, "member");
    DEBUG(d1debug, ("--> isfield = %d, size = %d\n", isfield, fldsize));
    return( new );
}
    

DN
dcl_mlist(oldlist, newmbr)
DN oldlist;
DN newmbr;
/* Make a list of struct/union member decl nodes.  The nodes
** have been built by dcl_mbr().  The list runs left to right.
** If "oldlist" is empty, start a new list.
*/
{
    DN dn;
    SX sid = DSU(newmbr).dn_susym;
    char * newname = sid == SY_NOSYM ? (char *) 0 : SY_NAME(sid);

    if (oldlist == DN_NULL)
	return( newmbr );

    /* Fold two loops together:
    **		1) look for member of same name as "newmbr".
    **		2) find end of "oldlist" list, so we can put new member there.
    */
    for (dn = oldlist;;) {
	SX oldsid = DSU(dn).dn_susym;
	DN next;

	if (oldsid != SY_NOSYM && (SY_NAME(oldsid) == newname))
	    UERROR("duplicate member name: %s", newname);
	if ((next = DD(dn).dn_next) == DN_NULL) {
	    DD(dn).dn_next = newmbr;
	    break;
	}
	dn = next;
    }
    return( oldlist );
}


void
dcl_e_soru(dn)
DN dn;
/* End of struct/union decl.  Process the list of members.
** Current context tells whether struct or union.
** Context is reset to the context of the struct/union decl.
*/
{
    int mbrno;
    BITOFF curoff = 0;			/* current s/u offset */
    BITOFF maxoff = 0;			/* maximum offset for s/u */
    int isunion = dcl_state.dcl_context == DCL_UNION;
					/* remember current context */
    BITOFF align = ALSTRUCT;		/* start off with least restrictive
					** alignment in s/u
					*/

    /* Restore context to the level of the struct/union declaration. */
    dcl_pop();

    /* Now process members for it. */
    for (mbrno = 0 ; dn != DN_NULL; dn = DD(dn).dn_next ) {
	SX sym = DSU(dn).dn_susym;
	int isfield = DSU(dn).dn_suisfield;
	BITOFF fldsize = DSU(dn).dn_sufldsize;
	T1WORD mbrtype = DSU(dn).dn_sutype;
	BITOFF newoffset;

	/* Allocate offset if there's a member name, or if it's a field
	** (for which it's padding).
	*/
	if (sym != SY_NOSYM || isfield == D_FIELD)
	    newoffset = al_struct(&curoff, mbrtype, isfield, fldsize);

	if (sym != SY_NOSYM) {
	    BITOFF malign = TY_ALIGN(mbrtype);
	    ty_mkmbr(dcl_state.dcl_btype, sym);
	    mbrno++;
	    if (isfield == D_FIELD)
		SY_FLDPACK(sym, fldsize, newoffset);
	    else
		SY_OFFSET(sym) = newoffset;
	    DEBUG(d1debug > 1,
		("offset for %s is %d (%#o)\n",
			SY_NAME(sym), SY_OFFSET(sym), SY_OFFSET(sym)));
	    /* Alignment of structure is only adjusted by named members.
	    ** (This is a compatibility issue.)
	    */
#ifdef	PACK_PRAGMA
	    /* Constrain member alignment to no more than selected value. */
	    if (Pack_align && malign > Pack_align)
		malign = Pack_align;
#endif
	    align = AL_ALIGN(align, malign);
	}
	if (curoff > maxoff) maxoff = curoff;
	if (isunion) curoff = 0;
    }
    /* Establish size and alignment of this s/u */
    maxoff = AL_ALIGN(maxoff, align);
    if (maxoff == 0) {
	UERROR("zero-sized struct/union");
	maxoff = TY_SIZE(TY_INT);
    }
    else if (mbrno == 0) {
	WERROR("struct/union has no named members");
	/* Add null member so type is complete. */
	ty_mkmbr(dcl_state.dcl_btype, SY_NOSYM);
    }

    ty_e_sue(dcl_state.dcl_btype, maxoff, align);
    DEBUG(d1debug, ("s/u size is %d\n", maxoff));
    DB_SUE(dcl_state.dcl_btype);	/* do debug info. for s/u */
#ifdef LINT
    ln2_suedef(dcl_state.dcl_btype);
#endif
    return;
}


void
dcl_nosusemi()
/* This routine gets called if the ; that's required at the
** end of a struct/union declaration list is omitted.  This
** code is here mostly for backward compatibility.
*/
{
/*STRICT*/
    WERROR("syntax requires \";\" after last struct/union member");
    return;
}


/* Enum routines.
** You would expect that enumeration types don't need to hold
** onto the enumerator names and values until later, and we don't.
** However, if there's a cast expression that defines the value
** of the enumerator and the cast gives rise to a new type being
** built, the type handling gets confused.  Here's an example:
**
**	enum e { e1 = (enum ec { e21,e22 }) 2 };
**
** The problem is that the type handling assumes that members of
** an s/u/e will arrive sequentially.  In this case they don't.
**
** dcl_enumval is part of dcl_state because enumerations could be
** nested, as in the example above, and their default values must
** be kept distinct.
*/


void
dcl_s_enu(tag, haslist)
char * tag;
int haslist;
/* Begin an enum declaration.  "tag" is the tag, possibly empty.
** "haslist" is 0 if there is no list of enumerators, or non-0
** otherwise.  This routine declares the tag and initializes the
** current enumerator value.
**
** If there is an enumeration list and this is a top-level decl.,
** turn off potential complaints about useless declarations.
*/
{
    dcl_tag(tag,haslist,D_NOFORCE);	/* sets base type */
    if (haslist == D_LIST) {
	dcl_state.dcl_enumval = -1;	/* initialize enumerator value, which
					** is pre-incremented
					*/
	if (dcl_state.dcl_context == DCL_TOP)
	    dcl_complain = 0;
    }
    return;
}

DN
dcl_elist(oldlist, newmbr)
DN oldlist;
DN newmbr;
/* Make a list of enumerators out of an oldlist and a new
** member (newmbr).  If oldlist is null, start a new list
** with newmbr as its first member.  The list is kept right
** to left because enumerators aren't order-sensitive.
*/
{
    if (oldlist != DN_NULL)
	DD(newmbr).dn_next = oldlist;
    
    return( newmbr );
}


void
dcl_e_enu(dn)
/* Finish off the current enumeration.  Called at the end
** of an enumeration list, if any.  Walk the list of
** enumerators to create the type.  Then finish up the
** type.  dn is the list of enumerators in right-to-left
** order (which doesn't matter).
*/
DN dn;
{
    int mbrno = 0;

    DEBUG(d1debug > 1, ("end of enum\n"));

    for ( ; dn != DN_NULL; ++mbrno, dn = DD(dn).dn_next)
	ty_mkmbr(dcl_state.dcl_btype, DE(dn).dn_esym);

    /* Now finish up the type. */
    ty_e_sue(dcl_state.dcl_btype, TY_SIZE(TY_INT), ALINT);
/*HOOK:  if non-int sized enums allowed, set size, align here;
** need range of values to do that
*/
    DB_SUE(dcl_state.dcl_btype);	/* do debug info. for enum */
    return;
}


DN
dcl_menu(s, hasexpr)
char * s;
int hasexpr;
/* Declare member of enumeration declaration node.  hasexpr
** is non-zero if there's an expression for the enumerator,
** 0 otherwise.
*/
{
    SX sid;
    DN new;

    sid = sy_lookup(s, SY_NORMAL, SY_CREATE);
    /* Hide previous version of symbol.  Check for proper scoping. */
    if ( !SY_ISNEW(sid) ) {
	SY_LEVEL_t lev = SY_LEVEL(sid);

	if (lev >= sy_getlev())
	    UERROR("identifier redeclared: %s", s);
	else if (lev == SL_FUNARG)
	    WERROR("enumeration constant hides parameter: %s", s);
	sid = sy_hide(sid);
    }
    SY_CLASS(sid) = SC_MOE;
    if (hasexpr == D_NOEXPR) {
	if (dcl_state.dcl_enumval == T_INT_MAX)
	    WERROR("enumerator value overflows INT_MAX (%ld)", (long) T_INT_MAX);
	dcl_state.dcl_enumval++;		/* next value */
    }
    SY_OFFSET(sid) = dcl_state.dcl_enumval;
    SY_TYPE(sid) = dcl_state.dcl_btype;	/* remember owning enum */
    /* The enumerator is in scope unless there's an initializer, in
    ** which case we wait until after evaluating it before marking
    ** it INSCOPE.
    */
    if (hasexpr == D_NOEXPR)
	SY_FLAGS(sid) |= SY_INSCOPE;	/* enumerator is now in scope */

    /* Fill in enumerator node. */
    new = newdn();
    DD(new).dn_op = DN_ENUM;
    DD(new).dn_next = DN_NULL;
    DE(new).dn_esym = sid;

    DCL_PRINT(sid, "enumerator");
    return( new );
}


DN
dcl_eexpr(dn, p)
DN dn;
ND1 * p;
/* Evaluate the initializer expression p for the enumerator
** whose declaration node is dn and set its value.  Also,
** make the enumerator "in-scope".  Look out for enumerator
** in its own initializer expression.
*/
{
    SX sid = DE(dn).dn_esym;		/* sid of enumerator */
    static void dcl_findenum();

    DEBUG(d1debug > 1, ("in dcl_eexpr()\n"));

    /* Check whether the enumerator was used in its own expression.
    ** Get the constant expression value and set it in the symbol's
    ** table entry.  Mark the symbol in-scope.
    */
    dcl_findenum(SY_NAME(sid), p);
    SY_OFFSET(sid) = dcl_state.dcl_enumval = tr_inconex(p);
    SY_FLAGS(sid) |= SY_INSCOPE;
    DEBUG(d1debug > 1,
	("--> %s gets value %ld\n", SY_NAME(sid), (long) dcl_state.dcl_enumval));
    return( dn );
}


static void
dcl_findenum(name, p)
char * name;
ND1 * p;
/* Look in tree p for a NAME node with name "name" in it.
** Complain if such is found.
** The looking uses a recursive tree walk which, although
** theoretically expensive, is usually short in practice.
** (The expression is almost always a single integer constant,
** and enumerator initializer expressions happen rarely,
** anyway.)
*/
{
    switch( optype( p->op ) ){
    case BITYPE:	dcl_findenum(name, p->right);	/*FALLTHRU*/
    case UTYPE:		dcl_findenum(name, p->left);	break;
    case LTYPE:
	if (p->op == NAME && SY_NAME(p->rval) == name)
	    WERROR("enumerator used in its own initializer: %s", name);
	break;
    }
    return;
}


void
dcl_noenumcomma()
/* Produce warning for trailing comma in enum declaration.  UNIX C
** allowed it quietly, ANSI C forbids.  Complain under -Xc, -v.
*/
{
    if (verbose || (version & V_STD_C))
	WERROR("trailing \",\" prohibited in enum declaration");
    return;
}


SX
dcl_g_arg(i)
int i;
/* Return symbol index of i-th argument (counting from 0).  If
** i is a bad argument number, return SY_NOSYM.
** Used by code (stmt.c) that fixes up register and other special
** case arguments.
*/
{
    return (i < dcl_nargs ?  ARG(i) : SY_NOSYM );
}



#ifndef	NODBG

void
dn_print(dn)
DN dn;
{
    while (dn != DN_NULL) {
	DPRINTF("%.2d:  ", dn);
	switch( DD(dn).dn_op ) {
	case DN_PTR:
	    DPRINTF("type = DN_PTR, to %d", DD(dn).dn_next);
	    break;
	case DN_ARY:
	    DPRINTF("type = DN_ARY [%d] of %d", DD(dn).dn_var.dn_arysize,
			DD(dn).dn_next);
	    break;
	case DN_FTN:
	    DPRINTF("type = DN_FTN returning %d, plist %d",
		DD(dn).dn_next, DD(dn).dn_var.dn_parmlist);
	    break;
	case DN_PARAM:
	    DPRINTF("type = DN_PARAM, next %d, tword %d",
		DD(dn).dn_next, DP(dn).dn_ptype);
	    break;
	case DN_VPARAM:
	    DPRINTF("type = DN_VPARAM, next %d", DD(dn).dn_next);
	    break;
	case DN_MBR:
	    DPRINTF("type = DN_MBR, name %s, next %d",
			SY_NAME(DSU(dn).dn_susym), DD(dn).dn_next);
	    break;
	case DN_ENUM:
	    DPRINTF("type = DN_ENUM, name %s, sid %d",
			SY_NAME(DE(dn).dn_esym), DE(dn).dn_esym);
	    break;
	default:  cerror("unknown declnode op in dn_print");
	}
	DPRINTF("\n");
	dn = DD(dn).dn_next;
    }
    return;
}


static char *
dcl_pclass(c)
SY_CLASS_t c;
/* Return print-string for class */
{
    switch(c) {
    case SC_NONE:	return("SC_NONE");
    case SC_AUTO:	return("SC_AUTO");
    case SC_EXTERN:	return("SC_EXTERN");
    case SC_REGISTER:	return("SC_REGISTER");
    case SC_STATIC:	return("SC_STATIC");
    case SC_TYPEDEF:	return("SC_TYPEDEF");
    case SC_MOS:	return("SC_MOS");
    case SC_STRUCT:	return("SC_STRUCT");
    case SC_MOU:	return("SC_MOU");
    case SC_UNION:	return("SC_UNION");
    case SC_MOE:	return("SC_MOE");
    case SC_ENUM:	return("SC_ENUM");
    case SC_LABEL:	return("SC_LABEL");
    case SC_PARAM:	return("SC_PARAM");
    case SC_ASM:	return("SC_ASM");
    default:
	cerror("unknown class %d in dcl_pclass()", c);
    }
    /*NOTREACHED*/
}


static void
dcl_print(sid, desc)
SX sid;
char * desc;
/* Print details about symbol whose ID is sid and whose description
** is desc. */
{
    static const struct symflag { SY_FLAGS_t val; const char * print; } sftab[] = {
	SY_DEFINED,	"DEF",
	SY_TENTATIVE,	"TENT",
	SY_ISREG,	"REG",
	SY_ISFIELD,	"FLD",
	SY_MOVED,	"MOVED",
	SY_TOMOVE,	"TOMOVE",
	SY_TAG,		"TAG",
	SY_MOSU,	"MOSU",
	SY_LABEL,	"LAB",
	SY_SET,		"SET",
	SY_REF,		"REF",
	SY_INSCOPE,	"INSCOPE",
	SY_DBOUT,	"DBOUT",
	0				/* end marker */
    };

    if (sid != SY_NOSYM) {
	const struct symflag * sfp = sftab;
	SY_FLAGS_t flags = SY_FLAGS(sid);
	int first = 1;
	DEBUG(d1debug,
	    ("--> Declaring %s \"%s\", symbol ID %d\n\tclass %s\n\tlevel %u\
\n\tflags ",
		    desc, ((sid == SY_NOSYM) ? "<noname>" : SY_NAME(sid)), sid,
		    dcl_pclass(SY_CLASS(sid)), SY_LEVEL(sid)));

	if (d1debug) {
	    for ( ; flags && sfp->val; ++sfp) {
		if (sfp->val & flags) {
		    if (!first)
			DPRINTF(" | ");
		    DPRINTF(sfp->print);
		    flags -= sfp->val;
		    first = 0;
		}
	    }
	    if (first)
		DPRINTF("%#o", flags);
	    else if (flags)
		DPRINTF(" | %#o", flags);
	}

	DEBUG(d1debug && SY_SAMEAS(sid) != SY_NOSYM,
		("\n\tsameas: %d", SY_SAMEAS(sid)));
	DEBUG(d1debug, ("\n\toffset: %d\n\ttype:\t",
			SY_OFFSET(sid)));
	TY_PRINT(SY_TYPE(sid),0);
    }
    else {
	DEBUG(d1debug, ("--> Declaring \"<noname>\", symbol ID 0"));
    }
    DEBUG(d1debug, ("\n"));
    return;
}

#endif
