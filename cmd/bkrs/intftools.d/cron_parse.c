/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/cron_parse.c	1.2.2.1"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <bktypes.h>
#include <cperrs.h>

#define TRUE	1
#define FALSE	0

#define BUFSIZE	1024

#define SPACE	" "
#define POUND	"#"
#define STAR	"*"
#define ALL	"all"
#define AUTO	"auto"
#define BACK	"back"
#define YES	"yes"
#define NO	"no"
#define BKSCHED	"bksched"
#define BKMSG	"bkmsg"
#define MAXFLDCNT	7
#define IFLDCNT		4
#define BKFLDCNT	3
#define MFLDCNT		1
#define TIMELEN		5
#define MAXLINE		4	/* number of decimal digits for line numbers */
#define SCHMODE		0x1	/* backup schedule mode */
#define MSGMODE		0x2	/* reminder schedule mode */

/* command name */
char *brcmdname;

/* flag signalling mode of invocation */
int imode = 0;

/* print field lengths for line number, time, days of week, months */
int iprlens[IFLDCNT] = { 4, 5, 12, 6 };

/* print field lengths for backup schedule -  mode, notify and table */
int bkprlens[BKFLDCNT] = { 5, 6, 31 };

/* print field lengths for reminder schedule - onames */
int mprlens[MFLDCNT] = { 40 };

/* print field lengths for entire display */
int prlens[MAXFLDCNT];

/* total number of fields in display */
int fldcnt;

/* print value array */
unsigned char *prvalues[MAXFLDCNT];

/* field separator character */
char fld_sep;

/* don't wrap output if fld_sep is specified */
int wrap = TRUE;
	
/* backup mode - automated or background */
char *mode;

/* months field value */
char *months;

/* notify user when backup done? Y/N */
char *notify;

/* onames for reminder message */
char *onames;

/* backup register table */
char *table;

/* user to notify */
char *user;

void exit();
void bkerror();

/* Program parses a file of cron lines of the form: */
/* mins hours mdays months wkdays /etc/backup -t table [-a] [-m user] #bksched# */
/* or of the form: */
/* mins hours mdays months wkdays /usr/oam/bin/bkmsg onames #bkmsg# */
/* It displays the parsed values in human-readable form, with or without */
/* headers, as requested.  Lines can also come from stdin. */
main( argc, argv )
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	int c;
	int errflag = FALSE;
	int hdrs = TRUE;	/* headers? */
	int i;
	int j;
	int lineno = 0;
	int getopt();

	FILE *fp;	/* file with cron lines */
	
	char *cmdstr;		/* command string field value */
	char cronline[BUFSIZE];	/* input cron line from file */
	char *hours;		/* hours field value */
	char line[MAXLINE + 1];	/* char representation of line number */
	char *mdays;		/* days of month field value */
	char *mins;		/* minutes field value */
	char timestr[TIMELEN + 1];	/* combined minutes and hours */
	char *wkdays;		/* days of week field value */

	void parse_msg();
	void parse_sched();
	void prt_header();
	void prt_wrap();
	void synopsis();

	brcmdname = argv[0];

	while (( c = getopt( argc, argv, "f:hm:?" )) != -1 )
		switch ( c ) {
		case 'f':
			if( strlen( optarg ) > 1 ) {
				bkerror( stderr, ERROR4, optarg );
				errflag = TRUE;
			}
			else {
				if ( *optarg == NULL )
					fld_sep = ':';
				else
					fld_sep = optarg[0];
					wrap = FALSE;
			}
			break;
		case 'h':
			hdrs = FALSE;
			break;

		case 'm':
			if ( strcmp( optarg, BKSCHED ) == 0 ) {
				fldcnt = IFLDCNT + BKFLDCNT;
				imode |= SCHMODE;
			}
			else if ( strcmp( optarg, BKMSG ) == 0 ) {
				fldcnt = IFLDCNT + MFLDCNT;
				imode |= MSGMODE;
			}
			else {
				bkerror( stderr, ERROR5, optarg );
				errflag = TRUE;
			}
			break;

		case '?':
			synopsis();
			exit ( 0 );
			/* NOTREACHED */
			break;

		default:
			bkerror(stderr,ERROR0, c);
			errflag = TRUE;
			break;
	}
	if ( errflag )
		exit( 1 );
	
	if ( optind == argc )
		fp = stdin;
	else {
		if ( (fp = fopen( argv[optind], "r" )) == NULL ) {
			bkerror( stderr, ERROR3, argv[optind] );
			exit( 1 );
		}

		if ( ++optind != argc ) {
			bkerror( stderr, ERROR1 );
			synopsis();
			exit( 1 );
		}
	}

	for (i = 0; i < IFLDCNT; i++)
		prlens[i] = iprlens[i];

	for (j = 0, i = IFLDCNT; i < fldcnt; j++, i++) {
		if ( imode & SCHMODE )
			prlens[i] = bkprlens[j];
		else
			prlens[i] = mprlens[j];
	}

	if ( hdrs )
		prt_header();

	while( fgets( cronline, BUFSIZE, fp) != NULL ) {
		lineno++;

		mins = strtok( cronline, SPACE );
		hours = strtok( NULL, SPACE );
		mdays = strtok( NULL, SPACE );
		months = strtok( NULL, SPACE );
		wkdays = strtok( NULL, SPACE );
		cmdstr = strtok( NULL, POUND );

		if ( strcmp( mins, STAR ) == 0 )
			mins = ALL;
		if ( strcmp( hours, STAR ) == 0 )
			hours = ALL;
		if ( strcmp( months, STAR ) == 0 )
			months = ALL;
		if ( strcmp( wkdays, STAR ) == 0 )
			wkdays = ALL;

		if ( imode & SCHMODE )
			parse_sched( cmdstr );
		else
			parse_msg( cmdstr );

		sprintf( line, "%*d", iprlens[0], lineno );
		sprintf( timestr, "%s:%s", hours, mins );

		prvalues[0] = (unsigned char *)line;
		prvalues[1] = (unsigned char *)timestr;
		prvalues[2] = (unsigned char *)wkdays;
		prvalues[3] = (unsigned char *)months;

		if ( imode & SCHMODE ) {
			prvalues[4] = (unsigned char *)mode;
			prvalues[5] = (unsigned char *)notify;
			prvalues[6] = (unsigned char *)table;
		}
		else
			prvalues[4] = (unsigned char *)onames;

		if ( wrap )
			prt_wrap( fldcnt, prvalues, prlens );
		else
			for ( i = 0; i < fldcnt; i++ )
				fprintf( stdout, "%s%c", prvalues[i],
					( i < fldcnt - 1 ) ? fld_sep : '\n' );
	}
	exit ( 0 );
}

