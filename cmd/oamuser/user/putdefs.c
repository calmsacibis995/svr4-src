/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/putdefs.c	1.9.7.1"



#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <userdefs.h>
#include "users.h"
#include "messages.h"


/*******************************************************************************
 *  putusrdefs [-g group] [-b base_dir] [-k skel] [-s shell] 
		[-f inactive] [-e expire]
 ******************************************************************************/

extern int getopt(), valid_group(), valid_expire(), putusrdef();
extern void exit(), errmsg();
extern struct userdefs *getusrdef();
extern long strtol();

static struct userdefs *usrdefs;	/* defaults for useradd */

char *cmdname = "putusrdefs";
static char *usage = "usage:  putusrdefs [-g group] [-b base_dir] [-k skel_dir] [-s shell] [-f inactive] [-e expire]\n";

main(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	extern int optind;
	char *group = NULL;			/* group string from getopt */
	char *base_dir = NULL;			/* base_dir string from getopt */
	char *skel_dir = NULL;			/* skel_dir string from getopt */
	char *shellstr = NULL;			/* shell string from getopt */
	char *inactstr = NULL;			/* inactive string from getopt */
	char *expirestr = NULL;		/* expire string from getopt */
	char *ptr;			/* loc in a str, may be set by strtol */
	int ch;				/* return from getopt */
	int inact;			/* inactive */
	struct group *gptr;	/* validated group from -g */
	struct stat statbuf;		/* status buffer for stat */
	struct stat *bufptr;		/* pointer to status buffer */

	bufptr = &statbuf;

	while((ch = getopt(argc, argv, "g:b:r:k:s:f:e:")) != EOF)  {
		switch(ch) {
			case 'g':
				group = optarg;
				break;
			case 'b':
				base_dir = optarg;
				break;

			/* XXX -r is going away */
			case 'r':
				break;
			case 'k':
				skel_dir = optarg;
				break;
			case 's':
				shellstr = optarg;
				break;
			case 'f':
				inactstr = optarg;
				break;
			case 'e':
				expirestr = optarg;
				break;
			case '?':
				(void) fprintf( stdout, usage );
		}
	}

	if( optind != argc ) {
		(void) fprintf( stdout, usage );
		exit( EX_SYNTAX );
	}

	/* get default values */
	usrdefs = getusrdef();

	if( group ) {
		switch( valid_group( group, &gptr ) ) {
		case INVALID:
			errmsg( M_INVALID, group, "group id" );
			exit( EX_BADARG );
			/*NOTREACHED*/

		case TOOBIG:
			errmsg( M_TOOBIG, group );
			exit( EX_BADARG );
			/*NOTREACHED*/

		case UNIQUE:
			errmsg( M_GRP_NOTUSED, group );
			exit( EX_NAME_NOT_EXIST );
			/*NOTREACHED*/
		}

		usrdefs->defgroup = gptr->gr_gid;
		usrdefs->defgname = gptr->gr_name;

	}

	if( base_dir ) {
		if( REL_PATH( base_dir ) ) {
			errmsg( M_RELPATH, base_dir );
			exit( EX_BADARG );
		}
		if( stat( base_dir, &statbuf ) < 0
			|| (bufptr->st_mode & S_IFMT) != S_IFDIR ) {
			errmsg( M_INVALID, base_dir, "base directory" );
			exit( EX_BADARG );
		}

		usrdefs->defparent = base_dir;
		
	}

	if(skel_dir) {
		if( REL_PATH( skel_dir ) ) {
			errmsg( M_RELPATH, skel_dir );
			exit( EX_BADARG );
		}
		if( stat(skel_dir, &statbuf ) < 0
			|| (bufptr->st_mode & S_IFMT) != S_IFDIR ) {
			errmsg( M_INVALID, skel_dir, "directory" );
			exit( EX_BADARG );
		}

		usrdefs->defskel = skel_dir;
	}

	if(shellstr) {
		if( REL_PATH( shellstr ) ) {
			errmsg( M_RELPATH, shellstr );
			exit( EX_BADARG );
		}
		if( stat( shellstr, &statbuf ) < 0
			|| (bufptr->st_mode & S_IFMT) != S_IFREG
			|| (bufptr->st_mode & 0555) != 0555 ) {
			errmsg( M_INVALID, shellstr, "shell" );
			exit( EX_BADARG );
		}

		usrdefs->defshell = shellstr;

	}

	if(inactstr) {
		/* convert inactstr to integer */
		inact = strtol( inactstr, &ptr, (int) 10);
		if( *ptr || inact < 0 ) {
			errmsg( M_INVALID, inactstr, "inactivity period" );
			exit( EX_BADARG );
		}

		usrdefs->definact = inact;
	}

	/* expiration string is a date, newer than today */
	if( expirestr ) {
		if( *expirestr ) {
			if( valid_expire( expirestr, (time_t *)0 ) == INVALID ) {
			errmsg( M_INVALID, expirestr, "expiration date" );
			exit( EX_BADARG );
			}
			usrdefs->defexpire = expirestr;
		} else
			/* Unset the expiration date */
			usrdefs->defexpire = "";
	}

	if( putusrdef( usrdefs ) < 0 ) {
		errmsg( M_UPDATE, "created" );
		exit( EX_UPDATE );
	}

	exit( EX_SUCCESS );
	/*NOTREACHED*/
}
