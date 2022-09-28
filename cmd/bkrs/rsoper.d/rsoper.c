/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/rsoper.c	1.6.2.1"

#include	<sys/types.h>
#include	<signal.h>
#include	<stdio.h>
#include	<string.h>
#include	<bkrs.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkerrors.h>
#include	<restore.h>
#include	<rsoper.h>
#include	<errors.h>

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

static char *basename();
static int getddev();
static int getodev();
static void synopsis();
static int chk_ids();

extern void brloginit();
extern int getopt();
extern int is_rsjobid();
extern void exit();
extern int validcombination();
extern int cancel();
extern void bkerror();
extern char *d_resolve();
extern int p_arch_info();
extern void get_arch_info();
extern int l_fill();
extern int l_arch_check();
extern char *d_merge();
extern int l_pre_check();
extern int d_allocate();
extern int rstm_init();
extern int rsspawn();
extern void d_free();
extern int l_toc_post_check();
extern int unlink();
extern int l_post_check();
extern char *bkstrtok();
extern char *d_reserve();
extern argv_t *s_to_argv();

char *prog = "rsoper";

#define	NFLAGS	11
int nflags = NFLAGS;

/* FLAG to option mapping */
char *optname = "cdjmnorstuv";

/* The valid combinations of flags */
int allowed[] = {
	(dFLAG|sFLAG|vFLAG|tFLAG|mFLAG|nFLAG|oFLAG|uFLAG|jFLAG),
	rFLAG,
	cFLAG,
	0
};
/* Required combinations of flags */
int required[] = {
	dFLAG,
	rFLAG,
	cFLAG,
	0
};

/* Option argument errors */
#define cERROR	0x1
#define dERROR	0x2
#define jERROR	0x8
#define mERROR	0x4
#define	oERROR	0x10
#define	rERROR	0x20
#define uERROR	0x40

int errors[] = {
	(dERROR|mERROR|oERROR|uERROR|jERROR),
	(rERROR),
	(cERROR),
	0
};

char *brcmdname;
char *ddevice;	/* what the device holds the archive */
char *dchar;	/* characteristics of that device */
char *dmnames;	/* labels of the archive */
char *machine;	/* Machine where the archive is from */
char *method = 0;	/* Method that we're using */
char *oname = 0;	/* Origination name */
char *odevice = 0;	/* Origination device */
char *jobids;
argv_t *jobs;
char *user = 0;	/* Do restore requests for a particular user */
char *label = 0;	/* label on this volume */
int flags = 0;	/* Options that have been seen */
int rsflags = 0;	/* rsspawn() flags */
char tocname[ BKFNAME_SZ ];

