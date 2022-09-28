/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/optim.c	52.106.2.1"
/* optim.c */

/* This module contains the expression tree optimization routines. */

#include "p1.h"
#include <errno.h>
#include <setjmp.h>
#include <string.h>

#ifdef LINT
static int curgoal;
#define LN_OPTIM(tr,p,side)	{ curgoal = ln_getgoal(p,curgoal,side); \
                                casted = tr->flags & FF_ISCAST; \
                                tr = optimize(tr); \
                                if (casted) \
                                    tr->flags |= FF_ISCAST; \
                                curgoal = old_curgoal; }
#endif

extern void save_fpefunc();
static void op_fpe_handler();
static jmp_buf fpe_buf;
static int op_ispow2();
static ND1 * optimize();
static ND1 * op_uand();
static ND1 * op_cascade();
static ND1 * op_right_con();
static ND1 * op_arrayref();
static int op_int_oflow();
static int op_int_ocheck();
static CONVAL op_dtol();
static UCONVAL op_dtoul();
static int op_tysize();

#define OP_NOSEFF(p)	!((p)->flags & FF_SEFF)		/* no side effects */
#define OP_ISNNCON(p)	((p)->op == ICON && (p)->rval == ND_NOSYMBOL)
#define TY_DEUNSIGN(t)  tr_deunsign(t)


#ifndef	NODBG
static void  op_oprint();
#define	ODEBUG(lev,p,s) if (o1debug > (lev)) op_oprint(p,s)
#else
#define	ODEBUG(lev,p,s)
#endif

int o1debug = 0;		/* debug flag */
static int op_forinit;		/* non-0 if optim for initializer; if 0,
				** certain FP operations are suppressed
				*/

ND1 *
op_optim(p)
ND1 * p;
{
    ODEBUG(0, p, "BEFORE OPTIM");

#ifdef LINT
    curgoal = ln_dflgoal();
#endif
    cg_treeok();		/* assume tree is okay going in */

    p = optimize(p);
    /* Structure operations sometimes get a spurious STAR operator. */
    if (p->op == STAR && TY_ISSU(p->type)) {
	p->op = FREE;
	p = p->left;
    }
    ODEBUG(0, p, "AFTER OPTIM");
    return(p);
}

ND1 *
op_init(p)
ND1 * p;
/* Do optimizations for initializers.  Certain FP optimizations
** are enabled for initializers only.
*/
{
    op_forinit = 1;		/* flag enables special optimizations */
    p = op_optim(p);
    op_forinit = 0;		/* disable stuff again */
    return( p );
}

static ND1 *
optimize( p )
ND1 * p;
/* Remove unnecessary code.  Also, change implicit structure operations
** (CALL, ASSIGN, FUNARG) to explicit ones, rewrite them as appropriate
** for Pass 2.
*/
{
    static ND1 * op_fconfold();
    static ND1 * op_iconfold();
    static ND1 * op_left_con();
    static ND1 * op_su();
    static ND1 * op_plus();
    static ND1 * op_mul();
    static ND1 * op_conv();
    static ND1 * op_bitfield();
    static ND1 * op_call();

    ND1 * l;
    ND1 * r;
    BITOFF offset;
    int vol_flag = 0;		/* to save volatile flag status */
#ifdef LINT
    int old_curgoal = curgoal;
    int eval1, eval2, casted;	/* eval[12] used for eval order undefined */
#endif

    if (p->flags & FF_WASOPT)
	return( p );

#ifdef LINT
    /*
    ** Check for side effects if the current goal is EFF.  If there
    ** were no side effects (ln_sides() returns 0), then set the
    ** goal to VAL to prevent future complaints.
    */
    if ((curgoal == EFF) && !ln_sides(p))
        curgoal = old_curgoal = VAL;
  
    /*
    ** Normally optimize the left subtree first (so expressions such as
    ** (a=1) && (b==a) get evaluated in the correct order),
    ** unless op is an assignment op, in which case we reverse (so
    ** a = a + 1; gets evaluated in the correct order.)
    */
    if (asgop(p->op)) {
	/* Assume all assign-ops are binary. */
	eval1 = ln_sidptr;
	LN_OPTIM(p->right, p, LN_RIGHT);
	if (p->right->op == ICON || p->right->op == FCON)
	    p = op_right_con(p);
	eval2 = ln_sidptr;
	if (optype(p->op) != LTYPE)
	    LN_OPTIM(p->left, p, LN_LEFT);
    } else if (optype(p->op) != LTYPE) {
        eval1 = ln_sidptr;
        LN_OPTIM(p->left, p, LN_LEFT);
	if (p->left->op == ICON || p->left->op == FCON)
	    p = op_left_con(p);
        if (optype(p->op) == BITYPE) {
            eval2 = ln_sidptr;
            LN_OPTIM(p->right, p, LN_RIGHT);
	    if (p->right->op == ICON || p->right->op == FCON)
		p = op_right_con(p);
        }
    }

    /*
    ** Main routines in ln_postop(); also check for evaluation order
    ** undefined.
    */
    ln_postop(p, curgoal, eval1, eval2);
    if (curgoal == EFF)
        curgoal = VAL;
#else

    /* Do left-side first to check for ||, && with constant operand. */
    if (optype(p->op) != LTYPE) {
	p->left = optimize(p->left);
	if (p->left->op == ICON || p->left->op == FCON)
	    p = op_left_con(p);
	/* p's optype could be different now. */
    }
    if (optype(p->op) == BITYPE) {
	p->right = optimize(p->right);
	if (p->right->op == ICON || p->right->op == FCON)
	    p = op_right_con(p);
	/* p's optype could be different now. */
    }
#endif

    l = p->left;
    r = p->right;

    /* Check for constant folding possibilities */
    /* As a result of arithmetic promotions, ICON op FCON (and vice versa)
    ** are now FCON op FCON.
    */
    switch (optype(p->op)){
    case UTYPE:
	if (l->op == ICON) p = op_iconfold(p);
	else if (l->op == FCON) p = op_fconfold(p);
	break;
    case BITYPE:
 	if ((l->op == ICON) && (r->op == ICON))  p = op_iconfold(p);
	else if ((l->op == FCON) && (r->op == FCON))  p = op_fconfold(p);
	else if (logop(p->op) && l->op == ICON && r->op == FCON) p = op_fconfold(p);
	break;
    }

    switch(p->op){
    case UNARY AND:
	switch(l->op){
	SY_CLASS_t sc;

	case NAME:
	    /* Turn & over NAME into an ICON
	    **
	    **		&
	    **		|    =>    ICON
	    **	       NAME
	    */
	    if (l->rval > 0){
	        if ((sc = SY_CLASS(l->rval)) == SC_AUTO)
		    break;
		else if (sc == SC_PARAM) {
#ifdef	SU_PARAM_PTR
		    if (sc == SC_PARAM && TY_ISSU(SY_TYPE(l->rval))) {
			p->op = FREE;
			l->type = ty_mkptrto(l->type);
			return( l );
		    }
#endif
		    break;
		}
	    }
	    l->op = ICON;
	    /* FALLTHRU */
	case STRING:
	    /* Treat STRING similar to  NAME:  set ICON bit. */
	    if (l->op == STRING)
		l->rval |= TR_ST_ICON;
	    l->type = p->type;
	    p->op = FREE;
	    p = l;
	    break;
	case STAR:
	    /* Eliminate & over STAR 
	    **
	    **		&
	    **		|    =>    nothing
	    **		*
	    */
	    ODEBUG(0, p, "U& over STAR optimizing");
	    p->op = FREE;
	    l->op = FREE;
	    l->left->type = p->type;	 /* preserve type of p */
	    p = l->left;
	    ODEBUG(0, p, "after U&-->* optimization");
	    break;
	}

	break;
    case FUNARG:
	/* structure argument tree rewrite */
	if ( TY_ISSU(p->type) )
	    p = op_su(p, STARG);
	break;
    case ASSIGN:
        /* structure assignment tree rewrite */
	if ( TY_ISSU(p->type) ){
	    p = op_su(p, STASG);
	    break;
	}
	/* Rewrite a = a op b to a op= b.
	**
	**	=				op=
	**    /   \			       /   \
	**  NAME   op		==>	     NAME   X
	**	 /    \
	**	NAME   X
	**
	** There's a special case when the NAME has type
	** other than int.  The trees look like:
	**
	**	=				op=
	**    /   \			       /   \
	**  NAME   CONV		==>	     CONV   X
	**	    |			       |
	**	    op			     NAME
	**	  /    \
	**	CONV	X
	**	  |
	**	NAME
	**
	** The p->right->op CONV is to the type of the =;
	** the other is to the type of op.
	** This improvement is safe only if the target machine does
	** not trap overflows, because we're changing the "size" of
	** the operation.
	*/
	if (l->op == NAME) {
	    int convcase = 0;		/* CONV case */

#ifndef TRAP_OVERFLOW
	    if (r->op == CONV) {
		convcase = 1;
		r = r->left;
	    }
#endif
	    switch (r->op){
	    case OR:
	    case AND:
	    case ER:
	    case LS:
	    case RS:
		if (TY_ISFPTYPE(l->type))
		    break;
		/* FALLTHRU */
	    case PLUS:
	    case MINUS:
	    case MUL:
	    case DIV:
	    {
		register ND1 * rl = r->left;

		if (convcase) {
		    if (rl->op != CONV)
			break;
		    rl = rl->left;
		}

		/* left side of ASSIGN must be == to l */
		if (   rl->op == NAME
		    && rl->lval == l->lval
		    && rl->rval == l->rval
		    && rl->type == l->type )
		{
		    ODEBUG(1, p, "rewrite ASSIGN to ASG OP: change:");
		    /* free original top of tree, change op */
		    p->left->op = FREE;
		    p->op = FREE;
		    if (convcase)
			p->right->op = FREE;
		    r->op += ((ASG PLUS) - PLUS);
		    p = r;
		    p->type = l->type;		/* really for CONV case */
		    ODEBUG(1, p, "to:");
		} /* end if */
	    } /* end block */
	    } /* end switch on right op */
	    l = p->left;			/* restore pointers */
	    r = p->right;
	} /* end if */
    	break;
    case CALL:
    case UNARY CALL:
	/* structure call tree rewrite */
	if ( TY_ISSU(p->type) )
	    p = op_su(p, (p->op == CALL ? STCALL : UNARY STCALL) );
	else if (! op_forinit && p->left->op == ICON && (p->left->flags & FF_BUILTIN) != 0)
	    p = op_call(p);		/* check for built-in rewrites */
	break;
    case STAR:
	switch (l->op) {
	case UNARY AND:
	    /* Eliminate STAR over &
	    **
	    **	*
	    **	|    =>    nothing
	    **	&
	    */
	    l->op = FREE;
	    l->left->type = p->type;		/* preserve the type of STAR */
	    p->op = FREE;
	    p = l->left;
	    break;
	case ICON:
	    /* STAR over ICON becomes NAME
	    **
	    **	*
	    **	|        =>    NAME+val
	    ** ICON(val)
	    */
	    /* Make this into NAME. */
	    p->op = FREE;
	    l->op = NAME;
	    l->type = p->type;
	    if (TY_ISVOLATILE(l->type)) l->flags |= FF_ISVOL;
	    p = l;
	    break;
	case STRING:
	    /* STAR over STRING reverts to STRING.  Type is STAR's type.
	    **
	    **	*
	    **	|	=>	STRING
	    ** STRING
	    */
	    l->type = p->type;
	    l->rval &= ~TR_ST_ICON;	/* treat like pseudo NAME node */
	    p->op = FREE;
	    p = l;
	    break;
	}
	break;
    case ASG MINUS:
    case MINUS:
	/* Change MINUS of constant to plus of its negative, if possible. */
	if (r->op == FCON)
	    r->dval = FP_NEG(r->dval);
	else if (   r->op == ICON
		 && r->rval == ND_NOSYMBOL
		 && ! op_int_ocheck(UNARY MINUS, r->lval, 0, (CONVAL *) 0)
	)
	    r->lval = - r->lval;
	else
#ifndef	TRAP_OVERFLOW
	    p->right = tr_generic(UNARY MINUS, r, r->type);
#else
	    /* computation could overflow due to rewrite */
	    break;
#endif
	p->op += (PLUS - MINUS);
	/*FALLTHRU*/
    case ASG PLUS:
    case PLUS:
	p = op_plus(p);
	break;
    case ASG MUL:
    case MUL:
	p = op_mul(p);
	break;	
    case CONV:
	p =  op_conv(p);
	break;
    case DOT:
	/* rewrite DOT operator, turn it into a pointer to p
	** then rewrite like the STREF op.
	*/
	if (p->right->flags & FF_ISVOL) l->flags |= FF_ISVOL;
	p->left = l = op_uand( l, ty_mkptrto(l->type) );
	/*FALLTHRU*/
    case STREF:
	/* if a pointer to volatile su, set vol_flag */
	if (TY_ISPTR(l->type) && TY_ISVOLATILE(TY_DECREF(l->type))) vol_flag = 1;
	/* rewrite STREF op, into pointer plus offset */
	/* if p is a bitfield, building the offset is different */
	if (p->flags & FF_ISFLD){
	    p = op_bitfield(p); 
	    break;
	}
	offset = SY_OFFSET(r->rval);		/* get offset in struct */

	/* Build pointer plus. Tree will look like:
	**
	**		*
	**		|
	**		+
	**	      /   \
	**	   NAME   ICON
	*/
	if (offset){		/* only build plus if offset is non-zero */
	    p->op = PLUS;
	    p->flags &= ~FF_ISVOL;		/* don't want the + node set */
	    p->type = ty_mkptrto(p->type);
	    p->right = tr_icon( (CONVAL) BITOOR(offset) );
	    p = tr_generic(STAR, p, r->type); 
	    r->op = FREE;
	}
	else {
	    p->op = FREE;
	    r->op = FREE;
	    p = l;
	    p = tr_generic(STAR, p, r->type);
	}
	ODEBUG(1, p, "rewritten structure reference");
	p = optimize( p );
	if(vol_flag)  p->flags |= FF_ISVOL; 
	break;

    case NAME:
	/* Change member of ENUM into integer constant. */
	if(p->rval > 0 && SY_CLASS(p->rval) == SC_MOE){
	    p->op = ICON;
	    p->lval = SY_OFFSET(p->rval);
	    p->rval = ND_NOSYMBOL;
	}
	break;

    /* Handle cascaded constants. */
    case OR:
    case ER:
    case AND:
    case LS:
    case RS:
	if (OP_ISNNCON(r))
	    p = op_cascade(p);
	break;
    } /*end switch*/

    /* Fix up assignment ops with CONV on left, if possible. */
    if (    (asgop(p->op))
	&&  ((l = p->left)->op == CONV)
	&&  (!TY_ISFPTYPE(l->type)) 
	&&  (TY_ISNUMTYPE(l->type))
	&&  (TY_ISNUMTYPE(l->left->type))
	&&  (op_tysize(l->type) >= op_tysize(l->left->type))
       ) {
	switch( p->op ){
	case ASG OR:
	case ASG ER:
	case ASG AND:
	    if (TY_ISFPTYPE(l->left->type))
		break;
	    /* FALLTHRU */
	case ASG MUL:
	case ASG PLUS:
	case ASG MINUS:
	    /* transfer CONV ops on left-side to the right-side
	    ** don't do it to ASG DIV, ASG MOD, ASG RS, ASG LS.
	    **
	    **    (CONV) A op= B  into A op= (CONV) B
	    */
	    ODEBUG(1, p, "*** ASG OP optimization ***");
	    p->left = l->left;
	    l->left = p->right;
	    l->type = p->type;
	    l->flags &= ~FF_WASOPT;	/* allow further optimization */
	    p->right = optimize( l );
	    /* unset volatile field for right-side when an ICON */
	    if (p->right->op == ICON)
		p->right->flags &= ~FF_ISVOL;
	    ODEBUG(1,  p, "*** result of ASG OP optim ***");
	    break;
	}
    }

    p->flags |= FF_WASOPT;
    return (p);
}


