/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/blok.c	1.3.3.1"

#include	"defs.h"

/*
 *	storage allocator
 *	(circular first fit strategy)
 */

#define SLOP	8
#ifdef cray
    typedef struct
    {
	char *cword;
    } cptr;
#   define BUSY 0x2000000000000000
#   define busy(x)	(Rcheat(((cptr*)(x))->cword)&BUSY)
#   define setbusy(x,y)	(((cptr*)(x))->cword = ADR(Rcheat(y)|BUSY))
#else
#   ifdef univac
#	define BUSY (01<<34)
#   else
#	define BUSY 01
#   endif /* univac */
#   define busy(x)	(Rcheat((x)->word)&BUSY)
#   define setbusy(x,y)	((x)->word = BLK(Rcheat(y)|BUSY))
#endif /* cray */

#define Rcheat(x)	((sizeof(char*)==sizeof(int))? \
				((int)(x)):\
				((long)(x)))
#define BLK(x)	((struct blk*)(x))

void		free();
char		*malloc();
#ifdef DBUG
    void	chkmem();
#endif	/* DBUG */

static char		*brkbegin;	/* first sbrk() value */
static struct blk	*blokp;
static struct blk	*bloktop;	/*top of arena (last blok) */

/*
 * equivalent to malloc(3) except that a data area stack is
 * maintained on top of the heap
 */

char	*malloc(nbytes)
unsigned 	nbytes;
{
	register int  rbytes = round(nbytes+BYTESPERWORD,BYTESPERWORD);
	while(1)
	{
		register struct blk *p = blokp;
		register struct blk *q;
		register int c=0;
		do
		{
			if(!busy(p))
			{
				while((q=p->word),!busy(q))
					p->word = q->word;
				if(ADR(q)-ADR(p) >= rbytes)
				{
					blokp = BLK(ADR(p)+rbytes);
					if(q > blokp)
						blokp->word = p->word;
					setbusy(p,blokp);
					return(ADR(p+1));
				}
			}
			q = p; p = BLK(Rcheat(p->word)&~BUSY);
		}
		while(p>q || (c++)==0);
		sh_addblok(rbytes);
	}
}

/*
 * add more space to the heap and move the stack to the top of the heap
 */

void	sh_addblok(reqd)
register int reqd;
/*@
	assume reqd!=0;
@*/
{
	register STKPTR	stakadr;
	if(sh.stakbot == 0)
	{
		sh_addmem(8*BRKINCR);
		blokp = BLK(brkbegin);
		stakadr = brkbegin + 6*BRKINCR;
		blokp->word = BLK(stakadr);
		goto first;
	}
	if(sh.stakbas!=sh.staktop)
	{
		register struct blk *blokstak;
		stak_push(0);
		stakadr= sh.stakbas + round(sh.staktop-sh.stakbas,BYTESPERWORD);
		blokstak=BLK(sh.stakbas)-1;
		blokstak->word=sh.stakbsy; sh.stakbsy=blokstak;
		setbusy(bloktop,stakadr);
		bloktop=BLK(stakadr);
	}
	reqd += (sh.staktop-sh.stakbot);
	reqd = round(reqd,BRKINCR);
	if(reqd)
	{
		sh_addmem((int)reqd+BRKINCR);
		reqd -= (sh.staktop-sh.stakbot);
	}
	blokp=bloktop;
	bloktop->word = bloktop+(reqd/sizeof(struct blk*));
	stakadr = (STKPTR)(bloktop->word);
first:
	bloktop = BLK(stakadr);
	setbusy(bloktop,brkbegin);
	stakadr=(STKPTR)(bloktop+2);
	{
		register STKPTR sp = stakadr;
		if(reqd = (sh.staktop-sh.stakbot))
		{
			while(reqd-- > 0)
				*sp++ = *sh.stakbot++;
			sp--;
		}
		sh.staktop = sp;
		sh.stakbas=sh.stakbot=stakadr;
	}
}

/*
 * mark the block free if address is in the heap
 */

void	free(ap)
register char	*ap;
{
	register struct blk *p;
	if(ap>brkbegin && ap<ADR(bloktop))
	{
		p = (struct blk*)(ap-sizeof(p->word));
		p->word = (struct blk*)(Rcheat(p->word)&~BUSY);
	}
}

#ifdef FS_3D
/*
 *	reallocates a block obtained from malloc()
 *	to have new size nbytes, and old content
 *	returns new location, or NULL on failure
 */