main( argc, argv )
char *argv[];
{
	int c, bad_args = 0, error_seen = 0, need_synopsis = FALSE;
	int toc = 0, rc;
	char *l_dchar;
	rs_entry_t *list = (rs_entry_t *)0;
	extern char *optarg;

	(void) sigset( SIGCLD, SIG_DFL );

	brcmdname = basename( argv[0] );

	/* Initialize log file */
	brloginit( brcmdname, RESTORE_T );

	while( (c = getopt( argc, argv, "c:d:j:m:no:r:stu:v?" )) != -1 )
		switch( c ) {
		case 'c':	/* Cancel a restore request */
			flags |= cFLAG;
			jobids = optarg;
			if( !is_rsjobid( jobids ) )
				bad_args |= cERROR;
			break;

		case 'd': /* ddevice:dchar:dnmnames description of the device */
			flags |= dFLAG;
			if( !getddev( optarg ) ) 
				bad_args |= dERROR;
			break;

		case 'j':	/* List of jobids */
			flags |= jFLAG;
			jobids = optarg;
			break;

		case 'm':	/* method name */
			flags |= mFLAG;
			method = optarg;
			break;

		case 'n':	/* print out archive information */
			flags |= nFLAG;
			break;

		case 'o':	/* oname:odevice - origination of archive */
			flags |= oFLAG;
			if( !getodev( optarg ) )
				bad_args |= oERROR;
			break;

		case 'r':	/* Remove a restore request */
			flags |= rFLAG;
			jobids = optarg;
			if( !is_rsjobid( jobids ) )
				error_seen = rERROR;
			break;

		case 's':	/* Display dots */
			flags |= sFLAG;
			rsflags |= RS_SFLAG;
			break;

		case 't':	/* Table of Contents volume for a method */
			flags |= tFLAG;
			rsflags |= RS_ISTOC;
			toc = TRUE;
			break;

		case 'u':	/* do restore operations for a particular user */
			flags |= uFLAG;
			user = optarg;
			break;

		case 'v':	/* Print restore objects when they are restored */
			flags |= vFLAG;
			rsflags |= RS_VFLAG;
			break;

		case '?':
		default:
			need_synopsis = TRUE;
			break;
	}

	if( need_synopsis ) {
		synopsis();
		exit( 1 );
	}

	if( error_seen ) exit( 2 );

	/* Check combinations of options */
	if( flags & rFLAG ) {

		if( !validcombination( 'r', flags, allowed[ 1 ], required[ 1 ] ) )
			exit( 1 );

		/* Remove the request */
		if( !cancel( jobids, TRUE ) ) {
			bkerror( stderr, ERROR11, "remove", jobids );
			exit( 1 );
		}

	} else if( flags & cFLAG ) {

		if( !validcombination( 'c', flags, allowed[ 2 ], required[ 2 ] ) )
			exit( 1 );

		/* Cancel the request */
		if( !cancel( jobids, FALSE ) ) {
			bkerror( stderr, ERROR11, "cancel", jobids );
			exit( 1 );
		}

	} else if( flags & dFLAG ) {

		/* Attempt to satisfy requests with this volume */
		if( !validcombination( 'd', flags, allowed[ 0 ], required[ 0 ] ) )
			exit( 1 );

		if( (flags & (sFLAG|vFLAG)) == (sFLAG|vFLAG) ) {
			bkerror( stderr, ERROR5, 'c', 's' );
			exit( 1 );
		}

		if( flags & (jFLAG) ) {
			if( !(jobs = s_to_argv( strdup( jobids ), " ," ) ) ) {
				bkerror( stderr, ERROR1, jobids );
				exit( 2 );
			}
			if( !chk_ids( jobs ) )
				exit( 2 );
		}

		if( flags & tFLAG ) {

			bkerror( stderr, ERROR16, 't' );
			exit( 2 );

		}

		/* If label names are given on the command line, use them */
		if( dmnames ) label = strtok( strdup( dmnames ), ", " );

		/*
			resolve device charactersitics from command line with those in
			the devmgt tables.
		*/
		dchar = (char *)d_resolve( ddevice, dchar );

		if( flags & nFLAG )
			/* Print out archive info */
			exit( !p_arch_info( ddevice, dchar, dmnames ) );
			
		/* perform a restore operation */
		get_arch_info( ddevice, dchar, &machine, &oname, &odevice, &method, &label,
			&toc );
#ifdef TRACE
		brlog( "main(): back from get_arch_info()" );
#endif

		/* if this is a toc, believe it. */
		if( toc ) rsflags |= RS_ISTOC;

		/* If no command line labels, use the one from the volume */
		if( !dmnames ) dmnames = label;

		if( !label ) {
			bkerror( stderr, ERROR20, ddevice );
			exit( 2 );
		}

		/*
			Make a list of all possible candidate requests that might
			be restored from this volume.
		*/
		rc = l_fill( &list, jobs, oname, odevice, method, dmnames, toc );

		if( !list ) {
			bkerror( stderr, ERROR21 );
			exit( 2 );
		}

		/* Look through the list for this archive, if found, get relevant dchars */
		if( l_arch_check( &list, &dmnames, &l_dchar, toc ) ) {
			dchar = (char *)d_merge( dchar, l_dchar );
		}

		/*
			Check that there is enough information known about this archive
			to attempt to satisfy a request. Also, prune the list so that
			all requests are consistent.
		*/
		if( l_pre_check( &list, &oname, &odevice, &method, &label, toc ) ) {
			/* Not enough info or no requests */
			bkerror( stderr, ERROR15 );
			if( !oname && !odevice ) {
				(void) fprintf( stderr, "\torigination name\n" );
				(void) fprintf( stderr, "\torigination device name\n" );
			}
			if( !method ) (void) fprintf( stderr, "\tmethod name\n" );
			if( !label ) (void) fprintf( stderr, "\tdestination volume name\n" );
			exit( 2 );
		} 

		/* Reserve the devices */
		rc = d_allocate( odevice, (argv_t *)0, &ddevice );
		switch( rc ) {
		case BKINTERNAL:
		case BKNOMEMORY:
			bkerror( stderr, ERROR24, odevice, ddevice );
			exit( 2 );
		
		case BKBUSY:
			bkerror( stderr, ERROR25, odevice, ddevice );
			exit( 2 );

		default:
			break;
		}

		rstm_init();

		/* Spawn the method */
		if( rc = rsspawn( list, oname, odevice, ddevice, dchar, dmnames, method,
			rsflags, tocname ) ) {
			switch( rc ) {
			case BKNOMEMORY:
				bkerror( stderr, ERROR22, "no memory available" );
				break;
			default:
				bkerror( stderr, ERROR19 );
				break;
			}
			/* Free up devices */
			d_free( odevice, ddevice );
			exit( 2 );
		}

		/* Free up devices */
		d_free( odevice, ddevice );

		if( (flags & tFLAG) || toc ) {
			if( l_toc_post_check( list, dmnames, tocname ) > 0 )
				rc = 2;
			else rc = 0;

			/* Remove the table of contents file */
			(void) unlink( tocname );

			exit( rc );

		} else if( l_post_check( list, dmnames ) > 0 )
			exit( 2 );

	} else {

		/* Bad combination of flags */
		bkerror( stderr, ERROR13, "drc" );
		synopsis();
		exit( 1 );

	}
	exit( 0 );
	/*NOTREACHED*/
}

