/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/lint.c	1.13"
#include <stdio.h>
#include <ctype.h>
#include "p1.h"
#include "lnstuff.h"
#include "ldope.h"
#include <string.h>

/*
** Global lint routines - routines in this file are performed during
** the build of trees, or when symbols are added/deleted from the
** symbol table.
*/

#ifdef __STDC__
static void ln_nullchk(int);
void ln_setcurfunc(SX);
#else
static void ln_nullchk();
void ln_setcurfunc();
#endif

int 	ln_ifflg=0;		/* is 1 if if statement has no conseq.   */
int	ln_elseflg=0;		/* is 1 if else statement has no conseq. */
int	ln_walk=1;		/* ok to walk this tree			*/
int	ln_stmt;		/* statement seen since a case label	*/
static	int ln_null;		/* perform null checks			*/
static	int ln_ifelse=1;	/* perform if/else checks		*/
#ifndef NODBG
int	ln_dbflag;		/* for lint debugging			*/
#endif


/*
** Return non-zero if node p is a negative constant.
*/
int
isnegcon(p)
ND1 *p;
{
    LNBUG(ln_dbflag > 1,("isnegcon"));
    return ((p->rval == ND_NOSYMBOL) && 
	    (((p->op == ICON) && (p->lval < 0)) ||
	     ((p->op == FCON) && (p->dval < 0))) );
}

/*
** Return non-zero if node p is a constant.
*/
int
iscon(p)
ND1 *p;
{
    LNBUG(ln_dbflag > 1,("iscon"));
    return ((p->rval == ND_NOSYMBOL) && ((p->op == ICON) || (p->op == FCON)));
}

/*
** Return non-zero if node p is a zero constant.
*/
int
iszero(p)
ND1 *p;
{
    LNBUG(ln_dbflag > 1,("iszero"));
    return ((p->rval == ND_NOSYMBOL) &&
	    (((p->op == ICON) && (p->lval == 0)) ||
	     ((p->op == FCON) && (p->dval == 0.0))));
}

/*
** Return non-zero if nodes l and r refer to the same symbol id.
*/
int
issameid(l,r)
ND1 *l, *r;
{
    LNBUG(ln_dbflag > 1,("issameid"));
    return ((l->op==NAME) && (r->op==NAME) && 
	    (l->rval==r->rval) && (l->lval==r->lval));
}

/*
** Return non-zero if the values of the constants at nodes l and
** r are not equal.
*/
int
notequal(l,r)
ND1 *l, *r;
{
    LNBUG(ln_dbflag > 1,("notequal"));
    return (((l->op==ICON) && (r->op==ICON) &&
	    ((l->rval!=r->rval) || (l->lval!=r->lval)))  
		||
	   ((l->op==FCON) && (r->op==FCON) &&
	    ((l->dval!=r->dval) || (l->lval!=r->lval))));
}



/* 
** Called from sy_clear:
** At the end of each block, process all the symbols at that scope.
*/
void
ln_symunused(sid, level)
int sid;
int level;
{
    SX curfunc = ln_curfunc();
    int tmpline;
    I7 class = SY_CLASS(sid);
    LNBUG( ln_dbflag > 1, ("ln_symunused: %d", sid));

    /*
    ** Is this needed ??
    */
    if (sid == SY_NOSYM)
	return;

    /*
    ** Use tmpline as the line number when reporting errors.
    ** Use the alternate error call [B]WERRORLN.
    */
    tmpline = SY_LINENO(sid);	/* set tmpline to declaration line number */

    /* 
    ** If this symbol is a tag, check to see if it is defined and don't do the
    ** remaining checks.
    */
    if (   (class == SC_ENUM) 
	&& !(TY_HASLIST(SY_TYPE(sid))) 
	&& (! LN_FLAG('h'))
       ) {
	WERROR("enum never defined: %s", SY_NAME(sid));
	return;
    }

    /*
    ** For variables that are not ref:
    */
    if (NOTREF(sid))  {
	switch (class) {
	    /*
	    ** If the class is static, the level is extern,
	    ** then issue "static unused"
	    ** SY_DEFINED is checked for symbols such as: static int i=1;
	    ** and SY_TENTATIVE is checked for symbols as: static int j;
	    **
	    ** If it is a static variable, but the level is not extern
	    ** (a local static variable), fallthru.
	    */
	    case SC_STATIC:
		if (SY_LEVEL(sid) == SL_EXTERN) {
		    if (SY_SAMEAS(sid) == SY_NOSYM)
			BWERRORLN(tmpline, 0, SY_NAME(sid));
		    break;
		}
		/* FALLTHRU */

	    /*
	    ** If an auto, and not set, issue 
	    **		"variable unused in function"
	    ** If it is set, and is not a pointer, issue 
	    **		"set but not used in function"
	    */
	    case SC_AUTO:
		if (curfunc == SY_NOSYM)
		    return;
		if (NOTSET(sid)) {
		    BWERRORLN(tmpline, 2, SY_NAME(sid),SY_NAME(curfunc));
		    return;
		}
		BWERRORLN(tmpline, 3, SY_NAME(sid), SY_NAME(curfunc));
		return;

	    case SC_EXTERN:
		if (NOTSET(sid) && (SY_FLAGS(sid)&SY_TOMOVE))
		    BWERRORLN(tmpline, 16, SY_NAME(sid));
		/* FALLTHRU */;
	}
    }

    /*
    ** If processing symbols at file-scope level and the symbol is
    ** external, then write out LUM entries for all that were 
    ** referenced or set.  If -W was specified, we don't want to generate
    ** an LUM here.
    */
    if (   (level == SL_EXTERN) 
	&& (SY_CLASS(sid) == SC_EXTERN) 
	&& (ISREF(sid) || ISSET(sid))
	&& (!LN_FLAG('W'))
       ) {
	SY_LINENO(sid) = er_getline();
	ln2_outdef(sid, LUM, 0);
    }
}



