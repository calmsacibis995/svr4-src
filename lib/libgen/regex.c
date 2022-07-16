/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:regex.c	1.1.3.4"

#ifdef __STDC__
	#pragma weak regex = _regex
#endif
#include "synonyms.h"

#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include "_wchar.h"


#define Popwchar(p, l) mbtowc(&l, p, MB_LEN_MAX)

#define SSIZE	50
#define TGRP	48
#define A256	01
#define A512	02
#define A768	03
#define	NBRA	10
#define CIRCFL	32
#define	EOF	-1

#define	CBRA	60
#define GRP	40
#define SGRP	56
#define PGRP	68
#define EGRP	44
#define RNGE	03
#define	CCHR	20
#define	CDOT	64
#define	CCL	24
#define	NCCL	8
#define	CDOL	28
#define	CEOF	52
#define	CKET	12
#define MCCHR	36
#define MCCL	72
#define NMCCL	76
#define BCCL	80
#define NBCCL	84

#define	STAR	01
#define PLUS	02
#define MINUS	16


char *__execute(), *__advance();
int __xpush(), __getrnge(), __cclass(), __bcclass(), __abcclass();
char *__xpop();

char	*__braslist[NBRA];
char	*__braelist[NBRA];
char	*__loc1;
int	__bravar[NBRA];
char 	*__st[SSIZE + 1];
char * 	*__eptr_;
char *  *__lptr_;
int	__cflg;
/*VARARGS2*/
char *
#ifdef __STDC__
regex(const char *addrc, const char *addrl, ...)
#else
regex(addrc, addrl, va_alist)
char *addrc, *addrl;
va_dcl
#endif
{
	char *cur;
	int in;
	char *adx[NBRA];
	va_list ap;
	register char *p1, *p2;
	
#ifdef __STDC__
	va_start(ap, addrl);
#else
	va_start(ap);
#endif
	for(in=0;in<NBRA;in++) {
		adx[in] = va_arg(ap, char *);
		__braslist[in] = 0;
		__bravar[in] = -1;
	}
	va_end(ap);
	__cflg = 0;
	cur = __execute(addrc,addrl);
	if(cur == (char *)-1)
		return((char *)-1);
	for(in=0;in<NBRA;in++) {
		if ((p1 = __braslist[in]) && (__bravar[in] >= 0)) {
			p2 = adx[__bravar[in]];
			while(p1 < __braelist[in]) *p2++ = *p1++;
			*p2 = '\0';
		}
	}
	if (!__cflg) return (addrl==cur)?(char *)0:cur;
	else return(cur);
}

