/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/cost.c	1.16"

# include "mfile2.h"

# ifndef DFLT_STRATEGY
# define DFLT_STRATEGY	EITHER
# endif


/* RS_EVENREG is mask of "even" register bits */
#if !defined(ODDPAIR) && !defined(RS_EVENREG)
extern RST rs_evenreg;
# define RS_EVENREG rs_evenreg
#endif

	/* used to identify subtrees */
# define NSUBTREES 10

static int nsubtree;
static NODE *subtree[NSUBTREES];
static int subgoal[NSUBTREES];
static RST sub_rs_want[NSUBTREES];	/* desired register for subtree result */
#ifndef	MYPICKST
static int pickst();
#endif
static void findsub(), makesub();
static RST bprt();

void
commute( p ) NODE *p; {
	/* commute p in place */
	register NODE *q;
#ifndef NODBG
	if(odebug)
	    fprintf(outfile, "commute: .=%u l=%u r=%u\n",p,p->in.left,p->in.right);
#endif

#ifdef CG
	/*If this is an ordered node, reverse the ordering*/
	if (firstl(p))
        {
                p->in.strat ^= (LTOR | RTOL);
        }
#endif

	q = p->in.left;
	p->in.left = p->in.right;
	p->in.right = q;
	}

lhsok( p )
NODE *p;
{
	/* p appears on the lhs of an assignment op */
	/* is it an OK substitute for a TEMP? */

	switch( p->tn.op ) {

	case NAME:
	case VAUTO:
	case VPARAM:
	case TEMP:
	case REG:
		return( 1 );

	}
	return( 0 );
}

#ifndef NODBG
void 
shpr(sp)
register SHAPE * sp;
{
    if (!sp) return;
    if (sp->op < 0 || sp->op > DSIZE) cerror( "shape op %d\n", sp->op );
    PUTS( opst[sp->op] );
    shpr(sp->sl);
    shpr(sp->sr);
}

void
pstrat( s ) {
	/* print a nice version of the strategy s */
	register i, flag;
	static char *stratnames[] = {
		"STORE",
		"LTOR",
		"RTOL",
		0 };
	flag = 0;
	for( i=0; stratnames[i]; ++i ){
		if( s & (1<<i) ) {
			if( flag ) PUTCHAR( '|' );
			PUTS( stratnames[i] );
			flag = 1;
			}
		}
	if( !flag ) PUTCHAR( '0' );
	}
#endif

RST
insout( p, goal, haveregs, rs_avail, rs_want )
NODE *p;
int goal;
int haveregs;			/* how many regs available for generating */
RST rs_avail;			/* vector of avail. reg. bits */
RST rs_want;			/* vector of regs we'd like result in */
{
	OPTAB *q;

	/* generate the actual instructions */

	do {
#ifndef NODBG
	    if( odebug > 1 )
		fprintf(outfile,  "insout(%d,%d,%d,%#o,%#o)\n",
				node_no(p), goal, haveregs, rs_avail,
				rs_want );
#endif

	    /* If goal is <= NRGS, better make sure tree goal is NRGS
	    ** so match() will pick suitable template.
	    */
	    if (goal <= NRGS && p->in.goal != NRGS) {
#ifndef	NODBG
		if ( odebug > 1 )
		    fprintf(outfile, "changing goal, node %d, from %d to %d\n",
						    node_no(p), p->in.goal, NRGS);
#endif
		p->in.goal = NRGS;
	    }

	    /* take first match */
	    if ( (q = match(p, (OPTAB *) 0)) != 0 ) 
		    return bprt( p, q, goal, haveregs, rs_avail, rs_want );

	    switch( p->tn.op ) {
	    case COMOP:			/* didn't match! */
		cerror( "COMOP match fails" );
	    	/* try commuting these OPs */
		/*FALLTHRU*/
	    case PLUS:
	    case MUL:
	    case OR:
	    case AND:
	    case ER:
		commute( p ); 
		if ( (q = match(p, (OPTAB *) 0)) != 0 )  
			return bprt( p, q, goal, haveregs, rs_avail, rs_want );
	    }

	    /* no match found for current OP; try rewrites */

	} while ( !rewass(p) );	/* returns 1 on major rewrite */

    return REWROTE;			/* rewass() did major rewrite */
}

SHAPE * leftshape;			/* these two filled in by match */
SHAPE * rightshape;

