/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/lckpwdf.c	1.2"

#ifdef __STDC__
	#pragma weak lckpwdf = _lckpwdf
	#pragma weak ulckpwdf = _ulckpwdf
#endif
#include "synonyms.h"
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#define LOCKFILE	"/etc/.pwd.lock"
#define S_WAITTIME	15

static struct flock flock =	{
			0,	/* l_type */
			0,	/* l_whence */
			0,	/* l_start */
			0,	/* l_len */
			0,	/* l_sysid */
			0	/* l_pid */
			} ;

/*	lckpwdf() returns a 0 for a successful lock within W_WAITTIME
	and -1 otherwise
*/

static int fildes = -1 ;
extern unsigned alarm() ;


static void
almhdlr(sig)
int sig;
{
}

int
lckpwdf()
{
	int retval ;
	if ( (fildes = creat (LOCKFILE, 0600)) == -1 )
		return (-1) ;
	else
		{
		flock.l_type = F_WRLCK ;
		(void) sigset (SIGALRM, almhdlr) ;
		(void) alarm (S_WAITTIME) ;
		retval = fcntl (fildes, F_SETLKW, (int)&flock) ; 
		(void) alarm (0) ;
		(void) sigset (SIGALRM, SIG_DFL) ;
		return ( retval ) ;
		}
}

/* 	ulckpwdf() returns 0 for a successful unlock and -1 otherwise
*/
int
ulckpwdf()
{
	if ( fildes == -1 ) 
		return (-1) ;
	else	{
		flock.l_type = F_UNLCK ;
		(void) fcntl (fildes, F_SETLK, (int)&flock) ;
		(void) close (fildes) ;
		fildes = -1 ;
		return (0) ;
		}
}	
