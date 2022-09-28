/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/restore.c	1.5.2.1"

#include	<sys/types.h>
#include	<signal.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<table.h>
#include	<errno.h>
#include	<bkrs.h>
#include	<errors.h>
#include	<restore.h>
#include	<bkhist.h>
#include	<rs.h>

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

extern char *bk_get_histlog_path();
extern char *getjobid(), *setcmdname();
extern char *f_rel_to_abs();

static void synopsis();
static int getodev();
extern int getopt();
extern time_t brgetdate();
extern uid_t getuid();
extern void exit();
extern void bkerror();
extern void brloginit();
extern int validcombination();
extern int is_rsjobid();
extern int cancel();
extern int rstm_init();
extern void brlog();
extern char *brerrno();
extern char *strncpy();
extern time_t time();
extern int view();
extern int request();
extern int strcmp();
extern void m_notify_operator();
extern pid_t getpid();
char *prog;

int nflags = NFLAGS;

int rs_flags = 0;

int current = 0;	/* Number of objects to restore */

/* FLAG to option mapping */
char *optname = "ACDFmoPvsS";

/* The valid combinations of flags */
int allowed[] = {
	(AFLAG | dFLAG | mFLAG | nFLAG | oFLAG | sFLAG | vFLAG ),
	cFLAG,
	(DFLAG | dFLAG | mFLAG | nFLAG | oFLAG | sFLAG | vFLAG ),
	(FFLAG | dFLAG | mFLAG | nFLAG | oFLAG | sFLAG | vFLAG ),
	(PFLAG | dFLAG | mFLAG | nFLAG | oFLAG | sFLAG | vFLAG ),
	(SFLAG | dFLAG | mFLAG | nFLAG | oFLAG | sFLAG | vFLAG ),
	(wFLAG | WFLAG | TFLAG | OFLAG | MFLAG ),
	(iFLAG | TFLAG | WFLAG ),
	0
};
/* Required combinations of flags */
int required[] = {
	AFLAG,
	cFLAG,
	DFLAG,
	FFLAG,
	PFLAG,
	SFLAG,
	nFLAG,
	0
};

#define	oERROR	0x1
#define	dERROR	0x2

/* Options to tell which option arguments were invalid */
#define	ALLBAD	( oERROR | dERROR )

#define OLDRESTORE 	"/usr/bin/restore"
#define SVR3RESTORE	"/usr/bin/.restore"

char *brcmdname;
char *oarg;
char *oname;
char *odevice;
char *rdate;	/* Restore to this date */
time_t rtime = 0;	/* time_t version of rdate */
int flags = 0;	/* Options that have been seen */
int h_tid = 0;	/* History table identifier */
ENTRY h_entry;	/* History table entry structure */

