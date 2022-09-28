/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/stmt.c	55.4"
/* stmt.c */

/* This module handles statement-level code. */

#include "p1.h"

/* This structure holds state information for each block. */

struct blkstk {
    int sm_break;		/* break label # */
    int sm_continue;		/* continue label # */
    int sm_top;			/* top-of-loop label # */
    int sm_test;		/* label # for loop test */
    int sm_default;		/* label # for switch default */
    int sm_ifelse;		/* label # to go to at end of if/else */
    int sm_flow;		/* flow status */
    ND1 * sm_control;		/* for/do/while controlling expr */
    ND1 * sm_iter;		/* for iterator */
    T1WORD sm_swtype;		/* type of switch controlling expression */
    T1WORD sm_swotype;		/* original type of controlling expression */
#ifdef FAT_ACOMP
    int sm_weight;		/* current symbol-use weight */
#endif
};

#define	SM_NOLABEL (0)		/* means no current label */

#ifndef	INI_BLKSTK
#define	INI_BLKSTK 20		/* room for 20-deep blocks */
#endif

#ifndef	GETLAB
extern int getlab();
#define	GETLAB() getlab()	/* routine to get unique int:  use CG's */
#endif

TABLE(Static, blkstk, static, blkstk_init,
		struct blkstk, INI_BLKSTK, 0, "block stack");
#define ty_blkstk struct blkstk
#define BB(i) (TD_ELEM(blkstk, ty_blkstk, (i)))

#define sm_sp TD_USED(blkstk)	/* stack pointer for block operations */

/* Status of current block.  Note that because the first thing
** that will push something onto the control stack is the { that
** starts a function, the following, initialized, block always
** starts off the control stack.
*/
static struct blkstk sm_curblock = {
    SM_NOLABEL,		/* sm_break */
    SM_NOLABEL,		/* sm_continue */
    SM_NOLABEL,		/* sm_top */
    SM_NOLABEL,		/* sm_test */
    SM_NOLABEL,		/* sm_default */
    SM_NOLABEL,		/* sm_ifelse */
    0,			/* sm_flow */
    ND1NIL,		/* sm_control */
    ND1NIL,		/* sm_iter */
    TY_NONE,		/* sm_swtype */
    TY_NONE,		/* sm_swotype */
#ifdef FAT_ACOMP
    SM_WT_INITVAL,	/* sm_weight */
#endif
};

/* sm_flow bits: */
#define	FW_LOOP		(1<<0)		/* Unconditional loop (no test needed). */
#define	FW_CONT		(1<<1)		/* Continue label required. */
#define	FW_BREAK	(1<<2)		/* Break label required. */
#define	FW_TEST		(1<<3)		/* Test label required. */
#define	FW_HADLC	(1<<4)		/* Denotes whether construct had
					** compound statement
					*/
#define	FW_NONE	0
#define	FW_ALL	(FW_LOOP|FW_CONT|FW_BREAK|FW_TEST|FW_HADLC)
#define	FW_ALLBUTLC (FW_ALL & ~FW_HADLC)

/* HACK:  FW_HADLC is used to solve a deficiency in line number
** handling:  the current line number is really the line number for
** the last token seen by the scanner (not parser).  Therefore, in
** this code:
**	for (;e;)
**	    if (e)
**	        statement
**	label: ;
**
** the grammar can't tell whether the "if" has an "else" until it
** reads the token after "statement", which is a label name.  Note
** that after processing the end of the "if" we process the end of
** the "for", which may involve generating a loop test at the bottom.
** The code for that loop test would have the line number of the label,
** because that is the current line number, but it would be wrong.
** If FW_HADLC is set, it is safe to generate the line number after a
** "for" or "while", because a {} pair brackets the loop code.  The
** token at the end of the loop must then be }.
** Also, generate line number at end of switch block to mark the
** beginning of the selection code.
*/


/* Define a bunch of short-hand names.  These refer to
** the current block of interest.
*/
#define	cur_break	sm_curblock.sm_break
#define	cur_continue	sm_curblock.sm_continue
#define cur_top		sm_curblock.sm_top
#define cur_test	sm_curblock.sm_test
#define cur_default	sm_curblock.sm_default
#define cur_ifelse	sm_curblock.sm_ifelse
#define cur_flow	sm_curblock.sm_flow
#define cur_control	sm_curblock.sm_control
#define cur_iter	sm_curblock.sm_iter
#define	cur_swtype	sm_curblock.sm_swtype
#define	cur_swotype	sm_curblock.sm_swotype
#define cur_weight	sm_curblock.sm_weight

static int sm_reached;		/* current statement is reachable if non-0 */
static int sm_nolineno = 0;	/* turned on locally to suppress debug line
				** numbers under some circumstances
				*/
static SX sm_curfunc;		/* symbol ID of current function */
static T1WORD sm_functype;	/* function type of current function */
#ifdef FAT_ACOMP
static int sm_curweight = SM_WT_INITVAL;
				/* current variable weight */
#endif


/* Type of loop code to generate:
**	LL_TOP:  generate test at top
**	LL_BOT:  generate test at bottom
**	LL_DUP:  generate test at top and bottom
*/

