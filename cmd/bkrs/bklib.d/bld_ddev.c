/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bld_ddev.c	1.1.2.1"

#include	<stdio.h>

extern void *malloc();

static
int
ncolons( str )
char *str;
{
	register n = 0;
	char *p;

	if( str ) {
		for( p = str; *p; p++ )
			if( *p == ':' ) n++;
	}
	return( n );
}

/* Copy <string> to <buffer>, escaping ':' */
static
char *
copy( buffer, string )
char *buffer, *string;
{
	if( string ) {
		while( *string ) {
			if( *string == ':' )
				*buffer++ = '\\';
			*buffer++ = *string++;
		}
	}

	return( buffer );
}

/*
	Build a <group>:<device>:<dchar>:<labels> string,
	escaping the ':'s.
*/
char *
bld_ddevice( dgroup, ddevice, dchar, dlabels )
char *dgroup, *ddevice, *dchar, *dlabels;
{
	register size, nescapes = 0;
	char *buffer, *ptr;

	nescapes = ncolons( dgroup ) + ncolons( ddevice )
		+ ncolons( dchar ) + ncolons( dlabels );

	size = nescapes + strlen( dgroup ) + strlen( ddevice )
		+ strlen( dchar ) + strlen( dlabels ) + 3 /*colons*/;

	if( !(buffer = (char *)malloc( size + 1 ) ) )
		/* no memory */
		return( buffer );

	if( !nescapes ) {
		/* No colons to escape */
		(void) sprintf( buffer, "%s:%s:%s:%s", dgroup, ddevice,
			dchar, dlabels );

	} else {
		ptr = copy( buffer, dgroup );
		*ptr++ = ':';

		ptr = copy( ptr, ddevice );
		*ptr++ = ':';

		ptr = copy( ptr, dchar );
		*ptr++ = ':';

		ptr = copy( ptr, dlabels );
		*ptr = '\0';
	}

	return( buffer );
}
