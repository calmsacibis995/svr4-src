/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbtroff:n8.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#include	<ctype.h>
#include	"tdef.h"
#include "ext.h"
#define	HY_BIT	0200	/* stuff in here only works for ascii */

/*
 * troff8.c
 * 
 * hyphenation
 */

char	hbuf[NHEX];
char	*nexth = hbuf;
tchar	*hyend;
#define THRESH 160 /*digram goodness threshold*/
int	thresh = THRESH;

hyphen(wp)
	tchar *wp;
{
	register j;
	register tchar *i;

	i = wp;
	while (punct(cbits(*i++)))
		;
	if (!alph(cbits(*--i)))
		return;
	wdstart = i++;
	while (alph(cbits(*i++)))
		;
	hyend = wdend = --i - 1;
	while (punct(cbits(*i++)))
		;
	if (*--i)
		return;
	if ((wdend - wdstart - 4) < 0)
		return;
	hyp = hyptr;
	*hyp = 0;
	hyoff = 2;
	if (!exword() && !suffix())
		digram();
	*hyp++ = 0;
	if (*hyptr) 
		for (j = 1; j; ) {
			j = 0;
			for (hyp = hyptr + 1; *hyp != 0; hyp++) {
				if (*(hyp - 1) > *hyp) {
					j++;
					i = *hyp;
					*hyp = *(hyp - 1);
					*(hyp - 1) = i;
				}
			}
		}
}


punct(i)
{
	if (!i || alph(i))
		return(0);
	else
		return(1);
}


alph(i)
{
	if (i >= 'a' && i <= 'z' || i >= 'A' && i <= 'Z')
		return(1);
	else
		return(0);
}


caseht()
{
	thresh = THRESH;
	if (skip())
		return;
	noscale++;
	thresh = atoi();
	noscale = 0;
}


casehw()
{
	register i, k;
	register char	*j;
	tchar t;

	k = 0;
	while (!skip()) {
		if ((j = nexth) >= (hbuf + NHEX - 2))
			goto full;
		for (; ; ) {
			if (ismot(t = getch()))
				continue;
			i = cbits(t);
			if (i == ' ' || i == '\n') {
				*j++ = 0;
				nexth = j;
				*j = 0;
				if (i == ' ')
					break;
				else
					return;
			}
			if (i == '-') {
				k = HY_BIT;
				continue;
			}
			*j++ = maplow(i) | k;
			k = 0;
			if (j >= (hbuf + NHEX - 2))
				goto full;
		}
	}
	return;
full:
	errprint("exception word list full.");
	*nexth = 0;
}


exword()
{
	register tchar *w;
	register char	*e;
	char	*save;

	e = hbuf;
	while (1) {
		save = e;
		if (*e == 0)
			return(0);
		w = wdstart;
		while (*e && w <= hyend && (*e & 0177) == maplow(cbits(*w))) {
			e++; 
			w++;
		};
		if (!*e) {
			if (w-1 == hyend || (w == wdend && maplow(cbits(*w)) == 's')) {
				w = wdstart;
				for (e = save; *e; e++) {
					if (*e & HY_BIT)
						*hyp++ = w;
					if (hyp > (hyptr + NHYP - 1))
						hyp = hyptr + NHYP - 1;
					w++;
				}
				return(1);
			} else {
				e++; 
				continue;
			}
		} else 
			while (*e++)
				;
	}
}


suffix()
{
	register tchar *w;
	register char	*s, *s0;
	tchar i;
	extern char	*suftab[];
	extern tchar *chkvow();

again:
	if (!alph(cbits(i = cbits(*hyend))))
		return(0);
	if (i < 'a')
		i -= 'A' - 'a';
	if ((s0 = suftab[i-'a']) == 0)
		return(0);
	for (; ; ) {
		if ((i = *s0 & 017) == 0)
			return(0);
		s = s0 + i - 1;
		w = hyend - 1;
		while (s > s0 && w >= wdstart && (*s & 0177) == maplow(cbits(*w))) {
			s--;
			w--;
		}
		if (s == s0)
			break;
		s0 += i;
	}
	s = s0 + i - 1;
	w = hyend;
	if (*s0 & HY_BIT) 
		goto mark;
	while (s > s0) {
		w--;
		if (*s-- & HY_BIT) {
mark:
			hyend = w - 1;
			if (*s0 & 0100)
				continue;
			if (!chkvow(w))
				return(0);
			*hyp++ = w;
		}
	}
	if (*s0 & 040)
		return(0);
	if (exword())
		return(1);
	goto again;
}


maplow(i)
register int	i;
{
	if (isupper(i)) 
		i = tolower(i);
	return(i);
}


vowel(i)
int	i;
{
	switch (maplow(i)) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'y':
		return(1);
	default:
		return(0);
	}
}


tchar *chkvow(w)
tchar *w;
{
	while (--w >= wdstart)
		if (vowel(cbits(*w)))
			return(w);
	return(0);
}


digram() 
{
	register tchar *w;
	register val;
	tchar * nhyend, *maxw;
	int	maxval;
	extern char	bxh[26][13], bxxh[26][13], xxh[26][13], xhx[26][13], hxx[26][13];

again:
	if (!(w = chkvow(hyend + 1)))
		return;
	hyend = w;
	if (!(w = chkvow(hyend)))
		return;
	nhyend = w;
	maxval = 0;
	w--;
	while ((++w < hyend) && (w < (wdend - 1))) {
		val = 1;
		if (w == wdstart)
			val *= dilook('a', cbits(*w), bxh);
		else if (w == wdstart + 1)
			val *= dilook(cbits(*(w-1)), cbits(*w), bxxh);
		else 
			val *= dilook(cbits(*(w-1)), cbits(*w), xxh);
		val *= dilook(cbits(*w), cbits(*(w+1)), xhx);
		val *= dilook(cbits(*(w+1)), cbits(*(w+2)), hxx);
		if (val > maxval) {
			maxval = val;
			maxw = w + 1;
		}
	}
	hyend = nhyend;
	if (maxval > thresh)
		*hyp++ = maxw;
	goto again;
}


dilook(a, b, t)
int	a, b;
char	t[26][13];
{
	register i, j;

	i = t[maplow(a)-'a'][(j = maplow(b)-'a')/2];
	if (!(j & 01))
		i >>= 4;
	return(i & 017);
}


