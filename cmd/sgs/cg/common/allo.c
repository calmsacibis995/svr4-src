/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/allo.c	1.11"

# include "mfile2.h"

NODE resc[NRGS];

int busy[NRGS];

# define TBUSY 0100

static void recltree(), tfreebut(), trbusy(), rfree(); 
static int freepair(), freereg(), shareit(), usable(), shared();

void 
allo0()
{
	/* free everything */
	register i;

	for( i=0; i<NRGS; ++i )
	{
		busy[i] = 0;
	}
}

#ifdef	REGSET
static RST curscratch;			/* tunneling between allo() and usable() */
#endif

void 
allo( p, q )
register NODE *p; 
register struct optab *q; 
{
	register n, i, j;
	register NODE *presc;
#ifdef	REGSET
	/* scratch registers are allocated from one set at a time; they
	** appear sequentially
	*/
	RST * scrptr = RSB(p->in.scratch); /* pointer to scratch bit vectors */

	curscratch = *scrptr++;		/* current set of scratch registers */
#endif

	n = q->needs;
	i = 0;
	/* This code assumes a double reg counts 1 */
	while( n & NCOUNT )
	{
		if( n&NPAIR ) 
		{
			j = freepair( p, n&NMASK );
			busy[j] |= TBUSY;
			busy[j+1] |= TBUSY;
#ifdef REGSET
			curscratch -= RS_BIT(j+1);	/* remove from set */
#endif
		}
		else 
		{
			j = freereg( p, n&NMASK );
			busy[j] |= TBUSY;
		}
#ifdef REGSET
		curscratch -= RS_BIT(j);	/* remove from set */
		if (curscratch == RS_NONE)
		    curscratch = *scrptr++;	/* replenish supply -- next set */
#endif
		n -= NREG;
		presc = &resc[i];
		presc->in.op = REG;
		presc->tn.rval = j;
		presc->tn.type = p->tn.type;
		presc->tn.lval = 0;
		presc->in.name = (char *) 0;
		++i;
	}

	/* turn off "temporarily busy" bit */
	for( j=0; j<NRGS; ++j )
	{
		busy[j] &= ~TBUSY;
	}
#ifndef NODBG
	if( rdebug > 1 )
	{
		fprintf(outfile, "allo( %d, %d ), %o",
				node_no(p), q->stinline, q->needs );
		for( j=0; j<i; ++j )
		{
			if( resc[j].tn.op == REG )
				fprintf(outfile, ", REG(%d)", resc[j].tn.rval );
			else
				fprintf(outfile, ", TEMP(%ld)", resc[j].tn.lval );
		}
		PUTCHAR( '\n' );
		prbusy( "busy:" );
	}
#endif
}

int tmpoff;  /* offset of next temp to be allocated */

int rdebug = 0;

freetemp( k )
register k;
{
	/* allocate k integers worth of temp space
	** we also make the convention that, if the number of words is more than 1,
	** it must be aligned for storing doubles... 
	*/
#ifdef	CG
			/*This has been obsoleted by next_temp*/
	return ( next_temp( TINT, (BITOFF) k*SZINT, (k>1 ? ALDOUBLE : ALINT))
			* SZCHAR);

#else

# ifndef BACKTEMP
	int t;

	if( k>1 )
	{
		SETOFF( tmpoff, ALDOUBLE );
	}

	t = tmpoff;
	tmpoff += k*SZINT;
	if( tmpoff > maxtemp ) maxtemp = tmpoff;
	return(t);

# else
	tmpoff += k*SZINT;
	if( k>1 ) 
	{
		SETOFF( tmpoff, ALDOUBLE );
	}
	if( tmpoff > maxtemp ) maxtemp = tmpoff;
	return( -tmpoff );
# endif	/* def BACKTEMP */
#endif	/* def CG */
}

# ifdef STACK
/* for stack machines, totally disable the register allocation */
freereg( p, n )
NODE *p; 
{
	return( 0 );
}

freepair( p, n )
NODE *p; 
{
	cerror( "pairs on a stack machine?" );
}

rbusy( r, t )
TWORD t; 
{
}
rfree( r, t )
TWORD t; 
{
}

