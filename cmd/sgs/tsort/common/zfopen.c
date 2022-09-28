/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:zfopen.c	1.1"

/*	fopen(3S) with error checking
*/

#include	"errmsg.h"
#include	<stdio.h>


FILE *
zfopen( severity, path, type )
int	severity;
char	*path;
char	*type;
{
	register FILE	*fp;	/* file pointer */

	if( (fp = fopen( path, type )) == NULL ) {
		char	*mode;

		if( type[1] == '+' )
			mode = "updating";
		else
			switch( type[0] ) {
			case 'r':
				mode = "reading";
				break;
			case 'w':
				mode = "writing";
				break;
			case 'a':
				mode = "appending";
				break;
			default:
				mode = type;
			}
		_errmsg( "UXzfopen1", severity,
			"Cannot open file \"%s\" for %s.",
			 path, mode );
	}
	return  fp;
}
