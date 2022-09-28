/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsstatus.d/rsstatus.c	1.8.2.1"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <table.h>
#include <rsstatus.h>
#include <bktypes.h>
#include <errors.h>

#define TRUE	1
#define FALSE	0

#define SP	" "
#define FS	'\t'
#define NULSTR	""
#define TOCMARK	"* "

#define DTYPE	(unsigned char *)"dtype"

/* Length in characters of string representation (including terminating null) */
/* of largest possible uid (decimal), assuming a 32-bit int. */
#define UIDLEN	11

/* Field counts for administrator and user displays. */
#define PRFLDCNT	9
#define UPRFLDCNT	5

extern struct passwd *getpwuid();

/* rsstatus table */
unsigned char *table;

/* invoking user's login - for non-administrator invocation */
char thisuser[9];

/* name of this command */
char *brcmdname;

/* user display (TRUE) or administrator display (FALSE) */
int udisp;

/* table id for rsstatus table */
int tid;

/* print header? */
int header = TRUE;

/* wrap field values? */
int wrap = TRUE;

/* field separator character */
char fld_sep;

/* pointer to array of pointers to job id strings user entered */
argv_t *j_argv;

/* pointer to array of pointers to user name strings user entered */
argv_t *u_argv;

/* userid - string form of uid */
unsigned char *userid;

/* destination device type user entered */
unsigned char *finddtype = NULL;

/* pointer to array of pointers to destination volume label strings user entered */
argv_t *dl_argv = NULL;

/* values to be printed on a given line */
unsigned char *prvalues[PRFLDCNT];

/* field lengths for wrapped printing */
/* Jobid, Login, Object, Date, Target, Bkup date, Method, Dtype, Volume labels */
int aprlens[PRFLDCNT] = { 11, 8, 7, 6, 7, 8, 7, 8, 7 };
int uprlens[UPRFLDCNT] = { 11, 8, 14, 24, 14 };

/* high-water mark for number of characters allocated for TOC volume label */
/* processing */
int labelchars = 0;

/* pointer to table-of-contents volume label string */
char *tocstring;

/* buffer for time conversion */
char tbuf[26];

/* formats in which date and time strings are printed */
char *ufdatefmt = "%a %b %d %Y %H:%M:%S";
char *afdatefmt = "%b %d %Y %H:%M:%S";
char *archfmt = "%a %b %d %Y";

/* pointer to an entry structure */
ENTRY eptr;

void exit();
void bkerror();

/* Program displays status of selected pending restore requests.  It allows */
/* administrators to display status for any restore requests, regardless of */
/* originating userid.  Non-administrators may only display information about */
/* their own restore requests.  Administrators also receive more information */
/* about each request than do non-administrators. */
/* Currently, administrators are determined by checking effective userid - it */
/* must be root.  The "admin_auth" routine should be replaced by code that */
/* uses the authorization software to determine whether an invoker is an */
/* authorized administrator or not. */
main (argc, argv)
int argc;
char *argv[];
{
	int optsok = FALSE;
	int admin_auth();
	int parse_opts();
	int tblopen();
	int uparse_opts();

	void prt_header();
	void prt_nowrhead();
	void prt_values();

	brcmdname = (char *)argv[0];

	udisp = !( admin_auth() );

	if ( udisp )
		optsok = uparse_opts(argc, argv);
	else
		optsok = parse_opts(argc, argv);

	if ( !optsok )
		exit ( 1 );

	if ( !tblopen() )
		exit ( 2 );

	if ( header )
		if ( wrap )
			prt_header();
		else
			prt_nowrhead();

	prt_values();

	TLclose( tid );

	exit ( 0 );
}

