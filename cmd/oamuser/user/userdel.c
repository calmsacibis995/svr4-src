/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/userdel.c	1.6.5.1"



#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <userdefs.h>
#include <errno.h>
#include "users.h"
#include "messages.h"

/*******************************************************************************
 *  userdel [-r] login
 *
 *	This command deletes user logins.  Arguments are:
 *
 *	-r - when given, this option removes home directory & its contents
 *
 *	login - a string of printable chars except colon (:)
 ******************************************************************************/

extern void errmsg(), exit();
extern char *prerrno();
extern int getopt(), check_perm(), isbusy();
extern int rm_files(), call_passmgmt(), edit_group();

extern char *optarg;		/* used by getopt */
extern int optind, opterr;	/* used by getopt */

struct passwd *getpwnam();

static char *logname;			/* login name to delete */
static char *nargv[20];		/* arguments for execvp of passmgmt */

char *cmdname = "userdel";

main(argc, argv)
int argc;
char **argv;
{
	int ch, ret = 0, rflag = 0, argindex, tries;
	struct passwd *pstruct;
	struct stat statbuf;

	opterr = 0;			/* no print errors from getopt */

	while( (ch = getopt(argc, argv, "r")) != EOF ) {
		switch(ch) {
			case 'r':
				rflag++;
				break;
			case '?':
				errmsg( M_DUSAGE );
				exit( EX_SYNTAX );
		}
	}

	if( optind != argc - 1 ) {
		errmsg( M_DUSAGE );
		exit( EX_SYNTAX );
	}

	logname = argv[optind];

	if( (pstruct = getpwnam(logname)) == NULL ) {
		errmsg( M_EXIST, logname );
		exit( EX_NAME_NOT_EXIST );
	}

	if( isbusy(logname) ) {
		errmsg( M_BUSY, logname, "remove" );
		exit( EX_BUSY );
	}

	/* that's it for validations - now do the work */
	/* set up arguments to  passmgmt in nargv array */
	nargv[0] = "passmgmt";
	nargv[1] = "-d";	/* delete */
	argindex = 2;		/* next argument */

	/* finally - login name */
	nargv[argindex++] = logname;

	/* set the last to null */
	nargv[argindex++] = NULL;

	/* remove home directory */
	if( rflag ) {
		/* Check Permissions */
		if( stat( pstruct->pw_dir, &statbuf ) ) {
			errmsg( M_OOPS, "find status about home directory", 
				prerrno( errno ) );
			exit( EX_HOMEDIR );
		}
			
		if( check_perm( statbuf, pstruct->pw_uid, pstruct->pw_gid,
		    S_IWOTH|S_IXOTH ) != 0 ) {
			errmsg( M_NO_PERM, logname, pstruct->pw_dir );
			exit( EX_HOMEDIR );
		}

		if( rm_files(pstruct->pw_dir, logname) != EX_SUCCESS ) 
			exit( EX_HOMEDIR );
	}

	/* now call passmgmt */
	ret = PEX_FAILED;
	for( tries = 3; ret != PEX_SUCCESS && tries--; ) {
		switch( ret = call_passmgmt( nargv ) ) {
		case PEX_SUCCESS:
			ret = edit_group( logname, (char *)0, (int **)0, 1 );
			if( ret != EX_SUCCESS )
				errmsg( M_UPDATE, "deleted" );
			break;

		case PEX_BUSY:
			break;

		case PEX_HOSED_FILES:
			errmsg( M_HOSED_FILES );
			exit( EX_INCONSISTENT );
			break;

		case PEX_SYNTAX:
		case PEX_BADARG:
			/* should NEVER occur that passmgmt usage is wrong */
			errmsg( M_DUSAGE );
			exit( EX_SYNTAX );
			break;

		case PEX_BADUID:
			/* uid is used - shouldn't happen but print message anyway */
			errmsg( M_UID_USED, pstruct->pw_uid );
			exit( EX_ID_EXISTS );
			break;

		case PEX_BADNAME:
			/* invalid loname */
			errmsg( M_USED, logname);
			exit( EX_NAME_EXISTS );
			break;

		default:
			errmsg( M_UPDATE, "deleted" );
			exit( ret );
			break;
		}
	}
	if( tries == 0 ) 
		errmsg( M_UPDATE, "deleted" );

	exit( ret );
	/*NOTREACHED*/
}
