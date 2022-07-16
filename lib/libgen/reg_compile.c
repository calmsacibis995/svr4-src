/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:reg_compile.c	1.2.8.2"

#ifdef __STDC__
	#pragma weak nbra = _nbra
	#pragma weak regerrno = _regerrno
	#pragma weak reglength = _reglength
	#pragma weak compile = __compile
#endif
#include "synonyms.h"

#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include "_wchar.h"
#include "_range.h"
#include "_regexp.h"

#define GETC() ((unsigned char)*sp++)
#define PEEKC() ((unsigned char)*sp)
#define ERROR(c) { \
		 	regerrno = c; \
			goto out; \
		    }
#define Popwchar	if((n = mbtowc(&cl, sp, MB_LEN_MAX)) == -1) \
				ERROR(67) \
			oldsp = sp; \
			sp += n; \
			c = cl;

int	nbra = 0, regerrno = 0, reglength = 0;

static unsigned char	_bittab[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
char *_compile();
char *strchr();

char *
compile(sp, ep, endbuf)
const char *sp;
char *ep, *endbuf;
{
	return _compile(sp, ep, endbuf, 0);
}

char *
_compile(sp, ep, endbuf, viflag)
register const char *sp; 
register char *ep;
char *endbuf;
int viflag;
{
	register wchar_t c;
	register int n;
	wchar_t d;
	register const char *oldsp;
	char *lastep;
	int cclcnt;
	char bracket[NBRA], *bracketp;
	int closed;
	int neg;
	int alloc;
	wchar_t lc, cl;
	int i, cflg;
	char *expbuf = ep;
	register char *start;

	regerrno = 0;
	reglength = 0;
	lastep = 0;
	bracketp = bracket;
	closed = 0;
	alloc = 0;
	
	oldsp = sp;
	if((c = *sp++) == '\0' ) {
		if(ep == (char *)0 || ep[1] == 0)
			ERROR(41);
		goto out;
	}
	nbra = 0;	
	if(ep == (char *)0) {
		/* malloc space */
		register const char *startsp = oldsp;
		n = 0;
		while(d = *startsp++) {
			if(d == '[')
				n += 33; /* add room for bitmaps */
		}
		n += 2 * (startsp - oldsp) + 3;
		if((ep = malloc(n)) == (char *)0)
			ERROR(50);
		expbuf = ep;
		alloc = 1;
		endbuf = ep + n;
	}
	
	if(c == '^')
		*ep++ = 1;
	else  {
		*ep++ = 0; 
		sp--;
	}

	endbuf--; /* avoid extra check for overflow */
	while(1) {
		if(ep >= endbuf)
			ERROR(50);
		Popwchar
		if(c != '*' && ((c != '\\') || (PEEKC() != '{')))
			lastep = ep;
		if(c == '\0') {
			*ep++ = CCEOF;
			if (bracketp != bracket)
				ERROR(42);
			goto out;
		}
		switch(c) {

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if(lastep == 0 || *lastep == CBRA || *lastep == CKET || *lastep == CBRC || *lastep == CLET)
				goto defchar;
			*lastep |= STAR;
			continue;

		case '$':
			/* look one character ahead to see if $ means
			   to anchor match at end of line */
			if((d = PEEKC()) != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			start = ep + 34;
			if(start > endbuf)
				ERROR(50);

			*ep++ = CCL;
			lc = 0;
			for(i = 0; i < 32; i++)
				ep[i] = 0;
				
			neg = 0;
			Popwchar
			if(c == '^') {
				neg = 1;
				Popwchar
			}
			if(multibyte) {
				if(neg) {
					/* do not negate bitmap for 
					   for multibyte characters */
					neg = 0;
					ep[-1] = NMCCL;
					/* turn off null byte */
					ep[0] |= 01; 
				} else
					ep[-1] = MCCL;
			}
			do {
				if(c == '\0')
					ERROR(49);
				if(c == '-' && lc != 0) {
					Popwchar
					if(c == '\0')
						ERROR(49);
					if(c == ']') {
						PLACE('-');
						break;
					}
					/*
					 * ranges do not span code sets 
					 */
					if(!multibyte || c <= 0177) 
						while(lc < c ) {
							PLACE(lc);
							lc++;
						}
					else if(valid_range(lc, c) && lc < c)
						/* insert '-' for range */
						*start++ = '-';
					if(viflag & 1)
						lc = 0;
					else
						lc = c;
				} else if(c == '\\' && (viflag & 1) && strchr("\\^-]", PEEKC())) {
					c = GETC();
					lc = c;
				} else
					lc = c;
				/* put eight bit characters into bitmap */
				if(!multibyte || c <= 0177 || c <= 0377 && iscntrl(c)) 
					 PLACE(c);
				else {
					/*
					 * insert individual bytes of 
					 * multibyte characters after 
					 * bitmap
					 */
					if(start + n > endbuf)
						ERROR(50);
					while(n--)
						*start++ = *oldsp++;
				}
				Popwchar
			} while(c != ']');
			
			if(neg) {
				for(cclcnt = 0; cclcnt < 32; cclcnt++)
					ep[cclcnt] ^= 0377;
				ep[0] &= 0376;
			}
			ep += 32;
			if(multibyte) {
				/*
				 * Only allow 256 bytes to
				 * represent multibyte characters
				 * character class
				 */
				if(start - ep > 255)
					ERROR(50);
				*ep = start - ep;
				ep = start;
			}
			continue;

		case '\\':
			Popwchar
			switch(c) {

			case '(':
				if(nbra >= NBRA)
					ERROR(43);
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;

			case ')':
				if(bracketp <= bracket) 
					ERROR(42);
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;

			case '{':
				if(lastep == (char *) 0)
					goto defchar;
				*lastep |= RNGE;
				cflg = 0;
				c = GETC();
			nlim:
				i = 0;
				do {
					if('0' <= c && c <= '9')
						i = 10 * i + c - '0';
					else
						ERROR(16);
				} while(((c = GETC()) != '\\') && (c != ','));
				if(i > 255)
					ERROR(11);
				*ep++ = i;
				if(c == ',') {
					if(cflg++)
						ERROR(44);
					if((c = GETC()) == '\\')
						*ep++ = 255;
					else
						goto nlim;
						/* get 2'nd number */
				}
				if(GETC() != '}')
					ERROR(45);
				if(!cflg)	/* one number */
					*ep++ = i;
				else if((int)(unsigned char)ep[-1] < (int)(unsigned char)ep[-2])
					ERROR(46);
				continue;

			case 'n':
				c = '\n';
				goto defchar;

			case '<':
				*ep++ = CBRC;
				continue;
			
			case '>':
				*ep++ = CLET;
				continue;

			default:
				if(c >= '1' && c <= '9') {
					if((c -= '1') >= closed)
						ERROR(25);
					*ep++ = CBACK;
					*ep++ = c;
					continue;
				}
			}
				
	/* Drop through to default to use \ to turn off special chars */

		defchar:
		default:
			lastep = ep;
			if(!multibyte || c <= 0177) {
				/* 8-bit character */
				*ep++ = CCHR;
				*ep++ = c;
			} else {
				/* multibyte character */
				*ep++ = MCCHR;
				if(ep + n > endbuf)
					ERROR(50);
				while(n--)
					*ep++ = *oldsp++;
			}
		}
	}
out:
	if(regerrno) {
		if(alloc)
			free(expbuf);
		return (char *)0;
	}
	reglength = ep - expbuf;
	if(alloc)
		return expbuf;
	return(ep);
}
