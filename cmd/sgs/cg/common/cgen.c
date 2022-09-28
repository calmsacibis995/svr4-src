/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/cgen.c	1.18"

# include "mfile2.h"
# include <malloc.h>
# include <memory.h>

static void rewlhs(), cfix(); 
static NODE *ind2type();

# define istnode(p) (p->in.op==REG && istreg(p->tn.rval))

/*
** For God's workes are, like him, all infinite
** And curious search but craftie sin's delight.
*/
void  
rewcom( p, goal )
NODE *p; 

{
	/* find all , ops, move as high as is legal */
	/* rewrite p in place; this takes some doing! */
	/* while we are at it, take care of setting the goal field */
	int o, ty, g1, g2;
	NODE *l, *r, *ql, *qr;
#ifdef	CG
	NODE *last;
	int paren_flag;
	/* int changed = 0;	flag: 1 = we did a rewrite*/
#endif

	o = p->tn.op;
	g1 = g2 = NRGS;
	p->tn.goal = goal;

	/* special cases for subtrees:
	** GENBR has left as Condition codes
	** COMOP has left as Effects
	** COLON, GENUBR, CM, GENLAB have descendents = node
	** CALL, STCALL, FORTCALL has right as Effects
	** All others use registers
	*/

	switch( o )
	{

	case FREE:
		cerror( "rewcom(%d) is FREE", node_no(p) );
		/*FALLTHRU*/
	case GENBR:
		g1 = CCC;
		break;

#ifdef	CG
	case LET:
		g2 = goal;		/* right-side goal */
		break;

	case SEMI:
			/*Special case: a rightfirst semi,
			  done for effects, needs to get a value,
			  do the left side, then test the value*/	
			/*Cfix will do the test of the value*/
		if ( firstl(p) == p->in.right && goal == CCC)
		{
			p->tn.goal = goal = NRGS;
		}
		/*FALLTHRU*/
#endif
	case COMOP:
		g1 = CEFF;
		g2 = goal;
		break;

	case COLON:
	case GENUBR:
	case CM:
	case GENLAB:
		g1 = g2 = goal;
		break;

	case CALL:
#ifdef IN_LINE
	case INCALL:		/* handle asm calls like regular calls */
#endif
	case STCALL:
	case FORTCALL:
		g2 = CEFF;
		break;
	}


	switch( ty = optype(o) )
	{

	case BITYPE:
		rewcom( r = p->in.right, g2 );
		/*FALLTHRU*/
	case UTYPE:
		rewcom( l = p->in.left, g1 );
		break;
	case LTYPE:
		return;
	}

	if( o==COMOP || o==COLON || o==GENLAB ) return;

#ifdef CG
	/*If a comma is trying to go past the last side of an ordered node,
	  rewrite it into a semi*/
	if ( (last = lastl(p)) != NULL && last->in.op == COMOP)
	{
		uncomma(last);
	}
	/*If a comma is trying to go past a parened node, it gets just past,
	  but use a LTOR, Parened semi instead of a comma. This prevents it
	  from going any further.*/

	paren_flag = (p->in.strat & PAREN);
	
#endif

	/* look for (A,B) op C and A op (B,C), and rewrite */
	/* A,B if A headed by GENBR can't be rewritten */
	/* the assumption is that B is executed immediately after A, */
	/* and this won't necessarily be true if op is commutative */

	if( l->tn.op == COMOP && l->in.left->tn.op != GENBR ) 
	{
		/* rewrite it... */
		/* (A,B) op C => A,(B op C) */
		/* also, for unary ops,  op (A,B) => A , (op B)  */
		ql = l->in.left;
		qr = l->in.right;
		*l = *p;  /* copies op, and other stuff if op is unary */
		l->in.left = qr;
		p->in.right = l;
		p->in.left = ql;
		p->tn.op = COMOP;
#ifdef CG
		if ( paren_flag)
		{
			uncomma(p);
		}
#endif
		rewcom( p, p->tn.goal );
	}
			/*The above rewrite might have put
			  a COMOP in p; must check again*/
	/* Can't rewrite right-side past the current node if the current
	** node is a COMOP, COLON, or GENLAB, or if there's no right node.
	*/
	if (   ty == UTYPE
	    || (o = p->in.op) == COMOP || o == COLON || o == GENLAB
	   ) return;

	/* Cheat a bit:  don't move COMOP on right side (ARG side)
	** of CALL OPs; this permits side effects within ARGs to
	** be done around the time the ARG is evaluated, since the
	** ARG is always done for effect.
	*/
	if( r->tn.op == COMOP && r->in.right->tn.op != GENBR && !callop(o))
	{
		/* rewrite, again */
		/* A op (B,C) => B,(A op C) */
		/* op is not unary now */
		ql = r->in.left;
		qr = r->in.right;
		*r = *p;
		p->tn.op = COMOP;
#ifdef CG
		if ( paren_flag)
		{
			uncomma(p);
		}
#endif
		p->in.left = ql;
		r->in.right = qr;
		rewcom( p, p->tn.goal );
	}
}

