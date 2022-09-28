/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.named/db_lookup.c	1.1.3.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/*
 * Table lookup routines.
 */

#include <sys/types.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include "db.h"

struct hashbuf *hashtab;	/* root hash table */
struct hashbuf *fcachetab;	/* hash table of cache read from file */

#ifdef DEBUG
extern int debug;
extern FILE *ddt;
#endif

/* 
 * Lookup 'name' and return a pointer to the namebuf;
 * NULL otherwise. If 'insert', insert name into tables.
 * Wildcard lookups are handled.
 */
struct namebuf *
nlookup(name, htpp, fname, insert)
	char *name;
	struct hashbuf **htpp;
	char **fname;
	int insert;
{
	register struct namebuf *np;
	register char *cp;
	register int c;
	register unsigned hval;
	register struct hashbuf *htp;
	struct namebuf *parent = NULL;

	htp = *htpp;
	hval = 0;
	*fname = "???";
	for (cp = name; c = *cp++; ) {
		if (c == '.') {
			parent = np = nlookup(cp, htpp, fname, insert);
			if (np == NULL)
				return (NULL);
			if (*fname != cp)
				return (np);
			if ((htp = np->n_hash) == NULL) {
				if (!insert) {
					if (np->n_dname[0] == '*' && 
					    np->n_dname[1] == '\0')
						*fname = name;
					return (np);
				}
				htp = savehash((struct hashbuf *)NULL);
				np->n_hash = htp;
			}
			*htpp = htp;
			break;
		}
		hval <<= HASHSHIFT;
		hval += c & HASHMASK;
	}
	c = *--cp;
	*cp = '\0';
	/*
	 * Lookup this label in current hash table.
	 */
	for (np = htp->h_tab[hval % htp->h_size]; np != NULL; np = np->n_next) {

#ifdef ALLOW_UPDATES
		/* Note: at this point, if np->n_data is NULL, we could be in
		   one of two situations: Either we have come across a name
		   for which all the RRs have been (dynamically) deleted, or
		   else we have come across a name which has no RRs
		   associated with it because it is just a place holder
		   (e.g., EDU).  In the former case, we would like to delete
		   the namebuf, since it is no longer of use, but in the
		   latter case we need to hold on to it, so future lookups
		   that depend on it don't fail.  The only way I can see of
		   doing this is to always leave the namebufs around
		   (although then the memory usage continues to grow whenever
		   names are added, and can never shrink back down completely
		   when all their associated RRs are deleted). */
#endif ALLOW_UPDATES

		if (np->n_hashval == hval &&
		    strcasecmp(name, np->n_dname) == 0) {
			*cp = c;
			*fname = name;
			return (np);
		}
	}
	if (!insert) {
		/*
		 * look for wildcard in this hash table
		 */
		hval = ('*' & HASHMASK)  % htp->h_size;
		for (np = htp->h_tab[hval]; np != NULL; np = np->n_next) {
			if (np->n_dname[0] == '*'  && np->n_dname[1] == '\0') {
				*cp = c;
				*fname = name;
				return (np);
			}
		}
		*cp = c;
		return (parent);
	}
	np = savename(name);
	np->n_parent = parent;
	np->n_hashval = hval;
	hval %= htp->h_size;
	np->n_next = htp->h_tab[hval];
	htp->h_tab[hval] = np;
	/* increase hash table size */
	if (++htp->h_cnt > htp->h_size * 2) {
		*htpp = savehash(htp);
		if (parent == NULL) {
			if (htp == hashtab)
			    hashtab = *htpp;
			else
			    fcachetab = *htpp;
		}
		else
			parent->n_hash = *htpp;
		htp = *htpp;
	}
	*cp = c;
	*fname = name;
	return (np);
}

/*
 * Does the data record match the class and type?
 */
match(dp, class, type)
	register struct databuf *dp;
	register int class, type;
{
#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"match(0x%x, %d, %d) %d, %d\n", dp, class, type,
			dp->d_class, dp->d_type);
#endif
	if (dp->d_class != class && class != C_ANY)
		return (0);
	if (dp->d_type != type && type != T_ANY)
		return (0);
	return (1);
}
