/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkspawn.c	1.5.2.1"

#include	<sys/types.h>
#include	<fcntl.h>
#include	<varargs.h>
#include	<signal.h>
#include	<errno.h>
#include	<backup.h>
#include	<string.h>

extern int execvp();
extern int dup2();
extern pid_t setpgrp();
extern int setuid();
extern int setgid();
extern int close();
extern void exit();
extern char *brerrno();
extern pid_t fork();
extern pid_t getpid();
extern int access();
extern void brlog();

#define	MAXARGS	25

static int mydup();

/*
	bkspawn( command, stdin, stdout, stderr, uid, gid, BKARGS, arg1, arg2, ... )
	char *command, *stdin, *stdout, *stderr, *arg1, *arg2;
	int uid, gid;

			--- OR ---

	bkspawn( command, stdin, stdout, stderr, uid, gid, BKARGV, argv )
	char *command, *stdin, *stdout, *stderr, *argv[];
	int uid, gid, argc;

	bkspawn() uses exexvp();
*/

int
bkspawn( va_alist )
va_dcl
{
	va_list ap;
	pid_t pid, chpid;
	uid_t uid;
	gid_t gid;
	int argno = 0, i, type;
	char *args[ MAXARGS ], *stdin, *stdout, *stderr, **argv;

	switch( pid = fork() ) {
	case -1:
		/* ERROR */
		return( -1 );

	case 0:
		/* CHILD PROCESS */
		chpid = getpid();

		/* read arguments */
#ifdef TRACE
		brlog( "entered CHILD process %ld", chpid );
#endif
		va_start( ap );
		args[ argno++ ] = va_arg( ap, char *);
		stdin = va_arg( ap, char *);
		stdout = va_arg( ap, char *);
		stderr = va_arg( ap, char *);
		uid = va_arg( ap, uid_t );
		gid = va_arg( ap, gid_t );
		type = va_arg( ap, int );
		if( type == BKARGS ) {
			while( (args[ argno++ ] = va_arg( ap, char * ) ) != (char *)0 )
				;
			args[ argno ] = (char *)0;
		} else {
			argv = (char **)va_arg( ap, char ** );
		}
		va_end( ap );

		/* Check existence of new process executable */
		if( access( args[ 0 ], 1 ) == -1 ) {
			brlog( "CHILD pid %ld: Unable to spawn process %s: %s", chpid,
				args[ 0 ], brerrno( errno ) );
			exit( 2 );
		}

		/* Change to new uid/gid */
		if( uid && (setuid(uid) == -1) ) {
			brlog( "CHILD pid %ld: Unable to setuid() to %ld", getpid(), uid );
			exit( 2 );
		}
		if( gid && (setgid( gid ) == -1) ) {
			brlog( "CHILD pid %ld: Unable to setgid() to %ld", getpid(), gid );
			exit( 2 );
		}
		(void) setpgrp();	/* so as not to see parent's signals */

		/* Set up new file descriptors */
		for( i = 3; i < 10; i++ )
			(void)close( i );
		if( !mydup( stdin, 0 ) ) {
			brlog( "CHILD pid %ld: Unable to use %s as stdin, errno %d",
				getpid(), stdin, errno );
			exit( 2 );
		}
		if(	!mydup( stdout, 1 ) ) {
			brlog( "CHILD pid %ld: Unable to use %s as stdout, errno %d",
				getpid(), stdout,  errno );
			exit( 2 );
		}
		if(	!mydup( stderr, 2 ) ) {
			brlog( "CHILD pid %ld: Unable to use %s as stderr, errno %d",
				getpid(), stderr, errno );
			exit( 2 );
		}

		(void) sigignore( SIGUSR1 );

		/* Actually do the exec */
		if( execvp( args[ 0 ], (type == BKARGS? args: argv) ) == -1 ) {
			brlog(
				"CHILD pid %ld: Cannot spawn command: \"%s\", errno %d\n",
				getpid(), errno );
			exit( 1 );
		}
		/*NOTREACHED*/
		break;

	default:
		/* PARENT PROCESS */
		return( pid );
	}
	/*NOTREACHED*/
}

static
int
mydup( file, fildes )
char *file;
int fildes;
{
	register mode = (fildes? (O_WRONLY|O_CREAT): (O_RDONLY|O_CREAT)), fd;
	if( !file ) (void) close( fildes );
	else if( strcmp( file, "-" ) ) {
		if( (fd = open( file, mode ) ) == -1 
			|| dup2( fd, fildes ) == -1 )
			return( 0 );
		(void) close( fd );
	}
	return( 1 );
}