static void 
rewlhs(p)
NODE *p; 

{
	/* rewrite x op= y as (x op= y),x */
	/* it would be really nice to optimize after doing this . . . */
	NODE *q, *t;
	q = talloc();
	*q = *p;
	t = tcopy( p->in.left );
	p->in.left = q;
	p->in.right = t;
	p->tn.op = COMOP;
#ifdef	CG
	p->in.strat = 0;
#endif
	return;
}
int
rewsto(p)
NODE *p; 
{
	/* a temp, t, is generated, and p is rewritten as ((t=p),t) */
	/* if p has the form x op= A, and x is of the right form, rewrite
	/* as ((x op= A), x) */

	int o, ao;
	NODE *t, *q;
	/* probably not perfect for structs: CHECK UP.. */

	while( (o=p->tn.op) == COMOP ) p = p->in.right;
	if( o == TEMP ) return(0);  /* nothing to do */
#ifdef CG
			/*If this is a list of ;lp's, with a temp at the
			  bottom, don't do an assignment, just use the
			  generated temp.*/
	if (semilp(p) && rewsemi(p))
		return 1;

        /* rewriting a parened node removes the parens restriction*/
        p->in.strat &= ~PAREN;
#endif
	if( o == STARG ) 
	{
		 /* store a structure argument */		/* like storing a usual argument, but we have addresses */

		t = talloc();
		*t = *p->in.left;  /* copy contents, mainly for type, etc. */
#ifdef CG
		t->in.strat = 0;	/*clear p's strategy bits */
#endif
		q = talloc();
		*q = *t;
		t->tn.op = TEMP;
		t->tn.lval = freetemp(argsize(p)/SZINT );
		t->tn.lval = BITOOR(t->tn.lval);
		t->tn.name = (char *) 0;
		t->tn.type = TSTRUCT;
		q->tn.op = UNARY AND;
		q->in.left = t;
		/* now, q has & TEMP */
		t = talloc();
		*t = *p;
		t->in.left = q;
		t->in.right = p->in.left;
		t->tn.op = STASG;
		/* now, t has (&TEMP) = struct */
		p->in.left = talloc();
		p->in.left->tn.op = COMOP;
		p->in.left->in.left = t;
		p->in.left->in.right = t = talloc(); /* copy q here */
		*t = *q;
		t->in.left = talloc();
		*t->in.left = *q->in.left;
		/* finally, have (&TEMP = struct),(&TEMP) */
		/* this should do it: whew! */
#ifndef NODBG
		if( odebug>1 ) e2print( p );
#endif
		return( 1 );
	}

#ifndef NODBG
	if( odebug>1 ) 
	{
		e2print( p );
		printf( "\nrewritten by rewsto as:\n" );
	}
#endif
	if( asgop(o) && o!=INCR && o!=DECR && lhsok( p->in.left ) ) {
		/* x op= y turns into (x op= y), x */
#ifdef CG
		/*Exception: if OCOPY is set, must actually copy the subtree*/
		if ((!(p->in.strat & OCOPY)) || p->in.left->in.op == TEMP
		|| istnode(p->in.left) )
#endif
		{
		rewlhs( p );
#ifndef NODBG
		if( odebug>1 ) e2print( p );
#endif
		return( 1 );
		}
	}
	ao = ASG o;
	if( asgbinop(ao) ) 
	{
		if( p->in.left->tn.op == TEMP ) 
		{
			p->tn.op = ao;
#ifdef	CG
			unorder(p);
#endif
			rewlhs( p );
#ifndef NODBG
			if( odebug>1 ) e2print( p );
#endif
			return( 1 );
		}
	}

	/* to rewrite in place, p becomes a COMOP; rhs is the temp, lsh
	** /* is t = p, where p has been converted to the intermediate type 
	*/
	/* after some debate, the type of the temp will be the type of p */

	t = talloc();
	*t = *p;  /* copy contents, mainly for type, etc. */
	q = talloc();
	*q = *p;
	t->tn.op = TEMP;
	t->tn.lval = freetemp(argsize(p)/SZINT );
	t->tn.lval = BITOOR(t->tn.lval);
	t->tn.name = (char *) 0;
	q->tn.op = ASSIGN;
	q->in.left = t;
	q->in.right = talloc();
#ifdef	CG
	q->in.strat = t->in.strat = 0;
#endif
	*(q->in.right) = *p;
		/* now, q has (t=p) */
	p->in.right = talloc();
	*(p->in.right) = *t;
	p->tn.op = COMOP;
	p->in.left = q;
	/* this should do it: whew! */
#ifndef NODBG
	if( odebug>1 ) e2print( p );
#endif
	return( 1 );
}

