/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/init.c	55.1"
/* init.c */

#include "p1.h"
#include <stdio.h>			/* for debug output */

/* This module contains the code to manage initializations,
** both static and run-time.  The following routines participate:
**	in_decl		establishes the symbol ID of the object to init
**	in_start	set up the initialization stack
**	in_end		end of initialization -- clean-up
**	in_init		called with next initializer
**	in_lc		called when a { is seen in an initializer
**	in_rc		called when a } is seen in an initializer
**			({} mismatches produce syntax errors)
**
** These routines are used internally:
**	in_first	figures out what to do with the very first
**			initializer value:  for auto's, discriminates between
**			initialization with an expression
**				f(){ struct s { int x; } sf, v=sf(); }
**			vs. initialization with static initializer
**				f(){ struct s v = { 1 };
**	in_nextelem	find the next place for a scalar initializer,
**			based on the current state of initialization
**	in_endelem	figure out how to unwind the current initialization
**			state after having seen a scalar initializer
**	in_push		push new initialization element onto the stack
**	in_pop		pop element from initialization stack
*/

/* This structure controls the recursive traversal of nested
** aggregates during initialization.
*/

struct instk 
{
    SIZE in_celem;	/* current number of elements so far */
    SIZE in_nelem;	/* number of elements available at current level */
    T1WORD in_type;	/* type for current aggregate element */
    int in_lcflag;	/* number of {'s at open at this level */
    char in_isfield;	/* non-zero if element is part of bit-field */
    char in_hadstring;	/* non-zero if quoted string was used as initializer
			** for current element (which is char array)
			*/
    BITOFF in_off;	/* offset at beginning of this level */
    BITOFF in_size;	/* size of item at this level */
};

#ifndef	INI_INSTK
#define	INI_INSTK 10
#endif

TABLE(Static, instk, static, instk_init,
		struct instk, INI_INSTK, 0, "init stack");
#define	ty_instk struct instk

#define	II(i) (TD_ELEM(instk, ty_instk, (i)))


static SX in_symbol;		/* identifier being initialized */
static int in_isfirst;		/* non-0 for first initializer */
static BITOFF in_curoff;	/* current initialization offset */
/* There are several different kinds of initialization handled here:
**
**	IN_STATIC	static/extern with static initializer
**	IN_AUTOSU	struct/union automatic with static initializer
**	IN_AUTOSTRING	char array automatic with string
**	IN_AUTOARRAY	other array automatic with static initializer
**	IN_AUTOEXPR	any automatic with run-time expression
*/
static int in_kind;		/* kind of initialization: */
#define	IN_STATIC	1	/* static data */
#define	IN_AUTOSU	2	/* struct/union auto init. */
#define	IN_AUTOSTRING	3	/* auto char array by string */
#define	IN_AUTOARRAY	4	/* init. of auto array */
#define	IN_AUTOEXPR	5	/* init. of auto with expression */
static ND1 * in_statlab;	/* node that corresponds to label for
				** static copy of initializer for automatic
				** aggregate
				*/

#define in_sp (TD_USED(instk)) 	/* initialization stack pointer */
#define	IN_EMPTY (~((SIZE) 0))	/* empty initialization stack */

static void in_push();
static void in_pop();
static void in_nextelem();
static void in_endelem();
static int in_usestr();
#ifndef	NODBG
static void in_print();
#endif


void
in_decl(sid)
SX sid;
/* Remember symbol index of identifier to be initialized.  Called
** from declaration processing when an init-declarator (sid) is seen.
*/
{
    in_symbol = sid;

    DEBUG(i1debug, ("in_decl(%d)\n", (int) sid));
    return;
}


void
in_start()
/* Mark start of initialization.  At this point we know the identifier
** being initialized (in_symbol), but we don't know whether there is a
** {} initializer list.
*/
{
    T1WORD t;

    DEBUG(i1debug, ("in_start()\n"));
#ifdef LINT
    ln_setgoal(VAL);
#endif
    in_curoff = 0;		/* starting bit offset for initialization */
    in_sp = IN_EMPTY;		/* initialize initialization stack */
    in_isfirst = 1;		/* next initializer is first */

    /* Various errors may lead to there being no symbol we're
    ** supposed to initialize.  Fake it.
    */
    if (in_symbol != SY_NOSYM)
	t = SY_TYPE(in_symbol);
    else
	t = TY_INT;
    in_push(t, (BITOFF) 0, TY_SIZE(t), 0);
    in_kind = IN_STATIC;		/* assume usual case */
    return;
}


