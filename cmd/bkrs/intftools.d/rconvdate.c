/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/rconvdate.c	1.1.2.1"

#include <string.h>
#include <sys/types.h>
#include <stdio.h>

char *month[] = {
	"invalid",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};
/* Program accepts on argument in the form mm/dd/yy and converts it to the */
/* form acceptable to the restore program: currently "mon dd,yy" */

main( argc, argv )
int argc;
char *argv[];
{
	int mindex;

	if ( argc != 2 ) {
		fprintf( stderr, "%s: expects exactly one argument.\n", argv[0] );
		exit( 1 );
	}

	mindex = atoi( strtok( argv[1], "/" ) );
	if ( mindex < 1 || mindex > 12 ) {
		fprintf( stderr, "%s: month %d out of range.\n", argv[0], mindex );
		exit( 1 );
	}
	fprintf( stdout, "%s %s, %s\n", month[mindex], strtok(NULL, "/"),
		strtok(NULL, "/") );
	exit( 0 );
}