regrcl( p )
NODE *p; 
{
}
# else
static int
freepair( p, n )
register NODE *p; 
register n;
{
	/* allocate a register pair */
	/* p gives the type */

#ifdef MYPAIR
	return (mypair(p, n));
#else
	register j;

	if( callop(p->in.op) )
	{
		j = callreg(p);
		if( j&1 ) cerror( "callreg returns bad pair" );
		if( usable( p, n, j ) ) return( j );
		/* have allocated callreg first */
	}
	if( n&NMASK )
	{
#ifdef ODDPAIR			/* if pair may start on odd reg. */
		for( j=0; j<NRGS; j++ )
#else
		for( j=0; j<NRGS; j+=2 )
#endif
			if( usable(p,n,j) && usable(p,n,j+1))
				return( j );
	}
	cerror( "allocation fails, op %s", opst[p->tn.op] );
	/* NOTREACHED */
#endif
}
static int 
freereg( p, n )
register NODE *p; 
register n;
{
	/* allocate a register */
	/* p gives the type */

	register j;
	register int t = optype( p->tn.op );

	if( callop(p->in.op) )
	{
		j = callreg(p);
		if( usable( p, n, j ) ) return( j );
		/* have allocated callreg first */
	}
	if( n&NMASK )
	{
		if( (n&LPREF) && (j = shared( getlt( p, t ) ) ) >= 0 &&
		   usable( p, n, j ) ) return( j );
		if( (n&RPREF) && (j = shared( getrt( p, t ) ) ) >= 0 &&
		   usable( p, n, j ) ) return( j );
		for( j=0; j<NRGS; ++j ) if( usable(p,n,j) ) return( j );
	}
	cerror( "allocation fails, op %s", opst[p->tn.op] );
	/* NOTREACHED */
}
static int
shared( p )
register NODE *p; 
{
	/* simple, at present */
	/* try to find a single register to share */
	register r, o;
#ifndef NODBG
	if( rdebug ) 
	{
		PUTS( "shared called on:\n" );
		e2print( p );
	}
#endif
	if( (o=p->tn.op) == REG ) 
	{
		r = p->tn.rval;
		if (r >= NRGS) return ( -1 );
#ifndef NODBG
		if( rdebug ) 
		{
			fprintf(outfile, "preference for %s\n", rnames[r] );
		}
#endif
		return( r );
	}
	/* we look for shared regs under unary-like ops */
	switch( optype( o ) ) 
	{

	case BITYPE:
		/* look for simple cases */
		/* look only on the left */
	case UTYPE:
		return( shared( p->in.left ) );
	}
	return( -1 );
}

static int
usable( p, n, r )
register NODE *p; 
register n,r;
{
	/* decide if register r is usable in tree p to satisfy need n */
	/* this does not concern itself with pairs */

	if( r>= NRGS || (busy[r] & TBUSY) ) return( 0 );
	if( busy[r] > 1 ) 
	{
		/*
		** uerror( "register %d too busy", r );
		*/
		return( 0 );
	}
#ifdef	REGSET
	/* must be one of registers whose scratch index was stashed in node */
	if( (RS_BIT(r) & curscratch) == 0 )
	    return( 0 );
#endif
	if( busy[r] == 0 )
	    return(1);

	/* busy[r] is 1: is there chance for sharing */
	return( shareit( p, r, n ) );

}

static int
shareit( p, r, n )
register NODE *p; 
register r;
int n;
{
	/* can we make register r available by sharing from p
	** given that the need is n 
	*/
	register NODE *sub;
	register int t = optype(p->tn.op);
	
	sub = getlt( p, t );
	if( (n&LSHARE) && ushare( sub, r ) )  {
		return 1;
	}
	sub = getrt( p, t );
	if( (n&RSHARE) && ushare( sub, r ) ) {
		return(1);
	}
	return(0);
}
int
ushare( p, r )
register NODE *p; 
register r;
{
	/* can we find register r to share in p */
	if( p->in.op == REG ) 
	{
		if( szty( p->tn.type ) == 2 && r==(p->tn.rval+1) ) return( 1 );
		return( r == p->tn.rval );
	}
	switch( optype( p->tn.op ) )
	{
	case BITYPE:
		if( ushare( p->in.right, r ) ) return( 1 );
		/*FALLTHRU*/
	case UTYPE:
		if( ushare( p->in.left, r ) ) return( 1 );
	}
	return(0);
}

void  
regrcl( p )
register NODE *p; 
{
	/* free registers in the tree (or fragment) p */
	register r;
	if( !p ) return;
	r = p->tn.rval;
	if( p->in.op == REG ) rfree( r, p->in.type );
	switch( optype( p->tn.op ) )
	{
	case BITYPE:
		regrcl( p->in.right );
		/* explict assignment to regs not accounted for */
		if( asgop(p->tn.op) && p->in.left->tn.op == REG ) break;
		/*FALLTHRU*/
	case UTYPE:
		regrcl( p->in.left );
	}
}
static void 
rfree( r, t )
register TWORD t; 
register r;
{
	/* mark register r free, if it is legal to do so */
	/* t is the type */

#ifndef NODBG
	if( rdebug )
	{
		fprintf(outfile, "rfree( %s, ", rnames[r] );
		t2print( t );
		PUTS( " )\n" );
	}
#endif
	if( istreg(r) )
	{
		if( --busy[r] < 0 ) cerror( "register overfreed");
		if( szty( t ) > 1 )
		{
			if( !istreg(r+1) ) cerror( "big register" );
			if( --busy[r+1] < 0 ) cerror( "register overfreed" );
		}
	}
}