char *realloc(ap, nbytes)
char	*ap;
unsigned nbytes;
{
	register unsigned size;
	register struct blk *p, *q;
	char *cp;
	unsigned osize;

	p = BLK(ap)-1;
	if(busy(p))
	{
		free(ap);
		blokp = p;
	}
	osize = ADR(p->word) - ap;
	cp = malloc(nbytes);
	if(cp && cp!=ap)
	{
		p = BLK(ap);
		q = BLK(cp);
		size = round(nbytes,BYTESPERWORD);
		if(osize < size)
			size = osize;
		while(size-- != 0)
			*q++ = *p++;
	}
	return(cp);
}
#endif /* FS_3D */

void sh_addmem(incr)
{
	register char *a;
#ifdef malloc
	if(incr < 0)
	{
		a = (char *)(sbrk(0));
		if(a != (sh.brkend+SLOP))
			incr = 0;
	}
#endif /* malloc */
	a = (char *)(sbrk(incr));
	if((int)a == -1)
		sh_fail(e_space,NIL);
	if(brkbegin==0)
	{
		/*
		 * The following shouldn't loop if sbrk() returns
		 * an aligned address with the busy bit off
		 */
		while(Rcheat(a)&BUSY)
		{
#ifdef DBUG
			p_setout(ERRIO);
			p_str("Unaligned initial sbrk() address",NL);
			p_flush();
#endif /* DBUG */
			a++;
		}
		brkbegin = a;
	}
	else	/* keep word aligned */
	{
		a = (char*)bloktop + round(a-(char*)bloktop,BYTESPERWORD);
#ifdef DBUG
		if(a != (sh.brkend+SLOP))
		{
			write(2,"addmem: address not contiguous\n",30);
			if((bloktop+2) != BLK(sh.stakbas))
				write(2,"bloktop wrong\n",14);
		}
#endif /* DBUG */
#ifdef malloc
		/*
		 * Standard malloc may also call sbrk(), so check that
		 * space is contiguous
		 */
		if(a != (sh.brkend+SLOP))
		{
			register struct blk *bp = bloktop+1;
			bp->word = sh.stakbsy;
			sh.stakbsy = bp--;
			bp->word = BLK(sh.brkend+SLOP)-1;
			bp = bp->word;
			setbusy(bp,a);
			bloktop = bp = BLK(a);
			setbusy(bp,brkbegin);
			sh.stakbot = sh.stakbas = (STKPTR)(bp+2);
		}
#endif /* malloc */
#ifdef DBUG
		if(a < (char*)bloktop)
		{
			p_setout(ERRIO);
			p_str("sbrk() address not monotonic",NL);
			p_flush();
		}
#endif /* DBUG */
	}
	sh.brkend=a+incr-SLOP;
#ifndef INT16
	if(sh.brkend > brkbegin + BRKMAX)
	{
		sh_fail(e_space,NIL);
	}
#endif	/* INT16 */
}

#ifdef DBUG
void chkmem()
{
	register struct blk *p = (struct blk*)brkbegin;
	register struct blk *q;
	register int 	us=0, un=0;
	char *cp;

	while(1)
	{
		q = (struct blk*) (Rcheat(p->word)&~BUSY);

		if(q<BLK(brkbegin) || q>bloktop)
			abort();
		if(p==bloktop)
			break;
		if(busy(p))
			us += q-p;
		else
		  	 un += q-p;
		if(p>=q)
		{
			p_flush();
			abort();
		}
		 p=q;
	}
	un *= sizeof(*q);
	us *= sizeof(*q);
	write(ERRIO,"free/used/missing:",18);
	cp = sh_itos(un);
	write(ERRIO,cp,strlen(cp));
	write(ERRIO," ",1);
	cp = sh_itos(us);
	write(ERRIO,cp,strlen(cp));
	write(ERRIO," ",1);
	cp = sh_itos((char*)bloktop - (char*)brkbegin - (un+us));
	write(ERRIO,cp,strlen(cp));
	write(ERRIO,"\n",1);
}

/*
 * returns 1 if <ap> is on heap and is free
 * returns 2 if <ap> is on heap and not beginning of a block
 * otherwise returns 0
 */

int	chkfree(ap)
register char	*ap;
{
	register struct blk* p;
	if(ap>brkbegin && ap<(char*)bloktop)
	{
		p = (struct blk*)(ap-sizeof(p->word));
		if(p->word<BLK(brkbegin) || p->word>bloktop)
			return(2);
		return(!busy(p));
	}
	return(0);
}
#endif	/* DBUG */
