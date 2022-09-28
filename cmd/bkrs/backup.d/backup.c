/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:backup.d/backup.c	1.15.5.1"

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <setjmp.h>
#include <pwd.h>
#include <backup.h>
#include <bkmsgs.h>
#include <bkreg.h>
#include <bkrs.h>
#include <errors.h>
#include <signal.h>
#include <errno.h>

#define	CWDSZ	BKFNAME_SZ
#define OLDBACKUP "/usr/bin/backup"
#define SVR3BACKUP "/usr/bin/.backup"

/* Does a process exist? */
#define proc_exist( pid )	(!kill( pid, 0 ) || errno != ESRCH)

/* Flags to tell which options have been seen */
#define unused	0x1
#define	AFLAG	0x2
#define	cFLAG	0x4
#define	CFLAG	0x8
#define	eFLAG	0x10
#define oFLAG	0x20
#define	iFLAG	0x40
#define	jFLAG	0x80
#define	mFLAG	0x100
#define	nFLAG	0x200
#define	RFLAG	0x400
#define	sFLAG	0x800
#define	SFLAG	0x1000
#define	tFLAG	0x2000
#define	uFLAG	0x4000
#define vFLAG	0x8000
/* flags for SVR3.2 functionality */
#define TFLAG	0x10000
#define wFLAG	0x20000
#define pFLAG	0x40000
#define hFLAG	0x80000
#define UFLAG	0x100000
#define fFLAG	0x200000
#define WFLAG	0x400000

#define	NFLAGS	15

long nflags = NFLAGS;

/* FLAG to option mapping */
char *optname = "aAcCefijmnRsStuvTwphUfW";

/* The valid combinations of flags */
int allowed[] = {
	(tFLAG | oFLAG | mFLAG | eFLAG | nFLAG | cFLAG ),
	(iFLAG | tFLAG | oFLAG | mFLAG | eFLAG | nFLAG | cFLAG | sFLAG | vFLAG ),
	(SFLAG | uFLAG | jFLAG | AFLAG),
	(RFLAG | uFLAG | jFLAG | AFLAG),
	(CFLAG | uFLAG | jFLAG | AFLAG),
	(wFLAG | TFLAG | WFLAG),
	(pFLAG | TFLAG | WFLAG),
	(UFLAG | TFLAG | WFLAG),
	(fFLAG | TFLAG | WFLAG),
	0
};
/* Required combinations of flags */
long required[] = {
	iFLAG,
	SFLAG,
	RFLAG,
	CFLAG,
	0
};

#define	cERROR 0x1
#define oERROR 0x2
#define	jERROR 0x4
#define mERROR 0x8
#define tERROR 0x10
#define	uERROR 0x20

/* Options to tell which option arguments were invalid */
#define	ALLBAD	( cERROR | oERROR | mERROR | tERROR | uERROR )

#define	NTRIES	3

static void bkstart();
static int incoming();
static int p_c_arg();
static void bksend();
static void synopsis();

extern void	bkerror();
static void new_message(), interrupt();
extern char *bk_get_bkoper_path();
extern void brloginit();
extern int getopt();
extern char *strdup();
extern pid_t jobidtopid();
extern void exit();
extern int validcombination();
extern void brlog();
extern uid_t getuid();
extern pid_t bkm_init();
extern char *strncpy();
extern char *bk_get_bkreg_path();
extern char *getcwd();
extern unsigned int strlen();
extern char *strcpy();
extern int access();
extern int bkregvalid();
extern gid_t getgid();
extern pid_t getpid();
extern char *strtok();
extern unsigned char *p_backup();
extern int in_dequeue();
extern unsigned int alarm();
extern int bkspawn();
extern char *brerrno();
extern pid_t wait();
extern void pr_estimate();
extern void pr_failure();
extern void pause();
extern int bkm_send();
extern int bkm_receive();
extern void in_enqueue();

long flags = 0;
pid_t bkdaemon;
int break_sent = FALSE;
int signalseen = 0;
char *brcmdname;
int bklevels = 0;

/* Number of attempts at restarting bkdaemon */
static int ntries = 0;
static jmp_buf	env;

/* Values of option arguments */
static char *table = NULL, *dname = NULL, *user = NULL, *c_arg = NULL;
static char *owner = NULL, *jarg;
static pid_t jobid = 0;
static uid_t owner_uid = 0;

