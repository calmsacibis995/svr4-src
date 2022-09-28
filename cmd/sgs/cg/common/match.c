/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/match.c	1.12"

# include "mfile2.h"
# ifdef SDB
# include "sdb.h"
# endif

#ifndef	NODBG
/* ==== DEBUG */
extern FILE * debugout;
/* ==== END DEBUG */
#endif

static int spshape();
static SHAPE *rtspine(), *mspine();

int fldsz, fldshf;

int sdebug = 0;
int zflag = 0;			/* non-zero to output stin line and cost for
				** each assembly instruction
				*/

/* Table of 2^n-1. */
static unsigned long pow2m1[] = 
{
/* 0 */		0x00000000L,
/* 1-4 */	0x00000001L,	0x00000003L,	0x00000007L,	0x0000000fL,
/* 5-8 */	0x0000001fL,	0x0000003fL,	0x0000007fL,	0x000000ffL,
/* 9-12 */	0x000001ffL,	0x000003ffL,	0x000007ffL,	0x00000fffL,
/* 13-16 */	0x00001fffL,	0x00003fffL,	0x00007fffL,	0x0000ffffL,
/* 17-20 */	0x0001ffffL,	0x0003ffffL,	0x0007ffffL,	0x000fffffL,
/* 21-24 */	0x001fffffL,	0x003fffffL,	0x007fffffL,	0x00ffffffL,
/* 25-28 */	0x01ffffffL,	0x03ffffffL,	0x07ffffffL,	0x0fffffffL,
/* 29-32 */	0x1fffffffL,	0x3fffffffL,	0x7fffffffL,	0xffffffffL,
};

/* the following definition is useful for describing the actual
** layout of each pshape used by templates.  In fact, however,
** table.c defines them as an array of SHAPE *, so this is a
** HACK!
*/

typedef struct {
    int mask;			/* bits for OPs that can head shapes in this
				** pshape
				*/
    SHAPE ** wildcards;		/* pointer to wildcard shapes, somewhere into
				** the following array
				*/
    SHAPE * sshapes[1];		/* really an indefinite size array:  the 1 is
				** a convenient fiction
				*/
} PSHAPE;

ttype( t )
register TWORD t; 
{
	/* return the coded type of t */
	/* this is called only from the first pass */

# ifdef TWOPTRS
	if( ISPTR(t) ) 
	{
		do 
		{
			t = DECREF(t);
		} while ( ISARY(t) );
		/* arrays that are left are usually only
		** in structure references... 
		*/
		if( TWOPTRS(t) ) return( TPOINT2 );
		return( TPOINT );
	}
# endif

	if( t != BTYPE(t) ) return( TPOINT ); /* TPOINT means not simple! */

	switch( t )
	{

	case CHAR:
		return( TCHAR );
	case SHORT:
		return( TSHORT );
	case STRTY:
	case UNIONTY:
		return( TSTRUCT );
	case INT:
		return( TINT );
	case UNSIGNED:
		return( TUNSIGNED );
	case USHORT:
		return( TUSHORT );
	case UCHAR:
		return( TUCHAR );
	case ULONG:
		return( TULONG );
	case LONG:
		return( TLONG );
	case FLOAT:
		return( TFLOAT );
	case DOUBLE:
		return( TDOUBLE );
	case VOID:
		return( TVOID );
	}
	cerror( "ttype(0%o)", t );
	/* NOTREACHED */
}

# define VALUE (RESC1|RESC2|RESC3|RLEFT|RRIGHT|REITHER)

