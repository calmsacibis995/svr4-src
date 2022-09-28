/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/hash.c	1.2.2.1"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */
	/*
	 * hash.c
	 * rotuines handle insertion, deletion of hashed monitor, file entries
	 */

#include "prot_lock.h"

#define MAX_HASHSIZE 100

extern int debug;

int HASH_SIZE;

typedef struct lm_vnode cache_fp;
typedef struct lm_vnode cache_me;

cache_fp *table_fp[MAX_HASHSIZE];
cache_me *table_me[MAX_HASHSIZE];

/*
 * find_me returns the cached entry;
 * it returns NULL if not found;
 */
struct lm_vnode *
find_me(svr)
	char *svr;
{
	cache_me *cp;

	if (debug)
		(void) printf("enter find_me()...\n");

	cp = table_me[hash(svr)];
	while ( cp != NULL) {
		if (strcmp(cp->svr, svr) == 0) {
			/* found */
			return (cp);
		}
		cp = cp->next;
	}
	return (NULL);
}

void
insert_me(mp)
	struct lm_vnode *mp;
{
	int h;

	if (debug)
		printf("enter insert_me()...\n");

	h = hash(mp->svr);
	mp->next = table_me[h];
	table_me[h] = mp;
}