/* -c option */
static bkrotate_t	c_date;
static int w1 = 0, w2 = 0, d1 = 0, d2 = 0;

/* Current working directory */
static char cwd[ CWDSZ ];

main( argc, argv )
char *argv[];
{
	extern char *optarg;
	extern int optind;
	int c,  error_seen = FALSE, bad_args = 0, rc;
	uid_t tmp;
	bkdata_t msg;
	struct passwd *pwd;
	char controlflag;
	/* svr3only flag when set, mains we've seen an option that is unique
	 * to SVR3.2.
	 * svr4only flag is set as SVR4 only options are found. This prevents 
 	 * execution of SVR3.2 backup in error case.  This would cause a 
	 * confusing usage message. 
	 * The conflict flag is set when we hit an option that is not unique
	 * to either SVR3.2 or SVR4.0.  This is resolved after all the options
	 * are processed. */
	char svr3only=0;
	int svr4only= 0;
	int conflict=0;

	brcmdname = (char *)argv[0];

	(void) sigignore( SIGUSR1 );
	(void) sigset( SIGCLD, SIG_DFL );

	/* If strings match, we're SVR3.2.  Just exec the old backup. */
	/* Initialize log file */
	if ( strcmp(brcmdname, OLDBACKUP ) )
		brloginit( brcmdname, BACKUP_T );
	else {
		runold(argc, argv, 0);
		exit( 1 ); /* exec failed */
	}

	/* t, c and u options in SVR4.0 take an argument.  In SVR3.2 they 
	 * didn't.  In order to prevent getopt from squawking when no argument 
	 * is specified, define the options as not taking arguments and 
	 * process the arguments independant of getopt. */

	while( (c = getopt( argc, argv, "ACcd:ef:o:hij:m:npRsSTtU:u:vW:w?" )) != -1 )
		switch( c ) {

		case 'A':
			/* Cancel, Suspend, or Restart ALL Backups */
			flags |= AFLAG;
			svr4only=1;
			break;

		case 'c': /* potential SVR3.2/SVR4.0 conflict */
			/* Set the new flag in the case of SVR3.2 in order
			   to check for conflicts. */
			if (optind < argc ) 
				if ( argv[optind][0] == '-' ) {
					flags |= wFLAG;
					svr3only=1;
				}
				else {
				/* Do backups for a specific date */
					flags |= cFLAG;
					c_arg = argv[optind++];
					if( !p_c_arg( strdup( c_arg ) ) ) 
						bad_args |= cERROR;
				}
			else { /* nothing follows. must be SVR3.2 */
				flags |= wFLAG;
				svr3only=1;
			}
			break;

		case 'C':
			/* Cancel the backups */
			flags |= CFLAG;
			svr4only=1;
			break;

		case 'd':
		case 'W': /* This option changes to "-W" as of SVR4.0 */
			flags |= WFLAG;
			svr3only=1;
			break;

		case 'e':
			/* Estimate number of Volumes required */
			flags |= eFLAG;
			svr4only=1;
			break;

		case 'f': /* this function should eventually be in backup */
			flags |= fFLAG;
			svr3only=1;
			break;

		case 'o':
			/* file system or data partition */
			flags |= oFLAG;
			svr4only=1;
			dname = optarg;
			if( !dname || !(*dname) ) bad_args |= oERROR;
			break;

		case 'h':
			flags |= hFLAG;
			svr3only=1;
			break;

		case 'i':
			/* Interactive backup */
			flags |= iFLAG;
			svr4only=1;
			break;

		case 'j':
			/* Control methods for a particular jobid */
			flags |= jFLAG;
			svr4only=1;
			jarg = optarg;
			if( !(jobid = jobidtopid( optarg ) ) )
				bad_args |= jERROR;
			break;

		case 'm':
			/* Mail to user */
			flags |= mFLAG;
			svr4only=1;
			user = optarg;
			if( !user || !(*user) ) bad_args |= mERROR;
			break;

		case 'n':
			/* No execute mode */
			flags |= nFLAG;
			svr4only=1;
			break;

		case 'p':
			flags |= pFLAG;
			svr3only=1;
			break;

		case 'R':
			/* Restart halted backups */
			flags |= RFLAG;
			svr4only=1;
			break;

		case 's':
			/* Dot mode */
			flags |= sFLAG;
			svr4only=1;
			break;

		case 'S':
			/* Suspend Backups */
			flags |= SFLAG;
			svr4only=1;
			break;

		case 'T': /* new option for SVR3.2 tape message */
			flags |= TFLAG;
			svr3only=1;
			break;

		case 't': /* potential SVR3.2/SVR4.0 conflict */
			if (optind < argc ) 
				if ( argv[optind][0] == '-' ) {
					flags |= TFLAG;
					svr3only=1;
				}
				else {
					/* Use this table */
					flags |= tFLAG;
					table = argv[optind++];
					if( !table || !(*table) ) bad_args |= tERROR;
				}
			else { /* nothing follows. must be SVR3.2 */
				flags |= TFLAG;
				svr3only=1;
			}
			break;

		case 'U': /* new option for SVR3.2 backup user */
			flags |= UFLAG;
			svr3only=1;
			break;

		case 'u': /* potential SVR3.2/SVR4.0 conflict */
			/* Suspend, Cancel, or Restart backups for a particular user */
			conflict=1; /*figure this out when we're done*/
			flags |= uFLAG;
			owner = optarg;
			/* error checking on owner won't be done until
			 * the conflict is resolved. */
			break;

		case 'v':
			/* Verbose Mode */
			flags |= vFLAG;
			svr4only=1;
			break;

		case 'w': /* new option for SVR3.2 complete backup */
			  /* this function should eventually be in backup */
			flags |= wFLAG;
			svr3only=1;
			break;

		case '?':
			synopsis();
			exit( 2 );
			/*NOTREACHED*/
			break;

		default:
			bkerror( stderr, ERROR0, c );
			error_seen = TRUE;
			break;
		}

	/* If invoked with a full path, force SVR4 style */
	if ( !strcmp(brcmdname, "/usr/sbin/backup" ) ) {
		svr3only=0;
		svr4only=1;
	}

	 /* Don't need to do anything if conflict is resolved in favor
	 * of SVR4.0.  Reset flags if SVR3.2 */

	if ( conflict )
		/* if neither flag is set, assume SVR3.2.  The -u option
		 * alone doesn't make sense in SVR4. */
		if ( (svr3only && !svr4only) || (!svr3only && !svr4only) ) {
			flags |= UFLAG;
			flags &= ~uFLAG;
			svr3only=1; /* in case -u is only option */
		} else
			if ( svr3only && svr4only ) {
				fprintf(stderr,"UNRESOLVABLE CONFLICT\n");
				exit( 1 );
			} else /* do error checking on SVR4 case */
				if( !owner || !(*owner) ) bad_args |= uERROR;

	if( optind != argc && !error_seen ) {
		for( ; optind < argc; optind++ )
			bkerror( stderr, ERROR1, argv[ optind ] );
		exit( 1 );
	}

	if( bad_args == ALLBAD ) {

		bkerror( stderr, ERROR3 );
		exit( 1 );

	} else if( bad_args ) {

		if( bad_args & cERROR )	{
			if( w1 != w2 || d1 != d2 )
				bkerror( stderr, ERROR6, 'c' );
			else bkerror( stderr, ERROR4, c_arg, 'c' );
		}
		if( bad_args & oERROR )	bkerror( stderr, ERROR4, dname, 'f' );
		if( bad_args & jERROR )	bkerror( stderr, ERROR4, jarg, 'j' );
		if( bad_args & mERROR )	bkerror( stderr, ERROR4, user, 'm' );
		if( bad_args & tERROR )	bkerror( stderr, ERROR4, table, 't' );
		if( bad_args & uERROR )	bkerror( stderr, ERROR4, owner, 'u' );
		exit( 1 );

	} else if( error_seen ) exit( 1 );
			

	/* If we have only SVR3.2 options, check combinations and exec cmd 
	 * If neither svr3only or svr4only is set, assume SVR4.0 since we
	 * want to produce a usage message. */

	if (svr3only && !svr4only) {
		if ( flags & wFLAG )
			if( !validcombination( 'w', flags, allowed[ 5 ], 0 ) )
				exit( 1 );
		else if ( (flags & hFLAG) && (flags != hFLAG) ) {
			bkerror(stderr,"-h option cannot be used with any other\n");
			exit(1);
		}
		else if ( flags & pFLAG )
			if( !validcombination( 'p', flags, allowed[ 6 ], 0 ) )
				exit( 1 );
		else if ( flags & fFLAG )
			if( !validcombination( 'f', flags, allowed[ 8 ], 0 ) )
				exit( 1 );
		else if ( flags & UFLAG )
			if( !validcombination( 'U', flags, allowed[ 7 ], 0 ) )
				exit( 1 );
		else {
			synopsis();
			exit(2);
		}
		runold(argc, argv, flags);
		exit(1); /* exec failed */
	}

	/* We know we're in the SVR4.0 case */
	if( flags & iFLAG ) {
		if( !validcombination( 'i', flags, allowed[ 1 ], required[ 0 ] ) )
			exit( 1 );
		else if( (flags & (sFLAG | vFLAG)) == (sFLAG | vFLAG) ) {
			bkerror( stderr, ERROR7, 's', 'v', 'i' );
			exit( 1 );
		}
	} else if( (flags & (CFLAG | RFLAG | SFLAG) ) ) {
		if( ( flags & CFLAG ) ) {
			controlflag = 'C';
			if( !validcombination( 'C', flags, allowed[ 4 ], required[ 3 ] ) )
				exit( 1 );
		} else if( (flags & RFLAG) ) {
			controlflag = 'R';
			if( !validcombination( 'R', flags, allowed[ 3 ], required[ 2 ] ) )
				exit( 1 );
		} else if( (flags & SFLAG) ) {
			controlflag = 'S';
			if( !validcombination( 'S', flags, allowed[ 2 ], required[ 1 ] ) )
				exit( 1 );
		}

		if( (flags & (uFLAG | AFLAG)) == (uFLAG | AFLAG) ) {
			bkerror( stderr, ERROR7, 'u', 'A', controlflag );
			exit( 1 );
		} 

		if( (flags & (uFLAG | jFLAG)) == (uFLAG | jFLAG) ) {
			bkerror( stderr, ERROR7, 'u', 'j', controlflag );
			exit( 1 );
		} 

		if( (flags & (jFLAG | AFLAG)) == (jFLAG | AFLAG) ) {
			bkerror( stderr, ERROR7, 'j', 'A', controlflag );
			exit( 1 );
		} 

	} else if( flags & mFLAG ) {
		pwd = getpwnam( user );
		endpwent();
		if( !pwd ) {
			bkerror( stderr, ERROR2, user );
			exit( 1 );
		}
	} else if( flags & uFLAG ) {
		pwd = getpwnam( owner );
		endpwent();
		if( !pwd ) {
			bkerror( stderr, ERROR2, owner );
			exit( 1 );
		}
		owner_uid = pwd->pw_uid;
	}

	if( setjmp( env ) != 0 ) {

		/* To get here, bkdaemon must have died */
		if( ntries++ > NTRIES ) {
			bkerror( stderr, ERROR10 );
			brlog( "backup daemon won't stay alive - exiting" );
			exit( 1 );
		}

		switch( flags & (SFLAG|RFLAG|CFLAG) ) {
		case SFLAG:
			bkerror( stderr, ERROR9, "suspend" );
			exit( 1 );
			break;

		case RFLAG:
			bkerror( stderr, ERROR9, "resume" );
			exit( 1 );
			break;

		case CFLAG:
			bkerror( stderr, ERROR9, "cancel" );
			exit( 1 );
			break;

		default:
			bkerror( stderr, ERROR8 );
		}
	}
	
	if( flags & (SFLAG | RFLAG | CFLAG ) ) {

		/* SUSPEND, RESUME, or CANCEL  methods */
		if( flags & uFLAG ) {
			/* Check Permissions to control methods */
			tmp = getuid();
			if( tmp && tmp != owner_uid ) {
				bkerror( stderr, ERROR15 );
				exit( 1 );
			}

			msg.control.uid = owner_uid;
			msg.control.flags |= CTL_UID;

		} else if( flags & AFLAG ) {
			/* Must be root to control ALL methods */
			if( getuid() ) {
				bkerror( stderr, ERROR15 );
				exit( 1 );
			}
			msg.control.flags |= CTL_ALL;
		}

		/* Plug into conversation with bkdaemon*/
		if( (bkdaemon = bkm_init( BKNAME, FALSE )) == -1 ) {
			/* No backups occurring right now */
			bkerror( stderr, ERROR32,
				((flags & SFLAG)? "suspend":
				((flags & RFLAG)? "resume":
				"cancel")) );
			exit( 2 );
		}

		(void) sigset( SIGUSR1, new_message );

		if( flags & jFLAG ) {
			msg.control.pid = jobid;
			msg.control.flags |= CTL_PID;
		} 
		if( !(flags & (uFLAG|AFLAG) ) )
			 msg.control.uid = getuid();

		if( flags & SFLAG ) bksend( bkdaemon, SUSPEND, &msg );
		else if( flags & RFLAG ) bksend( bkdaemon, RESUME, &msg );
		else bksend( bkdaemon, CANCEL, &msg );

	} else {

		/* START methods */
		int options;

		/* fill msg structure */
		(void) strncpy( &msg, "", sizeof( bkdata_t ) );
		options = 0;
		if( flags & cFLAG ) {
			if( IS_DEMAND( c_date ) ) options |= S_DEMAND;
			else {
				msg.start.week = w1;
				msg.start.day = d1;
			}
		}
		if( flags & eFLAG ) options |= S_ESTIMATE;
		if( flags & nFLAG )	options |= S_NO_EXECUTE;
		if( flags & iFLAG ) {
			options |= S_INTERACTIVE;
			if( flags & sFLAG ) options |= S_SEND_DOTS;
			else if( flags & vFLAG ) options |= S_SEND_FILENAMES;
		}
		msg.start.options = options;

		if( !table )
			table = (char *)strdup( bk_get_bkreg_path() );
		else if( *table != '/' || (dname && *dname != '/' ) )
			/* relative path name */
			if( !getcwd( cwd, CWDSZ ) ) {
				brlog( "Couldn't get current working directory" );
				bkerror( stderr, ERROR12 );
				exit( 1 );
			}

		if( *table != '/' ) {
			if( (strlen( table ) + strlen( cwd ) + 2 ) >= BKTEXT_SZ ) {
				bkerror( stderr, ERROR13, cwd, table );
				exit( 1 );
			}
			(void) sprintf( msg.start.table, "%s/%s", cwd, table );
		} else if( strlen( table ) >= BKTEXT_SZ ) {
			bkerror( stderr, ERROR5, table );
			exit( 1 );
		} else (void) strcpy( msg.start.table, table );

		if( access( msg.start.table, 04 ) ) {
			bkerror( stderr, ERROR14, msg.start.table );
			exit( 1 );
		}

		if( rc = bkregvalid( msg.start.table ) ) {
#ifdef TRACE
			brlog( "bkregvalid of %s returns %d", msg.start.table, rc );
#endif
			exit( 1 );
		}

		if( dname ) {
			if( *dname != '/' ) {
				if( (strlen( dname ) + strlen( cwd ) + 2) >= BKFNAME_SZ ) {
					bkerror( stderr, ERROR13, cwd, table );
					exit( 1 );
				}
				(void) sprintf( msg.start.fname, "%s/%s", cwd, dname );
			} else if( strlen( dname ) >= BKFNAME_SZ ) {
				bkerror( stderr, ERROR5, dname );
				exit( 1 );
			} else (void) strcpy( msg.start.fname, dname );
			if( access( msg.start.fname, 04 ) ) {
				bkerror( stderr, ERROR14, msg.start.fname );
				exit( 1 );
			}
		} else msg.start.fname[0] = '\0';

		if( flags & mFLAG )
			(void) strcpy( msg.start.user, user );

		msg.start.my_uid = getuid();
		msg.start.my_gid = (int) getgid();

#ifdef TRACE
	brlog(
	"do_start_m(): fname %s table %s uid %ld gid %ld options 0x%x week %d day %d",
		msg.start.fname, msg.start.table, msg.start.my_uid,
		msg.start.my_gid, msg.start.options, 
		msg.start.week, msg.start.day);
#endif
		/*
			bkdaemon may already be started from some other backup command.
			If so, plug in; if not, start it up.
		*/
		if( (bkdaemon = bkm_init( BKNAME, FALSE )) == -1 )	bkstart();

		(void) sigset( SIGUSR1, new_message );

#ifdef TRACE
		brlog( "Send START MSG" );
#endif
		bksend( bkdaemon, START, &msg );

		/* If running in the background, ignore SIGINTS */
		if( sigset( SIGINT, interrupt ) == SIG_IGN )
			sigset( SIGINT, SIG_IGN );

		if( !(flags & nFLAG) )
			(void) fprintf( stdout, "The backup job id is back-%ld\n", getpid() );
	}
	exit( incoming() );
}

