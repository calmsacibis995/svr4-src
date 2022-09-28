/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/reader.c	1.25"

# include "mfile2.h"
#ifndef	NODBG
/*========= DEBUGGING */
FILE * debugout;
#include <fcntl.h>
#define DEBUGOUT 9		/* debug output file number */
/* ======= END DEBUGGING */
#endif

#ifndef STINCC
static void cprt();
#endif

#ifdef	CG
FILE * debugfile = stdout;	/* For CG debugging */
extern int tmp_start;		/* For CG:  start of temp offsets */
#endif

/*	some storage declarations */

int lflag;
int e2debug;
int udebug;
int fast;
#ifdef IN_LINE
int asmdebug;
#endif

/* maxtemp is the maximum size (in bits) needed for temps so far */
/* maxarg is ditto for outgoing arguments */
/* maxboff is ditto for automatic variables */
/* earlier attempts to keep these on a per-block basis were silly */
int maxtemp;
extern int maxarg;
int maxboff;
NODE * condit(), *leteff();
static NODE * seq();
static int cond();

static NODE *
force(p)
register NODE *p; 
{
	register NODE *q, *r;
	if( !p ) cerror( "force" );
	q = talloc();
	*q = *p;
	r = talloc();
	*r = *p;
	q->tn.op = ASSIGN;
	q->in.right = p;
	q->in.left = r;
	r->tn.op = QNODE;
	r->tn.rval = callreg(p); /* the reg where the value will be forced */
#ifdef CG
	q->in.strat = r->in.strat = 0;
#endif
	return( q );
}

#ifndef	CG		/* p2init for CG is in nail.c */
p2init( argc, argv )
char *argv[];
{
	/* set the values of the pass 2 arguments */

	register int c;
	register char *cp;
	register files;

#ifndef	NODBG
/*  ======== DEBUG */
	if (fcntl(DEBUGOUT, F_GETFL) >= 0)
	    debugout = fdopen(DEBUGOUT, "a");
/*  ======== END DEBUG */
#endif

	allo0();  /* free all regs */
	files = 0;

	for( c=1; c<argc; ++c )
	{
		if( *(cp=argv[c]) == '-' )
		{
			while( *++cp )
			{
				switch( *cp )
				{

				case 'X':  /* pass1 flags */
					while( *++cp ) 
					{
						 /* VOID */ 
					}
					--cp;
					break;

				case 'l':  /* linenos */
					++lflag;
					break;

				case 'e':  /* expressions */
					++e2debug;
					break;

				case 'o':  /* orders */
					++odebug;
					break;

				case 'r':  /* register allocation */
					++rdebug;
					break;

				case 's':  /* shapes */
					++sdebug;
					break;

				case 'u':  /* Sethi-Ullman testing
						(machine dependent) */
					++udebug;
					break;

				case 'f':  /* try for faster compile speed */
					++fast;
					break;

#ifdef IN_LINE
				case 'a':  /* enhanced asm debugging */
					++asmdebug;
					break;
#endif
			
				default:
					cerror( "bad option: %c", *cp );
				}
			}
		}
		else files = 1;  /* assumed to be a ftitle */
	}

	mkdope();
	return( files );
}
#endif	/* ndef CG */

static NODE *
dlabel( p, l )
register NODE *p; 
{
	/* define a label after p is executed */
	register NODE *q;
	if( !p ) cerror( "dlabel" );
	q = talloc();
	q->tn.type = p->tn.type;
	q->in.left = p;
	q->tn.op = GENLAB;
	q->bn.label = l;
	return( q );
}


static NODE *
genbr( o, l, p )
register NODE *p; 
register o,l;
{
	/* after evaluating p, generate a branch to l */
	/* if o is 0, unconditional */
	register NODE *q;
	register int pop;

	if( !p ) cerror( "genbr" );
	if( l < 0 ) cerror( "genbr1" );
	q = talloc();
	q->tn.op = o?GENBR:GENUBR;
	q->tn.type = p->tn.type;
	q->in.left = p;
	q->bn.label = l;
	q->bn.lop = o;
#ifdef IEEE
/* For IEEE standard, have to differentiate between floating point comparisions:
/* CMP for no exception raised on non-trapping NaN's, used for == and != ;
/* CMPE to raise exceptions for all NaN's, used for all other relations.
*/
/* CG:  don't do this if exceptions are to be ignored */
	if(   o
	   && logop( pop=p->tn.op )
#ifdef	CG
	   && !(p->in.strat & EXIGNORE)
	   && (pop != BCMP)
#endif
	   && (pop != ANDAND) 
	   && (pop != OROR)) 
	{
	    if( (   (p->in.left->tn.type == TDOUBLE)
		 || (p->in.left->tn.type == TFLOAT)
		 || (p->in.right->tn.type == TDOUBLE)
		 || (p->in.right->tn.type == TFLOAT) 
	        )
		 &&  pop != EQ
		 &&  pop != NE
	       )
			p->tn.op = CMPE;
	    else
			p->tn.op = CMP;
	}
#else
	if( o && logop( pop=p->tn.op )
#ifdef CG
		&& (pop != BCMP)
#endif
		&& (pop != ANDAND) 
	        && (pop != OROR)
	) p->tn.op = CMP;
#endif
	return( q );
}


