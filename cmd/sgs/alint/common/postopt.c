/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/postopt.c	1.19"
/*
** Routines in this file are done after optimization.  This means
** most of the routines accept a node that has both children optimized,
** but the node itself has not been optimized yet.
*/

#include <stdio.h>
#include <ctype.h>
#include "p1.h"
#include "lnstuff.h"
#include "ldope.h"

#ifdef __STDC__
static int ln_iscast(ND1 *);
static void ln_folded(ND1 *, int);
static void ln_merge(int, int, int);
static void ln_logical(ND1 *);
static void ln_relation(ND1 *);
static void ln_conexpr(int, ND1 *, ND1 *);
static void ln_bitwise(ND1 *, ND1 *);
static void ln_assg(ND1 *);
/* static int ln_chkval(ND1 *, T1WORD, int, int); */
static void ln_conv(ND1 *);
static void ln_subscr(ND1 *, int);
static int ln_cpexp(ND1 *);
static void ln_prec(ND1 *, ND1 *);
#else
static int ln_iscast();
static void ln_folded();
static void ln_merge();
static void ln_logical();
static void ln_relation();
static void ln_conexpr();
static void ln_bitwise();
static void ln_assg();
/* static int ln_chkval(); */
static void ln_conv();
static void ln_subscr();
static int ln_cpexp();
static void ln_prec();
#endif


/*
** Tree expected is a CONV node with a NAME or ICON at the terminal,
** or a NAME or ICON node.
** Return non-zero if there is a cast somewhere.
*/
static int
ln_iscast(tr)
ND1 *tr;
{
    for (;;) {
	if (tr->flags & FF_ISCAST)
	    return 1;
	if (optype(tr->op) == UTYPE)
	    tr = tr->left;
	else return 0;
    }
    /* NOTREACHED */
}



/*
** defines, variables and struct for the "evaluation order undefined"
** check (ln_merge)
*/

#define LNAMES		200
#define VALSET		1
#define VALUSED		2

typedef struct sidlst {
	SX sid;
	short flags;
} SIDLST;

static SIDLST *ln_sids;
static int max_sids = 0;
int ln_sidptr = 0;


/*
** op_optim() calls this routine after the children of node p have
** been folded, but p itself has not yet been folded;
** goal is the current goal of the operator p->op;
** eval1 and eval2 are indexes into an array of structs for checking
** of evaluation order.
*/
/* ARGSUSED */
void
ln_postop(p, goal, eval1, eval2)
ND1 *p;
int goal;
int eval1, eval2;
{
    int op = p->op;
    LNBUG(ln_dbflag > 1, ("ln_postop: goal: %d op: %s walked: %d",
				goal,opst[p->op],(p->flags&FF_WASOPT)));

    if (! ln_walk)
	return;

    ln_folded(p, goal);

    if (optype(op) == BITYPE) {
	switch (op) {
	    case QUEST: case ANDAND: case OROR: case COLON: case COMOP:
		ln_sidptr = 0;
		break;
	    case ASSIGN: case ASG PLUS: case ASG MINUS: case ASG MUL:
	    case ASG AND: case ASG DIV: case ASG MOD: case ASG LS:
	    case ASG RS: case ASG OR: case ASG ER: case ASG ARS:
		while (p->left->op == CONV)
		    p = p->left;
		if ((p->left->op == NAME) || 
		    ((p->left->op == STAR) && (p->left->left->op == NAME))) {
		    ln_merge(eval1, eval2, 0);
		    break;
		}
		/* FALLTHRU */
	    default:
		ln_merge(eval1, eval2, 1);
	}
    }
}