static char *
basename( path )
char *path;
{
	register char *ptr;
	if( !(ptr = strrchr( path, '/' )) )
		return( path );
	return( ptr + 1 );
}

static void
synopsis()
{
	(void) fprintf( stderr,
		"rsoper -d ddevice[:[dchar][:[dmnames]]] [-n] [-s | v] [-t] [-m method]\n" );
	(void) fprintf( stderr, "\t[-u user] [-j jobids] [-o oname[:odevice]]\n" );
	(void) fprintf( stderr, "rsoper -r jobid\nrsoper -c jobid\n" );
}

/* Parse ddevice:dchar:dmnnames */
static int
getddev( ddev )
char *ddev;
{
	if( !(ddevice = bkstrtok( ddev, ":" )) ) return( 0 );
	if( !*ddevice ) ddevice = NULL;

	if( !(dchar = bkstrtok( NULL, ":" )) ) return( 1 );
	dmnames = bkstrtok( NULL, ":" );

	if( !*dchar ) dchar = NULL;
	if( !*dmnames ) dmnames = NULL;

	return( 1 );
}

/* Parse oname:odevice */
static int
getodev( odev )
char *odev;
{
	if( !(oname = bkstrtok( odev, ":" )) ) return( 0 );
	if( !*oname ) *oname = NULL;

	odevice = bkstrtok( NULL, ":" );
	if( !*odevice ) odevice = NULL;

	return( 1 );
}

/* Is each jobid in the list of the valid form */
static int
chk_ids( jobs )
argv_t *jobs;
{
	register i, rc = TRUE;

	for( i = 0; (*jobs)[i]; i++ )
		if( !is_rsjobid( (*jobs)[i] ) ) {
			rc = FALSE;
			bkerror( stderr, ERROR23, (*jobs)[i] );
		}

	return( rc );
}