void
in_end()
/* This routine is called when the end of an initializer is
** reached.
*/
{
    T1WORD t;
    BITOFF newoff;
    int dodebug = 0;			/* set to do debug output */

    DEBUG(i1debug, ("in_end()\n"));
#ifdef LINT
    ln_setgoal(EFF);
#endif

    /* Should return to top level if initializer began with
    ** {.
    */
    if (in_sp != 0 && II(0).in_lcflag != 0)
	cerror("confused in_end(), level %d", in_sp);

    /* If the top-level type is an array with no fixed size yet,
    ** change the type to one with a size.
    */
    t = II(0).in_type;
    if (TY_ISARY(t) && II(0).in_nelem == TY_NULLDIM) {
	t = ty_mkaryof(TY_DECREF(t), II(0).in_celem);
	II(0).in_type = t;
	SY_TYPE(in_symbol) = t;
	/* If we're initializing an auto array, allocate space for
	** the array now.
	*/
	switch( in_kind ){
	case IN_AUTOARRAY:
	case IN_AUTOSTRING:
	    al_auto(in_symbol);
	}
	/* We should do debug output for symbol, now that we know its
	** size.  Defer doing so until we've finished initialization,
	** because db_output changes location counters sometimes.
	*/
	dodebug = 1;
    }

    /* Calculate final offset expected.  Supply whatever padding is needed. */
    newoff = II(0).in_off + TY_SIZE(II(0).in_type);
#ifndef	NODBG
    if (i1debug)
	in_print();
    DEBUG(i1debug, ("in_end():  start offset %d, final offset %d\n",
		in_curoff, newoff));
#endif

    /* Clean up depends on what's being initialized */
    switch( in_kind ) {
    case IN_STATIC:
	if (newoff > in_curoff)
	    cg_zecode(newoff - in_curoff);
	cg_inend();
	break;
    case IN_AUTOEXPR:
	/* Initialization of automatics with a run-time expression
	** has already been handled like assignment.  Nothing to do
	** here.
	*/
	break;
    case IN_AUTOSU:
    case IN_AUTOARRAY:
    case IN_AUTOSTRING:
	/* Initialization of auto struct/union or array.  Generate
	** the block move code.  First, fill out static edition of
	** the aggregate, then copy the whole thing.
	** In the future, consider creating and using a block-zero
	** primitive, copy the shortened static initializer and
	** zero the rest.
	*/
	if (newoff > in_curoff)
	    cg_zecode(newoff - in_curoff);
	cg_inend();
	cg_bmove(in_symbol, in_statlab);
	break;
    default:
	cerror("can't handle case %d in in_end()", in_kind);
    }
    /* Produce debug information if we know a size now. */
    if (dodebug) {
	SY_FLAGS(in_symbol) &= (SY_FLAGS_t) ~SY_DBOUT;
	DB_SYMBOL(in_symbol);
    }

    cg_nameinfo(in_symbol);		/* emit type/size info for symbol */

    in_symbol = SY_NOSYM;		/* no identifier of interest */
    return;
}


void
in_lc()
/* Saw { during initialization. */
{
    DEBUG(i1debug > 1, ("in_lc(), enter level %d\n", in_sp));

#ifdef LINT
    ln_setgoal(VAL);
#endif
    /* The first { at top level is a special case that just marks
    ** the top element of the stack.
    */
    if (!(in_sp == 0 && II(0).in_lcflag == 0))
	in_nextelem(1);			/* descend one level */
    II(in_sp).in_lcflag++;
    DEBUG(i1debug > 1, ("in_lc(), exit  level %d\n", in_sp));
    return;
}


