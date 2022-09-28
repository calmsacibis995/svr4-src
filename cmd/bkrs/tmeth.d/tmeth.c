/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:tmeth.d/tmeth.c	1.6.2.1"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <bktypes.h>
#include <bkrs.h>
#include "method.h"

#ifndef TRUE
#define	TRUE 1
#define FALSE 0
#endif

extern int brstate;			/* libmeth state */

char *bkstrtok(), *basename(), *argv_to_s();
char *br_get_toc_path();
argv_t *s_to_argv();
void ev_dot(), ev_fname(), ev_bkdone(), ev_getvol();

char *method_name;		/* invocation command name */
ushort br_type = 0;			/* backup or restore */

long c_count = 0;			/* only do this many blocks */
char *ofsname = 0, *ofsdev = 0, *ofslab = 0, *ddev = 0;
char *dgroup = 0, *ddevice = 0, *dchar = 0, *dmnames = 0;
char *bkjobid, buffer[80], *errmsg;
argv_t *dvols;
char *opt, *c_arg, *L_arg, *s_arg;
int nvols = 1, total = 60, rc, flags = 0, nblks;
time_t endtime;

main( argc, argv )
char *argv[];
{
	register c, i, j, failed = 0;
	extern char *optarg;
	extern int optind, opterr;
	int error = 0, n_unknown = 0;
	time_t curtime;
	char *fname;

	method_name = basename( argv[0] );		/* full command name */
	
	opterr = 0;			/* we have no stderr */

	while( (c = getopt( argc, argv, "ABCc:dEFgiL:lmNoRrSs:T:tuVvx")) != -1 ) {
		switch (c) {

		case 'A':		/* automated operation */
			flags |= Aflag;
			break;

		case 'B':		/* is a backup */
			br_type |= IS_BACKUP;
			break;

		case 'C':		/* complete restore */
			br_type |= IS_RCOMP;
			break;

		case 'c':		/* blk count for data partition */
			flags |= cflag;
			c_arg = optarg;
			break;

		case 'd':		/* no history log */
			flags |= dflag;
			break;

		case 'E':		/* estimate, proceed */
			flags |= Eflag;
			break;

		case 'F':		/* is file restore */
			br_type |= IS_RFILE;
			break;

		case 'g':		/* Get volumes */
			flags |= gflag;
			break;

		case 'i':		/* exclude inode changes */
			flags |= iflag;
			break;

		case 'L':		/* number of labels to tell brhistory */
			L_arg = optarg;
			break;

		case 'l':		/* long history log */
			flags |= lflag;
			break;

		case 'm':		/* mount read only */
			flags |= mflag;
			break;

		case 'N':		/* estimate, stop */
			flags |= Nflag;
			break;

		case 'o':		/* permit override */
			flags |= oflag;
			break;

		case 'R':		/* is restore */
			br_type |= IS_RESTORE;
			break;

		case 'r':		/* include remote files */
			flags |= rflag;
			break;

		case 'S':		/* put out dots */
			flags |= Sflag;
			break;

		case 's':		/* Size */
			s_arg = optarg;
			break;

		case 'T':		/* debug pause count */
			total = atoi(optarg);
			break;

		case 't':		/* table of contents */
			flags |= tflag;
			break;

		case 'u':		/* Unmount file system before using it */
			flags |= uflag;;
			break;

		case 'V':		/* generate names */
			flags |= Vflag;
			break;

		case 'v':		/* validate */
			flags |= vflag;
			break;

		case 'x':		/* ignore exception list */
			flags |= xflag;
			break;

		case '?':		/* unknown option letter */
		default:		/* unexpected return */
			n_unknown++;
			break;
		}
	}

	/* Initialize */
	brinit( method_name,(int)(IS_B(br_type) ? BACKUP_T : RESTORE_T));

	/* Print out arguments for posterity */
	for( i = 1; i < argc; i++ )
		brlog( "argv[%d] is %s", i, argv[i] );

	if( ! (br_type & IS_BOTH) ) {
		brlog( "neither B nor R specified" );
		exit (1);
	}

	if((br_type & IS_BOTH) == IS_BOTH) {
		brlog( "both B and R specified" );
		exit (1);
	}

	if(n_unknown) {
		brlog( "unknown option specified" );
		exit( 1 );
	}

	if(IS_R(br_type)) {
		/* Do a RESTORE */
		if( ! (br_type & IS_FC) ) {
			brlog( "neither F or C flag specified for restore" );
			exit( 1 );
		}
		if((br_type & IS_FC) == IS_FC) {
			brlog( "both F and C flags specified for restore" );
			exit( 1 );
		}

		ofsname = argv[optind++];
		ofsdev = argv[optind]++;
		ddev = argv[optind++];

		if( br_type & IS_RFILE ) {
			/* Merely ask if these are supposed to succeed */
			for( i = optind; i < argc; i++ ) {

				/* Send dots */
				if( flags & Sflag ) {
					for( j = 0; j < 10; j++ ) {
						sleep( 2 );
						brsnddot();
					}
				} 

				fprintf( stdout, "%s %s: %s - succeed?\n",
					ofsname, ofsdev, argv[ i ] );

				if( gets( buffer ) && (*buffer == 'y' || *buffer == 'Y') ) {

					fname = strtok( argv[i], ":" );

					if( flags & Vflag ) {
						brsndfname( fname );
					}
					rsresult( fname, BRSUCCESS, (char *)0 );

				} else {
					failed++;
					rsresult( fname, BRUNSUCCESS, "not found" );
				}
			}
		} else {
			fprintf( stdout, "%s %s - succeed?\n", ofsname, ofsdev );
			if( gets( buffer ) && (*buffer == 'y' || *buffer == 'Y') ) 
				rsresult( argv[ optind + 2 ], BRSUCCESS, (char *)0 );
			else {
				failed++;
				rsresult( argv[ optind + 2 ], BRUNSUCCESS, "not found" );
			}
		}
	
		exit( failed? 2: 0 );

	} 

	/* Do a BACKUP */
	if( flags & tflag) {
		if(flags & (Aflag | lflag)) {
			brlog( "-t used with -A and/or -l" );
			error++;
		}
	}

	if( (flags & (lflag | dflag)) == (lflag | dflag)) {
		brlog( "both -l and -d specified" );
		error++;
	}

	if(flags & cflag) {
		c_count = strtol(c_arg, &opt, 0);
		if(c_arg == opt) {
		   brlog( "-c %s not numeric", c_arg );
		   error++;
		} else if (c_count <= 0) {
			brlog( "count <=0 specified" );
			error++;
		}
	}

	if( (flags & (Aflag | oflag)) == (Aflag | oflag)) {
		brlog( "both -A and -o specified" );
		error++;
	}

	if(error || n_unknown) exit(1);

	rc = BRSUCCESS;
	errmsg = "IT WORKED!!";

	nvols = (L_arg? atoi( L_arg ): 1 );
	nblks = nvols * 100;

	if(flags & Eflag) {		/* send fake estimate */
		c = brestimate( nvols, nblks );
	}

	if( flags & Nflag ) {
		c = brestimate( nvols, nblks );
		brreturncode( c, (s_arg? atoi( s_arg ): 100), errmsg );
		exit( 0 );
	}

	bkjobid = argv[ optind++ ];
	ofsname = argv[optind++];
	ofsdev = argv[optind++];
	ofslab = argv[optind++];
	ddev = argv[optind++];

	getddev( ddev );

	if( dmnames ) {
		dvols = s_to_argv( dmnames, "," );
		for( i = 0; (*dvols)[i] && i < nvols; i++ )
			;
		(*dvols)[i] = (char *)0;
	} 
	if( nvols > i ) nvols = i;

	time( &endtime );
	endtime += total;

	/* schedule events */
	ev_stop();
	if( flags & gflag )	{
		ev_getvol( 0 );
	}
	if( flags & Sflag ) ev_schedule( ev_dot, 0, 2 );
	else if( flags & Vflag ) ev_schedule( ev_fname, 0, 2 );
	ev_schedule( ev_bkdone, 0, total );
	ev_start();

	while( TRUE ) {
		switch (brstate) {
		case BR_PROCEED:

			brlog( "sleep for %d seconds", total );
			pause();
			time( &curtime );
			total = endtime - curtime;
			brlog( "returned from sleep: %d seconds left", total );
			break;

		case BR_SUSPEND:
			ev_stop();

			time( &curtime );
			total = endtime - curtime;

			c = brsuspend();
			if (c) {
				brlog( "brsuspend returned %d", c );
				errmsg = "brsuspend failed";
			}

			time( &curtime );
			endtime = curtime + total;
			ev_start();
			break;

		case BR_CANCEL:
			ev_stop();

			rc = brcancel();
			if( rc ) {
				brlog( "brcancel returned %d", c ); 
				errmsg = "brcancel failed";
			}
			brreturncode(BRCANCELED, 0, "CANCELLED");
			exit( 1 );
			break;

		}
	}
}

