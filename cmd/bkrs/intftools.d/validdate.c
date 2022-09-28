/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/validdate.c	1.2.2.1"

#include <string.h>
#include <sys/types.h>
#include <stdio.h>

void exit();

/* Program validates that input string represents a legitimate date. */
/* It does this by invoking getdate(3C) via brgetdate(). */

main( argc, argv )
int argc;
char *argv[];
{
	time_t brgetdate();

	if ( argc != 2 ) {
		fprintf( stderr, "%s: expects exactly one argument.\n", argv[0] );
		exit( 1 );
	}

	if ( brgetdate( argv[1] ) == 0 )
		exit( 1 );
	else exit( 0 );
}