iseff( p )
NODE *p; 

{
	/* return 1 if p has some side effects, 0 otherwise */
	/* If DOEXACT is set, assume that anything might have side effects*/
	int o;
	o = p->tn.op;
#ifdef CG
	if( (p->in.strat & DOEXACT) || callop(o) || asgop(o))
		return( 1 );
	switch(o)
	{ 
	case COPY:
	case COPYASM:
	case BMOVEO:
	case BMOVE:
	case RETURN:
	case JUMP:
	case GOTO:
	case VLRETURN:
	case NPRETURN:
	case EXSETV:
	case EXCLEAR:
	case EXTEST:
	case EXRAISE:
	case RREST:
	case RSAVE:
	case CAPRET:
			return( 1 );
	}
#else
	if( callop(o) || asgop(o) ) return( 1 );
#endif
	switch( optype( o ) )
	{
	case BITYPE:
		if( iseff( p->in.right ) ) return( 1 );
		/*FALLTHRU*/
	case UTYPE:
		return( iseff( p->in.left ) );
	}
	return( 0 );
}

NODE *
lhsto( p )
NODE *p; 

{
	/* find a piece of the LHS to be stored */
	/* if found, rewrite tree */
	NODE *q;
	int o;

	for( q = p->in.left; (o=q->tn.op)!=STAR; q=q->in.left )
	{
		if( optype(o) == LTYPE ) return( (NODE *)0);
	}
	/* q is now the * node, if there one */
	q = q->in.left;
	o = q->tn.op;
#ifdef CG
			/*Must still rewrite if this is the first side*/
        if( optype(o) == LTYPE && !(p->in.strat & LTOR) ) return( (NODE *)0 );
#else
	if( optype(o) == LTYPE ) return( (NODE *)0 );
#endif
	else return( q );
}

static int
c2bigger( p ) NODE *p; {
	/* p is a conversion op; does it make things bigger */
	register TWORD t, tl;

	t = p->tn.type;
	tl = p->in.left->tn.type;

	if( (t|tl)&TPOINT ) return( 0 );  /* pointers are funny */
#ifdef	CG
	if( (t|tl)&TFPTR ) return( 0 );	/* so are frame pointers */
#endif
	if( t&TDOUBLE ) return( 1 );
	if( tl&TDOUBLE ) return( 0 );
	if( t&TFLOAT ) return( 1 );
	if( tl&TFLOAT ) return( 0 );
	if( t&(TLONG|TULONG) ) return( 1 );
	if( tl&(TLONG|TULONG) ) return( 0 );
	if( t&(TINT|TUNSIGNED) ) return( 1 );
	if( tl&(TINT|TUNSIGNED) ) return( 0 );
	if( t &(TSHORT|TUSHORT) ) return( 1 );
	return( 0 );
	}

static NODE *
ind2type( p )
register NODE *p; 

{
	/* make the type of p be the appropriate type for an argument */
	register TWORD t;
	NODE *q;

	t = p->tn.type;
	if( t == TCHAR || t == TSHORT ) t = TINT;
	else if( t == TUCHAR || t == TUSHORT ) t = TUNSIGNED;
	/* For CG, we can pass floats as floats */
#ifndef	CG
	else if( t == TFLOAT ) t = TDOUBLE;
#endif
	else return( p );

	if( p->tn.op == CONV && c2bigger(p) ) 
	{
		p->tn.type = t;
		return( p );
	}
	q = talloc();
	q->tn.op = CONV;
	q->in.left = p;
	q->in.right = 0;
	q->tn.name = (char *) 0;
	q->tn.type = t;
	q->tn.goal = NRGS;
	return( q );
}

void
reweop( p )
register NODE *p; 

