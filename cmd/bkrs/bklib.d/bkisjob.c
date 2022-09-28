/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkisjob.c	1.5.2.1"

#include <string.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

/* Does this string have the right form for a restore jobid? */
int
is_rsjobid( string )
char *string;
{
	register len = strlen( string ), i;

	/*
		Valid form is 'rest-NA' where N is some number of digits and
		A is an alphabetic character.
	*/
	if ( len < 7 ) return ( FALSE );
	if( strncmp( string, "rest-", 5 ) ) return( FALSE );
	if( !isalpha( string[ len - 1 ] ) ) return( FALSE );
	for( i = 5; i < len - 1; i++ ) 
		if( !isdigit( string[ i ] ) ) return( FALSE );
	return( TRUE );
}

/* Does this string have the right form for a backup jobid? */
int
is_bkjobid( string )
char *string;
{
	register len = strlen( string ), i;

	/*
		Valid form is 'back-N' where N is some number of digits
	*/
	if( strncmp( string, "back-", 5 ) ) return( FALSE );
	for( i = 5; i < len; i++ ) 
		if( !isdigit( string[ i ] ) ) return( FALSE );
	return( TRUE );
}
