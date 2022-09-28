/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brsndfname.c	1.5.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<stdio.h>
#include	<bkrs.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<errno.h>

extern int brtype;
extern pid_t bkdaemonpid;
    
extern unsigned int strlen();
extern char *strcpy();
extern char *strncpy();
extern int bkm_send();
extern void brlog();

int
brsndfname( fname )
char *fname;
{
	bkdata_t msg;

	switch( brtype ) {
	case BACKUP_T:
		if( strlen( fname ) < BKTEXT_SZ ) 
			(void) strcpy( msg.text.text, fname );
		else {
			(void) strncpy( msg.text.text, fname, BKTEXT_SZ - 1 );
			(msg.text.text)[ BKTEXT_SZ - 1 ] = '\0';
		}

		if( bkm_send( bkdaemonpid, TEXT, &msg ) == -1 ) {
			brlog(
				"brsndfname(): Unable to send TEXT message to bkdaemon; errno %ld",
				errno );
			return( BRFAILED );
		}
		break;
	
	case RESTORE_T:
		(void) fprintf( stdout, "%s\n", fname );
		break;

	default:
		return( BRNOTINITIALIZED );
	}

	return( BRSUCCESS );
}