void 
rbusy(r, t )
register r; 
register TWORD t; 
{
	/* mark register r busy */

#ifndef NODBG
	if( rdebug )
	{
		fprintf(outfile, "rbusy( %s, ", rnames[r] );
		t2print( t );
		PUTS( " )\n" );
	}
#endif
	if( istreg(r) ) 
	{
		++busy[r];
		if( szty( t ) > 1 )
		{
			if( !istreg(r+1) ) cerror( "big register" );
			++busy[r+1];
		}
	}
}

#ifndef NODBG
prbusy( s )
char *s;
{
	/* print out the busy[] array */
	int i;
	fprintf(outfile, "%s [", s );
	for( i=0; i<NRGS-1; ++i ) fprintf(outfile, "%d,", busy[i] );
	fprintf(outfile, "%d]\n", busy[NRGS-1] );
}
#endif

# endif

#ifndef NODBG
rwprint( rw )
register rw;
{
	/* print rewriting rule */
	register i, flag;
	static char * rwnames[] = 
	{
		"RLEFT",
		"RRIGHT",
		"RESC1",
		"RESC2",
		"RESC3",
		"RESCC",
		"RNOP",
		"RNULL",
#ifdef	STINCC
		"REITHER",
#endif
		0,
	};
	flag = 0;
	for( i=0; rwnames[i]; ++i )
	{
		if( rw & (1<<i) )
		{
			if( flag ) PUTCHAR( '|' );
			++flag;
			fprintf(outfile, rwnames[i] );
		}
	}
	if( !flag ) fprintf(outfile, "?%o", rw );
}
#endif

void
reclaim( p, rw, goal )
register NODE *p; 
register rw, goal;
{
	register NODE *q;
	register o;
#ifdef	REGSET
	RST oldscratch;
#endif

	/* get back stuff */
#ifndef NODBG
	if( rdebug )
	{
		fprintf(outfile, "reclaim( %d, ", node_no(p) );
		rwprint( rw );
		PUTS( ", " );
		prgoal( goal );
		PUTS( " )\n" );
	}
#endif
	if( !p ) return;
	/* special cases... */
	if( (o=p->tn.op) == COMOP )
	{
		/* LHS has already been freed; don't free again */
		regrcl( p->in.right );
	}
	else regrcl( p );
	if( (o==FREE && rw==RNULL) || rw==RNOP ) return;
	if( callop(o) )
	{
		/* check that all scratch regs are free */
		callchk(p);  /* ordinarily, this is the same as allchk() */
	}
	if( rw == RNULL || (goal&FOREFF) )
	{
		/* totally clobber, leave nothing */
		tfree(p);
		return;
	}
	/* handle condition codes specially */
	if( (goal & FORCC) && (rw&RESCC)) 
	{
		/* result is CC register */
		tfree(p);
		p->in.op = CCODES;
		p->tn.lval = 0;
		p->tn.rval = 0;
		return;
	}
	q = 0;
#ifdef	STINCC
	/* For $E (result on either side), choose right side as result
	** if it is REG and the left side isn't.  Otherwise choose left.
	*/
	if (rw & REITHER) {
	    if (   p->in.right && p->in.right->in.op == REG
		&& !(p->in.left && p->in.left->in.op == REG)
	    )
		q = getr( p );
	    else
		q = getl( p );
	}
	else
#endif	/* def STINCC */
	     if( rw&RLEFT )	q = getl( p );
	else if( rw&RRIGHT )	q = getr( p );
	else if( rw&RESC1 )	q = &resc[0];
	else if( rw&RESC2 )	q = &resc[1];
	else if( rw&RESC3 )	q = &resc[2];
	else 
	{
		cerror( "illegal reclaim, op %s", opst[p->tn.op]);
	}
#ifdef	REGSET
	oldscratch = p->in.scratch;
#endif
	if( o == STARG ) p = p->in.left;  /* STARGs are still STARGS */
/* The code here used to look like:
/*	q = tcopy(q);
/*	tfree(p);
/*	*p = *q;  /* make the result replace the original */
/*	q->in.op = FREE;
/*
/* However, there's a nasty bug lurking:  suppose that a sub-tree of
/* p has had its code generated and has been freed.  Further suppose
/* that tcopy(q) reuses one of the nodes.  The subsequent tfree(p)
/* will also free the copy of q!  Be cleverer in the reclaim.
*/

	recltree(p,q);			/* p overwritten */

#ifdef	REGSET
	p->in.scratch = oldscratch;
#endif
}