OPTAB *
match( p, q)
register NODE *p; 
register OPTAB *q; 
{
	/* look for match in table */
	/* given a tree, p, and a template q, returns the next template that
	/* matches p */
	/* q == NULL starts the process off */
	/* match is not called recursively, so much of the setup can be done
	/* into static variables */

	static NODE *r, *l;
	static tl, tr, goal, tyop;
	static pop, lflg;
	extern SHAPE * leftshape, *rightshape;
	int leftres=0, rightres=0;
	static SHAPE * mexact();

	if( !q ) {
		/* startup code */

		q = ophead[pop=p->tn.op];
		l = getlo( p, pop );		/* left child */
		r = getro( p, pop );		/* right child */
		tl = l->in.type;
		tr = r->in.type;
		lflg = asgop(pop);  /* forces a match on lvalues */
		if( p->tn.goal == CCC ) goal = (VALUE|RESCC);
		else if( p->tn.goal == NRGS ) goal = VALUE;
		else goal = ~0;
		tyop = p->in.type;
	}

	else {
		q = q->nextop;
	}
	if( !q ) return( (OPTAB *)0 );

# ifndef NODBG
	if( sdebug ) {
		fprintf(outfile, "match tree %d, op %s\n", node_no(p), opst[pop] );
	}
# endif
	for( ; q ; q = q->nextop ){
		leftshape = rightshape = 0;

		/* some templates are no good for computing values */

		if( !( tl & q->ltype ) ) continue;
		if( !( tr & q->rtype ) ) continue;

		if( !( goal & q->rewrite ) ) continue;

# ifndef NODBG
		if( sdebug ) {
			fprintf(outfile, "	try table entry, line %d, op %s\n",
						q->stinline, opst[q->op] );
		}
# endif
		if( !( tyop & q->tyop ) ) {
# ifndef NODBG
			if( sdebug ) {
				fprintf(outfile,
				"\tentry line %d fails tyop, %o vs %o\n",
				q->stinline, tyop, q->tyop );
			}
# endif
			continue;
		}

#ifdef CG
			/*nodes that need exceptions need the right template*/
		if( ( (p->in.strat & EXHONOR) && (q->needs & NO_HONOR) )
		  || ( (p->in.strat & EXIGNORE) && (q->needs & NO_IGNORE) ) )
		{
#  ifndef NODBG
			if (sdebug) {
				fprintf(outfile,
				"\tentry line %d fails exceptions, %o vs %o\n",
				q->stinline, p->in.strat, q->needs);
			}
#  endif
			continue;
		}
#endif
			/*Set flag to indicate whether shapes must be result*/
		if ( p->in.goal != CEFF)
		{
			leftres = (q->rewrite & (RLEFT|REITHER));
			rightres = (q->rewrite & (RRIGHT|REITHER));
		}
		if( q->rshape ){
			if (q->needs & RMATCH)	/* match exactly? */
			    rightshape =
				mexact((PSHAPE *)q->rshape, (q->lshape ? r : p), 0, 0);
			else
			    rightshape=mspine(
				(PSHAPE *)q->rshape, (q->lshape)?r:p, 0, 
				rightres , (q->lshape? 0 : pop) );
			
			if (!rightshape) continue;
		}

		if( q->lshape ){
			if (q->needs & LMATCH)	/* exact match */
			    /* left side must be TEMP if template is asgop,
			    ** tree is not; note whether ASGOP in tree
			    */
			    leftshape =
				mexact((PSHAPE *)q->lshape, l, (!lflg && asgop(q->op)), lflg);
			else if( !lflg && asgop(q->op) ){
				/* q->op is an assignment op, pop isn't */
				/* thus, the lhs => a register or temp */
				leftshape = rtspine( (PSHAPE *)q->lshape, p );
			}
			else leftshape = mspine( (PSHAPE *)q->lshape, l, lflg, leftres, 0);

			if(! leftshape ) continue;
		}

		/* at this point, we have a match */

# ifndef NODBG
		if( sdebug ) {
			fprintf(outfile, "table line %d matches tree %d\n",
						q->stinline, node_no(p) );
		}
# endif

		return( q );  /* found match */
	}
	return( (OPTAB *)0 );
}


static SHAPE *
rtspine( psh, p )
PSHAPE *psh;				/* really a SHAPE ** */
NODE *p;
{
	/* special routine to put the lhs into a reg or a temp */
	/* looks for reg and/or temp in s */

	/* skip directly to wildcard shapes, if any */
	SHAPE ** ps = psh->wildcards;
	register SHAPE *s;

	while (s = *ps++) {
	    if (s->op == REG) {
		if( s->sh ) if( !spshape( s->sh, p ) ) continue;
		return( s );
	    }
	}
	/* no REG shape found */
	return( 0 );
}
static 
noresult( s )
register SHAPE *s; 
{
	if( !s ) return( 0 );
	if( s->op == INCR || s->op == DECR ) return( 1 );
	return( (s->sr && noresult(s->sr)) ||
	    (s->sl && noresult( s->sl )) );
}

