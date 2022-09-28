/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)iconv:sort.c	1.1.1.1"

#include <stdio.h>
#include "symtab.h"
#include "kbd.h"

extern int numnode;
extern struct kbd_map maplist[];
extern int curmap;

struct node *findprev();
struct node *nalloc();

/*
 * Bubble sort:  Inefficient, but sufficient & easy.  We're sorting
 * a singly linked list, not an array or anything...
 * This is used to sort the root of the "internal" tree, not the cornodes.
 * Called before we number the nodes!  May return a NEW ROOT!
 */

struct node *
sortroot(p)

	struct node *p;	/* the root */
{
	register int chg;
	register struct node *q, *t;

	if (! p)		/* sorting a null list is easy */
		return(p);
	/*
	 * IF we later want to expand this to sort all levels, then
	 * we need to check and NOT call "exproot" if p == root,
	 * where "root" is an extern node *.  We'd also have to
	 * reset "p" when we recur in that situation.  Note that we
	 * should sort bottom-to-top.
	 */
	exproot(p);	/* first, expand the root to fill gaps */
	chg = 1;	/* count number of changes */
	while (chg) {
		chg = 0;
		q = p;
		while (q->n_next) {
			if (q->n_val > q->n_next->n_val) {
				if (q != p) {
/* fprintf(stderr, "  swap other (%d, %d)\n", q->n_val, q->n_next->n_val); */
					t = findprev(p, q);
					t->n_next = q->n_next;
					q->n_next = q->n_next->n_next;
					t->n_next->n_next = q;
					q = t->n_next; /* after old "q" */
				}
				else { /* move the root */
/* fprintf(stderr, "  swap root (%d, %d)\n", q->n_val, q->n_next->n_val); */
					t = p->n_next;
					p->n_next = t->n_next;
					t->n_next = p;
					p = t;
					q = p->n_next;
				}
				++chg;
			}
			q = q->n_next;
		}
	}
	return(p);
}

#if 0
/*
 * Actually, didn't need to use this, but it's left here if someone
 * needs it sometime.
 */
nodecopy(to, from)

{
	to->n_val = from->n_val;
	to->n_flag = from->n_flag;
	to->n_num = from->n_num;
	to->n_next = from->n_next;
	to->n_what.n_child = from->n_what.n_child;
	to->n_node = from->n_node;
}
#endif

/*
 * Given the root of a level, and a "who" to find, find the
 * predecessor.  We need to do this whenever we're going to
 * swap two nodes in the linked list.  It's icky, but this is
 * amateur hour anyway, eh?
 */

struct node *
findprev(p, who)

	struct node *p, *who;
{
	while (p) {
		if (p->n_next == who)
			return(p);
		p = p->n_next;
	}
	return((struct node *) 0);
}

/*
 * Expand the root level of a tree to fill in gaps.  We work by
 * first figuring out what we have, then filling in what we
 * DON'T have by "nalloc()" and appending onto the root.  At most
 * we'll end up with 256 nodes, at least we'll have everything
 * filled between the min & max values (which we set in the
 * current map).
 */

exproot(p)
	register struct node *p;
{
	register struct node *q;	/* temp var */
	register struct node *tail;	/* tail of the list */
	register int i;
	unsigned char tab[256];
	register int imin, imax;

	imax = 0; imin = 256;
	q = p;
	for (i = 0; i < 256; i++)
		tab[i] = 0;
	/*
	 * Find tail and min/max values.
	 */
	while (q) {
		if ((int) q->n_val > imax)
			imax = q->n_val;
		if ((int) q->n_val < imin)
			imin = q->n_val;
		++(tab[q->n_val]);
		tail = q;
		q = q->n_next;
	}
	/* fprintf(stderr, "imin: %d, imax:%d\n", imin, imax); */
	maplist[curmap].map_min = imin;	/* our import/export agreement */
	maplist[curmap].map_max = imax;
	/*
	 * Now, "tail" points at the last node; keep it there.
	 */
	for (i = imin; i <= imax; i++) {
		if (! tab[i]) {	/* no entry for this */
			q = nalloc();
			q->n_flag = N_EMPTY;
			q->n_val = i;
			q->n_next = 0;
			tail->n_next = q;
			tail = q;
		}
	}
}
