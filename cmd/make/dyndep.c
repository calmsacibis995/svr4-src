/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)make:dyndep.c	1.5"
/*	@(#)make:dyndep.c	1.4 of 3/11/88	*/

#include "defs"

/* NOTE: to enter "$@" on a dependency line in a makefile "$$@" must
 *	be typed. This is because `make' expands macros upon reading
 *	them.
 */

#define IS_DYN(a)	strchr( (a), DOLLAR )

/*
**	Declare local functions and make LINT happy.
*/

static LINEBLOCK	 runtime();

void
dyndep(p)		/* check each dependency */
register NAMEBLOCK p;
{
	register LINEBLOCK lp, nlp, backlp = NULL;

	p->rundep = 1;
	for (lp = p->linep; lp; lp = lp->nextline) {
		if ( nlp = runtime(lp) )
			if (backlp)
				backlp->nextline = nlp;
			else
				p->linep = nlp;

		backlp = !nlp ? lp : nlp;
	}
}


/* Runtime()	determines if a dependent line contains "$@" or
 *	"$(@F)" or "$(@D)". If so, it makes a new dependent line and 
 *	inserts it into the dependency chain of the input name, p.
 *	Here, "$@" gets translated to p->namep. That is	the current
 *	name on the left of the colon in the makefile.  Thus,
 *		xyz:	s.$@.c
 *	translates into
 *		xyz:	s.xyz.c
 *	Also, "$(@F)" translates to the same thing without a preceding
 *	directory path (if one exists).
 */
static LINEBLOCK
runtime(lp)
register LINEBLOCK lp;
{
	union {
		int	u_i;
		NAMEBLOCK u_nam;
	} temp;
	register DEPBLOCK q, nq;
	register NAMEBLOCK pc;
	register CHARSTAR pc1;
	LINEBLOCK nlp;
	CHARSTAR subst();
	char	buf[ARY128];

	temp.u_i = NO;
	for (q = lp->depp; q; q = q->nextdep)
		if ((pc = q->depname) && (IS_DYN(pc->namep))) {
			temp.u_i = YES;
			break;
		}

	if ( !temp.u_i )
		return(NULL);

	nlp = ALLOC(lineblock);
	nlp->nextline = lp->nextline;
	nlp->shp   = lp->shp;

	nlp->depp  = nq = ALLOC(depblock);

	for (q = lp->depp; q; q = q->nextdep) {
		pc1 = q->depname->namep;
		if (IS_DYN(pc1)) {
			(void)subst(pc1, buf);
			CHKARY(runtime, buf, ARY128)  /*macro defined in defs*/
			if ( !(temp.u_nam = SRCHNAME(buf)) )
				temp.u_nam = makename(copys(buf));
			nq->depname = temp.u_nam;
		} else
			nq->depname = q->depname;

		if ( !(q->nextdep) )
			nq->nextdep = NULL;
		else
			nq->nextdep = ALLOC(depblock);

		nq = nq->nextdep;
	}
	return(nlp);
}