/*
** Called for each node after the children have been folded.
*/
static void
ln_folded(tr, goal)
ND1 *tr;
int goal;
{
    int op = tr->op;
    static const char asgmsg[]=
	"assignment operator \"=\" found where \"==\" was expected";
    LNBUG(ln_dbflag > 1,("ln_folded: op: %s goal: %s  towalk: %d", 
		opst[op], pgoal(goal), ln_walk));

    if (! ln_walk)
	return;

    /* this type was originally unsigned via constantU */
    if ((op == ICON) && (TY_ISUNSIGNED(tr->type)))
	tr->flags |= FF_ORGUNS;

    if (IS_SYMBOL(tr)) {
	SX sid = tr->rval;
	int str;
	LNBUG(ln_dbflag > 1, ("\tid: %s  goal: %d",SY_NAME(sid), goal));

	/* This type isn't a struct or union member */
	str =      (SY_CLASS(sid) == SC_MOS) 
		|| (SY_CLASS(sid) == SC_MOE)
		|| (SY_CLASS(sid) == SC_MOU);

	/*
	** If running as cxref, and this is not a member of a struct,
	** union, or enum, then make a USE or SET record.
	*/
	if (LN_FLAG('R') &&  !str && !(tr->flags & FF_WASOPT))
	    cx_use(sid, goal);

	/*
	** The variable is being used for a condition (if (a) ...),
	** for its value ( .. = a), or for effects (?)
	*/
	if (goal&(CC|EFF|VAL)) {
	    /*
	    ** It is being referenced before it has been assigned a value.
	    ** Issue message, and then SET it to prevent future complaints.
	    */
	    if (   NOTSET(sid)
		&& (SY_CLASS(sid)==SC_AUTO)
		&& !str 
		&& NOTDEFINED(sid)
	       ) {
		   WERROR("variable may be used before set: %s",SY_NAME(sid));
		   SY_FLAGS(sid) |= SY_SET;
	    }
	    SY_FLAGS(sid) |= SY_REF;
	}

	/* If used for address, set and ref. */
	if (goal&AVAL) {
	    SY_FLAGS(sid) |= (SY_SET|SY_REF);
	    
	    /* if running for cflow and sym is a function 
	    ** assume we will call this function.
	    */
	    if (LN_FLAG('W') && TY_ISFTN(SY_TYPE(sid)))
		ln2_outdef(sid, LUE, 0);
	}

	/* Used in assignment - set (if also ref, it was caught above) */
	if (goal&ASSG)
	    SY_FLAGS(sid) |= SY_SET;

	/*
	** Set information for "evaluation order undefined".
	*/
	if (TY_ISNUMTYPE(SY_TYPE(sid))) {
	    if (max_sids == 0) {
		max_sids = LNAMES;
		ln_sids = (SIDLST *) malloc(sizeof(SIDLST) * max_sids);
	    } else if (ln_sidptr >= max_sids) {
		max_sids += LNAMES;
		LNBUG(ln_dbflag > 1, ("expanding sidlst to %d", max_sids));
		ln_sids=(SIDLST *)realloc((char *)ln_sids,sizeof(SIDLST)*max_sids);
	    }
	    ln_sids[ln_sidptr].sid = sid;
	    ln_sids[ln_sidptr].flags = 
			(goal&AVAL) ? 0 : ((goal&ASSG) ? VALSET : VALUSED);
	    ++ln_sidptr;
	}
	/* If we are running for cflow and a global variable was set
	** or referenced, and -x was specified put out an LUM record.
	*/
	if (	LN_FLAG('x')
	     && LN_FLAG('W')
	     && (op == NAME)
	     && (!TY_ISFTN(SY_TYPE(sid)))
	     && (SY_CLASS(sid) == SC_EXTERN || SY_LEVEL(sid) == SL_EXTERN)
	   ) 
		ln2_outdef(sid, LUM, 0);
	return;
    }

    switch(op) {
	case EQ:
	case NE:
	    ln_relation(tr);
	    return;
	case ANDAND:
	case OROR:
	    ln_logical(tr);	/* "logical .... {false/true}"     */
	    if ((tr->left->op == ASSIGN) && !LN_FLAG('h'))
		WERROR(asgmsg);
	    return;
	case ASSIGN:
	    ln_assg(tr);
	    return;
	case CBRANCH:
	    if ((tr->left->op == ASSIGN) && (!ln_cpexp(tr->left))) {
		ND1 *pp = tr->left->right;
		unconv(pp);
		if (!callop(pp->op) && !LN_FLAG('h'))
			WERROR(asgmsg);
	    }
	    if (iscon(tr->left) && !LN_DIR(CONSTANTCONDITION))
		WERROR("constant in conditional context");
	    return;
	case COMPL:
	    ln_bitwise(tr->left, tr->left);
	    return;
	case CONV:
	    ln_conv(tr);
	    return;
	case NOT:
	    ln_conexpr(tr->op, tr->left, tr->left);
	    return;
	case STAR:
	    if ((tr->left->op == UNARY AND) && IS_SYMBOL(tr->left->left))
		ln_subscr(tr->left->left, goal);
	    else if(tr->left->op == ICON && IS_SYMBOL(tr->left))
		ln_subscr(tr->left, goal);
	    return;
    }

    /* 
    ** Function call - write info for pass2
    */
    if (callop(tr->op)) {
	ln2_funccall(tr, goal);
	return;
    }

    /* Relational operator - check for suspicious comparisons
    */
    if (RELATIONAL(op)) {
	ln_relation(tr);
	return;
    }

    if (SHOP(op)) {
	/* ln_shift(tr); */
	ln_bitwise(tr->left, tr->right);
	return;
    }

    if (BIOP(op)) {
	ln_bitwise(tr->left, tr->right);
	return;
    }
}



