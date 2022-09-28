/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/growaray.c	1.3.3.1"
/*
 *   ARRAY_GROW (ARP, MAXI)
 *
 *        Create or expand the size of an array of namnods, ARP,
 *        such that MAXI is a legal index into ARP.
 *
 *   See Also:  nam_link(III)
 */

#include	"name.h"

#define	astchar(c)	((c)=='*' || (c)=='@')

#define round(a,b)	((a+b-1)&~(b-1))
#ifdef FLOAT
    extern double	sh_arith();
#else
    extern long		sh_arith();
#endif /* FLOAT */
extern char	*sh_itos();
extern void	sh_fail();
extern void	free();

static int	arsize();
static int	next_value();
static char	*array_end();
static unsigned short	*cur_sub;


/*
 *   ARRAY_GROW (ARP, MAXI)
 *
 *        struct Namaray *ARP;
 *
 *        int MAXI;
 *
 *        Increase the size of the array of namnods given by ARP
 *        so that MAXI is a legal index.  If ARP is NULL, an array
 *        of the required size is allocated.  A pointer to the 
 *        allocated Namaray structure is returned.
 *
 *        MAXI becomes the current index of the array.
 */

struct Namaray *array_grow(arp,maxi)
register struct Namaray *arp;
{
	register struct Namaray *ap;
	register unsigned i = 0;
	register int newsize = arsize(maxi+1);
	if (maxi >= ARRMAX)
		sh_fail (sh_itos(maxi), e_subscript);
	ap = new_of(struct Namaray,(newsize-1)*sizeof(struct Nodval*));
	ap->maxi = newsize;
	ap->cur[0] = maxi;
	if(arp)
	{
		ap->nelem = arp->nelem;
		for(;i < arp->maxi;i++)
			ap->val[i] = arp->val[i];
		free((char *)arp);
	}
	else
		ap->nelem = 0;
	for(;i < newsize;i++)
		ap->val[i] = NULL;
	return(ap);
}


/*
 *   ARSIZE (MAXI)
 *
 *        int MAXI;
 *
 *   Calculate the amount of space to be allocated to hold
 *   an array into which MAXI is a legal index.  The number of
 *   elements that will actually fit into the array (> MAXI
 *   but <= ARRMAX) is returned.
 *
 *   ALGORITHM:  The size of an array can be incremented in 
 *               lots of ARRINCR elements.  Internal size is thus
 *               the least multiple of ARRINCR that is greater than
 *               MAXI.  (Note that 0-origin indexing is used.)
 */

static int	arsize(maxi)
register int maxi;
{
	register int i = round(maxi,ARRINCR);
	return (i>ARRMAX?ARRMAX:i);
}

/*
 * Get the Namval pointer for an array.
 * Allocate the space if necessary, if flag is A_ASSIGN
 * Delete space as necessary if flag is A_DELETE
 * After the lookup is done the last @ or * subscript is incremented
 */

struct Nodval *array_find(np,flag)
struct namnod *np;
{
	register unsigned dot;
	register struct Namaray *ap = array_ptr(np);
	register struct Nodval *nv = &np->value;
	int varindex = -1;	/* last * or @ index */
#ifdef MULTIDIM
	int index = -1;	/* subscript index */
	while(++index < NDIM)
#else
#	define index 0
#endif /* MULTIDIM */
	{
		np->value.namflg &= ~N_INDIRECT;
		dot = ap->cur[index];
		/* delete a is the same as delete a[@] */
		if(dot&ARRAY_UNDEF)
		{
#ifdef MULTIDIM
			ap->cur[index+1] = dot;
#endif /* MULTIDIM */
			dot = (flag==A_DELETE?ARRAY_AT:0);
			ap->cur[index] = dot;
		}
		if(!(dot&~ARRAY_MASK))
		{
			if(dot >= ap->maxi)
			{
				ap = array_grow(ap, (int)dot);
				nv->namval.aray = ap;
			}
		}
		else if(flag!=A_ASSIGN)
		{
			varindex = index;
			if((dot &= ARRAY_MASK) == 0)
			{
				cur_sub = ap->cur;
				next_value(nv,index);
				dot = ap->cur[index]&ARRAY_MASK;
			}
		}
		if(dot >= ap->maxi)
   			sh_fail (np->namid, e_subscript);
		if((nv = ap->val[dot]) == NULL)
		{
			if(flag!=A_ASSIGN)
				return(NULL);
			ap->nelem++;
			nv = new_of(struct Nodval,0);
			ap->val[dot] = nv;
			nv->namflg = np->value.namflg & ~N_ARRAY;
			nv->namval.cp = NULL;
		}
		else
			nv = unmark(nv);
		if(flag==A_DELETE)
		{
			ap->val[dot] = 0;
			if(--ap->nelem == 0)
			{
				free((char*)ap);
				np->value.namflg = 0;
				np->value.namval.aray = 0;
				return(0);
			}
		}
#ifdef MULTIDIM
		if(nv.namflg&N_ARRAY)
			ap = nv->namval.aray;
#else
#	undef index
#endif /* MULTIDIM */
	}
	if(varindex >= 0)
		array_ptr(np)->cur[varindex]++;
	np->value.namflg |= (nv->namflg&N_INDIRECT);
	return(nv);
}