static NODE *
oreff(p)
register NODE *p;
{
	register NODE *r, *l;
	NODE *condit();
	int lab;
	/* oreff is called if an || op is evaluated with goal=CEFF
	   The rhs of || ops should be executed only if the
	   lhs is false.  Since our goal is CEFF, we don't need
	   a result of the ||, but we need to
	   preserve that dependancy with this special case */

	/* We must catch this case before its children are
	   condit() and change the goal on it left child to CCC */
	   
	switch( cond(p->in.left) ){
	case 1:				/* always true */
		tfree(p->in.right);
		p->in.op = FREE;
		p = condit( p->in.left, CEFF, -1, -1);
		break;
	case 0:				/* always false */
		p->in.op = COMOP;
		p = condit( p, CEFF, -1, -1);
		break;
	default:			/* don't know */
		lab = getlab();
		p->in.op = FREE;
		r = condit( p->in.right, CEFF, -1, -1);
		/* what to do with left depends on whether right does anything:
		** if right side is null, do left purely for side effects
		*/
		if (r)
		    l = condit( p->in.left, CCC, lab, -1);
		else
		    l = condit( p->in.left, CEFF, -1, -1);
		p = seq(l, r);	/* put r after l */
		/* generate a label if anything done */
		if (r)
		    p = dlabel(p, lab);
	}
	return p;
}
static NODE *
andeff(p)
register NODE *p;
{
	register NODE *r, *l;
	NODE *condit(); 
	int lab;
	/* andeff is called if an && op is evaluated with goal=CEFF
	   The rhs of && ops should be executed only if the
	   lhs is true.  Since our goal is CEFF, we don't need
	   a result of the &&, but we need to
	   preserve that dependancy with this special case */

	/* We must catch this case before its children are
	   condit() and change the goal on it left child to CCC */
	   
	switch( cond(p->in.left) ){
	case 0:				/* always false */
		tfree(p->in.right);
		p->in.op = FREE;
		p = condit( p->in.left, CEFF, -1, -1);
		break;
	case 1:				/* always true */
		p->in.op = COMOP;
		p = condit( p, CEFF, -1, -1);
		break;
	default:
		lab = getlab();
		p->in.op = FREE;
		r = condit( p->in.right, CEFF, -1, -1);
		/* what to do with left depends on whether right does anything:
		** if right side is null, do left purely for side effects
		*/
		if (r)
		    l = condit( p->in.left, CCC, -1, lab);
		else
		    l = condit( p->in.left, CEFF, -1, -1);
		p = seq(l, r);	/* put r after l */
		/* generate a label if anything done */
		if (r)
		    p = dlabel(p, lab);
	}
	return p;
}
static int negrel[] = 
{
	 NE, EQ, GT, GE, LT, LE, UGT, UGE, ULT, ULE 
} ;  /* negatives of relationals */

/* Check a tree.  Return 1 if it always evaluates to non-zero,
** 0 if it always evaluates to 0, -1 if we can't tell.
*/
static
cond( p )
register NODE *p; 
{
	register o = p->tn.op;
	register NODE *q;
	int newcond;

	if (odebug > 2) printf("cond(%d)\n", node_no(p));
	switch( o ) 
	{

	case ICON:
		return( p->tn.lval || p->tn.name != (char *) 0 );

	case COMOP:
#ifdef	CG
	case SEMI:
#endif
		return( cond( p->in.right ) );

	case ANDAND:
		switch( cond(p->in.left) ){
		case 0:		return( 0 );
		case 1:		return( cond(p->in.right) );
		default:
		    if  (cond(p->in.right) == 0)
			return( 0 );
		    return( -1 );	/* don't know */
		}

	case OROR:
		switch( cond(p->in.left) ){
		case 1:		return( 1 );
		case 0:		return( cond(p->in.right) );
		default:
		    if (cond(p->in.right) > 0)
			return( 1 );
		    return( -1 );	/* don't know */
		}

	case NOT:
		switch( cond(p->in.left) ){
		case 1:		return( 0 );
		case 0:		return( 1 );
		default:	return( -1 );	/* don't know */
		}

	case QUEST:
		q = p->in.right;
		switch( cond(p->in.left) ){
		case 1:		return( cond(q->in.left) );
		case 0:		return( cond(q->in.right) );
		default:
		    /* Check if both sides return same value. */
		    newcond = cond(q->in.left);
		    if (newcond >= 0) {
			if (newcond == cond(q->in.right))
			    return( newcond );
		    }
		    return( -1 );	/* don't know */
		}

	default:
		return( -1 );
	}
}