/* Routine parses valid options for non-administrator version. */
/* It saves the invoking user's login, as non-administrators can */
/* only look at their own jobs. */
/* It returns TRUE if parsing was fine, FALSE if there was any error. */
/* It exits with 1 if user requests a synopsis (-?). */
int
uparse_opts(argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	int error_seen = FALSE;
	int c;
	int getopt();

	unsigned short getuid();

	char *j_arg;

	void synopsis();
	void valid_jids();

	argv_t *s_to_argv();

	while (( c = getopt( argc, argv, "f:hj:?" )) != -1 )
		switch ( c ) {
		case 'f':
			if ( strlen( optarg ) > 1 ) {
				bkerror( stderr, ERROR8, optarg );
				error_seen = TRUE;
			}
			else {
				if ( *optarg == NULL )
					fld_sep = FS;
				else
					fld_sep = optarg[0];
				wrap = FALSE;
			}
			break;

		case 'h':
			header = FALSE;
			break;

		case 'j':
			j_arg = optarg;
			j_argv = s_to_argv( j_arg, ", " );
			valid_jids( j_argv );
			break;

		case '?':
			synopsis();
			exit ( 1 );
			/* NOTREACHED */
			break;

		default:
			bkerror( stderr, ERROR0, c );
			error_seen = TRUE;
			break;
		}

	/* Check for non-option, invalid arguments */
	if ( optind < argc ) {
		bkerror( stderr, ERROR9, argv[optind] );
		synopsis();
		error_seen = TRUE;
	}

	if ( error_seen )
		return( FALSE );

	/* Save uid, since invoking user can only see his/her own jobs. */
	sprintf( thisuser, "%d", getuid() );
	u_argv = s_to_argv( thisuser, ", " );

	return ( TRUE );
}

/* Routine parses options for administrator version of the command. */
int
parse_opts(argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	argv_t *s_to_argv();

	void conv_users();
	void p_d_arg();
	void synopsis();
	void valid_jids();

	int error_seen = FALSE;
	int c;
	int getopt();

	char *d_arg;
	char *j_arg;
	char *u_arg;

	while (( c = getopt( argc, argv, "d:f:hj:u:?" )) != -1 )
		switch ( c ) {
		case 'd':
			d_arg = optarg;
			p_d_arg( strdup( d_arg ) );
			break;

		case 'f':
			if ( strlen( optarg ) > 1 ) {
				bkerror( stderr, ERROR8, optarg );
				error_seen = TRUE;
			}
			else {
				if ( *optarg == NULL )
					fld_sep = FS;
				else
					fld_sep = optarg[0];
				wrap = FALSE;
			}
			break;

		case 'h':
			header = FALSE;
			break;

		case 'j':
			j_arg = optarg;
			j_argv = s_to_argv( j_arg, ", " );
			valid_jids( j_argv);
			break;

		case 'u':
			u_arg = optarg;
			u_argv = s_to_argv( u_arg, ", " );
			conv_users();
			break;

		case '?':
			synopsis();
			exit ( 1 );
			/* NOTREACHED */
			break;

		default:
			bkerror( stderr, ERROR0, c );
			error_seen = TRUE;
			break;
		}

	/* Check for non-option, invalid arguments */
	if ( optind < argc ) {
		bkerror( stderr, ERROR9, argv[optind] );
		synopsis();
		error_seen = TRUE;
	}

	if ( error_seen )
		return ( FALSE );

	return ( TRUE );
}

void
valid_jids( argvp )
argv_t *argvp;
{
	char *pidptr;

	int slot = 0;
	int cur_pid = 0;
	int is_rsjobid();

	/* Validate that jobids are legal (in form only) - warn that illegal */
	/* ones are ignored. */
	while ( ( pidptr = (*argvp)[cur_pid++] ) != NULL ) {
		if ( is_rsjobid( pidptr ) )
			(*argvp)[slot++] = pidptr;
		else
			bkerror( stderr, ERROR1, pidptr );
	}
	(*argvp)[slot] = NULL;
}

/* Routine parses and validates the destination device argument, which is of */
/* the form [dtype]:[dlabels].  The dlabels are a list of volume labels. */
/* If that field is null, it implies any volume labels are acceptable. */
void
p_d_arg( string )
char *string;
{
	unsigned char *comma_sep();
	unsigned char *dlabels;

	char *dlptr;
	char *bkstrtok();

	int slot = 0;
	int cur_dl = 0;

	argv_t *s_to_argv();

	string = (char *)comma_sep( string );
	finddtype = (unsigned char *) bkstrtok( string, ":" );

	dlabels = (unsigned char *)bkstrtok( NULL, ":" );
	if ( dlabels ) {
		dl_argv = s_to_argv( dlabels, "," );

		/* Validate that dlabels are legal - warn that illegal */
		/* names are ignored. */
		while ( ( dlptr = (*dl_argv)[cur_dl++] ) != NULL ) {
			if ( strlen( dlptr ) > 6 )
				bkerror( stderr, ERROR1, dlptr );
			else
				(*dl_argv)[slot++] = dlptr;
		}
		(*dl_argv)[slot] = NULL;
	}
}

