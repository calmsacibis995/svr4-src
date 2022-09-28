/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rslib.d/rstest.c	1.2.2.1"

#include	<time.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<table.h>
#include	<errno.h>
#include	<bkrs.h>
#include	<restore.h>
#include	<bkhist.h>

#define	SKIPWS(p)	while( p && *p && isspace( *p ) ) p++

#define	Sflag	0x1
#define Fflag	0x2

extern char *bk_get_histlog_path();

int echo = 0;
FILE *sfptr;

/* print out the list of candidate archives for a particular request */
main( argc, argv )
char *argv[];
{
	int c, tid, rc, change = 1, succeeded = 1, flag = 0;
	ENTRY entry;
	TLdesc_t descr;
	char *path, *ptr, buffer[ 80 ], *myfgets();
	rs_rqst_t rqst;
	extern char *optarg;
	extern int optind;

	strncpy( &rqst, "", sizeof( rs_rqst_t ) );

	while( (c = getopt( argc, argv, "s:eSF") ) != -1 )
		switch( c ) {
		case 'e':	/* echo input to output */
			echo = 1;
			break;
		case 'F':	/* Fail ALL archives */
			flag |= Fflag;
			break;
		case 'S':	/* Succeed ALL archives */
			flag |= Sflag;
			break;
		case 's':	/* Tee input into optarg */
			if( !(sfptr = fopen( optarg, "w" ) ) )
				fprintf( stderr, "%s: cannot open %s for writing, errno %ld\n",
					argv[0], optarg, errno );
			break;
		default:
			fprintf( stderr, "synopsis: %s [-s shadowfile ]\n", argv[0] );
			break;
		}


	if( argc != optind + 4 ) {
		fprintf( stdout, "synopsis: oname odevice type date(in hex)\n" );
		exit( 2 );
	}

	rqst.oname = argv[ optind ];
	rqst.odev = argv[ optind + 1 ];
	rqst.type = argv[ optind + 2 ];
	rqst.date = strtol( argv[ optind + 3 ], (char **)0, 16 );

	brloginit( "rstest", RESTORE_T );
	rstm_init();

	if( !(path = bk_get_histlog_path()) ) {
		fprintf( stderr, "unable to get history log path\n" );
		exit( 1 );
	}

	/* First open the file */
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = H_ENTRY_F;

	if( (rc = TLopen( &tid, path, &descr, O_RDONLY, 0644 ) )
		!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
		if( rc == TLFAILED ) 
			fprintf( stderr, "TLopen of history table %s fails: %s",
				path, brerrno( errno ) );
		else fprintf( stderr, "TLopen of history table %s returns %d",
			path, rc );
		exit( 1 );
	}

	/* Get an entry element */
	if( !(entry = TLgetentry( tid )) ) {
		fprintf( stderr, "unable to get memory for history table entry\n" );
		TLclose( tid );
		exit( 1 );
	}

	if( flag & Fflag )
		succeeded = 0;

	while( rstm_move( tid, entry, &rqst, succeeded ) ) {
		fprintf( stdout, "Next candidate:\n" );
		fprintf( stdout, "\toname: %s odevice: %s\n",
			TLgetfield( tid, entry, H_ONAME ),
			TLgetfield( tid, entry, H_ODEVICE ) );
		fprintf( stdout, "\tmethod: %s date: %s\n",
			TLgetfield( tid, entry, H_METHOD ),
			TLgetfield( tid, entry, H_DATE ) );
		if( !flag ) {
			fprintf( stdout, "Enter 's' for success or 'f' for fail: " );
			if( !(ptr = myfgets( buffer, 80, stdin ) ) )
				break;
			SKIPWS(ptr);
			succeeded = (*ptr != 'f');

			/*
			fprintf( stdout, "Enter 'c' for change state, CR for same state: " );
			if( !(ptr = myfgets( buffer, 80, stdin ) ) )
				break;
			SKIPWS(ptr);
			change = (*ptr == 'c');
			*/
		}
	}

	exit( 0 );
}

char *
myfgets( buffer, size, fptr )
char *buffer;
int size;
FILE *fptr;
{
	register char *tmp;
	if( (tmp = fgets( buffer, size, fptr )) && sfptr )
		fprintf( sfptr, "%s", buffer );
	if( tmp && echo )
		fprintf( stdout, "%s", buffer );
	return( tmp );
}
