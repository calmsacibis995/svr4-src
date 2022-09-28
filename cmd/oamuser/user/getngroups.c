/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/getngroups.c	1.2.7.1"



#include	<stdio.h>

extern void exit();
extern int get_ngm();

/* Print out the value of NGROUPS_MAX in the kernel */

main()
{
	(void) fprintf( stdout, "%d\n", get_ngm() );
	exit( 0 );
	/*NOTREACHED*/
}