/*
** This is basically the same as the merge function from 4.2 lint.
** This could be optimized.
*/
static void
ln_merge(np1,np2,flag)
int np1,np2,flag;
{
    register SIDLST *npx, *npy;
    LNBUG(ln_dbflag > 1,("ln_merge: %d %d %d",np1,np2,flag));

    if (! ln_walk)
	return;

    for (npx = &ln_sids[np2]; npx < &ln_sids[ln_sidptr]; ++npx) {
	for (npy = &ln_sids[np1]; npy < &ln_sids[np2]; ++npy) {
	    if (npx->sid == npy->sid) {
		if ((npx->flags == 0) || (npx->flags == (VALSET|VALUSED)))
			/* EMPTY */ ;
		else if (((npx->flags | npy->flags) == (VALSET|VALUSED)) ||
			 (npx->flags & npy->flags & VALSET))
			if (flag)
			    WERROR("evaluation order undefined: %s",
					SY_NAME(npy->sid));

		if (npy->flags == 0)
		    npx->flags = 0;
		else npy->flags |= npx->flags;
		goto foundit;
	    }
	}
	ln_sids[np2].sid = npx->sid;
	ln_sids[np2].flags = npx->flags;
	++np2;
	foundit: ;
    }
    ln_sidptr = np2;
}