/* Match shape with only TEMPs as wildcards.  This is used when
** the number of registers available is limited.  Ordinarily
** the TEMP may appear anywhere in the shape.  We know from
** ordering considerations that a TEMP leaf shape appears before
** any others.  Usually we want trees where TEMP appears deeper
** in the tree, and these come later.  When match() calls us
** and the OP in the tree is X and the OP in the template is ASG X,
** we only allow matches with TEMP on the left.  If the tree OP is
** ASSIGN, we do not allow a leaf TEMP to match as a wildcard.
** Return the found shape, if any.
*/

static SHAPE *
mexact( psh, p, templeaf, tempdeep )
PSHAPE * psh;				/* really SHAPE **:  p-shape to check */
NODE * p;				/* node (tree) to match against */
int templeaf;				/* non-zero if only TEMP as leaf is okay */
int tempdeep;				/* non-zero if TEMP only allowed deep
					** in shape
					*/
{
    int retval = 0;			/* return value from smexact */
    SHAPE ** shplist = psh->sshapes;	/* list of shapes to check */
    SHAPE * s;				/* current shape */
    SHAPE * haveleaf = (SHAPE *) 0;	/* leaf TEMP found */
    SHAPE * havedeep = (SHAPE *) 0;	/* deep TEMP found */
    SHAPE * result;			/* presumed found shape */
    extern int shphd[];
    static int smexact();

#ifndef	NODBG
    if (sdebug > 1)
	fprintf(outfile,
	"	mexact called with node %d, shape list %d, templeaf = %d, tempdeep = %d\n",
					node_no(p), shplist-pshape, templeaf, tempdeep);
#endif

    /* quick check for impossible matches;
    ** allow for TEMP matching as wildcard
    */
    if ( ((shphd[TEMP] | shphd[p->in.op]) & psh->mask) != 0) {
	shplist = psh->sshapes;		/* skip to list of shapes */

	while (s = *shplist++) {	/* next shape */
	    if ((retval = smexact( s, p )) != 0) {
		if (retval > 0)		/* found true exact match */
		{
#ifdef CG
					/*CG: OCOPY flag suppresses exact match*/
		    if (p->in.strat & OCOPY || firstl(p) )
			continue;
#endif
		    return( s );	/* return it */
		}

		if (retval < 0) {	/* if some wildcard match of TEMP */
		    if (s->op == TEMP)
			haveleaf = s;	/* remember leaf TEMP found */
		    else 		/* remember last deep TEMP */
			havedeep = s;
		}
	    }
	} /* end while */
    }


    /* no exact match found;
    ** if no restrictions, choose deep TEMP over leaf
    */
    if (tempdeep)
	result = havedeep;
    else if (templeaf)
	result = haveleaf;
    else {				/* no restrictions */
	if (! (result = havedeep))
	    result = haveleaf;
    }

#ifndef	NODBG
    if (sdebug > 1) {
	fprintf(outfile, "	mexact returns %smatch", result ? "" : "no ");
	if (templeaf && haveleaf)
	    PUTS( ", haveleaf" );
	else if (tempdeep && havedeep)
    	    PUTS( ", havedeep" );
	PUTCHAR('\n');
    }
#endif
    return( result );
}


