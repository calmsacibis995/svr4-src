/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:canon.c	1.3.6.1"

#include "sys/types.h"
#include "sys/param.h"
#define bcopy(f,t,n)	memcpy(t,f,n)

#ifdef	pdp11
#define	SALIGN(p)		(char *)(((int)p+1) & ~1)
#define	IALIGN(p)		(char *)(((int)p+1) & ~1)
#define LALIGN(p)		(char *)(((int)p+1) & ~3)
#endif
#ifdef	vax
#define	SALIGN(p)		(char *)(((int)p+1) & ~1)
#define	IALIGN(p)		(char *)(((int)p+3) & ~3)
#define	LALIGN(p)		(char *)(((int)p+3) & ~3)
#endif
#if u3b2 || u3b5 || u3b15 || i386
#define	SALIGN(p)		(char *)(((int)p+1) & ~1)
#define	IALIGN(p)		(char *)(((int)p+3) & ~3)
#define	LALIGN(p)		(char *)(((int)p+3) & ~3)
#endif

#define SNEXT(p)		(char *)((int)p + sizeof (short))
#define INEXT(p)		(char *)((int)p + sizeof (int))
#define LNEXT(p)		(char *)((int)p + sizeof (long))


/*
 *convert from canonical to local representation
 */
fcanon(fmt,from,to)
register char *fmt, *from, *to;
{
	long ltmp;
	register char *cptr;
	char *lfmt, *tptr;

	tptr = to;
	while(*fmt){
		switch(*fmt++){
		case 's':
			to = SALIGN(to);
			from = LALIGN(from);
			cptr = (char *)&ltmp;
			*cptr++ = hibyte(hiword(*from));
			*cptr++ = lobyte(hiword(*from));
			*cptr++ = hibyte(loword(*from));
			*cptr++ = lobyte(loword(*from));
			*(short *)to = ltmp;
			to = SNEXT(to);
			from = LNEXT(from);
			continue;
		case 'i':
			to = IALIGN(to);
			from = LALIGN(from);
			cptr = (char *)&ltmp;
			*cptr++ = hibyte(hiword(*from));
			*cptr++ = lobyte(hiword(*from));
			*cptr++ = hibyte(loword(*from));
			*cptr++ = lobyte(loword(*from));
			*(int *)to = ltmp;
			to = INEXT(to);
			from = LNEXT(from);
			continue;
		case 'l':
			to = LALIGN(to);
			from = LALIGN(from);
			ltmp = *(long *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = LNEXT(from);
			continue;
		case 'b':
			*to++ = *from++;
			continue;
		case 'c':
			from = LALIGN(from);
			lfmt = fmt;
			ltmp = duatoi(&lfmt);
			fmt = lfmt;
			/* The following code was changed to be consistent
			 *	with the method used in the 3b2 kernel
			 * 	as the pre-existing code appeared in error.
			*/
			if(ltmp == 0) {
				cptr = (char *)&ltmp;
				*cptr++ = hibyte(hiword(*from));
				*cptr++ = lobyte(hiword(*from));
				*cptr++ = hibyte(loword(*from));
				*cptr++ = lobyte(loword(*from));
			}
			from = LNEXT(from);	
			while(ltmp--)
				*to++ = *from++;	
			continue;
		default:
			return(0);

		}
	}
	return(to - tptr);
}

/*
 *This routine converts from local to cononical representation
 */
tcanon(fmt, from, to, flag)
register char *fmt, *from, *to;
{
	long ltmp;
	char *lfmt, *tptr;
	register char *cptr = 0;
	char cbuf[1400];

	tptr = to;
	if(flag && from == to){
		cptr = to;
		to = cbuf;
	}
	while(*fmt){
		switch(*fmt++){
		case 's':
			to = LALIGN(to);
			from = SALIGN(from);
			ltmp = *(short *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = SNEXT(from);
			continue;
		case 'i':
			to = LALIGN(to);
			from = IALIGN(from);
			ltmp = *(int *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = INEXT(from);
			continue;
		case 'l':
			to = LALIGN(to);
			from = LALIGN(from);
			ltmp = *(long *)from;
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			from = LNEXT(from);
			continue;

		case 'b':
			*to++ = *from++;
			continue;
		case 'c':
			lfmt = fmt;
			ltmp = duatoi(&lfmt);
			fmt = lfmt;
			if(ltmp == 0){
				ltmp = strlen(from) + 1;
			}
			to = LALIGN(to);
			*to++ = hibyte(hiword(ltmp));
			*to++ = lobyte(hiword(ltmp));
			*to++ = hibyte(loword(ltmp));
			*to++ = lobyte(loword(ltmp));
			while(ltmp--)
				*to++ = *from++;	
			to = LALIGN(to);
			continue;
		default:
			return(0);
		}
	}
	if(cptr){
		bcopy(cbuf,cptr,(to - cbuf));
		return(to - cbuf);
	}
	return(to - tptr);
}

int
duatoi(str)
register char **str;
{
	register short n;

	n = 0;
	for( ; **str >= '0' && **str <= '9'; ++(*str))
		n = 10 * n + **str - '0';
	return(n);
}