void
in_rc()
/* Saw } during initialization. */
{
    DEBUG(i1debug > 1, ("in_rc(), enter level %d\n", in_sp));

#ifdef LINT
    ln_setgoal(EFF);
#endif
    /* Back up until we find an unmatched explicit { */
    for (;;) {
	if (II(in_sp).in_lcflag != 0) {
	    --II(in_sp).in_lcflag;
	    in_endelem();		/* done one element */
	    break;
	}
	if (in_sp == 0) break;		/* don't back up beyond beginning */
	in_pop();
	II(in_sp).in_celem++;		/* count one more at the level reached */
    }
    DEBUG(i1debug > 1, ("in_rc(), exit  level %d\n", in_sp));
    return;
}


static int
in_usestr(p, t)
ND1 * p;
T1WORD t;
/* Return 1 if p is a STRING initializer of an appropriate type for
** t.  If p is a wide string, t must be T_wchar_t.  Otherwise t must
** be a character type.
*/
{
    if (p->op == STRING) {
	switch( t = TY_UNQUAL(t) ){
	case TY_CHAR: case TY_UCHAR: case TY_SCHAR:
	    return( p->rval == 0 );	/* must be "narrow" string */
	default:
	    if (TY_EQTYPE(t, T_wchar_t) > 0)
		return( p->rval == 1 );	/* must be "wide" string */
	    break;
	}
    }
    return( 0 );
}


static ND1 *
in_first(p)
ND1 * p;
/* Do special processing for first initializer:
**	Are we initializing an AUTO or register?
**		if so, is it a struct/union, and is there a {}?
**			if yes, create dummy label, return NAME node
**				caller will do static init,
**				in_end() will do assign
**		if the left side is array of char and right side is string,
**			create dummy string, then do block assign
**		for autos, do assign
**	otherwise, do static initialization
**	Watch out for scalars in {} that initialize autos
**	(probably already caught).
**
** Warn about initialization of aggregates that lacks necessary {}.
** The return value is the initializer tree, possibly modified.  If
** the returned tree is ND1NIL, there's no further initialization
** to be done.
*/
{
    T1WORD t = SY_TYPE(in_symbol);
    T1WORD ttype = TY_TYPE(t);
    int class = SY_CLASS(in_symbol);
    int usestring = 0;			/* 1 if STRING is used to initialize
					** an array
					*/

    /* Warn about inapproriate initialization:
    **		struct/union init-declarator, scalar expression
    **		initialization of non-char array with scalar
    **
    ** Also, set usestring.
    */
    switch( ttype ){
    case TY_STRUCT:
    case TY_UNION:
	switch( class ) {
	default:
	    goto warn;			/* extern/static (particularly) */
	case SC_AUTO:
	case SC_REGISTER:
	    if (TY_ISSCALAR(p->type))
		goto warn;		/* have a problem */
	}
	break;
    case TY_ARY:
	/* If can use string in character array, set flag, avoid warning. */
	if ((usestring = in_usestr(p, TY_DECREF(t))) != 0)
	    break;
warn:;
	    if (II(0).in_lcflag == 0)
		WERROR("{}-enclosed initializer required");
	break;
    }

    switch( class ) {
    case SC_AUTO:
    case SC_REGISTER:
	if (   II(0).in_lcflag != 0
	    && (ttype == TY_STRUCT || ttype == TY_UNION)
	    ) {
	    cg_instart(t, C_READONLY);
	    in_statlab = cg_defstat(t);
	    in_kind = IN_AUTOSU;
	    /* leaving p alone causes in_init to build static initializer */
	}
	else if (ttype == TY_ARY) {
	    /* All of these cases need a label for a static initializer. */
	    cg_instart(t, C_READONLY);
	    in_statlab = cg_defstat(t);

	    /* Kind of initialization depends on whether we're using a
	    ** string to initialize the array.
	    */
	    in_kind = usestring ? IN_AUTOSTRING : IN_AUTOARRAY;

	    /* leaving p alone causes in_init to build static initializer */
	}
	else {
	    /* For other cases (initialization of an automatic scalar or
	    ** aggregate by an expression), build run-time assign tree,
	    ** make a statement.  Remove const-ness so tree building at
	    ** top level is happy.
	    */
	    ND1 * s = tr_symbol(in_symbol);
	    T1WORD stype = s->type = TY_UNCONST(s->type);
	    
	    /* Catch instances of non-struct/union initializer for
	    ** auto struct/union, give special message.  ("assignment
	    ** type mismatch" is confusing.)
	    */
	    if (TY_ISSU(stype) && !TY_ISSU(p->type)) {
		UERROR("struct/union-valued initializer required");
		t1free(s);
		t1free(p);
	    }
	    else {
		/* Check for initializers that won't fit for integral
		** types.  If the right side is a FP type, there's not
		** much we can do, unless it's an FCON.  We can't fold
		** the expression (op_init()) because we don't know the
		** run-time rounding modes.
		** To check that the integer initializer fits, we need to
		** optimize BEFORE any casts are applied for assignment.
		*/
		if (TY_ISINTTYPE(stype) && TY_ISNUMTYPE(p->type)) {
		    static const char mesg[] =
			"initializer does not fit"; /*ERROR*/
		    p = op_optim(p);
		    if (p->op == FCON) {
			/* Try to check whether the value will fit.  If
			** there's a very large FCON that overflows when
			** converting to an integral type, we may get two
			** messages.  (One in this block, one at the
			** sm_expr() call below.)
			*/
			ND1 * new = tr_newnode(FCON);

			*new = *p;

			p->op = CONV;
			p->left = new;
			p->flags = 0;
			p->type = TY_ISUNSIGNED(stype) ? TY_ULONG : TY_LONG;
			/* Pretend we have an initializer to fold expression.
			** Treat like expression in executable code.
			*/
			p = op_optim(p);
		    }
		    if (p->op == ICON && p->rval == ND_NOSYMBOL)
			in_chkval(p, stype, (int) TY_SIZE(stype), 0, mesg);
		}
#ifdef LINT
    		ln_setgoal(EFF);
#endif
		sm_expr(tr_build(ASSIGN, s, p));
		/* Record the fact that we've seen one instance of
		** the top level item.
		*/
		II(0).in_celem = 1;
	    }

	    in_kind = IN_AUTOEXPR;
	    p = ND1NIL;			/* flag initialization as complete */
	}
	break;
    case SC_STATIC:
    case SC_EXTERN:
	cg_instart(t, C_READWRITE);	/* choose appropriate section */
	cg_defnam(in_symbol);
	break;
    }
    return( p );
}



