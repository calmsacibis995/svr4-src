/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/ctypefcns.c	1.4"
#include "synonyms.h"
#include <ctype.h>

#undef isalpha
#undef isupper
#undef islower
#undef isdigit
#undef isxdigit
#undef isalnum
#undef isspace
#undef ispunct
#undef isprint
#undef isgraph
#undef iscntrl
#undef isascii
#undef _toupper
#undef _tolower
#undef toascii

#ifdef __STDC__
	#pragma weak isascii = _isascii
	#pragma weak toascii = _toascii
#define isascii _isascii
#define toascii _toascii
#endif

int
isalpha(c)
int c;
{
	return((_ctype + 1)[c] & (_U | _L));
}

int
isupper(c)
int c;
{
	return((_ctype + 1)[c] & _U);
}

int
islower(c)
int c;
{
	return((_ctype + 1)[c] & _L);
}

int
isdigit(c)
int c;
{
	return((_ctype + 1)[c] & _N);
}

int
isxdigit(c)
int c;
{
	return((_ctype + 1)[c] & _X);
}

int
isalnum(c)
int c;
{
	return((_ctype + 1)[c] & (_U | _L | _N));
}

int
isspace(c)
int c;
{
	return((_ctype + 1)[c] & _S);
}

int
ispunct(c)
int c;
{
	return((_ctype + 1)[c] & _P);
}

int
isprint(c)
int c;
{
	return((_ctype + 1)[c] & (_P | _U | _L | _N | _B));
}

int
isgraph(c)
int c;
{
	return((_ctype + 1)[c] & (_P | _U | _L | _N));
}

int
iscntrl(c)
int c;
{
	return((_ctype + 1)[c] & _C);
}

int
isascii(c)
int c;
{
	return(!(c & ~0177));
}

int
_toupper(c)
int c;
{
	return((_ctype + 258)[c]);
}

int
_tolower(c)
int c;
{
	return((_ctype + 258)[c]);
}

int
toascii(c)
int c;
{
	return((c) & 0177);
}
