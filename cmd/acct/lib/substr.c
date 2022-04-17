/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:lib/substr.c	1.5.2.2"
/*
	Place the `len' length substring of `as' starting at `as[origin]'
	in `aresult'.
	Return `aresult'.
 
  Note: The copying of as to aresult stops if either the
	specified number (len) characters have been copied,
	or if the end of as is found.
	A negative len generally guarantees that everything gets copied.
*/

char *substr(as, aresult, origin, len)
char *as, *aresult;
int origin;
register unsigned len;
{
	register char *s, *result;

	s = as + origin;
	result = aresult;
	++len;
	while (--len && (*result++ = *s++)) ;
	if (len == 0)
		*result = 0;
	return(aresult);
}