static RST
bprt( p, q, goal, haveregs, rs_avail, rs_want )
NODE *p;
OPTAB *q;
int goal;
int haveregs;				/* registers available for tree */
RST rs_avail;				/* these regs available */
RST rs_want;				/* want result in one of these */
{
	/* this routine is called to print out the actual instructions */
	/* it is called with a tree node p, a template q, and a goal goal */
	/* bprt calls bcost, and then captures the left and right shapes */
	/* it then uses findsub to determine the preconditions and goals */
	/* a local copy of this information must be made, since bprt can be
	/* called recursively */
	/* then, bprt calls insout to output the instructions that establish
	/* the preconditions.  Finally, it can output its own instruction */

#define is_one_reg(b)	((b) && ((b) & (b-1)) == 0)
	RST rs_reclaim();
	int curregs;			/* registers still available */
	int s, o, k;
	NODE *l, *r;
	int nn;
	int needregs;
	int mygoal[NSUBTREES];
	NODE *mysubs[NSUBTREES];
	RST myrs[NSUBTREES];		/* returned reg. set for each s.t. */
	RST mywant[NSUBTREES];		/* local wanted result registers */
	RST rs_left, rs_right;		/* OR of regs used by left, right */
	RST unavail_left;		/* Registers not available on left 
					** because right subtree matched
					** single register.
					*/
	RST unavail_right;		/* as above, but on right */
	RST cur_rs;			/* working set of registers */
	int lstart, lend, subs;
					/* mysubs[n] is left sub-tree */
#define	is_lst(n) (lstart <= (n) && (n) < lend)
	RST want;			/* "want" set for subtree */
	RST leftwant, rightwant;	/* restrictions to wanted set */
	RST rc;
	int savins;
	SHAPE * ls = leftshape;		/* make local copies */
	SHAPE * rs = rightshape;
/*	NODE *rewrite = 0;		/*subtree to rewrite into temp*/
	NODE *try_rewrite = 0;		/*Suggested subtree to rewrite into temp*/
	NODE *first;
	int do_rewcom = 0;		/*1 if we need an extra rewcom*/
	RST poss_regs;
	int poss_cnt = 0;

	/* if we need to produce a result in a register and no
	** suitable registers are available, give up now.
	*/
	if (goal <= NRGS && (rs_want & rs_avail) == 0)
	    return OUTOFREG;

	l = getl( p );
	r = getr( p );
	if (q->rshape && !q->lshape)
		r = p;

	/* set some initial values, in case we bail out early for ,OP or
	** INCALL; rs_reclaim will need these if we don't go through the
	** subtree loops.
	*/
	cur_rs = rs_avail;
	rs_left = rs_right = 0;

	/*choose the strategy*/
	s = pickst(p);
	o = p->tn.op;
# ifndef NODBG
	if( odebug>1 ) {
		fprintf(outfile,  "	matches %d, ls = %d(%s), rs = %d(%s),  s= ",
			q->stinline, ls-shapes,
			ls?opst[ls->op]:"SHNL",
			rs-shapes,
			rs?opst[rs->op]:"SHNL" );
		pstrat( s );
		PUTCHAR( '\n' );
		}
# endif

#ifdef IN_LINE
	/* don't do anything with INCALL trees. 
	/* just stick it in inst[] and handle it in insprt()
	*/
	if( o==INCALL || o==UNARY INCALL ) {
	    if (haveregs < NRGS)
		return( OUTOFREG );	/* assume need all regs */
	    goto generate;
	}
#endif

#ifdef CG
	/* For CG: LET's are handled differently. First, do the left side.
	   Then, do the right side; if we run out of registers,
	   then try temps*/
	if (p->in.op == LET)
	{
		RST docse();

		if( (rc = docse(p,goal,haveregs, rs_avail, rs_want,
			ls->sregset,  &rs_left)) & RS_FAIL)
			return rc;
		rs_right = rc;
#ifndef NODBG
		if (odebug)
			fprintf(outfile, "LET %d rhs done regs=0%o\n", node_no(p), rc);
#endif
		goto generate;
	}
#endif


	/* handle COMOP differently; this has more to do with the register
	/* allocation than the ordering */

	if( o == COMOP ) {
			/*COMOP's require all of the registers*/
			/*only time this is a problem is with GENLABs
			  (since commas can't rewrite past them)*/
		if ( haveregs < NRGS)
			return OUTOFREG;
		if ((rc = insout( l, CEFF, haveregs, rs_avail, RS_NONE )) != 0 )
			return rc;
		if ((rc = insout( r, goal, haveregs, rs_avail, rs_want )) & RS_FAIL)
		    return rc;
		rs_right = rc;		/* for rs_reclaim subtree */
		cur_rs -= (rc & RS_NRGS); /* mark result regs unavailable */
		goto generate;
	}
			/*remember how far we have gotten incase we back up*/
	savins = nins;
			/*try the subtrees according to strategy s:*/
dosubs:
	nsubtree = lstart = lend = 0;
			/*lstart and lend bound the left subtrees*/
	if(rs && s == RTOL) {
		findsub( r, rs );
#ifdef CG
			/*copy register variables, if OCOPY is set*/
		if (r->tn.op == REG && (r != p) && (r->in.strat & OCOPY) &&
			(RS_BIT(r->tn.rval) & rs->sregset) )
			makesub(r);
#endif
	}

	if( ls ) {
		lstart = nsubtree;
		findsub( l, ls );
		/* we must arrange to copy a reg variable on the lhs
		/* of a binary op, in some cases (cf. bcost) */
		if (    asgop(q->op)
		    && !asgop(o)
		    && l->tn.op == REG
		    && lstart == nsubtree	/* none so far */
		)
			makesub( l );
#ifdef CG
			/* Also, copy the regvar if the OCOPY flag is set*/
		else if (l->tn.op == REG && l != p && (l->in.strat & OCOPY) &&
			(RS_BIT(l->tn.rval) & ls->sregset) )
			makesub(l);
#endif
		lend = nsubtree;
		}

	if(rs && ((s == LTOR) || (s == EITHER) ) ) {
		findsub( r, rs );
#ifdef CG
		if (r->tn.op == REG && (r != p) && (r->in.strat & OCOPY) &&
			(RS_BIT(r->tn.rval) & rs->sregset) )
			makesub(r);
#endif
	}

	nn = nsubtree;

	/* make a local copy,counting registers */
	needregs = q->needs & NCOUNT;

	poss_regs = RSB(q->rneeds)[1];
	poss_cnt = 0;

	/* Assume we'll pick up problems with pairs later when
	** we check scratch registers.  This is really just a
	** quick sanity check.
	*/
	if ( q->needs & NPAIR ) needregs *= 2; /* account for pairs */

	subs = 0;
	for( k=0; k<nn; ++k ) 
	{
		mygoal[k] = subgoal[k];
		mysubs[k] = subtree[k];
		mywant[k] = sub_rs_want[k];
		poss_regs |= mywant[k];
		if ( mygoal[k] == NRGS)
		{
			subs += szty(mysubs[k]->tn.type);
			/*Only count registers if they can't be shared*/
			if (is_lst(k))	/*left subtree*/
			{
				if ( !(q->needs & LSHARE)) 
					needregs += szty(mysubs[k]->tn.type);
			}
			else	/*right subtree*/
			{
				if ( !(q->needs & RSHARE)) 
					needregs += szty(mysubs[k]->tn.type);
			}
		}
	}
	poss_regs &= rs_avail;
	while (poss_regs) {
	    poss_cnt++;
	    poss_regs = poss_regs & (poss_regs-1);	/* turn off low bit */
	}
	if (poss_cnt > haveregs)
		cerror("too many registers available");

	/*Need at least a reg for each subtree*/
	if ( needregs < subs)
		needregs = subs;
#ifndef NODBG
	if (e2debug)
	{
		if ( needregs > poss_cnt )
			PUTS( "Out of registers: ");
		fprintf(outfile, "%d needs %d regs, have %d regs\n",
				node_no(p), needregs, poss_cnt);
	}
#endif

	if (needregs > poss_cnt) {	/* We're out of registers */
	    if ( haveregs >= NRGS) {

	    /* we're out of REGs, even though we have access to all of them.
	    ** Choose a sub-tree to throw into a TEMP and try again.  If we
	    ** fail, panic.  Scramble takes care of this.
	    */

		goto scramble;
	    }
	    return OUTOFREG;	/* let someone at higher level figure 
				** out what to do
				*/
	}

# ifndef NODBG
	if( odebug>1 ) {  /* subtree matches are: */
		fprintf(outfile,  "\t\t%d matches\n", nn );
		for( k=0; k<nn; ++k ) {
			fprintf(outfile, "\t\tnode %d, goal %d\n",
					node_no(mysubs[k]), mygoal[k] );
			}
		}
# endif

	/* do the subtrees */

	curregs = haveregs;
	cur_rs = rs_avail;		/* currently available registers */
	rs_left = rs_right = 0;		/* no subset regs yet */
	leftwant = rightwant = RS_TOT;
					/* start off with no want-set restrictions */

	/* If a shape on the one side matches a specific register,
	** remove the register from the available set on the other side.
	*/
	unavail_right = unavail_left = 0;
	if (	(rs->op == REG) && 
		(is_one_reg(rs->sregset))
	   ) 
		unavail_left = rs->sregset;
		
	if (	(ls->op == REG) && 
		(is_one_reg(ls->sregset))
	   )
		unavail_right = ls->sregset;

	/* If a sub-tree shape is REG, and if the template's result is
	** for that side, restrict the subtree "want" set to this tree's
	** "want" set.
	*/
	if (       (q->rewrite & (RLEFT|REITHER))
		&& ls->op == REG	/* left shape is REG */
		&& lstart != lend	/* there is a left subtree */
	    )
	    leftwant = rs_want;
	else if (  (q->rewrite & (RRIGHT|REITHER))
		 && rs->op == REG	/* right shape is REG */
					/* there is a right subtree */
		 && (lstart > 0 || lend <= nsubtree)
	    )
	    rightwant = rs_want;

	/* If a template has certain registers that are unconditionally
	** required by it, don't pass them along as wanted registers
	** unless they can be shared.  This code can only deal with the
	** single need case.
	*/
	if (q->rneeds) {		/* template needs scratch regs. */
	    RST * rneeds = RSB(q->rneeds);
	    /* check if only one set and one vector */
	    if (rneeds[0] == 1 && rneeds[3] == 0) {
		RST mustscratch = rneeds[1];	/* rneeds[1] == rneeds[2] */
		if ((q->needs & LSHARE) == 0)
		    leftwant &= ~mustscratch;
		if ((q->needs & RSHARE) == 0)
		    rightwant &= ~mustscratch;
	    }
	}

	/* If template is "op=" and tree is "op", restrict
	** left-side registers to scratch registers.
	*/
	if (asgop(q->op) && !asgop(p->in.op))
	    leftwant &= RS_NRGS;

	for( k=0; k<nn; ++k ) {
		NODE * tsub = mysubs[k];
		int tgoal = mygoal[k];
		RST mywantsubk = mywant[k];
		int instnow = nins;		/* remember # of inst. so far */
		int tmp_regs = curregs;
		int tmp_rs = cur_rs;

		/* Generate the code for the subtree.
		** In RCC, if goal is not NRGS, we assume we have all the regs.
		** This is not true in CG.
		*/

		want = mywantsubk;

		/* Choose "want" set with restriction, if any */
		if (is_lst(k)) {	/* left sub-tree */
		    want &= leftwant;
		    if (unavail_left & tmp_rs) {
			want &= ~unavail_left;
			tmp_rs &= ~unavail_left;
			tmp_regs--;
		    }
		}
		else {			/* right sub-tree */
		    want &= rightwant;
		    if (unavail_right & tmp_rs) {
			want &= ~unavail_right;
			tmp_rs &= ~unavail_right;
			tmp_regs--;
		    }
		}

		if (!want && tgoal <= NRGS)
		    want = mywantsubk;	/* zero is a sure loser */

# ifndef NODBG
		if( odebug>2 )
			fprintf(outfile,  "\t\tcalling insout(%d,%d,%#o,%#o)\n",
					node_no(tsub), tmp_regs, tmp_rs, want );
# endif

		for (;;) {		/* two loops max. with different "want" */
		    rc = insout(	tsub,
					/*Goal:*/
					tgoal,
					/*Number of available registers:*/
#ifdef CG
					tmp_regs,
#else
					(tgoal == NRGS ? curregs : NRGS),
#endif 

					tmp_rs,
					want
				);
		    if (rc == REWROTE) return( REWROTE );
		    if ((rc & RS_FAIL) == 0 || want == mywantsubk)
			break;
		    want = mywantsubk;	/* try sub-optimal case */
		    nins = instnow;	/* back up instruction stream */
		}
		switch( rc ) {		/* how'd we do? */
		default:		/* success */
		    myrs[k] = rc;	/* save result REG */
		    if (tgoal == NRGS) {
			curregs -= szty(tsub->tn.type);
			/* remember subtree's reg on left/right side */
			if (is_lst(k))	/* left sub-tree */
			    rs_left += rc;
			else		/* right sub-tree */
			    rs_right += rc;
			/* sub-tree register may be outside set of currently
			** available registers (might be reg. variable)
			*/
			cur_rs -= rc & RS_NRGS;	/* remove subtree's scratch
						** register(s)
						*/
		    }
		    break;

		case REWROTE:	return REWROTE;

		case OUTOFREG:	/*If out of registers, try to recover:*/
		    {
#ifndef NODBG
			if (odebug)
				fprintf(outfile, "Subtree %d out of regs\n",
							node_no(tsub));
#endif
			/*Try the other ordering, if possible*/

			if ( s == EITHER)
			{
#ifndef NODBG
			    if (odebug)
				fprintf(outfile, "%d First order fails;try RTOL\n",
					    node_no(p));
#endif
			    s = RTOL;
			    nins = savins;
			    goto dosubs;
			}

			/*otherwise, rewrite to temps at highest level*/

			/*for RCC, it does no good to rewrite a tree with
			  goal != NRGS; but for CG, where the tree may
			  not have all regs available, it may help.*/
#ifdef CG
			if ( haveregs != NRGS)
#else
			if ( haveregs != NRGS || tgoal != NRGS)
#endif
				return OUTOFREG;

			/* If the problem is that a wanted register is
			** unavailable, try to spill a sub-tree occupying it,
			** as long as it isn't a leaf node.
			*/

			if ((cur_rs & mywant[k]) == 0) {
			    int i;
			    RST want = mywant[k];

			    for (i = 0; i < k; ++i) {
				if (   (myrs[i] & want)
				    && optype(mysubs[i]->in.op) != LTYPE
				    && rewsto(mysubs[i])
				) {
#ifndef	NODBG
				    if (odebug)
					fprintf(outfile,
					"Subtree %d goes into TEMP (for reg %o)\n",
						node_no(mysubs[i]), want);
#endif
					return( REWROTE );
				}
			    }
			}
			/* couldn't spill for particular register */

			/* rewrite sub-tree to TEMP, if possible */
			/*For CG: if this op is unordered, and the
			  subtree is for nrgs , rewrite it; else let
			  scramble choose*/
#ifdef CG
			if ( !firstl(p) && tgoal == NRGS &&
			   !(tsub->in.strat & (LTOR|RTOL)))
				try_rewrite = tsub;
#else
#  ifndef NODBG
			if (odebug)
				fprintf(outfile, "Subtree %d goes into TEMP\n",
					node_no(tsub));
#  endif
			try_rewrite = tsub;
#endif
			goto scramble;
		    }
		}
	}

	/* put onto the instruction string the info about the instruction */
    generate:
	/* sanity check; rs_left and rs_right must be set correctly */
	if (rs_left & rs_right)
	    cerror("subtrees share registers!");

	rc = rs_reclaim(p, q, goal, ls, rs_left, rs, rs_right, cur_rs, rs_want);
	/* make sure this worked */
	if (rc & RS_FAIL) {
	    /* Try to find a sub-tree that's sitting in a register we
	    ** think we need and try to rewrite it.
	    */
#ifndef	NODBG
	    if (odebug)
		fprintf(outfile, "Not enough scratch registers, need 0%o\n",
								rc & ~RS_FAIL);
#endif
	    for (k=0; k<nn; ++k) {
		NODE * subtree = mysubs[k];
		if (   (myrs[k] & rc)
		    && optype(subtree->in.op) != LTYPE
		    && rewsto(subtree)) {
		    if (odebug)
			fprintf(outfile, "Spill subtree %d to TEMP\n",
						node_no(subtree));
		    return( REWROTE );
		}
	    }
	    return( OUTOFREG );		/* oh well, hope for the best */
	}
#ifndef NODBG
	if (odebug)
		fprintf(outfile,
			"Generate #%d: node %d template %d result regs 0%o\n",
			nins, node_no(p), q->stinline, rc);
# endif
	if( nins >= NINS )
	    td_enlarge(&td_inst, 0);	/* expand instruction table */
	inst[nins].p = p;
	inst[nins].q = q;
	inst[nins].goal = goal;
	inst[nins].rs_want = rs_want;	/* remember desired result regs */
	/* Remember avail. regs for cfix(); scratch result regs. will be
	** available, too, as will left and right regs.
	*/
	inst[nins].rs_avail = cur_rs | ((rc | rs_left | rs_right) & RS_NRGS);
	/* a special case: REG op= xxx, should be done as early as possible */
	if( asgop(o) && p->in.left->tn.op == REG && o != INCR && o != DECR
			&& goal!=CEFF && goal!=CCC && !istreg(p->in.left->tn.rval)){
		/* "istreg" guards against rewriting returns, switches, etc. */
		if (rewsto(p))
		    return( REWROTE );		/* indicate we rewrote this */
		else
		    inst[nins].goal = CTEMP;
	}
	++nins;
#ifdef STATSOUT
	if (td_inst.td_max < nins) td_inst.td_max = nins;
#endif
	return rc;			/* return result regs used */

scramble:
	/*Come here when we are out of registers,
	  even though we have access to all of them.  Choose a subtree
	  to throw into a TEMP and try again. If we fail, panic.
	  For rcc: For now, just rewrite the first suitable subtree.
	  We make the assumption (with crossed fingers) that a leaf
	  node is an inappropriate candidate for spilling to a TEMP:
	  Usually TEMPs, AUTOs, and PARAMs are equivalent as operands.*/

	/*For CG, If this is an ordered node, and the first side is
	  not for NRGS, it cannot help to rewrite the node. Descend the
	  second side.*/
#ifdef CG
        while (((first=firstl(p)) != 0) && first->in.goal != NRGS)
        {
                p = lastl(p);
                try_rewrite = 0;
        }
#endif
				  
			/*If no rewrite suggested, CG
			  should pick the first side of an ordered node*/
#ifdef CG
	if (try_rewrite == 0 &&  (try_rewrite = firstl(p)) != 0)
	{
		/* If this is an ordered semi, do an extra rewcom.
		  This makes sure the first side gets written
		  higher before the unordered semi gets rewritten
		  to a COMOP*/
		if ( p->in.op == SEMI)
			do_rewcom = 1;
	}
#endif
			/*If a rewrite is suggested, try it*/
			/*For CG, its OK to try to rewrite leaves;
			  ordered nodes may make it neccessary*/
#ifdef CG
	if ( try_rewrite && rewsto(try_rewrite) )
#else
	if ( try_rewrite && optype(try_rewrite->in.op) != LTYPE &&
		rewsto(try_rewrite) )
#endif
	{
#ifndef NODBG
		if (odebug)
			fprintf(outfile, "Picked subtree %d spills to TEMP\n",
						node_no(try_rewrite));
#endif
#ifdef CG
			/*Rewriting the first side of an ordered
			  node to TEMP removes the need for ordering*/
		if ( try_rewrite == firstl(p))
			unorder(p);
		if ( do_rewcom)
			rewcom(p, p->tn.goal);
#endif
		return REWROTE;
	}

			/*Otherwise, take the first one*/
	for(k = 0; k < nn; ++k)
	{
		NODE *subtree;
		if (    mygoal[k] == NRGS
		     && optype( (subtree = mysubs[k])->in.op) != LTYPE
		     && rewsto(subtree)
		) {
#ifndef NODBG
                if (odebug)
                        fprintf(outfile, "Subtree %d spills to TEMP\n",
                                node_no(subtree));
#endif
#ifdef CG
		if ( do_rewcom)
			rewcom(p, p->tn.goal);
#endif
			return REWROTE;
		}
	}
	return OUTOFREG;	/*Pass the buck. Let some
				  one at a higher level figure out
				  what to do*/
}

