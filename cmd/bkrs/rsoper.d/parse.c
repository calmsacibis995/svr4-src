/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/parse.c	1.1.2.1"

#include	<sys/types.h>
#include	<string.h>

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif

/* Is item "item" in the list "list" */
p_in_list( item, list, separators )
char *item, *list, *separators;
{
	register char *token;

	if( !item ) return( FALSE );

	if( !(token = strtok( list, separators )) ) return( FALSE );
	if( !strcmp( item, token ) ) return( TRUE );

	while( token = strtok( (char *)0, separators ) )
		if( !strcmp( item, token ) ) return( TRUE );

	return( FALSE );
}