/*
** Called from dcl_e_func()
**
** Must be called before sy_declev() is called (so that the parameters
** are still in scope.)
** Complain about unused parameters. If the ARGSUSEDn lint directive
** is used, complain only about the first n parameters.
*/
void
ln_params(nparam)
int nparam;
{
    int upto, i, tmpline;
    SX curfunc = ln_curfunc();
    LNBUG( ln_dbflag > 1, ("ln_params"));

    /* 
    ** If the current function has not been defined, then do not issue
    ** these diagnostics.
    ** [i.e. int fun(int a, int b);  - prototype: don't complain about a&b]
    **
    ** Also for asm functions, don't complain about unused arguments.
    */
    if (NOTDEFINED(curfunc) || (SY_CLASS(curfunc) == SC_ASM))
	goto end;

    /* The ARGSUSED directive was not used, check all paramters */
    if (LN_DIR(ARGSUSED) == -1)
	upto = nparam;

    /* User messed up - they already were warned about this earlier */
    else if (LN_DIR(ARGSUSED) > nparam) 
	upto = nparam;

    /* the ARGSUSED directive was specified, check only first n arguments */
    else upto = LN_DIR(ARGSUSED);

    /* 
    ** If we have a VARARGS function, then don't complain about the last
    ** argument which is a "..."
    */
    if (TY_ISVARARG(SY_TYPE(curfunc)))
	upto--;

    for (i = 0;i < upto; i++) {
	SX sid = dcl_g_arg(i);
	tmpline = SY_LINENO(sid);

	if (ISSET(sid) && NOTREF(sid) && !TY_ISPTR(SY_TYPE(sid)))
	    BWERRORLN(tmpline, 3, SY_NAME(sid), SY_NAME(curfunc));

	/* "argument unused in function" */
	if (   NOTSET(sid) 
	    && NOTREF(sid)
	    && (! LN_FLAG('v'))
	   )
		BWERRORLN(tmpline, 1,SY_NAME(sid),SY_NAME(curfunc));
    }

    end:
    LN_DIR(ARGSUSED) = -1;		/* reset to default */
}




/*
** Called for every node during optimization that has a goal
** of EFF (effects).
** If the null checking flag is off, the op has a side effect,
** or the special case of casts to void, return 1.
** Otherwise there is no side effect so issue a warning.
** If the operator is EQ (==), then issue "== found where = ..." messages.
*/
int
ln_sides(tr)
ND1 *tr;
{
    int op=tr->op;
    LNBUG(ln_dbflag > 1, ("ln_sides: op: %s",opst[op]));

    if (! ln_null
	|| LN_SIDE(op)
  	|| (op == CONV)
       )
	return 1;

    BWERROR(5);

    if (op == EQ)
      WERROR("equality operator \"==\" found where assignment \"=\" expected");
    return 0;
}




/*
** Write info on function definition for curfunc.
** Main can only have zero or two arguments in a strictly conforming program
** Issued under -Xc only
*/
void
ln_funcdef(curfunc, nargs)
SX curfunc;
int nargs;
{
    LNBUG( ln_dbflag > 1, ("ln_funcdef"));

    ln_setcurfunc(curfunc);

    /*
    ** Write out info for lint2 - (0 is for not a prototype declaration).
    */
    ln2_funcdef(curfunc, nargs, 0);

    if (LN_FLAG('R'))
    	cx_ident(curfunc, - er_getline());

    if ((version&V_STD_C) && (strcmp(SY_NAME(curfunc), "main") == 0)) {
	T1WORD ty = TY_TYPE(TY_DECREF(SY_TYPE(curfunc)));
	if ((nargs != 0) && (nargs != 2))
	    WERROR("only 0 or 2 parameters allowed: main()");
	if ((ty != TY_INT) && (ty != TY_SINT))
	    WERROR("function must be of type int: main()");
    }
}