#ifndef	FOR_LOOP_CODE
#define	FOR_LOOP_CODE	LL_DUP		/* duplicate for-loop tests */
#endif

#ifndef	WH_LOOP_CODE
#define	WH_LOOP_CODE	LL_BOT		/* while-loop tests at bottom */
#endif

#ifdef LINT
/*
** Lint always wants loops to be generated at the top only to
** prevent duplicate testing (and duplicate warning messages.)
*/
int sm_for_loop_code = LL_TOP;
int sm_while_loop_code = LL_TOP;
#else
int sm_for_loop_code = FOR_LOOP_CODE;	/* default for-loop test type */
int sm_while_loop_code = WH_LOOP_CODE;	/* default while-loop test type */
#endif


/* Support for adjusting weights in statements. */

#ifndef SM_WT_LOOPWT
#define SM_WT_LOOPWT 10
#endif

#ifndef SM_WT_BOUND
#define SM_WT_BOUND 10000
#endif

/* New weight for for loops and other statements, based on current weight. */
#ifndef SM_WT_FOR
#define SM_WT_FOR(cur) (cur > SM_WT_BOUND ? cur : (cur*SM_WT_LOOPWT))
#endif

#ifndef SM_WT_WHILE
#define SM_WT_WHILE(cur) (cur > SM_WT_BOUND ? cur : (cur*SM_WT_LOOPWT))
#endif

#ifndef SM_WT_DO
#define SM_WT_DO(cur) (cur > SM_WT_BOUND ? cur : (cur*SM_WT_LOOPWT))
#endif

#ifndef SM_WT_IF
#define SM_WT_IF(cur) ((cur+1)/2)
#endif

#ifndef SM_WT_SWITCH
#define SM_WT_SWITCH(cur) ((cur+1)/2)
#endif


#ifdef FAT_ACOMP

int
sm_g_weight()
/* Return current symbol weight. */
{
    return sm_curweight;
}

#endif

static void
sm_pushblk()
/* Push current block on stack, start new one.  New block inherits
** current state information.
*/
{
    TD_NEED1(blkstk);		/* need an entry */
    TD_CHKMAX(blkstk);		/* keep statistics */
    BB(sm_sp) = sm_curblock;	/* push current block on stack */
    ++sm_sp;
#ifdef FAT_ACOMP
    cur_weight = sm_curweight;	/* set new weight to be saved */
#endif
#ifdef LINT
    /*
    ** This is done to help with if/else has no consequent checking.
    ** Don't want to do the if/else check until the corresponding
    ** } of an if/else is seen.
    */
    ln_inclev();
#endif
    return;
}


static void
sm_popblk(flowmask)
int flowmask;
/* Restore status as of previous block.  Only propagate
** backward the flow bits that are set in flowmask.  If
** the old block has a default label, propagate it backward.
** Consider:
**
**	switch(e){
**	    if(e) {
**		default: ...
**	    }				# propagate default from if to switch
*/
{
    int flow = cur_flow & flowmask;
    int defaultlab = cur_default;

    if (sm_sp <= 0)
	cerror("block stack underflow, %d", sm_sp);
    
    --sm_sp;
    sm_curblock = BB(sm_sp);		/* Affects cur_* variables. */
#ifdef FAT_ACOMP
    sm_curweight = cur_weight;		/* Restore current weight from block */
#endif
    cur_flow |= flow;
    /* Propagate the default label backward if it's new. */
    if (defaultlab != cur_default) {
	if (cur_default == SM_NOLABEL)
	    cur_default = defaultlab;
	else if (defaultlab != SM_NOLABEL)
	    cerror("overwriting default");
    }
#ifdef LINT
    /*
    ** For "if/else has no consequent" checking.
    */
    ln_declev();
#endif
    return;
}



int
sm_genlab()
/* Generate a new label number. */
{
    return( GETLAB() );			/* Get unique label number. */
}


static void
sm_chkreach(isloop)
int isloop;
/* Check whether the current execution point can be reached.
** Generate a warning if not.  If isloop != 0, we're entering
** a loop.
*/
{
    if (!sm_reached) {
	if (isloop)
	    WERROR("loop not entered at top");
	else
#ifdef LINT
	    BWERROR(9);
#else
	    WERROR("statement not reached");
#endif
	sm_reached = 1;			/* pretend statement is reachable */
    }
    return;
}


void
sm_begf(sid)
SX sid;
/* Set up for function entry.  Called from declaration processing
** when the start of a function definition is recognized.  Transform
** any parameters that need it:  register parameters must be copied
** to their resting places, as appropriate; old-style float parameters
** must be truncated.  Also output debug information for them.
*/
{
    int argno;
    SX paramsid;

    if (sm_sp != 0)
	cerror("bad sm_sp in sm_begf(), %d", sm_sp);
    
    sm_reached = 1;			/* first statement (start of function)
					** is reachable
					*/

    sm_curfunc = sid;			/* remember current symbol ID */
    sm_functype = SY_TYPE(sid);		/* remember its type, too */

    al_begf();				/* start auto allocation */
    DB_BEGF(sid);			/* generate debug info at start of func. */
    cg_begf(sid);			/* generate function prologue */
    DB_S_FCODE();			/* generate debug info at start of
					** function's code.
					*/

    for (argno = 0; (paramsid = dcl_g_arg(argno)) != SY_NOSYM; ++argno) {
	CG_COPYPRM(paramsid);
	DB_SYMBOL(paramsid);
    }
    return;
}


