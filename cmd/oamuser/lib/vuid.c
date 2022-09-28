/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/vuid.c	1.2.7.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<pwd.h>
#include	<userdefs.h>
#include	<users.h>

#include	<sys/param.h>

#ifndef MAXUID
#include	<limits.h>
#define	MAXUID	UID_MAX
#endif

struct passwd *getpwuid();

int
valid_uid( uid, pptr )
uid_t uid;
struct passwd **pptr;
{
	register struct passwd *t_pptr;

	if( uid <= 0 ) return( INVALID );
	if( uid <= DEFRID ) {
		if( pptr ) *pptr = getpwuid( uid );

		return( RESERVED );
	}

	if( uid > MAXUID ) return( TOOBIG );

	if( t_pptr = getpwuid( uid ) ) {
		if( pptr ) *pptr = t_pptr;
		return( NOTUNIQUE );
	}

	return( UNIQUE );
}
