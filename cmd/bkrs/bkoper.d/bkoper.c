/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkoper.d/bkoper.c	1.10.2.1"

#include	<sys/types.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<time.h>
#include	<bkrs.h>
#include	<bktypes.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkerrors.h>
#include	<bkstatus.h>
#include	<bkoper.h>
#include	<errors.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define	SKIPWS(p)	if( p ) while( *p == ' ' || *p == '\t' ) p++
#define	SEEKWS(p)	if( p ) while( *p && *p != ' ' && *p != '\t' && *p != '\n' )\ p++
#define	BSIZE	512

char *brcmdname;
char *prog;
char *bkmsg(), *basename();
int bklevels = 0;
pid_t	bkdaemonpid;

static void synopsis(), commands();

extern int getopt();
extern void exit();
extern int uname_to_uid();
extern void bkerror();
extern pid_t bkm_init();
extern void l_init();
extern int l_update();
extern int l_print();
extern int l_empty();
extern int l_tail();
extern int atoi();
extern int l_setdot();
extern int l_service();
extern int l_head();

extern bko_list_t *lhead, *ltail, *ldot;

argv_t *users = 0, *s_to_argv();
static char buffer[ BSIZE ], *ptr;

main( argc, argv )
char *argv[];
{
	extern char *optarg;
	int c;
	register bad = 0, i, number;
	char command;

	prog = brcmdname = basename( argv[0] );

	while( (c = getopt( argc, argv, "u:" )) != -1 )
		switch( c ) {
		case 'u':
			/* Do for this user only */
			users = s_to_argv( optarg, " ," );
			break;

		default:
			synopsis();
			exit( 1 );
			break;
		}

#ifdef TRACE
	brloginit( brcmdname, BACKUP_T );
#endif

	/* Validate the users */
	if( users ) 
		for( bad = 0, i = 0; (*users)[i]; i++ )
			if( !VALID_USER( (*users)[i] ) ) {
				bkerror( stderr, ERROR8, (*users)[i] );
				bad++;
			}

	if( bad > 0 ) exit( 1 );

	/* Connect with bkdaemon */
	if( (bkdaemonpid = bkm_init( BKNAME, FALSE )) == -1 ) {
		(void) fprintf( stdout, bkmsg( ERROR9 ) );
		exit( 0 );
	}

	/* Initialize list mgmt routines */
	l_init();

	/* Get first set of headers */
	if( !l_update( users ) ) {
		/* None to do */
		(void) fprintf( stdout, bkmsg( ERROR9 ) );
		exit( 0 );
	}

	/* Dot is first entry */
	(void) l_print( L_DOT, L_DOLLAR );

	(void) fprintf( stdout, bkmsg( ERROR10 ) );

	while( TRUE ) {
		if( l_update( users ) )
			(void) fprintf( stdout, bkmsg(ERROR1) );
		else if( L_EMPTY )
			(void) fprintf( stdout, bkmsg(ERROR2) );

		(void) fprintf( stdout, bkmsg(ERROR11) );

		if( fgets( buffer, BSIZE, stdin ) == buffer ) {
			ptr = buffer;
			SKIPWS(ptr);
			switch( *ptr ) {
			case '!':
				/* SHELL ESCAPE */
				ptr++;
				(void) system( ptr );
				break;

			case '?':
				/* PRINT SUMMARY OF COMMANDS */
				commands();
				break;

			case '=':
				/* PRINT CURRENT HEADER NUMBER */
				(void) fprintf( stdout, bkmsg(ERROR12), L_DOT );
				break;

			case 'P':
			case 'T':
			case 'p':
			case 't':
				/* INTERACT WITH THE N"TH HEADER */
				command = *ptr++;
				SKIPWS(ptr);
				if( isdigit( *ptr ) ) {
					/* Service a particular list entry */
					if( !(number = atoi( ptr )) ) {
						bkerror( stderr, ERROR13, command );
						break;
					}

					if( !l_setdot( number ) )
						bkerror( stderr, ERROR14, number );

				} else {
					SKIPWS(ptr);
					if( *ptr != '\n' ) {
						bkerror( stderr, ERROR13, command );
						break;
					}
				}
				(void) l_service( L_DOT );
				(void) l_print( L_DOT, L_DOT );
				break;

			case 'H':
			case 'h':
				/* PRINT ALL HEADERS */
				(void) l_print( L_HEAD, L_DOLLAR );
				break;

			case 'Q':
			case 'q':
				/* QUIT */
				if( l_update( users ) )
					(void) fprintf( stdout, bkmsg(ERROR1) );
				else {
					exit( (L_DOLLAR > 0)? 2: 0 );
					/*NOTREACHED*/
				}
				break;

			default:
				SKIPWS(ptr);
				if( isdigit( *ptr ) ) {
					if( !(number = atoi( ptr )) ) {
						bkerror( stderr, ERROR13, 'p' );
						break;
					}

					if( !l_setdot( number ) ) {
						bkerror( stderr, ERROR14, number );
						break;
					}

				} else if( *ptr != '\n' ) {
					bkerror( stderr, ERROR0 );
					break;
				}
	
				(void) l_service( L_DOT );
				(void) l_print( L_DOT, L_DOT );

			}
		} else break;
	}
}

static void
synopsis()
{
	(void) fprintf( stderr, "bkoper [-u users]\n" );
}

static void
commands()
{
	(void) fprintf( stdout,
		"!shell-command    Escape to the shell.\n" );
	(void) fprintf( stdout,
		"=                 Print the current backup operation number.\n" );
	(void) fprintf( stdout,
		"?                 Print this summary of commands.\n" );
	(void) fprintf( stdout,
		"[p|t] [n]         Interact with the backup operation\n" );
	(void) fprintf( stdout,
		"                  described by the n'th header. n\n" );
	(void) fprintf( stdout,
		"                  defaults to the current backup operation.\n" );
	(void) fprintf( stdout,
		"h                 Print the list of backup operations.\n" );
	(void) fprintf( stdout,
		"q                 Quit.\n" );
}
