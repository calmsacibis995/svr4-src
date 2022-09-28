/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/vgid.c	1.4.7.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<grp.h>
#include	<users.h>
#include	<sys/param.h>
#include	<userdefs.h>

/*
	MAXUID should be in param.h; if it isn't,
	try for UID_MAX in limits.h
*/
#ifndef	MAXUID
#include	<limits.h>
#define	MAXUID	UID_MAX
#endif

struct group *getgrgid();

/*  validate a GID */
int
valid_gid( gid, gptr )
gid_t gid;
struct group **gptr;
{
	register struct group *t_gptr;

	if( gid < 0 ) return( INVALID );

	if( gid > MAXUID ) return( TOOBIG );

	if( t_gptr = getgrgid( gid ) ) {
		if( gptr ) *gptr = t_gptr;
		return( NOTUNIQUE );
	}

	if( gid <= DEFGID ) {
		if( gptr ) *gptr = getgrgid( gid );
		return( RESERVED );
	}

	return( UNIQUE );
}
