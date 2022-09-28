/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/trees.c	55.12"
/* trees.c */

/* This module contains the first pass tree building routines. */

#include "p1.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>

int b1debug = 0;		/* debug flag */
static int tr_leftside_assign();
static int tr_ptr_assignable();
static int tr_amb_chk();
static tr_iseom();
static tr_isnpc();
static tr_islval();
static void tr_enumchk();
static ND1 * tr_promote();
ND1 * tr_newnode();
static void tr_type_mess();
static void tr_hidden();
static ND1 * tr_incrdecr();
static ND1 * tr_arith();
static ND1 * tr_uarith();
static ND1 * tr_bitwise();
static ND1 * tr_asgop();
static ND1 * tr_logic();
static ND1 * tr_assign();
static ND1 * tr_contoptr();
static ND1 * tr_cast();
static ND1 * tr_relation();
static ND1 * tr_call();
static ND1 * tr_addr();
static ND1 * tr_args();
static ND1 * tr_comma();
static ND1 * tr_star();
static ND1 * tr_qc();
static ND1 * tr_struct();
static ND1 * tr_cbranch();
static void tr_arconv();
void tr_eprint();
void tr_e1print();
static int argno;

/* some static diagnostic messages */
static const char incomp_mesg[] =
	"operands have incompatible types: op \"%s\"";  /*ERROR*/
static const char enum_mesg_op[] =
	"enum type mismatch: op \"%s\"";		/*ERROR*/
static const char enum_mesg_arg[] =
	"enum type mismatch: arg #%d";			/*ERROR*/
static const char cast_mesg[] =
	"a cast does not yield an lvalue";		/*ERROR*/

#define	FF_AMBFLGS	(FF_ISAMB|FF_ISCON)
#define FF_MASK		(FF_AMBFLGS|FF_SEFF)

#define TR_ISAMBCON(p)	((p)->flags & FF_ISCON)
#define TR_ISAMBIG(p)	((p)->flags & FF_AMBFLGS)
#define ARGNO	argno+1
#define TR_ISLVAL(p)	tr_islval(p)
#define TR_ISNPC(p)	(tr_isnpc(&(p)) != 0 )
#define TR_IC_TYPE(t)	(!TY_ISFTN(t) && TY_TYPE(t) != TY_VOID && TY_SIZE(t) == 0 )
#define TY_ALSTRUCT(t)	(TY_ALIGN(t) % ALSTRUCT == 0) 
#define TY_ISEOM(p)	tr_iseom(p)		/* is p a enum or a moe? */
#define TY_PLAINPTR(t)  (TY_ISPTR(t) && !TY_ISQUAL(t))
#define TY_NULLPTR(p)  ((p)->op == ICON && (p)->lval == 0 && (p)->rval == ND_NOSYMBOL && TY_ISINTTYPE((p)->type))
#define NO_MESS		0
#define YES_MESS	1
#define NOTLVAL		0
#define LVAL		1
#define MODLVAL		2
#define EQUALITY(o)	(o == EQ || o == NE)
#define RELATIONAL(o)	(o == LT || o == GT || o == LE || o == GE)
#define SHIFT(o)	(o == LS || o == RS || o == ASG LS || o == ASG RS)

#ifndef	NODBG
static void tr_b1print();
#define	TPRINT(p) if (b1debug) tr_b1print(p)
#else
#define	TPRINT(p)
#endif

ND1 *
tr_build(op, l, r)
register int op;
register ND1 * l;
register ND1 * r;
/* Build and expression tree from an operator (op) and two operands
** (nodes l and r).  Return a pointer to the result tree.
*/
{
    ND1 * ret;
    static ND1 * tr_opclass();

#ifndef NODBG
    if (b1debug)	/* print arguments to tr_build() */
        DPRINTF("tr_build( %s, l=%d, r=%d )\n",
                opst[op], (l ? node_no(l) : -1), (r ? node_no(r) : -1));
#endif

    cg_treeok();			/* assume tree okay going in */

    /* check for illegal void type */
    switch(op){
    case CM:
    case COMOP:
    case QUEST:
    case COLON:
    case CAST:
    case CALL:
    case UNARY CALL:
    /* ++, -- handled elsewhere. */
    case INCR:
    case DECR:
	break;
    case UNARY AND:			/* allow extern void v; &v; */
	break;
    default:
	if (optype(op) != LTYPE && l != ND1NIL) {
	    if (   TY_TYPE(l->type) == TY_VOID
		|| (optype(op) == BITYPE && TY_TYPE(r->type) == TY_VOID)
	       )
		UERROR("operand cannot have void type: op \"%s\"", opst[op]);
	}
	break;
    }
    ret = tr_opclass(op, l, r);

    /* Propagate side effect flags from either operand to operator node */
    switch( optype(ret->op) ){
    case BITYPE:
	ret->flags |= (FF_SEFF & ret->right->flags);
	/*FALLTHRU*/
    case UTYPE:
	ret->flags |= (FF_SEFF & ret->left->flags);
    }

#ifndef NODBG
    if(b1debug)
	tr_eprint( ret );
#endif

    return( ret );
}

static int
tr_ispos(p)
ND1 * p;
/* Return 1 if the object that p represents is always known to
** have a positive value under both CI4.1 or ANSI C promotion
** rules.  Specifically, the sign bit for the type will always
** be zero.
**
** Treat char special if they're non-negative.
*/
{
    T1WORD t = p->type;		/* type to consider */

    /* For ambiguous node, look under CONV to get to unambiguous
    ** part of tree.  If call operator, get to the declared
    ** return type.
    */
    if (TR_ISAMBIG(p)) {
	switch( p->op ) {
	case CONV:
	    p = p->left;
	    t = p->type;
	    break;
	case CALL:
	case UNARY CALL:
	    t = TY_DECREF(TY_DECREF(p->left->type));
	    break;
	}
    }
#ifndef C_CHSIGN
    else if (p->op == CONV && TY_TYPE(p->left->type) == TY_CHAR)
	return( 1 );		/* always positive */
#endif

    /* Bitfields are a special case.  We want the ones that always
    ** promote to a value smaller than int.
    */
    if (p->flags & FF_ISFLD) {
	switch( TY_TYPE(t) ) {
	case TY_UCHAR: case TY_USHORT: case TY_UINT:
	    /* If field is ambiguous, it must be smaller than int, which
	    ** means the value is positive as int/unsigned int.
	    */
	    if (TR_ISAMBIG(p))
		return( 1 );		/* must be non-negative */
	    break;
#ifndef SIGNEDFIELDS
	case TY_CHAR: case TY_SHORT: case TY_INT: case TY_ENUM:
	    return( 1 );		/* must promote positive */
#endif
	}
	return( 0 );
    }

    /* Positive constants are always okay. */
    if (p->op == ICON && p->rval == ND_NOSYMBOL && p->lval >= 0)
	return( 1 );

    switch( TY_TYPE(t) ){
    case TY_UCHAR:
    case TY_USHORT:
	return (TY_SIZE(t) < TY_SIZE(TY_INT));
    case TY_INT:
	if (logop(p->op))
	    return( 1 );		/* value always 0 or 1 */
    }
    return( 0 );
}


static int
tr_amb_chk(op,l,r)
int op;
ND1 * l;
ND1 * r;
/* Check whether the semantics of op are ambiguous (value-reserving
** vs. unsigned-preserving), given that at least one of the operands
** has the ambiguous semantics bit set (FF_ISAMB).  (This routine also
** checks shift operators, for which it's possible neither operand is
** ambiguous.)  Issue a warning if the other side does not already have
** an unsigned type of suitable size.
**
** Shift operators are special:  in PCC, the result is unsigned if
** either operand is unsigned.  In ANSI C, the result is unsigned only
** if the left side is unsigned.  There are three conditions to check
** for:
**	1)  Different result bits.
**	2)  Same result, but different result type.
**	3)  No difference.
** 
**
** Note the peculiar semantics of arguments l and r when op == CONV.
**      l:      the node being converted.
**      r:      the new node being created (i.e. the CONV node).
** In particular, the type and flag fields must be set in the new
** r node before calling this routine.
**
** Note that if an unambiguous operand has type "unsigned long", there is
** no ambiguity in the outcome, no message is required, and no ambiguity
** bit needs to be propagated.
**
** The message is different if the ambiguity is due to a constant whose
** type is different in UNIX C and ANSI C.
*/
{
    static const char opmesg[] =
	"semantics of \"%s\" change in ANSI C; use explicit cast"; /*ERROR*/
    static const char conmesg[] =
	"ANSI C treats constant as unsigned: op \"%s\"";	/*ERROR*/
    ND1 * mesgnode = 0;			/* flags tell which message */

    /* If left type is an unambiguous unsigned long, the result type
    ** (for all operators) is an unambiguous unsigned long.
    ** Likewise for unsigned int, if right type is no larger.
    */
    if (!TR_ISAMBIG(l)) {
	switch( TY_TYPE(l->type)) {
	case TY_UINT:
	    if (TY_SIZE(r->type) > TY_SIZE(TY_UINT))
		break;
	    /*FALLTHRU*/
	case TY_ULONG:
	    return( 0 );		/* unambiguous result */
	}
    }
   if (op == CONV) {   /* look for ambiguous conversion to floating point */
	if (TY_ISFPTYPE(r->type)) {
	    if (tr_ispos(l))
		return 0;
	    mesgnode = l;
	}
    }
    else if (shfop(op)) {			/* operands may be unambiguous */
	/* Look for cases with different result type first. */
	if (TR_ISAMBIG(l)) {
	    /* Get an unsigned result with any mode if left side has
	    ** ambiguous constant, right side is ambiguous object.
	    */
	    if (TR_ISAMBCON(l) && (r->flags & FF_ISAMB) != 0)
		return( 0 );
	}
	else if (   TY_ISSIGNED(l->type)
		 /* Right side unambiguously unsigned, or ambiguous but
		 ** not because of a constant.
		 */
		 && (   (TY_ISUNSIGNED(r->type) && !TR_ISAMBIG(r))
		     || (r->flags & FF_ISAMB) != 0
		     )
		)
		/* EMPTY */ ;
	else
	    return( 0 );		/* no change */

	/* Don't propagate information about constant, which
	** can't affect outcome.
	*/
	r->flags &= ~FF_ISCON;

	/* Look for cases with different result. */
	if (    (op == RS || op == ASG RS)
#ifndef C_SIGNED_RS
	    && verbose
#endif
	    && !tr_ispos(l)
	)
	    mesgnode = l;
	else if (!TR_ISAMBIG(l) && !TR_ISAMBIG(r))
	    return( FF_ISAMB );		/* no bits to propagate */
	
	/* Propagate existing bits. */
    } /* end shift op cases */
    else if (TY_TYPE(r->type) == TY_ULONG && !TR_ISAMBIG(r))
	return( 0 );			/* not ambiguous */
    else if (   !TR_ISAMBIG(r)
	     && TY_TYPE(r->type) == TY_UINT
	     && TY_SIZE(l->type) <= TY_SIZE(TY_UINT)
	     )
	     return( 0 );		/* not ambiguous */
    else if (!ambop(op))
	/*EMPTY*/ ;			/* fall out to return new flag bits */
    else if (tr_ispos(l) && tr_ispos(r))
	/* EMPTY */ ;			/* just propagate flag */
    else if (TR_ISAMBIG(l)) {
	if (TR_ISAMBIG(r)) {
	    mesgnode = TR_ISAMBCON(l) ? l : r;
	}
	/* Right side unambiguous.  No message if both are effectively
	** positive.  If the right side is unsigned, that forces the
	** type of the left.
	*/
	else if (! TY_ISUNSIGNED(r->type))
	    mesgnode = l;
    }
    /* Left side unambiguous. */
    else if (TR_ISAMBIG(r)) {
	if (! TY_ISUNSIGNED(l->type))
	    mesgnode = r;
    }
    
    if (mesgnode) {
	WERROR(TR_ISAMBCON(mesgnode) ? conmesg : opmesg, opst[op]);
	return( 0 );
    }
    return( (l->flags|r->flags) & FF_AMBFLGS );
}