main( argc, argv )
char *argv[];
{
	register some = 0, i, error = 0;
	int c, bad_args = 0, error_seen = 0, need_synopsis = 0;
	char *type, *path;
	rs_rqst_t rqst;
	TLdesc_t descr;
	extern char *optarg;
	extern int	optind;
	/* svr3only flag when set, mains we've seen an option that is unique
	 * to SVR3.2.
	 * svr4only flag is set as SVR4 only options are found. This prevents 
 	 * execution of SVR3.2 restore in error case.  This would cause a 
	 * confusing usage message. 
	 * The conflict flag is set when we hit an option that is not unique
	 * to either SVR3.2 or SVR4.0.  This is resolved after all the options
	 * are processed. */
	int svr3only=0;
	int svr4only= 0;
	int conflict=0;

	(void) sigset( SIGCLD, SIG_DFL );

	if ( !strcmp(argv[0],OLDRESTORE) ) {
		runold(argc, argv, 0);
		exit(1); /* exec failed */
	}

	while( (c = getopt( argc, argv, "AcDd:FiMmnOPTtW:owvsS?" )) != -1 )
	{
#ifdef TRACE
fprintf(stderr,"3.2: %d, 4.0: %d, opt: %c\n",svr3only, svr4only, c);
#endif
		switch( c ) {
		case 'A':	/* Do a whole Disk restore */
			flags |= AFLAG;
			svr4only=1;
			break;
			
		case 'c':	/* Cancel a restore request */
			conflict=1;
			flags |= cFLAG;
			break;

		case 'd':	/* Restore from a particular date */
			/* This SVR3.2/SVR4.0 conflict must be resolved
			 * by context.  Figure it out later. */
			conflict=1;
			rdate = optarg;
			flags |= dFLAG;
			break;

		case 'D':	/* Directory restore */
			flags |= DFLAG;
			svr4only=1;
			break;

		case 'F':	/* File restore */
			flags |= FFLAG;
#ifdef TRACE
fprintf(stderr,"Fflag means 4.o only\n");
#endif
			svr4only=1;
			break;

		case 'i':
			flags |= iFLAG;
			svr3only=1;
			break;

		case 'M':
			flags |= MFLAG;
			svr3only=1;
			break;

		case 'm':	/* Send mail to user */
			flags |= mFLAG;
			svr4only=1;
			break;

		case 'n':	/* Do not restore, give visual display */
			flags |= nFLAG;
			svr4only=1;
			break;

		case 'O':	
			flags |= OFLAG;
			svr3only=1;
			break;

		case 'o':	/* output target */
			if ( optind < argc ) /*arguments follow*/
				if ( argv[optind][0] == '-' ) {
					flags |= OFLAG;
					svr3only=1;
				} else  {
					conflict++;
					flags |= oFLAG;
					oarg = argv[optind++];
				}
			else {
				flags |= OFLAG;
				svr3only=1;
			}
			break;

		case 'P':	/* Device partition restore */
			flags |= PFLAG;
			svr4only=1;
			break;

		case 's':	/* send dots */
			conflict=1;
			flags |= sFLAG;
			rs_flags |= RS_SFLAG;
			break;

		case 'S':	/* File system restore */
			flags |= SFLAG;
			svr4only=1;
			break;

		case 't':
		case 'T':
			flags |= TFLAG;
			svr3only=1;
			break;

		case 'v':	/* Send file names */
			flags |= vFLAG;
			svr4only=1;
			rs_flags |= RS_VFLAG;
			break;

		case 'W':
			flags |= WFLAG;
			svr3only=1;
			break;

		case 'w':
			flags |= wFLAG;
			svr3only=1;
			break;

		default:
#ifdef TRACE
fprintf(stderr,"default case on getopts\n");
#endif
			need_synopsis = TRUE;
			break;
		}
	}

	/* If invoked with a full path, force SVR4 style */
	if ( !strcmp(brcmdname, "/sbin/restore" ) ) {
		svr3only=0;
		svr4only=1;
	}

	/*
		If argv[0] is not recogizable as 'urestore' or 'restore', assume
		that root may do -A, -C, -P, and -S.
	*/
	if( !(brcmdname = setcmdname( argv[ 0 ] ) ) )
		brcmdname = ((flags & (AFLAG|cFLAG|PFLAG|SFLAG) ) && !getuid() )?
			"restore": "urestore";

	prog = brcmdname;
#ifdef TRACE
fprintf(stderr,"PROG: **%s**\n",prog);
#endif

	if( need_synopsis ) {
		synopsis();
		exit( 1 );
	}

#ifdef TRACE
fprintf(stderr,"check for conflicts\n");
fprintf(stderr,"A: 3.2: %d, 4.0: %d\n",svr3only, svr4only);
#endif
	/* Don't need to do anything if conflict is resolved in favor
	   of SVR4.0.  Reset flags if SVR3.2. */

	if ( conflict ) { /* If neither flag is set, assume SVR3.2 version since
		 * at least 1 arg is required in SVR4 */
		if ( (svr3only && !svr4only) || (!svr3only && !svr4only) ) {
			if ( flags & cFLAG ) {
				flags |= wFLAG;
				flags &= ~cFLAG;
			}
			if ( flags & sFLAG ) {
				flags |= MFLAG;
				flags &= ~sFLAG;
			}
			if ( flags & dFLAG ) {
				flags |= WFLAG;
				flags &= ~dFLAG;
			}
			if ( flags & oFLAG ) {
				flags |= OFLAG;
				svr3only=1;
			}
			svr3only=1; /* in case neither is set */
		} else
#ifdef TRACE
fprintf(stderr,"B: 3.2: %d, 4.0: %d\n",svr3only, svr4only);
#endif
			if ( svr3only && svr4only ) {
				bkerror(stderr,"Ambiguous combination of options.\n");
#ifdef TRACE
fprintf(stderr,"B1: 3.2: %d, 4.0: %d\n",svr3only, svr4only);
#endif
				synopsis();
				exit( 1 );
			}
	}
#ifdef TRACE
fprintf(stderr,"C: 3.2: %d, 4.0: %d\n",svr3only, svr4only);
#endif

	/* We've resolved the conflict with the dFLAG and oFLAG.  
	 * Do the SVR4 argument handling. */
	if ( svr4only && (flags & dFLAG ) ) {
		flags &= ~dFLAG;
		if( !(rtime = brgetdate( rdate ) ) )
			bad_args |= dFLAG;
	}
	if ( svr4only && (flags & oFLAG) )
		if( !getodev( oarg ) )
			bad_args |= oERROR;

	if( bad_args == ALLBAD ) {
		bkerror( stderr, ERROR3 );
		exit( 1 );

	} else if( bad_args ) {
		if( bad_args & dERROR ) bkerror( stderr, ERROR4, rdate, 'd' );
		if( bad_args & oERROR ) bkerror( stderr, ERROR4, oarg, 'o' );
		exit( 1 );

	} else if( error_seen ) exit( 1 );

	/* If we have only SVR3.2 options, check combinations and exec cmd.
	 * If we still have arguments left and we haven't see any SVR4 options
	 * assume we're dealing with SVR3.2 */

#ifdef TRACE
fprintf(stderr,"D: 3.2: %d, 4.0: %d\n",svr3only, svr4only);
#endif
	if ( (argc != optind) && !svr4only ) {
#ifdef TRACE
fprintf(stderr,"Running old\n");
#endif
		runold(argc, argv, flags);
		exit (1); /* exec failed */
	}

	if ( svr3only && !svr4only ) {
		if( !validcombination( 'w', flags, allowed[ 6 ], 0 ) )
			exit( 1 );
		else 
			if( !validcombination( 'i', flags, allowed[ 7 ], 0 ) )
				exit( 1 );
		runold(argc, argv, flags);
		exit(1); /* exec failed */
	}
#ifdef TRACE
fprintf(stderr,"E: 3.2: %d, 4.0: %d\n",svr3only, svr4only);
#endif

	/* if neither svr3only or svr4only is set we have two cases:
	 * 1. SVR3.2 version was invoked without arguments.
	 * 2. SVR4.0 version was invoked incorrectly.
	 * We will assume SVR4.  This will cause a usage message
	 * to be produced. */

	/* We know we're in the SVR4.0 case */
	/* Initialize log file */
	brloginit( brcmdname, RESTORE_T );

#ifdef TRACE
fprintf(stderr,"past brloginit\n");
#endif
	if( flags & cFLAG ) {

		/* Cancel jobids */

		if( !validcombination( 'c', flags, allowed[ 1 ], required[ 1 ] ) )
			exit( 1 );

		/* check jobid syntax */
		for( i = optind; i < argc; i++ ) {

			if( !is_rsjobid( argv[ i ] ) ) {

				bkerror( stderr, ERROR4, argv[ i ], 'c' );
				error = TRUE;
			}
		}
		if( error ) exit( 1 );

		/* Actually do the CANCEL */
		for( i = optind; i < argc; i++ ) {

			if( !cancel( argv[ i ] ) ) {

				bkerror( stderr, ERROR11, argv[ i ] );
				error = TRUE;
			}
		}
		exit( error? 2: 0 );

	} else if( flags & AFLAG ) {

		if( !validcombination( 'A', flags, allowed[ 0 ], required[ 0 ] ) )
			exit( 1 );

		if( (flags & (sFLAG|vFLAG)) == (sFLAG|vFLAG) ) {
			bkerror( stderr, ERROR5, "s or v" );
			exit( 1 );
		}

		type = R_DISK_TYPE;

	} else if( flags & DFLAG ) {
		if( odevice && (bad_args & oERROR) ) {
			bkerror( stderr, ERROR4, oarg, 'o' );
			exit( 1 );
		}

		if( !validcombination( 'D', flags, allowed[ 2 ], required[ 2 ] ) )
			exit( 1 );

		if( (flags & (sFLAG|vFLAG)) == (sFLAG|vFLAG) ) {
			bkerror( stderr, ERROR5, "s or v" );
			exit( 1 );
		}
		type = R_DIRECTORY_TYPE;

	} else if( flags & FFLAG ) {
		if( odevice && (bad_args & oERROR) ) {
			bkerror( stderr, ERROR4, oarg, 'o' );
			exit( 1 );
		}
	
		if( !validcombination( 'F', flags, allowed[ 3 ], required[ 3 ] ) )
			exit( 1 );

		if( (flags & (sFLAG|vFLAG)) == (sFLAG|vFLAG) ) {
			bkerror( stderr, ERROR5, "s or v" );
			exit( 1 );
		}
		type = R_FILE_TYPE;

	} else if( flags & PFLAG ) {

		if( !validcombination( 'P', flags, allowed[ 4 ], required[ 4 ] ) )
			exit( 1 );

		if( (flags & (sFLAG|vFLAG)) == (sFLAG|vFLAG) ) {
			bkerror( stderr, ERROR5, "s or v" );
			exit( 1 );
		}
		type = R_PARTITION_TYPE;

	} else if( flags & SFLAG ) {

		if( !validcombination( 'S', flags, allowed[ 5 ], required[ 5 ] ) )
			exit( 1 );

		if( (flags & (sFLAG|vFLAG)) == (sFLAG|vFLAG) ) {
			bkerror( stderr, ERROR5, "s or v" );
			exit( 1 );
		}
		type = R_FILESYS_TYPE;

	} else {
		synopsis();
		exit( 1 );
	}

	/* Initialize Strategy Algorithm */
	rstm_init();

	/* Open history table */
	if( !(path = bk_get_histlog_path()) ) {
		brlog( "Unable to get history log path" );

	} else {
		int rc;

		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = H_ENTRY_F;

		if( (rc = TLopen( &h_tid, path, &descr, O_RDONLY, 0644 ) )
			!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
			if( rc == TLFAILED ) 
				brlog( "TLopen of history table %s fails: %s",
					path, brerrno( errno ) );
			else brlog( "TLopen of history table %s returns %d",
				path, rc );
			h_tid = 0;

		} else {
			/* Get an entry element */
			if( !(h_entry = TLgetentry( h_tid )) ) {
				brlog( "unable to get memory for history table entry\n" );
				TLclose( h_tid );
				h_tid = 0;
			}
		}
	}


	/* Fill in common things in request structure */
	(void) strncpy( (char *) &rqst, "", sizeof( rs_rqst_t ) );

	rqst.date = rtime? rtime: time( 0 );
	rqst.type = type;
	rqst.re_oname = oname;
	rqst.re_odev = odevice;
	rqst.send_mail = (flags & mFLAG);

	/* Make a restore request */
	for( ; optind < argc; optind++ ) {

		rqst.object = f_rel_to_abs( argv[ optind ]);
		if( flags & nFLAG ) {
			if( !view( h_tid, h_entry, &rqst ) ) {
				bkerror( stdout, ERROR16, argv[ optind ] );
				some++;
			}
		} else {
			(void) fprintf( stdout, "%s:\n", rqst.object );

			if( !request( h_tid, h_entry, &rqst ) ) {
				some++;
				bkerror( stdout, ERROR15, argv[optind], rqst.jobid );
			}
		}
	}

	if( IS_URESTORE(brcmdname) && some && !(flags & nFLAG) )
		m_notify_operator();

	exit( some? 2: 0 );
	/*NOTREACHED*/
}