char *
__execute(addrc,addrl)
char *addrc,*addrl;
{
	register char *p1, *p2, c;
	char *i;
	wchar_t cl;
	int n;

	p1 = addrl;
	p2 = addrc;
	__eptr_ = &__st[SSIZE];
	__lptr_ = &__st[0];
	if (*p2==CIRCFL) {
		__loc1 = p1;
		if(i=__advance(p1,++p2))
			return(i);
		else if(i == (char *) -1)
			return((char *)-1);
		else
			return(addrl);
	}
	if (*p2==CCHR) {
	/* fast check for first character */
		c = p2[1];
		do {
			if (*p1!=c)
				continue;
			__eptr_ = &__st[SSIZE];
			__lptr_ = &__st[0];
			if (i=__advance(p1, p2))  {
				__loc1 = p1;
				return(i);
			} else if(i == (char *)-1)
				return((char *)-1);
		} while (*p1++);
		return(addrl);
	} 
	if (multibyte) {
		do {
			__eptr_ = &__st[SSIZE];
			__lptr_ = &__st[0];
			if ((i = __advance(p1, p2))) {
				__loc1 = p1;
				return(i);
			} else if (i == (char *)-1)
				return((char *)-1);
			n = Popwchar(p1, cl);
			if(n < 0)   /* skip past illagal multibyte char's */
				p1++;
			else
				p1 += n;
		} while(n);
		return(addrl);
	}
	/* regular algorithm */
	do {
		__eptr_ = &__st[SSIZE];
		__lptr_ = &__st[0];
		if (i=__advance(p1, p2))  {
			__loc1 = p1;
			return(i);
		} else if (i == (char *)-1)
			return((char *)-1);
	} while (*p1++);
	return(addrl);
}
char *
__advance(lp, ep)
register char *lp, *ep;
{
 	char *curlp;
	register wchar_t c, d;
	char *sep, *dp, *rp;
	char *ptr;
	int n,i,lcnt,dcnt,gflg;
	wchar_t cl;
	int neg;

	gflg = 0;
	for (;;) {
		c = -1;
		neg = 1;
		switch(*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);
	
	case MCCHR:
		ep += Popwchar(ep, cl);
		c = cl;
		if((n = Popwchar(lp, cl)) <= 0 || c != cl)
			return(0);
		lp += n;
		continue;

	case EGRP|RNGE:
		return(lp);
	case EGRP:
	case GRP:
		ep++;
		continue;

	case EGRP|STAR:
		(void)__xpop(0);
	case EGRP|PLUS:
		if(__xpush(0,++ep) == -1)
			return((char *)-1);
		return(lp);

	case CDOT:
		if((n = Popwchar(lp, cl)) > 0) {
			lp += n;
			continue;
		}
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		lp++;
		return(0);

	case CEOF:
		__cflg = 1;
		return(lp);

	case TGRP:
	case TGRP|A768:
	case TGRP|A512:
	case TGRP|A256:
		i = (((ep[-1]&03)<<8) + ((unsigned char)*ep));
		ep++;
		if(__xpush(0,ep + i + 2) == -1)
			return((char *)-1);
		gflg = 1;
		(void)__getrnge(&lcnt,&dcnt,&ep[i]);
		while(lcnt--)
			if (!(lp=__advance(lp,ep)))
				return(0);
		if(__xpush(1,curlp=lp) == -1)
			return((char *)-1);
		while(dcnt--)
			if(!(dp=__advance(lp,ep))) break;
			else if(__xpush(1,lp=dp) == -1)
					return((char *)-1);
		ep = __xpop(0);
		goto star;
	case MCCHR|RNGE:
		ep += Popwchar(ep, cl);
		c = cl;
		(void)__getrnge(&lcnt,&dcnt,ep);
		while(lcnt--) {
			if((n = Popwchar(lp, cl)) <= 0 || cl != c)
				return(0);
			lp += n;
		}
		curlp = lp;
		while(dcnt--) {
			if ((n = Popwchar(lp, cl)) <= 0 || cl != c)
				break;
			lp += n;
		}
		if (dcnt < 0)
			n = Popwchar(lp, cl);
		if (n == -1)
			return(0);
		lp += (n ? n : 1);
		ep += 2;
		goto mstar;
	case CCHR|RNGE:
		sep = ep++;
		(void)__getrnge(&lcnt,&dcnt,ep);
		while(lcnt--)
			if(*lp++!=*sep) return(0);
		curlp = lp;
		while(dcnt--)
			if(*lp++!=*sep) break;
		if (dcnt < 0) lp++;
		ep += 2;
		goto star;
	case CDOT|RNGE:
		(void)__getrnge(&lcnt,&dcnt,ep);
		if (multibyte){
			while (lcnt--) {
				if((n = Popwchar(lp, cl)) <= 0) return(0);
				lp += n;
			}
		} else {
			while (lcnt--)
				if (*lp++ == '\0') return(0);
		}
		curlp = lp;
		if (multibyte) {
			while(dcnt--) {
				if((n = Popwchar(lp, cl)) < 0) 
					return 0;
				if(n == 0) {
					lp++;
					break;
				}
				lp += n;
			}
		} else {
			while(dcnt--) 
				if (*lp++ == '\0') break;
		}
		if (dcnt < 0) {
			if((n = Popwchar(lp, cl)) == -1)
				return(0);
			lp += (n ? n :1);
		}
		ep += 2;
		goto mstar;
	case NCCL|RNGE:
		neg = 0;
	case CCL|RNGE:
		(void)__getrnge(&lcnt,&dcnt,(ep + ((unsigned char)*ep)));
		while(lcnt--)
			if(!__cclass(ep,*lp++,neg)) return(0);
		curlp = lp;
		while(dcnt--)
			if(!__cclass(ep,*lp++,neg)) break;
		if (dcnt < 0) lp++;
		ep += (*ep + 2);
		goto star;
	case NMCCL|RNGE:
		neg = 0;
	case MCCL|RNGE:
		(void)__getrnge(&lcnt,&dcnt,(ep + ((unsigned char)*ep)));
		rp = lp;
		while(lcnt--)
			if((d = __bcclass(ep,&rp,neg)) != 1) {
				return(0);
			}
		curlp = rp;
		while(dcnt--)
			if((d = __bcclass(ep,&rp,neg)) != 1) {
				break;
			}
		if (d == -1)
			return(0);
		lp = rp;
		if (dcnt < 0) {
			if((n = Popwchar(lp, cl)) == -1)
				return(0);
			lp += (n ? n : 1);
		}
		ep += (*ep +2);
		goto mstar;
	case NBCCL|RNGE:
		neg = 0;
	case BCCL|RNGE:
		(void)__getrnge(&lcnt,&dcnt,(ep + ((unsigned char)*ep)));
		rp = lp;
		while(lcnt--)
			if((d = __abcclass(ep,&rp,neg)) != 1) {
				return(0);
			}
		curlp = rp;
		while(dcnt--)
			if((d = __abcclass(ep,&rp,neg)) != 1) {
				break;
			}
		lp = rp;
		if (dcnt < 0) lp++;
		ep += (*ep +2);
		goto star;
	case NCCL:
		neg = 0;
	case CCL:
		if (__cclass(ep, *lp++, neg)) {
			ep += *ep;
			continue;
		}
		return(0);
	case NBCCL:
		neg = 0;
	case BCCL:
		rp = lp;
		if ((d = __abcclass(ep, &rp, neg)) == 1) {
			ep += *ep;
			lp = rp;
			continue;
		}
		return(0);
	case NMCCL:
		neg = 0;
	case MCCL:
		rp = lp;
		if ((d = __bcclass(ep, &rp, neg)) == 1) {
			ep += *ep;
			lp = rp;
			continue;
		}
		return(0);

	case CBRA:
		__braslist[*ep++] = lp;
		continue;

	case CKET:
		__braelist[*ep] = lp;
		__bravar[*ep] = ep[1];
		ep += 2;
		continue;

	case CDOT|PLUS:
		if ((n = Popwchar(lp, cl)) <= 0) return(0);
		lp += n;
	case CDOT|STAR:
		curlp = lp;
		if (!multibyte) {
			while (*lp++);
			goto star;
		}
		else {
			while ((n = Popwchar(lp, cl)) > 0)
				lp += n;
			if (n == -1)
				return(0);
			if(n == 0)
				lp++;
		}
		goto mstar;

	case CCHR|PLUS:
		if (*lp++ != *ep) return(0);
	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;
	case MCCHR|PLUS:
		ep += Popwchar(ep, cl);
		c = cl;
		lp += Popwchar(lp, cl);
		d = cl;
		if (d != c) return(0);
	case MCCHR|STAR:
		curlp = lp;
		if (c == -1) {
			ep += Popwchar(ep, cl);
			c = cl;
		}
		while ((n = Popwchar(lp, cl)) > 0 && cl == c)
			lp += n;
		if (n == -1)
			return(0);
		lp += (n ? n : 1);
		goto mstar;

	case PGRP:
	case PGRP|A256:
	case PGRP|A512:
	case PGRP|A768:
		if (!(lp=__advance(lp,ep+1))) return(0);
	case SGRP|A768:
	case SGRP|A512:
	case SGRP|A256:
	case SGRP:
		i = (((ep[-1]&03) << 8) + (*ep&0377));
		ep++;
		if(__xpush(0,ep + i) == -1)
			return((char *)-1);
		if(__xpush(1,curlp=lp) == -1)
			return((char *)-1);
		while (ptr=__advance(lp,ep))
			if(__xpush(1,lp=ptr) == -1)
				return((char *)-1);
		ep = __xpop(0);
		gflg = 1;
		goto star;

	case CCL|PLUS:
	case NCCL|PLUS:
		if (!__cclass(ep,*lp++,ep[-1]==(CCL|PLUS))) return(0);
	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (__cclass(ep, *lp++, ((ep[-1]==(CCL|STAR)) || (ep[-1]==(CCL|PLUS)))));
		ep += *ep;
		goto star;
	case NBCCL|PLUS:
		neg = 0;
	case BCCL|PLUS:
		rp = lp;
		if ((d = __abcclass(ep,&rp,neg)) != 1) return(0);
		lp = rp;
	case BCCL|STAR:
	case NBCCL|STAR:
		rp = lp;
		curlp = lp;
		while ((d = __abcclass(ep, &rp, ((ep[-1]==(BCCL|STAR)) || (ep[-1]==(BCCL|PLUS))))) == 1);
		ep += *ep;
		lp = rp;
		goto star;
	case NMCCL|PLUS:
		neg = 0;
	case MCCL|PLUS:
		rp = lp;
		if ((d = __bcclass(ep,&rp,neg)) != 1) return(0);
		lp = rp;
	case MCCL|STAR:
	case NMCCL|STAR:
		curlp = rp = lp;
		while ((d = __bcclass(ep, &rp, ((ep[-1]==(MCCL|STAR)) || (ep[-1]==(MCCL|PLUS))))) == 1);
		if(d == -1)
			return(0);
		ep += *ep;
		lp = rp;
		goto mstar;
	default:
		return(0);

	}
	}
	star:
		do {
			if(!gflg) lp--;
			else if (!(lp=__xpop(1))) break;
			if (ptr=__advance(lp, ep)) 
				return(ptr);
		} while (lp > curlp);
		return(0);

	mstar:
		do {
			register char *p1, *p2;
			lp--;
			p1 = lp -eucw2;
			p2 = lp -eucw3;
			if ((unsigned char)*lp >= 0200) {
				if (p1 >= curlp && (unsigned char)*p1 == SS2)
					lp = p1;
				else if (p2 >= curlp && (unsigned char)*p2 == SS3)
					lp = p2;
				else
					lp = lp -eucw1 + 1;
			}
			if (ptr=__advance(lp, ep))
				return(ptr);
		} while (lp > curlp);
		return(0);

}
int
__cclass(aset, ac, af)
char *aset, ac;
int af;
{
	register char *set, c;
	register n;

	set = aset;
	if ((c = ac) == 0)
		return(0);
	n = *set++;
	while (--n) {
		if (*set == MINUS) {
			if (((int)set[2] - (int)set[1]) < 0) return(0);
			if ((int)*++set <= (int)c) {
				if ((int)c <= (int)*++set)
					return(af);
			}
			else ++set;
			++set;
			n -= 2;
			continue;
		}
		if (*set++ == c)
			return(af);
	}
	return(!af);
}
int
__bcclass(ep, rp, neg)
register char *ep, **rp;
int neg;
{
	register char *lp;
	register wchar_t c, d, f;
	register int n;
	int p;
	char *endep;
	wchar_t cl;

	lp = *rp;
	if((p = Popwchar(lp, cl)) <= 0) {
		*rp = lp;
		return(-1);
	}
	lp += (p ? p : 1);
	*rp = lp;
	c = cl;
	n = *ep++;
	if (n <= 0)
		return(!neg);
	endep = ep + n;
	while(--n >= 0) {
		if(ep >= endep)
			return(!neg);
		ep += Popwchar(ep, cl);
		d = cl;
		if(d == MINUS) {
			p = Popwchar(ep, cl);
			ep += p;
			d = cl;
			n -= p;
			if(f <= c && c <= d)
				return(neg);
		}
		if(d == c)
			return(neg);
		f = d;
	}
	return(!neg);
}
int
__abcclass(ep, rp, neg)
register char *ep, **rp;
int neg;
{
	register char *lp;
	register char c, d, f;
	register int n;
	char *endep;
	lp = *rp;
	if((c = *lp++) == 0) {
		*rp = lp;
		return(-1);
	}
	*rp = lp;
	n = *ep++;
	if (n <= 0)
		return(!neg);
	endep = ep + n;
	while(--n >= 0) {
		if(ep >= endep)
			return(!neg);
		d = *ep++;
		if(d == MINUS) {
			d = *ep++;
			n -= 2;
			if(f <= c && c <= d)
				return(neg);
		}
		if(d == c)
			return(neg);
		f = d;
	}
	return(!neg);
}
int
__xpush(i,p) int i; char *p;
{
	if (__lptr_ >= __eptr_) return(-1);
	if (i) *__lptr_++ =  (char *)p;
	else   *__eptr_-- =  (char *)p;
	return(1);
}
char *
__xpop(i) int i;
{
	if (i)
		return (__lptr_ < &__st[0])?0:*--__lptr_;
	else
		return (__eptr_ > &__st[SSIZE])?0:*++__eptr_;
}
int
__getrnge(i,j,k) int *i,*j; char *k;
{
	*i = (*k++&0377);
	if (*k == -1) *j = 20000;
	else *j = ((*k&0377) - *i);
	return(1);
}