static ND1 *
op_cascade(p)
ND1 * p;
/* Look for cascaded operators with constants on the right.
** Try to fold them if there is no overflow.
*/
{
    ND1 * l;
    ND1 * r;
    ND1 * lr;
    int op = p->op;
    int evalop = 0;

    ODEBUG(1, p, "Start cascade optimization");

    /* Look for two binary ops and an ICON to the right of the lower one. */
    if ( !(   optype(op) == BITYPE
	   && optype((l = p->left)->op) == BITYPE
	   && l->right->op == ICON
	  )
    )
	return( p );

/* This macro for combinations of operators in a cascade tree. */
#define OP_OP(l,t) ((t)*DSIZE+l)

    r = p->right;

    if (! OP_ISNNCON(r)) {
	/* Commute operators, where possible, if overflow is not
	** a problem.  This moves constants rightward in the
	** tree in hopes of finding a place to fold.
	*/
	if (op == l->op) {
	    switch(op) {
#ifndef TRAP_OVERFLOW
	    case PLUS:
	    case MUL:
#endif
	    case AND:
	    case OR:
	    case ER:
		p->right = l->right;
		l->right = r;
		/* We may interchange ICON ptr l->right with int
		** expression @ r.  Adjust type.
		*/
		l->type = r->type;
	    }
	}
#ifndef	TRAP_OVERFLOW
	else if (op == MUL && l->op == LS && OP_ISNNCON(l->right)) {
	    /* In this case the multiply may have been changed to a
	    ** left shift:
	    **
	    **		*			<<
	    **	      /   \		      /    \
	    **	     <<	  E2	  ==>	     *	  ICON
	    **	   /	\		   /   \
	    **	  E1   ICON		  E1   E2
	    */
	    p->left = l->left;
	    l->left = p;
	    p = l;
	}
#endif
	return( p );
    }

    /* Now have this tree, where the top node is not an
    ** assignment operator, but the lower one might be.
    **
    **		OP1
    **	      /    \
    **	     OP2  ICON(b)
    **     /    \
    **    T   ICON(a)
    */
    
    /* Determine the combinations that we can deal with. */

    lr = l->right;

    switch( OP_OP(l->op, op) ){
#ifndef	TRAP_OVERFLOW
    case OP_OP(MINUS, PLUS):
	/* Just negate the right-side constant. */
	r->lval = - r->lval;
	/*FALLTHRU*/
#endif
    case OP_OP(MINUS, MINUS):
    case OP_OP(PLUS,PLUS):		evalop = PLUS; break;
#if 0	/* X - ICON becomes X + -ICON. */
    case OP_OP(PLUS, MINUS):		evalop = MINUS; break;
#endif
    case OP_OP(MUL, MUL):		evalop = MUL; break;
#ifndef	TRAP_OVERFLOW
    case OP_OP(LS, MUL):
	if (lr->lval >= 0 && lr->lval < TY_SIZE(p->type)) {
	    lr->lval = 1L << lr->lval;
	    evalop = MUL;
	    l->op = MUL;
	}
	break;
    case OP_OP(MUL, LS):
	if (r->lval >= 0 && r->lval < TY_SIZE(p->type)) {
	    r->lval = 1L << r->lval;
	    evalop = MUL;
	}
	break;
#endif
    case OP_OP(OR, OR):			evalop = OR; break;
    case OP_OP(ER, ER):			evalop = ER; break;
    case OP_OP(AND, AND):		evalop = AND; break;
    case OP_OP(RS, RS):
    case OP_OP(LS, LS):
	/* Avoid if shifting too far. */
	if (lr->lval+r->lval < TY_SIZE(p->type))
	    evalop = PLUS;
	break;
    }

    if (evalop == 0)
	return( p );			/* nothing to do */

#ifdef	TRAP_OVERFLOW			/* If hardware traps overflows. */
    /* Check for combinations that would cause overflow if the constants
    ** were combined.  Don't rewrite such trees.
    */
    {
	if (op == LS || op == RS)
	    /*EMPTY*/ ;
	else if (   TY_ISSIGNED(p->type)
		 && op_int_ocheck(evalop, l->right->lval, r->lval, (CONVAL *)0)
		)
		return( p );
    }
#endif

    /* Now change the tree to look like this.
    **
    **		  OP
    **		/    \
    **	       T  ICON(a OP b)
    */
    switch( evalop ) {
    case PLUS:	lr->lval = (UCONVAL) lr->lval + (UCONVAL) r->lval; break;
#if 0
    case MINUS:	lr->lval = (UCONVAL) lr->lval - (UCONVAL) r->lval; break;
#endif
    case MUL:	lr->lval = (UCONVAL) lr->lval * (UCONVAL) r->lval; break;
    case ER:	lr->lval = (UCONVAL) lr->lval ^ (UCONVAL) r->lval; break;
    case OR:	lr->lval = (UCONVAL) lr->lval | (UCONVAL) r->lval; break;
    case AND:	lr->lval = (UCONVAL) lr->lval & (UCONVAL) r->lval; break;
    default:
	cerror("confused op_cascade()");
    }
    r->op = FREE;
    p->op = FREE;
    p = op_right_con(l);
    ODEBUG(1, p, "End cascade optimization");
    return( p );
}