static void 
findsub( p, s )
NODE *p;
SHAPE *s;
{
	register newgoal;
	RST regwant = RS_NONE;		/* assumed registers needed */

	if( !s )
		return;

# ifndef NODBG
	if( e2debug>1 ) {
		fprintf(outfile,  "\t\tfindsub( %d, %d )\n", node_no(p), s-shapes );
		}
# endif

	switch( s->op ) {

	case TEMP:
		if( p->tn.op == TEMP ) return;
		newgoal = CTEMP; break;

	case FREE:	newgoal = CEFF; break;
	case CCODES:	newgoal = CCC; break;
	case REG:
		/* this is an exact match if the tree node is REG
		** (must be register variable at this stage) and
		** the register variable belongs to the shape's
		** register set
		*/
		if(
		        p->tn.op == REG
		    && (RS_BIT(p->tn.rval) & s->sregset) != 0
		    ) return;		/* exact match */

		newgoal = NRGS;
		regwant = s->sregset;	/* want result in shape's registers */
		break;

	    default:
		if( s->op == p->tn.op ) {
		    /* look at subtrees; for callops, do right to left */
		    if (callop(s->op)) {
			if( s->sr ) findsub( getr(p), s->sr );
			if( s->sl ) findsub( getl(p), s->sl );
		    }
		    else {
			if( s->sl ) findsub( getl(p), s->sl );
			if( s->sr ) findsub( getr(p), s->sr );
		    }
		    return;
		}
	} /* end switch */

	/* record new subtree */
	subtree[nsubtree] = p;
	subgoal[nsubtree] = newgoal;
	sub_rs_want[nsubtree] = regwant;
	++nsubtree;			/* have another subtree */
}