{
	/* Rewrite A OP= B as A = A OP B.
	** Also, rewrite (CONV A) OP= B as A = (CONV ( (CONV A) OP B ) )
	** Rewritten in place.
	** On input, the type of the OP= equals the type of A.
	** The type of the OP node on output is the type of B,
	** the type of the = node on output is the type of A.
	** EXCEPT:
	**	>>= and <<= always have a right side of INT.
	**	Therefore, the type of OP (<< or >>) is the type
	**	of the left!
	*/

	register NODE *q, *t;
	register TWORD ty;

#ifndef NODBG
	if( odebug>1 ) 
	{
		e2print( p );
		printf( "\nrewritten by reweop as:\n" );
	}
#endif
	/* rewrite tree with duplicate left subtree in new right subtree */
	/* there is an implicit q->in.right = p->in.right in  *q = *p */
	q = talloc();
	*q = *p;	
	q->in.left = tcopy( p->in.left );
	if( p->in.left->tn.op == CONV ) 
	{
		/* ( CONV (A) ) op= B becomes A = CONV ( CONV(A) op B ) */
		/* the op is done to the type of B */
		/* the assignment is done to the type of A */
		t = p->in.left;
		ty = t->in.left->tn.type;
		p->in.left = t->in.left;
		p->in.right = t;
		t->in.left = q;
		/* now, have the tree built; fix the types */
		t->tn.type = ty;
	}
	else 
	{
		p->in.right = q;
	}
	/* NOTE: no =ops for structures... */
	p->tn.op = ASSIGN;
	q->tn.op = NOASG q->tn.op;
	p->tn.type = p->in.left->tn.type;
	if (q->tn.op != LS && q->tn.op != RS)
	    q->tn.type = q->in.right->tn.type;
#ifndef NODBG
	if( odebug>1 ) e2print( p );
#endif
}

rewass( p )
NODE *p; 

{
	NODE *q;
#ifdef	CG
	NODE * rewrite;
#endif
	int o;
	/* look for =ops to be rewritten */

#ifndef NODBG
	if( odebug ) 
	{
		printf( "rewass called with:\n" );
		e2print( p );
	}
#endif
	o = p->tn.op;
	if( o == UNARY AND ) 
	{
		if( p->in.left->tn.op == RNODE ) 
		{
			/* this should happen only with structure returns */
			q = p->in.left;
			q->tn.op = ICON;
			*p = *q;
			q->tn.op = FREE;
			return(0);  /* keep going in costs */
		}
		/* this case should happen only with short structures */
		(void) rewsto( p->in.left );
		/* & f() has turned into & ( t=f(),t) */
#ifndef NODBG
		if( odebug ) 
		{
			printf( "\nrewritten by rewass as:\n" );
			e2print( p );
		}
#endif
		return(1);
	}
	if( !asgop(o) || o==ASSIGN ) 
	{
		if( o==ASSIGN ) 
		{
			/* look for funny nodes on lhs */
			o = p->in.left->tn.op;
			if( o==RNODE || o==QNODE || o==SNODE ) 
			{
				/* force into r0 */
				p->in.left->tn.op = REG;
				p->in.left->tn.rval = callreg( p->in.right );
#ifndef NODBG
				if( odebug ) 
				{
					printf( "funny node redone\n" );
					e2print(p);
				}
#endif
				return(0);
			}
		}
		else if (optype(o) != LTYPE)
		{
			int changed;
			TWORD olt = p->in.left->tn.type;
			/* this case is, for example, 
				unsigned char a, b;
				...   a*b
			/* we convert both to a reasonable type */
			/* the result is assumed to be automatically
			/* converted downwards if it should be... */

			p->in.left = ind2type( p->in.left );
			changed = p->in.left->in.type != olt;
			if (optype(o) == BITYPE) {
			    TWORD ort = p->in.right->in.type;
			    p->in.right = ind2type( p->in.right );
			    changed |= (p->in.right->in.type != ort);
			}
#ifndef NODBG
			if( odebug ) {
				printf( "conversions inserted" );
				e2print(p);
			}
#endif
			/* if this didn't work, we are in trouble */
			if( changed ) {
				/* we have changed something */
				return( 0 );
			}
		}
#ifdef CG
			/*Cannot match this node at all.
			  CG has some last ditch stuff it can do:*/

		if ( (rewrite = firstl(p)) != NULL)
		{
			/*otherwise, if this is an ordered node, rewrite
			  the first child to temp*/
			unorder(p);
#ifndef NODBG
			if ( odebug)
			{
				fprintf(outfile, "Rewriting first side:\n");
				e2print(p);
			}
#endif
			if (rewsto(rewrite))
				return REWROTE;	/*major rewrite*/
		}
#endif
			/*Give up!*/
		e2print(p);
		cerror( "can't deal with op %s", opst[o] );
	}
	if( o == INCR || o == DECR ) 
	{
		/* very crude: a++ becomes (a+=1)-1 */
#ifndef NODBG
		if( odebug>1 ) 
		{
			e2print( p );
			printf( "\nrewritten by rewass as:\n" );
		}
#endif
		if( p->in.goal == CEFF )
		{
			p->in.op = ((o==INCR)?ASG PLUS:ASG MINUS);
		}
		else
		{
			/* rewrite tree with duplicate left subree in new
			/* right subtree...
			/* there is an implict q->in.left = p->in.left in *q = * p
			*/
			q = talloc();
			*q = *p;	
			q->in.right = tcopy( p->in.right );
			p->in.left = q;
			q->tn.op = ((o==INCR)?ASG PLUS:ASG MINUS);
			p->tn.op = ((o==INCR)?MINUS:PLUS);
			if (q->in.left->in.op == FLD)
			{
			    int size;
                            int fldsz = UPKFSZ(q->in.left->tn.rval);
			    int t = q->in.left->in.type;

			    if (t & (TINT | TUNSIGNED))
				size = SZINT;
			    else if (t & (TSHORT | TUSHORT))
				size = SZSHORT;
			    else if (t & (TCHAR | TUCHAR))
				size = SZCHAR;
			    else if (t & (TLONG | TULONG))
				size = SZLONG;
			    else
				cerror("invalid type for bitfield");

			    if (fldsz != size)
			    {

				NODE* r;

				r = talloc();
				*r = *p;
				r->in.left = p->in.left;
				r->in.right = p->in.right;
				p->in.left = r;
				p->in.op = AND;

				r = talloc();
			    	r->in.op = ICON;
			    	r->tn.type = t;
				r->tn.lval = ~( ~0L << fldsz); /* mask fldsz */
				r->tn.rval = 0;
			    	p->in.right = r;
			    }
			}
		}
#ifndef NODBG
		if( odebug ) 
		{
			printf( "\nrewritten by rewass as:\n" );
			e2print( p );
		}
#endif
		return(1);
	}
	/* find out if some subtree has to be stored into a temp... */
	if( (q = lhsto(p)) != 0 )  
	{
		if( !rewsto( q ) ) cerror( "rewass0" );  /* q => t=q,t */
		rewcom( p, p->tn.goal );  /* move COMOP to the top */
		if( p->tn.op != COMOP ) cerror( "rewass1" );
		if( !asgop( p->in.right->tn.op ) ) cerror( "rewass2" );
		reweop( p->in.right );
	}
	else 
		reweop( p );  /* rewrite p as an =OP */
	return(1);
}

