/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/mbtowc.c	1.6"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <ctype.h>
#include <stdlib.h>
#include "_wchar.h"

int
mbtowc(wchar, s, n)
wchar_t *wchar;
const char *s;
size_t n;
{
	register int length;
	register wchar_t intcode;
	register c;
	char *olds = (char *)s;
	wchar_t mask;
	int lflag = 0;
	int shift;
	
	if(s == (char *)0)
		return 0;
	if(n == 0)
		return(-1);
	c = (unsigned char)*s++;
	if(!multibyte || c < 0200) {
		if(wchar)
			*wchar = c;
		return(c ? 1 : 0);
	}
	lflag = _ctype[520] > 3 || eucw1 > 2;
	intcode = 0;
	if (c == SS2) {
		if(!(length = eucw2)) 
			goto lab1;
		if(lflag)
			mask = P01;
		else 
			mask = H_P01;
		goto lab2;
	} else if(c == SS3) {
		if(!(length = eucw3)) 
			goto lab1;
		if(lflag)
			mask = P10;
		else
			mask = H_P10;
		goto lab2;
	} 
lab1:
	if(iscntrl(c)) {
		if(wchar)
			*wchar = c;
		return(1);
	}
	length = eucw1 - 1;
	if(lflag)
		mask = P11;
	else
		mask = H_P11;
	intcode = c & 0177;
lab2:
	if(length + 1 > n || length < 0)
		return(-1);
	shift = 8 - lflag;
	while(length--) {
		if((c = (unsigned char)*s++) < 0200 || iscntrl(c))
			return(-1);
		intcode = (intcode << shift) | (c & 0177);
	}
	if(wchar)
		*wchar = intcode | mask;
	return((char *)s - olds);
}	

#undef mblen

int
mblen(s, n)
const char *s;
size_t n;
{
	return(mbtowc((wchar_t *)0, s, n));
}
