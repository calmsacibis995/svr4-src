/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/expdate.c	1.2.7.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<userdefs.h>
#include	<users.h>

extern void exit();
extern int valid_expire();

/* Validate an expiration date */
main( argc, argv )
char *argv[];
{
	if( argc != 2 ) {
		(void) fprintf( stderr, "synopsis: expdate date\n" );
		exit( EX_SYNTAX );
	}
	exit( valid_expire( argv[1], 0 ) == INVALID?
		EX_FAILURE: EX_SUCCESS );
	/*NOTREACHED*/
}