char *
getjobid()
{
	static char buffer[ 30 ];

	if( current > NRESTORES ) 
		/* Too many - type two commands */
		return( (char *) NULL );

	(void) sprintf( buffer, "rest-%ld%c", getpid(),
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"[current] );

	current++;

	return( buffer );
}

/* Parse oname:odevice */
static int
getodev( odev )
char *odev;
{
	char *strtok();
	if( !(oname = strtok( odev, ":" )) ) return( 0 );
	odevice = strtok( odev, NULL );
	return( 1 );
}

static void
synopsis()
{
	if( IS_URESTORE(brcmdname) ) {
		(void) fprintf( stderr,
			"urestore [ -mn ] [ -s|v ] [ -o target ] [ -d date ] -F file ...\n" );
		(void) fprintf( stderr,
			"urestore [ -mn ] [ -s|v ] [ -o target ] [ -d date ] -D dir ...\n" );
		(void) fprintf( stderr, "urestore -c jobid\n" );
	} else {
		(void) fprintf( stderr,
			"restore [ -mn ] [ -s|v ] [ -o target ] [ -d date ] -P partition_dev ...\n" );
		(void) fprintf( stderr,
			"restore [ -mn ] [ -s|v ] [ -o target ] [ -d date ] -A disk_dev ...\n" );
		(void) fprintf( stderr,
			"restore [ -mn ] [ -s|v ] [ -o target ] [ -d date ] -S fs_dev ...\n" );
		(void) fprintf( stderr,
			"restore -w [ -W device ] [ -O ] [ -T ] [ -M ] \n");
		(void) fprintf( stderr,
			"restore [ -W device ] [ -O ] [ -T ] [ -M ] [ pattern [pattern] ...] \n");
		(void) fprintf( stderr,
			"restore -i [ -W device ] [ -T ] \n");
	}
}

runold(argc,argv, flags)
int argc;
char **argv;
int flags;
{
	char **nargv;
	int i;

	nargv=(char **)malloc((argc+2)*sizeof(char *));
	memcpy((char *)(nargv+1),(char *)argv,(argc+1)*sizeof(char *));
	nargv[0]="sh";
	nargv[1]=SVR3RESTORE;
	/* 
	 * The following code is needed until SVR3.2 functionality is
	 * incorporated into SVR4.0 restore.  It changes the new option
	 * names back to the old so that the restore command will work
	 */
	if ( flags & (WFLAG | OFLAG | MFLAG | TFLAG | wFLAG ) ) {
		for( i=2; i <= argc; i++) 
			switch (nargv[i][1]) {
			case 'W':
				nargv[i][1]='d';
				break;
			case 'O':
				nargv[i][1]='o';
				break;
			case 'M':
				nargv[i][1]='s';
				break;
			case 'T':
				nargv[i][1]='t';
				break;
			case 'w':
				nargv[i][1]='c';
				break;
			}
	}
	execv("/sbin/sh",nargv);
	fprintf(stderr,"exec failed errno %d\n",errno);
}