static int
smexact( s, p )
SHAPE * s;
NODE * p;
{
    register int sop;		/* shape's OP */
    int retval = 0;		/* presumed result */

    for (;;) {
	/* make sure special shapes match first */
	if ( (s->sh) && !(spshape(s->sh, p))) break;

	if ((sop = s->op) != p->in.op) { /* trees mismatch */
	    if (sop != TEMP)		/* if not TEMP, we lose */
		retval = 0;
	    else
		retval = -1;		/* TEMP matched as a wildcard */
	    break;
	}
	
	switch( optype( sop ) ) {	/* OPs matched */
	case BITYPE:
	    /* right sides must match */
	    if (! smexact( s->sr, p->in.right)) break;
	    /*FALLTHRU*/
	
	case UTYPE:
	    s = s->sl;			/* turn recursion into iteration */
	    p = p->in.left;
	    continue;

	default:
#ifdef	REGSET
	    if (sop == REG && (s->sregset & RS_BIT(p->tn.rval)) == 0)
		retval = 0;		/* fail if register not part of set */
	    else
#endif
		retval = 1;		/* leaf matched:  success! */
	    break;
	}
	/* only reach here when all done */
	break;				/* exit loop */
    }
    return( retval );
}
static 
smspine( s, p, flag )
register NODE	*p; 
register SHAPE	*s; 
{
	/* match the spine of s, including special shapes */
	/* this is a submatch, called only by mspine */
	/* flag is nonzero if STAR's must be skipped */

	register sop, pop;

again:

	sop = s->op;
	pop = p->tn.op;

# ifndef NODBG
	if( sdebug>1 )
	{
		fprintf(outfile,  "			smspine(%d[%s], %d[%s])\n",
					s-shapes, opst[sop], node_no(p), opst[pop] );
	}
# endif

	if( ( s->sh ) && !spshape( s->sh,p) ) return( 0 );
	if( sop != pop ) 
	{
		if( sop == CCODES || sop == FREE ) return(1);
		return(
		           !flag
			&& (sop==REG )
#ifdef	REGSET
			/* shape's register set must include scratch */
			&& (s->sregset & RS_NRGS) != 0
#endif
		    );
	}
	if( sop == STAR ) 
	{
		flag = 0;
		goto recurse;
	}
#ifdef	REGSET
	/* In REGSET instance, one register matches another if they
	** have intersecting register sets.  Otherwise, if the shape
	** REG contains a scratch register we have a wildcard match.
	*/
	else if ( sop == REG )
	    return(
		       (RS_BIT(p->tn.rval) & s->sregset) != 0
		    || (s->sregset & RS_NRGS) != 0
		    );
#endif

	switch( optype( sop ) ) 
	{

	case BITYPE:
		if( !smspine( s->sr, p->in.right, 0 ) ) return(0);
		flag = asgop(sop);
		/* FALLTHRU */

	case UTYPE:
recurse:
		s = s->sl;
		p = p->in.left;
		goto again;

	default:
		return( 1 );
	}
}

static SHAPE *
mspine( psh, p, flag, result, op)
PSHAPE	*psh;				/* really a SHAPE ** */
register NODE	*p;
int flag;
int result;
int op;
{
	/* match the spine of s, including special shapes */
	/* ps points to a list of shapes */
	/* flag = 1 forces regs and temps to match exactly */
	/* the matching shape, if any, is returned */

	register SHAPE ** ps = psh->sshapes; /* start matching at normal place */
	register SHAPE *s;
	int pop;
	extern int shphd[];		/* shphd[op] non-zero if OP can head
					** ANY shape
					*/

# ifndef NODBG
	if( sdebug ) 
	{
		fprintf(outfile,  "\tmspine( %d, %d, %d )\n",
					ps-pshape, node_no(p), flag );
	}
# endif
	pop = p->tn.op;			/* current op in tree */

	/* skip directly to shapes beginning with wildcards if OP is REG,
	** or if current OP can't possibly be head of a shape; first word
	** is vector of allowable OP bits for these shapes
	*/
	/* first, check whether restricted OP is present in shape's OPs */

	if (op != 0 && (shphd[op] & psh->mask) == 0)
	    return( 0 );		/* can't possibly match */
	if (pop == REG || (shphd[pop] & psh->mask) == 0)
	    ps = psh->wildcards;	/* skip to wildcards, if any */

	/* take the first match */
	while (s = *ps++)
	{
			/*Is there a restricted op?*/
		if ( op && (op != s->op))
			continue;
			/*Must the shape be usable as a result?*/
		if ( result && noresult(s))
			continue;
		switch( s->op ) 
		{

		case REG:
				/*flag forces exact match*/
			if( flag && pop != REG) continue;
#ifdef	REGSET
			/* register sets must match */
			if ( flag && (RS_BIT(p->tn.rval) & s->sregset) == 0)
				continue;
			/* for REG shape to act as wildcard, its register
			** set must include scratch regs
			*/
			if (pop != REG && (s->sregset & RS_NRGS) == 0) continue;
#endif
			if( s->sh ) if( !spshape(s->sh,p) ) continue;
			/* FALLTHRU */

		case FREE:
		case CCODES:

# ifndef NODBG
			if( sdebug )
			{
				fprintf(outfile, 
				"\t\tmspine( %d[%s], %d[%s] ), matches\n",
				s-shapes, opst[s->op], node_no(p), opst[pop] );
			}
# endif

			return( s );

		default:

			if ( s->op != pop)
				continue;
#ifdef CG
			/* For CG: If the OCOPY flag is set, this means
			   an exact match is suppressed, except for leaf templates.
			   This is a leaf template if a restricted op "op"
			   is supplied.*/

			if ( ( p->in.strat & OCOPY ) && (op == 0) )
				continue;
			if (firstl(p))
				continue;
#endif
			if( pop == STAR ) 
			{
				if( !smspine( s->sl, p->in.left, 0 ) ) continue;
			}

			/* general case: check the subtrees also */
			else if( s->sl ) 
			{
				if( s->sr ) 
				{
					if( !smspine( s->sr, p->in.right, 0 ) )
						continue;
					if( !smspine( s->sl, p->in.left, (int)asgop(s->op)) )
						continue;
				}
				else 
				{
					if( !smspine( s->sl, p->in.left, flag ))
						continue;
				}
			}
			else if( s->sr && !smspine( s->sr, p->in.right, 0 ) )
				continue;

			/* if we get this far, we are in good shape */

			if( s->sh ) if( !spshape( s->sh, p ) ) continue;

# ifndef NODBG
			if( sdebug )
			{
				fprintf(outfile,
					"\t\tmspine( %d[%s], %d[%s] ), matches\n",
					s-shapes, opst[s->op], node_no(p), opst[pop] );
			}
# endif

			return( s );
		} /*end switch*/
	} /*end for*/

	/* fall out with no match */
	return( 0 );
}