void
in_init(p)
ND1 * p;
/* Saw initializer tree p. */
{
    BITOFF newoff;			/* offset for new initializer */
    T1WORD t;
    int isfield;
    static const char mesg[] =
	"string literal must be sole array initializer"; /*ERROR*/

    DEBUG(i1debug, ("in_init()\n"));

    if (p == ND1NIL)
	cerror("in_init() called with NIL");

    /* First initializer is sometimes special. */
    if (in_isfirst) {
	p = in_first(p);
	in_isfirst = 0;
	if (p == ND1NIL) return;	/* nothing further to do */
    }

    in_nextelem(0);			/* go as many levels as necessary */
    newoff = II(in_sp).in_off;
    t = II(in_sp).in_type;
    isfield = II(in_sp).in_isfield;

#ifndef	NODBG
    if (i1debug) {
	in_print();
	fprintf(stderr, "--->old off %lu, new off %lu, element type is %lu, ",
		in_curoff, newoff, (unsigned long) t);
	ty_print(t);
	putc('\n', stderr);
    }
#endif

    if (newoff > in_curoff)
	cg_zecode(newoff - in_curoff);

    /* Check whether this is an additional initializer after a
    ** STRING:
    **		char ca[] = { "abc", 1 };
    */
    if (in_sp > 0 && II(in_sp-1).in_hadstring)
	WERROR(mesg);

    /* If we're using a STRING in a context where it initializes an array,
    ** generate the string in-line.
    */
    if (in_usestr(p, t) && in_sp > 0 && TY_ISARY(II(in_sp-1).in_type)
    ) {
	SIZE asize = II(in_sp-1).in_nelem;	/* size of array */
	SIZE stsize = p->lval;			/* size of initializing string */

	/* Check whether we've already started initializing array.
	** UNIX C allowed such a thing!
	**	char ca[] = { 1, "abc" };
	*/
	if (II(in_sp-1).in_celem != 0)
	    WERROR(mesg);

	/* Remember we had a string initializer for the array. */
	II(in_sp-1).in_hadstring = 1;

	/* Check whether string has more bytes than remain. */
	if (asize != TY_NULLDIM) {
	    /* Adjust size by number of elements already used.
	    ** (This is an error condition kept for compatibility.)
	    */
	    SIZE curelem = II(in_sp-1).in_celem; /* NOTE:  unsigned! */
	    if (asize > curelem)
		asize -= curelem;
	    else
		asize = 0;		/* no room left */

	    if (stsize > asize) {
		WERROR("%d extra byte(s) in string literal initializer ignored",
			stsize-asize);
		stsize = asize;
	    }
	}

	/* Generate NUL explicitly if there's room for it. */
	if (asize > stsize)
	    ++stsize;

	if (stsize != 0)
	    /* read-only doesn't matter to cg_strinit here */
	    (void) cg_strinit(p, stsize, 0, C_READWRITE);
	else
	    t1free(p);

	/* Update current offset and number of elements. */
	in_pop();			/* Discard scalar-type entry. */
	in_curoff = newoff + stsize * TY_SIZE(t);

	/* Bump current element number by number used.
	** This has the defect of allowing multiple initializers,
	** but that's needed for UNIX C compatibility anyway.
	*/
	II(in_sp).in_celem += stsize;

    }
    else {
	/* Convert initializer to desired type, optimize to a constant.
	** Check that constant fits in the space provided.
	** Anything else is an error.
	*/
	if (TY_ISINTTYPE(t) && TY_ISNUMTYPE(p->type)) {
	    if (TY_ISFPTYPE(p->type))
		p = tr_conv(p, (T1WORD) (TY_ISUNSIGNED(t) ? TY_ULONG : TY_LONG),
			"can't convert??", TR_CAST);
	    p = op_init(p);
	    if (p->op == ICON && p->rval == ND_NOSYMBOL)
		in_chkval(p, t, (int) II(in_sp).in_size, isfield,
			"initializer does not fit");	/*ERROR*/
	}
	else {
	    p = tr_conv(p, t, "initialization type mismatch", TR_ASSIGN);
	    p = op_init(p);
	}
	switch( p->op ) {
	case ICON:
	case FCON:
	case STRING:			/* becomes ICON later in cg_ecode() */
	    cg_incode(p, II(in_sp).in_size);
	    break;
	default:
	    UERROR("non-constant initializer: op \"%s\"", opst[p->op]);
	}

	/* Remember offset at end of current initializer. */
	in_curoff = newoff + II(in_sp).in_size;
	/* Done scalar.  Count off one more element. */
	II(in_sp).in_celem++;
    }
    in_endelem();
    return;
}


