/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/w1opt.c	1.1.2.3"
/* w1opt.c
**
**	Intel 386 optimizer:  for one-instruction window
**
*/

/* #include "defs" -- optim.h takes care of this */
#include "optim.h"
#include "optutil.h"

/*	D A N G E R
**
** This definition selects the highest numbered register that we
** can arbitrarily choose as a temporary in the multiply strength
** reduction that is performed below.  It should equal the highest
** numbered temporary register used by the compiler (1 less than
** lowest numbered register used for register variables.
*/

/* w1opt -- one-instruction optimizer
**
** This routine handles the single-instruction optimization window.
** See individual comments below about what's going on.
** In some cases (which are noted), the optimizations are ordered.
*/

boolean					/* true if we make any changes */
w1opt(pf,pl)
register NODE * pf;			/* pointer to first instruction in
					** window (and last)
					*/
NODE * pl;				/* pointer to last instruction in
					** window (= pf)
					*/
{

    register int cop = pf->op;		/* single instruction's op code # */
    boolean retval = false;		/* return value:  in some cases
					** we fall through after completing
					** an optimization because it could
					** lead into others.  This variable
					** contains the return state for the
					** end.
					*/
/* eliminate dead code:
**
**	op ...
**   where the destination operand and all registers set are dead
*/

    if (
	    isdead(dst(pf),pf)		/* are the destination and */
	&&  !(sets(pf) & pf->nlive)	/* all regs set dead? */
	&&  ! isbr(pf)			/* some branches set variables
					** and jump:  keep them */
	&&  (pf->op1 == NULL || !isvolatile( pf,1 )) /* are operands non- */
	&&  (pf->op2 == NULL || !isvolatile( pf,2 )) /* volatile? */
	&&  (pf->op3 == NULL || !isvolatile( pf,3 ))
	&&  sets(pf) != 0		/* leave instructions that don't
					** set any regs alone */
	&&  !(sets(pf) & (FP0 | FP1 | FP2 | FP3 | FP4 | FP5 | FP6 |FP7))
					/* don't mess with fp instr */
	)
    {
	wchange();			/* Note we're changing the window */
	ldelin2(pf);			/* preserve line number info */
	mvlivecc(pf);			/* preserve condition codes line info */
	DELNODE(pf);			/* discard instruction */
	return(true);			/* announce success */
    }

/*
**	cmp[bwl] $0,R	->	test[bwl] R,R
*/


    if (
	    (
	    cop == CMPL || cop == CMPW || cop == CMPB
	    )
	&&  strcmp(pf->op1,"$0") == 0
	&&  isreg(pf->op2)
	)

    {
	wchange();			/* note change */
	switch ( cop ) {		/* change the op code */
	    case CMPL:
		chgop(pf,TESTL,"testl"); break;
	    case CMPW:
		chgop(pf,TESTW,"testw"); break;
	    case CMPB:
		chgop(pf,TESTB,"testb"); break;
	}
	pf->op1 = pf->op2;		/* both operands point at R */
	makelive(pf->op2,pf);		/* make register appear to be live
	pf->nlive |= CONCODES;		** hereafter so "compare" isn't thrown
					** away.  (Otherwise R might be dead
					** and we would eliminate the inst.)
					*/
	return(true);			/* made a change */
    }

/*
**	addl $1,O1	->	incl O1
**
**	Also for dec[bwl] and inc[bw]
*/


    if (
	    (
	    cop == ADDL || cop == ADDW || cop == ADDB ||
	    cop == SUBL || cop == SUBW || cop == SUBB
	    )
	&&  strcmp(pf->op1,"$1") == 0
	)

    {
	wchange();			/* note change */
	switch ( cop ) {		/* change the op code */
	    case ADDL:
		chgop(pf,INCL,"incl"); break;
	    case ADDW:
		chgop(pf,INCW,"incw"); break;
	    case ADDB:
		chgop(pf,INCB,"incb"); break;
	    case SUBL:
		chgop(pf,DECL,"decl"); break;
	    case SUBW:
		chgop(pf,DECW,"decw"); break;
	    case SUBB:
		chgop(pf,DECB,"decb"); break;
	}
	pf->op1 = pf->op2;		/* both operands point at R */
	pf->op2 = NULL;
	makelive(pf->op1,pf);		/* make register appear to be live
					** hereafter so "compare" isn't thrown
					** away.  (Otherwise R might be dead
					** and we would eliminate the inst.)
					*/
	return(true);			/* made a change */
    }
/*
**	mov[bwl] $0,R1	->	xor[bwl] R1,R1
**
**	This is the same speed, but more compact.
*/


    if (
	    (
	    cop == MOVL || cop == MOVW || cop == MOVB
	    )
	&&  strcmp(pf->op1,"$0") == 0
	&&  isreg(pf->op2)
	)

    {
	wchange();			/* note change */
	switch ( cop ) {		/* change the op code */
	    case MOVL:
		chgop(pf,XORL,"xorl"); break;
	    case MOVW:
		chgop(pf,XORW,"xorw"); break;
	    case MOVB:
		chgop(pf,XORB,"xorb"); break;
	}
	pf->op1 = pf->op2;		/* both operands point at R */
	makelive(pf->op1,pf);		/* make register appear to be live
					** hereafter so "compare" isn't thrown
					** away.  (Otherwise R might be dead
					** and we would eliminate the inst.)
					*/
	return(true);			/* made a change */
    }

/* get rid of useless arithmetic
**
**	addl	$0,O		->	deleted  or  cmpl O,$0
**	subl	$0,O		->	deleted  or  cmpl O,$0
**	orl	$0,O		->	deleted  or  cmpl O,$0
**	xorl	$0,O		->	deleted  or  cmpl O,$0
**	sall	$0,O		->	deleted  or  cmpl O,$0
**	sarl	$0,O		->	deleted  or  cmpl O,$0
**	shll	$0,O		->	deleted  or  cmpl O,$0
**	shrl	$0,O		->	deleted  or  cmpl O,$0
**	mull	$1,O		->	deleted  or  cmpl O,$0
**	imull	$1,O		->	deleted  or  cmpl O,$0
**	divl	$1,O		->	deleted  or  cmpl O,$0
**	idivl	$1,O		->	deleted  or  cmpl O,$0
**	andl	$-1,O		->	deleted  or  cmpl O,$0

**	mull	$0,O		->	movl $0,O
**	imull	$0,O		->	movl $0,O
**	andl	$0,O		->	movl $0,O
**
** Note that since we've already gotten rid of dead code, we won't
** check whether O (O2) is live.  However, we must be careful to
** preserve the sense of result indicators if a conditional branch
** follows some of these changes.
*/

/* Define types of changes we will make.... */

#define	UA_NOP		1		/* no change */
#define UA_DELL		2		/* delete instruction */
#define UA_DELW		3		/* delete instruction */
#define UA_DELB		4		/* delete instruction */
#define UA_MOVZL	5		/* change to move zero to ... */
#define UA_MOVZW	6		/* change to move zero to ... */
#define UA_MOVZB	7		/* change to move zero to ... */
/* We must have a literal as the first operand. */
    if ( isnumlit(pf->op1) )
    {
	int ultype = UA_NOP;		/* initial type of change = none */

	switch(atol(pf->op1+1))			/* branch on literal */
	{
	case 0:				/* handle all instructions with &0
					** as first operand
					*/
	    switch (cop)
	    {
	    case ADDL:
	    case SUBL:
	    case ORL:
	    case XORL:
	    case SALL:
	    case SHLL:
	    case SARL:
	    case SHRL:
		ultype = UA_DELL;
		break;
	    
	    case ADDW:
	    case SUBW:
	    case ORW:
	    case XORW:
	    case SALW:
	    case SHLW:
	    case SARW:
	    case SHRW:
		ultype = UA_DELW;
		break;
	    
	    case ADDB:
	    case SUBB:
	    case ORB:
	    case XORB:
	    case SALB:
	    case SHLB:
	    case SARB:
	    case SHRB:
		ultype = UA_DELB;
		break;
	    
	    case MULL:
	    case IMULL:
	    case ANDL:
		ultype = UA_MOVZL;	/* convert to move zero */
		break;

	    case MULW:
	    case IMULW:
	    case ANDW:
		ultype = UA_MOVZW;	/* convert to move zero */
		break;
	    
	    case MULB:
	    case IMULB:
	    case ANDB:
		ultype = UA_MOVZB;	/* convert to move zero */
		break;
	    }
	    break;			/* done $0 case */

	case 1:				/* &1 case */
	    switch( cop )		/* branch on op code */
	    {
	    case DIVL:
	    case IDIVL:
	    case MULL:
	    case IMULL:
		ultype = UA_DELL;	
		break;
	    
	    case DIVW:
	    case IDIVW:
	    case MULW:
	    case IMULW:
		ultype = UA_DELW;	
		break;
	    
	    case DIVB:
	    case IDIVB:
	    case MULB:
	    case IMULB:
		ultype = UA_DELB;	
		break;
	    }
	    break;			/* done $1 case */
	
	case -1:			/* $-1 case */
	    switch ( cop )		/* branch on op code */
	    {
	    case ANDL:
		ultype = UA_DELL;	
		break;
	    
	    case ANDW:
		ultype = UA_DELW;	
		break;
	    
	    case ANDB:
		ultype = UA_DELB;	
		break;
	    }
	    break;			/* end $-1 case */
	} /* end switch on immediate value */
/* Now do something, based on selections made above */

	switch ( ultype )
	{
	case UA_MOVZL:			/* change to move zero to operand */
	case UA_MOVZW:			/* change to move zero to operand */
	case UA_MOVZB:			/* change to move zero to operand */
	    if (isvolatile(pf,2))	/* if dest is volatile, don't */
	      	break;			/* touch this. */
	    wchange();
	    pf->op1 = "$0";		/* first operand is zero */
	    pf->op2 = dst(pf);		/* second is ultimate destination */
	    pf->op3 = NULL;		/* clean out if there was one */
	    switch ( ultype ) {
		case UA_MOVZL:
		    chgop(pf,MOVL,"movl");	/* change op code */
		    break;
		case UA_MOVZW:
		    chgop(pf,MOVW,"movw");	/* change op code */
		    break;
		case UA_MOVZB:
		    chgop(pf,MOVB,"movb");	/* change op code */
		    break;
	    }
	    retval = true;		/* made a change */
	    break;
	
/* For this case we must be careful:  if a following instruction is a
** conditional branch, it is clearly depending on the result of the
** arithmetic, so we must put in a compare against zero instead of deleting
** the instruction.
*/

	case UA_DELL:			/* delete instruction */
	case UA_DELW:			/* delete instruction */
	case UA_DELB:			/* delete instruction */

	    if (isvolatile(pf,2))	/* if dest is volatile, don't */
	      	break;			/* touch this. */
	    wchange();			/* we will make a change */

	    if ( ! isdeadcc(pf) )
	    {
		switch ( ultype ) {
		    case UA_DELL:
			chgop(pf,TESTL,"testl");
			break;
		    case UA_DELW:
			chgop(pf,TESTW,"testw");
			break;
		    case UA_DELB:
			chgop(pf,TESTB,"testb");
			break;
		}
		pf->op1 = pf->op2;	/* always test second operand */
		pf->op3 = NULL;		/* for completeness */
		retval = true;		/* made a change */
	    }
	    else
	    {
		ldelin2(pf);		/* preserve line number info */
		mvlivecc(pf);		/* preserve cond. codes line info */
		DELNODE(pf);		/* not conditional; delete node */
		return(true);		/* say we changed something */
	    }
	    break;
	} /* end case that decides what to do */
	
	cop = pf->op;			/* reset current op for changed inst. */

    } /* end useless arithmetic removal */
/* discard useless mov's
**
**	movw	O,O		->	deleted
** Don't worry about condition codes, because mov's don't change
** them anyways.
*/

    if ( (  pf->op == MOVB
	 || pf->op == MOVW
	 || pf->op == MOVL
	 )
	&&  strcmp(pf->op1,pf->op2) == 0
	&&  !isvolatile(pf,1)		/* non-volatile */
	)
    {
	wchange();			/* changing the window */
	ldelin2(pf);			/* preserve line number info */
	mvlivecc(pf);			/* preserve condition codes line info */
	DELNODE(pf);			/* delete the movw */
	return(true);
    }


/* For Intel 386, a shift by one bit is more efficiently
** done as an add.
**
**	shll $1,R		->	addl R,R
**
*/

    {
	if( ( pf->op == SHLL ||
	      pf->op == SHLW ||
	      pf->op == SHLB )
	    && strcmp( pf->op1, "$1" ) == 0 
	    && isreg(pf->op2)		/* safe from mmio */
	    ) {
		if( pf->op == SHLL ) {
			chgop( pf, ADDL, "addl" );
			pf->op1 = pf->op2;
			return( true );
		}
		if( pf->op == SHLW ) {
			chgop( pf, ADDW, "addw" );
			pf->op1 = pf->op2;
			return( true );
		}
		if( pf->op == SHLB ) {
			chgop( pf, ADDB, "addb" );
			pf->op1 = pf->op2;
			return( true );
		}
	}
    }

    return(retval);			/* indicate whether anything changed */
}