# ifdef NONEST
subcall( p )
register NODE *p; 

{
	/* return 1 if p contains a callop */
	register o, t;

	o = p->tn.op;
			/*For CG, only reject these if the NOFNEST is set*/
#if !defined(CG) || defined(NOFNEST)
	if( callop(o)
# ifndef UCALLBAD
	    && o!=UNARY CALL
# ifdef IN_LINE
	    && o!=UNARY INCALL	/* pretend asm calls are regular calls */
# endif
# endif
# ifndef STCALLBAD
	    && o!=UNARY STCALL
# endif
# ifndef UFCALLBAD
	    && o!=UNARY FORTCALL
# endif
	    ) return( 1 );
#endif	/* !defined(CG) || defined(NOFNEST) */
#ifdef CG
#  ifdef NOANEST
			/*Also for CG: Can't nest ALLOCS*/
	if ( o == ALLOC)
		return( 1 );
#  endif
			/*Can't nest function calls EVER if the function
                          returns via NPRETURN or VLRETURN.  This is true
                          iff  the strategy has CHGSTK set.*/
        if (callop(o) && (p->in.strat & CHGSTK) )
                return( 1 );
#endif
	t = optype( o );
	if( t==BITYPE && subcall(p->in.right) ) return( 1 );
	if( t!=LTYPE ) return( subcall( p->in.left ) );
	return( 0 );
}

nonest( p )
register NODE *p; 

{
	register o, t;
	/* right now, this is very crude */
	/* find arguments below a call; store them */
	/* nonest is called up to a call; stocm is called within args */

	o = p->tn.op;
	t = optype( o );
	if( o==CALL || o==STCALL || o==FORTCALL) 
	{
	    /* A common rewrite for ++/-- can sometimes leave a ,OP
	    ** where a CM node should be.  Take care of the right
	    ** side of the ,OP first, then the left, where the args are.
	    */
	    NODE * q = p->in.right;

	    if (q->in.op == COMOP) {
		nonest( q->in.right );
		q = q->in.left;
	    }

	    stocm( q );
	    nonest( p->in.left );
	    return;
	}
	if( t == BITYPE ) nonest( p->in.right );
	if( t != LTYPE ) nonest( p->in.left );
}

