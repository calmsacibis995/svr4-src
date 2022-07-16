/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

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
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)kern-fs:dnlc.c	1.3"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/cred.h"
#include "sys/dnlc.h"
#include "sys/kmem.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"

/*
 * Directory name lookup cache.
 * Based on code originally done by Robert Elz at Melbourne.
 *
 * Names found by directory scans are retained in a cache
 * for future reference.  It is managed LRU, so frequently
 * used names will hang around.  Cache is indexed by hash value
 * obtained from (vp, name) where the vp refers to the
 * directory containing the name.
 *
 * For simplicity (and economy of storage), names longer than
 * some (small) maximum length are not cached; they occur
 * infrequently in any case, and are almost never of interest.
 */

#define	NC_HASH_SIZE		8	/* size of hash table */

#define	NC_HASH(namep, namelen, vp)	\
	((namep[0] + namep[namelen-1] + namelen + (int) vp) & (NC_HASH_SIZE-1))

/*
 * Macros to insert, remove cache entries from hash, LRU lists.
 */
#define	INS_HASH(ncp, nch)	nc_inshash(ncp, nch)
#define	RM_HASH(ncp)		nc_rmhash(ncp)

#define	INS_LRU(ncp1, ncp2)	nc_inslru((struct ncache *) ncp1, \
				  (struct ncache *) ncp2)
#define	RM_LRU(ncp)		nc_rmlru((struct ncache *) ncp)

#define	NULL_HASH(ncp)		(ncp)->hash_next = (ncp)->hash_prev = (ncp)

/*
 * The name cache itself, dynamically allocated at startup.
 */
struct ncache *ncache;

/*
 * Hash list of name cache entries for fast lookup.
 */
struct nc_hash	{
	struct ncache *hash_next;
	struct ncache *hash_prev;
} nc_hash[NC_HASH_SIZE];

/*
 * LRU list of cache entries for aging.
 */
struct nc_lru {
	struct ncache *hash_next;	/* hash chain, unused */
	struct ncache *hash_prev;
	struct ncache *lru_next;	/* LRU chain */
	struct ncache *lru_prev;
} nc_lru;

struct ncstats ncstats;		/* cache effectiveness statistics */

STATIC int doingcache = 1;

#if defined(__STDC__)

STATIC void		dnlc_rm(struct ncache *);
STATIC struct ncache	*dnlc_search(vnode_t *, char *, int, int, cred_t *);

STATIC void		nc_inshash(struct ncache *, struct ncache *);
STATIC void		nc_rmhash(struct ncache *);
STATIC void		nc_inslru(struct ncache *, struct ncache *);
STATIC void		nc_rmlru(struct ncache *);

#else

STATIC void		dnlc_rm();
STATIC struct ncache	*dnlc_search();

STATIC void		nc_inshash();
STATIC void		nc_rmhash();
STATIC void		nc_inslru();
STATIC void		nc_rmlru();

#endif

/*
 * Initialize the directory cache.
 * Put all the entries on the LRU chain and clear out the hash links.
 */
void
dnlc_init()
{
	register struct ncache *ncp;
	register int i;

	if (ncsize <= 0
	  || (ncache =
	      (struct ncache *)kmem_zalloc(ncsize * sizeof(*ncache), KM_SLEEP))
	    == NULL) {
		doingcache = 0;
		cmn_err(CE_NOTE, "No memory for name cache\n");
		return;
	}
	nc_lru.lru_next = (struct ncache *) &nc_lru;
	nc_lru.lru_prev = (struct ncache *) &nc_lru;
	for (i = 0; i < ncsize; i++) {
		ncp = &ncache[i];
		INS_LRU(ncp, &nc_lru);
		NULL_HASH(ncp);
		ncp->dp = ncp->vp = NULL;
	}
	for (i = 0; i < NC_HASH_SIZE; i++) {
		ncp = (struct ncache *) &nc_hash[i];
		NULL_HASH(ncp);
	}
}

/*
 * Add a name to the directory cache.
 */