void
synopsis()
{
	fprintf(stdout, "Usage: %s [-h] [-f c] [-m mode] [file]\n", brcmdname );
}

void
prt_header()
{
	int dashes = 0;
	int i;

	for (i = 0; i < fldcnt; i++ )
		dashes += prlens[i];
	dashes += fldcnt - 1;

	(void)fprintf( stdout,
		"\n%-*s %-*s %-*s %-*s",
		iprlens[0], "Line",
		iprlens[1], "Time",
		iprlens[2], "Days of week",
		iprlens[3], "Months");

	if ( imode & SCHMODE )
		(void)fprintf( stdout,
			" %-*s %-*s %-*s\n",
			bkprlens[0], "Mode",
			bkprlens[1], "Notify",
			bkprlens[2], "Table" );
	else
		(void)fprintf( stdout, " %-*s\n", mprlens[0], "Onames");

	for ( i=0; i < dashes; i++ )
		(void)fprintf( stdout, "-" );
	(void)fprintf( stdout, "\n" );
}

void
parse_sched( cmdstr )
char *cmdstr;
{
	int i;

	argv_t *c_argv;	/* command string in argv-like structure */
	argv_t *s_to_argv();

	char *carg;	/* command arguments */

	mode = BACK;
	notify = NO;
	c_argv = s_to_argv( cmdstr, SPACE );

	i = 1;
	while( (carg = (*c_argv)[i++]) != NULL ) {
		if ( strcmp( carg, "-t" ) == 0 )
			table = (*c_argv)[i++];
		else if ( strcmp( carg, "-a" ) == 0 )
			mode = AUTO;
		else if ( strcmp( carg, "-m" ) == 0 ) {
			notify = YES;
			user = (*c_argv)[i++];
		}
		else {
			bkerror( stderr, ERROR2, carg );
			exit( 2 );
		}
	}
}

void
parse_msg( cmdstr )
char *cmdstr;
{

	(void) strtok( cmdstr, " " );
	onames = strtok( NULL, " " );
}