stocm( p )
register NODE *p; 

{
	/* all call arguments below p must be stored */
	register NODE *q;
	register o;

	while( (o=p->tn.op) == CM )
	{
		stocm( p->in.right );
		p = p->in.left;
	}

	if( o != STARG && o != FUNARG ) cerror( "stocm" );

	q = p->in.left;
	if( subcall( q ) ) 
	{
		if( o == FUNARG ) rewsto( q );
		else 
		{
			/* structure argument with call beneath */
			(void) rewsto( p );
		}
		/* now q will be done outside of a call, so use nonest */
		nonest( q );
	}
}
# endif

#ifndef NODBG
outshp( pp )
SHAPE **pp; 

{
	SHAPE *p;

	if (pp == 0)
		return;

	for( ; p = *pp; ++pp )
	{
		printf("\t\t");
		shpr(p);
#ifdef	STINCC
		putchar('\n');
#else
		printf( " (%d)\n", p->sc );
#endif
	}
}

tabpr()
{
	register	OPTAB	*p;
	for (p =table; ;p++)
	{
		printf("Dump of table[%d] (stinline %d)\n", p-table, p->stinline );
		printf("\top = %s\n", opst[p->op]);
		printf("\tnextop = %d\n", p->nextop?p->nextop-table:-1 );
		printf("\tlshape = %d\n", p->lshape-pshape);
		printf("\tltype = 0%o\n", p->ltype);
		printf("\trshape = %d\n", p->rshape-pshape);
		printf("\trtype = 0%o\n", p->rtype);
		printf("\tneeds = %d\n", p->needs);
		printf("\trewrite = %d\n", p->rewrite);
		printf("\tcstring = %s", p->cstring);
#ifndef	STINCC
		printf("\tcost = %d\n", p->cost);
#endif
		printf("\tLeft:\n");
		outshp(p->lshape);
		printf("\tRight:\n");
		outshp(p->rshape);
		printf("\n");
	}
}
#endif

void
codgen( p )
NODE *p; 

{

	/* generate the code for p; */
	int i, flag;

#ifndef NODBG
	if (odebug > 5)
	{
		tabpr();
		/* NOTREACHED */
	}
#endif

# ifndef IN_LINE	/* nonest interferes with asm expansion */
# ifdef NONEST
	nonest(p);
# endif
# endif

	/* if we make drastic changes to the tree (e.g., introduce temps)
	** /* we will go back and do the whole thing again 
	*/
	/* statistics indicate that this happens about 10% of the time */
	/* if the percentage rises, there are many things that can be done to
	** /* improve matters 
	*/
	/* for example, RNODE, etc., could be removed by reader, and some of
	** /* the op rewriting could be discovered by reader as well 
	*/

again:

#ifdef	CG
			/* various typechecking ops; also, set the
			   OCOPY flags*/
	(void)tnumbers(p);
	typecheck(p,0);
#endif

	/* move the comma ops as high as practical */

	rewcom( p, CEFF );
#ifdef	CG
			/*Must do it again - we rewrote the tree*/
	if (tnumbers(p))
		goto again;

	cse_ptr = cse_list;
#endif

#ifndef NODBG
	if( odebug ) 
	{
		printf( "After goals are computed:" );
		e2print( p );
	}
#endif


#ifndef	STINCC

	/* compute the costs */

	if( costs( p ) ) goto again;  /* if rewritten, do again */

#ifndef NODBG
	if( odebug ) 
	{
		printf( "After costs are computed:" );
		e2print( p );
	}
#endif

	/* do a trial code generation */
	nins = 0;
	insout( p, CEFF );
#else	/* STINCC */

#ifdef	CG
			/*Parens at highest node are meaningless*/
	p->in.strat &= ~PAREN;
#endif
	/* do a trial code generation */
	nins = 0;
	switch (INSOUT( p, CEFF ))
	{
	case REWROTE:
		goto again;
	case OUTOFREG:
		e2print( p );
		cerror("Runs out of registers");
	/*default: fall thru*/
	}

#endif	/* ndef STINCC */

	/* rewrite stored subtrees as assignments to temps, with COMOP's */
	flag = 0;
	for( i=0; i<nins; ++i ) 
	{
		if( inst[i].goal == CTEMP ) 
		{
#ifndef NODBG
			if( odebug ) 
			{
				printf( "subtree is stored in temp:\n" );
				e2print( inst[i].p );
			}
#endif
			if( rewsto( inst[i].p ) ) {
				if( !fast ) goto again;
				/* otherwise, rewrite all temps now */
				flag = 1;
			}
		}
	}
	if( flag ) goto again;

#ifndef NODBG
	if( odebug ) e2print(p);
#endif
	/* if not just costing, output the actual instructions */
#ifdef	CG
	if (!costing)
#endif
	    insprt();
}

