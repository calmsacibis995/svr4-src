/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/useradd.c	1.11.5.1"



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<limits.h>
#include	<grp.h>
#include	<string.h>
#include	<userdefs.h>
#include	"users.h"
#include	"messages.h"
#include	"userdisp.h"

/*******************************************************************************
 *  useradd [-u uid [-o] | -g group | -G group [[,group]...] | -d dir [-m]
 *		| -s shell | -c comment | -k skel_dir] ] login
 *  useradd -D [ -g group ] [ -b base_dir | -f inactive | -e expire ]
 *
 *	This command adds new user logins to the system.  Arguments are:
 *
 *	uid - an integer less than 2**16 (USHORT)
 *	group - an existing group's integer ID or char string name
 *	dir - home directory
 *	shell - a program to be used as a shell
 *	comment - any text string
 *	skel_dir - a skeleton directory
 *	base_dir - a directory
 *	login - a string of printable chars except colon (:)
 ******************************************************************************/

extern struct userdefs *getusrdef();
extern long strtol();
extern void dispusrdef(), exit();

static void cleanup();

extern char *optarg;		/* used by getopt */
extern int optind, opterr;	/* used by getopt */

extern uid_t findnextuid();
extern void errmsg();
extern int getopt(), check_perm(), valid_group(), valid_expire();
extern int putusrdef(), valid_login(), valid_uid();
extern int call_passmgmt(), edit_group(), create_home();
extern int **valid_lgroup();

static uid_t uid;			/* new uid */
static char *logname;			/* login name to add */
static struct userdefs *usrdefs;	/* defaults for useradd */

static char *nargv[20];		/* arguments for execvp of passmgmt */
static int argindex;			/* argument index into nargv */

char *cmdname = "useradd";

static char homedir[ PATH_MAX + 1 ];		/* home directory */
static char gidstring[10];		/* group id string representation */
static gid_t gid;			/* gid of new login */
static char uidstring[10];		/* user id string representation */
static char *uidstr = NULL;			/* uid from command line */
static char *base_dir = NULL;			/* base_dir from command line */
static char *group = NULL;			/* group from command line */
static char *grps = NULL;			/* multi groups from command line */
static char *dir = NULL;			/* home dir from command line */
static char *shell = NULL;			/* shell from command line */
static char *comment = NULL;			/* comment from command line */
static char *skel_dir = NULL;		/* skel dir from command line */
static long inact;			/* inactive days */
static char *inactstr = NULL;			/* inactive from command line */
static char inactstring[10];	/* inactivity string representation */
static char *expirestr = NULL;		/* expiration date from command line */