#ifndef CG
tempok( p )
NODE *p; 

{
	/* decide if the result of p is OK as a temp */
	int o;
	if( (o = p->tn.op) == TEMP ) return( 1 );
	if( asgop(o) && o!=INCR && o!=DECR ) 
	{
		return( p->in.left->tn.op == TEMP );
	}
	return( 0 );
}
#endif

int sideff;

static void 
rmside( p )
NODE *p; 

{
	/* p is a tree with side effects: remove them */
	/* this is done in place */
	int o;
	NODE *q;

	o = p->tn.op;
	switch( o ) 
	{

	case INCR:
	case DECR:
	case ASG PLUS:
	case ASG MINUS:
		if( p->in.right->tn.op == ICON ) p->in.right->tn.op = FREE;
		else 
		{
			regrcl( p->in.right );
			tfree( p->in.right );
		}
		q = p->in.left;
		*p = *q;
		q->tn.op = FREE;
		rmside( p );
		return;
	}

	switch( optype( o ) )
	{

	case BITYPE:
		rmside( p->in.right );
		/*FALLTHRU*/
	case UTYPE:
		rmside( p->in.left );
	}
}
void 
expand( p, goal, cp, q )
NODE *p;  
char *cp; 
OPTAB *q;
{
	/* generate code by interpreting table entry */

	CONSZ val;
	NODE *q1, *q2;
	int c;
#if !defined(RTOLBYTES) && !defined(SZFIELD)
	TWORD t;
#endif

/*# ifdef SDB
/*	if( (c = p->tn.op) == CALL || c == UNARY CALL )
/*	{
/*		/* number of args (for sdb) */
/*		pstab( S_NARGS, p->stn.argsize/SZINT);
/*	}
/*# endif
/**/

#ifndef	NODBG
/* ====== DEBUG */
	/* output OP, stinline onto special debug file */
	if (debugout != NULL)
	    fprintf(debugout, "%s\t%d\n", opst[p->in.op], q->stinline);
/* ====== END DEBUG */
#endif

#ifdef CG
#  ifdef EX_BEFORE
	EX_BEFORE(p, goal, q);
#  endif
#endif

	for( ; *cp; ++cp )
	{

		switch( *cp )
		{

#ifdef VOL_OPND_END
		case VOL_OPND_END:
			vol_opnd_end();
			PUTCHAR( VOL_OPND_END );
			continue;
#endif	
		case '\\':
			++cp;
			/* FALLTHRU */
		default:
			PUTCHAR( *cp );
			continue;  /* this is the usual case... */

		case '\n':
			if( zflag )
			{  /* add stin line number of production as a comment */
				fprintf(outfile,
					"\t\t%s %d", COMMENTSTR, q->stinline );
			}
			PUTCHAR( *cp );
#ifdef VOL_SUPPORT
			vol_instr_end();
#endif
			continue;

		case 'Y':
			/* don't check this string: deleted by match */
			continue;

		case 'Z':  /* special machine dependent operations */
			zzzcode( p, &cp, q );
			continue;

		case 'E':	/* exit */
			if( zflag )
			{  /* add stin line number of production as a comment */
				fprintf(outfile,
					"\t\t%s %d", COMMENTSTR, q->stinline );
			}
			PUTCHAR( '\n' );
#ifdef VOL_SUPPORT
			vol_instr_end();
#endif
			return;

# ifndef STACK
		case 'D':	/* test for dependency */
			q1 = getadr( p, &cp );
			if( cp[1] == '!' )
				c=0;
			else if( cp[1] == '=' )
				c=1;
			else
				cerror("bad D option");
			++cp;
			q2 = getadr( p, &cp );
			if( q1->tn.op != REG ) cerror( "bad D" );
			if( ushare( q2, q1->tn.rval ) == c ) continue;
			goto cream;
# endif

		case 'R':  /* put an operand into a particular register */
			/* if the operand is in the register, delete the line */
			q1 = getadr( p, &cp );
			c = (*++cp == '=');
			q2 = getadr( p, &cp );
			if( q1->tn.op != REG )
			{
				if( c ) goto cream;
				continue;
			}
			if( q2->tn.op != REG )
			{
				if( c ) goto cream;
				continue;
			}
			if( q1->tn.rval != q2->tn.rval )
			{
				if( c ) goto cream;
				continue;
			}
			if( c ) continue;
cream:
			/* clobber a line to newline, or up to an E */
			while( *++cp != '\n' && *cp != 'E' ) 
			{
				if( *cp == '\\' ) ++cp; /* hide next char */
				if( !*cp ) return;
			}
			continue;

		case 'F':  /* this line deleted if FOREFF is active */
			if( goal & FOREFF ) goto cream;
			continue;

		case '?':  /* this line retained if FORCC is active */
			if( !(goal & FORCC) ) goto cream;
			continue;

		case 'S':  /* field size */
		case 'H':  /* field shift */
		case 'M':  /* field mask, in place */
		case 'N':  /* field mask, right shifted */
			c = *cp;
			if( cp[1] == '~' ) 
			{
				if( c == 'M' ) c = 'm';
				else if( c == 'N' ) c = 'n';
				else cerror( "bad ~ after S or H" );
				++cp;
			}
			else if( cp[1] == '?' ) 
			{
				if( c == 'H' ) c = 'h';
				else cerror( "bad ? after S, M, or N" );
				++cp;
			}
			q1 = getadr(p,&cp);
			if( q1->tn.op != FLD ) cerror( "bad FLD for %c", c );
			fldsz = UPKFSZ(q1->tn.rval);
# ifdef RTOLBYTES
			fldshf = UPKFOFF(q1->tn.rval);
# else
#  ifdef SZFIELD
			fldshf = SZFIELD - fldsz - UPKFOFF(q1->tn.rval);
#  else
			t = q1->tn.type;
			if( t & (TLONG|TULONG) )
			{
				fldshf = SZLONG - fldsz - UPKFOFF(q1->tn.rval);
			}
			else if( t & (TSHORT|TUSHORT) )
			{
				fldshf = SZSHORT - fldsz - UPKFOFF(q1->tn.rval);
			}
			else if( t & (TCHAR|TUCHAR) )
			{
				fldshf = SZCHAR - fldsz - UPKFOFF(q1->tn.rval);
			}
			else fldshf = SZINT - fldsz - UPKFOFF(q1->tn.rval);
#  endif /* def SZFIELD */
# endif /* def RTOLBYTES */
			if( c == 'h' ) 
			{
				if( fldshf == 0 ) goto cream;
				continue;
			}
			if( c=='S' || c=='H' )
			{
				fprintf(outfile,  "%d", c=='S' ? fldsz : fldshf );
				continue;
			}
			val = pow2m1[fldsz];
			if( c=='M' || c == 'm' ) val <<= fldshf;
			if( c=='m' || c == 'n' ) val = ~val;
# ifdef MYMASK
			MYMASK(val);
# else
			fprintf(outfile,  "%ld" , val );
# endif
			continue;

		case 'L':  /* output special label field */
			fprintf(outfile,  "%d", p->bn.label );
			continue;

		case 'C': /* for constant value only */
			conput( q1 = getadr( p, &cp ) );
			if( !sideff ) cerror( "constant with side effects?" );
			continue;

		case 'I': /* in instruction */
			insput( q1 = getadr( p, &cp ) );
			if( sideff ) rmside( q1 );
			continue;

		case 'A': /* address of */
			adrput( q1 = getadr( p, &cp ) );
			if( sideff ) rmside( q1 );
			continue;

		case 'U': /* for upper half of address, only */
			upput( q1 = getadr( p, &cp ) );
			if( sideff ) rmside( q1 );
			continue;
#ifdef TMPSRET
		case 'T': /* grab a temp for structure return and give it */
			  /* to adrput */
			q1 = talloc();
			q1->tn.op = TEMP;
			q1->tn.type = TSTRUCT;
			q1->tn.lval = freetemp(p->stn.stsize / SZINT);
			q1->tn.lval = BITOOR(q1->tn.lval);
#ifdef MYTMPSRET
	/* this MYTMPSRET macro was added to the machine independent code
	** because adrput was not general enough for long pointers
	** which have parts printed by upput in addition to adrput.
	*/
			MYTMPSRET(q1);
#else
			adrput(q1);
#endif
			tfree(q1);
			continue;
#endif

		}
	}
#ifdef CG
#  ifdef EX_AFTER
	EX_AFTER(p, goal, q);
#  endif
#endif
}