void
sm_endf()
/* Generate function epilogue and finish off function.  Called
** from declaration processing after we process the closing }
* of a function definition.
*/
{
    if (sm_sp != 0)
	cerror("bad sm_sp in sm_endf(), %d", sm_sp);
    
#ifdef LINT
    /* Check for notreached lint directive on 
    **   "function falls off bottom without returning value"
    **
    ** Put out "function returns value" record (LRV) if NOTREACHED
    ** was set.  This takes care of code like
    **
    ** 	int f() {
    **		printf("shouldn't get here\n");
    **		exit(2);
    **		/*NOTREACHED* /	
    **	}
    */
    {
	extern void ln_setcurfunc();
	int reached = ln_rch();	/* 1 if LN_DIR(NOTREACHED) was 0 */
	int voidfunc = TY_DECREF(sm_functype) == TY_VOID;
	if (sm_reached) {
	    if (!voidfunc) {
		if (reached)
	            BWERROR(15, SY_NAME(sm_curfunc));
		else
		    ln2_retval();
	    }
	}
	ln_setcurfunc(SY_NOSYM);
    }
#endif

    DB_E_FCODE();			/* generate debug info at end of
					** function's code
					*/
    cg_endf(al_endf());			/* generate epilogue code */
    cg_nameinfo(sm_curfunc);		/* emit size/type information */
    DB_ENDF();				/* generate debug info at end of func. */
    return;
}


void
sm_lc()
/* Handle the { at the beginning of a block. */
{
    sy_inclev();			/* bump symbol table level */
    cur_flow |= FW_HADLC;		/* current construct has compound stmt. */
    al_s_block();			/* save current allocation info */
    sm_pushblk();			/* start new block */
    DB_S_BLOCK();			/* generate debug code for start of blk. */
    return;
}


void
sm_rc()
/* Handle the } at the end of a block.  If we're exiting the outermost
** block of a function, discard function parameters, too.  Propagate
** flow status backward.
*/
{
    sy_declev();			/* exit symbol table level */
    sm_popblk(FW_ALL);			/* exit block and propagate flags */
    al_e_block();			/* do end-of-block allocation clean-up */
    DB_E_BLOCK();			/* generate debug code for end of blk. */
    return;
}


static SX sm_dolab();

void
sm_deflab(s)
char * s;
/* Define label s at this point in the code. */
{
    SX sid = sm_dolab(s,1);
    cg_deflab(SY_OFFSET(sid));
#ifdef LINT
    if (! sm_reached)
	ln_stmt = 0;
#endif
    sm_reached = 1;			/* code now reachable */
    return;
}


static void
sm_gengoto(l)
int l;
/* Generate branch to label l.  Put out line number information,
** if appropriate.  Usually this is called at a point equivalent
** to the beginning of a statement (because of the line number
** stuff).  Note that code after this point is not reachable.
*/
{
    DB_LINENO();			/* do line number */
    cg_goto(l);				/* output branch */
    sm_reached = 0;			/* next code not reachable */
    return;
}


void
sm_goto(s)
char * s;
/* Generate goto to label s at this point in the code.
** Called from grammar.
*/
{
    SX sid = sm_dolab(s,0);
    
    sm_chkreach(0);
#ifdef LINT
    ln_ifflg = ln_elseflg = 0;
#endif
    sm_gengoto(SY_OFFSET(sid));
    return;
}


static SX
sm_dolab(s, flag)
char * s;
int flag;
/* Do the hard work of creating label "s".  If flag is non-0,
** we have a defining instance for the label.
*/
{
    SX sid = sy_lookup(s, SY_LABEL, SY_CREATE);

    if (SY_ISNEW(sid)) {
	/* new label */
	SY_CLASS(sid) = SC_LABEL;
	SY_OFFSET(sid) = sm_genlab();
	SY_FLAGS(sid) |= SY_INSCOPE;	/* labels always in scope */
	DB_SYMBOL(sid);			/* debug information for label */
    }
    else {
	/* Existing label.  Use it again. */
	if (flag && (SY_FLAGS(sid) & SY_DEFINED))
	    UERROR("label redefined: %s", s);
    }
    /* Reference is either a defining instance or a reference. */
    SY_FLAGS(sid) |= (flag ? SY_DEFINED : SY_REF);
    return( sid );
}


