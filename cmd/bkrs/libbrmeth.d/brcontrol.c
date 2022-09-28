/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brcontrol.c	1.3.2.1"

#include	<signal.h>
#include	<bkrs.h>

extern int brtype;
extern int bkcancel();
extern int bksuspend();

int
brcancel()
{
	switch( brtype ) {
	case BACKUP_T:
		return( bkcancel() );
	case RESTORE_T:
		return( BRSUCCESS );
	default:
		return( BRNOTINITIALIZED );
	}
}

int
brsuspend()
{
	switch( brtype ) {
	case BACKUP_T:
		return( bksuspend() );
	case RESTORE_T:
		return( BRSUCCESS );
	default:
		return( BRNOTINITIALIZED );
	}
}