/*
** ln_rch
**
** If the lintdirective NOTREACHED was used, LN_DIR(NOTREACHED) is 1 - reset the
** flag, and return 0.
*/
int
ln_rch()
{
    LNBUG( ln_dbflag > 1, ("ln_rch"));

    if (LN_DIR(NOTREACHED) == 1) {
	LN_DIR(NOTREACHED) = 0;
	return 0;
    }
    return 1;
}




/*
** Called from tr_ccon(); if the character constant has more than one
** character, and the -Xc option was used, issue diagnostic.
*/
void
ln_chconst(len)
int len;
{
    if ((len > 1) && ((version&V_STD_C) || LN_FLAG('p')))
        WERROR("nonportable character constant");
}



/*
** Called from sm_expr()
*/
void
ln_expr(p)
ND1 *p;
{
    LNBUG( ln_dbflag > 1, ("ln_expr"));

    /* Turn on null checks */
    ln_nullchk(1);

    if (p != ND1NIL) {
	/*
	** A statement (expression) was seen - don't issue if/else
	** messages.
	*/
	ln_ifflg = ln_elseflg = 0;
	ln_stmt = 1;

	/*
	** If this is a return tree, and the function is returning a pointer
	** to an auto, issue diagnostic.
	** Tell pass2 that the function has a return in it.
	*/
	if ((p->op == ASSIGN) && (p->left->op == RNODE)) {
	    ND1 *r = p->right;
	    static char ptr_msg[]="function returns pointer to %s";
	    unconv(r);
	    ln2_retval();
	    if ((r->op == UNARY AND) && IS_SYMBOL(r->left)) {
		I7 class = SY_CLASS(r->left->rval);
		if (class == SC_AUTO)
		    WERROR(ptr_msg, "automatic");
		else if (class == SC_PARAM)
		    WERROR(ptr_msg, "parameter");
	    }
	}
    } 
}



/*
** Reset ln_sidptr for evaluation order undefined, reset CONSTCOND flag,
** and perform if/else statement check if the flag isn't set.
*/
void
ln_endexpr()
{
    LNBUG(ln_dbflag > 1, ("ln_endexpr"));
    ln_sidptr = 0;
    ln_nullchk(0);
    LN_DIR(CONSTANTCONDITION) = 0;
    if (ln_ifelse)
	ln_if_else();
}



/*
**
*/
void
ln_if_else()
{
    static const char msg[]="statement has no consequent: %s";
    LNBUG( ln_dbflag > 1, ("ln_if_else"));

    if (ln_ifflg && !LN_DIR(EMPTY) && !LN_FLAG('h'))
	WERRORLN(ln_ifflg, msg, "if");
    else if (ln_elseflg && !LN_DIR(EMPTY) && !LN_FLAG('h'))
	WERRORLN(ln_elseflg, msg, "else");

    LN_DIR(EMPTY) = ln_ifflg = ln_elseflg = 0;
}


#ifndef NODBG
char *
pgoal(g)
int g;
{
    if (g&ASSG) {
	if (g&CC)
	    return "ASSG|CC";
	else if (g&EFF)
	    return "ASSG|EFF";
	else if (g&VAL)
	    return "ASSG|AVAL";
	else if (g&AVAL)
	    return "ASSG|AVAL";
	else return "ASSG";
    };
    switch(g) {
	case CC: return "CC";
	case EFF: return "EFF";
	case VAL: return "VAL";
	case AVAL: return "AVAL";
    }
    /* NOTREACHED */
}
#endif