#ifndef	MYPICKST
static 
pickst(p)
NODE *p;
{
	register int s;
	register int o = p->in.op;


	/* determine the code generation strategy */

#ifdef CG
	/*Was one specified?*/

	if ( (s = (p->in.strat & (LTOR|RTOL))) != 0 ) 
		return s;
#endif

	s = LTOR;  /* default strategy */

	if( optype(o) == BITYPE ) {

		switch( o ) {

		case CALL:
		case STCALL:
		case FORTCALL:
# ifdef IN_LINE
		case INCALL:	/* handle asm calls like regular calls */
# endif
# ifndef LTORARGS
		case CM:  /* function arguments */
# endif
			s = RTOL;
			break;

		default:
# ifdef STACK
			if( asgop(o) ) s = RTOL;
			else s = LTOR;
# else
			s = DFLT_STRATEGY;
# endif
			break;
		}
	}
	return s;
}
#endif	/* ndef PICKST */
/* rs_reclaim determines what register (if any) the result of a
** template will appear in.  We try to anticipate the actions of
** both allo() and reclaim() (allo.c).  In some ways, though, the
** job is easier here.  Inputs are the tree and template for
** which we're generating code, the registers used by the left
** and right subtrees, the left and right shapes that matched,
** and the registers available at this point.  We must figure out
** where the result is and whether the scratch registers that are
** available are sufficient.  If so, we return the bit vector
** corresponding to the result.  If not, we return a failure flag
** plus a bit vector of registers that we needed and didn't have.
*/

