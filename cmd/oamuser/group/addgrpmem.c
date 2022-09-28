/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:group/addgrpmem.c	1.6.6.1"



#include	<sys/types.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<limits.h>
#include	<grp.h>
#include	<pwd.h>
#include	<unistd.h>
#include	<errno.h>
#include	"users.h"
#include	<userdefs.h>

extern char *optarg;
extern int optind;

extern pid_t	getpid();
extern int getopt(), unlink(), copyfile();
extern void exit();

struct group *getgrnam();
struct group *getgrgid();
struct group *fgetgrent();
struct passwd *getpwnam();
void putgrent();

static char *usage = "usage: addgrpmem -g group login [login ...]\n";

/*******************************************************************************
 *  addgrpmem -g group login [login ...]
 *
 *	This command adds logins to the supplementary membership for
 *	the group given.
 *
 *	group - a string of printable characters excluding colon(:) and less
 *		than MAXGLEN characters long.
 ******************************************************************************/

main(argc, argv)
int argc;
char *argv[];
{
	register gid_t gid;
	register i, modified = 0, ch;
	char *grpname = NULL, tname[ 20 ], **memptr;
	FILE *e_fptr, *t_fptr;
	struct group *grpstruct;	/* group structure from fgetgrent */

	while( (ch = getopt(argc, argv, "g:")) != EOF )
		switch(ch) {
			case 'g':
				grpname = optarg;
				break;
			case '?':
				(void) printf( usage );
				exit( EX_SYNTAX );
		}

	if( !grpname || argc < 4 ) {
		(void) fprintf( stdout, usage );
		exit( EX_SYNTAX );
	}

	if( (grpstruct = getgrnam(grpname)) == NULL ) {
		(void) fprintf( stdout, "invalid group: %s\n", grpname );
		exit( EX_NAME_EXISTS );
	}

	gid = grpstruct->gr_gid;

	if( (e_fptr = fopen( GROUP, "r" )) == NULL ) {
		(void) fprintf( stdout, "unable to open /etc/group\n" );
		exit( EX_UPDATE );
	}

	(void) sprintf( tname, "/tmp/%ld", getpid() );

	if( (t_fptr = fopen( tname, "w+" )) == NULL ) {
	(void) 	printf( "unable to open tmp file needed to modify /etc/group\n" );
		exit( EX_UPDATE );
	}

	/* Look for groups matching this gid */
	errno = 0;
	while( (grpstruct = fgetgrent( e_fptr ) ) != NULL) {
		
		if( grpstruct->gr_gid == gid ) {

			/* now go through & add logins from command line */

			memptr = grpstruct->gr_mem;
			for( i = optind; i < argc; i++ ) {

				if( getpwnam(argv[i]) != NULL ) {
					/* valid login */
					*memptr++ = argv[i];
					modified++;

				} else (void) fprintf( stdout, "%s is not a valid login\n",
					argv[i] );
			}

			*memptr = NULL;

		}
		putgrent( grpstruct, t_fptr );

	}

	(void) fclose( e_fptr );
	(void) fclose( t_fptr );

	if( errno == EINVAL ) {
		(void) unlink( tname );
		(void) fprintf( stdout,
			"/etc/group contains bad entries -- it was not modified.\n" );
		exit( EX_UPDATE );
	}

	/* Update GROUP file, if needed */
	if( modified && copyfile( tname, GROUP ) < 0 ) {
		(void) unlink( tname );
		exit( EX_UPDATE );
	}

	(void) unlink( tname );

	exit( EX_SUCCESS );
	/*NOTREACHED*/
}
