/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/validweeks.c	1.4.2.1"

#define TRUE	1
#define FALSE	0

/* Program takes as arguments a string of week ranges and the rotation period */
/* and returns 0 if the weeks in the ranges are ok, 1 if there is some error. */
main( argc, argv )
int argc;
char *argv[];
{
	int ok = FALSE;
	int begin;
	int end;
	int period;
	int atoi();

	unsigned char *ptr;
	unsigned char *p_weekrange();

	void exit();

	if( argc != 3 )
		exit( 1 );

	ptr = (unsigned char *) argv[1];
	period = atoi( argv[2] );
	while( ptr = p_weekrange( ptr, &begin, &end ) ) {
		if ( (begin > period) || (end > period) ) {
			ok = FALSE;
			break;
		}
		if( !*ptr ) {
			ok = TRUE;
			break;
		}
		ptr++;
	}

	exit( !ok );
}
