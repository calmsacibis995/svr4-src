/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:group/groupadd.c	1.4.6.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<limits.h>
#include	<userdefs.h>
#include	<users.h>
#include	"messages.h"

extern char *optarg;		/* used by getopt */
extern int optind, opterr;	/* used by getopt */

extern int getopt(), errmsg(), valid_gname();
extern gid_t findnextgid();
extern int valid_gid(), add_group();
extern long strtol();
extern void exit();

/*******************************************************************************
 *  groupadd [-g gid [-o]] group
 *
 *	This command adds new groups to the system.  Arguments are:
 *
 *	gid - a gid_t less than MAXUID
 *	group - a string of printable characters excluding colon(:) and less
 *		than MAXGLEN characters long.
 ******************************************************************************/

char *cmdname = "groupadd";

main(argc, argv)
int argc;
char *argv[];
{
	int ch;				/* return from getopt */
	gid_t gid;			/* group id */
	int oflag = 0;	/* flags */
	register rc;
	char *gidstr = NULL;	/* gid from command line */
	char *grpname;			/* group name from command line */

	while((ch = getopt(argc, argv, "g:o")) != EOF)
		switch(ch) {
			case 'g':
				gidstr = optarg;
				break;
			case 'o':
				oflag++;
				break;
			case '?':
				errmsg( M_AUSAGE );
				exit( EX_SYNTAX );
		}

	if( (oflag && !gidstr) || optind != argc - 1 ) {
		errmsg( M_AUSAGE );
		exit( EX_SYNTAX );
	}

	grpname = argv[optind];

	switch( valid_gname( grpname, NULL ) ) {
	case INVALID:
		errmsg( M_GRP_INVALID, grpname );
		exit( EX_BADARG );
		/*NOTREACHED*/
	case NOTUNIQUE:
		errmsg( M_GRP_USED, grpname );
		exit( EX_NAME_EXISTS );
		/*NOTREACHED*/
	}

	if( gidstr ) {
		/* Given a gid string - validate it */
		char *ptr;

		gid = (gid_t) strtol( gidstr, &ptr, 10 );

		if( *ptr ) {
			errmsg( M_GID_INVALID, gidstr );
			exit( EX_BADARG );
		}

		switch( valid_gid( gid, NULL ) ) {
		case RESERVED:
			errmsg( M_RESERVED, gid );
			break;

		case NOTUNIQUE:
			if( !oflag ) {
				errmsg( M_GRP_USED, gidstr );
				exit( EX_ID_EXISTS );
			}
			break;

		case INVALID:
			errmsg( M_GID_INVALID, gidstr );
			exit( EX_BADARG );

		case TOOBIG:
			errmsg( M_TOOBIG, gid );
			exit( EX_BADARG );

		}

	} else {

		if( (gid = findnextgid()) < 0 ) {
			errmsg( M_GID_INVALID, "default id" );
			exit( EX_ID_EXISTS );
		}

	}

	if( (rc = add_group(grpname, gid) ) != EX_SUCCESS )
		errmsg( M_UPDATE, "created" );

	exit( rc );
	/*NOTREACHED*/
}