/* Reclaim result tree:  discard entire tree, except for
** result part, mark scratch registers busy, overwrite incoming
** node with result node.
*/

static void 
recltree(tree, result)
NODE * tree;
NODE * result;
{
    /* easy case:  result is resc[n] */
    if (&resc[0] <= result && result < &resc[NRGS-1])
	tfree(tree);			/* discard entire tree */
    else
	tfreebut( tree, result );	/* free all but result */

    result->in.type = tree->in.type;	/* result type is tree type */
    trbusy( result );			/* mark scratch regs busy */
    *tree = *result;			/* copy over first node */
    result->in.op = FREE;		/* make old result node FREE */
    return;
}

/* Free tree except for nodes at or below "except" */

static void 
tfreebut(tree, except)
NODE * tree;
NODE * except;
{
    if (tree != except) {
	switch( optype(tree->in.op) ){
	case BITYPE:	tfreebut(tree->in.right, except);
	/*FALLTHRU*/
	case UTYPE:	tfreebut(tree->in.left, except);
	}
	tree->in.op = FREE;
    }
    return;
}

/* Mark registers in tree busy */

static void 
trbusy(tree)
register NODE * tree;
{
    if (tree->in.op == REG)
	rbusy( tree->tn.rval, tree->tn.type );
    else {
	switch( optype(tree->in.op) ){
	case BITYPE:
	    trbusy(tree->in.right);
	    /*FALLTHRU*/
	case UTYPE:
	    trbusy(tree->in.left);
	}
    }
    return;
}

NODE *
tcopy( p )
register NODE *p; 
{
	/* make a fresh copy of p */
	register NODE *q;
	register r;

	q=talloc();
	*q = *p;
	r = p->tn.rval;
	if( p->in.op == REG ) rbusy( r, p->in.type );
	switch( optype(q->in.op) )
	{
	case BITYPE:
		q->in.right = tcopy(p->in.right);
		/*FALLTHRU*/
	case UTYPE:
		q->in.left = tcopy(p->in.left);
	}
	return(q);
}

void 
allchk()
{
	/* check to ensure that all register are free */
	register i;

	for( i=0; i<NRGS; ++i )
	{
		if( busy[i] )
		{
			cerror( "register allocation error");
		}
	}
}

/* this may not be the best place for this routine... */
argsize( p )
register NODE *p; 
{
	/* size of the arguments */
	register t;
	t = 0;
	if( p->tn.op == CM )
	{
		t = argsize( p->in.left );
		p = p->in.right;
	}
#ifdef CG
	if( p->tn.type & TDOUBLE )
#else
	if( p->tn.type & (TDOUBLE|TFLOAT) )
#endif
	{
		SETOFF( t, ALDOUBLE );
		t += SZDOUBLE;
	}
	else if( p->tn.type & (TLONG|TULONG) )
	{
		SETOFF( t, ALLONG );
		t += SZLONG;
	}
	else if( p->tn.type & (TPOINT|TPOINT2) )
	{
		SETOFF( t, ALPOINT );
		t += SZPOINT;
	}
	else if( p->tn.type & TSTRUCT )
	{
		BITOFF align = p->stn.stalign;

		/* Object alignment must be multiple of stack alignment. */
		SETOFF( align, ALSTACK );
		SETOFF( t, align );	  /* alignment */
		t += p->stn.stsize;  /* size */
	}
#ifdef CG
	else if ( p->tn.type & TFLOAT)
	{
		SETOFF( t, ALFLOAT);
		t += SZFLOAT;
	}
	else if ( p->tn.type & TFPTR)
	{
		SETOFF( t, ALFPTR);
		t += SZFPTR;
	}
#endif
	else 
	{
		SETOFF( t, ALINT );
		t += SZINT;
	}
	return( t );
}

#ifdef	REGSET
/* This routine is used to mirror the fact that a register [pair]
** has been moved to different registers.
*/

void 
reallo(p, r)
NODE * p;				/* reallocate p's registers */
int r;					/* they've moved to r */
{
    if (p->tn.op != REG)
	cerror("non-REG in reallo");
    
    /* allocate before freeing to insure no overlap */
    rbusy( r, p->tn.type );
    rfree( p->tn.rval, p->tn.type );
    p->tn.rval = r;			/* reset register number in node */
    return;
}
#endif