/*
** Called after children have been folded.
** Expected tree is:
**
**			ANDAND/OROR
**			/	  \
**			?	  ?
**
** if operator is ANDAND and		|  if operator is OROR and
**   - left op is EQ 			|    - left op is NE
**   - right op is EQ			|    - left op is NE
**   - there is the same variable on one side of each of the {EQ,NE}
**   - the other sides of the {EQ,NE} have constants {ICON,FCON}
**   - the constants are not the same
**   - issue "logical AND ...."         |    - issue "logical OR ...."
**
** This is a "very specialized" check that isn't often useful.
** Fixes some problems with Steffens check,
** [i.e. (a==1) && (a==1) would issue diagnostic "...always false" ]
** and adds some [i.e. (0) && (expr), (1) || (expr)]
**
*/
static void
ln_logical(tr)
ND1 *tr;
{
    ND1 *l = tr->left,  *r = tr->right;
    static const char expfalse[] ="logical expression always false: op \"&&\"";
    static const char exptrue[] ="logical expression always true: op \"||\"";
    LNBUG( ln_dbflag > 1, ("ln_logical"));

    /* 
    ** Check cases of logical and's involving a constant
    ** zero, or cases of logical or's involving a non-zero constant.
    ** Allow CONSTANTCONDITION to suppress this warning in this
    ** case (sometimes macros cause this problem.)
    */
    switch (tr->op) {
	case ANDAND:
	    if ((iszero(l) || iszero(r)) && !LN_DIR(CONSTANTCONDITION)) {
		WERROR(expfalse);
		return;
	    }
	    break;
	case OROR:
	    if ((notzerocon(l) || notzerocon(r)) && !LN_DIR(CONSTANTCONDITION)){
		WERROR(exptrue);
		return;
	    }
	    break;
    }
	
    if (((tr->op == ANDAND) && (l->op == EQ) && (r->op == EQ)) ||
        ((tr->op == OROR) && (l->op == NE) && (r->op == NE))) {

	ND1 *ll= l->left, *lr= l->right, 
	    *rl= r->left, *rr= r->right,
	    *idl, *idr, *vall, *valr;

	unconv(ll);
	unconv(lr);
	unconv(lr);
	unconv(rr);

	if (IS_SYMBOL(ll)) {
	    idl = ll;
	    vall = lr;
	} else if (IS_SYMBOL(lr)) {
	    idl = lr;
	    vall = ll;
	} else return;

	if (IS_SYMBOL(rl)) {
	    idr = rl;
	    valr = rr;
	} else if (IS_SYMBOL(rr)) {
	    idr = rr;
	    valr = rl;
	} else return;

	if (issameid(idl,idr) && notequal(vall,valr))
	    WERROR(tr->op == ANDAND ? expfalse : exptrue);
    }
}



/*
** If the type is char, and it is being compared against 
** a negative number or being compared against zero with the relational
** operators, issue diagnostic.
** Issued only with the -p flag.
**
** If the type is unsigned, and it is being compared against
** a negative number or being compared against zero with the relational
** operators, issued "comparison of unsigned ....."
*/
#define NEG_OR_0(p)		(iszero(p) ? "0" : "negative constant")
#define SUS_COMP(p, op)		(((isnegcon(p) || (p->flags&FF_TRUNC)) \
					&& !(p->flags&FF_ORGUNS)) \
				||  \
				 (iszero(p) && RELATIONAL(op)))
static void
ln_relation(tr)
ND1 *tr;
{
    static const char susunsig[]=
	"suspicious comparison of %s with %s: op \"%s\"";
    ND1 *l = tr->left, *r = tr->right;
    int op = tr->op;
    LNBUG( ln_dbflag > 1, ("ln_relation"));

    if (ln_iscast(l) || ln_iscast(r))
	return;

    /*
    ** Get rid of implicit CONV's
    */
    unconv(l);
    unconv(r);

    if (! IS_SYMBOL(l) && ! IS_SYMBOL(r))
	return;

    if (LN_FLAG('p')) {
	if ((TY_TYPE(l->type) == TY_CHAR) && SUS_COMP(r, op))
	    WERROR(susunsig, "char", NEG_OR_0(r), ln_opst[op]);
	else if ((TY_TYPE(r->type) == TY_CHAR) && SUS_COMP(l,op))
	    WERROR(susunsig, "char", NEG_OR_0(l), ln_opst[op]);
    }

    if (TY_ISUNSIGNED(l->type) && SUS_COMP(r, op))
	WERROR(susunsig, "unsigned", NEG_OR_0(r), ln_opst[op]);
    else if (TY_ISUNSIGNED(r->type) && SUS_COMP(l, op))
	WERROR(susunsig, "unsigned", NEG_OR_0(l), ln_opst[op]);
}