main(argc, argv)
int argc;
char *argv[];
{
	int ch, ret, mflag = 0, oflag = 0, Dflag = 0, **gidlist;
	int tries;
	char *ptr;			/* loc in a str, may be set by strtol */
	struct group *g_ptr;
	struct stat statbuf;		/* status buffer for stat */

	opterr = 0;			/* no print errors from getopt */

	while((ch = getopt(argc, argv, "b:c:Dd:e:f:G:g:k:mos:u:")) != EOF )
		switch(ch) {
			case 'b':
				base_dir = optarg;
				break;

			case 'c':
				comment = optarg;
				break;

			case 'D':
				Dflag++;
				break;

			case 'd':
				dir = optarg;
				break;

			case 'e':
				expirestr = optarg;
				break;

			case 'f':
				inactstr = optarg;
				break;

			case 'G':
				grps = optarg;
				break;

			case 'g':
				group = optarg;
				break;

			case 'k':
				skel_dir = optarg;
				break;

			case 'm':
				mflag++;
				break;

			case 'o':
				oflag++;
				break;

			case 's':
				shell = optarg;
				break;

			case 'u':
				uidstr = optarg;
				break;

			case '?':
				errmsg( M_AUSAGE );
				exit( EX_SYNTAX );
		}


	/* get defaults for adding new users */
	usrdefs = getusrdef();

	if( Dflag ) {
		/* DISPLAY mode */

		/* check syntax */
		if( optind != argc ) {
			errmsg( M_AUSAGE );
			exit( EX_SYNTAX );
		}

		if( uidstr || oflag || grps || dir || mflag
				|| shell || comment || skel_dir ) {
			errmsg( M_AUSAGE );
			exit( EX_SYNTAX );
		}

		/* Group must be an existing group */
		if( group ) {
			switch( valid_group( group, &g_ptr ) ) {
			case INVALID:
				errmsg( M_INVALID, group, "group id" );
				exit( EX_BADARG );
				/*NOTREACHED*/
			case TOOBIG:
				errmsg( M_TOOBIG, "gid", group );
				exit( EX_BADARG );
				/*NOTREACHED*/
			case UNIQUE:
				errmsg( M_GRP_NOTUSED, group );
				exit( EX_NAME_NOT_EXIST );
			}

			usrdefs->defgroup = g_ptr->gr_gid;
			usrdefs->defgname = g_ptr->gr_name;

		} 

		/* base_dir must be an existing directory */
		if( base_dir ) {
			if( REL_PATH( base_dir ) ) {
				errmsg( M_RELPATH, base_dir );
				exit( EX_BADARG );
			}
			if( stat(base_dir, &statbuf) < 0
				&& ( statbuf.st_mode & S_IFMT ) != S_IFDIR ) {
				errmsg( M_INVALID, base_dir, "base directory" );
				exit( EX_BADARG );
			}

			usrdefs->defparent = base_dir;
		}

		/* inactivity period is an integer */
		if(inactstr) {
			/* convert inactstr to integer */
			inact = strtol( inactstr, &ptr, 10 );
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

		/* change defaults for useradd */
		if( putusrdef( usrdefs ) < 0 ) {
			errmsg( M_UPDATE, "created" );
			exit( EX_UPDATE );
		}

		/* Now, display */
		dispusrdef( stdout, (D_ALL & ~D_RID) );
		exit( EX_SUCCESS );

	}

	/* ADD mode */

	/* check syntax */
	if( optind != argc - 1 || base_dir || (skel_dir && !mflag) ) {
		errmsg( M_AUSAGE );
		exit( EX_SYNTAX );
	}

	logname = argv[optind];
	switch( valid_login( logname, (struct pwd **) NULL ) ) {
	case INVALID:
		errmsg( M_INVALID, logname, "login name" );
		exit( EX_BADARG );
		/*NOTREACHED*/

	case NOTUNIQUE:
		errmsg( M_USED, logname );
		exit( EX_NAME_EXISTS );
		/*NOTREACHED*/
	}

	if(uidstr) {
		/* convert uidstr to integer */
		uid = (uid_t) strtol(uidstr, &ptr, (int) 10);
		if( *ptr ) {
			errmsg( M_INVALID, uidstr, "user id" );
			exit( EX_BADARG );
		}

		switch( valid_uid( uid, NULL ) ) {
		case NOTUNIQUE:
			if( !oflag ) {
				/* override not specified */
				errmsg( M_UID_USED, uid);
				exit( EX_ID_EXISTS );
			}
			break;
		case RESERVED:
			errmsg( M_RESERVED, uid );
			break;
		case TOOBIG:
			errmsg( M_TOOBIG, "uid", uid );
			exit( EX_BADARG );
			break;
		}

	} else {

		if( (uid = findnextuid()) < 0 ) {
			errmsg( M_INVALID, "default id", "user id" );
			exit( EX_ID_EXISTS );
		}
	}

	if( group ) {
		switch( valid_group( group, &g_ptr ) ) {
		case INVALID:
			errmsg( M_INVALID, group, "group id" );
			exit( EX_BADARG );
			/*NOTREACHED*/
		case TOOBIG:
			errmsg( M_TOOBIG, "gid", group );
			exit( EX_BADARG );
			/*NOTREACHED*/
		case UNIQUE:
			errmsg( M_GRP_NOTUSED, group );
			exit( EX_NAME_NOT_EXIST );
			/*NOTREACHED*/
		}

		gid = g_ptr->gr_gid;

	} else gid = usrdefs->defgroup;

	if( grps ) {
		if( !*grps )
			/* ignore -G "" */
			grps = (char *)0;
		else if( !(gidlist = valid_lgroup( grps, gid )) )
			exit( EX_BADARG );
	}

	if( !dir ) {
		/* set homedir to home directory made from base_dir */
		(void) sprintf( homedir, "%s/%s", usrdefs->defparent, logname );

	} else if( REL_PATH(dir) ) {
		errmsg( M_RELPATH, dir );
		exit( EX_BADARG );

	} else (void) strcpy(homedir, dir);

	if( mflag ) {
		/* Does home dir. already exist? */
		if( stat(homedir, &statbuf) == 0 ) {
			/* directory exists - don't try to create */
			mflag = 0;

			if( check_perm( statbuf, uid, gid, S_IXOTH ) != 0 )
				errmsg( M_NO_PERM, logname, homedir);
		}
	}

	if( shell ) {
		if( REL_PATH( shell ) ) {
			errmsg( M_RELPATH, shell );
			exit( EX_BADARG );
		}
		/* check that shell is an executable file */
		if( stat( shell, &statbuf ) < 0
			|| (statbuf.st_mode & S_IFMT) != S_IFREG
			|| (statbuf.st_mode & 0555) != 0555 ) {

			errmsg( M_INVALID, shell, "shell" );
			exit( EX_BADARG );
		}
	} else shell = usrdefs->defshell;

	if( skel_dir ) {
		if( REL_PATH( skel_dir ) ) {
			errmsg( M_RELPATH, skel_dir );
			exit( EX_BADARG );
		}
		if( stat( skel_dir, &statbuf ) < 0
			&& (statbuf.st_mode & S_IFMT) != S_IFDIR ) {

			errmsg( M_INVALID, skel_dir, "directory" );
			exit( EX_BADARG );
		}
	} else skel_dir = usrdefs->defskel;

	if( inactstr ) {
		/* convert inactstr to integer */
		inact = strtol( inactstr, &ptr, 10 );
		if( *ptr || inact < 0 ) {
			errmsg( M_INVALID, inactstr, "inactivity period" );
			exit( EX_BADARG );
		}
	} else inact = usrdefs->definact;

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
			expirestr = (char *)0;

	} else expirestr = usrdefs->defexpire;

	/* must now call passmgmt */

	/* set up arguments to  passmgmt in nargv array */
	argindex = 0;
	nargv[argindex++] = "passmgmt";
	nargv[argindex++] = "-a";	/* add */

	if( comment ) {
		/* comment */
		nargv[argindex++] = "-c";
		nargv[argindex++] = comment;
	}

	/* flags for home directory */
	nargv[argindex++] = "-h";
	nargv[argindex++] = homedir;

	/* set gid flag */
	nargv[argindex++] = "-g";
	(void) sprintf(gidstring, "%ld", gid);
	nargv[argindex++] = gidstring;

	/* shell */
	nargv[argindex++] = "-s";
	nargv[argindex++] = shell;

	/* set inactive */
	nargv[argindex++] = "-f";
	(void) sprintf( inactstring, "%d", inact );
	nargv[argindex++] = inactstring;

	/* set expiration date */
	if( expirestr ) {
		nargv[argindex++] = "-e";
		nargv[argindex++] = expirestr;
	}

	/* set uid flag */
	nargv[argindex++] = "-u";
	(void) sprintf(uidstring, "%ld", uid);
	nargv[argindex++] = uidstring;

	if(oflag) nargv[argindex++] = "-o";
	
	/* finally - login name */
	nargv[argindex++] = logname;

	/* set the last to null */
	nargv[argindex++] = NULL;

	/* now call passmgmt */
	ret = PEX_FAILED;
	for( tries = 3; ret != PEX_SUCCESS && tries--; ) {
		switch( ret = call_passmgmt( nargv ) ) {
		case PEX_SUCCESS:
		case PEX_BUSY:
			break;

		case PEX_HOSED_FILES:
			errmsg( M_HOSED_FILES );
			exit( EX_INCONSISTENT );
			break;

		case PEX_SYNTAX:
		case PEX_BADARG:
			/* should NEVER occur that passmgmt usage is wrong */
			errmsg( M_AUSAGE );
			exit( EX_SYNTAX );
			break;

		case PEX_BADUID:
			/* uid is used - shouldn't happen but print message anyway */
			errmsg( M_UID_USED, uid );
			exit( EX_ID_EXISTS );
			break;

		case PEX_BADNAME:
			/* invalid loname */
			errmsg( M_USED, logname);
			exit( EX_NAME_EXISTS );
			break;

		default:
			errmsg( M_UPDATE, "created" );
			exit( ret );
			break;
		}
	}
	if( tries == 0 ) {
		errmsg( M_UPDATE, "created" );
		exit( ret );
	}

	/* add group entry */
	if( grps && edit_group( logname, (char *)0, gidlist, 0 ) ) {
		errmsg( M_UPDATE, "created" );
		cleanup( logname );
		exit( EX_UPDATE );
	}

	/* create home directory */
	if( mflag
		&& (create_home( homedir, skel_dir, uid, gid ) != EX_SUCCESS ) ) {
		(void) edit_group( logname, (char *)0, (int **)0, 1 );
		cleanup( logname );
		exit( EX_HOMEDIR );
	}

	exit( ret );
	/*NOTREACHED*/
}

static void
cleanup( logname )
char *logname;
{
	char *nargv[4];

	nargv[0] = "passmgmt";
	nargv[1] = "-d";
	nargv[2] = logname;
	nargv[3] = NULL;

	switch( call_passmgmt( nargv ) ) {
	case PEX_SUCCESS:
		break;

	case PEX_SYNTAX:
		/* should NEVER occur that passmgmt usage is wrong */
		errmsg( M_AUSAGE );
		break;

	case PEX_BADUID:
		/* uid is used - shouldn't happen but print message anyway */
		errmsg( M_UID_USED, uid );
		break;

	case PEX_BADNAME:
		/* invalid loname */
		errmsg( M_USED, logname );
		break;

	default:
		errmsg( M_UPDATE, "created" );
		break;
	}
}
