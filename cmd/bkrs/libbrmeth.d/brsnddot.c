/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brsnddot.c	1.5.2.1"

#include	<sys/types.h>
#include	<stdio.h>
#include	<time.h>
#include	<bkrs.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<errno.h>

extern int brtype;
extern pid_t bkdaemonpid;

extern int bkm_send();
extern void brlog();

int
brsnddot()
{
	switch( brtype ) {
	case BACKUP_T:
		if( bkm_send( bkdaemonpid, DOT, (char *)0 ) == -1 ) {
			brlog( "Unable to send DOT message to bkdaemon; errno %ld",
				errno );
			return( BRFAILED );
		}
		break;
	
	case RESTORE_T:
		(void) fprintf( stdout, "." );
		break;

	default:
		return( BRNOTINITIALIZED );
	}

	return( BRSUCCESS );
}
