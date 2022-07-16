/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/tfind.c	1.10"
/*LINTLIBRARY*/
/*
 * Tree search algorithm, generalized from Knuth (6.2.2) Algorithm T.
 *
 * The NODE * arguments are declared in the lint files as char *,
 * because the definition of NODE isn't available to the user.
 */

#ifdef __STDC__
	#pragma weak tfind = _tfind
#endif
#include "synonyms.h"
#include <search.h>
typedef char *POINTER;
typedef struct node { POINTER key; struct node *llink, *rlink; } NODE;

#define	NULL	0


/*	tfind - find a node, or return 0	*/
VOID *
tfind(ky, rtp, compar)
const VOID *ky;	/* Key to be located */
VOID *const *rtp;		/* Address of the root of the tree */
int	(*compar)();	/* Comparison function */
{
	POINTER key = (char *)ky;
	register NODE **rootp = (NODE **)rtp;
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
	return (VOID *)(NULL);
}
