/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/namscan.c	1.2.3.1"
/*
 *   GSCAN_ALL (FN, ROOT)
 *        Execute FN at each node in the linked memory trees,
 *        which are given by ROOT.
 *
 *   GSCAN_SOME (FN, ROOT, MASK, FLAG)
 *        Execute FN at those nodes in the linked memory trees
 *        that have certain attributes, as determined by MASK and
 *        FLAG. ROOT is the first of the list of memory trees.
 *
 *   SCAN_ALL (FN, ROOT)
 *        Execute function FN at each of the namnods in the tree
 *        given by ROOT.
 *
 */

#include	"name.h"

/* These routines are defined by this module */
int	gscan_all();
int	scan_all();
int	gscan_some();

static int scanmask;
static int scanflag;


/*
 *   GSCAN_ALL (FN, ROOT)
 *   
 *        int (*FN)();
 *
 *	  struct Amemory *root;
 *
 *   Execute FN at each node in the linked memory trees. 
 *   Note that the first tree need not exist.
 */

int	gscan_all(fn, root)
void	(*fn)();
struct Amemory *root;
{
	register struct Amemory *app = root;
	register int n = 0;
	while(app)
	{
		n += scan_all(fn,app);
		app = app->nexttree;
	}
	return(n);
}


/*
 *   GSCAN_SOME (FN, ROOT, MASK, FLAG)
 *        int (*FN)();
 *        struct Amemory *ROOT;
 *        int MASK;
 *        int FLAG;
 *
 *   Execute FN at each of the namnods in the trees given by ROOT
 *   that meet certain criteria, as determined by MASK and FLAG.
 *   If flag is non-zero then at least one of these mask bits must be on.
 *   If flag is zero then all the mask bits must be off to match.
 */

int	gscan_some(fn,root,mask,flag)
void (*fn)();
int mask,flag;
struct Amemory *root;
{
	register int n;
	scanmask = mask;
	scanflag = flag;
	n = gscan_all(fn,root);
	scanmask = scanflag = 0;
	return(n);
}

/*
 *   SCAN_ALL (FN, ROOT)
 *        int (*FN)();
 *        struct Amemory *ROOT;
 *
 *   Execute FN at each node in the tree given by ROOT, according
 *   to the values of scanmask and scanflag, which are established
 *   in scan_some().
 */

int	scan_all(fn,root)
void (*fn)();
struct Amemory *root;
{
	register struct namnod *np;
	register int i;
#ifdef NAME_SCOPE
	register int smask = scanmask^N_AVAIL;
#else
	register int smask = scanmask;
#endif /* NAME_SCOPE */
	register int count = 0;
	int k;
	for(i=0;i <= root->memsize;i++)
	{
		for(np=root->memhead[i];np;np= np->namnxt)
		{
			k = np->value.namflg&smask;
			if((scanflag?scanflag&k:k==0))
			{
				if(np->value.namval.cp==0)
					if(nam_istype(np,~N_DEFAULT)==0)
						continue;
				if(fn)
				{
					if(nam_istype(np,N_ARRAY))
						array_dotset(np,0);
					(*fn)(np);
				}
				count++;
			}
		}
	}
	return(count);
}

