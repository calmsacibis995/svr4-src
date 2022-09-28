/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brinvlbls.c	1.4.2.1"

#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <bkrs.h>
#include <backup.h>
#include <bkmsgs.h>

extern int brtype;
extern pid_t bkdaemonpid;

extern char *strcpy();
extern int bkm_send();
extern void brlog();

static int fill_buffer();

int
brinvlbl( label )
char *label;
{
	bkdata_t msg;

	if( brtype != BACKUP_T )
		return( BRNOTALLOWED );

	if( !label || !*label )
		return( BRBADARGS );

	if( strlen( label ) > BKLABEL_SZ ) {
		brlog( "brinvlbl(): label name %s is too long", label );
		return( BRBADARGS );
	}

	(void) strcpy( msg.inv_lbls.label, label );

	if( bkm_send( bkdaemonpid, INVL_LBLS, &msg ) == -1 ) {
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
