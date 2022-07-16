/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libw:mbftowc.c	1.2"
#include <ctype.h>
#include <stdlib.h>
#include "_wchar.h"
/* returns number of bytes read by *f */
int mbftowc(s, wchar, f, peekc)
char *s;
wchar_t *wchar;
int (*f)();
int *peekc;
{
	register int length;
	register wchar_t intcode;
	register c;
	register shift;
	int lflag;
	char *olds = s;
	wchar_t mask;
	
	if((c = (*f)()) < 0)
		return 0;
	*s++ = c;
	if(!multibyte || c < 0200) {
		*wchar = c;
		return(1);
	}
	lflag = _ctype[520] > 3 || eucw1 > 2;
	intcode = 0;
	if (c == SS2) {
		if(!(length = eucw2)) 
			goto lab1;
		if (lflag)
			mask = P01;
		else
			mask = H_P01;
		goto lab2;
	} else if(c == SS3) {
		if(!(length = eucw3)) 
			goto lab1;
		if (lflag)
			mask = P01;
		else
			mask = H_P01;
		goto lab2;
	} 

lab1:
	if(iscntrl(c)) {
		*wchar = c;
		return(1);
	}
	if (lflag)
		mask = P11;
	else
		mask = H_P11;
	length = eucw1 - 1;
	intcode = c & 0177;
lab2:
	if(length < 0)
		return -1;
	
	shift = 8 - lflag;
	while(length--) {
		*s++ = c = (*f)();
		if(c < 0200 || iscntrl(c)) {
			if(c >= 0) 
				*peekc = c;
			--s;
			return(-(s - olds));
		}
		intcode = (intcode << shift) | (c & 0177);
	}
	*wchar = intcode | mask;
	return(s - olds);
}	