void
sm_expr(p)
ND1 * p;
/* Generate code for expression tree p:  check whether the
** code can be reached; optimize the tree; generate code for
** the resulting tree.
*/
{
#ifdef LINT
    ln_expr(p);
#endif
    /* Accept silently unreachable empty statements unless verbose. */
    if (p != ND1NIL || verbose)
#ifdef LINT
	/* If the -b option was used, then don't complain about
	** unreachable empty statements.
	*/
	if ((p != ND1NIL) || !LN_FLAG('b'))
#endif
	sm_chkreach(0);

    if (p != ND1NIL) {
	if (! sm_nolineno)
	    DB_LINENO();		/* emit debugging line number if not
					** suppressed
					*/
	p = op_optim(p);
	CG_ECODE(p);
    }
#ifdef LINT
    ln_endexpr();
#endif
    return;
}


static ND1 *
sm_bbuild(p,to)
ND1 * p;
int to;
/* Build a CBRANCH to "to". */
{
    p = tr_build(CBRANCH, p, tr_icon((CONVAL) to));
    return( p );
}


/* Routines to handle if statements. */

void
sm_if_start(p)
ND1 * p;
/* Start an IF statement.  p points at the expression to test. */
{
    int lab = sm_genlab();

    sm_chkreach(0);

#ifdef FAT_ACOMP
    sm_curweight = SM_WT_IF(sm_curweight); /* Adjust weight for if */
#endif

    sm_pushblk();
    cur_ifelse = lab;
    sm_expr( sm_bbuild(p,lab) );
#ifdef LINT
    /* set flag for "statement has no consequent: if" */
    ln_ifflg = er_getline();
#endif
    return;
}


void
sm_if_else()
/* Saw "else".  Tie off end of then-clause, start else-clause.
** The code looks like this:
**
**	<up to here>
**	goto <new-label>		# if reachable
** oldlabel:
**
** Change the end-label for current if-else to <new-label>.
*/
{
    int newlab;

#ifdef LINT
    ln_if_else();		/* check for no consequent on if      */
    ln_elseflg = er_getline();	/* flag for 'else stmt has no effect' */
#endif
    if (sm_reached) {
	newlab = sm_genlab();
	cg_goto(newlab);		/* no debugger line needed or wanted */
    }
    else
	newlab = SM_NOLABEL;		/* End of if not reachable except
					** by falling out of else.
					*/
    cg_deflab(cur_ifelse);
    cur_ifelse = newlab;
    sm_reached = 1;
    return;
}


void
sm_if_end()
/* Reached end of if.  Tie off with label.  Propagate flow information
** to outer block.
*/
{
    if (cur_ifelse != SM_NOLABEL) {
	cg_deflab(cur_ifelse);
	sm_reached = 1;
    }

    /* The existance of an } that's part of an "if" does not
    ** apply to outer blocks, but other flow information does.
    */
    sm_popblk(FW_ALLBUTLC);
#ifdef LINT
    /* Check to see if if/else has no consequent. */
    ln_if_else();
#endif
    return;
}

/* Loops have the following general forms.  Some of the labels are
** omitted or merged if controlling expressions or iterators are
** omitted or have a known constant value.
**
**	LL_TOP			LL_BOT			LL_DUP
**				goto testlab		if (!e) goto breaklab
**    toplab:		    toplab:	    	toplab:
**	if (!e) goto breaklab
**	stmt			stmt			stmt
**    contlab:		    contlab:	    	contlab:
**	iter			iter			iter
**			    testlab:
**	goto toplab		if (e) goto toplab	if (e) goto toplab
**    breaklab:		    breaklab:		breaklab:
**		
** For while/for loops, there has been a convention that users put
** breakpoints on the } that corresponds to the end of a loop.
** This works because a line number got generated when the loop-end
** code got generated at the end.  Support this behavior here, but
** don't generate a line number if there is no }.
*/

static int
sm_chkcontrol(pp)
ND1 **pp;
/* Check whether controlling expression pointed at by pp is
** contant or not.  Return optimized tree through pp, and
** flag as return value:
**	-1	non-constant expression
**	0	constant expression, always false
**	1	constant expression, always true
** If the expression is constant, return a NIL pointer.  If
** it's always true, set FW_LOOP in current control block.
*/
{
#ifndef LINT
    int conval = -1;			/* presumably not constant expr */
#endif
    ND1 * p = *pp;			/* grab the expression tree */

    /* Empty loop control is equivalent to always-true expression.
    ** (Syntax constrains this to for-loops only.)
    */
    if (p == ND1NIL) {
	cur_flow |= FW_LOOP;
	return( 1 );
    }

#ifdef LINT
    return ( -1 );
#else

    p = op_optim(p);

    /* Figure out if we know what's going to happen in the loop:
    ** is there a constant controlling expression?
    ** NOTE:  There's an assumption here that all symbols resolve
    ** no non-0 addresses.  That's commonly true, but not
    ** guaranteed for all implementations.
    */
    if (p->op == ICON)
	conval = ! (p->lval == 0 && p->rval == ND_NOSYMBOL);
    else if (p->op == FCON)
	conval = ! FP_ISZERO(p->dval);

    /* Dispose of unneeded tree if loop result always known. */
    if (conval >= 0) {
	t1free(p);
	p = ND1NIL;
    }
    if (conval > 0)
	cur_flow |= FW_LOOP;

    *pp = p;				/* Return altered tree. */
    return( conval );
#endif
}