/* Parse "demand" or week:day */
static int
p_c_arg( string )
char *string;
{
	unsigned char *wkstr, *daystr;

	if( !(wkstr = (unsigned char *)strtok( string, ":" ) )
		|| !(daystr = (unsigned char *)strtok( NULL, ":" ) ) ) {
		wkstr = (unsigned char *) string;
		daystr = NULL;
	}

	return( p_backup( wkstr, daystr, c_date, &w1, &w2, &d1, &d2 ) 
		&& w1 == w2 && d1 == d2 );
}

static int
incoming()
{
	register done = FALSE, exitcode = 0;
	long rc = -1;
	int type, status;
	char *bkoperpath = bk_get_bkoper_path();
	bkdata_t data;

	(void) sigset( SIGALRM, (void (*)())new_message );
	while( !done ) {
		while( !in_dequeue( &type, &data ) ) {
			(void) alarm( 30 );	/* poll the message queue. */
			(void) sigpause( SIGUSR1 );
			(void) alarm( 0 );

		}
			
		switch( type ) {
		case DOT:
			(void) fprintf( stdout, "." );
			(void) fflush( stdout );
			break;

		case GET_VOLUME:
			/* A method script needs an operator */
			if( flags & iFLAG ) {
				if( flags & sFLAG ) 
					(void) fprintf( stdout, "\n" );
				(void) fprintf( stdout, "invoking bkoper...\n" );
				if( bkspawn( bkoperpath, "-", "-", "-", 0, 0, BKARGS, 0 ) == -1 ) {
					brlog( "could not spawn bkoper: %s", brerrno( errno ) );
					bkerror( stderr, ERROR16, brerrno( errno ) );
				} else {
					rc = -1;
					while( rc == -1 ) {
						rc = (long) wait( &status );
						if( errno == ECHILD ) break;
					}
					if( rc == -1 || status )
						brlog( "bkoper returned %d, errno %d status 0x%lx",
							rc, errno, status );
				}
				(void) fprintf( stdout, "bkoper terminated\n" );
			} else brlog(
				"Received GET_VOLUME msg. and this is not an interactive backup." );
			bksend( bkdaemon, DISCONNECTED, 0 );
			break;

		case ESTIMATE:
			BEGIN_CRITICAL_REGION;
			if( flags & sFLAG ) 
				(void) fprintf( stdout, "\n" );
			pr_estimate( &data );
			END_CRITICAL_REGION;
			break;

		case FAILED:
			BEGIN_CRITICAL_REGION;
			if( flags & sFLAG ) 
				(void) fprintf( stdout, "\n" );
			pr_failure( &data );
			END_CRITICAL_REGION;
			exitcode = 2;
			break;

		case TEXT:
			if( flags & sFLAG ) 
				(void) fprintf( stdout, "\n" );
			BEGIN_CRITICAL_REGION;
			(void) fprintf( stdout, "%s\n", data.text.text );
			END_CRITICAL_REGION;
			break;

		case DONE:
			done = TRUE;
			break;

		default:
			brlog( "received a message of unexpected type: %d", type );
		}
	}
	return( exitcode );
}