char *
basename( path )
char *path;
{
	register char *ptr;
	if( !(ptr = strrchr( path, '/' )) ) 
		return( path );
	return( ptr + 1 );
}

/* Parse dgroup:ddevice:dchar:dmnnames */
getddev( ddev )
char *ddev;
{
	dgroup = bkstrtok( ddev, ":" );
	if( !*dgroup ) dgroup = NULL;

	ddevice = bkstrtok( NULL, ":" );
	if( !*ddevice ) ddevice = NULL;

	if( dchar = bkstrtok( NULL, ":" ) )
		dmnames = bkstrtok( NULL, ":" );

	if( !*dchar ) dchar = NULL;
	if( !*dmnames ) dmnames = NULL;

}

/* do a senddot and re-schedule */
void
ev_dot()
{
	switch( rc = brsnddot() ) {
	case BRSUCCESS:
		if( !(ev_schedule( ev_dot, 0, 2 ) ) )
			brlog( "ev_dot(): unable to reschedule ev_dot" );
		break;

	default:
		brlog( "ev_dot(): brsnddot() returned %d", rc );
		ev_stop();
		break;
	}
}

/* do a senddot and re-schedule */
void
ev_fname( count )
long count;
{
	char buffer[ 80 ];

	sprintf( buffer, "file%d", count );

	switch( rc = brsndfname( buffer ) ) {

	case BRSUCCESS:
		if( !(ev_schedule( ev_fname, count++, 2 ) ) )
			brlog( "ev_fname(): unable to reschedule ev_fname" );
		break;

	default:
		brlog( "ev_fname(): brsndfname() returned %d", rc );
		rc = BRCANCELED;
		errmsg = "brsndfname failed";
		ev_bkdone();
		break;
	}
}