void
dnlc_enter(dp, name, vp, cred)
	register vnode_t *dp;
	register char *name;
	vnode_t *vp;
	cred_t *cred;
{
	register unsigned int namlen;
	register struct ncache *ncp;
	register int hash;

	if (!doingcache)
		return;
	if ((namlen = strlen(name)) > NC_NAMLEN) {
		ncstats.long_enter++;
		return;
	}
	hash = NC_HASH(name, namlen, dp);
	if (dnlc_search(dp, name, namlen, hash, cred) != NULL) {
		ncstats.dbl_enters++;
		return;
	}
	/*
	 * Take least recently used cache struct.
	 */
	ncp = nc_lru.lru_next;
	if (ncp == (struct ncache *) &nc_lru) {	/* LRU queue empty */
		ncstats.lru_empty++;
		return;
	}
	/*
	 * Remove from LRU, hash chains.
	 */
	RM_LRU(ncp);
	RM_HASH(ncp);
	/*
	 * Drop hold on vnodes (if we had any).
	 */
	if (ncp->dp != NULL)
		VN_RELE(ncp->dp);
	if (ncp->vp != NULL)
		VN_RELE(ncp->vp);
	if (ncp->cred != NULL)
		crfree(ncp->cred);
	/*
	 * Hold the vnodes we are entering and
	 * fill in cache info.
	 */
	ncp->dp = dp;
	VN_HOLD(dp);
	ncp->vp = vp;
	VN_HOLD(vp);
	ncp->namlen = namlen;
	bcopy(name, ncp->name, (unsigned)namlen);
	ncp->cred = cred;
	if (cred)
		crhold(cred);
	/*
	 * Insert in LRU, hash chains.
	 */
	INS_LRU(ncp, nc_lru.lru_prev);
	INS_HASH(ncp, (struct ncache *)&nc_hash[hash]);
	ncstats.enters++;
}

/*
 * Look up a name in the directory name cache.
 */
vnode_t *
dnlc_lookup(dp, name, cred)
	vnode_t *dp;
	register char *name;
	cred_t *cred;
{
	register int namlen;
	register int hash;
	register struct ncache *ncp;

	if (!doingcache)
		return NULL;
	if ((namlen = strlen(name)) > NC_NAMLEN) {
		ncstats.long_look++;
		return NULL;
	}
	hash = NC_HASH(name, namlen, dp);
	if ((ncp = dnlc_search(dp, name, namlen, hash, cred)) == NULL) {
		ncstats.misses++;
		return NULL;
	}
	ncstats.hits++;
	/*
	 * Move this slot to the end of LRU
	 * chain.
	 */
	RM_LRU(ncp);
	INS_LRU(ncp, nc_lru.lru_prev);
	/*
	 * If not at the head of the hash chain,
	 * move forward so will be found
	 * earlier if looked up again.
	 */
	if (ncp->hash_prev != (struct ncache *) &nc_hash[hash]) {
		RM_HASH(ncp);
		INS_HASH(ncp, ncp->hash_prev->hash_prev);
	}
	return ncp->vp;
}

/*
 * Remove an entry in the directory name cache.
 */
void
dnlc_remove(dp, name)
	vnode_t *dp;
	register char *name;
{
	register int namlen;
	register struct ncache *ncp;
	int hash;

	if (!doingcache)
		return;
	if ((namlen = strlen(name)) > NC_NAMLEN)
		return;
	hash = NC_HASH(name, namlen, dp);
	while (ncp = dnlc_search(dp, name, namlen, hash, ANYCRED))
		dnlc_rm(ncp);
}

/*
 * Purge the entire cache.
 */
void
dnlc_purge()
{
	register struct nc_hash *nch;
	register struct ncache *ncp;

	if (!doingcache)
		return;
	ncstats.purges++;
start:
	for (nch = nc_hash; nch < &nc_hash[NC_HASH_SIZE]; nch++) {
		ncp = nch->hash_next;
		if (ncp != (struct ncache *) nch) {
			if (ncp->dp == NULL || ncp->vp == NULL)
				cmn_err(CE_PANIC, "dnlc_purge: NULL vp");
			dnlc_rm(ncp);
			goto start;
		}
	}
}

/*
 * Purge any cache entries referencing a vnode.
 */
void
dnlc_purge_vp(vp)
	register vnode_t *vp;
{
	register int moretodo;
	register struct ncache *ncp;

	if (!doingcache)
		return;
	do {
		moretodo = 0;
		for (ncp = nc_lru.lru_next; ncp != (struct ncache *) &nc_lru;
		  ncp = ncp->lru_next) {
			if (ncp->dp == vp || ncp->vp == vp) {
				dnlc_rm(ncp);
				moretodo = 1;
				break;
			}
		}
	} while (moretodo);
}

/*
 * Purge cache entries referencing a vfsp.  Caller supplies a count
 * of entries to purge; up to that many will be freed.  A count of
 * zero indicates that all such entries should be purged.  Returns
 * the number of entries that were purged.
 */