void
in_chkval(p, totype, len, isfield, mesg)
ND1 * p;
T1WORD totype;
int len;
int isfield;
char * mesg;
/* Check whether an initializer fits in the space available (len bits).
** p points to an ICON node.  totype is the desired type
** of the result.  Mimic the behavior of the value in the context
** of an expression, based on its type.  isfield is non-zero if
** the initializer is for a bitfield.  Warn, using "mesg", if the
** initializer does not fit.
** This code assumes the host and target are 2's complement machines!!
*/
{
    CONVAL val = p->lval;
    int signedcheck;

    if (val == 0)
	return;				/* fits by definition */
    /* Unsigned bitfields can use all the bits, as can plain bitfields
    ** on machines that don't sign extend them.
    ** Plain characters can use all bits if they are treated as
    ** unsigned.
    */
    signedcheck = TY_ISSIGNED(totype);
#ifndef	SIGNEDFIELDS
    if (isfield && signedcheck < 0)	/* plain field */
	signedcheck = 0;
#endif
#ifndef	C_CHSIGN
    if (!isfield && TY_EQTYPE(totype, TY_CHAR) > 0) /* plain char */
	signedcheck = 0;
#endif
    if (signedcheck != 0) {
	/* t is signed, one way or another.  Treat positive and
	** negative values differently.  Treat in-coming unsigned
	** as positive.
	*/
	if (val > 0 || TY_ISUNSIGNED(p->type)) {
	    if (((~0L << len-1) & val) == 0)
		return;			/* positive value okay */
	}
	else {
	    UCONVAL mask = (len <= 1) ? ~0L : ~0L << (len-1);
	    /* 2's complement-dependent */
	    if ((val & mask) == mask)	/* all high bits are 1 -- ok */
		return;
	}
    }
    else {				/* unsigned check */
	if (len == TY_SIZE(TY_LONG)) {
	    /* Unfortunately we can't tell here whether the value
	    ** had an explicit sign or not.  If it did, there is
	    ** an error.  If it didn't there's no error.  In -Xt
	    ** the type stays int (long).  In ANSI mode, a positive
	    ** value becomes unsigned.  Keep -Xt quiet.
	    */
#ifndef LINT
	    if (val > 0 || TY_ISUNSIGNED(p->type) || (version & V_CI4_1))
#else
	    if (val > 0 || TY_ISUNSIGNED(p->type) )
#endif
		return;			/* allow any positive value */
	}
	else if (((~0L << len) & val) == 0)
	    return;			/* value fits */
    }
    /* Reach here on failure. */
    if (TY_ISUNSIGNED(p->type))		/* incoming unsigned */
	WERROR("%s: %#x", mesg, (UCONVAL) val);
    else
	WERROR("%s: %ld", mesg, val);
    return;
}