static ND1 *
op_plus( p )
ND1 * p;
/* Optimize PLUS or ASG PLUS node, MINUS or ASG MINUS: do
** comm/assoc optimizations
*/
{
    ND1 * r = p->right;
    ND1 * l = p->left;

    /* Move vanilla ICON's to the right (and leave pointer
    ** ICON's on the left.)
    */
    if (OP_ISNNCON(l)){
	ND1 * temp;
	temp = l;
	l = p->left = r;
	r = p->right = temp;
    }
    ODEBUG(1, p,"Optimizing PLUS");

    /* C-tree:
    **		+
    **	      /   \		        &
    **	     &    ICON(val)    =>       |
    **	     |			     NAME+val
    ** 	    NAME
    **
    ** This tree must be pointer addition, in which case the value
    ** on the right is a byte offset.
    */
    if (l->op == UNARY AND && OP_ISNNCON(r) && (l->left->op == NAME))
    {
	ND1 * ll = l->left;
#ifndef LINT
	int space;
	switch( SY_CLASS(ll->rval) ){
	case SC_PARAM:	space = VPARAM; break;
	case SC_AUTO:	space = VAUTO; break;
	default:
	    cerror("op_plus:  unexpected class %d", SY_CLASS(ll->rval));
	}
	if (!op_int_ocheck(MUL, r->lval, (CONVAL)TY_SIZE(TY_CHAR), (CONVAL *)0)) {

	    ll->lval =
		cg_off_incr(space, ll->lval, (long) (r->lval * TY_SIZE(TY_CHAR)));
	    p->op = r->op = FREE;
	    return( l );
	}
	return( p );	/* no other optimizations */
#else
	ll->lval += r->lval;
	p->op = r->op = FREE;
	return( l );
#endif
    }

    /* Try to fold certain array or pointer reference operations. */

#ifndef TRAP_OVERFLOW
    if (TY_ISPTR(p->type) && !asgop(p->op))
	p = op_arrayref(p);
#endif

    /* Now move named ICON to right of PLUS to improve cascade optimizations. */
    if (p->op == PLUS && (l = p->left)->op == ICON && l->rval != ND_NOSYMBOL) {
	p->left = p->right;
	p->right = l;
    }

    /* Check for cascaded PLUS's. */
    p = op_cascade(p);

    /* Minimize number of negatives. */
    switch( p->op ){
	int change;
    case PLUS:
    case ASG PLUS:
    case MINUS:
    case ASG MINUS:
	change = 0;
	r = p->right;
	if (r->op == UNARY MINUS) {
	    p->right = r->left;
	    r->op = FREE;
	    change = 1;
	}
	else if (OP_ISNNCON(r) && r->lval < 0) {
	    /* Change + of negative constant back to - of positive constant. */
	    if (   ! TY_ISSIGNED(p->type)
		|| ! op_int_ocheck(UNARY MINUS, r->lval, 0, (CONVAL *) 0)
	    ) {
		r->lval = - r->lval;
		change = 1;
	    }
	}
	if (change) {
	    if (p->op == PLUS || p->op == ASG PLUS)
		p->op += (MINUS - PLUS);
	    else
		p->op += (PLUS - MINUS);
	}

    }

    /* In cascaded pointer expression, try to move ICON to right,
    ** in hopes it might be used in register/offset address mode.
    ** The + case is already handled by op_cascade().
    **
    **		+|-				  -
    **	      /     \			       /    \
    **	      -     X2		==>	     +|-   ICON
    **     /     \			    /    \
    **    X1	ICON			   X1	 X2
    */
#ifndef TRAP_OVERFLOW		/* because we're rearranging operators */
    /* recheck p->op: previous optimizations may have reduced tree */
    if ((p->op == PLUS || p->op == MINUS)
	&& TY_ISPTR(p->type) 
	&& !OP_ISNNCON(p->right)
       ) {
	l = p->left;
	if (l->op == MINUS && OP_ISNNCON(l->right)) {
	    /* Move the ICON to the right of p. */
	    ODEBUG(1, p, "Before +|- ICON rotate");
	    p->left = l->left;
	    l->left = p;
	    p = l;
	    ODEBUG(1, p, "After +|- ICON rotate");
	}
    }
#endif
    return (p);
}


#ifndef	TRAP_OVERFLOW	/* avoid because of generated constant multiply */

static ND1 *
op_arrayref(p)
ND1 * p;
/* Fold array and pointer references. */
{
    ND1 * l;
    ND1 * r = p->right;
    ND1 * rl;
    ND1 * new;
    int charcase;

    /* Look for trees that would result from an expression like
    **		array[X+C]
    **		array[X-C]
    **		array[C-X]
    ** where array is some array type, X is some index expression,
    ** C is a constant.
    ** The tree transform looks like:
    **
    **		+|-			+|-
    **	       /   \		       /   \
    **	      &	   *|<<	     -->     +|-    \
    **	    NAME  /	\	   /    \    *|<<
    **		+|-	C2	  &    *|<<  /   \
    **	       /   \		NAME  /   \  X	  C2 
    **	      X	    C1		    C1    C2
    **
    ** A couple of notes:  (& NAME) could look like ICON if
    ** the storage class for NAME is SC_STATIC or SC_EXTERN.
    ** The expectation is that the C1 * C2 tree will be
    ** collapsed and folded into the (& NAME) tree.
    ** Of course, the type of the top tree is PTR.
    ** NOTE:  There is a special case when C2 is 1 (for char
    ** arrays), in which case the *|<< tree is missing.
    **
    ** Look for trees that would result from an expression like
    **		ptr[X+C]
    **		ptr[X-C]
    **		ptr[C-X]
    ** where ptr is some pointer type, X is some index expression,
    ** C is a constant.
    ** The tree transform looks like:
    **
    **		+|-			+|-
    **	       /   \		       /   \
    **	      p	   *|<<	     -->     +|-    \
    **	         /	\	   /    \    *|<<
    **		+|-	C2	  p    *|<<  /   \
    **	       /   \		      /   \ C1	  C2 
    **	      X	    C1		     X    C2
    **
    ** The expectation is that the C1 * C2 tree will be
    ** collapsed and can be used as part of a *(REG + offset)
    ** address mode, where the pointer expression gets evaluated
    ** into a register.  On machines with fancy double indexing
    ** address modes, the shift/multiply might get handled by
    ** the address mode, too.
    **
    ** NOTE:  There is a special case when C2 is 1 (for char
    ** arrays), in which case the *|<< tree is missing.
    **
    */

    /* Look for a match on the right side first, because it is
    ** common to the two transformations.
    */
    switch( r->op ) {
    case PLUS:
    case MINUS:
	charcase = 1;
	break;
    case MUL:
    case LS:
	rl = r->left;
	if ((rl->op == PLUS || rl->op == MINUS) && OP_ISNNCON(rl->right)) {
	    charcase = 0;
	    break;
	}
	return( p );			/* no match */
    default:				/* other operators */
	return( p );			/* no match */
    }

    if (! OP_ISNNCON(r->right))
	return( p );

    /* Right side matches.  Check the left sides for array or ptr cases. */

    if (   (l = p->left)->op == ICON
	|| (l->op == UNARY AND && l->left->op == NAME)
    ) {
	/* Array case */
	if (charcase) {
	    ODEBUG(1, p, "Begin char array index rewrite");

	    rl = r->left;		/* the non-constant expression */
	    r->left = l;
	    r->flags &= ~FF_WASOPT;	/* allow further optimization */
	    p->left = r;
	    p->left->type = p->type;	/* pointer arithmetic */
	    p->right = rl;

	    ODEBUG(1, p, "End char array index rewrite");
	    p->left = optimize(p->left); /* do the folding we want */
	}
	else {
	    ODEBUG(1, p, "Begin array index rewrite");

	    /* Patch up right side first. */
	    r->left = rl->left;		/* rl still has +|- tree */

	    /* Do left side tree. */
	    rl->left = l;
	    l = p->left = rl;
	    l->flags &= ~FF_WASOPT;		/* do further optimization */
	    l->type = p->type;		/* pointer arithmetic */

	    new = t1alloc();		/* will be new *|<< node */
	    *new = *r;
	    new->flags &= ~FF_WASOPT;	/* will want to optimize again */
	    new->left = rl->right;
	    l->right = new;
	    new = new->right = t1alloc();	/* copy of C2 */
	    *new = *(r->right);

	    ODEBUG(1, p, "End array index rewrite");
	    p->left = optimize(p->left);
	}
    }
    else {
    /* Assume pointer cases. */
	if (charcase) {
	    ODEBUG(1, p, "Begin char pointer index rewrite");

	    rl = r->left;		/* the non-constant expression */
	    r->left = p;
	    r->type = p->type;		/* pointer arithmetic */
	    p->right = rl;
	    p = r;

	    ODEBUG(1, p, "End char pointer index rewrite");
	}
	else {
	    /* More complicated cases. */

	    ODEBUG(1, p, "Begin pointer index rewrite");

	    /* Create final left-side tree. */
	    r->left = rl->left;		/* rl still has +|- tree */

	    /* Current p eventually becomes left side.  Put together
	    ** new p and right side.
	    */
	    /* Build new p. */
	    l = p;
	    p = t1alloc();
	    p->op = rl->op;
	    p->flags = 0;
	    p->type = l->type;
	    p->left = l;
	    p->right = rl;

	    /* Put together new MUL/LS tree with both operands. */
	    rl->left = rl->right;
	    new = t1alloc();		/* will be new ICON scale node */
	    *new = *(r->right);
	    rl->right = new;
	    rl->op = r->op;
	    rl->flags &= ~FF_WASOPT;	/* to enable further optimization */

	    ODEBUG(1, p, "End pointer index rewrite");
	    p->right = optimize(p->right);
	}
    }
    return( p );
}
#endif


