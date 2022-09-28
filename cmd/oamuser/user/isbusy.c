/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/isbusy.c	1.3.4.1"



#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <utmp.h>

/* Maximium logname length - hard coded in utmp.h */
#define	MAXID	8

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

struct utmp *getutent();

/* Is this login being used */
isbusy( login )
register char *login;
{
	register struct utmp *utptr;

	while( (utptr = getutent()) != NULL )
		/*
		 * If login is in the utmp file, and that process
		 * isn't dead, then it "is_busy()"
		 */
		if( !strncmp( login, utptr->ut_user, MAXID ) && \
			utptr->ut_type != DEAD_PROCESS )
			return( TRUE );

	return( FALSE );
}