/* Routine converts user (login) strings to string representations of (numeric) */
/* uids.  Routine reports invalid users, i.e., those that could not be found in */
/* the password file, but continues processing.  The u_argv pointer ends up */
/* pointing to the addresses of these converted uid strings. */
void
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
		bkerror( stderr, ERROR7 );
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
			bkerror( stderr, ERROR1, (*u_argv)[userno] );
	}

	(*u_argv)[slot] = NULL;
}

/* Synopsis of command invocation. */
void
synopsis()
{
	if ( udisp )
		fprintf( stderr, "%s [-h] [-j jobids] [-f field_separator]\n", brcmdname );
	else
		fprintf( stderr, "%s [-h] [-d ddev] [-f field_separator] [-j jobids] [-u users]\n",
			brcmdname );
}

/* Open rsstatus table. */
int
tblopen()
{
	int rc;
	int stat();
	extern int errno;

	struct stat buf;

	char *br_get_rsstatlog_path();

	table = (unsigned char *)br_get_rsstatlog_path();

	/* Make sure table exists.  TLopen does not return failure if table */
	/* doesn't exist, even if read-only flag is set. */
	if ( stat( (char *)table, &buf ) != 0 ) {
		bkerror( stderr, ERROR2, table, errno );
		exit( 2 );
	}

	if (( rc = TLopen( &tid, table, (struct TLdesc *)NULL, O_RDONLY )) != TLOK
		&& rc != TLBADFS ) {
		if ( rc == TLDIFFFORMAT )
			bkerror( stderr, ERROR5, table ); /* warning */
		else {
			if ( rc == TLFAILED ) perror( brcmdname );
			else bkerror( stderr, ERROR4, table, rc );
			return ( FALSE );
		}
	}
	return ( TRUE );
}

/* Print header */
void
prt_header()
{
	int dashes = 0;
	int i;


	if ( udisp ) {
		for (i = 0; i < UPRFLDCNT; i++ )
			dashes += uprlens[i];
		dashes += UPRFLDCNT - 1;

		fprintf( stdout, "\n%-*s %-*s %-*s %-*s %-*s",
			uprlens[0],"Jobid",
			uprlens[1],"Login",
			uprlens[2],"File",
			uprlens[3],"Date",
			uprlens[4],"Target");
	}
	else {
		for (i = 0; i < PRFLDCNT; i++ )
			dashes += aprlens[i];
		dashes += PRFLDCNT - 1;

		fprintf( stdout,
			"%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s",
			aprlens[0], "Jobid",
			aprlens[1], "Login",
			aprlens[2], "File",
			aprlens[3], "Date",
			aprlens[4], "Target",
			aprlens[5], "Bkp date",
			aprlens[6], "Method",
			aprlens[7], "Dtype",
			aprlens[8], "Labels" );
	}

	fprintf( stdout, "\n" );
	for ( i=0; i < dashes; i++ )
		fprintf( stdout, "-" );
	fprintf( stdout, "\n" );
}

void
prt_nowrhead()
{
	if ( udisp )
		fprintf( stdout, "%s%c%s%c%s%c%s%c%s",
			"Jobid", fld_sep, "Login", fld_sep, "File",
			fld_sep, "Date", fld_sep, "Target");
	else
		fprintf( stdout, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s",
			"Jobid", fld_sep, "Login", fld_sep, "File",
			fld_sep, "Date", fld_sep, "Target", fld_sep, "Bkp date",
			fld_sep, "Method", fld_sep, "Dtype", fld_sep, "Labels" );
	fprintf( stdout, "\n");
}

