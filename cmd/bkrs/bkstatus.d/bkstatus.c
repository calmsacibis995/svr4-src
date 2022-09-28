/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkstatus.d/bkstatus.c	1.7.2.1"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <ctype.h>
#include <table.h>
#include <bkstatus.h>
#include <backup.h>
#include <bktypes.h>
#include <errors.h>

#define TRUE	1
#define FALSE	0

#define NULSTR	""
#define DASH	'-'

/* State letters */
#define ACTIVE		(unsigned char *)"a"
#define PENDING		(unsigned char *)"p"
#define WAITING		(unsigned char *)"w"
#define SUSPENDED	(unsigned char *)"s"
#define FAILED		(unsigned char *)"f"
#define COMPLETED	(unsigned char *)"c"

/* all states */
#define ALL	"acfpsw"

/* Size of chunks to malloc for entry queue. */
#define EMALLOCSZ	100

/* Number of fields to be printed */
#define PRFLDCNT	8


/* Flags to tell which options have been seen */
#define aFLAG	0x1	/* all states */
#define fFLAG	0x2	/* field separator character, suppress field wrap */
#define hFLAG	0x4	/* suppress headers */
#define jFLAG	0x8	/* filter on jobids */
#define pFLAG	0x10	/* set period */
#define sFLAG	0x20	/* filter on states */
#define uFLAG	0x40	/* filter on users */

/* Error flags */
#define jERROR	0x1	/* filter on jobids */
#define sERROR	0x2	/* filter on states */
#define uERROR	0x4	/* filter on users */

/* Length in characters of string representation (including terminating null) */
/* of largest possible uid (decimal), assuming a 32-bit int. */
#define UIDLEN	11

/* bkstatus table */
unsigned char *table;

/* name of this command */
char *brcmdname;

/* table id for bkstatus table */
int tid;

/* oflag with which to open table */
int oflag = O_RDONLY;

/* flags seen */
int flags = 0;

/* field separator character */
char fld_sep;

/* Length of fields for wrapped display - fields are Jobid, Tag, Oname, Odevice, */
/* Start Time, Dest, Status */
int prlens[PRFLDCNT] = { 10, 8, 7, 10, 10, 12, 10, 6 };

/* Time format string */
char *tfmt = "%b %d %R";

/* Buffer for time conversion - must provide enough space for corresponding */
/* format (see tfmt definition) */
char tbuf[13];

/* pointer to array of pointers to job id strings user entered */
argv_t *j_argv;

/* pointer to array of pointers to user name strings user entered */
argv_t *u_argv;

/* period length for log truncation */
int period;

/* default states on which to filter log */
char *states = "apsw";

/* switch indicating to print running states (apsw) */
/* used to avoid printing entries which have not completed, due to the */
/* daemon exiting abnormally, but which are still in the log with a */
/* state indicating running. */
int prt_running = TRUE;

/* switch indicating to print completed states (fc) */
int prt_completed = FALSE;

/* queue of pointers to entry structures */
ENTRY entryq[EMALLOCSZ];
ENTRY *entryqptr = entryq;

/* number of entries in the queue */
int entrycnt = -1;

/* size to malloc for next chunk of entry pointers */
int newsize = EMALLOCSZ * sizeof( ENTRY );

/* for lint */
void exit();
void bkerror();

/* Compile with debugging on for regression testing.  This allows the tester */
/* to invoke the command and specify whether bkdaemon is or is not active. */
/* The command should be invoked for testing as follows: */
/*      bkstatus options y      for daemon running       */
/*      bkstatus options n      for daemon dead          */
#ifdef DEBUG
int daemon;
#endif

/* Program displays selected information from the backup status log or allows */
/* user to set the period after which completed jobs may be deleted from the log. */
/* Note that the -p option cannot be used with any other options and that the */
/* -a and -s options are mutually exclusive.  The field separator can only be */
/* specified if the user chooses to suppress field wrap. */

