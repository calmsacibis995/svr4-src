/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkoper.d/utils.c	1.3.2.1"

#include	<sys/types.h>
#include	<string.h>
#include	<pwd.h>

extern	char	*errmsgs[];
extern	int	lasterrmsg;
extern char *strtok();
extern struct passwd *getpwnam();

uid_t
uname_to_uid( username )
char *username;
{
	struct passwd *pw;
	if( !(pw = getpwnam( username )) ) return( -1 );
	return( pw->pw_uid );
}

char *
bkmsg( msgid )
int msgid;
{
	if( msgid < lasterrmsg ) {
		return(  errmsgs[ msgid ] );
	}
	return( "UNKNOWN ERROR" );
}