/* Go through rsstatus table and print values from selected lines.  */
/* Lines are selected if all the criteria the user gave are satisfied. */
/* Criteria are applied from most to least restrictive. */
void
prt_values()
{
	register int entryno;
	int rc;
	int bkgetchars();
	int matchone();

	unsigned char *dchars;
	unsigned char *value;

	char *malloc();

	void prt_entry();

	if ( !(eptr = TLgetentry( tid )) ) {
		bkerror( stderr, ERROR6 );
		TLclose( tid );
		exit ( 2 );
	}

	entryno = 1;
	while ( (rc = TLread( tid, entryno++, eptr )) != TLBADENTRY ) {

		if ( (rc == TLBADID) || (rc == TLARGS) || (rc == TLDIFFFORMAT)
			|| (rc == TLNOMEMORY) ) {
			bkerror( stderr, ERROR3, entryno-1, rc );
			TLclose( tid );
			exit ( 2 );
		}

		/* Ignore comment lines. */
		if ( TLgetfield( tid, eptr, TLCOMMENT ) != NULL )
			continue;

		if ( (j_argv != NULL) && ((*j_argv)[0] != NULL) )
			if ( !matchone( j_argv, RST_JOBID ) )
				continue;

		if ( (u_argv != NULL) && ((*u_argv)[0] != NULL) )
			if ( !matchone( u_argv, RST_UID ) )
				continue;

		if ( (dl_argv != NULL) && ((*dl_argv)[0] != NULL) )
			if ( !matchone( dl_argv, RST_DLABEL ) )
				continue;

		/* If device type in table is not the one the user is */
		/* interested in, ignore this line. */
		if ( finddtype ) {
			dchars = TLgetfield( tid, eptr, RST_DCHAR );

			if( (value =
			    (unsigned char *)malloc( (unsigned)strlen( (char *)dchars ) ))
				== NULL ) {
				bkerror( stderr, ERROR10 );
				exit( 2 );
			}

			if ( !bkgetchars( dchars, DTYPE, value) )
				continue;

			if ( strcmp( (char *)value, (char *)finddtype ) != 0 )
				continue;
		}

		prt_entry();
	}
}

/* Routine determines whether the field value in the current line of the */
/* table matches at least one of the set of values passed in (via argvp). */
int
matchone( argvp, fldname )
argv_t *argvp;
unsigned char *fldname;
{
	int i;
	unsigned char *fldval;

	/* A null field value cannot match what the user entered. */
	if( (fldval = TLgetfield( tid, eptr, fldname ) ) == NULL )
		return( FALSE );

	i = 0;
	while( (*argvp)[i] != NULL ) {
		if ( strcmp( (char *)fldval, (*argvp)[i++] ) == 0 )
			return( TRUE );
	}
	return( FALSE );
}

/* Print values from entry in the table. */
void
prt_entry()
{
	void prt_ufields();
	void prt_afields();

	if ( udisp )
		prt_ufields();
	else
		prt_afields();
}

/* Print values that a user is to see from a selected line in the table. */
void
prt_ufields()
{
	void ffield_setup();
	void prt_wrap();
	void prt_nowrap();

	/* Set up field values to print. */
	ffield_setup( ufdatefmt );

	if ( wrap )
		prt_wrap( UPRFLDCNT, prvalues, uprlens );
	else
		prt_nowrap( UPRFLDCNT, prvalues, fld_sep );

}

/* Print values that an administrator is to see from a selected line in the table. */
void
prt_afields()
{
	unsigned char *dchar;
	unsigned char *devtype = NULL;
	unsigned char *labels;
	unsigned char *toclabels();

	char btbuf[26];
	char *tptr;
	char *malloc();

	time_t date;
	long strtol();

	int bkgetchars();

	void ffield_setup();
	void prt_nowrap();
	void prt_wrap();

	/* Set up first field values. */
	ffield_setup( afdatefmt );

	/* Convert time_t representation of date to a string. */
	if ( ((tptr = (char *)TLgetfield( tid, eptr, RST_ARCHDATE )) == NULL) ||
		(*tptr == NULL) )
		btbuf[0] = NULL;
	else {
		date = (time_t) strtol( tptr, (char **)NULL, 16);
		cftime( btbuf, archfmt, &date );
	}
	prvalues[5] = (unsigned char *)btbuf;

	/* Method */
	prvalues[6] = TLgetfield( tid, eptr, RST_METHOD );

	/* Destination device type */
	dchar = TLgetfield( tid, eptr, RST_DCHAR );
	if ( *dchar != (unsigned char)NULL ) {
		if( (devtype =
		    (unsigned char *)malloc( (unsigned)strlen( (char *)dchar ) ))
		    == NULL ) {
			bkerror( stderr, ERROR10 );
			exit( 2 );
		}
		if ( !bkgetchars( dchar, DTYPE, devtype ) )
			*devtype = (unsigned char)NULL;
	}
	else devtype = (unsigned char *)NULSTR;
	prvalues[7] = devtype;

	/* Data volume labels */
	/* If the data volume labels field is null, then check for TOC */
	/* volume labels.  If they exist, preface each with an asterisk */
	/* before printing. */
	prvalues[8] = TLgetfield( tid, eptr, RST_DLABEL );
	if ( *(prvalues[8]) == NULL) {
		labels = TLgetfield( tid, eptr, RST_TLABEL );
		if ( *labels != NULL )
			prvalues[8] = toclabels( labels );
	}


	/* Print fields administrator should see. */
	if ( wrap )
		prt_wrap( PRFLDCNT, prvalues, aprlens );
	else
		prt_nowrap( PRFLDCNT, prvalues, fld_sep );

}