static void
in_push(t, off, size, isfield)
T1WORD t;
BITOFF off;
BITOFF size;
char isfield;
/* Start new level with type t, offset off, and size size.
** Strip off const/volatile-ness of the objects being
** initialized.  If isfield is set, element is part of bitfield.
*/
{
    SIZE nelem;				/* Presumed number of elements. */

    if (in_sp == IN_EMPTY)		/* initial condition */
	in_sp = 0;
    else {
	/* in_sp is the current entry, which is one less than the number
	** used.  (And in_sp is TD_USED(instk).)
	*/
	++in_sp;			/* bump current stack pointer */
	TD_NEED1(instk);		/* make sure there's space */
	TD_CHKMAX(instk);		/* keep statistics */
    }

    DEBUG(i1debug > 1, ("in_push() to level %d\n", in_sp));

    /* Determine the effective and generic types. */

    II(in_sp).in_celem = 0;		/* no elements now */
    II(in_sp).in_type = t;		/* type */
    II(in_sp).in_lcflag = 0;		/* no { so far */
    II(in_sp).in_off = off;		/* remember starting offset */
    II(in_sp).in_size = size;		/* size of item */
    II(in_sp).in_isfield = isfield;
    II(in_sp).in_hadstring = 0;		/* no quoted string initializer */

    /* Figure out how many elements we've got.  For scalars, the number
    ** is 1, as it is for top level of all unions.  For structs, the
    ** number is 1 if it is incomplete, else the correct number.  For
    ** arrays, the number is what it is, including TY_NULLDIM, which
    ** means unknown.
    */

    nelem = 1;

    switch( TY_TYPE(t) ){
    case TY_STRUCT:
	if (! TY_HASLIST(t))
	    break;
	/*FALLTHRU*/
    case TY_ARY:
	nelem = TY_NELEM(t);
	if (nelem == TY_ERRDIM)		/* make error look like 0 */
	    nelem = 0;
	break;
    }

    II(in_sp).in_nelem = nelem;
    DEBUG(i1debug > 1, ("in_push() to in_sp %d, type %d, nelem %d\n",
			in_sp, t, II(in_sp).in_nelem));
    return;
}


static void
in_pop()
/* Exit one level of initialization. */
{
    DEBUG(i1debug > 1, ("in_pop(), enter at %d\n", in_sp));
    if (in_sp > 0)
	--in_sp;
    /* Shouldn't get called for in_sp == 0, either. */
    else
	cerror("in_pop():  too much popping");
    DEBUG(i1debug > 1, ("in_pop(), exit at %d\n", in_sp));
    
    return;
}