static NODE *
rcomma( p )
register NODE *p; 
{
	/* p is a COMOP (or SEMI); return the shrunken version thereof */

	NODE *alive;
	if (!p) return( p );		/* NILs remain so */
#ifdef	CG
	if( p->tn.op != COMOP && p->tn.op != SEMI ) cerror( "rcomma" );
#else
	if( p->tn.op != COMOP ) cerror( "rcomma" );
#endif

	if( p->in.left && p->in.right ) return( p );
	alive = (p->in.left) ? p->in.left : p->in.right;

		/*For CG: if we delete a paren'ed node, we must parenthesize
		  the node under it (if there is a node under it)*/
#ifdef CG
	if (alive && (p->in.strat & PAREN))
		alive->in.strat |= PAREN;
#endif
	p->tn.op = FREE;
	return( alive );
}
static NODE *
seq( p1, p2 )
register NODE *p1, *p2;
{
	/* execute p then q */
	register NODE *q;

	if (!p1) return p2;
	if (!p2) return p1;
	q = talloc();
	q->in.op = COMOP;
	q->in.type = p2->in.type;
	q->in.left = p1;
	q->in.right = p2;
	return q;
}
static NODE *
gtb( p, l )
register NODE *p; 
register l;
{
	register NODE *q;
	/* replace p by a trivial branch to l */
	/* if l is -1, return NULL */
	q = condit( p, CEFF, -1, -1 );
	if( l<0 ) return( q );
	if( !q ) 
	{
		q = talloc();
#ifdef	CG
		q->tn.op = NOP;
#else
		q->tn.op = ICON;
#endif
		q->tn.lval = 0;
		q->tn.name = (char *) 0;
		q->tn.type = TINT;
	}
	return( genbr( 0, l, q ) );
}

#define T_UNSIGNED(t)	((t) & (TUCHAR|TUSHORT|TUNSIGNED|TULONG|TPOINT|TPOINT2))
#define T_SIGNED(t)	((t) & (TCHAR|TSHORT|TINT|TLONG))

