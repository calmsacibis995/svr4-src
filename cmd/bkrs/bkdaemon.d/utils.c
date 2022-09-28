/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/utils.c	1.6.2.1"

#include <sys/types.h>
#include <string.h>
#include <backup.h>
#include <bkmsgs.h>
#include <bkdaemon.h>

extern int sprintf();

char *
typename( type )
int type;
{
	switch( type ) {
	case OWNER_P:	return( "OWNER" );
	case CONTROL_P:	return( "CONTROL" );
	case METHOD_P:	return( "METHOD" );
	case OPERATOR_P:	return( "OPERATOR" );
	default:	return( "UNKNOWN" );
	}
}

char *
msgname( msg )
int msg;
{
	static char buffer[ 25 ];
	switch( msg ) {
	case START:	return( "START" );
	case DOT:	return( "DOT" );
	case ESTIMATE:	return( "ESTIMATE" );
	case FAILED:	return( "FAILED" );
	case DONE:	return( "DONE" );
	case GET_VOLUME:	return( "GET_VOLUME" );
	case HISTORY:	return( "HISTORY" );
	case INVL_LBLS:	return( "INV_LBL" );
	case VOLUME:	return( "VOLUME" );
	case DISCONNECTED:	return( "DISCONNECTED" );
	case SUSPEND:	return( "SUSPEND" );
	case RESUME:	return( "RESUME" );
	case CANCEL:	return( "CANCEL" );
	case TEXT:	return( "TEXT" );
	case SUSPENDED:	return( "SUSPENDED" );
	case RESUMED:	return( "RESUMED" );
	case CANCELED:	return( "CANCELED" );
	default: 
		(void) sprintf( buffer, "UNKNOWN( %d )", msg );
		return( buffer );
	}
}
