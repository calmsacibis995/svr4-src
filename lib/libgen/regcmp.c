/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:regcmp.c	1.1.4.2"

#ifdef __STDC__
	#pragma weak regcmp = _regcmp
#endif 
#include "synonyms.h"

#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include "_wchar.h"
#include "_range.h"

#define Popwchar	if((n = mbtowc(&cl, sp, MB_LEN_MAX)) == -1) \
				goto cerror; \
			sp += n; \
			c = cl;

#define SSIZE	50
#define TGRP	48
#define A256	02
#define ZERO	01
#define	NBRA	10
#define CIRCFL	32
#define SLOP	5
#define	EOFS	0

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

extern int strlen();
extern int strncmp();
extern void free();
char *  __rpush();
char *	__rpop();
char *	*__sp_;
char *	*__stmax;
int	__i_size;
/*VARARGS1*/
char *
#ifdef __STDC__
regcmp(const char *cs1, ...)
#else
regcmp(cs1, va_alist)
char *cs1;
va_dcl
#endif
{
	register wchar_t c;
	register char *ep;
	register const char *sp;
	char *oldep;
	const char *adx;
	int n,i,cflg;
	char *lastep, *sep, *eptr;
	int nbra,ngrp;
	int cclcnt, neg;
	char *stack[SSIZE];
	wchar_t cl, lc;
	va_list ap;
	__sp_ = stack;
	*__sp_ = (char *) 0;
	__stmax = &stack[SSIZE];
#ifdef __STDC__
	va_start(ap, cs1);
#else
	va_start(ap);
#endif
	adx = cs1;
	i = nbra = ngrp = 0;
	while(adx) {
		i += strlen(adx);
		adx = va_arg(ap, const char *);	
	}
	va_end(ap);
#ifdef __STDC__
	va_start(ap, cs1);
#else
	va_start(ap);
#endif
	sp = cs1;
	adx = va_arg(ap, const char *);
	if((sep = ep = malloc((unsigned int)(2*i+SLOP))) == (char *)0) {
		va_end(ap);
		return(0);
	}
	c = (unsigned char) *sp++;
	if (c == EOFS) {
		if (adx)
			sp = adx;
		else
			goto cerror;
		adx = va_arg(ap, const char *);
	}
	if (c=='^') {
		c = (unsigned char) *sp++;
		*ep++ = CIRCFL;
	}
	if ((c=='*') || (c=='+') || (c=='{')) {
		goto cerror;
	}
	sp--;
	for (;;) {
		if(multibyte) {
			Popwchar
		} else {
			c = (unsigned char) *sp++;
		}
		if (c == EOFS) {
			if (adx) {
				sp = adx;
				adx = va_arg(ap, const char *);
				continue;
			}
			*ep++ = CEOF;
			if (--nbra > NBRA || *__sp_ != (char *) 0) {
				goto cerror;
			}
			__i_size = ep - sep;
			va_end(ap);
			return(sep);
		}
		if ((c!='*') && (c!='{')  && (c!='+'))
			lastep = ep;
		switch (c) {

		case '(':
			if (!__rpush(ep)) {
				goto cerror;
			}
			*ep++ = CBRA;
			ep++;
			continue;
		case ')':
			if (!(eptr=__rpop())) {
				goto cerror;
			}
			c = (unsigned char) *sp;
			if (c == '$') {
				sp++;
				if ('0' > (c = (unsigned char) *sp++) || c > '9') {
					goto cerror;
				}
				*ep++ = CKET;
				*ep++ = *++eptr = (char) nbra++;
				*ep++ = (c-'0');
				continue;
			}
			*ep++ = EGRP;
			*ep++ = ngrp++;
			switch (c) {
			case '+':
				*eptr = PGRP;
				break;
			case '*':
				*eptr = SGRP;
				break;
			case '{':
				*eptr = TGRP;
				break;
			default:
				*eptr = GRP;
				continue;
			}
			i = ep - eptr - 2;
			for (cclcnt = 0; i >= 256; cclcnt++)
				i -= 256;
			if (cclcnt > 3)  {
				goto cerror;
			}
			*eptr |= cclcnt;
			*++eptr = i;
			continue;

		case '\\':
			if(multibyte) {
				Popwchar
			} else {
				c = (unsigned char) *sp++;
			}
			goto defchar;

		case '{':
			if (*lastep == CBRA || *lastep == CKET)
				goto cerror;
			*lastep |= RNGE;
			cflg = 0;
		nlim:
			if ((c = (unsigned char) *sp++) == '}') goto cerror;
			i = 0;
			do {
				if ('0' <= c && c <= '9')
					i = (i*10+(c-'0'));
				else goto cerror;
			} while (((c = (unsigned char) *sp++) != '}') && (c != ','));
			if (i>255) goto cerror;
			*ep++ = i;
			if (c==',') {
				if (cflg++) goto cerror;
				if((c = (unsigned char) *sp++) == '}') {
					*ep++ = -1;
					continue;
				}
				else {
					sp--;
					goto nlim;
				}
			}
			if (!cflg) *ep++ = i;
			else if (((int)(unsigned char)ep[-1]) < ((int)(unsigned char)ep[-2])) goto cerror;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '+':
			if (*lastep==CBRA || *lastep==CKET)
				goto cerror;
			*lastep |= PLUS;
			continue;

		case '*':
			if (*lastep==CBRA || *lastep==CKET)
			goto cerror;
			*lastep |= STAR;
			continue;

		case '$':
			if ((*sp != EOFS) || (adx))
				goto defchar;
			*ep++ = CDOL;
			continue;
		case '[':
			lc = 0;
			if(multibyte) {
				Popwchar
			} else {
				c = (unsigned char) *sp++;
			}
			neg = 0;
			if (c == '^') {
				neg = 1;
				if(multibyte) {
					Popwchar
				} else {
					c = (unsigned char) *sp++;
				}
			}
			if (multibyte) {
				if (neg) {
					*ep++ = NMCCL;
				} else {
					*ep++ = MCCL;
				}
			} else {
				if (neg) {
					*ep++ = NBCCL;
				} else {
					*ep++ = BCCL;
				}
			}
			ep++;
			cclcnt = 1;
			do {
				if (c==EOFS) {
					goto cerror;
				}
				if ((c == '-') && (lc != 0)) {
					if(multibyte) {
						Popwchar
					} else {
						c = (unsigned char) *sp++;
					}
					if (c == ']') {
						*ep++ = (unsigned char) '-';
						cclcnt++;
						break;
					}
					if (!multibyte || c <= 0177) {
						if (lc < c) {
							*ep++ = MINUS;
							cclcnt++;
						}
					} else if (valid_range(lc, c) && lc < c) {
						*ep++ = MINUS;
						cclcnt++;
					}
				}
				lc = c;
				if(!multibyte || c <= 0177 || c <= 0377 && iscntrl(c)) {
					*ep++ = c;
					cclcnt++;
				} else {
					oldep = ep;
					if ((n = wctomb(ep, c)) == -1) {
						goto cerror;
					}
					ep += n;
					cclcnt += ep - oldep;
				}
				if(multibyte) {
					Popwchar
				} else {
					c = (unsigned char) *sp++;
				}
			} while (c != ']');
			if (cclcnt > 256) {
				goto cerror;
			}
			lastep[1] = cclcnt;
			continue;

		defchar:
		default:
			if (!multibyte || c <= 0177) {
				*ep++ = CCHR;
				*ep++ = c;
			} else {
				*ep++ = MCCHR;
				if ((n = wctomb(ep,c)) == -1) {
					goto cerror;
				}
				ep += n;
			}
		}
	}
   cerror:
	free(sep);
	va_end(ap);
	return(0);
}
char *
__rpop() {
	return (*__sp_ == (char *) -1)?(char *) 0:*__sp_--;
}
char *
__rpush(ptr) char *ptr;
{
	if (++__sp_ > __stmax) return(0);
	*__sp_ = (char *)ptr;
	return((char *)1);
}
