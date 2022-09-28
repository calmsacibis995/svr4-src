/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:group/groupmod.c	1.3.8.1"



#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <grp.h>
#include <userdefs.h>
#include <users.h>
#include "messages.h"

extern char *optarg;
extern int optind;

/*******************************************************************************
 *  groupmod -g gid [-o] | -n name group
 *
 *	This command modifies groups on the system.  Arguments are:
 *
 *	gid - a gid_t less than UID_MAX
 *	name - a string of printable characters excluding colon (:) and less
 *		than MAXGLEN characters long.
 *	group - a string of printable characters excluding colon(:) and less
 *		than MAXGLEN characters long.
 ******************************************************************************/

extern int getopt(), valid_gid(), valid_gname(), mod_group();
extern void exit(), errmsg();
extern long strtol();

struct group *getgrnam();

char *cmdname = "groupmod";

main(argc, argv)
int argc;
char *argv[];
{
	int ch;				/* return from getopt */
	gid_t gid;			/* group id */
	int oflag = 0;	/* flags */
	int valret;			/* return from valid_gid() */
	char *gidstr = NULL;			/* gid from command line */
	char *newname = NULL;			/* new group name with -n option */
	char *grpname;			/* group name from command line */

	oflag = 0;	/* flags */

	while((ch = getopt(argc, argv, "g:on:")) != EOF)  {
		switch(ch) {
			case 'g':
				gidstr = optarg;
				break;
			case 'o':
				oflag++;
				break;
			case 'n':
				newname = optarg;
				break;
			case '?':
				errmsg( M_MUSAGE );
				exit( EX_SYNTAX );
		}
	}

	if( (oflag && !gidstr) || optind != argc - 1 ) {
		errmsg( M_MUSAGE );
		exit( EX_SYNTAX );
	}

	grpname = argv[optind];
	if( getgrnam(grpname) == (struct group *)NULL) {
		errmsg( M_NO_GROUP, grpname );
		exit( EX_NAME_NOT_EXIST );
	}

	if( gidstr ) {
		/* convert gidstr to integer */
		char *ptr;

		gid = (gid_t) strtol(gidstr, &ptr, 10);

		if( *ptr ) {
			errmsg( M_GID_INVALID, gidstr );
			exit( EX_BADARG );
		}

		switch( valid_gid( gid, NULL ) ) {
		case RESERVED:
			errmsg( M_RESERVED, gid );
			exit(  EX_BADARG  );
			/*NOTREACHED*/

		case NOTUNIQUE:
			if( !oflag ) {
				errmsg( M_GRP_USED, gidstr );
				exit( EX_ID_EXISTS );
			}
			break;

		case INVALID:
			errmsg( M_GID_INVALID, gidstr );
			exit( EX_BADARG );
			/*NOTREACHED*/

		case TOOBIG:
			errmsg( M_TOOBIG, gid );
			exit( EX_BADARG );
			/*NOTREACHED*/

		}

	} else gid = -1;

	if( newname ) {
		switch( valid_gname( newname, NULL ) ) {
		case INVALID:
			errmsg( M_GRP_INVALID, newname );
			exit( EX_BADARG );
		case NOTUNIQUE:
			errmsg( M_GRP_USED, newname );
			exit( EX_NAME_EXISTS );
		}
	}

	if( (valret = mod_group(grpname, gid, newname)) != EX_SUCCESS )
		errmsg( M_UPDATE, "modified" );

	exit(valret);
	/*NOTREACHED*/
}