static void
sm_lp_start(p, codetype)
ND1 * p;
int codetype;
/* Do top-of-loop processing for for/while loops with controlling
** expression p.  Values for codetype, the type of loop code desired,
** are:  LL_TOP, LL_BOT, LL_DUP.  Check for constant loop test.
** Generate appropriate labels and jumps, depending on codetype.
*/
{
    int conval = -1;			/* -1 if not constant loop, 0 if
					** constant 0, 1 if constant 1
					*/
    sm_chkreach(1);

    sm_pushblk();

#ifdef	OPTIM_SUPPORT
    OS_LOOP(OI_LSTART);			/* mark start of loop */
#endif
    cur_iter = ND1NIL;			/* assume no iterator */
    cur_control = ND1NIL;		/* assume no loop test */
    cur_flow = 0;			/* no flow bits yet */
    /* This code assumes label generation is cheap and that labels
    ** may be freely wasted.
    */
    cur_top = sm_genlab();		/* label at top of loop */
    cur_continue = sm_genlab();		/* label for continue */
    cur_break = sm_genlab();		/* label after bottom of loop */


    conval = sm_chkcontrol(&p);

    /* If loop never executed, skip it. */
    if (conval == 0) {
	sm_gengoto(cur_break);
	cur_flow |= FW_BREAK;		/* will need to generate label */
#ifdef	OPTIM_SUPPORT
	OS_LOOP(OI_LBODY);		/* body of loop follows, even though
					** it's never executed
					*/
#endif
    }
    else {
	/* Build a positive sense loop test now.  This flushes out any
	** possible problems with the expression tree at the point where
	** the user wrote the code.
	*/
	if (conval < 0)
	    p = sm_bbuild(p, cur_break);

	switch( codetype ) {
	case LL_TOP:
#ifdef	OPTIM_SUPPORT
	    OS_LOOP(OI_LBODY);		/* body of loop follows */
#endif
	    cg_deflab(cur_top);
	    if (conval < 0) {		/* Either 1 or -1. */
		sm_expr(p);
		cur_flow |= FW_BREAK;	/* will need break label */
	    }

	    cur_flow |= FW_LOOP;	/* force branch to top at end */
	    break;
	
	case LL_BOT:
	    if (conval < 0) {		/* Either 1 or -1. */
		cur_test = sm_genlab();	/* will need a test label */
		cur_flow |= FW_TEST;
		sm_gengoto(cur_test);	/* force out line number, too */
		cur_control = p;
	    }
#ifdef	OPTIM_SUPPORT
	    OS_LOOP(OI_LBODY);		/* body of loop follows */
#endif
	    cg_deflab(cur_top);
	    break;

	case LL_DUP:
	    if (conval < 0) {		/* Loop value is not constant. */
		/* HACK!  If there have been errors, discard the duplicate test.
		** The errors may have been from the controlling expression,
		** and we would end up duplicating the messages later.
		** (We have to wait until now in case sm_bbuild() saw the
		** error.)
		*/
		cur_control = (nerrors ? ND1NIL : tr_copy(p));
		sm_expr(p);
		cur_flow |= FW_BREAK;	/* will need break label */
	    }
#ifdef	OPTIM_SUPPORT
	    OS_LOOP(OI_LBODY);		/* body of loop follows */
#endif
	    cg_deflab(cur_top);
	    break;
	
	default:
	    cerror("bad code %d in sm_lp_start()", codetype);
	}
	sm_reached = 1;			/* top of loop is reachable */
    }
    return;
}



static void
sm_lp_end(suppress)
int suppress;
/* Generate loop-end code for current loop.  Produce iterations,
** tests, labels, as needed.  Also, check for reachability of
** bottom of loop.  Need to suppress line number information for
** the end-of-loop code if suppress is set and FW_HADLC is not set.
*/
{
    /* Sequence is:  continue label, iterator, test label, test
    ** (and branch to top), break label.  Don't do the continue
    ** label if it's the same as the top or test label.
    */
    if (   (cur_flow & FW_CONT)
	&& cur_continue != cur_top
	&& cur_continue != cur_test
    ) {
	cg_deflab(cur_continue);
	sm_reached = 1;
    }

    /* Check for conditions where we need to be able to reach
    ** bottom-of-loop stuff.  Seeing a "continue" is equivalent
    ** to reaching the bottom.
    */
    if (!sm_reached && (cur_flow & FW_CONT) == 0)
    {
	WERROR("end-of-loop code not reached");
	sm_reached = 1;
    }

    if (suppress && (cur_flow & FW_HADLC) == 0)
	sm_nolineno = 1;

    if (cur_iter != ND1NIL)
	sm_expr(cur_iter);

    if (cur_flow & FW_TEST) {
	cg_deflab(cur_test);
	sm_reached = 1;
    }
    
#ifdef	OPTIM_SUPPORT
    OS_LOOP(OI_LCOND);			/* loop conditional follows */
#endif

    /* This could be unreachable if loop-always has "continues". */
    if (sm_reached && (cur_flow & FW_LOOP)) {
	cg_goto(cur_top);
	sm_reached = 0;
    }
    else if (cur_control != ND1NIL) {
	/* Build inverted sense conditional branch to top.  If
	** loop test is NIL, loop is always false; don't generate test.
	**
	** Assume top of tree is CBRANCH.  Invert sense of test by
	** inserting ! operator.  The assumption here is that the
	** CBRANCH node was successfully built, which means it should
	** be possible to insert the NOT without complaint.
	*/
	if (cur_control->op != CBRANCH)
	    cerror("confused sm_lp_end()");

	cur_control->right->lval = cur_top;	/* want to branch to loop top */
	cur_control->left = tr_build(NOT, cur_control->left, ND1NIL);
	sm_expr(cur_control);
    }

#ifdef	OPTIM_SUPPORT
    OS_LOOP(OI_LEND);			/* denote end of loop code */
#endif

    if (cur_flow & FW_BREAK) {
	cg_deflab(cur_break);
	sm_reached = 1;			/* code now reachable */
    }

    sm_popblk(FW_NONE);			/* Discard flow information. */
    sm_nolineno = 0;
    return;
}