NODE *
condit( p, goal, t, f )
register NODE *p; 
register goal,t,f;
{
	/* generate code for conditionals in terms of GENLAB and GENBR nodes */
	/* goal is either CEFF, NRGS, or CCC */
	/* also, delete stuff that never needs get done */
	/* if goal==CEFF, return of null means nothing to be done */

	register o, lt, lf, l;
	register NODE *q, *q1, *q2;

	o = p->tn.op;

#ifndef NODBG
	if( odebug >2 ) 
	{
		fprintf(outfile, "condit( %d (%s), %s, %d, %d )\n", (int)node_no(p),
		opst[o], goal==CCC?"CCC":(goal==NRGS?"NRGS":"CEFF"),
		t, f );
	}
#endif
	if( o == CBRANCH ) 
	{
		p->in.right->tn.op = p->tn.op = FREE;
		l = p->in.right->tn.lval;
		p = p->in.left;
		switch( cond(p) ){
		case 1:		return( gtb(p,-1) );	/* always true */
		case 0:		return( gtb(p,l) );	/* always false */
		default:				/* don't know */
		    return( condit( p, CCC, -1, l ) );
		}
	}

	/* a convenient place to diddle a few special ops */
	if( callop(o) )
	{
		if ( optype(o) == BITYPE ) {
		     BITOFF argsz = argsize(p->in.right);
        	     if ((p->stn.argsize = (unsigned short)argsz) != argsz)
			  cerror( "argument size is too big");
	        }
		else p->stn.argsize = 0;
		if( goal==CEFF ) goal = NRGS;
		/* flow on, so that we can handle if( f(...) )... */
	}
	else if (   goal == CEFF
		 && (
#ifdef CG
			/*If DOEXACT bit is on, don't edit this out*/
			(p->in.strat & DOEXACT)
		     ||
#endif
			asgop(o)
		     || o == STASG
		     || o == INIT
		     )
		)
		goal=NRGS;

	/* do a bit of optimization */

	if( goal == NRGS ) 
	{
		if( logop(o) )
		{
			/* must make p into ( p ? 1 : 0 ), then recompile */
			q1 = talloc();
			q1->tn.op = ICON;
			q1->tn.name = (char *) 0;
			q1->tn.lval = 1;
			q1->tn.type = p->tn.type;
			q2 = talloc();
			*q2 = *q1;
			q2->tn.lval = 0;
			q = talloc();
			q->tn.op = COLON;
			q->tn.type = p->tn.type;
			q->in.left = q1;
			q->in.right = q2;
			q1 = talloc();
			q1->tn.op = o = QUEST;
			q1->tn.type = p->tn.type;
			q1->in.left = p;
			q1->in.right = q;
			p = q1;  /* flow on, and compile */
		}
	}

	if( goal != CCC ) 
	{
		if( o == QUEST ) 
		{
			/* rewrite ? : when goal not CCC */
			lf = getlab();
			l = getlab();
			p->tn.op = COMOP;
			q = p->in.right;
			q1 = condit( q->in.left, goal, -1, -1 );
			q->in.right = condit( q->in.right, goal, -1, -1 );
			switch( cond(p->in.left) ){
			case 1:				/* always true */
				q->tn.op = FREE;
				tfree( q->in.right );
				p->in.right = q1;
				p->in.left=condit( p->in.left, CEFF, -1, -1 );
				return( rcomma( p ) );
			case 0:				/* always false */
				q->tn.op = FREE;
				tfree( q1 );
				p->in.right = q->in.right;
				p->in.left=condit( p->in.left, CEFF, -1, -1 );
				return( rcomma( p ) );
			}
			/* Don't know disposition of condition. */
			if( !q1 ) 
			{
				if( !q->in.right ) 
				{
 					/* may still have work to do
 					** if left side of ? has effect
 					*/
 					q1 = condit(p->in.left, goal,
						-1, -1);
 					if (!q1)
 					{
 						tfree( p->in.left );
 					}
 					p->tn.op = q->tn.op = FREE;
					return( q1 );
				}
				/* rhs done if condition is false */
				p->in.left = condit( p->in.left, CCC, l, -1 );
				p->in.right = dlabel( q->in.right, l );
				q->tn.op = FREE;
				return( rcomma( p ) );
			}
			else if( !q->in.right ) 
			{
				/* lhs done if condition is true */
				p->in.left=condit( p->in.left, CCC, -1, lf );
				p->in.right = dlabel( q1, lf );
				q->tn.op = FREE;
				return( rcomma( p ) );
			}

			/* both sides exist and the condition is nontrivial */
			/* ( actually, condition may be trivial, but this 
			/*   doesn't happen often enough to risk being clever)
			*/
			p->in.left = condit( p->in.left, CCC, -1, lf );
			/* if there's a value resulting, create QNODEs */
			if (goal <= NRGS) {
			    q1 = force(q1);
			    q->in.right = force(q->in.right);
			}
			q1 = genbr( 0, l, q1 );
			q->in.left = dlabel( q1, lf );
			q->tn.op = COMOP;
			return( dlabel( rcomma(p) , l ) );
		}

		if( goal == CEFF ) 
		{
			/* some things may disappear */
			switch( o ) 
			{

#if defined(CG) && defined(VOL_SUPPORT)
			default:
			    if ((p->in.strat & VOLATILE) == 0)
				break;
			    /* force evaluation for value to touch object */
			    /*FALLTHRU*/
#endif
			case CBRANCH:
			case GENBR:
			case GENUBR:
			case GENLAB:
			case CALL:
			case UNARY CALL:
#ifdef IN_LINE
			case INCALL:		/* handle asm calls as if */
			case UNARY INCALL:	/* they were regular calls */
#endif
			case FORTCALL:
			case UNARY FORTCALL:
			case STCALL:
			case UNARY STCALL:
			case STASG:
			case INIT:
			case MOD:   /* do these for the side effects */
			case DIV:
			case UOP0:
			case UOP1:
			case UOP2:
			case UOP3:
			case UOP4:
			case UOP5:
			case UOP6:
			case UOP7:
			case UOP8:
			case UOP9:
#ifdef CG
				/*Special CG nodes*/
			case UNINIT:
			case SINIT:
			case DEFNAM:
			case COPY:
                        case COPYASM:
                        case JUMP:
                        case GOTO:
                        case ALIGN:
                        case BMOVE:
                        case BMOVEO:
                        case NPRETURN:
                        case VLRETURN:
                        case RETURN:
                        case EXSETV:
                        case EXCLEAR:
                        case EXTEST:
                        case EXRAISE:
			case CAPCALL:
			case CAPRET:
			case RSAVE:
			case RREST:
			case LABELOP:
#endif
				goal = NRGS;
			}
		}

		/* The rhs of && and || ops are executed only if the
		   result is not clear from the lhs.  If our goal is
		   CEFF, we don't need a result, but we need to
		   preserve that dependancy. So special case it. */
		/* For CG: the lhs is always for value; the rhs is
		  whatever the goal of the LET is*/
		if (goal==CEFF)  {
			if (o == ANDAND) return andeff(p);
			if (o == OROR) return oreff(p);
#ifdef CG
			if (o == LET) return leteff(p);
#endif
		}
		/* This next batch of code wanders over the tree getting
		   rid of code which is for effect only and has no
		   effect */
		switch( optype(o) ) 
		{
		case LTYPE:
#ifdef CG
			/*If the DOEXACT bit is set, don't remove it*/
                        if( goal == CEFF && (!(p->in.strat & DOEXACT)))
#else
			if( goal == CEFF ) 
#endif
			{
				p->tn.op = FREE;
				return( NIL );
			}
			break;

		case BITYPE:
			p->in.right = condit( p->in.right, goal, -1, -1 );
			/*FALLTHRU*/
		case UTYPE:
#ifdef CG
			/*This duplicates goal-setting code in rewcom.
			  The lhs of COMOP or SEMI is always for effects*/
			switch(o)
			{
			case COMOP:
			case SEMI:
				p->in.left = condit( p->in.left, CEFF, -1, -1 );
				break;
			default:
				p->in.left = condit( p->in.left, goal, -1, -1 );
				break;
			}
#else
			p->in.left = condit( p->in.left, o==COMOP?CEFF:goal,
			-1, -1 );
#endif
		}

		/* If we are only interested in effects, we quit here */
		if(   goal == CEFF
		   || o == COMOP
#ifdef	CG
		   || o == SEMI
#endif
		) 
		{
#ifdef	CG
			if (p->in.strat & DOEXACT)
				return( p );
#endif
			/* lhs or rhs may have disappeared */
			/* op need not get done */

			switch( optype(o) )
			{

			case BITYPE:
#ifdef	CG
				if (p->tn.op != SEMI)
#endif
				    p->tn.op = COMOP;
				p = rcomma(p);
				return ( p );

			case UTYPE:
				p->tn.op = FREE;
				return( p->in.left );

			case LTYPE:
				p->tn.op = FREE;
				return( NIL );
			}
		}
		return( p );
	}

	/* goal must = CCC from here on */

	switch( o ) 
	{

	case ULE:
	case ULT:
	case UGE:
	case UGT:
	case EQ:
	case NE:
	case LE:
	case LT:
	case GE:
	case GT:
		if(t<0 ) 
		{
			o = p->tn.op = negrel[o-EQ];
			t = f;
			f = -1;
		}
		if( p->in.right->in.op == ICON &&
		    p->in.right->tn.lval == 0 &&
		    p->in.right->in.name == (char *) 0 
		) {
			q1 = p->in.left;
			q2 = q1->in.left;
			if (	q1->in.op == CONV &&
				T_SIGNED(q1->in.type) &&
				T_UNSIGNED(q2->in.type) &&
				gtsize(q1->in.type) > gtsize(q2->in.type)
			) {
				switch(o) {
				case EQ:
				case NE:
					break;
				case LE:
					o = p->in.op = EQ;
					break;
				case LT:
					return gtb(p, f);
				case GE:
					return gtb(p, t);
				case GT:
					o = p->in.op = NE;
					break;
				}
			}

#ifndef NOOPT
			/* if chars are unsigned, do these optimizations
			   as if this is an unsigned compare*/
#ifndef CHSIGN
			if (
			    ( p->in.left->tn.type == TCHAR ||
			      ( p->in.left->in.op == CONV && 
			        p->in.left->in.left->tn.type == TCHAR ) )
			     && o >= LE && o <= GT)
				o += UGT - GT;
#endif

			/* the question here is whether we can assume that */
			/* unconditional branches preserve condition codes */
			/* if this turned out to be no, we would have to */
			/* explicitly handle this case here */

			switch( o ) 
			{

			case UGT:
			case ULE:
				o = p->in.op = (o==UGT)?NE:EQ;
				/*FALLTHRU*/
			case EQ:
			case NE:
			case LE:
			case LT:
			case GE:
			case GT:
				if(    logop( p->in.left->tn.op )
				    || p->in.left->in.op == QUEST )
				{
					/* situation like (a==0)==0
					** or ((i ? 0 : 1) == 0
					** ignore optimization
					*/
					goto noopt;
				}
				break;

			case ULT:  /* never succeeds */
				return( gtb( p, f ) );

			case UGE:
				/* always succeeds */
				return( gtb( p, t ) );
			}
			p->tn.op = p->in.right->tn.op = FREE;
			p = condit( p->in.left, NRGS, -1, -1 );
			p = genbr( o, t, p );
			if( f<0 ) return( p );
			else return( genbr( 0, f, p ) );
noopt: ;
# endif
		}

		p->in.left = condit( p->in.left, NRGS, -1, -1 );
		p->in.right = condit( p->in.right, NRGS, -1, -1 );
		p = genbr( o, t, p );
		if( f>=0 ) p = genbr( 0, f, p );
		return( p );

#ifdef	CG
	case BCMP:
			/*Don't call condit on the CM's that
			  hold these 'ternary' nodes together.*/
		p->in.left = condit(p->in.left, NRGS, -1, -1);
		p->in.right->in.left = condit(p->in.right->in.left, NRGS, -1, -1);
		p->in.right->in.right = condit(p->in.right->in.right, NRGS, -1, -1);
		if( t>=0 ) p = genbr( NE, t, p );
		if( f>=0 ) p = genbr( (t>=0)?0:EQ, f, p );
		return( p );

	case SEMI:
			/*RTOL semi for condition codes .
			  we must save a value, do the lhs, then
			  test the value*/
		if ( p->in.strat & RTOL)
		{
			p->in.right = condit(p->in.right, NRGS, -1, -1);
			p->in.left = condit(p->in.left, CEFF, -1, -1);

			/* The lhs may have vanished*/
			/* But, the rhs must still exist (it was for NRGS) */
			p = rcomma(p);

			/*Do the branches*/
			if( t>=0 ) p = genbr( NE, t, p );
			if( f>=0 ) p = genbr( (t>=0)?0:EQ, f, p );

			return p;
			
		}
		/*Normal (non-rtol) semicolons:*/
		/*FALLTHRU*/
#endif	/* def CG */

	case COMOP:
		p->in.left = condit( p->in.left, CEFF, -1, -1 );
		p->in.right = condit( p->in.right, CCC, t, f );
		return( rcomma( p ) );

	case NOT:
		p->tn.op = FREE;
		return( condit( p->in.left, CCC, f, t ) );

	case ANDAND:
		lf = f<0 ? getlab() : f;
		p->tn.op = COMOP;
		if (cond(p->in.left) > 0)
		{
			/* left is always true */
			p->in.left = condit( p->in.left, CEFF, -1, -1 );
			p->in.right = condit( p->in.right, CCC, t, f );
		}
		else  {
			/* lhs not always true */
			if (cond(p->in.right) > 0)
			{
				/* rhs is always true */
				p->in.right =
				   condit( p->in.right, CEFF, -1, -1 );
				if (p->in.right)  {
				    /* const with sideeffect */
				    p->in.left = 
					condit( p->in.left, CCC, -1,lf);
				    p->in.right = condit( p->in.right,
					CCC, t, t );
				} else
				    p->in.left =
				     condit( p->in.left, CCC, t, f );
			} else  {
				p->in.left =
				     condit( p->in.left, CCC, -1, lf );
				p->in.right =
				   condit( p->in.right, CCC, t, f );
			}
		}
		if( (q = rcomma( p )) != 0 ) 
		{
		    if( f<0 ) q = dlabel( q, lf );
		}
		return( q );

	case OROR:
		lt = t<0 ? getlab() : t;
		p->tn.op = COMOP;
		if (cond(p->in.left) == 0)
		{
			/* left is always false */
			p->in.left = condit( p->in.left, CEFF, -1, -1 );
			p->in.right = condit( p->in.right, CCC, t, f );
		}
		else  {
			/* left is not always false */
			if (cond(p->in.right) == 0)
			{
				/* right always false */
				p->in.right =
				   condit( p->in.right, CEFF, -1, -1 );
				if (p->in.right) { /* right side has side effect */
				    p->in.left = 
					condit( p->in.left, CCC, lt,-1);
				    if (f >= 0)		/* if fall-thru, no branch */
					p->in.right = genbr( 0, f, p->in.right );
				}
				else
				    p->in.left =
				        condit( p->in.left, CCC, t, f );
			} else  {
				p->in.left =
				     condit( p->in.left, CCC, lt, -1 );
				p->in.right =
				   condit( p->in.right, CCC, t, f );
			}
		}
		if( (p = rcomma( p )) != 0 ) 
		{
		    if( t<0 ) p = dlabel( p, lt );
		}
		return( p );

	case QUEST:
		lf = f<0 ? getlab() : f;
		lt = t<0 ? getlab() : t;
		p->in.left = condit( p->in.left, CCC, -1, l=getlab() );
		q = p->in.right;
		q1 = condit( q->in.left, goal, lt, lf );
		q->in.left = dlabel( q1, l );
		q->in.right = condit( q->in.right, goal, t, f );
		p->tn.op = COMOP;
		q->tn.op = COMOP;
		if( t<0 ) p = dlabel( p, lt );
		if( f<0 ) p = dlabel( p, lf );
		return( p );

	default:
		/* get the condition codes, generate the branch */
		switch( optype(o) )
		{
		case BITYPE:
			p->in.right = condit( p->in.right, NRGS, -1, -1 );
			/*FALLTHRU*/
		case UTYPE:
			p->in.left = condit( p->in.left, NRGS, -1, -1 );
		}
		if( t>=0 ) p = genbr( NE, t, p );
		if( f>=0 ) p = genbr( (t>=0)?0:EQ, f, p );
		return( p );
	}
}
void
p2compile( p )
register NODE *p; 
{

	NODE *dolocal();

	if( lflag ) lineid( lineno, ftitle );
#ifdef CG
	nins = 0;	/*Must do this here for costing */
			/*Do special local rewrites*/
	p = dolocal(p);
			/*Check for special nail nodes (standalones) */
	if ( p2nail(p) ) 
		return;
			/*Remember where temps start*/
	tmpoff = tmp_start;
#else
	tmpoff = 0;  /* expression at top level reuses temps */
#endif

	/* generate code for the tree p */

# ifdef MYREADER
	MYREADER(p);  /* do your own laundering of the input */
# endif
	/* eliminate the conditionals */
# ifndef NODBG
	if( p && odebug>2 ) e2print(p);
# endif
	p = condit( p, CEFF, -1, -1 );
	if( p ) 
	{
#ifdef	MYP2OPTIM
    	        NODE * MYP2OPTIM();
		p = MYP2OPTIM(p);	/* perform local magic on trees */
#endif
		/* expression does something */
		/* generate the code */
# ifndef NODBG
		if( odebug>2 ) e2print(p);
# endif
		codgen( p );
	}
# ifndef NODBG
	else if( odebug>1 ) PUTS( "null effect\n" );
# endif
	allchk();
	/* tcheck will be done by the first pass at the end of a ftn. */
	/* first pass will do it... */
}

