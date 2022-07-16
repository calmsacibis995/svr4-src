/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:dirname.c	1.2.3.2"

/*
	Return pointer to the directory name, stripping off the last
	component of the path.
	Works similar to /bin/dirname
*/

#ifdef __STDC__
	#pragma weak dirname = _dirname
#endif
#include "synonyms.h"

#include	<string.h>

char *
dirname( s )
char	*s;
{
	register char	*p;

	if( !s  ||  !*s )			/* zero or empty argument */
		return  ".";

	p = s + strlen( s );
	while( p != s  &&  *--p == '/' )	/* trim trailing /s */
		;
	
	if ( p == s && *p == '/' )
		return "/";

	while( p != s )
		if( *--p == '/' ) {
			while ( *p == '/' )
				p--;
			*++p = '\0';
			return  s;
		}
	
	return  ".";
}