void
ev_bkdone()
{
	time_t curtime;

	if( rc == BRSUCCESS ) {
		time( &curtime );
		/*
			if time is not up yet, reschedule. This can happen when
			them method is SUSPENDED.
		*/
		if( brstate == BR_SUSPEND ) {
			/*
				If we get here then the suspend has not been seen,
				reschedule to see it later 
			*/
			ev_schedule( ev_bkdone, 0, 2 );
			return;
		}
		if( endtime > curtime ) {
			ev_schedule( ev_bkdone, 0, endtime - curtime );
			return;
		}
		brlog( "returned from sleep: 0 seconds left" );
		if( !(flags & dflag) ) {
			brlog( "call brhistory()" );
			brhistory( ofsname, ofsdev, curtime, (s_arg? atoi( s_arg ): 100),
				(dmnames? dvols: (char *(*)[])0), nvols,
				((flags & tflag)? BR_ARCHIVE_TOC: 0),
				((flags & lflag)? br_get_toc_path( bkjobid ): (char *)0) );
		}
	}

	brreturncode( rc, (s_arg? atoi( s_arg ): 100), errmsg );

	exit( rc? 1: 0 );
}

void
ev_getvol( thisvol )
long thisvol;
{
	if( flags & gflag ) {
		if( (*dvols)[thisvol] ) {

			brlog( "getvolume %s", (*dvols)[thisvol] );
			if( (rc = brinvlbl( (*dvols)[thisvol] ) ) != BRSUCCESS )
				brlog( "ev_getvol(): brinvlbl() returns %d" );

			rc = brgetvolume( (*dvols)[thisvol],
				(flags & oflag), (flags & Aflag),  buffer );
			if( rc != BRSUCCESS ) {
				brlog( "brgetvolume() returns %d", rc );

				rc = BRCANCELED;
				errmsg = "CANCELLED";

				ev_bkdone();
			}
			brlog( "brgetvolume() returns volume %s", buffer );

			if( thisvol < nvols )
				ev_schedule( ev_getvol, thisvol++, total/nvols );
		} 
	}
}