int array_next(np)
struct namnod *np;
{
	cur_sub =  array_ptr(np)->cur;
	return(next_value(&np->value,0));
}

/*
 * This routine is called for an array, i.e. nv->namflg&N_ARRAY must be set
 * This routine sets dot to the next index, if any.
 * The return value is zero, if there are no more elements or if the no
 * component has changed.
 * If a component has changed, the ARRAY_AT or ARRAY_STAR is returned depending
 * on which index has changed
 * The routine is called recursively
 */

static int next_value(nv,index)
register struct Nodval *nv;
{
	register unsigned savdot = cur_sub[index++];
	register unsigned dot = savdot&ARRAY_MASK;
	register int ret;
#ifdef MULTIDIM
	register int nextdot;
	struct Nodval *nvnext;
#endif /* MULTIDIM */
	savdot &= ~ARRAY_MASK;
	if(savdot==0)
	{
#ifdef MULTIDIM
		if((nv=nv->namval.aray->val[dot]) && (nv->namflg&N_ARRAY))
			return(next_value(nv,index));
#endif /* MULTIDIM */
		return(0);
	}
	if(savdot&ARRAY_STAR)
		ret = '*';
	else
		ret = '@';
	while(dot <  nv->namval.aray->maxi)
	{
#ifdef MULTIDIM
		if(nvnext = nv->namval.aray->val[dot])
		{
			register int next;
			cur_sub[index-1] = dot|savdot;
			if(nvnext->namflg&N_ARRAY)
			{
				if(next=next_value(nvnext,index))
					return(next);
				next = cur_sub[index];
				if(next&~ ARRAY_MASK)
					cur_sub[index] &= ~ARRAY_MASK;
				else if(nvnext->namval.aray->val[next])
					return(ret);
			}
			else if(next==0)
				return(ret);
		}
#else
		if(nv->namval.aray->val[dot])
		{
			cur_sub[index-1] = dot|savdot;
			return(ret);
		}
#endif /* MULTIDIM */
		dot++;
	}
	return(0);
}

/*
 * process an array subscript for node <np> given the subscript <cp>
 */

char *array_subscript(np,cp)
struct namnod *np;
register char *cp;
{
	register int c;
	register int dot;
	char *sp;
#ifdef MULTIDIM
	register int index = -1;
	while(++index < NDIM)
	{
		if( *cp != '[')
		{
			array_ptr(np)->cur[index] = ARRAY_UNDEF;
			continue;
		}
#endif /* MULIDIM */
		cp = array_end(sp=cp);
		c = *++sp;
		if(astchar(c) && cp==sp+1)
		{
			if(!nam_istype(np,N_ARRAY))
				return(cp+1);
			if(c == '*')
				dot = ARRAY_STAR;
			else
				dot = ARRAY_AT;
		}
		else
		{
			c = *cp;
			*cp = 0;
			dot = (int)sh_arith((char*)sp);
			*cp = c;
			if(dot >= ARRMAX || (dot < 0))
				sh_fail(np->namid,e_subscript);
			if(!nam_istype(np,N_ARRAY))
				array_dotset(np,dot);
		}
		cp++;
#ifdef MULTIDIM
		array_ptr(np)->cur[index] = dot;
	}
#else
		array_ptr(np)->cur[0] = dot;
#endif /* MULIDIM */
	if(*cp == '[')
		sh_fail(np->namid,e_subscript);
	return(cp);
}


/*
 * skip to a matching ']' and return pointer to matched character
 * routine assumes that you are sitting on the '['
 */

static char *array_end(string)
register char *string;
{
	register int count = 1;
	register int c;
	while(count>0 && (c= *++string))
	{
		if(c=='[')
			count++;
		else if(c==']')
			count--;
	}
	return(string);
}


void array_dotset(np, n)
register struct namnod *np;
{
	register struct Namaray *ap;
	register struct Nodval *vp;
	if(!nam_istype(np,N_ARRAY))
	{
		ap = array_grow((struct Namaray *)NULL,n);
		if(np->value.namval.cp)
       		{
			vp = new_of(struct Nodval,0);
			*vp = np->value;
			ap->nelem = 1;
	       	 	ap->val[0] = vp;
		}
		np->value.namval.aray = ap;
		nam_ontype(np,N_ARRAY);
        }
	else
		ap = array_ptr(np);
#ifdef MULTIDIM
	{
		register int i = NDIM;
		while(--i >=  0)
			ap->cur[i] = n;
	}
#else
	ap->cur[0] = n;
#endif /*MULTIDIM */
}
