/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brestimate.c	1.5.2.1"

#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <bkrs.h>
#include <backup.h>
#include <bkmsgs.h>

extern int brtype;
extern pid_t bkdaemonpid;

extern void brlog();
extern int bkm_send();

int
brestimate( volumes, blocks )
int volumes, blocks;
{
	bkdata_t msg;

	if( brtype != BACKUP_T ) return( BRNOTALLOWED );

	msg.estimate.volumes = volumes;
	msg.estimate.blocks = blocks;

	if( bkm_send( bkdaemonpid, ESTIMATE, &msg ) == -1 ) {
		brlog( "brestimate(): bkm_send() returns errno %ld", errno );
		switch( errno ) {
			case EFAULT:
				return( BRFAULT );
			case E2BIG:
				return( BRTOOBIG );
			case EEXIST:
			case EINVAL:
			default:
				return( BRFATAL );
		}
	}
	return( BRSUCCESS );
}