RST
rs_reclaim(tnode, tmplt, goal, lshape, lregs, rshape, rregs, avail, want)
NODE * tnode;			/* node we're generating code for */
OPTAB * tmplt;			/* template that matched */
int goal;			/* what we're generating code for */
SHAPE * lshape;			/* left shape that matched */
RST lregs;			/* bit vector of registers used by left */
SHAPE * rshape;			/* right shape that matched */
RST rregs;			/* bit vector of registers used by right */
RST avail;			/* bit vector of available registers */
RST want;			/* bit vector of wanted result regs */
{
    RST result = RS_NONE;
    RST shared = RS_NONE;	/* registers shared with left or right */
    RST scratch = RS_NONE;	/* selected scratch registers (assume none) */
    int rewrite = tmplt->rewrite;
    int resultsize = szty(tnode->in.type);
				/* number of registers in result */

#ifndef	NODBG
    if (rdebug)
	fprintf(outfile,
	"rs_reclaim called:  node %d, lregs %#o, rregs %#o, avail %#o, want %#o\n",
				node_no(tnode), lregs, rregs, avail, want);
#endif

    /* Step 1.  Figure out disposition of scratch registers */

    /* sharing effectively adds to the set of available registers */

    if (tmplt->needs & LSHARE) {
	if (lregs & avail)
	    cerror("left registers prematurely available");
	shared |= lregs;
    }
    if (tmplt->needs & RSHARE) {
	if (rregs & avail)
	    cerror("right registers prematurely available");
	shared |= rregs;
    }
    avail |= shared;

    /* try to select scratch registers */

    if (tmplt->rneeds) {		/* do we need any scratch regs? */
	RST pref = RS_NONE;		/* no preferred registers */
	static RST getscratch();

	/* Make a half-hearted attempt to choose "preferred registers":
	** if LPREF or RPREF is set, and if there are registers on the
	** corresponding side, try to choose a scratch register set that
	** contains the desired register(s).  In fact, the following
	** test only chooses a scratch register set with AT LEAST one
	** of the preferred registers, but that's usually good enough.
	*/
	if ((tmplt->needs & LPREF) && lregs)
	    pref = lregs;
	if ((tmplt->needs & RPREF) && rregs)
	    pref |= rregs;

	scratch = getscratch(tnode, tmplt->rneeds, rewrite, avail, want, pref);
	if (scratch & RS_FAIL) {	/* failed to get scratch regs */
#ifndef	NODBG
	    if (rdebug)
		fprintf(outfile, "rs_reclaim fails, returns %#o for node %d\n",
			    scratch, node_no(tnode));
#endif
	    return( scratch );		/* return mask of desired registers */
	}
    }
    else
	tnode->in.scratch = 0;		/* no scratch registers needed */

    /* Step 2.  Figure out result register, if any */

    if (goal <= NRGS) {			/* only return reg. for these */
	if (rewrite & REITHER) {	/* result on "better" side */
	    /* Take right side if REG and left side is not REG.  Take
	    ** left side otherwise.
	    */
	    if (rshape && rshape->op == REG && !(lshape && lshape->op == REG))
		rewrite ^= (RRIGHT | REITHER); /* change apparent result */
	    else
		rewrite ^= (RLEFT | REITHER);
	}

	/* Leaf templates get handed in as right sub-trees.  Move
	** stuff around to satisfy later need for left or right
	** subshape tree, which would be a sub-tree of rshape.
	*/
	if (rewrite & RLEFT) {		/* result on left */
	    if (rshape && !lshape) {	/* leaf template */
		lshape = rshape;
		lregs = rregs;
		if (optype(lshape->op) != LTYPE)
		    lshape = lshape->sl;
	    }
	    if (lshape && lshape->op == REG) {
		/* If register variable on left, result is that variable.
		** However, if lregs != 0, there is a sub-tree on the left
		** that implies the REG was moved to a scratch reg.
		*/
		if (tnode->in.left->tn.op == REG && !lregs) {
		    result = RS_BIT( tnode->in.left->tn.rval );
		    if (resultsize == 2)
			/* grab adjacent register, too */
			result |= RS_PAIR(result);
		}
		else
		    result = lregs;	/* registers in left set */
	    }
	}
	else if (rewrite & RRIGHT) {	/* result on right */
	    /* Check for leaf template. */
	    if (rshape && !lshape && optype(rshape->op) == BITYPE)
		rshape = rshape->sr;

	    if (rshape && rshape->op == REG) {
		if (tnode->in.right->tn.op == REG && !rregs) {
		    result = RS_BIT( tnode->in.right->tn.rval );
		    if (resultsize == 2)
			/* grab adjacent register, too */
			result |= RS_PAIR(result);
		}
		else
		    result = rregs;	/* registers in right set */
	    }
	}
	else if (rewrite & RESC1|RESC2|RESC3) {
	    static RST getscrresult();
	    result = getscrresult(tnode, tmplt, rewrite);
	}
	/* For goal <= NRGS, we MUST claim to return a value, and
	** in the proper register.  All result registers must be
	** in "wanted" set.
	*/
	if (!result || (result & ~want) != 0) {	/* either none, or in wrong reg. */
	    /* Anticipate cfix()'s action.  See if we can choose
	    ** a register from "wanted" registers.  After the code
	    ** for this template has been generated, the subtree
	    ** registers and the scratch registers will all be
	    ** available again.
	    ** The selected register must be suitable for the type
	    ** of the result.
	    */
	    RST tscratch = (avail|scratch|lregs|rregs) &
				want &  RS_FORTYPE(tnode->in.type);

	    while (tscratch != 0) {
		result = RS_CHOOSE( tscratch );
		tscratch -= result;	/* forget this bit, now */
		if (resultsize <= 1) break;
		/* if not ODDPAIR, reg must be "even"; in either
		** case, second reg must be avail
		*/
		else if (
#ifndef ODDPAIR
			   (result & RS_EVENREG) != 0 &&
#endif
			(RS_PAIR(result) & tscratch) != 0
			) {
			    result |= RS_PAIR(result);
			    break;
		}
		else
		    result = 0;		/* make sure it looks like we failed */
	    }
	    if (result == 0) {		/* still no luck! */
		result = RS_FAIL | want;
#ifndef	NODBG
		if (rdebug)
		    fprintf(outfile,
			"rs_reclaim fails (want), returns %#o for node %d\n",
							want, node_no(tnode));
#endif
		return( result );
	    }
	}
    } /* end if NRGS */
#ifndef NODBG
    if (rdebug)
	fprintf(outfile, "rs_reclaim succeeds, returns %#o for node %d\n",
							result, node_no(tnode));
#endif
    return( result );
}
/* getscratch -- choose scratch registers for current node
**
** needoffset is offset in register bit vector table to structure
** like this:
**	nvec	number of bit vectors per need
**	OR of all scratch registers for this need
**	m * |	OR of vectors for this set of scratch registers
**	    |		nvec individual scratch bit vectors
*/

