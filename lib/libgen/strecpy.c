/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:strecpy.c	1.2.5.2"

#ifdef __STDC__
	#pragma weak strecpy = _strecpy
	#pragma weak streadd = _streadd
#endif
#include "synonyms.h"

/*
	strecpy(output, input, except)
	strecpy copys the input string to the output string expanding
	any non-graphic character with the C escape sequence.
	Esacpe sequences produced are those defined in "The C Programming
	Language" pages 180-181.
	Characters in the except string will not be expanded.
	Returns the first argument.

	streadd( output, input, except )
	Identical to strecpy() except returns address of null-byte at end
	of output.  Useful for concatenating strings.
*/

#include	<ctype.h>
#include	<string.h>
#include	<stdio.h>
char *streadd();


char *
strecpy( pout, pin, except )
char	*pout;
const char	*pin;
const char	*except;
{
	(void)streadd( pout, pin, except );
	return  pout;
}


char *
streadd( pout, pin, except )
register char	*pout;
register const char	*pin;
const char	*except;
{
	register unsigned	c;

	while( c = *pin++ ) {
		if( !isprint( c )  &&  ( !except  ||  !strchr( except, c ) ) ) {
			*pout++ = '\\';
			switch( c ) {
			case '\n':
				*pout++ = 'n';
				continue;
			case '\t':
				*pout++ = 't';
				continue;
			case '\b':
				*pout++ = 'b';
				continue;
			case '\r':
				*pout++ = 'r';
				continue;
			case '\f':
				*pout++ = 'f';
				continue;
			case '\v':
				*pout++ = 'v';
				continue;
			case '\007':
				*pout++ = 'a';
				continue;
			case '\\':
				continue;
			default:
				(void)sprintf( pout, "%.3o", c );
				pout += 3;
				continue;
			}
		}
		if( c == '\\'  &&  ( !except  ||  !strchr( except, c ) ) )
			*pout++ = '\\';
		*pout++ = c;
	}
	*pout = '\0';
	return  (pout);
}