void
ffield_setup( tfmtstr )
char *tfmtstr;
{
	unsigned char *type;

	char *tptr;

	time_t date;
	long strtol();

	struct passwd *pwp;

	int atoi();

	/* Jobid */
	prvalues[0] = TLgetfield( tid, eptr, RST_JOBID );

	/* Login - use uid if id is not in passwd file */
	if( ((userid = TLgetfield( tid, eptr, RST_UID )) == NULL) ||
		(*userid == NULL) )
		prvalues[1] = (unsigned char *)NULSTR;
	else {
		if ( (pwp = getpwuid( atoi( userid ))) == NULL )
			prvalues[1] = userid;
		else
			prvalues[1] = (unsigned char *)pwp->pw_name;
	}

	/* Oname for restore object */
	prvalues[2] = TLgetfield( tid, eptr, RST_OBJECT );

	/* Date of object to be restored */
	if ( ((tptr = (char *)TLgetfield( tid, eptr, RST_FDATE )) == NULL) ||
		(*tptr == NULL) )
		tbuf[0] = NULL;
	else {
		date = (time_t) strtol( tptr, (char **)NULL, 16);
		cftime( tbuf, tfmtstr, &date );
	}
	prvalues[3] = (unsigned char *)tbuf;

	/* Target  - this is the contents of the "target" field only for objects */
	/* of type file (F) or directory (D).  File system, data partition and */
	/* entire disk restores write the target object into the "redev" field. */
	type = TLgetfield( tid, eptr, RST_TYPE );
	if ( (strcmp( (char *)type, "F" ) == 0) ||
		(strcmp( (char *)type, "D" ) == 0) )
		prvalues[4] = TLgetfield( tid, eptr, RST_TARGET );
	else
		prvalues[4] = TLgetfield( tid, eptr, RST_REDEV );
}

/* Routine breaks out toc labels from a comma- or blank-separated string */
/* and prefaces each by a mark to indicate that these are toc volumes */
static unsigned char *
toclabels( labels )
unsigned char *labels;
{
	char *tlabel;
	char *malloc();

	int tocstrlen;
	int tsend;

	/* WARNING - this value is calculated as the worst case required    */
	/* to hold the string of labels, prefaced by a 2-character tocmark, */
	/* separated by commas and terminated by a null character.  If a    */
	/* longer tocmark is substituted, this will need to be recalculated.*/
	tocstrlen = 2 * strlen( (char *)labels ) + 3;

	if ( labelchars < tocstrlen ) {
		if ( tocstring != NULL )
			free( tocstring );
		if ( (tocstring = malloc( tocstrlen )) == NULL ) {
			bkerror( stderr, ERROR10 );
			TLclose( tid );
			exit( 2 );
		}
		labelchars = tocstrlen;
	}

	tlabel = strtok( (char *)labels, ", " );
	tsend = 0;
	while ( tlabel != NULL ) {
		sprintf( tocstring + tsend, "%s%s,", TOCMARK, tlabel );
		tsend = strlen( tocstring );
		tlabel = strtok( NULL, ", " );
	}
	tocstring[strlen( tocstring ) - 1] = NULL;

	return( (unsigned char *)tocstring );
}