ND1 *
tr_newnode(op)
int op;
/* interface routine to t1alloc; returns a new ND1 node with the
** flags field initialized to zero and op field assigned an operator.
*/
{
    ND1 * new = t1alloc();
    new->op = op;
    new->flags = 0;
    return(new);
}

ND1 *
tr_icon(i)
CONVAL i;
/* Make an integer constant node out of constant i. */
{
    ND1 * new = tr_newnode(ICON);

    new->type = TY_INT;
    new->lval = i;
    new->rval = ND_NOSYMBOL;
    TPRINT(new);
    return( new );
}


ND1 *
tr_int_const(s)
char * s;
/* Convert the NUL-terminated string s into an integer
** constant of the appropriate type, according to the
** rules of Sect. 3.1.3.2.  Return the resulting ICON
** node.  A constant that begins with 0 may, in fact,
** have digits 8 and 9 embedded.  Warn about same, treat
** 8 and 9 as 7+1, 7+2.
*/
{
    ND1 * new = tr_newnode(ICON);
    unsigned long result;		/* result value */
    char * end;				/* pointer to end of string */
    int isunsigned = 0;
    int islong = 0;

    new->rval = ND_NOSYMBOL;		/* unnamed ICON */

    errno = 0;
    result = strtoul(s, &end, 0);	/* take any base */
    if (errno == ERANGE){
	end = "";
        WERROR("integral constant too large");
    }
    if (*end == '8' || *end == '9') {
	/* There's an embedded decimal digit in an octal string.
	** Since the library routine can't handle this, translate
	** the number ourselves and warn about the mistake.
	*/
	result = 0;

	for (end = s; *end; ++end) {
	    if (!isdigit(*end))
		break;
	    /* Have new (sort of) octal digit.  No range check,
	    ** since old compiler didn't, either.
	    */
	    if (*end > '7')
		WERROR("bad octal digit: '%c'", *end);
	    result <<= 3;
	    result += *end - '0';
	}
    }
    
    /* Determine the type of the constant:  check explicit
    ** modifiers and the value of the object.
    **
    ** Check explicit modifiers.  At most there can be
    ** one of each of [uU] and [lL].
    */
    for ( ; *end; ++end) {
	switch (*end) {
	case 'l': case 'L':	islong = 1; break;
	case 'u': case 'U':	isunsigned = 1; break;
	default:
	    cerror("bad int modifier");
	}
    }

    /* Now categorize by value. */
    if (result > T_INT_MAX) {
	int makeunsigned = 0;
#ifndef NOLONG
	/* can be unsigned int if octal/hex, between INT_MAX and UINT_MAX */
	if (result <= T_UINT_MAX && *s == '0')
	    makeunsigned = 1;
	else if (result <= T_LONG_MAX)	/* between UINT_MAX and LONG_MAX */
	    islong = 1;
	else {				/* greater than LONG_MAX */
	    makeunsigned = 1;
	    islong = 1;
	}
#else
	/* int == long, uint == ulong */
	makeunsigned = 1;
#endif
	/* UNIX C didn't promote any constants to unsigned, just
	** to long.
	*/
	if (makeunsigned && !isunsigned) {
	    new->flags |= FF_ISCON;
	    if (version & V_CI4_1) {	
		/* duplicate old behavior:  make a signed long */
		makeunsigned = 0;
		islong = 1;
	    }
	}
	isunsigned |= makeunsigned;
    }

    if (islong) {
	if (isunsigned)
	    new->type = TY_ULONG;
	else
	    new->type = TY_LONG;
    }
    else if (isunsigned)
	new->type = TY_UINT;
    else
	new->type = TY_INT;
    
    new->lval = result;
    TPRINT(new);
    return( new );
}


ND1 *
tr_fcon(s,len)
char * s;
int len;
/* Make a floating-point constant node.  This handles long double,
** double, and float constants.  Presently, long doubles are read in
** as FCONs and typed as doubles, also their values are put in the 
** dval entry of the node.  This must change when double extended
** precision arithmetic is implemented.
** s points at the characters for the constant; len is the length of
** the string, not including a (mandatory) trailing NUL.
*/
{
    ND1 * new = tr_newnode(FCON);
    char * name;

    errno = 0;
    new->dval = FP_ATOF(s);
    switch( s[len-1] ) {
    case 'f':  case 'F':			/* float constant */
	new->dval = op_dtofp(new->dval);
	new->type = TY_FLOAT;
	name = "float";
	break;
    case 'l':  case 'L':			/* explicit long double */
	/* assume ldcheck() doesn't zero errno */
	cg_ldcheck();				/* check for disposition of
						** long double
						*/
	new->type = TY_LDOUBLE;
	name = "long double";
	break;
    default:					/* assume regular double */
	new->type = TY_DOUBLE;
	/* LATER:  will need to truncate long double to double here */
	name = "double";
	break;
    }
    if (errno) {
	UERROR("conversion of floating-point constant to %s out of range",
		name);
	/* Force known good value. */
	new->dval = FP_LTOD(1L);		/* don't use 0.0 in case of
						** division
						*/
    }
    TPRINT(new);
    return( new );
}


ND1 *
tr_ccon(s,len,iswide)
char * s;
unsigned int len;
int iswide;
/* Build a node for character constants from string s with length
** len.  Allow as many characters as will fit in a target INT.
** For consistency with PCC, characters are collected in a
** machine-dependent order.  For RTOLBYTES machines, each new
** character goes in the low-order part of the constant; earlier
** characters are shifted out of the way.  Otherwise, each new
** character is left-shifted into place.
** If iswide is non-zero, the string consists of wide characters
** which must be fetched by lx_mbtowc().
*/
{
    ND1 * new = tr_newnode(ICON);
    int nchars;
    CONVAL val = 0;                     /* result value */
    static const char mesg[] = "character constant too long";	/*ERROR*/

#ifdef LINT
    /* "nonportable character constant" */
    ln_chconst(len);
#endif

    if (iswide) {
	if (len > 1)
	    WERROR(mesg);
	val = lx_mbtowc(s);		/* get single wide character */
	new->type = T_wchar_t;
    }
    else {
	if (len > TY_SIZE(TY_INT)/TY_SIZE(TY_CHAR)) {
	    WERROR(mesg);
	    len = TY_SIZE(TY_INT)/TY_SIZE(TY_CHAR);
	}

	for (nchars = 0; nchars < len; ++nchars) {
	    CONVAL newval = tr_truncate((CONVAL)s[nchars], TY_CHAR);
	    if (nchars == 0)
		val = newval;
	    else {
# ifdef RTOLBYTES
		val <<= TY_SIZE(TY_CHAR);
		val |= (newval & BITMASK(TY_SIZE(TY_CHAR)));
# else
		val &= BITMASK(TY_SIZE(TY_CHAR)*nchars);
		val |= newval << (TY_SIZE(TY_CHAR)*nchars);
# endif
	    }
	}
	new->type = TY_INT;
    }

    new->lval = val;
    new->rval = ND_NOSYMBOL;
    TPRINT(new);
    return( new );
}


ND1 *
tr_name(s)
char * s;
/* Make a NAME node from a symbol name. */
{
    SX sid = sy_lookup(s, SY_NORMAL, SY_LOOKUP);
    int flag = 0;
    ND1 * p;

    /* Check undefined symbols to see if they correspond to
    ** built-in functions.  Treat as a built-in only if:
    **	1) No symbol table entry exists for the builtin-version.
    **	2) A symbol table entry exists for the unprefixed symbol.
    ** If those conditions hold, set the FF_BUILTIN flag.
    **
    ** Otherwise, leave undefined symbols undefined.
    ** They'll be detected in tr_build(), which allows us
    ** to create function calls implicitly.
    */
    if (   s[0] == BUILTIN_PREFIX[0]
	&& strncmp(s, BUILTIN_PREFIX, sizeof(BUILTIN_PREFIX)-1) == 0
	&& (sid = sy_lookup(s, SY_NORMAL, SY_LOOKUP)) == SY_NOSYM
	&& (sid = sy_lookup(
		    st_lookup(s+sizeof(BUILTIN_PREFIX)-1), SY_NORMAL, SY_LOOKUP)
			    ) != SY_NOSYM
	    )
	    flag = FF_BUILTIN;
    else
	sid = sy_lookup(s, SY_NORMAL, SY_CREATE);

    p = tr_symbol(sid);
    p->flags |= flag;
    return( p );
}


ND1 *
tr_symbol(sid)
SX sid;
/* Make a NAME leaf node from a symbol index. */
{
    ND1 * new = tr_newnode(NAME);

    new->rval = sid;
#ifdef FAT_ACOMP
    SY_WEIGHT(sid) = SM_WT_USE(SY_WEIGHT(sid), sm_g_weight());
#endif
    new->lval = 0;
    if (SY_CLASS(sid) == SC_MOE) new->type = TY_INT;
    else new->type = SY_TYPE(sid);
    if (TY_ISVOLATILE(new->type)
	|| ( TY_ISARY(new->type) && TY_ISMBRVOLATILE(new->type) ))
	new->flags = FF_ISVOL;
    TPRINT(new);
#ifndef LINT
    /* lint doesn't want it REF here */
    SY_FLAGS(sid) |= SY_REF;
#endif
    return( new );
}


ND1 *
tr_dotdotdot()
/* Return node representing &... .  First, find "...".
** Then build & node of type void *.
*/
{
    ND1 * retnode = tr_name(st_nlookup("...", 4));

    if ((version & V_STD_C) != 0)
	WERROR("syntax error: \"&...\" invalid");
    retnode = tr_chkname(retnode, 0);	/* not for function */
#ifdef  OPTIM_SUPPORT
    OS_UAND(retnode->rval);		/* note we're taking address */
#endif
    return( tr_generic(UNARY AND, retnode, TY_VOIDSTAR) );
}


CONVAL
tr_inconex(p)
ND1 * p;
/* Return the integral constant expression that corresponds to tree p. */
{
    CONVAL retval;
    static const char errstr[] =
		"integral constant expression expected";    /*ERROR*/

    p = op_init(p);
    if (p->op == ICON && p->rval == 0) {
        if (! TY_ISINTTYPE(p->type))
            UERROR("%s", errstr);
        retval = p->lval;
    }
    else {
	UERROR("%s", errstr);
	retval = 1;
    }
    t1free(p);
    return( retval );
}

ND1 *
tr_string(s,len,iswide)
char * s;
unsigned int len;
int iswide;
/* Turn string s, of length len (which includes the trailing NUL)
** into a STRING node.  If iswide is non-zero, the string literal
** is for wide characters.
** The resulting string type is array of "len" chars for
** non-wide strings.  For wide strings, len must be multiplied
** by the size of a wchar_t.  The type is then array of T_wchar_t.
*/
{
    ND1 * new = tr_newnode(STRING);

    /* Trailing NUL (or wide NUL) is included when storing. */
    new->string = st_nlookup(s, iswide ? len * sizeof(wchar_t) : len);

    /* array size includes NUL */
    new->type = ty_mkaryof((T1WORD) (iswide ? T_wchar_t : TY_CHAR), (SIZE) len);
    new->lval = len-1;			/* length, not including NUL */
    new->rval = iswide ? TR_ST_WIDE : 0;/* remember whether this is wide str. */
    return( new );
}

ND1 *
tr_type(t)
T1WORD t;
/* Return a TYPE tree that corresponds to type t. */
{
    ND1 * p = tr_newnode(TYPE);

    p->type = t;
    return( p );
}


