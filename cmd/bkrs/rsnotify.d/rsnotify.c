/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsnotify.d/rsnotify.c	1.6.2.1"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <table.h>
#include <rsnotify.h>
#include <errors.h>

#define TRUE	1
#define FALSE	0

#define NO_OPER		(unsigned char *)"no operator"
#define TIMESIZE	11

extern struct passwd *getpwnam();

/* name of this command */
char *brcmdname;
/* Full path for rsnotify table */
unsigned char *table;

/* table id for rsnotify table */
int tid;

/* entry pointer */
ENTRY eptr;

/* entry number */
int entryno;

/* rwflag with which to open rsnotify table */
int rwflag = O_RDONLY;

/* login user typed in - to set operator name to */
char *login;

/* Time format string */
char *tfmt = "%b %d %R %Y";

/* Buffer for time conversion - must provide enough space for corresponding */
/* format (see tfmt definition) */
char tbuf[21];

void exit();
void bkerror();

/* Program to display or set the operator to be notified when a restore */
/* request needs operator intervention. */
main (argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	int c;
	int error_seen = FALSE;
	int printit = TRUE;
	int is_new;
	int entryexists;
	int findentry();
	int getopt();
	int stat();
	int tblopen();
	int valid_login();

	struct stat buf;

	void display();
	void set();
	void synopsis();

	char *bk_get_notify_path();


	brcmdname = (char *)argv[0];

	while (( c = getopt( argc, argv, "u:?" )) != -1 )
		switch ( c ) {
		case 'u':
			login = optarg;
			if ( !valid_login() ) {
				bkerror( stderr, ERROR1, login );
				error_seen = TRUE;
			}
			else {
				printit = FALSE;
				rwflag = O_RDWR;
			}
			break;

		case '?':
			synopsis();
			exit( 1 );
			/* NOTREACHED */
			break;

		default:
			bkerror( stderr, ERROR0, c );
			error_seen = TRUE;
			break;
		}

	/* Check for non-option, invalid arguments */
	if ( optind < argc ) {
		bkerror( stderr, ERROR1, argv[optind] );
		synopsis();
		exit( 1 );
	}

	if ( error_seen )
		exit( 1 );

	table = (unsigned char *)bk_get_notify_path();
	is_new = (stat( (char *)table, &buf ) != 0);
	if ( is_new && printit )
		entryexists = FALSE;
	else {
		if ( !tblopen( &tid, table, rwflag, is_new ) )
			exit( 2 );

		entryexists = findentry();
	}

	if ( printit )
		display( entryexists );
	else
		set( entryexists );

	TLclose( tid );
	exit( 0 );
}

/* Check whether login is valid.  Valid logins are: any login containing */
/* exclamation points (i.e., these are not validated), the null string or */
/* a login that appears in the local password file. */
int
valid_login()
{
	struct passwd *pwp;

	if ( *login == NULL )
		return( TRUE );

	if ( strchr( login, '!' ) != NULL )
		return( TRUE );

	pwp = getpwnam( login );
	if ( pwp == NULL )
		return( FALSE );

	return( TRUE );
}

/* Display the operator name and the date from the file.  If the operator */
/* name field is null or there is no entry in the table, display */
/* "no operator". */
void
display( exists )
int exists;
{
	unsigned char *name;
	unsigned char *tptr;

	time_t date;

	long strtol();

	if( exists ) {
		name = TLgetfield( tid, eptr, RNTFY_OPERNAME );
		if ( *name == NULL )
			name = NO_OPER;

		/* convert time_t representation of date to human-readable */
		tptr = TLgetfield( tid, eptr, RNTFY_TIME );
		if ( *tptr == NULL ) {
			tbuf[0] = '.';
			tbuf[1] = (char)NULL;
		}
		else {
			tbuf[0]=' ';
			date = (time_t) strtol( tptr, (char **)NULL, 16 );
			cftime( &tbuf[1], tfmt, &date );
		}
	}
	else {
		name = NO_OPER;
		tbuf[0] = '.';
		tbuf[1] = (char)NULL;
	}

	fprintf(stdout, "Restore request operator: %s assigned%s\n",
		name, tbuf );
}

/* Find the operator entry (first non-commentary entry in rsnotify */
/* table. */
int
findentry()
{
	int rc;

	if ( (eptr = TLgetentry( tid )) == NULL ) {
		bkerror( stderr, ERROR2, eptr );
		TLclose( tid );
		exit( 2 );
	}

	entryno = 1;
	while ( (rc = TLread( tid, entryno, eptr )) != TLBADENTRY ) {

		if ( (rc == TLBADID) || (rc == TLARGS) || (rc == TLDIFFFORMAT)
			|| (rc == TLNOMEMORY) ) {
			bkerror( stderr, ERROR3, entryno, rc );
			TLclose( tid );
			exit( 2 );
		}

		/* Ignore comment lines. */
		if ( TLgetfield( tid, eptr, TLCOMMENT ) != NULL ) {
			entryno++;
			continue;
		}

		/* Found non-commentary line */
		return( TRUE );
	}
	/* No non-commentary line in file, decrement entryno to point to last */
	/* entry in file. */
	entryno--;
	return( FALSE );
		
}

/* Set operator and time */
void
set( exists )
int exists;
{
	char tbuffer[TIMESIZE];

	int rc;

	if ( (rc = TLassign( tid, eptr, RNTFY_OPERNAME, login )) != TLOK ) {
		bkerror( stderr, ERROR4, "login", entryno, rc );
		TLclose( tid );
		exit( 2 );
	}

	(void)sprintf( tbuffer, "%#x", time( NULL ) );
	if ( (rc = TLassign( tid, eptr, RNTFY_TIME, tbuffer )) != TLOK ) {
		bkerror( stderr, ERROR4, "time", entryno, rc );
		TLclose( tid );
		exit( 2 );
	}
	if ( exists )
		TLwrite( tid, entryno, eptr );
	else
		TLappend( tid, entryno, eptr );

	TLsync( tid );
		
}

/* Synopsis of command invocation. */
void
synopsis()
{
	(void)fprintf( stderr, "%s [-u user]\n", brcmdname );
}

/* Open table. If table doesn't exist, create it for set. */
int
tblopen( tid, table, rwflag, is_new )
int *tid;
unsigned char *table;
int rwflag;
int is_new;
{
	int rc;
	int insert_format();

	struct TLdesc description;

	strncpy( (char *)&description, "", sizeof( struct TLdesc ) );
	if ( is_new )
		description.td_format = RNTFY_ENTRY_F;

	if (( rc = TLopen( tid, table, &description, rwflag, 0777 )) != TLOK
		&& rc != TLBADFS ) {
		if ( rc == TLDIFFFORMAT )
			bkerror( stderr, ERROR5, table ); /* warning */
		else {
			if ( rc == TLFAILED ) perror( brcmdname );
			else bkerror( stderr, ERROR6, table, rc );
			return ( FALSE );
		}
	}

	if ( is_new )
		(void)insert_format( *tid, RNTFY_ENTRY_F );
	return ( TRUE );
}