void
sm_wh_init()
/* Mark beginning of while (prior to controlling expression). */
{
#ifdef FAT_ACOMP
    sm_curweight = SM_WT_WHILE(sm_curweight); /* adjust weight for loop */
#endif
    return;
}


void
sm_wh_start(p)
ND1 * p;
/* Saw beginning of while.  p is controlling expression. */
{
    sm_lp_start(p, sm_while_loop_code);	/* Standard loop start code. */

    /* We may be able to discard the continue label if it could be
    ** the same as the top or test label.
    */
    if ((cur_flow & FW_LOOP) != 0 || sm_while_loop_code == LL_TOP)
	cur_continue = cur_top;
    else if (cur_flow & FW_TEST)
	cur_continue = cur_test;
    return;
}


void
sm_wh_end()
/* Generate code at end of while loop. */
{
    sm_lp_end(1);			/* Standard loop end code. */
    return;
}




void
sm_for_init(p)
ND1 * p;
/* Generate code for initialization part of for loop. */
{
#ifdef LINT
    ln_ifflg = ln_elseflg = 0;
#endif
    sm_chkreach(1);
    sm_expr(p);
#ifdef FAT_ACOMP
    sm_curweight = SM_WT_FOR(sm_curweight); /* Adjust weight for loop. */
#endif
    return;
}



void
sm_for_control(p)
ND1 * p;
/* Generate code for controlling expression part of for loop. */
{
    sm_lp_start(p, sm_for_loop_code);	/* Standard loop prologue. */
    return;
}


void
sm_for_iter(p)
ND1 * p;
/* Do processing associated with seeing a for-loop iterator. */
{
    /* Just save the expression until we need it. */
    if ((cur_iter = p) != ND1NIL)
	cur_iter = op_optim(p);

    /* Do some optimizations of labels if there's no iterator:
    ** If it's an unconditional loop, the continue label is the top label.
    ** If the loop test is at the top, the continue label is the top label.
    ** Otherwise the continue label is the test label, if there is one.
    */
    if (p == ND1NIL) {
	if ((cur_flow & FW_LOOP) != 0 || sm_for_loop_code == LL_TOP)
	    cur_continue = cur_top;
	else if (cur_flow & FW_TEST)
	    cur_continue = cur_test;
    }
    return;
}


void
sm_for_end()
/* Do end of for-loop processing. */
{
    sm_lp_end(1);			/* Do standard end-of-loop code. */
    return;
}



void
sm_do_start()
/* Start of do-while loop. */
{
    sm_chkreach(1);

#ifdef FAT_ACOMP
    sm_curweight = SM_WT_DO(sm_curweight); /* Adjust weight for loop */
#endif

    /* Start new control block.  Establish necessary labels,
    ** generate loop top label.
    */
    sm_pushblk();
    cur_flow = 0;
    cur_iter = ND1NIL;
    cur_top = sm_genlab();
    cur_continue = sm_genlab();
    cur_break = sm_genlab();

#ifdef	OPTIM_SUPPORT
    /* Loop starts now, and body starts immediately. */
    OS_LOOP(OI_LSTART);
    OS_LOOP(OI_LBODY);
#endif

    cg_deflab(cur_top);
    return;
}


void
sm_do_end(p)
ND1 * p;
/* Generate end of do-while loop with controlling expression p. */
{
    /* Do setup for controlling expr.  If constant true, force loop to top. */
    if (sm_chkcontrol(&p) == 1)
	cur_flow |= FW_LOOP;
    /* Build tree here to flush errors in expression.  Branch gets reversed
    ** and target label gets overwritten in sm_lp_end().  Use the same HACK
    ** as previously to avoid putting NOT on bad tree if there were prior
    ** errors.
    */
    if( (cur_control = p) != ND1NIL) {
	cur_control = sm_bbuild(p, cur_top);
	if (nerrors != 0)
	    cur_control = ND1NIL;
    }
    sm_lp_end(0);			/* do standard end-of-loop code */
    return;
}


