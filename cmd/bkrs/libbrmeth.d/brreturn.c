/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brreturn.c	1.5.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkrs.h>

extern int brtype;
extern pid_t bkdaemonpid;

extern int bkm_send();
extern unsigned int strlen();
extern char *strcpy();

brreturncode( reason, nblocks, errortext )
int reason, nblocks;
char *errortext;
{
	bkdata_t msg;

	switch( brtype ) {

	case BACKUP_T:
		if( reason == BRSUCCESS ) {
			msg.done.nblocks = nblocks;
			if( bkm_send( bkdaemonpid, DONE, &msg ) == -1 )
				return( BRFATAL );
		} else {
			if( strlen( errortext ) > BKTEXT_SZ )
				return( BRTOOBIG );
			msg.failed.reason = reason;
			(void) strcpy( msg.failed.errmsg, errortext );
			if( bkm_send( bkdaemonpid, FAILED, &msg ) == -1 )
				return( BRFATAL );
		}
		break;

	case RESTORE_T:
		break;

	default:
		return( BRNOTINITIALIZED );
	}
	return( BRSUCCESS );
}