/*
** return a new goal;
**	- op is the current op
**	- goal is the current goal
**	- side is the side of the op to return a goal for
*/
int
ln_getgoal(tr, goal, side)
ND1 *tr;
int goal, side;
{
    int op = tr->op;
    LNBUG( ln_dbflag > 1, ("ln_getgoal: op: %s, goal: %s, side: %s",
	  opst[op], pgoal(goal), (side==LN_RIGHT) ? "right" : "left"));


    /* ln_preop(tr, goal); */

    /*
    **          CBRANCH (goal)
    **         /       \
    **       (CC)     (VAL)
    */
    if (op == CBRANCH)
	return (side == LN_RIGHT) ? VAL : CC;

    /*
    **        ASGOP (goal)
    **       /     \
    **  (ASSG|goal) (VAL)
    **
    ** For assignments: 
    **	- for a normal assign: right side used for its value.  Left side
    **	  is only set (e.g. a=b=c;  a&b both set, but only c is referenced;
    **    in b=c, b is not referenced - the value of the expression is.)
    **
    **  - for other assignments (INCR, DECR, etc...), pass ASSG or'ed with
    **	  the original goal.
    **    (e.g. a[b++]=10;  b is set AND referenced.)
    */
    if (asgop(op)) 
	return (side == LN_RIGHT) ? VAL : (op == ASSIGN) ? ASSG : ASSG|goal;

    /*
    **         CALL (goal)
    **        /    \
    **    (VAL)   (VAL)
    **
    ** Pass VAL down to the function name, and pass VAL down to
    ** the arguments.
    */
    if (callop(op))
	return VAL;

    /*
    **         U&  (goal)
    **         |
    **        (AVAL)
    **
    ** The address is being used - so don't say "used before set" on this
    ** type (e.g.  int *a,b;  a=&b; - don't complain about b used before set).
    */
    if (op == UNARY AND)
	return (goal&ASSG) ? (ASSG|AVAL) : (AVAL);

    /*
    **          COMOP (goal)
    **         /     \
    **       (EFF)   (goal)
    **
    ** For a COMOP, the left side is always used for effects, and pass the
    ** original goal down the right.
    */
    if (op == COMOP)
	return (side == LN_RIGHT) ? goal : EFF;

    /*
    **       DOT (ASSG)				!DOT  (ASSG)
    **      /   \			       /    \	
    **  (ASSG)   (VAL)			     (VAL)  (VAL)
    **
    ** This is the only case where an 
    ** assignment is passed down
    */
    if (goal&ASSG)
	if (op == DOT)
	    return (side == LN_RIGHT) ? VAL : goal;
	else return (side == LN_RIGHT) ? VAL : goal|VAL;

    if (goal&AVAL)
	if (op == DOT)
	    return goal;
	else return VAL;

    /*
    **         ANDAND|OROR|QUEST  (goal)
    **              /   \
    **            (CC)   : (goal)
    **
    ** For expressions like: "(a==b) && (a=10)" we don't want to complain about
    ** null effects because it really means "if (a==b) a=10".
    ** Treat the left as a condition code, and the right pass down the original
    ** goal.
    */
    if ((op == QUEST) || (op == ANDAND) || (op == OROR))
	return (side == LN_RIGHT) ? goal : CC;

    if ((op == CONV) && (goal == EFF))
	return VAL;

    /*
    ** End of special cases - for all other ops, return the same goal.
    */
    return goal;
}




/*
** Called from acgram.y when: L_LP e L_RP was seen.
** Set the paren flag on the tree for e.
*/
void
ln_paren(tr)
ND1 *tr;
{
    LNBUG( ln_dbflag > 1, ("ln_paren"));

    tr->flags |= FF_PAREN;
}



static SX ln_func;

SX
ln_curfunc()
{
    LNBUG(ln_dbflag > 1, ("ln_curfunc"));
    return ln_func;
}

void
ln_setcurfunc(sid)
SX sid;
{
    LNBUG(ln_dbflag > 1, ("ln_setcurfunc: %d", sid));
    ln_func = sid;
}


static void
ln_nullchk(val)
int val;
{
    LNBUG(ln_dbflag > 1, ("ln_nullchk: %d",val));
    ln_null = val;
}



/*
** Check to see if execution falls through one case statement to
** another (by not having a break, return, goto, continue.)
** This can be suppressed by using the FALLTHRU or FALLTHROUGH
** directive.
*/
void
ln_chkfall(rch)
int rch;
{
    LNBUG(ln_dbflag > 1, ("ln_chkfall: %d", rch));

    if (rch && ln_rch()) {
	if (LN_DIR(FALLTHRU))
	    LN_DIR(FALLTHRU) = 0;
	else if (ln_stmt && !LN_FLAG('h'))
	    WERROR("fallthrough on case statement");
    }
    ln_stmt = 0;
}


static int dfl_goal = EFF;

void
ln_setgoal(goal)
int goal;
{
    dfl_goal = goal;
}

int
ln_dflgoal()
{
    return dfl_goal;
}


static char curlev = 0;


void
ln_inclev()
{
    static old_if = 0, old_else = 0;
    LNBUG(ln_dbflag > 1, ("ln_inclev"));

    if (ln_ifflg || ln_elseflg) {
	if ((old_if != ln_ifflg) || (old_else != ln_elseflg)) {
	    old_if = ln_ifflg;
	    old_else = ln_elseflg;
	    ln_ifelse = 0;
	    curlev = 0;
	} else curlev++;
    }
}

void
ln_declev()
{
    LNBUG(ln_dbflag > 1, ("ln_declev"));

    if (curlev == 0) {
	ln_ifelse = 1;
	ln_if_else();
    } else curlev--;
}
