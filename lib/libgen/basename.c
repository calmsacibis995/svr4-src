/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:basename.c	1.2.3.2"

/*
	Return pointer to the last element of a pathname.
*/

#ifdef __STDC__
	#pragma weak basename = _basename
#endif
#include "synonyms.h"

#include	<string.h>


char *
basename( s )
char	*s;
{
	register char	*p;

	if( !s  ||  !*s )			/* zero or empty argument */
		return  ".";

	p = s + strlen( s );
	while( p != s  &&  *--p == '/' )	/* skip trailing /s */
		*p = '\0';
	
	if ( p == s && *p == '\0' )		/* all slashes */
		return "/";

	while( p != s )
		if( *--p == '/' )
			return  ++p;

	return  p;
}
