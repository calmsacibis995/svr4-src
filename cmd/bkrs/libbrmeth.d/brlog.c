/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brlog.c	1.5.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<stdio.h>
#include	<varargs.h>
#include	<backup.h>

static	char *cmdname;
static	int	mytype;
static FILE	*logfptr = 0;
static int nolog = FALSE;
static pid_t mypid = 0;

extern char *strdup();
extern pid_t getpid();
extern char *brlogpath();

void brloginit();

/*
	brlog takes a printf-type argument list and prints it, along with
	a date/time stamp to an error log.
	brlog( fmt, arg1, arg2, ... )
*/

/*VARARGS*/
void
brlog( va_alist )
va_dcl
{
	va_list	args;
	char buf[ 25 ], *fmt;
	time_t now;

	if( !logfptr ) brloginit( "UNKNOWN", 0 );
	if( nolog ) return;

	va_start( args );
	fmt = va_arg( args, char *);

#ifndef	TRACE
	now = time( 0 );
	(void) cftime( buf, "%D %T", &now );
	(void) fprintf( logfptr, "%s ", buf );
#endif
	(void) fprintf( logfptr, "%s (%ld): ", cmdname, mypid );
	(void) vfprintf( logfptr, fmt, args );
	va_end( args );
	(void) fprintf( logfptr, "\n" );
	(void) fflush( logfptr );
}

/* brloginit initilializes the logfile pointer */
void
brloginit( commandname, type )
char *commandname;
int type;
{

	mytype = type;
	cmdname = (char *)strdup( commandname );
	if( !(logfptr = fopen( brlogpath( type ), "a" ) ) ) {
		nolog = TRUE;
		return;
	}
	mypid = getpid();
} 