static INST inst_init[INI_NINS];

TD_INIT(td_inst, INI_NINS, sizeof(INST), 0, inst_init, "instruction table");

/* REGSET passes two extra arguments to cfix() */
#ifdef	REGSET
#  define CFIX(a,b) cfix(a, b, pi->rs_want, pi->rs_avail)
#else
#  define CFIX(a,b) cfix(a, b)
#endif

void
insprt()
{

	int i, ninsav;
	register INST *pi;
	register NODE *p;
	register OPTAB *q;
	register c, goal;
#ifdef	CG
	int any_exact;			/* Flag:  on if any DOEXACTs in tree */
#endif

	/* don't use nins for loop control in case IN_LINE is defined,
	/* giving the possibilty that nins gets changed, and insprt()
	/* gets called recursively.
	*/
	ninsav = nins;

	for( pi=inst,i=0; i<ninsav; ++i,++pi )
	{
		p = pi->p;
		q = pi->q;
		c = pi->goal;
		if( c == CCC && (q->rewrite&RESCC) ) goal = FORCC;
		else if( c == CEFF ) goal = FOREFF;
		else goal = INREG;
#ifndef NODBG
		if(odebug > 4)
		{
			printf("INSOUT: %d c=",i);
			preff(c);
			printf(" goal=");
			prgoal(goal);
			printf("\n");
			e2print(p);
		}
#endif

#ifdef IN_LINE
	        if (p->in.op == INCALL || p->in.op == UNARY INCALL)
		{
		    /* handle asm inline calls here, don't use expand() */
		    int n;
		    char * savinst;

		    /* have to preserve inst[] since 
		    /* it may be overwritten when insout() is called
		    /* to generate code for argument evaluation or
		    /* a real function call
		    */
		    n = sizeof(INST) * (ninsav-i);
		    if (!(savinst = malloc((unsigned) n)))
			cerror("can't save instructions for INCALL");
		    (void) memcpy( savinst, (char*)(pi), (unsigned)n);

		    as_gencode((ND2 *) p, outfile);

		    /* At this point the asm call has either been expanded
		    /* or converted to a regular CALL or UNARY CALL
		    */

		    if (p->in.op == UNARY CALL || p->in.op == CALL)
		    {
		    	/* have to produce code for regular function call */
#ifndef	STINCC
			costs(p);
		    	nins = 0;
		    	insout(p,CEFF); 
#else
		    	nins = 0;
			if (INSOUT(p,CEFF))	/* expect zero return */
				cerror("Can't generate in-line CALL");
#endif
		    	insprt();  
		    }

		    /* put everything back the way it was */
		    (void) memcpy( (char*)(pi), savinst, (unsigned)n);
		    free( savinst );
		    nins = ninsav;  /* to be good and safe */
		    /* reset "pi":  instruction generation may have resulted
		    ** in reallocation of the instruction array
		    */
		    pi = &inst[i];

		    allo( p, q );		/* safe to allocate now */

		 }
		else	
#endif
		{ /* non-asm template expansion */


#ifdef CG
			/*If there are any DOEXACTs, protect them
			  from optimizers*/
			/*Also, if there are any ";lp" that have a
			  null left child, remove them from the tree*/

		if ( (any_exact = pre_ex(p)) != 0)
			protect(p);
#endif

# ifdef TMPSRET
		/* not the best place in the world, but... */
		if (p->in.op == STCALL || p->in.op == UNARY STCALL)
			expand(p, goal, TMPSRET, q);
#endif
		/* Look for the case where we wanted to use an ASGOP template
		** for a non-ASGOP tree.  If the left side of the tree is a
		** register variable, get it into a scratch register.
		*/

#ifndef	REGSET
/* REGSET guarantees things are in suitable register */
		if (   asgop(q->op)
		    && !asgop(p->in.op)
		    && p->in.left->tn.op == REG
		    && !istreg(p->in.left->tn.rval)
		    )
		    CFIX( p->in.left, NRGS );
#endif	/* REGSET */

		allo( p, q );
		expand( p, goal, q->cstring, q );

#ifdef	CG
		if ( any_exact)
			unprot(p);
#endif
		} /* end non-asm template expansion */


		reclaim( p, q->rewrite, goal );

		/* now, if condition codes needed, test */
		if( c == CCC && p->tn.op != CCODES ) 
		{
			CFIX( p, CCC );
			if( p->tn.op != CCODES ) cerror( "ctest fails" );
		}
		else if( c>=0 && c<=NRGS) {
		    if (p->tn.op != REG )
			CFIX( p, NRGS );
#ifdef	REGSET
		    /* make sure we're in a suitable register */
		    if ((pi->rs_want & RS_BIT(p->tn.rval)) == 0) {
			RST choices = pi->rs_want & pi->rs_avail;
			int regno;

			if (choices == 0)
			    cerror("no choices for RS_MOVE");

			regno = rs_rnum( RS_CHOOSE(choices) );
			RS_MOVE(p, regno);
			reallo(p, regno);	/* adjust reg. allo. info. */
		    }
#endif	/*REGSET*/
		}
	}
}