static RST				/* returned selected or desired regs */
getscratch(p, needoffset, rewrite, avail, want, pref)
NODE * p;				/* node to choose scratch for */
int needoffset;				/* offset of register bit vectors */
int rewrite;				/* desired rewrite for template */
RST avail;				/* available registers */
RST want;				/* desired result registers */
RST pref;				/* "preferred" scratch registers */
{
    RST * rneeds = RSB(needoffset);	/* pointer to register bit vectors */
    int nvec = *rneeds + 1;		/* get number of vectors per needs set */
    register RST * sptr;		/* pointer to scratch register vectors */
    register RST scratch;
    RST notavail;			/* ~(registers available) */

    if (nvec == 2)
	nvec = 1;			/* total vector same as individual */

    rneeds += 2;			/* skip to first real bit vector set */

    if (pref) {				/* if there are any preferences */
	/* if result is in scratch register, it would be nice to choose
	** scratch registers from the wanted set; try them first
	*/
	if ((rewrite & (RESC1|RESC2|RESC3)) && (notavail = want & avail)) {
	    notavail = ~notavail;
	    for ( sptr = rneeds; (scratch = *sptr) != 0; sptr += nvec ) {
		if ((scratch & notavail) == 0 && (scratch & pref))
		    goto gotscratch; /* found suitable set */
	    }
	}
	notavail = ~avail;	/* try to choose from available set */
    
	for ( sptr = rneeds; (scratch = *sptr) != 0; sptr += nvec ) {
	    if ((scratch & notavail) == 0 && (scratch & pref))
		goto gotscratch;	/* found suitable set */
	}
    }

    if ((rewrite & (RESC1|RESC2|RESC3)) && (notavail = want & avail)) {
	notavail = ~notavail;
	for ( sptr = rneeds ; (scratch = *sptr) != 0; sptr += nvec ) {
	    if ((notavail & scratch) == 0)
		goto gotscratch;
	}
    }
    
    /* last resort:  grab any suitable scratch register set */
    notavail = ~ avail;
    for ( sptr = rneeds ; (scratch = *sptr) != 0 ; sptr += nvec ) {
	/* are all bits available? */
	if ((notavail & scratch) == 0)
	    goto gotscratch;	/* yes */
    }

    /* Failed to get scratch registers.  Return OR of all scratch
    ** registers that aren't currently available.
    */
    return( (rneeds[-1] & ~avail) | RS_FAIL );

    /* Come here with "sptr" pointing at current scratch set, "scratch"
    ** set to its contents.
    */
gotscratch:
    /* Remember scratch register vector:  offset of individual vectors */
    p->in.scratch = sptr + (nvec > 1 ? 1 : 0) - rsbits;
    return( scratch );			/* return success */
}
/* Get result registers, given node and rewrite.
**
** This routine examines the scratch registers associated with a node
** and determines which one(s) will be the result registers, given rewrite.
*/