static void
in_nextelem(onelevel)
int onelevel;
/* Descend into the initialization stack.  If "onelevel", just go
** down one level into an aggregate.  Otherwise, go as far as is
** necessary to find the next scalar.
*/
{

    do {
	T1WORD t;			/* type at current level */
	T1WORD ttype;			/* generic type at current level */
	SX sid;

	if (in_sp == IN_EMPTY)
	    cerror("confused stack in in_nextelem()");

	DEBUG(i1debug > 1, ("in_nextelem(%d), level %d\n", onelevel, in_sp));

	t = II(in_sp).in_type;
	ttype = TY_TYPE(t);

	if (ttype == TY_STRUCT || ttype == TY_UNION) {
	    BITOFF size;
	    BITOFF offset;
	    int isfield = 0;
	    SIZE curelem = II(in_sp).in_celem;	/* current element number */

	    /* Check whether we're done with this struct/union and need }:
	    ** if the s/u at the current level is full and we had a {.
	    **		struct { int x; } v = { 1, 2 };
	    **					 ^
	    */
	    if (curelem >= II(in_sp).in_nelem) {
		UERROR("too many struct/union initializers");
		/* To continue from error, need some kind of initialization
		** element to look for now.  For better or worse, pick the
		** last element of the struct.
		*/
		curelem = II(in_sp).in_nelem-1;
	    }

	    /* Get next member of structure. */
	    sid = TY_G_MBR(TY_UNQUAL(t), curelem);
	    if (   sid == SY_NOSYM		/* happens with incomplete s/u */
		|| (ttype == TY_STRUCT && SY_CLASS(sid) != SC_MOS)
		|| (ttype == TY_UNION  && SY_CLASS(sid) != SC_MOU)
		)
		cerror("insane structure member in nextelem()");

	    if(SY_FLAGS(sid) & SY_ISFIELD) {
		SY_FLDUPACK(sid, size, offset);	/* sets size, offset */
		isfield = 1;
	    }
	    else {
		size = TY_SIZE(SY_TYPE(sid));
		offset = SY_OFFSET(sid);
	    }
	    in_push(SY_TYPE(sid), II(in_sp).in_off+offset, size, isfield);
	}
	else if (ttype == TY_ARY) {
	    T1WORD mtype = TY_DECREF(t);	/* member type */
	    BITOFF tysize = TY_SIZE(mtype);	/* its size */
	    /* Check whether done array. */
	    if (   II(in_sp).in_nelem != TY_NULLDIM
		&& II(in_sp).in_celem >= II(in_sp).in_nelem
		) {
	        UERROR("too many array initializers");
		/* Push the an array-type element onto the initialization
		** stack anyway to allow compilation to proceed.
		*/
	    }

	    /* Put new element onto the stack. */
	    in_push(
		mtype,
		II(in_sp).in_off + (II(in_sp).in_celem * tysize),
		tysize,
		0
		);
	}
	else {
	    /* Scalar.  If current element is non-zero, must have
	    ** too many initializers.  Just flag first extra one.
	    */
	    if (II(in_sp).in_celem == 1)
		UERROR("too many initializers for scalar");
	    break;			/* hit scalar */
	}
    } while (!onelevel);

    DEBUG(i1debug > 1, ("in_nextelem() exit level %d\n", in_sp));
    return;
}


static void
in_endelem()
/* Done with one scalar.  Adjust initialization stack. */
{
    DEBUG(i1debug > 1, ("in_endelem() enter at %d\n", in_sp));

    /* If we're waiting for a closing }, we can't back up. */
    while (II(in_sp).in_lcflag == 0 && in_sp > 0) {
	in_pop();			/* back out one level */

	/* Bump element number we've finished, and check whether
	** we're done with this item.
	*/
	II(in_sp).in_celem++;
	if (   II(in_sp).in_nelem != TY_NULLDIM
	    && II(in_sp).in_celem < II(in_sp).in_nelem
	    )
	    break;			/* continue at lower level */
    }

    DEBUG(i1debug > 1, ("in_endelem() exit at %d\n", in_sp));
    return;
}


#ifndef	NODBG

static void
in_print()
/* Debug:  print initialization stack. */
{
    int i;

    fprintf(stderr, "++++\n");

    for (i = 0; i <= in_sp; i++) {
	fprintf(stderr, "lev %d, off %5lu, siz %3lu {'s %d, fld %d, cur %3u, N %3u, ",
			i, II(i).in_off, II(i).in_size, II(i).in_lcflag,
			II(i).in_isfield, II(i).in_celem, II(i).in_nelem);
	ty_print(II(i).in_type);
	putc('\n', stderr);
    }
    fprintf(stderr, "----\n");
    return;
}

#endif
