/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/findnod.c	1.2.3.1"
/*
 *   NAM_SEARCH (NAME, ROOT, TYPE)
 *
 *        Return a pointer to the namnod in the list
 *        of namnods given by ROOT whose namid is NAME.  If 
 *        TYPE&N_ADD is set, create a new node with namid
 *        NAME.  If TYPE&N_NONULL is set and the node is found,
 *	  but is empty an NULL value is returned.  If TYPE&N_NOSCOPE
 *	  is set, the search will be confined to the top scope.
 *
 *   NAM_ALLOC (NAME)
 *
 *        Allocate a namnod, setting its namid to NAME and its
 *        value to VALUE to NULL.
 *
 *   NAM_COPY(NODE, TYPE)
 *
 *	  Return a pointer to a namnod in the last Shell tree
 *	  whose name is the same as that in NODE.
 *	  If TYPE is non-zero the attributes of NODE
 *	  are also copied.
 *
 *   See Also:  nam_link(III), nam_hash(III)
 */

#include	"sh_config.h"
#include	"name.h"
#ifdef KSHELL
#   include       "shtype.h"
#else
#   include       <ctype.h>
#endif	/* KSHELL */

extern char	*strcpy();

/*
 *   NAM_SEARCH (NAME, ROOT, TYPE)
 *
 *        char *NAME;
 *
 *        struct Amemory *ROOT;
 *
 *        int TYPE;
 *
 *   Return a pointer to the namnod in a linked list of
 *   namnods (given by ROOT) whose namid is NAME.  If TYPE
 *   is non-zero, a new namnod with the given NAME will
 *   be inserted, if none is found.
 *
 */

struct namnod *nam_search(name,root,type)
const char *name;
struct Amemory *root;
{
	register struct namnod *np;
	register struct Amemory *ap = root;
	register const char *cp,*sp;
	register struct namnod *nq;
	register int i;
	int hash;

	hash = nam_hash(name);
	while(1)
	{
		i = (hash&ap->memsize);
		for(nq=0,np=ap->memhead[i]; np; nq=np,np=np->namnxt)
		{
#ifdef NAME_SCOPE
			if(!(np->value.namflg&N_AVAIL))
#endif /* NAME_SCOPE */
			{
				/* match even if np->name has an = in it */
				cp = np->namid;
				sp = name;
				do
				{
					if(*sp==0 || *sp=='=')
					{
						if(*cp)
							break;
						if((type&N_NULL) && isnull(np))
							return((struct namnod*)0);
						if(nq==0)
							return(np);
						nq->namnxt = np->namnxt;
						goto found;
					}
				}
				while(*sp++ == *cp++);
			}
		}
		if((type&N_NOSCOPE) || ap->nexttree==0)
			break;
		ap = ap->nexttree;
	}
	if(!(type&N_ADD))
		return((struct namnod*)0);
	np = nam_alloc(name);
found:
	np->namnxt = ap->memhead[i];
	ap->memhead[i] = np;
	return(np);
}

/*
 *   NAM_ALLOC (NAME)
 *
 *        char *NAME;
 *
 *   Allocate a namnod, setting its namid to NAME and its value
 *   to VALUE to NULL.  A pointer to the allocated node is returned.
 *   NULL is returned if there is no space to be allocated
 *   for the namnod.
 *
 */

struct namnod *nam_alloc(name)
const char *name;
{
	register struct namnod *np;
	if((np=new_of(struct namnod,strlen(name)+1)) == (struct namnod*)NULL)
		return(np);
	np->namid = (char *)(np+1);
	strcpy (np->namid, name);
	np->value.namflg = N_DEFAULT;
	np->value.namval.cp = NULL;
	np->namnxt = NULL;
	np->value.namsz = 0;
	np->value.namenv = 0;
	return(np);
}

#ifdef NAME_SCOPE
extern struct Amemory *var_tree;
struct namnod *nam_copy(node, type)
struct namnod *node;
int type;
{
	register struct namnod *oldnp = node;
	register struct namnod *newnp;
	register struct Amemory *rootp=var_tree;
	extern	char	*strchr();
	char *cp;
	while(rootp->nexttree)
		rootp = rootp->nexttree;	/* skip to last tree */
	newnp = nam_search(oldnp->namid,rootp,N_ADD);
	if(type==0)
		return(newnp);
	oldnp->value.namflg &= ~N_CWRITE;
	newnp->value.namflg = oldnp->value.namflg&~(N_INDIRECT|N_FREE|N_ALLOC);
	newnp->namid = oldnp->namid;
	oldnp->value.namflg |= N_AVAIL;
	if(nam_istype(oldnp, N_ARRAY))
	{
		register struct namaray *ap1,*ap2;
		int dot;
		char *val;
		ap1 = array_ptr(oldnp);
		dot = ap1->adot;
		ap2 = array_grow((struct namaray*)0,ap1->maxi);
		newnp->value.namval.aray = ap2;
		for(ap1->adot=0;ap1->adot < ap1->maxi;ap1->adot++)
			if(val=nam_strval(oldnp))
			{
				ap2->adot = ap1->adot;
				nam_putval(newnp,val);
			}
		ap2->adot = dot;
	}
	else if(type==2 )
		nam_putval(newnp,nam_strval(oldnp));
	return(newnp);
}
#endif	/* NAME_SCOPE */