#ifndef  CG
p2bbeg( aoff, myreg ) 
register aoff;
#ifdef	REGSET				/* in RCC, myreg is register bit vector */
RST myreg;
#else
register myreg;
#endif
{
	static int myftn = -1;
	SETOFF( aoff, ALSTACK );
	if( myftn != ftnno )
	{
		 /* beginning of function */
		maxboff = aoff;
		myftn = ftnno;
		maxtemp = 0;
		maxarg = 0;
	}
	else 
	{
		if( aoff > maxboff ) maxboff = aoff;
	}
# ifdef SETREGS
	SETREGS(myreg);
# endif
}

p2bend()
{
	SETOFF( maxboff, ALSTACK );
	SETOFF( maxarg, ALSTACK );
	SETOFF( maxtemp, ALSTACK );
	eobl2();
	maxboff = maxarg = maxtemp = 0;
}
#endif

#ifndef NODBG
static char *cnames[] = 
{
	"CEFF",
	"NRGS",
	"CCC",
	0,
};
void
prgoal( goal ) 
register goal;
{
	/* print a nice-looking description of goal */

	register i, flag;

	flag = 0;
	for( i=0; cnames[i]; ++i )
	{
		if( goal & (1<<i) )
		{
			if( flag ) PUTCHAR( '|' );
			++flag;
			PUTS( cnames[i] );
		}
	}
	if( !flag ) fprintf(outfile, "?%o", goal );

}