NODE *
getlr( p, c )
NODE *p; 

{

	/* return the pointer to the left or right side of p, or p itself,
	** depending on the optype of p 
	*/

	switch( c ) 
	{

	case '1':
	case '2':
	case '3':
		return( &resc[c-'1'] );

	case 'L':
		return( optype( p->in.op ) == LTYPE ? p : p->in.left );

	case 'R':
		return( optype( p->in.op ) != BITYPE ? p : p->in.right );

	}
	cerror( "bad getlr: %c", c );
	/* NOTREACHED */
}

NODE *
getadr( p, pc )
NODE *p; 
char **pc; 

{
	/* like getlr, but allows (LL), (LR), etc. */
	int c;
	sideff = 1;
	c = *++(*pc);
	if( c == '-' )
	{
		c = *++(*pc);
		sideff = 0;
	}
	if( c == '.' ) return( p );
	else if( c != '(' ) return( getlr( p, c ) );

	for(;;) 
	{
		switch( c = *++(*pc) ) 
		{

		case ')':
			return( p );

		case 'L':
			p = getl( p );
			continue;

		case 'R':
			p = getr( p );
			continue;

		default:
			cerror( "bad table char in (): %c", c );
		}
	}
	/* NOTREACHED */
}

static int
spshape( sh, p )
NODE *p; 

