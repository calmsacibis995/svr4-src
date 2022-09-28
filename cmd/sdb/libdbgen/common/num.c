/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libdbgen/common/num.c	1.1"
/*	
	true if string is numeric characters
*/

#include	<ctype.h>
#include	"libgen.h"


int
num( string )
register char *string;
{
	if( !string  ||  !*string )
		return  0;	/* null pointer or null string */
	while( *string )
		if( !isdigit( *(string++) ) )
			return  0;
	return  1;
}