void
e2print( p )
register NODE *p; 
{
#ifdef CG
	cgprint(p,0);
	return;
#else
#ifdef	STINCC
	PUTS( "\n*********\n" );
#else
	PUTS( "\n********* costs=(0,...,NRGS;EFF;TEMP;CC)\n" );
#endif
	e22print( p ,"T");
	PUTS("=========\n");
#endif /*def CG */
}
void
e22print( p ,s)
register NODE *p; 
char *s;
{
	static down=0;
	register ty;

	ty = optype( p->tn.op );
	if( ty == BITYPE )
	{
		++down;
		e22print( p->in.right ,"R");
		--down;
	}
	e222print( down, p, s );

	if( ty != LTYPE )
	{
		++down;
		e22print( p->in.left, "L" );
		--down;
	}
}
void
e222print( down, p, s )
NODE *p; 
char *s;
{
	/* print one node */
	int d;

	for( d=down; d>1; d -= 2 ) PUTCHAR( '\t' );
	if( d ) PUTS( "    " );

	fprintf(outfile, "%s.%d) op= '%s'",s, (int)node_no(p), opst[p->in.op] );
#ifdef CG
	prstrat(p->in.strat);	/*print the strategy field*/
#endif
	switch( p->in.op ) 
	{
		 /* special cases */
	case REG:
		fprintf(outfile, " %s", rnames[p->tn.rval] );
		break;

	case ICON:
	case NAME:
	case VAUTO:
	case VPARAM:
	case TEMP:
		PUTCHAR(' ');
		adrput( p );
		break;

	case STCALL:
	case UNARY STCALL:
		fprintf(outfile, " args=%d", p->stn.argsize );
	case STARG:
	case STASG:
		fprintf(outfile, " size=%d", p->stn.stsize );
		fprintf(outfile, " align=%d", p->stn.stalign );
		break;

	case GENBR:
		fprintf(outfile, " %d (%s)", p->bn.label, opst[p->bn.lop] );
		break;

	case CALL:
	case UNARY CALL:
#ifdef IN_LINE
	case INCALL:
	case UNARY INCALL:
#endif
		fprintf(outfile, " args=%d", p->stn.argsize );
		break;

	case GENUBR:
	case GENLAB:
	case GOTO:
#ifdef	CG
	case LABELOP:
	case JUMP:
#endif
		fprintf(outfile, " %d", p->bn.label );
		break;

	case FUNARG:
		fprintf(outfile, " offset=%d", p->tn.rval );
		break;

	case FCON:
		fprintf(outfile," %lf", p->fpn.dval);
		break;

        case FLD:
                fprintf(outfile, " rval = 0%o", p->tn.rval);
                break;

#ifdef CG
			/*Print out CG nodes:*/
        case COPYASM:
        case COPY:

                if ( p->in.name && p->in.name[0] )
                        fprintf(outfile, " '%s'", p->in.name);
                else
                        PUTS( " (null)" );
                break;

        case BEGF:
	case RSAVE:
	case RREST:
                {
                int i;
                        for (i=0; i<TOTREGS; ++i)
                        {
                                if ( ((char *)(p->in.name))[i])
                                        fprintf(outfile, "%d,",i);
                        }
                }
                break;

        case ENDF:
                fprintf(outfile, " autos=%d, regs=", p->tn.lval);
                break;

        case LOCCTR:
                fprintf(outfile, " locctr=%d",p->tn.lval);
                break;


	case SWCASE:
		fprintf(outfile, " [%d-%d],default %d", p->tn.lval, p->tn.rval, p->bn.label);
		break;

	case DEFNAM:
		fprintf(outfile, "%s, flags=0%o, size=%d", p->tn.name, p->tn.lval,p->tn.rval);
		break;

	case UNINIT:
		fprintf(outfile, "%d units", p->tn.lval);
		break;

	case SINIT:
		fprintf(outfile, " '%s' (length %d)",p->tn.name,p->tn.lval);
		break;

	case FCONV:
		fprintf(outfile, " autos=%d", p->tn.lval);
		break;

	case LET:
	case CSE:
		fprintf(outfile, " id=%d", p->csn.id);
		break;
#endif /* def CG */

	}

	PUTS( ", " );
#ifdef	CG
	if (p->in.strat & OCOPY)
	    PUTS( "<Ocopy>,");
#endif
	t2print( p->in.type );
#ifndef	STINCC
	PUTS( ", c=[" );
	{
	    register int i;
	    for( i=0; i<NRGS; ++i ) cprt( p->in.cst[i], "," );
	}
	cprt( p->in.cst[NRGS], "; " );
	cprt( p->in.cst[CEFF], "; " );
	cprt( p->in.cst[CTEMP], "; " );
	cprt( p->in.cst[CCC], "]" );
#endif
	switch( p->tn.goal ) {
	case CEFF:	PUTS( " (EFF)" ); break;
	case CCC:	PUTS( " (CC)" ); break;
	case 0:		PUTS( " (NULL)" ); break;
	default:	if (p->tn.goal != NRGS)
			    fprintf(outfile, "(BAD GOAL: %d)\n", p->tn.goal );
			break;
	}
	PUTCHAR( '\n' );
}

