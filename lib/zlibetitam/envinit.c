/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:envinit.c	1.1"
/*******************************************************************
 *                                                                 *
 * name: envinit()                                                 *
 *                                                                 *
 * function:  initialize environment variables that used to be     *
 *            hard coded path names in the original TAM files:     *
 *                                                                 *
 *               path.h                                            *
 *                                                                 *
 *******************************************************************/

#include "path.h"
#include <string.h>

char	*KMAPDIR, *TAMHELP;

extern char *getenv(), *malloc();

static char *makepath( p, q )
char	*p,*q;
{
	char	*s;

	if( !(s = malloc((unsigned)(strlen(p) + strlen(q) + 1)))) {
		return (char*)0;
	}

	(void)strcpy( s, p );
	(void)strcat( s, q );
	return s;
}

void
_envinit()
{
	KMAPDIR		= makepath( TAMLIB, "/" );
	TAMHELP		= makepath( TAMLIB, "/tamhelp" );
}