static RST
getscrresult(p, tmplt, rewrite)
NODE * p;				/* node with p->in.scratch set */
OPTAB * tmplt;				/* template being matched */
int rewrite;
{
    int which = 1;			/* assume RESC1 */
    /* point at individual scratch bit vectors for node */
    RST * pscratch = RSB(p->in.scratch);
    RST tscratch;
    RST result;

    if (rewrite & RESC2) which = 2;
    else if (rewrite & RESC3) which = 3;

    for ( tscratch = *pscratch++; which > 0; --which ) {
	if (tscratch == 0)
	    tscratch = *pscratch++;	/* get next bit vector */
	result = RS_CHOOSE(tscratch);	/* choose scratch reg */
	tscratch -= result;
	if (tmplt->needs & NPAIR) {	/* remove adjacent bit */
	    tscratch -= RS_PAIR(result);/* assume it's there */
	    result |= RS_PAIR(result);	/* result is the pair */
	}
    }
    /* even if $P was not set, if the result needs more than
    ** one register, get a pair of registers.
    */
    if (result && szty(p->in.type) == 2 && !(tmplt->needs & NPAIR)) {
	if (!RS_PAIR(result) & tscratch)
	    cerror("second part of result unavailable!");
	result |= RS_PAIR(result);
    }
    return( result );
}
/* makesub -- make node a sub-tree */
static void 
makesub(p)
NODE *p;
{
	subtree[nsubtree] = p;
	subgoal[nsubtree] = NRGS;
	sub_rs_want[nsubtree] = RS_NRGS;
	++nsubtree;
	return;
}
/* produce register number corresponding to a register bit */

int
rs_rnum(regbit)
RST regbit;				/* bit vector:  should be one bit */
{
    int i;

    regbit = RS_CHOOSE(regbit);		/* allow only one register bit */

    for (i = 0; regbit > 1; ++i)
	regbit >>= 1;			/* shift until we shift off the end */
    
    return( i );
}
/* Produce register bit corresponding to highest numbered register
** in a bit-vector.
*/

#ifndef CG
RST
RS_RCHOOSE(regbit)
register RST regbit;
{
    register int regno;
    register RST mask;

    for (regno = TOTREGS - 1; regno >= 0; --regno) {
	if ((mask = RS_BIT(regno)) & regbit)
	    return( mask );
    }
    return( RS_NONE );			/* no register bits set */
}
#endif