/*
** Check for both operands being constant.
** Can be suppressed with -h or CONSTANTCONDITION
*/
static void
ln_conexpr(op,l,r)
int op;
ND1 *l, *r;
{
    LNBUG( ln_dbflag > 1, ("ln_conexpr"));

    if (!LN_FLAG('h') && !LN_DIR(CONSTANTCONDITION) && iscon(l) && iscon(r))
	WERROR("constant operand%s to op: \"%s\"", 
		(optype(op)==BITYPE) ? "s": "", opst[op]);
}



/*
** If running with -Xc and there is an signed
** variable in a bitwise operation, issue diagnostic.
*/
static void
ln_bitwise(l,r)
ND1 *l, *r;
{
    LNBUG( ln_dbflag > 1, ("ln_bitwise"));

    if (   (version&V_STD_C) 
	&& (   (TY_ISSIGNED(l->type) && !(l->op == ICON))
	    || (TY_ISSIGNED(r->type) && !(r->op == ICON))
	   )
       )
	/* "bitwise operation on signed value possibly nonportable" */
	BWERROR(12);

    /* Check for precedence confusion. */
    ln_prec(l,r);
}


static int
ln_chksz(ty, p)
T1WORD ty;
ND1 *p;
{
    if (p->op == QUEST)
	return ln_chksz(ty, p->right->left) && ln_chksz(ty, p->right->right);
    if (   TY_ISINTTYPE(p->type) 
	&& IS_SYMBOL(p)
	&& (TY_SIZE(ty) < TY_SIZE(p->type))
       )
	return 0;
    return 1;
}



/*
** Tree expected is:
**
**                =
**               / \
**              ??  ??
**
*/
static void
ln_assg(tr)
ND1 *tr;
{
    ND1 *l = tr->left, *r = tr->right;
    LNBUG( ln_dbflag > 1, ("ln_assg"));

    if (ln_iscast(l) || ln_iscast(r))
	return;

    if (iscon(r)) {
	/*
	** Lint's treatment of constants could be improved ...
	**
	** To issue "assignment of negative ...", the tree must look
	** like:
	**			=
	**		      /   \
	**		unsigned   ICON with negative value,
	**		 type           that was not cast to unsigned
	**
	** This unfortunately will also suppress the following:
	**	unsigned ui;
	** 	ui = (signed) -1;
	** because the FF_ISCAST is propagated down, but lint doesn't
	** know what the cast was.
	*/
	if (   TY_ISUNSIGNED(l->type) 
	    && isnegcon(r) 
	    && !(r->flags&FF_ORGUNS)
	    && !(TY_ISUNSIGNED(r->type) && (r->flags&FF_ISCAST))
	   )
	    WERROR("assignment of negative constant to unsigned type");

	/*
	** Complain about truncation when the FF_TRUNC is set, *and*
	** the type of the integer was not set with "U".
	** This will, unfortunately, suppress some valid complaints.
	** But it is better (?) to miss some cases, than to issue
	** bogus messages.
	*/
	if (   (r->flags & FF_TRUNC)
	    && !(r->flags & FF_ORGUNS)
	   )
	    WERROR("constant truncated by assignment");

	if (l->op == FLD) {
	    int sz = 0;
	    sz = UPKFSZ(l->rval);
	    if (sz != 0)
		in_chkval(r, l->type, sz, 1, 
			"precision lost in bit-field assignment");
	    return;
	}
    }

    unconv(l);
    unconv(r);

    /* 
    ** "assignment causes implicit narrowing conversion"
    */
    if (!LN_FLAG('a') && !ln_chksz(l->type, r))
	BWERROR(14);

    /* 
    ** "conversion to larger integral type may sign-extend incorrectly" 
    */
    if (   LN_FLAG('p') 
	&& !LN_FLAG('a')
	&& TY_ISNUMTYPE(l->type) 
	&& (TY_TYPE(r->type) == TY_CHAR)
	&& (TY_SIZE(l->type) > TY_SIZE(r->type))
       )
	    BWERROR(13);
}



