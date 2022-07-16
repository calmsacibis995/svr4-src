/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/tsearch.c	2.11"
/*LINTLIBRARY*/
/*
 * Tree search algorithm, generalized from Knuth (6.2.2) Algorithm T.
 *
 *
 * The NODE * arguments are declared in the lint files as char *,
 * because the definition of NODE isn't available to the user.
 */

#ifdef __STDC__
	#pragma weak tdelete = _tdelete
	#pragma weak tsearch = _tsearch
	#pragma weak twalk = _twalk
#endif
#include "synonyms.h"
#include <search.h>
#include <stdlib.h>
typedef char *POINTER;
typedef struct node { POINTER key; struct node *llink, *rlink; } NODE;

#define	NULL	0

VOID *
tsearch(ky, rtp, compar)	/* Find or insert key into search tree*/
const VOID *ky;			/* Key to be located */
VOID **rtp;			/* Address of the root of the tree */
int	(*compar)();		/* Comparison function */
{
	POINTER key = (char *)ky;
	register NODE **rootp = (NODE **)rtp;
	register NODE *q;	/* New node if key not found */

	if (rootp == NULL)
		return (NULL);
	while (*rootp != NULL) {			/* T1: */
		int r = (*compar)(key, (*rootp)->key);	/* T2: */
		if (r == 0)
			return ((VOID *)*rootp);	/* Key found */
		rootp = (r < 0) ?
		    &(*rootp)->llink :		/* T3: Take left branch */
		    &(*rootp)->rlink;		/* T4: Take right branch */
	}
	q = (NODE *) malloc(sizeof(NODE));	/* T5: Not found */
	if (q != NULL) {			/* Allocate new node */
		*rootp = q;			/* Link new node to old */
		q->key = key;			/* Initialize new node */
		q->llink = q->rlink = NULL;
	}
	return ((VOID *)q);
}

VOID *
tdelete(ky, rtp, compar)	/* Delete node with key key */
const VOID *ky;		/* Key to be deleted */
VOID **rtp;			/* Address of the root of tree */
int	(*compar)();		/* Comparison function */
{
        POINTER key = (char *) ky;
        register NODE **rootp = (NODE **)rtp;
	NODE *p;		/* Parent of node to be deleted */
	register NODE *q;	/* Successor node */
	register NODE *r;	/* Right son node */
	int ans;		/* Result of comparison */

	if (rootp == NULL || (p = *rootp) == NULL)
		return (NULL);
	while ((ans = (*compar)(key, (*rootp)->key)) != 0) {
		p = *rootp;
		rootp = (ans < 0) ?
		    &(*rootp)->llink :		/* Take left branch */
		    &(*rootp)->rlink;		/* Take right branch */
		if (*rootp == NULL)
			return (NULL);		/* Key not found */
	}
	r = (*rootp)->rlink;			/* D1: */
	if ((q = (*rootp)->llink) == NULL)	/* Llink NULL? */
		q = r;
	else if (r != NULL) {			/* Rlink NULL? */
		if (r->llink == NULL) {		/* D2: Find successor */
			r->llink = q;
			q = r;
		} else {			/* D3: Find NULL link */
			for (q = r->llink; q->llink != NULL; q = r->llink)
		 		r = q;
			r->llink = q->rlink;
			q->llink = (*rootp)->llink;
			q->rlink = (*rootp)->rlink;
		}
	}
	free((POINTER) *rootp);		/* D4: Free node */
	*rootp = q;			/* Link parent to replacement */
	return ((VOID *)p);
}

void
twalk(rt, action)		/* Walk the nodes of a tree */
VOID *rt;			/* Root of the tree to be walked */
void	(*action)();		/* Function to be called at each node */
{
	NODE *root = (NODE *)rt;
	static void  __twalk();

	if (root != NULL && action != NULL)
		__twalk(root, action, 0);
}

static void
__twalk(root, action, level)	/* Walk the nodes of a tree */
register NODE	*root;		/* Root of the tree to be walked */
register void	(*action)();	/* Function to be called at each node */
register int	level;
{
	if (root->llink == NULL && root->rlink == NULL)
		(*action)(root, leaf, level);
	else {
		(*action)(root, preorder, level);
		if (root->llink != NULL)
			__twalk(root->llink, action, level + 1);
		(*action)(root, postorder, level);
		if (root->rlink != NULL)
			__twalk(root->rlink, action, level + 1);
		(*action)(root, endorder, level);
	}
}