t2print( t )
TWORD t;
{
	int i;
	static struct {
		TWORD mask;
		char * string;
		} t2tab[] = {
			TANY, "ANY",
			TINT, "INT",
			TUNSIGNED, "UNSIGNED",
			TCHAR, "CHAR",
			TUCHAR, "UCHAR",
			TSHORT, "SHORT",
			TUSHORT, "USHORT",
			TLONG, "LONG",
			TULONG, "ULONG",
			TFLOAT, "FLOAT",
			TDOUBLE, "DOUBLE",
			TPOINT, "POINTER",
			TPOINT2, "POINTER2",
			TSTRUCT, "STRUCT",
#ifdef	CG
			TFPTR, "TFPTR",
#endif
			TVOID, "VOID",
			0, 0
			};

	for( i=0; t && t2tab[i].mask; ++i ) {
		if( (t&t2tab[i].mask) == t2tab[i].mask ) {
			fprintf(outfile, " %s", t2tab[i].string );
			t ^= t2tab[i].mask;
			}
		}
	}

#ifndef STINCC
static void
cprt( c, s )
register char *s; 
register c;
{
	if( c >= INFINITY ) fprintf(outfile, "*%s", s );
	else fprintf(outfile, "%d%s", c, s );
}
#endif

# else	/* ndef NODBG */
/*ARGSUSED*/
void
e2print( p )
NODE *p; 
{
	werror( "e2print not compiled" );
}
/*ARGSUSED*/
void
e222print( i, p, s )
NODE *p; 
char *s;
{
	werror( "e222print not compiled" );
}

# endif