void
sm_break()
/* Generate jump to break label.  PCC was quiet about
** unreachable breaks, which can cause noise with yacc/lex
** output.  Make noise only if verbose.  Avoid generating
** branch if not reachable.
*/
{
    int wasreached = sm_reached;

    if (verbose)
#ifdef LINT
	/* Don't complain about unreachable break statements with -b. */
	if (! LN_FLAG('b'))
#endif
	sm_chkreach(0);

#ifdef LINT
    ln_ifflg = ln_elseflg = 0;
#endif
    if (cur_break == SM_NOLABEL)
	UERROR("\"break\" outside loop or switch");
    else if (wasreached) {
	sm_gengoto(cur_break);
	cur_flow |= FW_BREAK;		/* will need label later */
    }

    sm_reached = 0;			/* following statements not reached */
    return;
}


void
sm_continue()
/* Generate jump to continue label. */
{
    sm_chkreach(0);
#ifdef LINT
    ln_ifflg = ln_elseflg = 0;
#endif
    if (cur_continue != SM_NOLABEL) {
	sm_gengoto(cur_continue);
	cur_flow |= FW_CONT;		/* will need label later */
    }
    else
	UERROR("\"continue\" outside loop");
    return;
}


void
sm_return(p)
ND1 * p;
/* Return (optional) expression p from current function. */
{
    ND1 * retnode;
    T1WORD rettype = TY_DECREF(sm_functype);	/* type returned from func. */

    sm_chkreach(0);
#ifdef LINT
    ln_ifflg = ln_elseflg = 0;
#endif

    /* Build suitable assign to RNODE, generate code and RETURN. */
    if (p) {
	p = tr_return(p, sm_functype);	/* build suitable assign to RNODE */
	sm_expr(p);
    }
    else if (verbose && TY_TYPE(rettype) != TY_VOID)
	WERROR("function expects to return value: %s", SY_NAME(sm_curfunc));

    retnode = t1alloc();
    retnode->op = RETURN;
    /* If no value being returned, RETURN node has void type. */
    retnode->type = p != ND1NIL ? rettype : TY_VOID;
    sm_expr(retnode);

    sm_reached = 0;
    return;
}

/* Routines to handle switch statement and cases. */

void
sm_sw_start(p)
ND1 * p;
/* Handle beginning of switch statement; p is controlling
** expression.
*/
{
    T1WORD t = p->type;
    T1WORD ptype = t;			/* p's original type */
    int change = 0;			/* set to 1 to change switch type */
#ifdef LINT
    ln_stmt = 0;
#endif

    /* Verify that expression is integral type. */
    if (TY_ISINTTYPE(t)) {
	/* ANSI 3.6.4.2:  The integral promotions are performed on the
	** controlling expression.
	** Force less than INT-sized objects to be INT.
	*/
	if (TY_SIZE(t) < TY_SIZE(TY_INT))
	    change = 1;
    }
    else {
	/* non-integral type */
	static const char mesg[] =
		"switch expression must have integral type";	/* ERROR */
	if (TY_ISFPTYPE(t))
	    WERROR(mesg);		/* just warn and convert value */
	else
	    UERROR(mesg);		/* can't handle */
	change = 1;
    }

    if (change) {
	p = tr_conv(p, TY_INT, "invalid switch expression type", TR_CAST);
	t = TY_INT;
    }
    /* t is now type of controlling expressions and each member. */

    sm_chkreach(0);

#ifdef FAT_ACOMP
    sm_curweight = SM_WT_SWITCH(sm_curweight); /* Adjust weight for switch */
#endif

    sm_pushblk();

    cur_break = sm_genlab();		/* break label for switch */
    cur_default = SM_NOLABEL;		/* no default label yet */
    cur_swtype = t;			/* current switch type */
    cur_swotype = ptype;		/* expression's original type */
    cur_flow &= ~FW_BREAK;		/* haven't seen break in this context */

    /* There's a subtle assumption here that code is generated a statement
    ** (rather than a function) at a time:  we assume that the code for each
    ** switch case must be emitted before the actual switch-begin code.
    ** Therefore we must branch around all those cases to get to the switch
    ** test, which will be generated AFTER them.
    */

    cur_test = sm_genlab();		/* label on the test */
    sm_gengoto(cur_test);

#ifdef LINT
    /* normally the starting goal will be for effects; set it here
    ** for value.
    */
    ln_setgoal(VAL);
#endif
    cg_swbeg(op_optim(p));		/* pass controlling expr. to CG */
#ifdef LINT
    /* and reset back to the default.
    */
    ln_setgoal(EFF);
#endif
    return;
}


