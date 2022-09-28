/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/rstoc.c	1.4.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkrs.h>

extern int brtype;
extern pid_t bkdaemonpid;

extern unsigned int strlen();
extern char *strcpy();
extern int bkm_send();

int
rstocname( tocname )
char *tocname;
{
	bkdata_t msg;

	switch( brtype ) {

	case RESTORE_T:

		if( !tocname ) return( BRBADARGS );

		if( strlen( tocname ) > BKFNAME_SZ )
			return( BRTOOBIG );

		(void) strcpy( msg.rst_o_c.tocname, tocname );

		if( bkm_send( bkdaemonpid, RSTOC, &msg ) == -1 )
			return( BRFATAL );
		break;

	case BACKUP_T:
		break;

	default:
		return( BRNOTINITIALIZED );
	}
	return( BRSUCCESS );
}

