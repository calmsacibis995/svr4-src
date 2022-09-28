/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/bkid.c	1.2.2.1"

#include <stdio.h>
#define TRUE	1
#define FALSE	0

void exit();

/* Program validates a backup jobid for form and exits 0 if valid, 1 */
/* if invalid, 2 if there is an error. */
main( argc, argv )
int argc;
char *argv[];
{
	int is_bkjobid();

	if ( argc != 2 ) {
		fprintf( stderr, "Usage: %s jobid\n", argv[0] );
		exit( 2 );
	}

	if( is_bkjobid( argv[1] ) == TRUE )
		exit( 0 );
	else
		exit( 1 );
}
