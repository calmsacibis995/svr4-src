/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cg:i386/picode.c	1.10"
/*
 *	picode.c - rewrite the tree for NAME, ICON, and FCON
*/

#include "mfile2.h"

static NODE *picode(), *build_in(), *build_tn();

#ifdef NODBG
#define DEBUG(msg, node)
#else
#define DEBUG(msg, node)	\
		if (odebug) 	\
			(void) fprintf(outfile, msg), \
			(void) e2print(node)
	
#endif

void
myreader(p) NODE *p;
{
	if (picflag) {
		NODE *q;
		DEBUG("\n\n--- BEFORE picode()\n", p);

                q = picode(p);

		DEBUG("\n\n+++ AFTER picode()\n", q);
                if (p->in.op == FREE) {
                	*p = *q;
			q->in.op = FREE;
		}
        }
}

/* Rewrite the tree for NAME, ICON, and FCON nodes. 
 * This is a common version of tree-rewrite routine for PIC.
 * If there is any machine-dependent special requirements
 * for tree-rewrite,  a local version of tree-rewrite routine 
 * for PIC can be defined for that instance.
*/
static NODE*
picode(p) NODE *p;
{
	register ty, o;
	NODE *q, *r;

	if((o = p->in.op) == INIT ) return p;
	ty = optype( o );

	switch (ty) {
	case BITYPE:
		p->in.right = picode(p->in.right);
		if ((o == CALL || o == STCALL) && p->in.left->in.op == ICON ) 
		{
			if ( p->in.left->tn.name != ((char *)0) )	
			     p->in.left->tn.strat |= PIC_PLT;
			return p;
		}
#ifdef	IN_LINE
		else if ( o == INCALL && p->in.left->in.op == ICON )
		{
			if ( p->in.left->tn.name != ((char *)0) )	
			     p->in.left->tn.strat |= PIC_PLT;
			return p;
		}
#endif
		else if (	(o == PLUS)
			     && (optype(p->in.right->in.op) == BITYPE)
			     && (p->in.right->in.right->in.op == ICON)
		             && (p->in.right->in.right->in.name != (char *) 0) 
			)
		{
			/*
			** Optimize static array address operations.
			*/
			NODE *q;
			int stat = p->in.right->in.right->in.rval & (NI_FLSTAT|NI_BKSTAT);
			p->in.left = picode(p->in.left);
			if (stat) {
				q = p->in.right;
				p->in.right = p->in.right->in.right;
				q->in.right = p->in.left;
				p->in.left = q;
			}
			DEBUG("static array address optimizization", p);
			return p;
		}
		else
			p->in.left = picode(p->in.left);
		return p;
	case UTYPE:
		if (o == STAR && p->in.left->in.op == PLUS) {
			NODE * q = p->in.left->in.right;
			NODE *t;

			/* when it is an array, build the tree for the offset */
			if (q->in.op == ICON && q->in.name != (char *)0) {
			    int stat = q->in.rval & (NI_BKSTAT|NI_FLSTAT);
			    if (q->in.lval == 0 || stat) {
				p->in.left->in.right = picode(p->in.left->in.left);
				p->in.left->in.left = picode(q);
			    }
			    else {
				NODE * l;
				l = build_in(PLUS, p->in.type,
					build_tn(ICON, q->in.type, q->in.name, 0, q->tn.rval),
					p->in.left->in.left);
				q->in.name = (char *)0;
				q->in.type = TINT;
				p->in.left->in.left = l;
				p->in.left->in.left->in.left = picode(p->in.left->in.left->in.left);
				p->in.left->in.left->in.right = picode(p->in.left->in.left->in.right);
			    }
			    if (stat) {
				/* Rewrite for static array reference.
				** Name ICON must be bubbled-up.  Final
				** tree looks like:
				**
				**		STAR
				**		 |
				**		 +
				**		/ \
				**	       +   ICON(+/- offset)
				**	      / \
				**          REG expr
				*/
				q = p->in.left;
				t = q->in.left->in.right;
				q->in.left->in.right = q->in.right;
				q->in.right = t;
				q = p->in.left;
			    }
			    return p;
			}   
		}

		if ((o == UNARY CALL || o == UNARY STCALL) && p->in.left->in.op == ICON)
		{
			if ( p->in.left->tn.name != ((char *)0) )	
			     p->in.left->tn.strat |= PIC_PLT;
			return p;
		}
#ifdef	IN_LINE
		else if ( o == UNARY INCALL && p->in.left->in.op == ICON )
		{
			if ( p->in.left->tn.name != ((char *)0) )	
			     p->in.left->tn.strat |= PIC_PLT;
			return p;
		}
#endif
		else
			p->in.left = picode(p->in.left);
		return p;
	case LTYPE:
		switch (o) {
			int stat, array_ref;
#ifndef SETDCON
		case FCON:
			fcons(p);		/* in nail.c */
			return picode(p);
#endif
		case NAME:
		case ICON:
			if (p->tn.name == ((char *)0)) return p;

			stat = p->tn.rval & (NI_FLSTAT|NI_BKSTAT);
			array_ref = p->tn.lval != 0;

			q = build_tn(
				ICON, 
				TINT, 
				p->tn.name, 
				stat ? p->tn.lval : 0, 
				p->tn.rval);
			q->tn.strat |= PIC_GOT;
			r = build_in(
				PLUS,
				TPOINT,
				build_tn(REG, TPOINT, (char *)0, 0, BASEREG),
				q);

			if (!stat)  {
				r = build_in(STAR, TPOINT, r, NIL);

				/* Build the tree for the offset.  This
				** must be done because exprs of the form
				** id@GOT+3 are not legal.  For a static
				** reference, we allow id@GOTOFF+3.
				*/
				if (array_ref) {
				    q = build_tn(ICON, TINT,(char *)0,p->tn.lval,0);
				    r = build_in(PLUS, TPOINT, r, q);
				}
			}
			if (o == NAME) 
				r = build_in(STAR, p->in.type, r, NIL);
			else
				r->in.type = p->in.type;
			if (p->tn.strat & VOLATILE)
				r->in.strat |= VOLATILE;
			p->in.op = FREE;
			return r;
		}
		return p;
	}
	/* NOTREACHED */
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 * build an interior tree node
 */
static NODE *
build_in( op, type, left, right )
        int op;         /* operator     */
        TWORD type;     /* result type  */
        NODE * left;    /* left subtree */
        NODE * right;   /* right subtree*/
{
        register NODE *q = talloc();
        q->in.op = op;
        q->in.type = type;
        q->in.name = (char *)0;
        q->in.left = left;
        q->in.right = right;
        return q;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 * build a terminal tree node
 */
static NODE *
build_tn( op, type, name, lval, rval )
        int op;         /* operator     */
        TWORD type;     /* result type  */
        char * name;    /* symbolic part of constant */
        CONSZ  lval;    /* left part of value */
        int    rval;    /* right part of value*/
{
        register NODE *q = talloc();
        q->tn.op = op;
        q->tn.type = type;
        q->tn.name = name;
        q->tn.lval = lval;
        q->tn.rval = rval;
        return q;
}
