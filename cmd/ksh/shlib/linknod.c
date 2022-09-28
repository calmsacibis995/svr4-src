/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/linknod.c	1.2.3.1"
/*
 *   NAM_LINK (NODP, ROOT)
 *        
 *        Link the node given by NODP into the memory tree
 *        given by ROOT.
 *
 *   RMNVAL (NV)
 *
 *        Remove freeable space associated with the Nodval NV.
 *
 *
 *   See Also:  nam_search(III), gettree(III)
 */

#include	"name.h"

#ifndef KSHELL
void	rmnval ();
extern void	free();
#endif /* !KSHELL */


/*
 *   NAM_LINK (NODP, ROOT)
 *
 *        struct namnod *NODP;
 *
 *        struct Amemory *ROOT;
 *
 *   Link the namnod pointed to by NODP into the memory tree
 *   denoted by ROOT.  If ROOT contains another namnod with
 *   the same namid as NODP, an array is created, with the
 *   previously inserted namnod as its first element and NODP as
 *   its second.  If a previously inserted node of the same namid
 *   already denotes an array of n elements, NODP becomes the
 *   n+1st element.
 */

#ifdef KSHELL
/* save code space by not handling the case of linking array elements */
void	nam_link(nodp,root)
register struct namnod *nodp;
register struct Amemory *root;
{
	register int i = nam_hash(nodp->namid);
	i &= root->memsize;
	nodp->namnxt = root->memhead[i];
	root->memhead[i] = nodp;
}
#else
void	nam_link(nodp,root)
struct namnod *nodp;
struct Amemory *root;
{
	register struct namnod *np,*nq,**npp;
	struct Nodval *nv;
	struct Namaray *ap;
	int dot;
	char *cp = nodp->namid;
	int i = nam_hash(cp);

	i &= root->memsize;
	nodp->namnxt = NULL;
	for(npp= &root->memhead[i],np= *npp;np;npp= &np->namnxt,np= *npp)
		if(strcmp(cp,np->namid)==0)
		{
			if (!(nam_istype (np, N_ARRAY)))
			{
				nq = nam_alloc(cp);
				nq->namnxt = np->namnxt;
				*npp = nq;
				nq->value.namflg =  np->value.namflg|N_ARRAY;
				nq->value.namval.aray = ap = array_grow((struct Namaray *)NULL,0);
				ap->val[0] = &np->value;
				nq->value.namsz = np->value.namsz;
				np = nq;
			}
			ap = array_ptr(np);
			dot = ++ap->adot;
			if (dot >= ap->maxi)
				np->value.namval.aray = ap = array_grow(ap,dot);
			if (nv = ap->val[dot])
				if (freeble (nv))
					rmnval (unmark (nv));
			ap->val[dot] = &nodp->value;
			return;
		}
	*npp = nodp;
}


/*
 *   RMNVAL (NV)
 *
 *        struct Nodval *NV;
 *
 *   Remove freeable string space attached to NV, and then
 *   free the Nodval structure itself.
 *
 */

void	rmnval (nv)
struct Nodval *nv;
{
	register int flag = nv->namflg;
	register union Namval *up = &nv->namval;

	up->cp = NULL;
	if (!(flag & N_FREE))
	{
		if (flag & N_INDIRECT)
			up = up->up;
		if (up->cp != NULL)
			free (up->cp);
	}
	free ((char*)nv);
	return;
}
#endif	/* KSHELL */