ND1 *
tr_su_mbr(op, p, s)
int op;
ND1 * p;
char * s;
/* Find the structure member named s that belongs to the struct/union
** type p.  Build a struct/union member node.
** There are a variety of cases, depending on some conditions:
**	issu	left side is s/u or (if op is STREF) s/u pointer
**	r_def	right side identifier is defined as an s/u member
**	r_uniq	right side identifier is a unique s/u member
**	r_mbr	right side s/u member belongs to left side s/u
*/
{
    ND1 * new = tr_newnode(NAME);
    T1WORD t = p->type;
    SX sid = 0;				/* symbol index for member name */
    int issu = 0; 			/* 1 if left side is struct/union */
    
    /* Look below left pointer (or array, which becomes pointer) for ->,
    ** to get to actual structure type.
    */
    if (op == STREF && (TY_ISPTR(t) || TY_ISARY(t)))
	t = TY_DECREF(t);

    /* Here's a decision table for what should be done:
    **	issu	r_def	r_uniq	r_mbr	action
    **	 -	 N	  -	  -	err:  s/u mbr req'd
    **	 Y	 Y	  -	  Y	ok
    **	 Y	 Y	  N	  N	err:  ill mbr use
    **	 Y	 Y	  Y	  N	warn:  ill mbr use
    **	 N	 Y	  N	  -	err:  non-uniq mbr demands s/u
    **	 N	 Y	  Y	  -	warn:  s/u [ptr] reqd
    **					(or, if bad align, bad struct offset)
    */

    if (TY_ISSU(t)) {
	issu = 1;
	/* Try to find s as member of t. */
	sid = TY_SUEMBR(t, s);
    }
    /* Normally we should get here with sid non-0.  Otherwise
    ** permit backward compatibility by allowing unique s/u 
    ** names for incorrect or non-s/u on left.
    */
    if (sid == SY_NOSYM) {
	sid = sy_sumbr(s);		/* try to get unique name */

	if (sid != SY_NOSYM) {	/* name is unique */
	    if (issu)
	        WERROR("improper member use: %s", s);
	}
        else {
	    /* Name is not unique.  If we find an s/u member name,
	    ** complain appropriately.
	    ** Otherwise, complain and create one.
	    */
	    sid = sy_lookup(s, SY_MOSU, SY_CREATE);
	    if (SY_ISNEW(sid)) {
		UERROR("undefined struct/union member: %s", s);
		SY_CLASS(sid) = SC_MOS;
		SY_TYPE(sid) = TY_INT;
		SY_OFFSET(sid) = 0;
		SY_FLAGS(sid) |= SY_INSCOPE;
	    }
	    else {
		if (issu)
		    UERROR("improper member use: %s", s);
		else
		    UERROR("non-unique member requires struct/union%s: %s",
			op == DOT ? " object" : " pointer", s);
	    }
	}
    }

    new->rval = sid;
    new->lval = 0;
    new->type = SY_TYPE(sid);
    if (TY_ISVOLATILE(new->type)) new->flags = FF_ISVOL;
    return( new );
}

ND1 *
tr_chkname(p, forfunc)
ND1 * p;
int forfunc;
/* Check node p to see if it is a name node with an undefined symbol.
** If forfunc is non-0, the node is for a function call, so do the
** appropriate thing for an undefined name.
** Also check for uses of implicit declarations that have been moved
** to top level and may give rise to conflicts.
*/
{
    if (p->op == NAME) {
	if (p->type == TY_NONE) {
	    /* Symbol is undefined.  For function calls, define a fake
	    ** function-returning-int.  For others, just create an int.
	    */
	    T1WORD t = TY_INT;
	    SY_CLASS_t class = SC_AUTO;

	    if (forfunc) {		/* symbol is function designator */
		if (verbose)
#ifndef LINT
		    WERROR("implicitly declaring function to return int: %s()", SY_NAME(p->rval));
#else
		    BWERROR(10, SY_NAME(p->rval));	/* buffer same msg */
#endif
		t = TY_FRINT;		/* function-returning-int */
		class = SC_EXTERN;
	    }
	    else
		UERROR("undefined symbol: %s", SY_NAME(p->rval));

	    /* Declare symbol at current level. */
	    p->rval = dcl_defid(p->rval, t, DS_NORM, class, sy_getlev());
	    p->type = SY_TYPE(p->rval);
	}
	else if (SY_FLAGS(p->rval) & SY_MOVED) {
	    /* The version that has been found is an external declaration
	    ** that moved at the end of an earlier block.  Warn about the use,
	    ** except if it's a function that returns int (for which it
	    ** doesn't matter) or one that looks like int, so long as it
	    ** doesn't have a prototype or hidden type information.  (The
	    ** hidden information couldn't be from a function definition
	    ** because the symbol entry was "moved".)
	    ** Then, to avoid future messages, create a hiding symbol table
	    ** entry.
	    */
	    if (   ! forfunc
		|| TY_EQTYPE(TY_FRINT, p->type) <= 0
		|| TY_HASPROTO(p->type)
		|| TY_HASHIDDEN(p->type)
		)
		WERROR("using out of scope declaration: %s", SY_NAME(p->rval));
	    p->rval = sy_hide(p->rval); /* shut off further similar messages by
					** making non-hidden entry for symbol
					*/
	    SY_FLAGS(p->rval) |= SY_INSCOPE;
	}
    }
    return ( p );
}

ND1 *
tr_sizeof(p)
ND1 * p;
/* Calculate size of operand's type.  Check for valid types and
** expressions.  Free the tree and return an ICON.
*/
{
    CONVAL retval = 0;

    if (TY_ISFTN(p->type))
	UERROR("cannot take sizeof function: %s", SY_NAME(p->rval));
    else if (p->type == TY_VOID)
	UERROR("cannot take sizeof void");
    else if ((p->op == DOT || p->op == STREF) && (p->flags & FF_ISFLD) != 0)
	WERROR("cannot take sizeof bit-field: %s", SY_NAME(p->right->rval));
    else {
	T1WORD t =
	    ty_chksize(p->type, "sizeof()",
			(TY_CVOID | TY_CTOPNULL | TY_CSUE), -1);
	retval = TY_SIZE(t);
    }
    
    t1free(p);
    p = tr_icon((CONVAL) (retval / TY_SIZE(TY_CHAR)) );	/* size in bytes */
    p->type = T_size_t;			/* type for sizeof() */
    return(p);
}

