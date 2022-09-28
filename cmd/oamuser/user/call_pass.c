/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/call_pass.c	1.3.7.1"



#include <sys/types.h>
#include <stdio.h>
#include <userdefs.h>

extern pid_t fork(), wait();
extern int execvp();
extern void exit();

int
call_passmgmt( nargv )
char *nargv[];
{
	int ret;

	switch( fork() ) {
	case 0:
		/* CHILD */
		if( freopen("/dev/null", "w+", stdout ) == NULL
			|| freopen("/dev/null", "w+", stderr ) == NULL
			|| execvp( nargv[0], nargv ) == -1 )
			exit( EX_FAILURE );

		break;

	case -1:
		/* ERROR */
		return( EX_FAILURE );

	default:
		/* PARENT */	
			
		if( wait(&ret) == -1 )
			return( EX_FAILURE );

		ret = ( ret >> 8 ) & 0xff;
	}
	return(ret);
		
}