/*
** ln_conv - Tree known is:
**
**    CONV type tc
**     |
**     ?  type tl
**
*/
static void
ln_conv(p)
ND1 *p;
{
    TWORD tc = p->type, tl = p->left->type;
    LNBUG(ln_dbflag > 1, ("ln_conv"));

    if (   (TY_ISPTR(tc) && (TY_TYPE(TY_DECREF(tc)) == TY_VOID)) 
	|| (TY_ISPTR(tl) && (TY_TYPE(TY_DECREF(tl)) == TY_VOID))
       )
	return;

    if (  !TY_ISPTR(tc) 
	&& (TY_TYPE(tc) != TY_VOID)
	&& TY_ISPTR(tl) 
	&& (TY_SIZE(TY_TYPE(tc)) < TY_SIZE(tl))
       )
	WERROR("conversion of pointer loses bits");

    if (LN_FLAG('p') && TY_ISPTR(tc) && TY_ISPTR(tl)) {
	if (! TY_EQTYPE(tc,tl))
	    /* pointer casts may be troublesome */
	    BWERROR(4);
    }
}




/*
** Check for negative or out-of-bounds subscripting.
** Tree expected is the tree under a STAR operator.
*/
static void
ln_subscr(tr, goal)
ND1 *tr;
int goal;
{
    SX sid = tr->rval;
    T1WORD ty;
    LNBUG(ln_dbflag > 1, ("ln_subscr"));

    if (sid == ND_NOSYMBOL)
	return;

    ty = SY_TYPE(sid);

    if (TY_ISARY(ty)) {
	int nelem = TY_NELEM(ty), subsc;

	/*
	** No dimension has been defined for this array, so no
	** checks can be done here.
	*/
	if ((nelem == TY_NULLDIM) || (nelem == TY_ERRDIM))
	    return;
	subsc = ((int) tr->lval / ((int) BITOOR(TY_SIZE(TY_DECREF(ty)))));

	/*
	** If negative subscript, issue negative message.
	** If >= nelem, issue > message (cannot be = because a[10] has
	** 10 elements, but can only address 0-9)
	*/
	if (subsc < 0)
	    WERROR("array subscript cannot be negative: %d", subsc);
	else if (goal != AVAL) {
	    if ((subsc >= nelem))
		WERROR("array subscript cannot be > %d: %d", (nelem-1), subsc);
	}
	/* allow for pointers one past the last element, which is valid ANSI */
	else if (subsc > nelem)
		WERROR("array subscript cannot be > %d: %d", nelem, subsc);
    }
}



/*
** This is used to suppress the "= found where == expected" message
** for cases like: (*p++ = *q++)
**
** Tree expected:                to return 1:
**      = (CC)                       = (CC)
**     / \                          /  \
**   ??  ??                        *    *
**                                 |    |	(where either or both X's are
**                                 X    X        a INCR or DECR node)
**
** Return 1 if the h flag was used.
*/
static int
ln_cpexp(tr)
ND1 *tr;
{
    LNBUG( ln_dbflag > 1, ("ln_cpexp"));

    if (LN_FLAG('h'))
	return 1;

    if (((tr->left->op == STAR) && (ln_dope[tr->left->left->op] & CPY)) ||
	((tr->right->op == STAR) && (ln_dope[tr->right->left->op] & CPY)))
	    return 1;

    return 0;
}




/*
** Called when a bit-wise expression was seen.
** Tree expected:
**
**         BIT-OP    
**         /    \     
**	  X1    X2
**
** Looking for LGOP on either X1 or X2
*/
static void
ln_prec(l,r)
ND1 *l, *r;
{
    LNBUG(ln_dbflag > 1, ("ln_prec"));

    if (!LN_FLAG('h') &&
	((LGOP(l->op) && (! (l->flags&FF_PAREN))) ||
	 (LGOP(r->op) && (! (r->flags&FF_PAREN)))))
		WERROR("precedence confusion possible; parenthesize");
}
