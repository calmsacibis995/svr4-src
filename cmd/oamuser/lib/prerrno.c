/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/prerrno.c	1.1.9.1"



#include	<errno.h>

extern char *sys_errlist[];
extern int sys_nerr;
extern int sprintf();

char *
prerrno( L_errno )
int L_errno;
{
	static char buffer[ 30 ];
	if( L_errno < sys_nerr ) return( sys_errlist[ L_errno ] );
	(void) sprintf( buffer, "Unknown errno %d", L_errno );
	return( buffer );
}