{
	long val;
	int t, i;

	if( !(sh&SPECIAL) ) cerror( "spshape" );
	if( sh&SPTYPE )
	{
# ifndef NODBG
		if( sdebug )
			fprintf(outfile,
			    "SPTYPE(%d, %o), ttype=%o\n", node_no(p), sh, p->tn.type );
# endif

		if( sh & p->tn.type ) return( 1 );
		else return( 0 );
	}
	t = (sh&STMASK);
	i = (sh&SVMASK);

	switch( t ) 
	{

	case SRANGE0:
	case SSRANGE:
		if( i<0 || i>31 ) cerror( "bad shape range" );
		/*FALLTHRU*/
	case SVAL:
	case SNVAL:
	case NACON:

		if( p->tn.op != ICON || p->tn.name != (char *) 0 ) return( 0 );
		if( t == NACON ) return(1);
		val = p->tn.lval;
		if( t==SVAL && val == i )
		    return( 1 );
		else if( t==SNVAL && val == -i )
		    return( 1 );
		else if( t==SRANGE0 && val >= 0 && val <= pow2m1[i] )
		    return( 1 );
		else if(   t==SSRANGE
			&& val >= -((long) pow2m1[i])-1
			&& val <= (long) pow2m1[i]
		     )
		    return(1);
		return( 0 );

	case SUSER:
		return( special( sh, p ) );

	default:
		cerror( "bad special call" );
		/* NOTREACHED */
	}
}