void
sm_sw_case(p)
ND1 * p;
/* Add case to current switch statement. */
{
    if (cur_swtype == TY_NONE) {
	UERROR("\"case\" outside switch");
	return;
    }
#ifdef LINT
    ln_chkfall(sm_reached);
#endif

    /* First, get the constant expression value (and warn if it isn't),
    ** rebuild a tree to fold as the right type.
    */
    if (TY_ISINTTYPE(p->type)) {
	CONVAL val;
	int lab = sm_genlab();
	T1WORD ptype = p->type;
	static ND1 node;

	node.op = ICON;
	node.type = ptype;
	node.rval = ND_NOSYMBOL;

	/* Check whether case value would fit in cur_swtype.  There are
	** two failure modes possible:
	**	1.  If the original controlling expression is smaller than
	**		the promoted controlling expression, the case
	**		value might be larger than fits in the controlling
	**		expression's type, and the case would never be reached.
	**	2.  If the case type is bigger than the converted controlling
	**		expression type, or has different signedness,
	**		the case type might be truncated.
	*/

	/* Check first case.  Fold constant first. */
	if (TY_SIZE(cur_swtype) > TY_SIZE(cur_swotype)) {
	    p = tr_conv(p, cur_swtype, "sm_sw_case1??", TR_CAST);
	    node.lval = val = tr_inconex(p);
	    in_chkval(&node, cur_swotype, (int) TY_SIZE(cur_swotype), 0,
			"unreachable case label");	/*ERROR*/
	}
	else if (verbose) {
	    /* Second case:  case value may be larger than converted
	    ** controlling expression type or have different signedness.
	    */
	    ND1 *p2 = tr_copy(p);	/* will have to optimize twice */

	    /* Check original expression against converted switch
	    ** expression type.
	    */
	    node.lval = val = tr_inconex(p);
	    in_chkval(&node, cur_swtype, (int) TY_SIZE(cur_swtype), 0,
			"case label affected by conversion");	/*ERROR*/
	    /* Truncate expression type to controlling expression type. */
	    p = tr_conv(p2, cur_swtype, "sm_sw_case2??", TR_CAST);
	    val = tr_inconex(p);
	}
	else {
	    p = tr_conv(p, cur_swtype, "sm_sw_case3??", TR_CAST);
	    val = tr_inconex(p);		/* just get value */
	}

	/* Generate switch code. */
	cg_deflab(lab);
	cg_swcase(val, lab);		/* CG flags duplicate cases */
    }
    else {
	/* Bogus expression type.  Disregard case but pretend following
	** code is reachable.  Free the expression tree.
	*/
	UERROR("non-integral case expression");
	t1free(p);
    }

    sm_reached = 1;
    return;
}


void
sm_default()
/* Create default label for current switch.  Called from grammar. */
{
    if (cur_swtype == TY_NONE)
	UERROR("\"default\" outside switch");
    else if (cur_default != SM_NOLABEL)
	UERROR("duplicate \"default\" in switch");
    else
	cur_default = sm_genlab();
    
#ifdef LINT
    if (! sm_reached)
	ln_stmt = 0;
#endif

    cg_deflab(cur_default);
    sm_reached = 1;
    return;
}


void
sm_sw_end()
/* Reached end of switch.  Do clean-up and exit code. */
{
    /* See remark under sm_sw_start():  this code assumes that cg_swend()
    ** will force out the switch test code here, so there was a jump
    ** to the test code when the switch beginning was processed.
    */

    /* Falling off the end of a switch is like an implicit "break". */
    if (sm_reached) {
	cur_flow |= FW_BREAK;
	cg_goto( cur_break );
    }

    /* If the switch begins with a { block, emit a line number to
    ** correspond to the }.
    */
    if ((cur_flow & FW_HADLC) != 0)
	DB_LINENO();

    cg_deflab(cur_test);

    cg_swend( cur_default );

    /* Bottom of switch is reachable if it's already reachable,
    ** or if there's a break somewhere, or if there's no "default".
    */
    if (cur_flow & FW_BREAK) {
	cg_deflab(cur_break);
	sm_reached = 1;
    }
    else if (cur_default == SM_NOLABEL)
	sm_reached = 1;
    
    cur_default = SM_NOLABEL;		/* forget this so it won't propagate */
    sm_popblk(FW_CONT);			/* just propagate "continue" info */
    return;
}


#ifndef	NODBG

static void
sm_pblkstk()
/* Print block stack */
{
    int i;

	DPRINTF("lev  brk  cnt  tst  if   flw\n");
	DPRINTF("cur %4d %4d %4d %4d %4d\n",
		sm_curblock.sm_break,
		sm_curblock.sm_continue,
		sm_curblock.sm_test,
		sm_curblock.sm_ifelse,
		sm_curblock.sm_flow
		);

    for (i = sm_sp-1; i >=0; --i) {
	DPRINTF("%3d %4d %4d %4d %4d %4d\n",
		i,
		BB(i).sm_break,
		BB(i).sm_continue,
		BB(i).sm_test,
		BB(i).sm_ifelse,
		BB(i).sm_flow
		);
    }
    return;
}

#endif

#ifdef LINT
/*
** The lint directive "NOTREACHED" was used - user knows it wasn't reached
** so pretend it was to eliminate "statement not reached" and
** "function falls off bottom without returning value"
** (called from lint when the NOTREACHED directive is seen.)
*/
void
sm_lnrch()
{
    sm_reached = 1;
}
#endif
