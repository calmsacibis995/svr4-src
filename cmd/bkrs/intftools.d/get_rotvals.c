/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/get_rotvals.c	1.3.2.1"

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <table.h>
#include <bkreg.h>
#include <rot_errs.h>
#include <bkerrors.h>

#define TRUE	1
#define FALSE	0

/* command name */
char *brcmdname;

/* Current rotations period in the file */
int curr_period;

/* Current week/day in the period */
int curr_week;
int curr_day;

/* Name of file containing bkreg table */
char *table;

/* Table id */
int tid;

/* Table search criteria */
struct TLsearch TLsearches[ TL_MAXFIELDS ];

void exit();
void bkerror();

/* Open table file whose name is only argument, search file for ROTATION and */
/* ROTATION STARTED comments, print rotation period and current week of rotation */
/* on standard output. */ 
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	int error_seen = FALSE;
	int rc;
	extern int optind;
	int getopt();
	int get_period();
	int get_rotate_start();
	int tblopen();
	int f_exists();

	void synopsis();

	extern char *optarg;

	brcmdname = argv[0];

	while ( (c = getopt( argc, argv, "t:?")) != -1 )
		switch ( c ) {
		case 't':
			/* Bkreg table name */
			table = optarg;
			break;

		case '?':
			synopsis();
			exit( 0 );
			/* NOTREACHED */
			break;

		default:
			bkerror( stderr, ERROR0, c );
			error_seen = TRUE;
			break;
		}

	if ( error_seen )
		exit( 1 );
	if ( optind != argc && !error_seen ) {
		for( ; optind < argc; optind++ )
			bkerror( stderr, ERROR1, argv[ optind ] );
		exit( 1 );
	}

	if ( !f_exists( table ) ) {
		fprintf(stdout, "Period=1:Cweek=1\n", curr_period, curr_week);
		exit( 0 );
	}

	if ( !tblopen( table, O_RDONLY ) )
		exit( 1 );

	if ( (rc = get_period( tid, &curr_period )) != 0 ) {
		switch( rc ) {
			case BKNOMEMORY:bkerror( stderr, ERROR3 );
					break;
			case BKBADREAD:	bkerror( stderr, ERROR4, "read", table );
					break;
			case BKBADFIELD:
			case BKBADVALUE:bkerror( stderr, ERROR4, "find", table );
					break;
			default:	bkerror( stderr, ERROR6, rc, "get_period" );
					break;
		}
		exit( 1 );
	}
	if ( (rc = get_rotate_start( tid, curr_period, &curr_week, &curr_day)) != 0 )
		if( (rc == BKNORSMSG) && (curr_period == 1) )
			curr_week = 1;
		else {
			switch( rc ) {
				case BKNOMEMORY:bkerror( stderr, ERROR3 );
						break;
				case BKBADREAD:	bkerror( stderr, ERROR5, "read",
							table );
						break;
				case BKBADFIELD:
				case BKBADVALUE:bkerror( stderr, ERROR5, "find",
							table );
						break;
				default:	bkerror( stderr, ERROR6, rc,
							"get_rotate_start" );
						break;
			}
			exit( 1 );
		}

	fprintf(stdout, "Period=%d:Cweek=%d\n", curr_period, curr_week);
	exit( 0 );
}

/* Returns TRUE if file exists and has data in it.  A zero-length file */
/* should be processed as though it is new. */
int
f_exists( filename )
char *filename;
{
	int stat();
	int rc;
	struct stat buf;

	if ( ((rc = stat( filename, &buf )) == 0) && (buf.st_size > 0) )
			return( TRUE );
	
	return( FALSE );
}


/* Open the bkreg table */
int
tblopen( table, mode )
char *table;
int mode;
{
	register rc;

	if( (rc = TLopen( &tid, table, NULL, mode, 0777 ) ) != TLOK ) {
		if( rc == TLFAILED ) perror( brcmdname );
		else bkerror( stderr, ERROR2, table, rc );
		return( FALSE );
	}

	return( TRUE );
}

/* Usage message */
void
synopsis()
{
	fprintf( stdout, "Usage: %s -t table\n", brcmdname );
}
