/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/get_ngm.c	1.2.7.1"



#include	<sys/param.h>

static int ngm = -1;

extern int getkval();

/*
	read the value of NGROUPS_MAX from the kernel 
*/
int
get_ngm()
{
	if( ngm != -1 ) return( ngm );

	if( !getkval( "ngroups_max", sizeof( int ), (char *)&ngm ) ) {
		/* No ngroups_max available - use maximum value */
		ngm = NGROUPS_UMAX;
	}

	return( ngm );
}