static ND1 *
op_mul( p )
ND1 * p;
/* Optimize MUL, ASG MUL node: do comm/assoc, MUL to LS optimizations. */
{
    ND1 * l = p->left;
    ND1 * r = p->right;
    int pow;

    /* Work with constants on the right. */
    if (   (l->op == ICON && r->op != ICON)
	|| (l->op == FCON && r->op != FCON)
    ) {
	p->left = r;
	p->right = l;
	l = p->left;
	r = p->right;
    }
    if (TY_ISVOLATILE(l->type))
	p->type = l->type;
    ODEBUG(1, p,"Optimizing MUL");
    /* C-tree:
    **		*				LS
    **	      /   \		=>	      /    \
    **	   NAME  ICON(pow^2)		    NAME   ICON(pow)
    */
    if (OP_ISNNCON(r) && (pow = op_ispow2(r->lval)) >= 0) {
	ODEBUG(1, p, "MUL to LS Optimization");
	if (pow == 0 && !(TY_ISVOLATILE(l->type) && asgop(p->op))) {
	    /* x*1 */
	    p->op = FREE;
	    r->op = FREE;
	    return( l );
	}
	if (pow > 0) {
	    r->lval = pow;
	    p->op += (LS - MUL);
	}
    }

    /* Look for cascades of operators. */
    p = op_cascade(p);
    return (p);
}


static int
op_ispow2( c )
register CONVAL c;
/* Determine if constant c a power of 2. */
{
    register i;
    if ( c <= 0 || (c&(c-1)) )  return(-1);
    for (i=0; c>1; ++i) c >>= 1;
    return (i);
}

static ND1 *
op_bitfield(p)
ND1 * p;
/* op_bitfield() rewrites DOT and STREF subtrees whose member of
** struct/union referenced is a bitfield.  p is the DOT or STREF
** subtree.
*/
{
    BITOFF offset, off, size, align;
    ND1 * l = p->left;
    ND1 * r = p->right;
    T1WORD rtype = r->type;

#ifdef ALFIELD
    align = ALFIELD;
#else
    align = TY_ALIGN(rtype);
#endif
    /* Rewriting the bitfield DOT or STREF subtrees is different
    ** in that the offset and size of the bitfield is important.
    ** The macro SY_FLDUPACK returns the size and offset of the
    ** first argument which is the SID.
    */
    SY_FLDUPACK(r->rval,size,offset);	/* get size and offset */
    r->op = FREE;
    off = (offset/align) * align; 

    if (off != 0){
	/* Build pointer plus. */
	p->op = PLUS;
	p->flags &= ~FF_ISVOL;
	p->right = tr_icon((CONVAL) BITOOR(off));
	offset -= off;
    }
    else {
	p->op = FREE;
	p = l;
    }
    p->type = ty_mkptrto(rtype);
    p = tr_generic(STAR, p, rtype);
    ODEBUG(1, p, "rewrite of bitfield reference");
    p = optimize( p );
    /* If the field has the size/alignment of an int/long, don't
    ** bother building a FLD node.  (Many CG's have a bug with
    ** full-word bit-fields.)  Remember that bit-fields have
    ** integral type.  (No check of type required.)
    */
    if (! (    size >= TY_SIZE(TY_INT)
	    && size == TY_SIZE(rtype)
	    && (off % align) == 0
	  )
    ) {
	p = tr_generic(FLD, p, p->type);
	p->rval = PKFIELD(size,offset);
    }
    return ( p );
}


static ND1 *
op_su(p, newop)
ND1 * p;
int newop;
/* Rewrite structure/union-related operations to explicit structure operation
** newop.  Structure/union operands become pointer to same.  Type of
** operations also becomes pointer to structure/union.  Stick a STAR
** above the new operator to retain type correctness of this subtree.
*/
{
    T1WORD otype = p->type;		/* original type */
    T1WORD tpsu = ty_mkptrto(otype);	/* new type for operands, op */
    int vol_flag = 0;			/* save volatile type status */

    switch( newop ){
    case STASG:
	/* On assignment, the type of the root is the left type.
	** When the right side is volatile, this info can be
	** lost unless we save it here.
	*/
	if (TY_ISVOLATILE(p->right->type)) vol_flag = 1;
	p->right = op_uand(p->right, tpsu);
    /* Don't put U& over RNODE. */
    if (p->left->op != RNODE) {
    /*FALLTHRU*/
    case STARG:
	p->left = op_uand(p->left, tpsu);
    }
	/* if the struct is volatile or a member within is vol, set field */
	if (TY_ISVOLATILE(otype) 
	    || vol_flag
	    || TY_ISMBRVOLATILE(otype))
	        p->flags |= FF_ISVOL;
    /*FALLTHRU*/
    case STCALL:
    case UNARY STCALL:
	/* Set new op and type for original node. */
	p->op = newop;
	p->type = tpsu;
	p->sttype = TY_DECREF(tpsu);	/* tuck away type in sttype */
	if (newop != STARG)		/* Preserve semantics, except for arg. */
	    p = tr_generic(STAR, p, otype);
	break;

    default:
	cerror("confused op_su(), op %d", newop);
    }
    return( p );
}


static ND1 *
op_conv(p)
ND1 * p;
/* optimize CONV nodes */
{
    static ND1 * op_adjust();
    ND1 * l = p->left;
    T1WORD ptype = p->type;
    T1WORD ltype = l->type;

    ODEBUG(0, p, "op_conv works on");
    /* free up CONV node now if both types are the same */
    if (TY_EQTYPE(ptype, ltype) == 1){
#ifdef FP_EXTENDED
	/* All fp arithmetic done in extended precision.  A
	** cast must cause the expression to lose precision.
	** If the cast is over a constant, optimize anyway since
	** the aritmetic is done in constant folding.
	*/
	if (	(TY_TYPE(ptype) == TY_FLOAT || TY_TYPE(ptype) == TY_DOUBLE)
	     && (p->flags & FF_ISCAST)
	     && (l->op != ICON)
	     && (l->op != FCON)
	   )
	    return(p);
#endif
	p->op = FREE;
	ODEBUG(0, l,"equal types, result tree");
	return(l);
    }

    /* Treat pointer type like int type if one is going to be
    ** converted to the other type anyway.  Rely on other transforms
    ** to clean up.
    */
    if (TY_ISINTTYPE(ptype) && TY_ISPTR(ltype))
	ltype = T_ptrtype;
    else if (TY_ISPTR(ptype) && TY_ISINTTYPE(ltype))
	ptype = T_ptrtype;

    /* If integral CONV is over another integral and they are
    ** the same size, free the CONV, paint over type.
    ** Or pointer CONV is over another pointer, do the same.
    */
    if (   (   (TY_ISINTTYPE(ptype) && TY_ISINTTYPE(ltype))
	    || (TY_ISPTR(ptype) && TY_ISPTR(ltype))
	   )
	&& TY_SIZE(ltype) == TY_SIZE(ptype)
	&& l->op != CONV
    ) {
	p->op = FREE;
	l->type = p->type;
	ODEBUG(0, l, "CONV over same size integral, result tree");
	return(l);
    };

    if (TY_ISPTR(ltype) || TY_ISARY(ltype)) return (p);

    switch(l->op){
    case ICON:
	if (TY_ISFPTYPE(ptype)){
	    FP_DOUBLE resval;
	    int isunsigned = TY_ISUNSIGNED(l->type);

	    if (TY_TYPE(ptype) == TY_LDOUBLE)
		cg_ldcheck();		/* check long double warning */

	    /* call the appropriate integral to double routine */
	    resval = isunsigned ?
		FP_ULTOD((UCONVAL) l->lval) : FP_LTOD(l->lval);

	    /* truncate to float if that's what we need */
	    if (TY_TYPE(ptype) == TY_FLOAT)
		resval = op_dtofp(resval);

	    /* Can make change if for initializer or if result is exact. */
	    errno = 0;
	    if (!op_forinit) {
		if (isunsigned) {
		    if (op_dtoul(resval) != (UCONVAL) l->lval || errno != 0)
			break;		/* no change */
		}
		else if (op_dtol(resval) != l->lval || errno != 0)
		    break;		/* no change */
	    }
	    l->op = FCON;
	    l->dval = resval;
	}
	goto conv_paint;
    case FCON:
        /* a CONV over an FCON can be removed (for initializers only). */
	if (TY_ISFPTYPE(ptype)){	/* float, double, or long double */
/* Conversions to and from long double will need more consideration here
** if long doubles are not treated as doubles.
*/
	    if (TY_TYPE(ptype) == TY_LDOUBLE)
		cg_ldcheck();		/* check long double warning */

	    if (TY_TYPE(ptype) == TY_FLOAT) {
		/* shrink doubles and long doubles to float */
		FP_DOUBLE resval = op_dtofp(l->dval); 
		if (errno) {
		    static const char mesg[] =
			"conversion of double to float is out of range"; /*ERROR*/
		    if (op_forinit)
			UERROR(mesg);
		    else
			WERROR(mesg);
		}
		/* If not for initializer, can only do this if value
		** is unchanged.
		*/
		if (! op_forinit && FP_CMPD(resval, l->dval) != 0)
		    break;
		l->dval = resval;
	    }
	} 
	else {	/* convert all else to long or unsigned long */
	    int isunsigned = TY_ISUNSIGNED(ptype);
	    UCONVAL ulval;
	    CONVAL lval;


	    if (isunsigned)
		ulval = op_dtoul(l->dval);
	    else
		lval = op_dtol(l->dval);

	    /* Only do conversion for non-initializers if there's no
	    ** change in value.
	    */
	    if (errno) {
		static const char mesg[] =
		    "conversion of double to integral is out of range"; /*ERROR*/
		if (op_forinit)
		    UERROR(mesg);
		else {
		    WERROR(mesg);
		    break;
		}
	    }
	    /* Conversion of FP type to int always chops, so rounding
	    ** modes don't figure in here.  Can always do it.
	    */

	    l->op = ICON;
	    l->lval = isunsigned ? ulval : lval;
	    l->rval = ND_NOSYMBOL;
	}
	goto conv_paint;
    case CONV:
	/* These only apply to arithmetic types. */
	if (! (   TY_ISNUMTYPE(ptype)
	       && TY_ISNUMTYPE(ltype)
	       && TY_ISNUMTYPE(l->left->type)
	       )
	    )
	    break;

	/*  Let's look at three levels of the tree where the
	**  top two are CONV over CONV.  With the three levels
	**  we are concerned about the type sizes. In the
	**  following four cases the middle node may be tossed out:
	**
	**  	    CASE 1	CASE 2	    CASE 3		CASE 4
	** Level
	**   1	    Small	Medium	    Large		Large
	**   	      |		  |	      |			  |
	**   2	    Large	Large	    Medium[same sign]   Medium[not unsigned]
	**	      |		  |	      |			  |
	**   3	    Medium	Small	    Small[same sign]	Small[unsigned]
	*/
	do {
	    T1WORD lltype;

	    if (op_tysize(ltype) <= op_tysize(l->left->type))
		break;
	    lltype = l->left->type;
	    if (op_tysize(ptype) <= op_tysize(ltype)) {
		/* Cases 1, 2.  Floating point-ness of at least one
		** pair of adjacent operands must be the same.
		*/
		if (TY_ISFPTYPE(ptype) == TY_ISFPTYPE(ltype))
		    /*EMPTY*/ ;
		else if (TY_ISFPTYPE(ltype) == TY_ISFPTYPE(lltype))
		    /*EMPTY*/ ;
		else
		    break;
	    }
	    else {
		/* Check cases 3, 4. */
		if (TY_ISUNSIGNED(ltype) == TY_ISUNSIGNED(lltype))
		    /*EMPTY*/ ;
		else if (!TY_ISUNSIGNED(ltype) && TY_ISUNSIGNED(lltype))
		    /*EMPTY*/ ;
		else
		    break;
	    }
	    /* All conditions met.  Toss out middle node, optimize. */
	    l->op = FREE;
	    p->left = l->left;
	    p = optimize(p);
	/*CONSTANTCONDITION*/
	} while(0);
	ODEBUG(0, p, "result of CONV over CONV optimization");
	break;
    case STAR:
    {
	OFFSET adj;

	if (! (   TY_ISINTTYPE(ptype)
	       && TY_ISINTTYPE(ltype)
	       && op_tysize(ptype) <= op_tysize(ltype)
	       )
	    )
	    return( p );
	/* get adjustment in bits of the two types */
	adj = cg_off_conv(NAME, 0, ltype, ptype) * TY_SIZE(TY_CHAR);
	if (cg_off_is_err(NAME,adj))
	    return ( p );
	if (adj)
	    l->left = op_adjust(l->left,adj);
	goto conv_paint;
    }
    case NAME:
    {
	int space;
	OFFSET newoff;

	if (! (   TY_ISINTTYPE(ptype)
	       && TY_ISINTTYPE(ltype)
	       && op_tysize(ptype) <= op_tysize(ltype)
	       )
	    )
	    return( p );

	/* Can't deal with register variables that are in registers. */
	if (   (SY_FLAGS(l->rval) & SY_ISREG) != 0
	    && SY_REGNO(l->rval) != SY_NOREG
	    )
	    return( p );

	/* Select correct address space in which to do address conversion. */
	switch( SY_CLASS(l->rval) ) {
	case SC_EXTERN:
	case SC_STATIC:		space = NAME;	break;
	case SC_AUTO:		space = VAUTO;	break;
	case SC_PARAM:		space = VPARAM; break;
	default:
	    cerror("strange NAME in op_conv()");
	}
	
#ifdef	FAT_ACOMP			/* temporary */
	if (space != NAME)
	    return( p );
#endif
	newoff = cg_off_conv(space, l->lval, ltype, ptype);
	if (cg_off_is_err(space, newoff))
	    return (p); 
	l->lval = newoff;
	goto conv_paint;
    } /* end NAME case */
    }	/* end switch */
    return (p);

conv_paint: ;
    if (l->op == ICON)
#ifdef LINT
    {	CONVAL old_lval = l->lval;
#endif
        l->lval = tr_truncate(l->lval, ptype);
#ifdef LINT
	/*
	** The value of this ICON is no longer the same; set FF_TRUNC so
	** lint will know that the constant was truncated.
	*/
	if (old_lval != l->lval)
	    l->flags |= FF_TRUNC;
    }
#endif
    l->type = ptype;
    p->op = FREE;
    return (l);
}

