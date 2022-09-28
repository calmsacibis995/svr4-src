/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/vgname.c	1.2.7.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<grp.h>
#include	<userdefs.h>
#include	<users.h>

struct group *getgrnam();

extern unsigned int strlen();

/*
 * validate string given as group name.
 */
int
valid_gname( group, gptr )
char *group;
struct group **gptr;
{
	register struct group *t_gptr;
	register char *ptr = group;

	if( !group || !*group || (int) strlen(group) >= MAXGLEN )
		return( INVALID );

	for( ; *ptr != NULL; ptr++ ) 
		if( !isprint(*ptr) || (*ptr == ':') )
			return( INVALID );

	if( t_gptr = getgrnam( group ) ) {
		if( gptr ) *gptr = t_gptr;
		return( NOTUNIQUE );
	}

	return( UNIQUE );
}
