/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/wctomb.c	1.6"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <ctype.h>
#include <stdlib.h>
#include "_wchar.h"

int
wctomb(s, wchar)
char *s;
wchar_t wchar;
{
	char *olds = s;
	register int size, index;
	unsigned char d;
	int shift, lflag;
	if(!s)
		return(0);
	if(!multibyte || wchar <= 0177 || wchar <= 0377 && iscntrl(wchar)) {
		*s++ = (char)wchar;
		return(1);
	}
	lflag = _ctype[520] > 3 || eucw1 > 2;
	if(lflag)
		switch(wchar & EUCMASK) {
			
			case P11:
				size = eucw1;
				break;
			
			case P01:
				*s++ = (char)SS2;
				size = eucw2;
				break;
			
			case P10:
				*s++ = (char)SS3;
				size = eucw3;
				break;
			
			default:
				return(-1);
		}
	else
		switch(wchar & H_EUCMASK) {
			
			case H_P11:
				size = eucw1;
				break;
			
			case H_P01:
				*s++ = (char)SS2;
				size = eucw2;
				break;
			
			case H_P10:
				*s++ = (char)SS3;
				size = eucw3;
				break;
			
			default:
				return(-1);
		}
	if((index = size) <= 0)
		return -1;	
	shift = 8 - lflag;
	while(index--) {
		d = wchar | 0200;
		wchar >>= shift;
		if(iscntrl(d))
			return(-1);
		s[index] = d;
	}
	return(s + size - olds);
}
