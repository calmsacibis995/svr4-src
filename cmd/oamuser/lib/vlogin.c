/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/vlogin.c	1.3.7.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<userdefs.h>
#include	<users.h>

struct passwd *getpwnam();

/*
 * validate string given as login name.
 */
int
valid_login( login, pptr )
char *login;
struct passwd **pptr;
{
	register struct passwd *t_pptr;
	register char *ptr = login;

	if( !login || !*login )
		return( INVALID );

	for( ; *ptr != NULL; ptr++ ) 
		if( !isprint(*ptr) || (*ptr == ':') )
			return( INVALID );

	if( t_pptr = getpwnam( login ) ) {
		if( pptr ) *pptr = t_pptr;
		return( NOTUNIQUE );
	}

	return( UNIQUE );
}