main (argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	void compress();
	void display();
	void do_pflag();
	void qsort();
	void select();
	void synopsis();

	argv_t *s_to_argv();

	int error_seen = FALSE;
	int bad_args = 0;
	int all_bad = 0;
	int c;
	int compar();
	int conv_users();
	int getopt();
	int p_integer();
	int tblopen();
	int valid_jids();
	int valid_states();

	char *j_arg;
	char *u_arg;

	brcmdname = (char *)argv[0];

	while (( c = getopt( argc, argv, "af:hj:p:s:u:?" )) != -1 )
		switch ( c ) {
		case 'a':
			flags |= aFLAG;
			states = ALL;
			prt_completed = TRUE;
			break;

		case 'f':
			if( strlen( optarg ) > 1 )
				bkerror( stderr, ERROR4, optarg );
			else {
				if ( *optarg == NULL )
					fld_sep = '\t';
				else
					fld_sep = optarg[0];
				flags |= fFLAG;
			}
			break;

		case 'h':
			flags |= hFLAG;
			break;

		case 'j':
			flags |= jFLAG;
			all_bad |= jERROR;
			j_arg = optarg;
			j_argv = s_to_argv( j_arg, ", " );
			if( !valid_jids() )
				bad_args |= jERROR;
			break;

		case 'p':
			flags |= pFLAG;
			p_integer( optarg, &period );
			if ( period <= 0 || period > WK_PER_YR  ) {
				error_seen = TRUE;
				bkerror( stderr, ERROR1, optarg );
				bkerror( stderr, ERROR16, WK_PER_YR );
			}
			else
				oflag = O_RDWR;
			break;

		case 's':
			flags |= sFLAG;
			all_bad |= sERROR;
			states =  strdup( optarg );
			compress( states );
			if ( !valid_states() )
				bad_args |= sERROR;
			break;

		case 'u':
			flags |= uFLAG;
			all_bad |= uERROR;
			u_arg = optarg;
			u_argv = s_to_argv( u_arg, ", " );
			if ( !conv_users() )
				bad_args |= uERROR;
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

#ifdef DEBUG
	daemon = 0;
	if( optind < argc ) {
		if( strlen( argv[optind] ) > 1 )
			fprintf( stderr,
			"Error - debug option must be y or n.\n");
		else {
			if( argv[optind][0] == 'y')
				daemon = 1;
			else if( argv[optind][0] == 'n')
				daemon = 0;
			else
				fprintf( stderr,
				"Error - debug option must be y or n.\n");
			
		}
	}
#else
	/* Check for non-option, invalid arguments */
	if( optind < argc ) {
		bkerror( stderr, ERROR1, argv[ optind ] );
		synopsis();
		exit(1);
	}
#endif

	if ( flags & pFLAG ) {
		if ( (flags & aFLAG) || (flags & fFLAG) || (flags & hFLAG) ||
		     (flags & sFLAG) || (flags & jFLAG) || (flags & uFLAG) ) {
			bkerror( stderr, ERROR2, 'p' );
			error_seen = TRUE;
		}
	}
	else  if ( (flags & aFLAG) && (flags & sFLAG) ) {
		bkerror( stderr, ERROR3, 'a', 's' );
		error_seen = TRUE;
	}

	if ( (bad_args != 0) && (bad_args == all_bad) ) {
		bkerror( stderr, ERROR12 );
		exit( 1 );
	}

	if ( error_seen )
		exit( 1 );

	if ( !tblopen( oflag ) )
		exit ( 2 );

	if ( flags & pFLAG )
		do_pflag();
	else {
		select();
		qsort( (char *)entryqptr, (unsigned)entrycnt,
			(unsigned)sizeof( ENTRY ), compar );
		display();
	}

	TLclose( tid );
	exit( 0 );

}

/* Remove blanks and commas from string */
void
compress( string )
char *string;
{
	int r = 0;
	int w = 0;

	while ( string[r] != NULL ) {
		if ((string[r] != ',') && (string[r] != ' '))
			string[w++] = string[r];
		r++;
	}
}

/* Validate the form of job ids, warning that invalid ones will be ignored. */
int
valid_jids()
{
	char *jidptr;

	int slot = 0;
	int cur_jid = 0;
	int is_bkjobid();

	while ( ( jidptr = (*j_argv)[cur_jid++] ) != NULL ) {
		if ( is_bkjobid( jidptr ) )
			(*j_argv)[slot++] = jidptr;
		else
			bkerror( stderr, ERROR10, jidptr );
	}
	(*j_argv)[slot] = NULL;
	if ( slot == 0 )
		return( FALSE );
	else
		return( TRUE );
}

/* Validate states input, warning that invalid ones will be ignored. */
int
valid_states()
{
	int slot = 0;
	int cur_state;

	for( cur_state = 0; cur_state < strlen( states ); cur_state++ )
		if ( strchr((char *)ST_STATES, states[cur_state]) != NULL ) {
			states[slot++] = states[cur_state];

			if ( (states[cur_state] == 'c') ||
			     (states[cur_state] == 'f') )
				prt_completed = TRUE;
		}
		else
			bkerror( stderr, ERROR11, states[cur_state] );
	states[slot] = NULL;
	if ( slot == 0 )
		return( FALSE );
	else
		return( TRUE );
}

/* Set rotation period in table to new value. */
void
do_pflag()
{
	int rc;
	int insert_rotation();

	if( (rc = insert_rotation( tid, period )) != 0 ) {
		bkerror( stderr, ERROR5, table, rc );
		exit( 2 );
	}
}

/* Read through table and add lines that match criteria to the entry queue. */
/* Criteria are applied from most to least restrictive. */
/* Table is read from the bottom to the top, as the only valid "apsw" entries */
/* occur at the bottom of the log, after the last ST_START entry.  If the */
/* bkdaemon process has died, then there are no valid "apsw" entries. */
/* Completed and failed entries are printed from the entire table.  Note */
/* that this means that entries that erroneously indicate they are still */
/* running will not be printed. */
void
select()
{
	register int entryno;
	int rc;
	int bkm_init();
	int matchone();
	int matchstates();

	void done_states();

	ENTRY eptr;
	ENTRY nextentry();

	unsigned char *fldval;

	eptr = nextentry();
	if ( eptr == NULL ) {
		bkerror( stderr, ERROR14 );
		TLclose( tid );
		exit( 2 );
	}
	entryno = TLEND;

	/* set flag if daemon not running */
#ifdef DEBUG
	if ( !daemon ) {
#else
	if ( bkm_init( BKNAME, 0 ) == -1 ) {
#endif
		prt_running = FALSE;
		if ( prt_completed )
			done_states();
	}

	while ( (prt_running || prt_completed) &&
		(rc = TLread( tid, entryno--, eptr )) != TLBADENTRY ) {

		if ( (rc == TLBADID) || (rc == TLARGS) || (rc == TLDIFFFORMAT)
			|| (rc == TLNOMEMORY) ) {
			bkerror( stderr, ERROR7, entryno-1, rc );
			TLclose( tid );
			exit( 2 );
		}

		/* Ignore comment lines. */
		if ( TLgetfield( tid, eptr, TLCOMMENT ) != NULL )
			continue;

		/* Check for START/STOP entries to determine whether to keep */
		/* printing apsw entries */
		if ( prt_running ) {
			fldval = TLgetfield( tid, eptr, ST_JOBID );
			if ( (strcmp( (char *)fldval, (char *)ST_STOP ) == NULL) ||
			     (strcmp ((char *)fldval, (char *)ST_START) == NULL) ) {
				prt_running = FALSE;
				if ( prt_completed ) {
					done_states();
				}
			}
		}

		/* If there's nothing left to print, get out of this loop. */
		if ( !( prt_running || prt_completed ) )
			break;

		if ( !matchstates( eptr, entryno ) )
			continue;

		if ( (j_argv != NULL) && ((*j_argv)[0] != NULL) )
			if ( !matchone( j_argv, eptr, ST_JOBID ) )
				continue;

		if ( (u_argv != NULL) && ((*u_argv)[0] != NULL) ) 
			if ( !matchone( u_argv, eptr, ST_UID ) )
				continue;

		eptr = nextentry();
		if ( eptr == NULL ) {
			bkerror( stderr, ERROR14 );
			TLclose( tid );
			exit( 2 );
		}
	}

	/* Free last (unused) entry */
	*(entryqptr + entrycnt) = (ENTRY)NULL;
	TLfreeentry( tid, eptr );
}

/* Returns pointer to next entry structure in the entry queue.  Only called */
/* when previously returned pointer has been used to hold a table entry. */
/* Takes care of allocating more space when number of entries exceeds number */
/* of allocated slots. */
ENTRY
nextentry()
{
	void *realloc();

	entrycnt++;

	if ( (entrycnt > 0) && (entrycnt % EMALLOCSZ == 0) ) {
		newsize *= ((entrycnt / EMALLOCSZ) + 1);
		entryqptr = (ENTRY *)realloc( (char *)entryqptr, (unsigned)newsize );
	}

	*(entryqptr + entrycnt) = TLgetentry( tid );

	return( *(entryqptr + entrycnt) );
}

/* Print header, if required, then print table values. */
void
display()
{
	void prt_header();
	void prt_nowrhead();
	void prt_values();

	if ( !(flags & hFLAG) )
		if ( flags & fFLAG )
			prt_nowrhead();
		else
			prt_header();

	prt_values();
}

/* Set up array of field values to be printed and then print them, */
/* working through the sorted queue one entry at a time. */
void
prt_values()
{
	unsigned char *prvalues[PRFLDCNT];
	unsigned char *destdev;
	unsigned char *userid;
	unsigned char *sptr;
	char *tptr;

	int i;
	int atoi();

	ENTRY eptr;

	time_t date;
	long strtol();

	struct passwd *pwp;

	void prt_wrap();
	void prt_nowrap();

	for ( i = 0; i < entrycnt; i++ ) {
		eptr = *(entryqptr + i);

		prvalues[0] = TLgetfield( tid, eptr, ST_JOBID );
		prvalues[1] = TLgetfield( tid, eptr, ST_TAG ); 

		/* Convert uid numeric string into user's login */
		userid = TLgetfield( tid, eptr, ST_UID );
		if( *userid == NULL )
			prvalues[2] = (unsigned char *)NULSTR;
		else {
			pwp = getpwuid( atoi( userid ) );
			prvalues[2] = (unsigned char *)pwp->pw_name;
		}
		prvalues[3] = TLgetfield( tid, eptr, ST_ONAME ); 
		prvalues[4] = TLgetfield( tid, eptr, ST_ODEVICE ); 

		/* Convert character representation of time_t date into a date */
		/* string */
		tptr = (char *)TLgetfield( tid, eptr, ST_STARTTIME );
		if ( (tptr == (char *)NULL) || (*tptr == NULL) ) {
			tbuf[0] = DASH;
			tbuf[1] = (char)NULL;
		}
		else {
			date = (time_t) strtol( tptr, (char **)NULL, 16 );
			if ( date == (time_t)0 ) {
				tbuf[0] = DASH;
				tbuf[1] = (char)NULL;
			}
			else cftime( tbuf, tfmt, &date );
		}
		prvalues[5] = (unsigned char *)tbuf;

		/* if the ddev field is null, print the dgroup */
		destdev = TLgetfield( tid, eptr, ST_DDEVICE );
		if ( *destdev == NULL )
			destdev = TLgetfield( tid, eptr, ST_DGROUP );
			if ( *destdev  == NULL )
				destdev = (unsigned char *)NULSTR;
		prvalues[6] = destdev;

		sptr = TLgetfield( tid, eptr, ST_STATUS );
		if ( *sptr == NULL )
			prvalues[7] = (unsigned char *)NULSTR;
		else {
			if ( strcmp( (char *)sptr, (char *)ST_ACTIVE ) == 0 )
				prvalues[7] = ACTIVE;
			else if ( strcmp( (char *)sptr, (char *)ST_PENDING ) == 0 )
				prvalues[7] = PENDING;
			else if ( strcmp( (char *)sptr, (char *)ST_WAITING ) == 0 )
				prvalues[7] = WAITING;
			else if ( strcmp( (char *)sptr, (char *)ST_HALTED ) == 0 )
				prvalues[7] = SUSPENDED;
			else if ( strcmp( (char *)sptr, (char *)ST_FAILED ) == 0 )
				prvalues[7] = FAILED;
			else if ( strcmp( (char *)sptr, (char *)ST_SUCCESS ) == 0 )
				prvalues[7] = COMPLETED;
		}

		if ( flags & fFLAG )
			prt_nowrap( PRFLDCNT, prvalues, fld_sep );
		else
			prt_wrap( PRFLDCNT, prvalues, prlens );
	}
}


/* Routine converts user (login) strings to string representations of (numeric) */
/* uids.  Routine reports invalid users, i.e., those that could not be found in */
/* the password file, but continues processing.  The u_argv pointer ends up */
/* pointing to the addresses of these converted uid strings. */
int
conv_users()
{
	int nusers = 0;
	int nchars;
	int userno;
	int slot = 0;

	char *uids;
	char *cur_uidp;
	char *malloc();

	struct passwd *pwp;

	while ( (*u_argv)[nusers] != NULL ) nusers++;

	if ((uids = malloc( nusers * UIDLEN * sizeof( char ) )) == NULL) {
		bkerror( stderr, ERROR8 );
		exit ( 2 );
	}
	cur_uidp = uids;

	for ( userno = 0; userno < nusers; userno++ ) {
		if ( (pwp = getpwnam( (*u_argv)[userno] )) != NULL ) {
			nchars = sprintf( cur_uidp, "%d", pwp->pw_uid );
			(*u_argv)[slot++] = cur_uidp;
			cur_uidp += nchars + 1;
		}
		else
			bkerror( stderr, ERROR10, (*u_argv)[userno] );
	}

	(*u_argv)[slot] = NULL;
	if ( slot == 0 )
		return( FALSE );
	else
		return( TRUE );
}

/* Synopsis of command invocation. */
void
synopsis()
{
	(void)fprintf( stderr, "%s [-ah] [-f field_separator] [-j jobids] [-s states] [-u users]\n",
			brcmdname );
	(void)fprintf( stderr, "%s -p period\n", brcmdname );
}

/* Open bkstatus table. */
int
tblopen( oflag )
{
	int rc;
	int is_new;
	int insert_format();
	int stat();

	char *bk_get_statlog_path();

	struct stat buf;
	struct TLdesc description;

	strncpy( (char *)&description, "", sizeof( struct TLdesc ));
	table = (unsigned char *)bk_get_statlog_path();
	is_new = (stat( (char *)table, &buf ) != 0);

	if ( is_new ) {
		if ( oflag == O_RDONLY ) {
			bkerror( stderr, ERROR15, table );
			return( FALSE );
		}
		else
			description.td_format = ST_ENTRY_F;
	}

	if (( rc = TLopen( &tid, table, &description, oflag, 0777 )) != TLOK
		&& rc != TLBADFS ) {
		if ( rc == TLDIFFFORMAT )
			bkerror( stderr, ERROR9, table ); /* warning */
		else {
			if ( rc == TLFAILED ) perror( brcmdname );
			else bkerror( stderr, ERROR6, table, rc );
			return ( FALSE );
		}
	}
	if ( is_new )
		(void)insert_format( tid, ST_ENTRY_F );
	return ( TRUE );
}


/* Print header */
void
prt_header()
{
	int dashes = 0;
	int i;


	for (i = 0; i < PRFLDCNT; i++ )
		dashes += prlens[i];
	dashes += PRFLDCNT - 1;

	(void)fprintf( stdout,
		"\n%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s\n",
		prlens[0], "Jobid",
		prlens[1], "Tag",
		prlens[2], "User",
		prlens[3], "Oname",
		prlens[4], "Odevice",
		prlens[5], "Start Time",
		prlens[6], "Dest",
		prlens[7], "Status" );

	for ( i=0; i < dashes; i++ )
		(void)fprintf( stdout, "-" );
	(void)fprintf( stdout, "\n" );
}

void
prt_nowrhead()
{
	(void)fprintf( stdout, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n",
		"Jobid", fld_sep, "Tag", fld_sep, "User",
		fld_sep, "Oname", fld_sep, "Odevice", fld_sep, "Start Time",
		fld_sep, "Dest", fld_sep, "Status" );
}

/* Routine determines whether the field value in the current line of the */
/* table matches at least one of the set of values passed in (via argvp). */
int
matchone( argvp, eptr, fldname )
argv_t *argvp;
ENTRY eptr;
unsigned char *fldname;
{
	int i;
	unsigned char *fldval;

	/* A null field value cannot match what the user entered. */
	fldval = TLgetfield( tid, eptr, fldname );
	if( (fldval == NULL ) || (*fldval == NULL) )
		return( FALSE );

	i = 0;
	while( (*argvp)[i] != NULL ) {
		if ( strcmp( (char *)fldval, (*argvp)[i++] ) == 0 )
			return( TRUE );
	}
	return( FALSE );
}

/* Routine determines whether the status field value in the current line of the */
/* table matches at least one of the states the user specified or defaulted to. */
/* NOTE: if the prt_running is false (daemon died for some reason, leaving */
/* some entries in an inconsistent state) and there are entries with active, */
/* pending, waiting or suspended, those entries should be printed as having */
/* failed.  The entry is printed as failed, but the actual status in the    */
/* table is NOT changed. */
int
matchstates( eptr, entryno )
ENTRY eptr;
int entryno;
{
	int i;
	int rc;
	unsigned char *fldval;

	/* A null field value cannot match what the user entered. */
	fldval = TLgetfield( tid, eptr, ST_STATUS );
	if( (fldval == NULL) || (*fldval == NULL) )
		return( FALSE );

	i = 0;
	while( states[i] != (char)NULL )
		switch (states[i++]) {
		case 'a':
			if ( strcmp( (char *)fldval, (char *)ST_ACTIVE ) == 0 )
				return( TRUE );
			break;

		case 'c':
			if ( strcmp( (char *)fldval, (char *)ST_SUCCESS ) == 0 )
				return( TRUE );
			break;

		case 'f':
			if ( !prt_running &&
			     strcmp( (char *)fldval, (char *)ST_SUCCESS ) != 0 ) {
				if ( (rc = TLassign( tid, eptr, ST_STATUS, ST_FAILED ))
					!= TLOK ) {
					bkerror( stderr, ERROR17, entryno, rc );
					exit( 2 );
				}
				return( TRUE );
			}
			else if ( strcmp( (char *)fldval, (char *)ST_FAILED ) == 0 )
				return( TRUE );
			break;

		case 'p':
			if ( strcmp( (char *)fldval, (char *)ST_PENDING ) == 0 )
				return( TRUE );
			break;

		case 's':
			if ( strcmp( (char *)fldval, (char *)ST_HALTED ) == 0 )
				return( TRUE );
			break;

		case 'w':
			if ( strcmp( (char *)fldval, (char *)ST_WAITING ) == 0 )
				return( TRUE );
			break;

		default:
			bkerror( stderr, ERROR13, states[i-1] );
			exit( 2 );
		}
	return( FALSE );
}

/* Function returns 0 if objects pointed to by arguments are equal, <0 if arg1 */
/* should appear before arg2 in a sort, >0 if arg2 should appear first. */
/* Currently sorts table entries first by jobid, then by start time (most */
/* recent job first) of each backup operation.  (Note that a backup job may */
/* consist of more than one backup operations.)  Backup operations whose status */
/* is pending will have null start time values and should appear before jobs */
/* which have nonnull start times.  (Their start times will be later than all */
/* entries in which a start time currently appears.) */
compar( e1ptr, e2ptr )
ENTRY *e1ptr;
ENTRY *e2ptr;
{
	unsigned char *j1;
	unsigned char *j2;
	unsigned char *t1;
	unsigned char *t2;

	char *digits = "0123456789";

	int nj1;
	int nj2;

	long strtol();
	int atoi();

	j1 = TLgetfield( tid, *e1ptr, ST_JOBID );
	j2 = TLgetfield( tid, *e2ptr, ST_JOBID );
	nj1 = atoi( strpbrk( (char *)j1, digits ) );
	nj2 = atoi( strpbrk( (char *)j2, digits ) );

	if ( nj1 == nj2 ) {
		t1 = TLgetfield( tid, *e1ptr, ST_STARTTIME );
		t2 = TLgetfield( tid, *e2ptr, ST_STARTTIME );

		if ( strcmp( (char *)t1, (char *)t2 ) == 0 )
			return( 0 );
		else {
			if ( *t1 == NULL )
				return( -1 );
			else if ( *t2 == NULL )
				return( 1 );
			else
				return( strtol( t2, (char **)NULL, 16 ) -
				     strtol( t1, (char **)NULL, 16 ) );
		}
	}
	else return( nj1 - nj2 );
}

/* eliminate running states, leaving only the completed states */
void
done_states()
{
	int i,j;

	i = 0;
	j = 0;
	while( states[i] != NULL ) {
		if ( states[i] == 'c' || states[i] == 'f' )
			states[j++] = states[i];
		i++;
	}
	states[j] = NULL;
}