/* 
	bkstart() tries to start up bkdaemon.  It must wait for a signal from
	bkdaemon to indicate that it is alive.  It also has a timeout mechanism
	in case bkdaemon doesn't make it.
*/
static void
bkstart()
{
	register i;
	char pathname[ 100 ], *bk_get_bkdaemon_path();
	static void bkdaemon_wait();
#ifdef TRACE
	brlog( "attempt to start bkdaemon" );
#endif
	if( (bkdaemon = bkm_init( BKNAME, FALSE ) ) == -1 ) {
		signalseen = 0;

		(void) strcpy( pathname, bk_get_bkdaemon_path() );
		if( (bkdaemon = bkspawn( pathname, NULL, NULL, NULL, 0, 0, BKARGS, 0 ))
			== -1 ) {
			brlog( "Cannot spawn bkdaemon: fork() fails: errno %d", errno );
			exit( 1 );
		}

#ifdef TRACE
		brlog( "bkdaemon is spawned." );
#endif

		(void) sigset( SIGUSR1, (void (*)()) bkdaemon_wait );

		/* bkdaemon sends a SIGUSR1 when things are going */
		for( i = 0; i < NTRIES; i++, signalseen = 0 ) {

			(void) sigset( SIGALRM, (void (*)()) bkdaemon_wait );

			if( signalseen != SIGUSR1 ) {
				(void) alarm( 15 );
				/* wait to see if bkdaemon dies */
				pause();
				(void) alarm( 0 );
			}

			if( signalseen == SIGALRM ) {
				/* alarm went off */
				if( proc_exist( bkdaemon ) ) {
#ifdef TRACE
					brlog( "bkdaemon is still running" );
#endif
					continue;
				} 
			}
			break; 
		}

		(void) sigignore( SIGUSR1 );

		if( (bkdaemon = bkm_init( BKNAME, FALSE ) ) == -1 ) {
			brlog( "bkdaemon started and then died, errno %d", errno );
			longjmp( env, 1 );
		}
	}
#ifdef TRACE
		else brlog( "bkdaemon is already running." );
#endif
}