FP_DOUBLE
op_dtofp(d)
FP_DOUBLE d;
/* Convert double d to a double whose value has been truncated
** to float precision.  Set errno if there was an error while
** doing so.  Return the truncated-precision result.
*/
{
    errno = 0;
    if (setjmp(fpe_buf) != 0) {
	/* Set a value that can be used safely as a float later. */
	d = FP_LTOD(0L);
	goto fpe_err;			/* errno set */
    }
    save_fpefunc(op_fpe_handler);
    d = FP_DTOFP(d);
fpe_err:;
    save_fpefunc((void (*)()) 0);
    return( d );
}

/* Different machines are too unreliable with respect to
** whether they trap on conversions of double to integral
** types.  Simulate overflows.
*/

static UCONVAL
op_dtoul(d)
FP_DOUBLE d;
/* Convert FP_DOUBLE to unsigned long (UCONVAL) by truncation. */
{
    static int beenhere = 0;
    static FP_DOUBLE ulong_max_p_1;
    static FP_DOUBLE long_max;
    static FP_DOUBLE zero;
    UCONVAL ulval;

    if (!beenhere) {
	ulong_max_p_1 =
		FP_PLUS( FP_ULTOD((unsigned long) T_ULONG_MAX), FP_LTOD(1L) );
	long_max = FP_ULTOD((unsigned long) T_LONG_MAX);
	zero = FP_LTOD(0L);
	beenhere = 1;
    }
    errno = 0;

    /* Because we're doing truncation, anything strictly less than
    ** ULONG_MAX+1 is okay.
    */
    if (FP_CMPD(d, ulong_max_p_1) >= 0) {
	errno = ERANGE;
	ulval = (unsigned long) T_ULONG_MAX;
    }
    else if (FP_CMPD(d, zero) < 0) {
	errno = ERANGE;
	ulval = 0L;
    }
    else {
	/* This code is messier than necessary because lots of
	** compilers generate bad code for converting doubles
	** to unsigned longs, particularly when the double is
	** > T_LONG_MAX.  Do the conversion by successive
	** reduction instead.
	*/
	ulval = 0;
	while (FP_CMPD(d, long_max) > 0) {
	    ulval += T_LONG_MAX;
	    d = FP_MINUS(d, long_max);
	}
	ulval += FP_DTOUL(d);
    }
    return( ulval );
}

static CONVAL
op_dtol(d)
FP_DOUBLE d;
/* Convert double to long.  Rather than depend on trapping on
** host to get this right, do bounds checking explicitly.  Only
** use host double-to-long conversion when the value is in bounds.
** Return the converted value.
*/
{
    static int beenhere = 0;
    static FP_DOUBLE long_min_m_1;
    static FP_DOUBLE long_max_p_1;
    CONVAL lval;

    /* The (open) interval we want to check is
    **		(LONG_MIN-1, LONG_MAX+1)
    */
    if (!beenhere) {
	long_max_p_1 = FP_PLUS( FP_LTOD(T_LONG_MAX), FP_LTOD(1L) );
	long_min_m_1 = FP_MINUS( FP_LTOD(T_LONG_MIN), FP_LTOD(1L) );
	beenhere = 1;
    }

    errno = 0;
    if (FP_CMPD(d, long_min_m_1) <= 0) {
	errno = ERANGE;
	lval = T_LONG_MIN;
    }
    else if (FP_CMPD(d, long_max_p_1) >= 0) {
	errno = ERANGE;
	lval = T_LONG_MAX;
    }
    else
	lval = FP_DTOL(d);
    return( lval );
}


static ND1 *
op_adjust(p, adj)
ND1 * p;
OFFSET adj;
{
    ND1 * q;

    switch(p->op){
    case ICON:
        p->lval = cg_off_incr(ICON,p->lval,adj);  
	return(p);
    case PLUS:
    case MINUS:
        q = p->right;
        if (q->op != ICON) goto mkplus;
        if (p->op == MINUS)
	    adj =  -adj;
	q->lval = cg_off_incr(ICON, q->lval, adj);
	return (p);
    default:
mkplus: ;
        {   
            ND1 * new = tr_newnode(PLUS);

	    /* constuct a + node */
	    new->left = p;
	    new->right = tr_icon(BITOOR(adj));
	    new->type = p->type;
	    return(new);
	}
    }  /* end switch */
}

