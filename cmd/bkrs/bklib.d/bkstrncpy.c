/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkstrncpy.c	1.3.2.1"

#include <sys/types.h>
#include <string.h>

void
bkstrncpy( to, tosz, from, fromsz )
char *to, *from;
int tosz, fromsz;
{
	if( fromsz < tosz )
		(void) strcpy( to, from );
	else {
		(void) strncpy( to, from, tosz - 1 );
		to[ tosz - 1 ] = '\0';
	}
}