#ifndef	STINCC
SHTABLE sha;
#endif

int odebug = 0;

static void 
#ifdef	REGSET
cfix( p, goal, want, avail )
NODE * p;
int goal;
RST want;
RST avail;
#else
cfix( p, goal )
NODE *p; 
int goal;
#endif
{
	/* p is to be fixed according to goal (CCC or NRGS) */
	OPTAB *q;
	NODE *pp;
	int r;
	int expgoal;

#ifndef NODBG
	if(odebug > 4)
	{
		printf("CFIX: goal=");
		prgoal(goal);
		printf("\n");
		e2print(p);
	}
#endif
	if( goal == CCC ) 
	{
		r = RESCC;
		p->tn.goal = CCC;
	}
	else 
	{
		r = (RESC1|RESC2|RESC3);
		pp = getl( p );
		if( istnode( pp ) ) r |= RLEFT;
		pp = getr( p );
		if( istnode( pp ) ) r |= RRIGHT;
	}

	if( goal == CCC ) expgoal = FORCC;
	else expgoal = INREG;

	for( q=0; (q = match( p, q )) != 0; )
	{
		/* takes the first match (may not be cheapest) */
		/* template writers, take note! */
		if( q->rewrite & r ) 
		{
#ifdef	REGSET
/* rs_reclaim will set up the node to contain a bitmap for scratch
** registers needed by the template.
*/
			/* these are set my match() */
			extern SHAPE *leftshape, *rightshape;
			static RST cfix_scratch();
			RST rregs = cfix_scratch(p);

			/* "rregs" are not also available. */
			if (RS_FAIL & rs_reclaim(p, q, goal,
						leftshape, RS_NONE,
						rightshape, rregs,
						avail & ~rregs, want))
			    cerror("cfix rs_reclaim fails");
#endif
			/* generate the code on the spot */
			allo( p, q );
# ifdef TMPSRET
			/* likewise */
			if (p->in.op == STCALL || p->in.op == UNARY STCALL)
				expand(p, expgoal, TMPSRET, q);
#endif
			expand( p, expgoal, q->cstring, q );
			reclaim( p, q->rewrite, expgoal );
			return;
		}
	}
	e2print(p);
	cerror( "cfix trouble" );
}


#ifdef	REGSET

/* Determine the register bit vector that corresponds to the
** tree p.  We're only interested in scratch registers here:
** they're the ones that are no longer available.  We must make
** a recursive tree walk and OR together the bits of all scratch
** register found.  Also take account of register pairs.
*/
static RST
cfix_scratch(p)
NODE * p;
{
    RST curbits = 0;

    switch( optype(p->in.op) ){
    case BITYPE:
	curbits |= cfix_scratch(p->in.right);
	/*FALLTHRU*/
    case UTYPE:
	curbits |= cfix_scratch(p->in.left);
	break;
    case LTYPE:
	if (istnode( p )) {
	    curbits = RS_BIT(p->tn.rval);
	    /* account for register pairs */
	    if (szty(p->tn.type) > 1)
		curbits |= RS_PAIR(curbits);
	}
	break;
    }
    return(curbits);
}

#endif /* def REGSET */

#ifndef NODBG
preff(c)
{
	char buf[20];
	register char *p;

	p = c==CCC ? "CCC" : c==CTEMP ? "CTEMP" : c==CEFF ? "CEFF" : 0;
	if(!p)
	{
		sprintf(buf,"0%o",c);
		p = buf;
	}
	printf("%s",p);
}
#endif