static ND1 *
op_left_con(p)
ND1 * p;
/* The left side of p is an ICON or FCON.  Do any reasonable
** optimizations thereupon.  Must maintain fixed condition
** that left side of tree has been optimized.  Don't change
** multiplies to shifts yet:  if there's overflow, we would
** show the wrong op.
*/
{
    ND1 * l = p->left;
    int changed = 0;

    if (optype(p->op) == BITYPE && TY_ISVOLATILE(p->right->type))
	return( p );

    /* Skim off FCON's.  If op is ANDAND, OROR, QUEST, turn the FCON
    ** into an ICON.  Otherwise quit.
    */
    if (l->op == FCON) {
	switch( p->op ){
	case ANDAND:
	case OROR:
	case QUEST:
	    l->lval = ! FP_ISZERO(l->dval);
	    l->op = ICON;
	    l->type = TY_INT;
	    l->rval = ND_NOSYMBOL;
	    break;
	default:
	    return( p );
	}
    }

    if (OP_ISNNCON(l) && l->lval == 0) { /* Consider ops with left side zero. */
	switch( p->op ) {
	/* For these ops, left side can be discarded. */
	case PLUS:
	case OR:
	case ER:
	    l->op = FREE;
	    p->op = FREE;
	    p = p->right;
	    changed = 1;
	    break;

#ifndef	LINT				/* LINT wants to see these */
	/* For these ops, result is zero. */
	case ANDAND:
	case LS:
	case RS:
	    t1free(p->right);
	    p->op = FREE;
	    p = l;
	    p->type = TY_INT;		/* in case it was pointer */
	    break;
#endif
	
	/* For these ops, result is zero, but we may need to do right
	** side for side effects.  Build ,OP.
	*/
	case MUL:
	case AND:
	{
	    ND1 * temp = p->right;

	    p->op = COMOP;
	    p->left = temp;
	    p->right = l;
	    changed = 1;
	    break;
	}

	/* 0-x => -x */
	case MINUS:
	    p->op = UNARY MINUS;
	    l->op = FREE;
	    p->left = p->right;
	    p->right = ND1NIL;
	    changed = 1;
	    break;
	
	/* 0 in ?: */
	case QUEST:
	{
	    ND1 * colon = p->right;

	    l->op = FREE;
	    p->op = FREE;
	    p = colon->right;
	    p->type = colon->type;
	    t1free(colon->left);
	    colon->op = FREE;
	    changed = 1;
	    break;
	}

	/* Turn conditional branch into absolute branch. */
#if 0	/* not necessary here */
	case CBRANCH:
	    p->op = JUMP;
	    l->op = FREE;
	    p->label = p->right->lval;	/* fetch label from presumed ICON */
	    p->right->op = FREE;
	    break;
#endif
	} /* end switch for zero cases */
    } /* end zero constant cases */
    else {
	switch( p->op ){
	/* Result is 1. */
#ifndef	LINT			/* LINT wants to see this */
	case OROR:
	    t1free(p->right);
	    p->op = FREE;
	    p = l;
	    p->lval = 1;
	    p->type = TY_INT;		/* in case it was pointer */
	    p->rval = ND_NOSYMBOL;
	    break;
#endif
	
	case ANDAND:			/* Change (possible) address
					** constant to vanilla constant.
					*/
	case NOT:
	    l->lval = 1;
	    l->rval = ND_NOSYMBOL;
	    l->type = TY_INT;
	    break;
	/* Choose left side of ?: */
	case QUEST:
	{
	    ND1 * colon = p->right;
	    p->left->op = FREE;
	    p->op = FREE;
	    p = colon->left;
	    p->type = colon->type;
	    colon->op = FREE;
	    t1free(colon->right);
	    changed = 1;
	    break;
	}
#ifndef	LINT			/* LINT wants to see this */
	/* CBRANCH becomes NOP if operand is non-0. Assume an
	** ICON in this context will behave like a no-op.
	** (Don't use that op code to accommodate SPARC.)
	*/
	case CBRANCH:
	    p->op = ICON;
	    l->op = FREE;
	    p->right->op = FREE;
	    p->lval = 0L;
	    p->rval = ND_NOSYMBOL;
	    break;
#endif
	} /* end switch on non-zero cases */
    } /* end non-zero cases */

    /* if p is a new node, we must keep the left subtree optimized */
    if (changed && optype(p->op) != LTYPE) {
	p->left = optimize(p->left);
	if (p->op == COMOP && (p->left->op == ICON || p->left->op == FCON)) {
	    /* Discard ,OP's that arise in constant expressions. */
	    p->left->op = FREE;
	    p->op = FREE;
	    p = p->right;
	}
	if (p->op == QUEST && (p->left->op == ICON || p->left->op == FCON)) {
	    p = op_left_con(p);		/* Does not get done in optimize() */
	}
    }
    return( p );
}


static ND1 *
op_right_con(p)
ND1 * p;
/* Optimize stuff with a constant on the right side
** and return modified tree.  Don't change multiplies
** to shifts yet:  if there's overflow, we show the
** wrong op.
** Don't do anything when the right side is an FCON.
** Assume the left side is fully optimized already.
*/
{
    ND1 * r = p->right;
    static const char div0[] = "division by 0";	/*ERROR*/
    static const char mod0[] = "modulus by 0";	/*ERROR*/
    T1WORD objtype;
		
    if (TY_ISVOLATILE(p->left->type))
	return( p );

    /* Handle FCON's specially. */
    if (r->op == FCON) {
	switch(p->op) {
	case DIV:
	case ASG DIV:
	    if (FP_ISZERO(r->dval)) {
		if (op_forinit)
		    UERROR(div0);
		else
		    WERROR(div0);
	    }
	    break;
	case ANDAND:
	case OROR:
	    r->lval = ! FP_ISZERO(r->dval);
	    r->op = ICON;
	    r->type = TY_INT;
	    r->rval = ND_NOSYMBOL;
	    break;
	}
    }
    else if (OP_ISNNCON(r) && r->lval == 0) {
	switch( p->op ) {
	/* For these, zero is an identity. */
	case MINUS:
	case ASG MINUS:
	    /* For pointer subtraction particularly, result type is integral;
	    ** must change type of left operand.
	    */
	    p->left->type = p->type;
	    /*FALLTHRU*/
	case PLUS:
	case ASG PLUS:
	case OR:
	case ASG OR:
	case ER:
	case ASG ER:
	case LS:
	case ASG LS:
	case RS:
	case ASG RS:
	    /* If ASG op, remove a CONV on the left. */
	    if (asgop(p->op) && p->left->op == CONV) {
		p->left->op = FREE;
		p->left = p->left->left;
	    }
	    p->op = FREE;
	    p = p->left;
	    r->op = FREE;
	    break;

	/* For these, must do left side for effects, but result is zero.
	** Beware volatile type on left:  can't discard operation.
	** If we know the left side is a constant, we can discard it.
	*/
	case AND:
	case MUL:
	    if (p->left->op == ICON) {
		p->left->op = FREE;
		p->op = FREE;
		p = r;
	    }
	    else
		p->op = COMOP;
	    break;

	/* Result is zero.  Remove conversion on left of asgop,
	** if any, change ICON type.
	*/
	case ASG MUL:
	case ASG AND:
	    if (asgop(p->op) && p->left->op == CONV) {
		p->left->op = FREE;
		p->left = p->left->left;
	    }
	    p->op = ASSIGN;
	    p->right->type = p->type;
	    break;
	
	/* Division, modulus by zero. */
	case MOD:
	case ASG MOD:
	case DIV:
	case ASG DIV:
	{
	    /* Must leave the tree alone if executable, because the code
	    ** may never be reached.  (ANSI requires the module to compile
	    ** otherwise.)
	    */
	    const char * mesg =
		(p->op == DIV || p->op == ASG DIV) ? div0 : mod0;
	    if (op_forinit)
		UERROR(mesg);
	    else
		WERROR(mesg);
	    break;
	} /* end mod/div case */
	} /* end switch on zero cases */
    }
    else if (OP_ISNNCON(r)) {
	int pow2;

	/* Non-zero constants on right. */
	switch( p->op ) {
	case ASG MUL:
	case ASG DIV:
	case MUL:
	case DIV:
	    if (r->lval == 1) {
		r->op = FREE;
		if (asgop(p->op) && p->left->op == CONV) {
		    p->left->op = FREE;
		    p->left = p->left->left;
		}
		p->op = FREE;
		p = p->left;
		goto done;
	    }
	    break;

	/* Mod of 1 gives result of 0, since quotient is dividend.
	** Rewrite  x %= 1 as x = 0.  Rewrite x % 1 as x,0 to pick
	** up side effects if x is not a constant.
	** For signed mod, -1 behaves the same way.  (The remainder
	** is always 0.)
	*/
	case MOD:
	case ASG MOD:
	    if (r->lval == 1 || (TY_ISSIGNED(p->type) && r->lval == -1)) {
		r->lval = 0;
		if (p->op == MOD) {
		    if (p->left->op == ICON) {
			p->left->op = FREE;
			p->op = FREE;
			p = r;
			goto done;
		    }
		    else
			p->op = COMOP;
		}
		else {
		    if (p->left->op == CONV) {
			p->left->op = FREE;
			p->left = p->left->left;
		    }
		    p->op = ASSIGN;
		    p->right->type = p->type;
		}
	    }
	    break;
	case LS:
	case ASG LS:
	case RS:
	case ASG RS:
	{
	    objtype = p->type;

	    /* Check for negative shift count and count too big. */
	    if (asgop(p->op) && p->left->op == CONV)
		objtype = p->left->left->type;

	    /* Assumes 2's complement! */
	    if ( (UCONVAL) r->lval >= TY_SIZE(objtype)) {
		WERROR("shift count negative or too big: %s %ld",
		    opst[p->op], r->lval);
		if (r->lval < 0)
		    r->lval = 0;		/* avoid cascading messages */
	    }
	}
	    break;
	} /* end switch of operators with non-zero constants */

	/* Do other conversions if right side is power of 2. */
	if ((pow2 = op_ispow2(r->lval)) >= 0) {
	    objtype = p->left->type;
	    if (asgop(p->op) && p->left->op == CONV)
		objtype = p->left->left->type;

	    switch( p->op ) {
	    /* Turn unsigned divisions into shifts. */
	    case DIV:
	    case ASG DIV:
		if (TY_ISUNSIGNED(objtype)) {
		    /* Make a right shift. */
		    p->op += (RS - DIV);
		    r->lval = pow2;
		}
		break;
	    /* Turn modulus into AND. */
	    case MOD:
	    case ASG MOD:
		if (TY_ISUNSIGNED(objtype)) {
		    p->op += (AND - MOD);
		    r->lval = ((unsigned long) 1 << pow2) - 1;
		}
		break;
	    }
	} /* end power of 2 optimizations */
    } /* end non-zero constant on right */
    else {
	/* Address constant on right. */
	switch( p->op ){
	case ANDAND:
	case OROR:
	    /* Turn these into vanilla integer constants with value 1. */
	    r->rval = ND_NOSYMBOL;
	    r->lval = 1;
	    r->type = TY_INT;
	    break;
	}
    }
done:;
    return( p );
}


