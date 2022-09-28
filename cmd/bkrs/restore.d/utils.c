/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/utils.c	1.6.2.1"

#include	<string.h>
#include	<sys/types.h>
#include	<pwd.h>
#include	<table.h>

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

extern char *brcmdname;
extern uid_t getuid();
extern struct passwd *getpwuid();

static char *restorename = "restore";
static char *urestorename = "urestore";

static
char *
basename( path )
char *path;
{
	register char *ptr;
	if( !(ptr = strrchr( path, '/' )) ) 
		return( path );
	return( ptr + 1 );
}

char *
setcmdname( path )
char *path;
{
	register char *ptr;
	if( ptr = basename( path ) )
		if( !strcmp( ptr, restorename ) || !strcmp( ptr, urestorename ) )
			return( ptr );
	return( getuid()? urestorename: restorename );
}

/* Return the number of flags in 'flags'  that are set in 'flag' */
int
n_flags( flag, flags )
int flag, flags;
{
	register total = 0;
	while( flags != 0 ) {
		total += ((flag&1) & (flags&1));
		flag >>= 1;
		flags >>= 1;
	}
	return( total );
}

/* Use Effective UID to get the log name of the envoking user */
char *
rsgetlogin()
{
	struct passwd *pw;
	if( !(pw = getpwuid( getuid() ) ) )
		return( "" );
	endpwent();
	return( pw->pw_name );
}