static ND1 *
tr_opclass(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* Determines the class of an operator, calls the appropriate
** routine, and returns a pointer to the expression tree built.
*/
{
    switch(op){
    case PLUS:
    case MINUS:
    case MUL:
    case DIV:
    case MOD:
        return(tr_arith(op, l, r));
    case INCR:
    case DECR:
        return(tr_incrdecr(op, l, r));
    case UPLUS:
    case UNARY MINUS:
    case NOT:
    case COMPL:
	return(tr_uarith(op, l));
    case RS:
    case LS:
    case AND:
    case OR:
    case ER:
	return(tr_bitwise(op, l, r));
    case ASG MUL:
    case ASG DIV:
    case ASG MOD:
    case ASG AND:
    case ASG OR:
    case ASG ER:
    case ASG LS:
    case ASG RS:
    case ASG PLUS:
    case ASG MINUS:
	return(tr_asgop(op, l, r));
    case EQ:
    case NE:
    case LE:
    case LT:
    case GE:
    case GT:
    case UGT:
    case UGE:
    case ULT:
    case ULE:
        return(tr_relation(op, l, r));
    case ANDAND:
    case OROR:
	return(tr_logic(op, l, r));
    case ASSIGN:
	return(tr_assign(op, l, r));
    case CAST:
	return(tr_cast(l, r));
    case CALL:
    case UNARY CALL:
	return(tr_call(op, l, r));
    case UNARY AND:
	return(tr_addr(l));
    case CM:
    case COMOP:
	return(tr_comma(op, l, r));
    case QUEST:
    case COLON:
	return(tr_qc(op, l, r));
    case STREF:
    case DOT:
	return(tr_struct(op, l, r));
    case STAR:
	return(tr_star(l));
    case CBRANCH:
	return(tr_cbranch(l, r));
    case LB:
        /* recognize array subscripting */
	/* got to call tr_build() each time to pass flags */
        return( tr_build( STAR, tr_build( PLUS, l, r ), ND1NIL ) );
    default:
	cerror("can't handle OP %s", opst[op]);
	/*NOTREACHED*/
     }
}

static ND1 *
tr_asgop(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* build an assignment-operator node; left side must be a modifiable lvalue */
{
    T1WORD ltype = l->type;	/* save original left type */
    ND1 * p;

    /* Left-side operator has to be a modifiable lvalue, otherwise return l. */
    if (!tr_leftside_assign(l, op)) {
	t1free(r);
	return(l);
    }

    /* A structure or union may not be one of the operands */
    if (TY_ISSU(ltype) || TY_ISSU(r->type)){
	UERROR(incomp_mesg, opst[op]);
	return(l);
    }
    /* For += and -=, the left operand may be a pointer, in which
    ** case the right operand shall have integral type.
    */
    if (TY_ISPTR(ltype)){
	switch (op){
	case ASG MINUS:
	case ASG PLUS:
	    if (TY_ISINTTYPE(r->type))  break;
	    /*FALLTHRU*/
	default:
            UERROR(incomp_mesg, opst[op]);
	    r->type = TY_INT;	/* force to int to reduce further complaints */
	}
    }

    /* do bitwise or arithmetic promotions */
    switch (op){
    case ASG LS: case ASG RS: case ASG AND: case ASG OR: case ASG ER:
	p = tr_bitwise(op, l, r);
	break;
    default:
        p = tr_arith(op, l, r);
    }

    p->type = ltype;			/* result type is type of left */
    p->flags |= FF_SEFF;		/* node has side effects */
    return (p);
}
	
static ND1 *
tr_qc(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* build a QUEST or COLON node for the ?: conditional operator */
{
    ND1 * new = tr_newnode(op);
    register T1WORD ltype, rtype;
    T1WORD t;				/* new type */

    if (TY_ISARYORFTN(l->type))		/* do this for both QUEST and COLON */
	l = tr_contoptr(l);

    /* return if op is QUEST */
    if ( op == QUEST ){
	/* the question operand must have scalar type */
	if (!TY_ISSCALAR(l->type))
	    UERROR("first operand must have scalar type: op \"?:\"");
	/* Propagate ambiguous semantics flag. */
	new->flags |= (r->flags & FF_AMBFLGS);
	t = r->type;
	goto qc_fillnode;
    }

    /* The rest of the code pertains to the COLON */
    /* convert arrays and functions to "pointer to" */
    if (TY_ISARYORFTN(r->type))		/* we converted the left already */
	r = tr_contoptr(r);

    ltype = l->type;
    rtype = r->type;

    /* Both operands have arithmetic type, convert by the
    ** usual arithmetic promotion rules.
    */
    if (TY_ISNUMTYPE(l->type) && TY_ISNUMTYPE(r->type)){
        /* promote type, if necessary */
        l = tr_promote( l );
        r = tr_promote( r );

	/* perform arithmetic conversion */
	tr_arconv(&l, &r);

	new->flags |= ((l->flags|r->flags) & FF_AMBFLGS);
	t = l->type;
    }
    else if (TY_EQTYPE(ltype,rtype) > 0) {
	/* Other types that are exactly equivalent need nothing further. */
	t = ltype;
    }
    /* If both operands are pointers to differently qualified versions
    ** of compatible types, the result has the composite type. We have
    ** 		qual(1) T(L) * : qual(2) T(R) *
    ** We need the basic composite type:
    **		comp_type = composite of T(L) and T(R).
    ** We need the qualified type of the two sides which is the union of
    ** the two:  qual(1) U qual(2) => qual(1U2).
    ** We get the complete composite type by calling ty_mkqual by passing
    ** comp_type and qual(1U2).
    ** Pointers to unqualified objects are taken care of here also.
    */
    else if(TY_ISPTR(ltype) && TY_ISPTR(rtype)) {
	T1WORD deref_l = TY_DECREF(ltype);
	T1WORD deref_r = TY_DECREF(rtype);
	int eqtype = TY_EQTYPE(TY_UNQUAL(deref_l), TY_UNQUAL(TY_DECREF(rtype)));
	int quals = TY_GETQUAL(deref_l) | TY_GETQUAL(deref_r);
	
	if (eqtype != 0) {		/* types compatibly equivalent */
	    if (eqtype < 0)
		WERROR(incomp_mesg, ":");

	    /* For (mostly) compatible types, the result type is a pointer
	    ** to the union-qualified composite of the pointed-to types.
	    */
	    if (eqtype != 0) {
		t = ty_mkptrto(
			ty_mkqual(
			    ty_mkcomposite(TY_UNQUAL(deref_l), TY_UNQUAL(deref_r)),
			    quals
			)
		    );
	    }
	}
	else {				/* types incompatible */
	    /* If one of the operands is a pointer to an object or incomplete
	    ** type and the other is a pointer to a qualified or unqualified
	    ** version of void, the pointer to an object or incomplete type
	    ** is converted to type pointer to void, and the result has that
	    ** type, qualified by the union of qualifiers.
	    */

	    if (!TY_ISFTN(deref_l) && TY_TYPE(deref_r) == TY_VOID)
		t = l->type = ty_mkptrto(ty_mkqual(TY_VOID, quals));
	    else if (!TY_ISFTN(deref_r) && TY_TYPE(deref_l) == TY_VOID)
		t = r->type = ty_mkptrto(ty_mkqual(TY_VOID, quals));
	    else {			/* incompatible pointer types */
		UERROR(incomp_mesg, ":");
		t = ltype;		/* choose one arbitrarily */
	    }
	}
	
    }
    /* One may be pointer and the other a null pointer constant */ 
    else if ( TY_ISPTR(ltype) && TR_ISNPC(r) )
	t = r->type = ltype;
    else if ( TY_ISPTR(rtype) && TR_ISNPC(l) )
	t = l->type = rtype;
	/* This is here to be compatible with UNIX C where we allow
	** illegal integer/pointer combinations with a warning.
	** Paint type of ICON to be the type of the pointer.
	*/
    else if (   (TY_ISPTR(rtype) && TY_ISINTTYPE(ltype))
	     || (TY_ISPTR(ltype) && TY_ISINTTYPE(rtype))
	    ){
		WERROR("improper pointer/integer combination: op \":\"");
		if (TY_ISPTR(rtype))  /* paint type */
		    t = l->type = rtype;
		else
		    t = r->type = ltype;
    }
    else {
	UERROR(incomp_mesg, ":");
	t = ltype;			/* choose a type arbitrarily */
    }

    /* Fill in new node. */
qc_fillnode: ;
    new->type = t;
    new->right = r;
    new->left = l;
    return (new);
}

static ND1 *
tr_struct(op, l , r)
int op;
ND1 * l;
ND1 * r;
/* handle struct and union references, build DOT and STREF nodes */
{
    ND1 * new = tr_newnode(op);
    int lqual;				/* left side qualifiers */

    new->type = r->type;

    switch(op){
    case DOT:
	/* If type is not a struct, do not allow as left side of DOT:
	**	- unaligned with struct types;
	**	- arrays or functions;
	**	- calls to functions;
	**	- enum members;
	**	- struct/union/enum tags;
	**	- pointers.
	*/
	if ( !TY_ISSU(l->type) ){
	    static const char str1[] =
		"left operand of \".\" must be struct/union object"; /* ERROR */
	    if (  TY_ISARYORFTN(l->type)
	        || callop(l->op)
	        || TY_ISEOM(l)
	        || TY_ISPTR(l->type)
		|| (l->op == NAME && SY_REGNO(l->rval) != SY_NOREG)
	       )
	        UERROR("%s", str1);
	    else {
	        static const char str2[] =
		    "cannot access member of non-struct/union object"; /*ERROR*/
	        WERROR("%s", str1);
		if (SY_FLAGS(r->rval) & SY_ISFIELD){
		    int fsiz, off;
		    SY_FLDUPACK(r->rval, fsiz, off);
		    if (TY_SIZE(l->type) < (fsiz + off) )
		        UERROR("%s", str2);
		}
	        else if (TY_SIZE(l->type) < (TY_SIZE(r->type) + SY_OFFSET(r->rval)) )
		    UERROR("%s", str2);
	    }
	}
	lqual = TY_GETQUAL(l->type);
	break;
    case STREF:
    {
	static const char mesg[] =
	    "left operand of \"->\" must be pointer to struct/union"; /*ERROR*/
	/* Type must be struct/union and a pointer. */
	if (TY_ISARY(l->type))
	    l = tr_contoptr(l);

        if ( TY_ISPTR(l->type)){
	    T1WORD t = TY_DECREF(l->type);

	    lqual = TY_GETQUAL(t);

	    if ( TY_ISSU(t) ) break;

	    /* If type is a pointer, but not a struct/union pointer
	    ** allow anything but pointer to void (which need not be
	    ** supported for compatibility).
	    */
	    if (TY_TYPE(t) != TY_VOID) {
		WERROR(mesg);
		break;
	    }
	    /* Fall into message. */
	}
	/* not a pointer */
	UERROR(mesg);
	l->type = ty_mkptrto(l->type);	/* convert to pointer to keep going */
	lqual = TY_GETQUAL(l->type);
	break;
    }
    } /* end switch */

    /* Qualify result based on left operand qualifier. */
    new->type = ty_mkqual(new->type, lqual);

    /* if left or right is has volatile flag set, do so in new */
    new->flags |= (l->flags|r->flags) & FF_ISVOL;

    /* If it is a bit-field, set flags to reflect so */
    if (SY_FLAGS(r->rval) & SY_ISFIELD)
        new->flags |= FF_ISFLD;

    /* Set flag to indicate that left side of dot is not modifiable lvalue. */
    if (op == DOT) {
	switch( l->op ){
	case DOT:
	    if ((l->flags & FF_BADLVAL) == 0)
		break;
	    /*FALLTHRU*/			/* propagate flag */
	case CALL:
	case UNARY CALL:
	case QUEST:
	case COMOP:
	    new->flags |= FF_BADLVAL;
	    break;
	}
    }

    new->left = l;
    new->right = r;
#ifndef NODBG
    if(b1debug)
	tr_eprint( new );
#endif
    return (new);
}

static ND1 *
tr_uarith(op, l)
int op;
register ND1 * l;
/* routine for building nodes for unary arithmetic operators: !, -, +, ~ */
{
    ND1 * new = tr_newnode(op);

    /* "The operand of the unary + operator shall have scalar type;
    ** of the - operator, arithmetic type; of the ~ operator,
    ** integral type; of the ! operator, scalar type." (3.3.3.3)
    */
    switch(op){
    case NOT:
	if (TY_ISARYORFTN(l->type))
	    l = tr_contoptr(l);
	if (!TY_ISSCALAR(l->type)) tr_type_mess("!", "scalar");
	break;
    case UPLUS:
	l = tr_promote( l );
	if (!TY_ISNUMTYPE(l->type)) tr_type_mess("UNARY +", "arithmetic");
	break;
    case UNARY MINUS:
	l = tr_promote( l );
	if (!TY_ISNUMTYPE(l->type)) tr_type_mess("UNARY -", "arithmetic");
	break;
    case COMPL:
	l = tr_promote( l );
	if (!TY_ISINTTYPE(l->type)) tr_type_mess("~", "integral");
	break;
    }

    /* set node info */
    if (op == NOT)
	new->type = TY_INT;
    else {
        new->flags |= (FF_MASK & l->flags);
	new->type = l->type;
    }

    new->left = l;
    new->right = ND1NIL;
    return (new);
}

static ND1 *
tr_arith(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* Build binary arithmetic node for these operators and
** their assignment operator equivalents:
**	+ - * / %
** The assignment operators have already been checked for
** appropriateness with respect to pointers and integers.
*/
{
    ND1 * new = tr_newnode(op);
    T1WORD ltype;
    T1WORD rtype;
    int flags;

    /* Convert array/function reference. */
    if (TY_ISARYORFTN(l->type))
	l = tr_contoptr(l);
    if (TY_ISARYORFTN(r->type))
	r = tr_contoptr(r);

    /* Skim off unusual cases:
    **		ptr + int
    **		ptr - int
    **		ptr - ptr
    */
    if (op == PLUS && TY_ISPTR(r->type)) {
	/* Reverse operands to simplify life. */
	ND1 * temp = l;
	l = r;
	r = temp;
    }
    ltype = l->type;
    rtype = r->type;

    if (TY_ISPTR(ltype)) {
	T1WORD pltype = TY_DECREF(ltype);
	CONVAL lsize;

	switch (op) {
	case PLUS:
	case ASG PLUS:
	case MINUS:
	case ASG MINUS:
	    /* Have a potentially valid pointer operation.  Verify types. */
	    if (   !TY_ISPTROBJ(ltype)
		|| (TY_ISPTR(rtype) && !TY_ISPTROBJ(rtype))
	    ) {
		UERROR("cannot do pointer arithmetic on operand of unknown size");
		lsize = 1;
	    }
	    else
		lsize = TY_SIZE(pltype);	/* remember size of object */

	    if (op == MINUS && TY_ISPTR(rtype)) {
		T1WORD prtype = TY_DECREF(rtype);
		if (TY_EQTYPE(TY_UNQUAL(pltype), TY_UNQUAL(prtype)) <= 0) {
		    static const char ptrsub[] =
			"improper pointer subtraction"; /*ERROR*/
		    if (lsize == TY_SIZE(prtype))
			WERROR(ptrsub);
		    else
			UERROR(ptrsub);
		}
		new->left = l;
		new->right = r;
		new->type = T_ptrdiff_t;
		return( tr_arith(DIV, new, tr_icon((CONVAL) BITOOR(lsize))));
	    }

	    /* ptr +/- something */
	    if (! TY_ISINTTYPE(rtype)) {
		/* Bad type.  Force good type and continue. */
		UERROR(incomp_mesg, opst[op]);
		r = tr_generic(CONV, r, TY_INT);
		rtype = TY_INT;
	    }
	    new->left = l;
	    new->right = tr_arith(MUL, r, tr_icon((CONVAL) BITOOR(lsize)));
	    new->type = l->type;
	    return( new );
	} /* end switch */
    } /* end pointer cases */

    /* Check for appropriate types for various operators.  MOD
    ** must have integral type.  Others may have arithmetic type.
    */
    if (op == MOD || op == ASG MOD) {
	if (!TY_ISINTTYPE(ltype) || !TY_ISINTTYPE(rtype))
	    tr_type_mess(opst[op], "integral");
    }
    else if (!TY_ISNUMTYPE(ltype) || !TY_ISNUMTYPE(rtype)) {
	switch( op ){
	case PLUS:  case ASG PLUS:
	case MINUS: case ASG MINUS:
	    UERROR(incomp_mesg, opst[op]);
	    break;
	default:
	    tr_type_mess(opst[op], "arithmetic");
	    break;
	}
    }

    flags = (l->flags | r->flags) & FF_MASK;	/* initial flags */
    l = tr_promote(l);
    r = tr_promote(r);
    /* ltype, rtype no longer valid. */

    if (TR_ISAMBIG(l) || TR_ISAMBIG(r))
	flags = tr_amb_chk(op, l, r);

    /* Convert types if not the same.  This may force all kinds
    ** of inappropriate types to int.
    */
    tr_arconv(&l, &r);

    /* The result type is the type of the left (especially for
    ** assignment ops).
    */
    new->type = l->type;
    new->left = l;
    new->right = r;
    new->flags |= flags;
    return( new );
}


static ND1 *
tr_bitwise(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* build bitwise operator nodes: |, &, ^, <<, >> */
{
    ND1 * new = tr_newnode(op);

    /* Each of the operands shall  have integral type. */
    if (!TY_ISINTTYPE(l->type) || !TY_ISINTTYPE(r->type))
	tr_type_mess (opst[op], "integral");

    /* Integral promotions are performed on each of the operands. */
    l = tr_promote( l );
    r = tr_promote( r );


    /* Check all shift operators for ambiguity.
    ** UNIX C did standard binary operator promotions.
    ** Check for change from that.
    */
    if (TR_ISAMBIG(l) || TR_ISAMBIG(r) || shfop(op))
	new->flags |= tr_amb_chk(op, l, r);

    /* For the shift operators LS, RS, ASG LS, and ASG RS
    ** the right operand is converted to int; the type of the
    ** result is that of the promoted left operand. (3.3.7)
    ** The compatibility promotion rules require promoting
    ** both operands to a common type.
    */
    if (shfop(op)) {
	/* Apply different promotion rules. */
	if (version & V_CI4_1)
	    tr_arconv(&l, &r);
	/* Convert right to int in either case. */
	if (! TY_EQTYPE(r->type, TY_INT))
	    r = tr_generic(CONV, r, TY_INT);
    }
    else {
        /* perform arithmetic conversions */
	tr_arconv(&l, &r);
    }
    new->right = r;
    new->left = l;
    new->type = l->type;
    return (new);
}

static ND1 *
tr_incrdecr(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* Build a node for the ++ or -- operator.  This routine is used
** for both prefix and postfix incr and decr.  If the operand is
** in the left tree, it is prefix incr or decr, else it is
** postfix (and operand is in right tree).
*/
{
    ND1 * new = tr_newnode(op);
    register T1WORD type;
    int post = 0;

    /* Is this post_incrdecr? */
    if (l == ND1NIL){
 	post = 1;	/* mark it post incr or decr */
	l = r;	/* switch left and right trees */
	r = ND1NIL;
     }
     type = l->type;		/* type of node is left node type */

    /* check for unknown size */
    if (TY_ISPTR(l->type) && !TY_ISPTROBJ(l->type)) {
        UERROR("unknown operand size: op \"%s\"", opst[op]);
        return (l);	
    }
    /* operand has to be a modifiable lvalue and of scalar type */
    /* scalar = integer || pointer || float type		*/
    if(!TY_ISSCALAR(type)){
	tr_type_mess((op == INCR) ? "++" : "--", "scalar");
        return (l);
    }
    /* Check to see if operand is a modifiable lvalue. */
    if (!tr_leftside_assign(l, op)) return (l);

    /* build an ICON right node if the type is integer or ptr type */
    if ( TY_ISINTTYPE(type) ){		/* integer type */
	r = tr_icon((CONVAL) 1); 
	r->type = type;
    }
    else if ( TY_ISPTR(type) )		/* pointer type */
        r = tr_icon( (CONVAL) (TY_SIZE(TY_DECREF(type))/TY_SIZE(TY_CHAR)) );

    /* else if float or double - build FCON right node */
    else {				/* float type */
	r = tr_newnode(FCON);
	r->type = type;
        r->dval = FP_LTOD(1L);
    }
    /* insert node info */
    new->right = r;
    new->left = l;
    new->type = type;
    if (post == 0){
      new->op = (op == INCR ? ASG PLUS : ASG MINUS);
      if (TY_ISINTTYPE(type)) new->right->type = TY_UNQUAL(type);
    }
    new->flags |= FF_SEFF;		/* operations have side effects */
    return (new);
}

static ND1 *
tr_relation(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* build a node for relational or equality operator */
{
    ND1 * new = tr_newnode(op);
    static const char improper[] =
	"improper pointer/integer combination: op \"%s\""; /*ERROR*/
    T1WORD ltype = l->type;
    T1WORD rtype = r->type;

    new->type = TY_INT;

    if (TY_ISARYORFTN(ltype)) {
	l = tr_contoptr(l);
	ltype = l->type;
    }
    if (TY_ISARYORFTN(rtype)) {
	r = tr_contoptr(r);
	rtype = r->type;
    }

    /* Handle numeric types first. */
    if (TY_ISNUMTYPE(ltype) && TY_ISNUMTYPE(rtype)) {
	/* Take care of enum check before doing any promotions. */
	if (verbose && TY_ISEOM(l) && TY_ISEOM(r)) {

	    /* Look for:
	    **		enum <op> moe'
	    **		moe' <op> enum
	    **		enum <op> enum
	    **		moe' <op> moe'
	    */
	    if (TY_TYPE(ltype) == TY_ENUM)
		tr_enumchk(r, ltype, op);
	    else if (TY_TYPE(rtype) == TY_ENUM)
		tr_enumchk(l, rtype, op);
	    /* Both must be enum constants. */
	    else if (! TY_EQTYPE(SY_TYPE(l->rval), SY_TYPE(r->rval)))
		WERROR("enum constants have different types: op \"%s\"",
			    opst[op]);
	}

        /* promote types if necessary */
        l = tr_promote( l );
        r = tr_promote( r );
	if (TR_ISAMBIG(l) || TR_ISAMBIG(r))
	    new->flags |= tr_amb_chk(op, l, r);

        /* perform arithmetic conversions */
	tr_arconv(&l, &r);
    }
    /* End numeric types, begin pointer types. */
    else if (TY_ISPTR(ltype) && TY_ISPTR(rtype)) {
	/* Consider unqualified types pointed at. */
	T1WORD tl = TY_UNQUAL(TY_DECREF(ltype));
	T1WORD tr = TY_UNQUAL(TY_DECREF(rtype));
	int eqtype = TY_EQTYPE(tl, tr);
	int warn = 0;			/* no message required */

	if (RELATIONAL(op)) {
	    /* Can't compare function pointers this way. */
	    if (eqtype <= 0 || TY_ISFTN(tl))
		warn = 1;
	}
	else if (eqtype <= 0) {
	    /* These are equality operators with type mismatch.
	    ** If one side is qualified or unqualified void *,
	    ** convert the other to that type.
	    */
	    if (TY_TYPE(tl) == TY_VOID && !TY_ISFTN(tr))
		r = tr_generic(CONV, r, ltype);
	    else if (TY_TYPE(tr) == TY_VOID && !TY_ISFTN(tl))
		l = tr_generic(CONV, l, rtype);
	    else
		warn = 1;
	}
	
	/* Done pointer/pointer checks. */
	if (warn)
	    WERROR("operands have incompatible pointer types: op \"%s\"",
		opst[op]);
    }
    /* Neither numeric/numeric nor pointer/pointer.
    ** Look for null pointer constant cases, equality ops only.
    */
    else if (TY_ISPTR(ltype) && TY_ISINTTYPE(rtype)) {
	if (! (EQUALITY(op) && TR_ISNPC(r)))
	    WERROR(improper, opst[op]);
	r = tr_generic(CONV, r, ltype);
    }
    else if (TY_ISPTR(rtype) && TY_ISINTTYPE(ltype)) {
	if (! (EQUALITY(op) && TR_ISNPC(l)))
	    WERROR(improper, opst[op]);
	l = tr_generic(CONV, l, rtype);
    }
    else
	UERROR(incomp_mesg, opst[op]);	/* fundamentally incompatible ops */
    
    new->left = l;
    new->right = r;
    /* ltype, rtype invalid now. */
    if (   RELATIONAL(op)
	&& (   TY_ISUNSIGNED(l->type) || TY_ISPTR(l->type)
	    || TY_ISUNSIGNED(r->type) || TY_ISPTR(r->type)
	   )
    )
	new->op += (ULE-LE);		/* make unsigned version of test */
    return( new );
}


static ND1 *
tr_call( op, l, r )
int op;
ND1 *l;
ND1 *r;
/* tr_call builds nodes for call to function operators */
{
    ND1 * new;
    T1WORD funtype;
    int funargno;			/* number of known function params */
    ND1 * ll;

    if (!TY_ISPTR(l->type))
	l = tr_generic( UNARY AND, l, ty_mkptrto(l->type));

    funtype = TY_DECREF(l->type);	/* get presumed function type */

    /* left operator must be a function */
    if (!TY_ISFTN(funtype)){
	UERROR("function designator is not of function type");
	return (l);
    }

    new = tr_newnode(op);
    new->left = l;
    argno = 0;				/* set by tr_args() */
    new->right = (op == CALL) ? tr_args(r, funtype) : ND1NIL;


    /* Check for mismatches on number of arguments.
    ** These are warnings:
    **		too few, hidden
    **		too many, hidden
    ** These are fatals:
    **		too few, prototype
    **		too many (prototype and no "...")
    ** The warnings about hidden information only occur for
    ** verbose.  Otherwise function calls in the same file with
    ** old-style varargs functions would give erroneous warnings
    ** on valid old-style code.
    */

    if ((funargno = TY_NPARAM(funtype)) >= 0 && funargno != argno) {
	static const char mesg[] =
		"%s mismatch: %d arg%s passed, %d expected";	/*ERROR*/

	if (TY_HASPROTO(funtype)) {
	    if (argno < funargno || !TY_ISVARARG(funtype))
		UERROR(mesg, "prototype", argno,
			(argno != 1 ? "s" : ""), funargno);
	}
#ifdef	LINT
	/* lint is verbose by default; lint can provide information
	** about variable arguments through VARARGS comment.
	*/
	else if (argno < funargno || !TY_ISVARARG(funtype))
#else
	/* TY_ISVARARG(funtype) always false for compiler for hidden info */
	else if (verbose)		/* hidden info */
#endif
	    WERROR(mesg, "argument", argno,
		    (argno != 1 ? "s" : ""), funargno);
    }

    /* Node type is effective return type of call.  It's
    ** ambiguous if it's a small unsigned type, or if
    ** the left operand is a NAME node with the SY_AMBIG
    ** bit set.
    */
    new->type = TY_ERETTYPE(funtype);
    funtype = TY_DECREF(funtype);	/* declared return type */
    if (TY_ISUNSIGNED(funtype) && TY_SIZE(funtype) < TY_SIZE(TY_INT))
	new->flags |= FF_ISAMB;
    else if (   l->op == UNARY AND
	     && (ll = l->left)->op == NAME
	     && (SY_FLAGS(ll->rval) & SY_AMBIG) != 0
    ) {
	new->flags |= FF_ISAMB;
	/* Set "int" for -Xt, "unsigned int" for -Xa/-Xc. */
	new->type = (version & V_CI4_1) ? TY_INT : TY_UINT;
    }
    /* check to see if return type has a known size */
    else if (TY_ISSUE(new->type) && TY_SIZE(new->type) == 0)
        UERROR("cannot return incomplete type");

    new->flags |= FF_SEFF;		/* side effects */
    new->string = al_g_regs();		/* currently active registers */
    al_call(new);			/* function calls may affect register
					** allocation
					*/
    return (new);
}

static ND1 *
tr_args( node, funtype )
ND1 *node;
T1WORD funtype;
/* build an ARG node */
{
    ND1 * new;				/* pointer to new node */

    if (node->op == CM){	/* list of arguments separated by comma */
	node->left = tr_args(node->left,funtype);
	node->right = tr_args(node->right,funtype);
	return (node);
    }

    /* if array, convert to "pointer to" */ 
    if(TY_ISARYORFTN(node->type))
	node = tr_contoptr(node);

    /* In case of errors, new is old node. */
    new = node;

    /* "An argument may be any expression of any object type." */ 
    if (TY_TYPE(node->type) == TY_VOID)
	UERROR("void expressions may not be arguments: arg #%d",ARGNO);
    else if (TY_ISSUE(node->type) && TY_SIZE(node->type) == 0)
	UERROR("argument cannot have unknown size: arg #%d", ARGNO);
    else {	
	char buff[100];
        new = tr_newnode(FUNARG);

	if (TY_HASPROTO(funtype) && argno < TY_NPARAM(funtype)){
            T1WORD type = TY_PROPRM(funtype,argno);
#ifndef NODBG
            if(b1debug > 1){
                DPRINTF("argno=%d funtype=",argno);
                ty_print(type); DPRINTF(" node->type=");
                ty_print(node->type); DPRINTF("\n");
            }
#endif
	    /* convert if arg type does not match prototype */
	    sprintf(buff,"argument is incompatible with prototype: arg #%d",ARGNO); /*UERROR*/
	    node = tr_conv(node, type, buff, TR_ARG);
        }
        else {
	    /* Process according to regular arithmetic promotion rules. */
	    if (TY_TYPE(node->type) == TY_FLOAT)
		node = tr_generic( CONV, node, TY_DOUBLE );
	    else
            	node = tr_promote(node);

	    /* Check promoted parameter type against hidden information,
	    ** if any.
	    */
	    
	    if (   verbose
		&& TY_HASHIDDEN(funtype) && argno < TY_NPARAM(funtype)
	    ) {
		T1WORD hidtype = TY_UNQUAL(TY_PROPRM(funtype, argno));
		if (TY_EQTYPE(hidtype, TY_UNQUAL(node->type)) <= 0)
		    tr_hidden(hidtype, TY_UNQUAL(node->type));
	    }
	}
	new->left = node;
	new->type = node->type;
	new->right = ND1NIL;
	new->flags = node->flags & FF_MASK;
    }
    argno++;
    return (new);
}

static void
tr_hidden(tfunc,targ)
T1WORD tfunc;
T1WORD targ;
/* Test for suitability of passing an argument of type targ
** to an old-style function with a known, hidden parameter type
** tfunc.  Issue diagnostic where appropriate.
** The types are assumed to be incompatible.
**
** The possible types for both arguments are those resulting from
** the default argument promotions:
**	int/unsigned int/signed int
**	enum
**	long/unsigned long/signed long
**	pointers
**	s/u
*/
{
    T1WORD gtarg = TY_TYPE(targ);	/* generic type of argument */

    switch(TY_TYPE(tfunc)){
    case TY_PTR:
	if (gtarg == TY_PTR && tr_ptr_assignable(tfunc, targ))
	    return;			/* pointers okay */
	break;
    case TY_INT:
    case TY_UINT:
    case TY_SINT:
    case TY_ENUM:
	switch(gtarg){
	case TY_INT:
	case TY_UINT:
	case TY_SINT:
	case TY_ENUM:
	    return;			/* these are okay */
	}
	break;
    case TY_LONG:
    case TY_ULONG:
    case TY_SLONG:
	switch(gtarg){
	case TY_LONG:
	case TY_ULONG:
	case TY_SLONG:
	    return;			/* these are okay */
	}
	break;
    }
    /* Everything else is a mismatch. */
    WERROR("argument does not match remembered type: arg #%d", ARGNO);
    return;
}


static ND1 *
tr_addr( l )
ND1 *l;
/* build a UNARY AND node */
{
    /* "The operand of the unary & operator shall be either a
    ** function designator or an lvalue that designates an object
    ** that is not a bit-field and is not declared with the
    ** register storage class specifier." (3.3.3.2)
    */
    if ((l->op == DOT || l->op == STREF)
	&& (SY_FLAGS(l->right->rval) & SY_ISFIELD))
	UERROR("cannot take address of bit-field: %s",SY_NAME(l->right->rval));
    else if (TR_ISLVAL(l) || TY_ISARYORFTN(l->type)){
    	if (l->op == NAME && (SY_FLAGS(l->rval) & SY_ISREG)){
	    static const char addrmesg[] =
		"cannot take address of register";	/*ERROR*/
	    if (SY_REGNO(l->rval) != SY_NOREG)
		UERROR("%s: %s", addrmesg, SY_NAME(l->rval));
	    else
		WERROR("%s: %s", addrmesg, SY_NAME(l->rval));
	}
    }
    else
        UERROR("unacceptable operand for unary &");

    if (l->op == NAME) {		/* notify optimizer that addr. taken */
#ifdef	FAT_ACOMP
	if (l->rval != ND_NOSYMBOL)
	    SY_FLAGS(l->rval) |= SY_UAND; /* note address taken */
#endif
#ifdef  OPTIM_SUPPORT
        OS_UAND(l->rval);
#endif
    }
    return (tr_generic( UNARY AND, l, ty_mkptrto(l->type) ));
}

static ND1 *
tr_star( l )
ND1 *l;
/* build a STAR node */
{
    T1WORD type;

    /* "The operand of the unary * operator shall have pointer type,
    ** other than pointer to void." (3.3.3.2)
    */
    switch( TY_TYPE(l->type) ){
    case TY_ARY:
    case TY_FUN:
	l = tr_contoptr(l);
	/*FALLTHRU*/
    case TY_PTR:
	type = TY_DECREF(l->type);
	break;
    default:
	UERROR("cannot dereference non-pointer type");
	type = l->type;
	break;
    }
    return(tr_generic(STAR, l, type));
}

static ND1 *
tr_cbranch( l, r )
ND1 *l;
ND1 *r;
/* build a CBRANCH node */
{
    ND1 * new = tr_newnode(CBRANCH);

    /* controlling expression of an selection and iteration statements
    ** shall have scalar type.
    */
    new->left = l;
    new->right = r;
    if (!TY_ISSCALAR(l->type)){
	UERROR("controlling expressions must have scalar type");
	l->type = TY_INT;	/* avoid further complaints */
    }
    new->type = l->type;
    return (new);
}



static ND1 *
tr_logic( op, l, r)
int op;
ND1 *l;
ND1 *r;
/* tr_logic builds node for logical || and && operators */
{
    ND1 * new = tr_newnode(op);

    if(TY_ISARYORFTN(l->type))
	l = tr_contoptr(l);
    if(TY_ISARYORFTN(r->type))
	r = tr_contoptr(r);

    /* Each of the operands shall have scalar type */
    if(!(TY_ISSCALAR(l->type) && TY_ISSCALAR(r->type)))
	tr_type_mess(opst[op], "scalar");

    new->left = l;
    new->right = r;
    new->type = TY_INT;
    return ( new );
}

#if 0
ND1 *
tr_init( p, t)
ND1 * p;
T1WORD t;
{
    ND1 * new = tr_newnode(INIT);

    /* If p is an array, convert to pointer to */
    if (TY_ISARYORFTN(p->type)) p = tr_contoptr(p);

    /* convert p to type t */
    p = tr_conv( p, t, "initialization type mismatch", TR_CAST );

    new->left = p;
    new->type = t;
    return (new);
}
#endif

static ND1 *
tr_assign( op, l, r)
int op;
ND1 * l;
ND1 * r;
/* tr_assign builds an simple assignment node */
{
    ND1 * new = tr_newnode(op);
    T1WORD ltype = l->type;
    T1WORD rtype = r->type;

    /* propagate a bit-field flag if left side is bit-field */
    if (l->flags & FF_ISFLD)
        new->flags |= FF_ISFLD;

    /* check to see if right operand is a known size */
    if (TY_ISSUE(rtype) && TY_SIZE(rtype) == 0)
        UERROR("unknown operand size: op \"=\"");

    /* Check to see if left side is a modifiable lvalue, if not return now. */
    if (!tr_leftside_assign(l, op)) {
	t1free(r);
	return (l);
    }

    /* convert right type to "pointer to" if necessary */
    if (TY_ISARYORFTN(rtype)){
	r = tr_contoptr(r);
	rtype = r->type;
    }

    if ( TY_EQTYPE(ltype, rtype) <= 0 ){
    /* "In the simple assignment with =, the value of the right
    ** operand is converted to the type of the left operand."
    ** (3.3.16.1)
    */
	r = tr_conv( r, ltype, "assignment type mismatch", TR_ASSIGN );
    }
    new->right = r;
    new->left = l;
    new->type = l->type;
    new->flags |= FF_SEFF;		/* operation has side effects */
    return ( new );
}

static int
tr_leftside_assign(l, op)
ND1 * l;
int op;
/* Check l and see if it is a modifiable lvalue.  If not, see if 
** because of a cast.  Certain casts are allowed on the left side
** of assigns and asgops to be compatible with UNIX 4.2 behavior.
** Choose a different message if such a cast exists.  Also modify
** message if operator is ++/--, because the user sees no "left
** operand".  (Can't check on UTYPE for this:  they're binary ops
** internally.)
*/
{
    const char *errmsg;

    /* Left hand side needs to be an lvalue for assignments. */
    if (TR_ISLVAL(l) == MODLVAL)
	return (1);
    if (l->op == CONV)
	errmsg = cast_mesg;
    else if (op == INCR || op == DECR)
	errmsg = "operand must be modifiable lvalue: op \"%s\""; /*ERROR*/
    else
	errmsg = "left operand must be modifiable lvalue: op \"%s\""; /*ERROR*/

    UERROR(errmsg, opst[op]);
    return (0);
}


static
tr_isnpc(p)
ND1 **p;
/* Returns a 1 if the node is a null pointer constant, if not
** return a 0.  "An integral constant expression with the value 0,
** or such an expression cast to type void *, is called a null pointer
** constant." (3.2.2.3)
** p is the address of the tree that we want to look at.  Make a copy
** of this tree, optimize the copy.  If it is a Null Pointer Constant
** make p the address of the optimized tree.  This allows us to change
** p yet also return a 0 or 1.
*/
{
    ND1 * p2;
    ND1 * p1 = *p;		/* we passed the address of p */
    SX sid = p1->rval;

    /* check easy stuff first: ICON = 0, member of enum = 0 */
    if (TY_NULLPTR(p1)
	|| (p1->op == NAME && SY_CLASS(sid) == SC_MOE && SY_OFFSET(sid) == 0)
	|| (p1->op == CONV && TY_EQTYPE(p1->type,TY_VOIDSTAR) > 0 && TY_NULLPTR(p1->left)))
	return (1);

    /* check tougher trees */
    p2 = tr_copy(p1);
#ifdef LINT
    { int old_walk = ln_walk;
    /* Don't do lint checks on the copy of the tree. */
    ln_walk = 0;
    p2 = op_optim(p2);
    ln_walk = old_walk;
    }
#else
    p2 = op_optim(p2);
#endif
    if (TY_NULLPTR(p2)
	|| (p2->op == CONV && TY_EQTYPE(p1->type,TY_VOIDSTAR) > 0
	    && TY_NULLPTR(p2->left)))
    {
	t1free(p1);
	**p = *p2;		/* change the tree at the address of p */
	t1free(p2);
	return (1);
    }
    else t1free(p2);
    return (0);		/* not a null pointer constant */
}

static
tr_iseom(p)
register ND1 * p;
/* return 1 if p is an enumeration type or member of enumeration */
{
    return (  p->op == NAME
	      && (TY_TYPE(p->type) == TY_ENUM 
	          || (TY_TYPE(p->type) == TY_INT && p->rval != ND_NOSYMBOL
	             && SY_CLASS(p->rval) == SC_MOE)) );
}

static
tr_islval(p)
register ND1 * p;
/* Checks the left hand side of an operator to determine
** whether it is a modifiable lvalue.  Return values:
**	NOTLVAL = not an lvalue
**	LVAL    = is an lvalue
**	MODLVAL = is a modifiable lvalue
** LINT needs to be more informative with messages about the illegal
** uses of operands who are not lvalues.
*/
{
    /* An lvalue is an expression that designates an object */

    /* A modifiable lvalue is an expression that designates an
    ** object that does not have array type and does not have a
    ** type declared with the const type specifier. (3.3.0.1)
    */

    switch( p->op ){
    case NAME:
        if (SY_CLASS(p->rval) == SC_MOE)
	    return(NOTLVAL);
	/*FALLTHRU*/
    case RNODE:
	if( TY_ISARYORFTN(p->type))
	    return(NOTLVAL);	
	break;
    case STREF:
    case STAR:
	/* These are valid lvalues. */
	break;
    case DOT:
	/* Treat cases specially that we know are not lvalues. */
	if((p->flags & FF_BADLVAL) == 0 && !tr_islval(p->left))
	    return (NOTLVAL);
	/* For special cases that are not lvalues (but which previous
	** compiler accepted), issue warning and accept expression as
	** lvalue.  Also, cancel flag to eliminate duplicate message.
	*/
	if (p->flags & FF_BADLVAL) {
	    WERROR("left operand of \".\" must be lvalue in this context");
	    p->flags &= ~FF_BADLVAL;
	}
	break;
    case CONV:
	/* CONV's are never lvalues.  However, because UNIX C used to
	** do on-the-fly optimization, some conversions were invisible
	** (like long to int), and the compiler allowed them.  Duplicate
	** the old behavior.
	*/
	if (tr_islval(p->left) == MODLVAL) {
	    T1WORD ltype = p->left->type;

	    /* Note that the CONV node and its decendant must have
	    ** the following properties:  same size, unsignedness,
	    ** both integral or pointers.
	    ** Since the tree is a modifiable lvalue, assume such a conversion
	    ** can get folded into the lvalue and just issue a warning.
	    ** Otherwise make it an error.
	    */
	    if (   TY_SIZE(p->type) == TY_SIZE(ltype)
		&& TY_ISUNSIGNED(p->type) == TY_ISUNSIGNED(ltype)
		&& (   (TY_ISINTTYPE(p->type) && TY_ISINTTYPE(ltype))
		    || (TY_ISPTR(p->type) && TY_ISPTR(ltype))
		   )
	    ) {
		WERROR(cast_mesg);
		return( MODLVAL );
	    }
	}
	return( NOTLVAL );

    default:
	/* LINT should be specific with messages about non-lvalues */
	return(NOTLVAL);
    }
	
    /* a modifiable lvalue may not be of type "const" */
    return ((TY_ISCONST(p->type) || TY_ISMBRCONST(p->type)) ? LVAL : MODLVAL);

}
    
static ND1 *
tr_cast(l, r)
ND1 * l;
ND1 * r;
/* build a CAST node */
{
    r = tr_conv( r, l->type, "invalid cast expression", TR_CAST );
    r->flags |= FF_ISCAST;
    r->flags &= ~FF_AMBFLGS;		/* cast is never ambiguous */
    l->op = FREE;
    return ( r );
}

ND1 *
tr_generic(op, node, totype)
int op;
register ND1 * node;
register T1WORD totype;
/* build a node of type totype */
{
    ND1 * new = tr_newnode(op);

    new->left = node;
    new->right = ND1NIL;
    new->type = totype;
    new->flags = node->flags & FF_AMBFLGS;
    if (op == CONV && TR_ISAMBIG(node)) {
	new->flags = tr_amb_chk(CONV, node, new);
    }
    if ((op == CONV && node->flags & FF_ISVOL)
	|| (op == STAR && TY_ISVOLATILE(totype)))
	new->flags |= FF_ISVOL;
    TPRINT(new);
    return ( new );
}

ND1 *
tr_conv( p, t, st, assignable )
ND1 * p;
T1WORD t;
char *st;
int assignable;
/* tr_conv decides if it is legal to assign or cast a symbol
** to a given type.  If it is legal, the appropriate CONV
** node is inserted at the root and the tree is returned.
** If it is illegal, the warning string is output before
** the CONV node is inserted and the tree is returned.
*/
{
    register T1WORD t1, t2;
    /* convert arrays and functions to "pointer to" */
    if (TY_ISARYORFTN(p->type)) 
        p = tr_contoptr(p);

    /* Try to assign, convert an argument, or cast t1 to t2 */
    /* We want to look at the un-qualified types */

    /* look at unqualified versions for now */
    t1 = TY_UNQUAL(p->type);
    t2 = TY_UNQUAL(t);

    switch(assignable){
    case TR_ASSIGN:
    case TR_ARG:
        /* if the types match, no need to do anything */
        if (TY_EQTYPE(t1, t2) > 0)
	     return (p);

	/* take care of enums */
	if (TY_TYPE(t2) == TY_ENUM){
	    /* complain about enums only in verbose */
	    if (verbose)
		tr_enumchk(p, t, (assignable == TR_ASSIGN) ? ASSIGN : FUNARG);
	    break;
	}
        /* Both operators may have arithmetic type */
	if (TY_ISNUMTYPE(t1) && TY_ISNUMTYPE(t2)) break;

	/* left operand may be pointer, right may be null pointer constant. */
	if (TY_ISPTR(t2) && (TR_ISNPC(p))){
	    t1 = p->type;
	    if (!TY_ISPTR(t1)) break;
	}
        /* If both are pointers, they must be pointers to compatible types. */
	if (TY_ISPTR(t1) && TY_ISPTR(t2)){
	    /* The checking of two pointers to see if one is assignable
	    ** to the other is a tricky process.  Pointers have to be
	    ** compared level by level.  tr_ptr_assignable() does this and
	    ** returns 1 if t1 can be assigned to t2.
	    */
	    if (tr_ptr_assignable(t, p->type))  break;
	    /* if we get here, we've got an illegal pointer assignment/argument */
	    WERROR("%s", st);
	    break;
	} /* end both pointers */

        /* Compatibility with UNIX C: allow illegal pointer/integer assigns */
        if (   (TY_ISPTR(t1) && TY_ISINTTYPE(t2))
            || (TY_ISPTR(t2) && TY_ISINTTYPE(t1))
	){
	    if (assignable == TR_ASSIGN)
                WERROR("improper pointer/integer combination: op \"=\"");
	    else
		WERROR("improper pointer/integer combination: arg #%d", ARGNO);
	    break;			/* break out to insert conversion to
					** proper type
					*/
        }
	UERROR("%s",st);
	p->type = t;	/* paint type and return and try to keep going */
	return(p);

    case TR_CAST:
	/* "Unless the type name specifies void type, the type name
	** shall specify scalar type and the operand shall have
	** scalar type." (3.3.4)
	** t1 = cast-expression;  t2 = (type-name)
	*/
	if (TY_TYPE(t2) == TY_VOID) break;  /* anything can be cast to void */

	if (TY_TYPE(t1) == TY_VOID){
	    UERROR("improper cast of void expression");
	    break;
	}
	else if (!TY_ISSCALAR(t1) || !TY_ISSCALAR(t2))
	    goto cast_err;

	/* types may be equivalent, but differ only by their qualified type */
	if (TY_EQTYPE(TY_UNQUAL(t1),TY_UNQUAL(t2)) > 0) break;

        /* a numerical value can be cast to another numerical type */
	if (TY_ISNUMTYPE(t1) && TY_ISNUMTYPE(t2)) break;

	/* a pointer can be converted to an integral type */
	if (TY_ISPTR(t1) && TY_ISINTTYPE(t2)) break;

	/* an arbitrary integer may be converted to a pointer */
	if (TY_ISINTTYPE(t1) && TY_ISPTR(t2)) break;

	/* cast to (void *) and any qualified version is allowed silently */
	if (TY_ISPTR(t2) && TY_EQTYPE(TY_UNQUAL(TY_DECREF(t2)),TY_VOID)) break; 

	/* "A pointer to an object of one type may be converted to
	** a pointer to an object of another type.  The resulting
	** pointer might not be valid if it is improperly aligned
	** for the type of object pointed to. It is guaranteed,
	** that a pointer to an object of a given alignment may be
	** converted to a pointer to an object of a less strict 
	** alignment and back again." (3.3.4)
	*/
#ifdef LINT
	if (! LN_FLAG('h') && TY_ISPTROBJ(t1) && TY_ISPTROBJ(t2)){
	    /* check for possible alignment problems */
	    if (TY_ALIGN(TY_DECREF(t2)) > TY_ALIGN(TY_DECREF(t1))) {
		/* pointer cast may result in improper alignment */
		BWERROR(11);	/* buffer message if ! -h */
	    }
	    break;
	}
#endif
	/* whatever pointers remain are okay */
	if (TY_ISPTR(t1) && TY_ISPTR(t2)) break;
cast_err: ;
	UERROR("%s",st);
	p->type = t;	/* paint type and return and try to keep going */
	return(p);
    default:
	cerror("illegal argument to tr_conv()"); return(p);
    }
    return (tr_generic(CONV, p, t));
}

static void
tr_enumchk(p, ltype, op)
ND1 * p;
T1WORD ltype;
int op;
/* Check the assignability of an enum type. ltype is an enum.
** Check out the right type (p->type) to see if it can be
** assigned to ltype.
*/
{
    T1WORD ptype = p->type;

    /* Determine the enum type of p, if any.  If it's an
    ** enumeration constant, get the enumeration type of
    ** type member.
    */
    if (p->op == NAME && SY_CLASS(p->rval) == SC_MOE)
	ptype = SY_TYPE(p->rval);

    /* ltype is an enum.  See if it agrees with ptype. */
    if (TY_TYPE(ptype) == TY_ENUM && !TY_EQTYPE(ptype,ltype)) {
	if (op == FUNARG) 
	    WERROR(enum_mesg_arg, ARGNO);
	else
	    WERROR(enum_mesg_op, opst[op]);
    }
}

static ND1 *
tr_promote( p )
ND1 * p;
/* tr_promote promotes types according to the appropriate rules */
{
    T1WORD oldtype = TY_TYPE(p->type);
    T1WORD newtype;

    /* Bitfields have somewhat unusual promotion rules:
    ** 1.  unsigned types, if smaller than int-size, promote ambiguously.
    ** 2.  int, long, enum, if the field is the same size as the type, and
    **		plain fields are non-negative, must be converted (unambiguously)
    **		to an unsigned equivalent.
    ** 3.  Otherwise, fields smaller than their type promote to int.
    */
    if (p->flags & FF_ISFLD) {
	ND1 * stref = p;
	SX fldsid;			/* SID of bitfield member */
	BITOFF fsize;			/* bitfield size */
	BITOFF off;			/* value unused after being set */

	newtype = TY_NONE;		/* assume no conversion needed */


	/* Search a tree from root p to find the node of op DOT or
	** STREF, take size. Search left side of tree only.
	*/
	while (stref->op != DOT && stref->op != STREF){
	    stref = stref->left;
	    if (stref == ND1NIL)
		cerror("tr_promote(): ND1NIL in tree walk");
	}
	fldsid = stref->right->rval;
	SY_FLDUPACK(fldsid, fsize, off);	/* off not used */

	switch (oldtype) {
	case TY_CHAR:
	case TY_SCHAR:
	case TY_SHORT:
	case TY_SSHORT:
	    newtype = TY_INT;		/* always promote to int */
	    break;
	case TY_UCHAR:
	case TY_USHORT:
	case TY_UINT:
	    if (fsize < TY_SIZE(TY_INT)){
		/* if the expression has seen a cast, no need to warn
		** about ambiguity, because there is no ambiguity.
		*/
		if ((p->flags & FF_ISCAST) == 0)
		    p->flags |= FF_ISAMB; 
		if (version & V_CI4_1) {
		    if (oldtype != TY_UINT)
			newtype = TY_UINT;
		}
		else
		    newtype = TY_INT;
	    }
	    /* else:  type and size must be TY_UINT.  No change required. */
	    break;
#ifndef SIGNEDFIELDS
	    /* On machines where plain bitfields are non-negative, if
	    ** the bitfield type is int, long, or enum, and the bitfield
	    ** size is the size of the type, the bitfield must be promoted
	    ** to the corresponding unsigned type, according to ANSI C's
	    ** rules.
	    ** Note that this conversion does not imply a semantic
	    ** ambiguity by itself.
	    */
	case TY_INT:
	case TY_LONG:
	case TY_ENUM:
	    if (fsize == TY_SIZE(SY_TYPE(fldsid)))
		newtype = (oldtype == TY_LONG) ? TY_ULONG : TY_UINT;
	    break;
#endif
	}
	/* Apply conversion to new type if needed. */
	if (newtype != TY_NONE)
	    p = tr_generic(CONV, p, newtype);
	return(p);
    }

    /* ANSI C integral promotion behavior: A char, a short, or an
    ** int bit-field, or their signed or unsigned varieties, may be
    ** used in an expression wherever an int may be used.  In all
    ** cases the value is converted to an int if an int can represent
    ** all values of the original type; otherwise it is converted
    ** to an unsigned int. (3.2.1.1) Value Preserving Rules.
    ** UNIX C goes by Unsigned Preserving Rules.
    */
    switch (oldtype){
    case TY_CHAR: case TY_SCHAR:
    case TY_SHORT: case TY_SSHORT:
	newtype = TY_INT;
	break;
    case TY_UCHAR:
    case TY_USHORT:
	if (TY_SIZE(oldtype) < TY_SIZE(TY_INT)){
	    p->flags |= FF_ISAMB;
	    if (version & V_CI4_1)
		newtype = TY_UINT;
	    else newtype = TY_INT;
	}
	else newtype = TY_UINT;
	break;
    default:
	return (p);
    }
    p = tr_generic(CONV, p, newtype);
    return (p);
}

static void
tr_arconv( pl, pr )
ND1 **pl;
ND1 **pr;
/* This routine performs arithmetic conversions.  Since UNIX C
** and ANSI C behave differently in this area, both rules are
** implemented then executed according to whether or not the
** ANSI conforming option was selected.  Areas in which UNIX C
** differ from ANSI C should be warned about. These areas deal
** with the added types and the value preserving rules.
** pl and pr point, respectively, to pointers to the left and
** right operands.
*/
{
    register T1WORD tl = (*pl)->type;
    register T1WORD tr = (*pr)->type;
    register T1WORD ltype = TY_TYPE(tl);
    register T1WORD rtype = TY_TYPE(tr);
    int amb = 0;	/* FF_ISAMB if ambiguous */
	
    /* This section performs arithmetic conversions according to
    ** the ANSI standard and UNIX C:
    **
    **	First, if either operand has type long double, the other
    **	operand is converted to long double.
    **
    **	Otherwise, if either operand has type double, the other
    **	operand has type double.
    **
    **	Otherwise, if either operand has type float, the other
    **	operand has type float.
    **
    **	Otherwise, if either operand has type unsigned long int,
    **	the other operand is converted to unsigned long int.
    **
    **  Otherwise, if either operand is long and the other is
    **  unsigned int, convert according to the version run.
    **
    **	Otherwise, if either operand has type long int, the
    **	other operand is converted to long int.
    **
    **	Otherwise, the integral promotions are performed.  Then
    **	if either operand has type unsigned int, the other operand
    **	is converted to unsigned int.
    **
    **	Otherwise, both operands have type int.
    **  (3.2.1.5)
    */
    if ( (ltype == TY_LDOUBLE) || (rtype == TY_LDOUBLE) )
	tl = tr = TY_LDOUBLE;
    else
    if ( (ltype == TY_DOUBLE) || (rtype == TY_DOUBLE) )
	tl = tr = TY_DOUBLE;
    else
    if ( (ltype == TY_FLOAT) || (rtype == TY_FLOAT) )
	tl = tr = TY_FLOAT;
    else
    if ( (ltype == TY_ULONG) || (rtype == TY_ULONG) )
	tl = tr = TY_ULONG;
    else
    if ((ltype == TY_LONG && rtype == TY_UINT)
	|| (ltype == TY_UINT && rtype == TY_LONG))
    {
	if (version & V_CI4_1) tl = tr = TY_ULONG;  /* unsigned preserving */
	else {	/* ANSI C value preserving */
	    if (TY_SIZE(TY_LONG) == TY_SIZE(TY_INT))
		tl = tr = TY_ULONG;
	    else { /* long > int */
		tl = tr = TY_LONG;
		amb = FF_ISAMB;
	    }
	}
    }
    else
    if ( (ltype == TY_LONG) || (rtype == TY_LONG) )
	tl = tr = TY_LONG;
    else
    if ( (ltype == TY_UINT) || (rtype == TY_UINT ))
	tl = tr = TY_UINT;

    /* if tl and tr have changed since the beginning of the routine,
    ** conv to new type.
    */
    if ( TY_TYPE(tl) != ltype ) 
	*pl = tr_generic(CONV, *pl, tl);
    if ( TY_TYPE(tr) != rtype ) 
	*pr = tr_generic(CONV, *pr, tr);
    (*pl)->flags |= amb;
    (*pr)->flags |= amb;
    return;
}


ND1 *
tr_return(p, t)
ND1 * p;
T1WORD t;
/* Build tree to return value of expression p from function
** with return type t.  Return the resulting tree.
*/
{
    /* Check whether the return value, if any, matches the
    ** function type.  Build suitable assign to RNODE.
    ** The result first gets promoted to the declared type
    ** of the function, then to the effective return type
    ** (which callers expect).
    */
    if (p != ND1NIL) {
        ND1 * rnode;

	/* A return statement with an expression shall not appear
	** in a function whose return type is void.
	*/
	if (TY_ERETTYPE(t) == TY_VOID){
	    UERROR("void function cannot return value");
	    return (p);
	}
        p = tr_conv(p, TY_DECREF(t), "return value type mismatch", TR_ASSIGN);
        p = tr_conv(p, TY_ERETTYPE(t), "sm_return??", TR_ASSIGN);
        rnode = tr_newnode(RNODE);
        rnode->type = TY_UNQUAL(p->type);

        p = tr_build(ASSIGN, rnode, p);
    }
    return( p );
} 

static ND1 *
tr_comma(op, l, r)
int op;
ND1 * l;
ND1 * r;
/* build a comma operator node, and CM delimiter for args */
{
    ND1 * new = tr_newnode(op);
    
    if (op == CM)
	new->type = TY_INT;		/* CM is always int */
    else { 				/* COMOP */
	/* ,OP has side effects if either branch has them. */
	new->flags |= (l->flags|r->flags) & FF_SEFF;
	/* Right side array/function must be converted to pointer.
	** (This is incompatibility with UNIX C.)
	*/
	if (TY_ISARYORFTN(r->type))
	    r = tr_contoptr(r);
	new->type = r->type;
    }
    new->left = l;
    new->right = r;
    return (new);
}


ND1 *
tr_copy(p)
ND1 * p;
/* Make a copy of tree p. */
{
    ND1 * newp;
    ND1 * newl = p->left;
    ND1 * newr = p->right;

    switch( optype(p->op) ) {
    case BITYPE:
        newr = tr_copy(p->right);	/*FALLTHRU*/
    case UTYPE:
        newl = tr_copy(p->left);	/*FALLTHRU*/
    case LTYPE:
        newp = t1alloc();
        *newp = *p;
        newp->left = newl;
        newp->right = newr;
    }
    return( newp );
}


static ND1 *
tr_contoptr(p)
ND1 * p;
/* convert "array of types" to "pointer to type" with U& parent */
{
    if ( TY_ISFTN(p->type) )
        p = tr_generic( UNARY AND, p, ty_mkptrto(p->type));
    else {
	/* Special check where operand has a "." operator and
	** it produces a non-lvalue.  Strictly speaking it's
	** invalid to convert the array to a pointer:
	**	stf.ia[3]
	** Use TR_ISLVAL to issue diagnostic.
	*/
	if (p->op == DOT && !TR_ISLVAL(p))
	    cerror("tr_contotptr():  . not lvalue");

	/* have diagnose implicit address taken of a register */
	if (p->op == NAME && (SY_FLAGS(p->rval) & SY_ISREG)) {
	    WERROR("cannot take address of register: %s", SY_NAME(p->rval));
	    SY_FLAGS(p->rval) |= SY_UAND;
	}
	p = tr_generic( UNARY AND, p, ty_mkptrto(TY_DECREF(p->type)));
    }
    p->flags |= (FF_MASK & p->left->flags);
    return (p);
}


CONVAL
tr_truncate( v, t )
CONVAL v;
T1WORD t;
/* Truncate value v into simple type t.  This has to be done
** as if on the target machine.
*/
{
    switch( TY_TYPE(t) ){
    case TY_SCHAR:
	if ( v & ONEBIT(TY_SIZE(TY_CHAR)-1) )
	    return ( v | ~BITMASK(TY_SIZE(TY_CHAR)) );
	/*FALLTHRU*/
    case TY_CHAR:
#ifdef C_CHSIGN 
	if ( v & ONEBIT(TY_SIZE(TY_CHAR)-1) )
	    return ( v | ~BITMASK(TY_SIZE(TY_CHAR)) );
	/*FALLTHRU*/
#endif
    case TY_UCHAR:
	return ( v & BITMASK(TY_SIZE(TY_CHAR)) );
    case TY_SHORT: case TY_SSHORT:
	if ( v & ONEBIT(TY_SIZE(TY_SHORT)-1) )
	    return ( v | ~BITMASK(TY_SIZE(TY_SHORT)) );
	/*FALLTHRU*/
    case TY_USHORT:
	return ( v & BITMASK(TY_SIZE(TY_SHORT)) );
    case TY_INT: case TY_SINT:
	if ( v & ONEBIT(TY_SIZE(TY_INT)-1) )
	    return ( v | ~BITMASK(TY_SIZE(TY_INT)) );
	/*FALLTHRU*/
    case TY_UINT:
	return ( v & BITMASK(TY_SIZE(TY_INT)) );
    default:
	return (v);
    }
}

static int
tr_ptr_assignable(ltype, rtype)
T1WORD ltype;
T1WORD rtype;
/* Determine if pointer rtype may be assigned to pointer ltype. */
{
    T1WORD rqual;

    /* Examine things pointed to. */
    ltype = TY_DECREF(ltype);
    rtype = TY_DECREF(rtype);
    rqual = TY_GETQUAL(rtype);

    /* Left side must be at least as qualified as right. */
    if ((TY_GETQUAL(ltype) & rqual) != rqual)
	return( 0 );		/* qualifiers are bad */


    if (TY_EQTYPE(TY_UNQUAL(ltype), TY_UNQUAL(rtype)) > 0)
	return( 1 );			/* point to equivalent types */

    /* Okay if either is void, as long as other side is not
    ** function type.
    */
    ltype = TY_TYPE(ltype);
    rtype = TY_TYPE(rtype);
    if (   (ltype == TY_VOID && rtype != TY_FUN)
	|| (rtype == TY_VOID && ltype != TY_FUN)
	)
	return( 1 );
	
    return( 0 );			/* pointers not assignable */
}

static void
tr_type_mess(s1, s2)
char *s1;
char *s2;
/* Print out error message common to many semantic checking routines */
{
    UERROR( "operands must have %s type: op \"%s\"", s2, s1 );
}

#ifndef NODBG
/* Debugging expression treee printing routines */

void
tr_eprint(p)
ND1 * p;
{
    DPRINTF("\n++++++++\n");
    tr_e1print(p, "T");
    DPRINTF("---------\n");
}

void
tr_e1print(p, s)
register ND1 *p;
char *s;
{
    register ty, d;
    static down = 0;
 
    ty = optype( p->op );

    if( ty == BITYPE )
    {
        ++down;
        tr_e1print( p->right ,"R");
        --down;
    }

    for( d=down; d>1; d -= 2 ) DPRINTF("\t");
    if( d ) DPRINTF("    ");

    DPRINTF("%s=%d) %s, ", s, (int) node_no(p), opst[p->op]);
    if( ty == LTYPE )
    {
        DPRINTF("lval=%ld", p->lval );
        DPRINTF( ", rval=%d, ", p->rval );
    }
    if (p->op == FLD)
         DPRINTF("foff=%d, fsiz=%d, ", UPKFOFF(p->rval), UPKFSZ(p->rval));
    DPRINTF("type=");
    ty_print( p->type );
    DPRINTF(", flags=%o",p->flags); 
    DPRINTF("\n");
    if( ty != LTYPE )
    {
         ++down;
         tr_e1print( p->left ,"L");
         --down;
    }
}

static void
tr_b1print(new)
ND1 * new;
{
        DPRINTF("node %d: op=%s, type=",node_no(new),opst[new->op]);
        ty_print(new->type);
        DPRINTF(", lval=%d\n",new->lval);
}

#endif