static ND1 *
op_iconfold(p)
ND1 * p;
/* Perform constant folding on integer constants (ICONs).
** p contains the operator, with left and right (when binary)
** ICON operand leaf nodes.
 */
{
    ND1 * l = p->left;
    ND1 * r = p->right;
    CONVAL lval = l->lval;
    int o = optype(p->op);
    int u = TY_ISUNSIGNED(p->type);

    ODEBUG(0, p,"In op_iconfold(), attempt to fold tree");

    /* This constant folding code assumes that the target machine
    ** is of a two's complement architecture type.
    */

    /*		op 
    **	       /   	=>	ICON(with operator applied)
    **	    ICON 
    */
    if (o == UTYPE){		/* constant folding for unary ops */
	if (l->rval != ND_NOSYMBOL)
	    return (p);

	/* Check for overflow in signed integral operations. */
	if (u || !op_int_oflow(p)) {
	    switch( p->op ){
	    case UNARY MINUS:	l->lval = -(lval); break;
	    case NOT:		l->lval = !(lval); break;
	    case COMPL:		l->lval = ~lval; break;
	    case UPLUS:		break;
	    default:		return (p);
	    }
	}
    }
    else	/* a binary operator */
    /*		
    **		op
    **	       /  \	=>	ICON (operation applied)
    **	    ICON  ICON
    */
    {
        CONVAL rval = r->lval;		/* set rval now we know r is not ND1NIL */
	/* pointer subtraction is okay */
	if ( l->rval != ND_NOSYMBOL && r->rval == l->rval && p->op == MINUS)
	    l->rval = ND_NOSYMBOL;
	/* weed out non-folding cases */
	else {
	    if (l->rval != ND_NOSYMBOL && r->rval != ND_NOSYMBOL) return (p);
	    if (r->rval != ND_NOSYMBOL && p->op != PLUS) return (p);
	    if (l->rval != ND_NOSYMBOL && p->op != MINUS && p->op != PLUS) return (p);
	}

	if (u || !op_int_oflow(p)) {
	    switch( p->op ) {
	    case PLUS:		
		l->lval += rval; 
		if (r->rval != ND_NOSYMBOL)	/* l->rval == ND_NOSYMBOL */
		    l->rval = r->rval;
		break;
	    case MINUS:		l->lval -= rval; break;
	    case MUL:		l->lval *= rval; break;
	    case DIV:
		/* Divide by zero already diagnosed.  Treat as zero. */
		if (rval == 0) {
		    if (op_forinit)
			l->lval = 0;
		    else
			return( p );	/* leave tree alone */
		}
		else if ( u )		/* unsigned */
		    l->lval = (UCONVAL) l->lval / rval;
		else
		    l->lval /= rval;
		break;
	    case MOD:
		/* Modulus by zero already diagnosed.  Treat as zero. */
		if (rval == 0) {
		    if (op_forinit)
			l->lval = 0;
		    else
			return( p );	/* leave tree alone */
		}
		else if ( u )		/* unsigned */
		    l->lval = (UCONVAL) lval % rval;
		else
		    l->lval %= rval;
		break;
	    case EQ:		l->lval = (lval == rval); break;
	    case NE:		l->lval = (lval != rval); break;
	    case LT:		l->lval = (lval < rval); break;
	    case LE:		l->lval = (lval <= rval); break;
	    case GT:		l->lval = (lval > rval); break;
	    case GE:		l->lval = (lval >= rval); break;
	    case ULT:		l->lval = (UCONVAL) lval < (UCONVAL) rval; break;
	    case ULE:		l->lval = (UCONVAL) lval <= (UCONVAL) rval; break;
	    case UGT:		l->lval = (UCONVAL) lval > (UCONVAL) rval; break;
	    case UGE:		l->lval = (UCONVAL) lval >= (UCONVAL) rval; break;
	    case ANDAND:        l->lval = (lval && rval); break;
	    case OROR:		l->lval = (lval || rval); break;
	    case AND:		l->lval &= rval; break;
	    case OR:		l->lval |= rval; break;
	    case ER:		l->lval ^= rval; break;
	    case LS:		l->lval <<= rval; break;
	    case RS:
	    {
		UCONVAL tval = (UCONVAL) l->lval >> rval;
#ifdef	C_SIGNED_RS
	    /* For machines with signed right shifts, simulate the
	    ** signed shift:  if the left operand is negative, OR
	    ** in shifted sign bits.
	    */
				if (!u && l->lval < 0 && rval > 0)
				    tval |= ~( (~(UCONVAL) 0) >> rval );
#endif
				l->lval = (CONVAL) tval;
				break;
	    }
	    default:		return (p);
	    }
	}
	p->right->op = FREE;
    } /* end if-else */

    l->lval = tr_truncate(l->lval,p->type);
    l->type = p->type;		/* in case of pointer subtraction */
    p->op = FREE; 
    ODEBUG(0, l,"Successful fold, new tree is:");
    return( l );
} 

static int
op_int_oflow(p)
ND1 * p;
/* Check for overflow in integral operations during constant
** folding.  p is the operation tree.  The operation is assumed
** to be for a signed integral type.  The operands are presumed
** to be ICON's or NAME's (pointer plus/minus constant case).
** Return 1 if a warning was issued; p->left->lval has the
** value folded as if the operands were unsigned (to avoid
** overflows).  Otherwise return 0; all nodes are unchanged.
** Return 0 if no overflow, or the appropriate value as if the
** constants had been folded unsigned (to avoid possible traps).
*/
{
    ND1 * r = (optype(p->op) == UTYPE) ? p->left : p->right;

    if (op_int_ocheck(p->op, p->left->lval, r->lval, &p->left->lval)) {
	WERROR("integer overflow detected: op \"%s\"", opst[p->op]);
	return( 1 );
    }
    return( 0 );
}


static int
op_int_ocheck(op, lval, rval, presult)
int op;
CONVAL lval;
CONVAL rval;
CONVAL * presult;
/* Just do the check for integer overflow.  op is the operation,
** lval and rval are the operands.  presult points to the place to store
** the result, but only if overflow would have occurred.  If it's zero,
** no value is stored.  (This result is the replacement result in lieu
** of doing the calculation.)  Return 0 if the calculation does not
** overflow, 1 if it does.
*/
{
    
    int warn = 0;			/* set to 1 to warn */
    UCONVAL retval = 0;

    DEBUG(o1debug, ("In op_int_ocheck()\n"));

    switch( op ){			/* look at operations of interest */
#if H_CON_MIN < -H_CON_MAX
    case UNARY MINUS:
	if (lval <= H_CON_MIN)
	    retval = (UCONVAL) H_CON_MIN;
	break;
#endif
    
    case PLUS:
	if (rval >= 0) {
	    if (lval > (H_CON_MAX - rval))
		warn = 1;
	}
	else {				/* rval is negative */
	    /* Beware possible overflow in -rval. */
#if 0
	    if (lval < (H_CON_MIN - rval))
#endif
	    if (lval < -((rval + H_CON_MAX) - (H_CON_MAX + H_CON_MIN)))
		warn = 1;
	}
	if (warn)
	    retval = (UCONVAL) lval + (UCONVAL) rval;
	break;
    case MINUS:
	if (rval <= 0) {
	    if (lval > (H_CON_MAX + rval))
		warn = 1;
	}
	else {				/* rval is positive */
	    if (lval < (H_CON_MIN + rval))
		warn = 1;
	}
	if (warn)
	    retval = (UCONVAL) lval - (UCONVAL) rval;
	break;
    case MUL:
	/* Be careful to avoid H_CON_MIN/(-1), which could overflow,
	** or H_CON_MIN/0.
	*/
	if (lval > 0) {
	    if (rval >= 0) {
		if (rval > H_CON_MAX/lval)
		    warn = 1;
	    }
	    else if (rval < H_CON_MIN/lval)
		warn = 1;
	}
	else if (lval < 0) {
	    if (rval >= 0) {
		if (lval < H_CON_MIN/rval)
		    warn = 1;
	    }
	    else if (lval < H_CON_MAX/rval)
		warn = 1;
	}
	if (warn)
	    retval = (UCONVAL) lval * (UCONVAL) rval;
	break;
#if H_CON_MIN < -H_CON_MAX
    case DIV:
	/* Special case, 2's complement, H_CON_MIN/-1 */
	if (lval == H_CON_MIN && rval == -1)
	    retval = (UCONVAL) H_CON_MIN;
	break;
#endif
    case LS:
	/* Check whether bits would be shifted into sign bit or
	** lost altogether.
	*/
	if (rval > 0) {
	    if (rval >= TY_SIZE(TY_LONG))
		warn = 1;
	    else {
		/* mask gives bits that must be all 0 or all 1.
		** Beware if rval+1 == TY_SIZE(TY_LONG):  undefined.
		** Shift by rval+1 the hard way.
		*/
		UCONVAL mask = ~(~((UCONVAL) 0L) >> rval);
		mask >>= 1;

		if (lval >= 0) {
		    if (((UCONVAL) lval & mask) != 0)
			warn = 1;
		}
		else if ((((UCONVAL) lval) & mask) != mask)
		    warn = 1;
		if (warn)
		    retval = lval << rval;
	    }
	}
	break;
    default:
	break;				/* other ops okay */
    }
    if (warn || retval) {
	if (presult)
	    *presult = (CONVAL) retval;
	return( 1 );
    }
    return( 0 );
}


static ND1 *
op_fconfold(p)
ND1 * p;
/* Perform constant folding on floating point constants.
** p contains the operator, with left and right (when binary)
** FCON operand leaf nodes.
 */
{
    ND1 * l = p->left;
    FP_DOUBLE lval = l->dval;
    static const char mesg[] =
	"floating-point constant calculation out of range: op \"%s\""; /*ERROR*/
    int fpresult = 0;			/* 1 if op has FP result */
    
    errno = 0;		/* check float/double range error */
    if (optype(p->op) == UTYPE){	/* do we have a unary operand? */
	switch(p->op){
	case UNARY MINUS:	l->dval = FP_NEG(lval); break;
	case NOT:		l->lval = FP_ISZERO(lval); break;
	case UPLUS:		break;
	default:		return (p);
	}
    }
    else {	/* it is a binary operand */
	FP_DOUBLE rval = p->right->dval;

	if (setjmp(fpe_buf) != 0)
	    goto fpe_stuff1;
	save_fpefunc(op_fpe_handler);

	switch( p->op ) {
	case DIV:
	    fpresult = 1;		/* division gets FP result */
	    if (FP_ISZERO(rval)) {
		if (!op_forinit)
		    return( p );	/* leave tree alone */
		lval = FP_LTOD(0L);
	    }
	    else if (FP_ISZERO(lval))
		fpresult = 0;		/* hack to return left node */
	    else
		lval = FP_DIVIDE(lval, rval);
	    break;
	case PLUS:	fpresult = 1; lval = FP_PLUS(lval, rval); break;
	case MINUS:	fpresult = 1; lval = FP_MINUS(lval, rval); break;
	case MUL:	fpresult = 1; lval = FP_TIMES(lval, rval); break;
	case EQ:	l->lval = FP_CMPD(lval, rval) == 0; break;
	case NE:	l->lval = FP_CMPD(lval, rval) != 0; break;
	case LT:	l->lval = FP_CMPD(lval, rval) < 0; break;
	case LE:	l->lval = FP_CMPD(lval, rval) <= 0; break;
	case GT:	l->lval = FP_CMPD(lval, rval) > 0; break;
	case GE:	l->lval = FP_CMPD(lval, rval) >0 ; break;
#if 0	/* not reached because of op_right_con() */
	case ANDAND:
	    l->lval = !FP_ISZERO(lval) && !FP_ISZERO(rval);
	    break;
	case OROR:
	    l->lval = !FP_ISZERO(lval) || !FP_ISZERO(rval);
            break;
#endif
	default:
	    save_fpefunc((void(*)()) 0);	/* unset trap handler */
	    return (p);
	}
    }

fpe_stuff1: ;
    /* Could only get error on + - * / */
    if (errno) {
	if (op_forinit)
	    UERROR(mesg, opst[p->op]);
	else
	    WERROR(mesg, opst[p->op]);
    }
    save_fpefunc((void(*)()) 0);

    if (logop(p->op)){		/* clean out FCON info in logical op */
	l->op = ICON;
	l->type = TY_INT;
	l->rval = ND_NOSYMBOL;
    }
/* LATER: truncate long double to double or float */
    else if ( TY_TYPE(p->type) == TY_FLOAT ){
	lval = op_dtofp(lval);		/* truncate to float */
	if (errno)
	    WERROR("conversion of double to float is out of range");
    }

    /* Don't actually fold the floating point result unless it's for
    ** an initializer.  Otherwise, free up nodes that are no longer needed.
    */
    if (fpresult && op_forinit)
	l->dval = lval;
    if (! fpresult || op_forinit) {
	if (optype(p->op) == BITYPE)
	    p->right->op = FREE;
	p->op = FREE; 
	p = l;
    }
    return( p );
}