static void
interrupt()
{
	bkdata_t msg;
	if( !break_sent ) {
		msg.control.uid = getuid();
		msg.control.pid = getpid();
		msg.control.flags = CTL_PID|CTL_UID;
		if( bkm_send( bkdaemon, CANCEL, &msg ) == -1 && errno == ENOENT )
			exit( 0 );
		break_sent = TRUE;
		(void) fprintf( stdout, "...Cancelling backups\n" );
	}
#ifdef TRACE
	else brlog( "saw BREAK" );
#endif
	(void) sigset( SIGINT, SIG_DFL );
}

static void
bkdaemon_wait( sig )
{
	(void) sigignore( SIGALRM );
#ifdef TRACE
	brlog( "Signal %d arrived", sig );
#endif
	signalseen = sig;
}

/* Send a message - if the destination is bkdaemon and it has died, 
	attempt to start over.
*/
static void
bksend( destination, msgtype, data )
int destination, msgtype;
bkdata_t	*data;
{
	if( bkm_send( destination, msgtype, data ) != -1 ) return;

	brlog( "bkm_send returns -1; errno %d", errno );

	if( destination == bkdaemon && errno == ENOENT ) {
		longjmp( env, 1 );
	} 
	exit( 1 );
}

static void
synopsis()
{
	(void) fprintf( stderr,
		"backup [ -t table ] [ -o oname[:odevice] ] [ -m user ] [ -en ] [ -c week:day | demand ]\n" );
	(void) fprintf( stderr,
		"backup -i [ -t table ] [ -o oname[:odevice] ] [ -m user ] [ -en ] [ -s | -v ] [ -c week:day | demand ]\n" );
	(void) fprintf( stderr, "backup -S | R | C [ -u user | -A | -j jobid ]\n" );
	(void) fprintf( stderr, "backup -w | -p | -f <files> | -U \"user [user ...]\" [ -d device ] [ -T ] \n");
	(void) fprintf( stderr, "backup -h\n");

}

static void
new_message( sig )
int sig;
{
	register more = TRUE;
	int type;
	pid_t orig;
	bkdata_t data;
	
	BEGIN_SIGNAL_HANDLER;

#ifdef TRACE
	brlog( "Signal %d arrived", sig );
#endif

	while( more ) {
		if( bkm_receive( &orig, &type, &data ) == -1 ) {
			more = FALSE;
			if( errno == EINTR )
				brlog( "no message received yet" );
			else if( errno != ENOMSG )
				brlog( "bkm_receive returns -1; errno %d", errno );
#ifdef BOZO
			else brlog( "new_message(): no message received." );
#endif
		} else in_enqueue( type, &data );
	}
	END_SIGNAL_HANDLER;
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
	nargv[1]=SVR3BACKUP;
	/* 
	 * The following code is needed until SVR3.2 functionality is
	 * incorporated into SVR4.0 backup.  It changes the new option
	 * names back to the old so that the backup command will work.
	 */
	if ( flags & (WFLAG | UFLAG | TFLAG | wFLAG ) ) {
		for( i=2; i <= argc; i++) 
			switch (nargv[i][1]) {
			case 'W':
				nargv[i][1]='d';
				break;
			case 'U':
				nargv[i][1]='u';
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