int
dnlc_purge_vfsp(vfsp, count)
	register struct vfs *vfsp;
	register int count;
{
	register int moretodo;
	register struct ncache *ncp;
	register int n = 0;

	if (!doingcache)
		return 0;
	do {
		moretodo = 0;
		for (ncp = nc_lru.lru_next; ncp != (struct ncache *) &nc_lru;
		  ncp = ncp->lru_next) {
			if ((ncp->dp != NULL && ncp->dp->v_vfsp == vfsp)
			  || (ncp->vp != NULL && ncp->vp->v_vfsp == vfsp)) {
				n++;
				dnlc_rm(ncp);
				if (count == 0 || n < count)
					moretodo = 1;
				break;
			}
		}
	} while (moretodo);

	return n;
}

/*
 * Purge any cache entry.
 * Called by, e.g., iget() when inode freelist is empty.
 * Returns 1 if a cache entry was purged, 0 if the cache was
 * empty and there were none to purge.
 */
int
dnlc_purge1()
{
	register struct ncache *ncp;

	if (!doingcache)
		return 0;
	for (ncp = nc_lru.lru_next; ncp != (struct ncache *) &nc_lru;
	    ncp = ncp->lru_next) {
		if (ncp->dp) {
			dnlc_rm(ncp);
			return 1;
		}
	}
	cmn_err(CE_NOTE, "dnlc_purge1: no entries to purge\n");
	return 0;
}

/*
 * Obliterate a cache entry.
 */
STATIC void
dnlc_rm(ncp)
	register struct ncache *ncp;
{
	/*
	 * Remove from LRU, hash chains.
	 */
	RM_LRU(ncp);
	RM_HASH(ncp);
	/*
	 * Release ref on vnodes.
	 */
	VN_RELE(ncp->dp);
	ncp->dp = NULL;
	VN_RELE(ncp->vp);
	ncp->vp = NULL;
	if (ncp->cred != NOCRED) {
		crfree(ncp->cred);
		ncp->cred = NOCRED;
	}
	/*
	 * Insert at head of LRU list (first to grab).
	 */
	INS_LRU(ncp, &nc_lru);
	/*
	 * And make a dummy hash chain.
	 */
	NULL_HASH(ncp);
}

/*
 * Utility routine to search for a cache entry.
 */
STATIC struct ncache *
dnlc_search(dp, name, namlen, hash, cred)
	register vnode_t *dp;
	register char *name;
	register int namlen;
	int hash;
	cred_t *cred;
{
	register struct nc_hash *nhp;
	register struct ncache *ncp;

	nhp = &nc_hash[hash];
	for (ncp = nhp->hash_next; ncp != (struct ncache *) nhp;
	  ncp = ncp->hash_next) {
		if (ncp->dp == dp && ncp->namlen == namlen
		  && (cred == ANYCRED || ncp->cred == cred)
		  && *ncp->name == *name	/* fast chk 1st chr */
		  && bcmp(ncp->name, name, namlen) == 0)
			return ncp;
	}
	return NULL;
}

/*
 * Name cache hash list insertion and deletion routines.  These should
 * probably be recoded in assembly language for speed.
 */
STATIC void
nc_inshash(ncp, hp)
	register struct ncache *ncp;
	register struct ncache *hp;
{
	ncp->hash_next = hp->hash_next;
	ncp->hash_prev = hp;
	hp->hash_next->hash_prev = ncp;
	hp->hash_next = ncp;
}

STATIC void
nc_rmhash(ncp)
	register struct ncache *ncp;
{
	ncp->hash_prev->hash_next = ncp->hash_next;
	ncp->hash_next->hash_prev = ncp->hash_prev;
}

/*
 * Insert into LRU list.
 */
STATIC void
nc_inslru(ncp2, ncp1)
	register struct ncache *ncp2;
	register struct ncache *ncp1;
{
	register struct ncache *ncp3;

	ncp3 = ncp1->lru_next;
	ncp1->lru_next = ncp2;
	ncp2->lru_next = ncp3;
	ncp3->lru_prev = ncp2;
	ncp2->lru_prev = ncp1;
}

/*
 * Remove from LRU list.
 */
STATIC void
nc_rmlru(ncp)
	register struct ncache *ncp;
{
	ncp->lru_prev->lru_next = ncp->lru_next;
	ncp->lru_next->lru_prev = ncp->lru_prev;
}