static ND1 *
op_uand( p, t)
ND1 *p;
T1WORD t;
/* build a UNARY AND node over p, but enforce optim tree-rewrite rules.
** Some operators require the UNARY AND to be distributed within.
*/
{
    ND1 * colonop;

    switch( p->op ) {
    case QUEST:
        /* Put & over each side of :, change type of : */
	colonop = p->right;		/* assume this is the : */
	colonop->left = op_uand(colonop->left, t);
	colonop->right = op_uand(colonop->right, t);
	colonop->type = t;
	p->type = t;
	break;
    case COMOP:
	/* Put & over right side. */
	p->right = op_uand(p->right, t);
	p->type = t;
	break;
    case NAME:
	/* Treat as if address taken (which it was). */
	if (p->rval != ND_NOSYMBOL)
	    SY_FLAGS(p->rval) |= SY_UAND;
	/* FALLTHRU */
    default:
	p = optimize( tr_generic(UNARY AND, p, t) );
    }
    return (p);
}

static ND1 *
op_call(p)
ND1 * p;
/* Try to recognize calls to particular built-in functions and
** convert to pure trees.  We know the left side of the callop is
** an ICON for which a builtin function use appeared.
** Look for:
**	strcpy(X, "string") -> BMOVE of "string" if X has no side effects
**	strncpy(X, "string", sizeof("string")+1) -> (same as strcpy)
**	strlen("string") -> ICON of appropriate size
*/
{
    static int first = 1;
    static char * n_bi_strcpy;
    static char * n_bi_strncpy;
    static char * n_bi_strlen;
    ND1 * tp = p->right;
    char * s = SY_NAME(p->left->rval);

    if (first) {
	first = 0;
	n_bi_strcpy	=	st_nlookup("strcpy",  sizeof("strcpy"));
	n_bi_strncpy	=	st_nlookup("strncpy", sizeof("strncpy"));
	n_bi_strlen	=	st_nlookup("strlen",  sizeof("strlen"));
    }

    if (p->op == UNARY CALL)
	/* EMPTY */;			/* only handle w/arg functions */
    /* strlen() case */
    if      (s == n_bi_strlen) {
	/* Must have single FUNARG node whose operand is STRING. */
	if (   tp->op == FUNARG
	    && (tp = tp->left)->op == STRING
	    && (tp->rval & TR_ST_WIDE) == 0
	) {
	    ODEBUG(0, p, "strlen(): before rewrite");
	    p->left->op = FREE;
	    p->right->op = FREE;
	    tp->op = FREE;
	    p->op = ICON;
	    /* Remember to look for NUL in string, in case of "ab\0c" */
	    p->lval = strlen(tp->string);
	    p->rval = ND_NOSYMBOL;
	    ODEBUG(0, p, "strlen(): after rewrite");
	}
    }
    /* strcpy() case */
    else if (s == n_bi_strcpy) {
	/* First argument must have no side-effects.
	** Second argument must be string.
	*/
	if (   tp->op == CM
	    && (tp->left->op == FUNARG)
	    && (tp->left->flags & FF_SEFF) == 0
	    && (tp = tp->right)->op == FUNARG
	    && (tp = tp->left)->op == STRING
	    && (tp->rval & TR_ST_WIDE) == 0
	) {
	    /* Convert the tree as follows:
	    **
	    **		CALL				,OP
	    **	       /    \			       /   \
	    **       ICON    ,			     BMOVE  X
	    **		   /   \		     /   \
	    **		  ARG  ARG		    ICON  ,
	    **		  /     \			 / \
	    **		 X	STRING			X  STRING
	    **
	    ** Specific node mappings:
	    **		CALL	-> BMOVE
	    **		ICON	-> ICON
	    **		,	-> ,
	    **		X	-> X
	    **		STRING	-> STRING
	    **		ARG	-> ,OP
	    **		ARG	-> freed
	    */
	    ODEBUG(0, p, "strcpy(): before rewrite");
	    /* Take care of ICON. */
	    p->left->lval = strlen(tp->string)+1;
	    tp = p->left;
	    tp->type = TY_INT;
	    tp->rval = ND_NOSYMBOL;

	    p->op = BMOVE;
	    p->type = TY_CHAR;

	    /* Adjust left side of CM, build ,OP */
	    tp = p->right->left;	/* first ARG node, becomes ,OP */
	    p->right->left = tp->left;
	    tp->right = tr_copy(tp->left);
	    tp->left = p;
	    tp->op = COMOP;

	    /* Get rid of second ARG node. */
	    p->right->right->op = FREE;
	    p->right->right = p->right->right->left;

	    p = tp;
	    ODEBUG(0, p, "strcpy(): after rewrite");
	}
    }
    else if (s == n_bi_strncpy) {
	ND1 * tp2;
	ND1 * stringnode;

	/* First argument must have no side-effects.
	** Second argument must be string.
	** Third must be integer constant.
	** The string to be copied must be at least as long as
	** the constant length requests.  (We can't force zero
	** filling.)
	*/
	if (   tp->op == CM
	    && (tp2 = tp->left)->op == CM
	    && tp2->left->op == FUNARG
	    && (tp2->left->flags & FF_SEFF) == 0
	    && (stringnode = tp2->right)->op == FUNARG
	    && (stringnode = stringnode->left)->op == STRING
	    && (stringnode->flags & TR_ST_WIDE) == 0
	    && (tp2 = tp->right)->op == FUNARG
	    && (tp2 = tp2->left)->op == ICON
	    && tp2->rval == ND_NOSYMBOL
	    && (int) strlen(stringnode->string) + 1 >= tp2->lval
	) {
	    /* Convert the tree as follows:
	    **
	    **		CALL				,OP
	    **	       /    \			       /   \
	    **       ICON1   ,1 		     BMOVE  X
	    **		   /   \		     /   \
	    **		  ,2   ARG3		    ICON  ,
	    **		 / \     \			 / \
	    **	       ARG1 ARG2 ICON2			X  STRING
	    **		/    \
	    **	       X   STRING
	    **
	    ** Specific node mappings:
	    **		CALL	-> BMOVE
	    **		ICON2	-> ICON
	    **		,2	-> ,
	    **		,1	-> ,OP
	    **		X	-> X
	    **		STRING	-> STRING
	    **		ARG1,2,3 -> freed
	    **		ICON1	-> freed
	    */

	    ODEBUG(0, p, "strncpy(): before rewrite");

	    /* Right now:
	    ** tp	points at ,1
	    ** tp2	points at ICON2
	    ** stringnode points at STRING
	    ** p	points at CALL
	    **
	    ** Take care of CALL -> BMOVE.
	    */
	    p->op = BMOVE;
	    p->type = TY_CHAR;
	    p->left->op = FREE;
	    p->left = tp2;

	    tp2 = tp->left;		/* ,2 */

	    /* Build ,OP */
	    tp->right->op = FREE;	/* ARG3 */
	    tp->op = COMOP;
	    tp->type = p->type;
	    tp->left = p;
	    tp->right = tp2->left->left; /* X */
	    p->right = tp2;		/* ,2 */
	    p = tp;

	    /* Tidy up lower CM to be BMOVE's CM */
	    tp2->left->op = FREE;
	    tp2->left = tr_copy(tp->right);	/* X */
	    tp2->right->op = FREE;
	    tp2->right = stringnode;

	    ODEBUG(0, p, "strncpy(): after rewrite");
	}
    }

    return ( p );
}


static int
op_tysize( t )
T1WORD t;
/* Use type t to return a pseudo size for the purpose of optimizing
** CONV nodes.
*/
{
    switch(TY_UNQUAL(TY_TYPE(t))){
    case TY_CHAR:  case TY_SCHAR:  case TY_UCHAR:
    case TY_SHORT: case TY_SSHORT: case TY_USHORT:
    case TY_INT:   case TY_SINT:   case TY_UINT:
    case TY_LONG:  case TY_SLONG:  case TY_ULONG:
    case TY_ENUM:
	return( TY_SIZE(t) );

    case TY_FLOAT:	return(TY_SIZE(TY_LONG) + 1);	/* must appear > long */
    case TY_DOUBLE:	return(TY_SIZE(TY_LONG) + 2);
    case TY_LDOUBLE:	return(TY_SIZE(TY_LONG) + 3);
    }
    cerror("op_tysize:  bad type");
    /*NOTREACHED*/
}

static void
op_fpe_handler()
/* floating point exception handler for optim.c */
{
    errno = ERANGE;
    longjmp(fpe_buf,1);
    /*NOTREACHED*/
}


#ifndef NODBG

static void
op_oprint(p,s)
ND1 * p;
char * s;
{
    DPRINTF("\n*** %s ***\n", s);
    tr_e1print(p, "T");
    DPRINTF("---------\n");
}

#endif
