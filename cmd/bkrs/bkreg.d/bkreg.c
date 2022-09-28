/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkreg.d/bkreg.c	1.15.4.5"

#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <table.h>
#include <errno.h>
#include <backup.h>
#include <bkreg.h>
#include <errors.h>

#define min( a, b ) ((a < b)? a: b)
#define max( a, b ) ((a > b)? a: b)

#define FLDSEP ':'

/* Max digits in period */
#define PDLEN	2

/* Max digits in priority */
#define MAXPLEN	11

/* Maximum number of columns in a vertical display */
#define MAXCOL	4

/* Flags to tell which options have been seen */
#define aFLAG	0x1
#define AFLAG	0x2
#define bFLAG	0x4
#define cFLAG	0x8
#define CFLAG	0x10
#define dFLAG	0x20
#define DFLAG	0x40
#define eFLAG	0x80
#define fFLAG	0x100
#define hFLAG	0x200
#define mFLAG	0x400
#define oFLAG	0x800
#define OFLAG	0x1000
#define pFLAG	0x2000
#define PFLAG	0x4000
#define rFLAG	0x8000
#define RFLAG	0x10000
#define sFLAG	0x20000
#define tFLAG	0x40000
#define vFLAG	0x80000
#define wFLAG	0x100000
#define NFLAGS	21

int nflags = NFLAGS;

/* FLAG to option mapping */
char *optname = "aAbcCdDefhmoOpPrRstvw";

/* The valid combinations of flags */
unsigned allowed[] = {
	(pFLAG | tFLAG | wFLAG),
	(aFLAG | tFLAG | oFLAG | cFLAG | mFLAG | bFLAG | dFLAG | DFLAG | PFLAG),
	(eFLAG | tFLAG | oFLAG | cFLAG | mFLAG | bFLAG | dFLAG | DFLAG | PFLAG),
	(rFLAG | tFLAG ),
	(CFLAG | hFLAG | vFLAG | tFLAG | fFLAG | cFLAG ),
	(AFLAG | tFLAG | cFLAG | hFLAG | sFLAG | vFLAG ),
	(OFLAG | tFLAG | cFLAG | hFLAG | sFLAG | vFLAG ),
	(RFLAG | tFLAG | cFLAG | hFLAG | sFLAG | vFLAG ),
	(tFLAG | cFLAG | hFLAG | sFLAG | vFLAG ),
	0
};
unsigned required[] = {
	(pFLAG),
	(aFLAG | oFLAG | cFLAG | mFLAG | dFLAG ),
	(eFLAG),
	(rFLAG),
	(CFLAG),
	(AFLAG),
	(OFLAG),
	(RFLAG),
	0
};

/* Options to tell which option arguments were invalid */
#define aERROR	0x1
#define bERROR	0x2
#define CERROR	0x4
#define dERROR	0x8
#define DERROR	0x10
#define eERROR	0x20
#define fERROR	0x40
#define mERROR	0x80
#define oERROR	0x100
#define pERROR	0x200
#define PERROR	0x400
#define rERROR	0x800
#define tERROR	0x1000

struct TLsearch TLsearches[ TL_MAXFIELDS ];
unsigned flags = 0;
char *demand = "demand";

/* Set up display field structure - used by all display options to */
/* determine which fields to display in what order and field sizes */

#define NDFLDS 16

int ndisp_flds = NDFLDS;
struct disp_fld {
	unsigned char *fldname; /* name of field in table file */
	unsigned char *coptname; /* name of field on command line */
	int fldlen;
	unsigned char *dispname;
} fields[] = {
	(unsigned char *)"cweek", (unsigned char *)"cweek", 2,
		(unsigned char *)"Cw",
	R_DAY, (unsigned char *)"days", 5,
		(unsigned char *)"Days",
	R_DCHAR, (unsigned char *)"dchar", 30,
		(unsigned char *)"Dcharacteristics",
	R_DDEVICE, (unsigned char *)"ddevice", 30,
		(unsigned char *)"Ddevice",
	R_DEPEND, (unsigned char *)"depend", 14,
		(unsigned char *)"Depends",
	R_DGROUP, (unsigned char *)"dgroup", 14,
		(unsigned char *)"Dgroup",
	R_DMNAME, (unsigned char *)"dlabel", 30,
		(unsigned char *)"Dlabel",
	R_METHOD, (unsigned char *)"method", 14,
		(unsigned char *)"Method",
	R_OPTIONS, (unsigned char *)"moptions", 10,
		(unsigned char *)"Options",
	R_ODEVICE, (unsigned char *)"odevice", 30,
		(unsigned char *)"Odevice",
	R_OLABEL, (unsigned char *)"olabel", 12,
		(unsigned char *)"Olabel",
	R_ONAME, (unsigned char *)"oname", 14,
		(unsigned char *)"Oname",
	(unsigned char *)"period", (unsigned char *)"period", 2,
		(unsigned char *)"Pd",
	R_PRIORITY, (unsigned char *)"prio", 3,
		(unsigned char *)"Pri",
	R_TAG, (unsigned char *)"tag", 14,
		(unsigned char *)"Tag",
	R_WEEK, (unsigned char *)"weeks", 6,
		(unsigned char *)"Weeks",
	(unsigned char *)"",(unsigned char *)"", 0,
		(unsigned char *)""
};

/* Format string for preheader displaying rotation and cweek. */
char *rotfmt = "Rotation Period = %d\tCurrent Week = %d\n";

/* Format string for repeating originating device header. */
char *odevfmt = "\n\nOriginating Device: %s %s %s\n";

int fld_cnt;
int *display;

int Cdisp[NDFLDS];
int Cfld_cnt = 0;

/* Set up order of fields to be displayed for static display options,       */
/* i.e., -A, -O, -R and default display.  Fields defined by index into the  */
/* fields array.                                                            */

/* -A display is: period, current week, tag, orig. name, orig. device, orig. */
/* media name, week, day, method, backup options, priority, depends, dest.   */
/* group, dest. device, dest. media name and characteristics.                */
int Adisp[] = {12, 0, 14, 11, 9, 10, 15, 1, 7, 8, 13, 4, 5, 3, 6, 2};
int Afld_cnt = NDFLDS;

/* -O display is: tag, oname, odevice, omname, depends. */
int Odisp[] = {14, 11, 9, 10, 4};
int Ofld_cnt = 5;

/* -R display is: tag, method, priority, dgroup, dchar. */
int Rdisp[] = {14, 7, 13, 5, 2};
int Rfld_cnt = 5;

/* default display is: tag, week, day, method, options, priority, dgroup. */
int defdisp[] = {14, 15, 1, 7, 8, 13, 5};
int deffld_cnt = 7;

/* Field lengths for columns in vertical output */
int vprlens[] = {18, 18, 18, 18};

/* Global pointers to option arguments */
char *a_arg, *b_arg = 0, *c_arg, *C_arg, *d_arg, *D_arg, *e_arg;
char *f_arg, *m_arg, *o_arg, *p_arg, *P_arg, *r_arg, *w_arg;

char *brcmdname;
int curr_period;	/* Current Rotational period in the file */
int curr_week, curr_day;	/* Current week/day in the period */

/* Rotational period given with -p option */
int period;

/* tag specified with -a, -e or -r argument */
unsigned char *tag;

/* parsed from -c argument */
unsigned char optweek[WK_PER_YR + 1];
unsigned char *weeks;
unsigned char *days;
char *allweeks = "all";

/* Parsed from -d argument */
unsigned char *dgroup, *ddevice = 0, *dchar = 0, *dmnames = 0;

/* Parsed from -D argument, a comma-separated list of tags */
unsigned char *depend = 0;

/* Field separator character, changed by -f argument */
char fs = FLDSEP;

/* Parsed from -m argument */
unsigned char *method;

/* Parsed from -o argument */
unsigned char *oname, *odevice, *olabel = 0;

/* Priority given with -P argument */
int pri = -1;
unsigned char c_pri[MAXPLEN];

/* -t argument */
char *table;
int	tid;	/* table identifier */

/* Current week specified by -w argument */
int cweek = 1;

int prtflag = TRUE;

extern char *sys_errlist[];
extern int sys_nerr;

extern int errno;

/* for lint */
extern void exit();
extern void bkerror();

main( argc, argv )
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	char *bk_get_bkreg_path();

	int i;
	int bad_args = 0;
	int all_bad = 0;
	int c;
	int error_seen = FALSE;
	int mode;
	int getopt();
	static int p_c_arg();
	static int p_C_arg();
	static int p_d_arg();
	static int p_D_arg();
	static int p_f_arg();
	static int p_m_arg();
	static int p_p_arg();
	static int p_P_arg();
	static int p_o_arg();
	static int p_w_arg();
	static int p_tag();
	static int tblopen();
	int unlink();
	int validcombination();
	static int validpropts();

	static void do_aflag();
	static void do_eflag();
	static void do_pflag();
	static void do_prtflag();
	static void do_rflag();
	static void synopsis();

	register rc;

	brcmdname = argv[0];

	table = bk_get_bkreg_path();

	while( (c = getopt( argc, argv,"a:Ab:c:C:d:D:e:f:hm:o:Op:P:r:Rst:vw:?")) != -1 )
		switch( c ) {
		case 'a':
			/* Add new file system or data partition */
			flags |= aFLAG;
			all_bad |= aERROR;
			prtflag = FALSE;
			a_arg = optarg;
			if( !p_tag( a_arg ) ) bad_args |= aERROR;
			break;

		case 'A':
			/* Print a complete report of current schedule */
			flags |= AFLAG;
			break;

		case 'b':
			/* Method Script arguments: "ac<cnt>defikmnptux" */
			flags |= bFLAG;
			all_bad |= bERROR;
			b_arg = optarg;
			if ( *b_arg == NULL )
				bad_args |= bERROR;
			break;

		case 'c':
			/* -c week:day */
			/* NOTE: validation of this depends on whether command */
			/* is invoked to display or to edit the table, hence   */
			/* cannot be done until all args have been parsed.     */
			flags |= cFLAG;
			c_arg = optarg;
			break;
		case 'C':
			/* Customized report */
			flags |= CFLAG;
			all_bad |= CERROR;
			C_arg = optarg;
			if( !p_C_arg((unsigned char *)strdup(C_arg)) )
				bad_args |= CERROR;
			break;

		case 'd':
			/* Destination device:
				"dgroup|:ddevice[:[dchar]:[dmnames]]" */
			flags |= dFLAG;
			all_bad |= dERROR;
			d_arg = optarg;
			if( !p_d_arg( strdup( d_arg ) ) ) bad_args |= dERROR;
			break;

		case 'D':
			/* Dependencies */
			flags |= DFLAG;
			all_bad |= DERROR;
			D_arg = optarg;
			if( !p_D_arg(  D_arg  ) ) bad_args |= DERROR;
			break;

		case 'e':
			/* Edit current schedule */
			flags |= eFLAG;
			all_bad |= eERROR;
			prtflag = FALSE;
			e_arg = optarg;
			if( !p_tag( e_arg ) ) bad_args |= eERROR;
			break;

		case 'f':
			/* Set field separator character */
			flags |= fFLAG;
			all_bad |= fERROR;
			f_arg = optarg;
			if( !p_f_arg(  f_arg  ) ) bad_args |= fERROR;
			break;

		case 'h':
			/* Suppress headers in display */
			flags |= hFLAG;
			break;

		case 'm':
			/* Method */
			flags |= mFLAG;
			all_bad |= mERROR;
			m_arg = optarg;
			if( !p_m_arg(  m_arg  ) ) bad_args |= mERROR;
			break;

		case 'o':
			/* Originating device: "name:dev[:label]" */
			flags |= oFLAG;
			all_bad |= oERROR;
			o_arg = optarg;
			if( !p_o_arg( strdup( o_arg ) ) ) bad_args |= oERROR;
			break;

		case 'O':
			/* Print summary of all originating devices */
			flags |= OFLAG;
			break;

		case 'p':
			/* Set rotation period: -p rp# (1 - 52)*/
			flags |= pFLAG;
			all_bad |= pERROR;
			prtflag = FALSE;
			p_arg = optarg;
			if( !p_p_arg( p_arg ) ) bad_args |= pERROR;
			break;
		case 'P':
			/* Set priority (0 - 100) */
			flags |= PFLAG;
			all_bad |= PERROR;
			P_arg = optarg;
			if( !p_P_arg( P_arg ) ) bad_args |= PERROR;
			break;

		case 'r':
			/* Remove line in backup register with specified tag. */
			flags |= rFLAG;
			all_bad |= rERROR;
			prtflag = FALSE;
			r_arg = optarg;
			if( !p_tag( r_arg ) ) bad_args |= rERROR;
			break;

		case 'R':
			/* Print summary of all destination devices */
			flags |= RFLAG;
			break;

		case 's':
			/* Suppress wraparound in report */
			flags |= sFLAG;
			break;

		case 't':
			/* Specifies a specific table to be used */
			flags |= tFLAG;
			all_bad |= tERROR;
			table = optarg;
			if( !table || !(*table) ) bad_args |= tERROR;
			break;

		case 'v':
			/* Vertical rather than horizontal display */
			flags |= vFLAG;
			break;

		case 'w':
			/* Set current week, overriding default */
			/* NOTE: validation must be postponed until period */
			/* is set. */
			flags |= wFLAG;
			w_arg = optarg;
			break;

		case '?':
			synopsis();
			exit( 1 );
			/*NOTREACHED*/
			break;

		default:
			bkerror( stderr, ERROR0, c );
			error_seen = TRUE;
			break;
		}


	if( optind != argc && !error_seen ) {
		for( ; optind < argc; optind++ )
			bkerror( stderr, ERROR1, argv[ optind ] );
		error_seen = TRUE;
	}

	if( error_seen )
		exit( 1 );

	if( (bad_args != 0) && (bad_args == all_bad) ) {
	 	bkerror( stderr, ERROR2 );
		exit( 1 );
	}
	if( bad_args ) {
		if( bad_args & aERROR )
			bkerror( stderr, ERROR3, a_arg, 'a' );

		if( bad_args & bERROR )
			bkerror( stderr, ERROR3, b_arg, 'b' );

		if( bad_args & CERROR )
			bkerror( stderr, ERROR3, C_arg, 'C' );

		if( bad_args & dERROR )
			bkerror( stderr, ERROR3, d_arg, 'd' );

		if( bad_args & DERROR )
			bkerror( stderr, ERROR3, D_arg, 'D' );

		if( bad_args & eERROR )
			bkerror( stderr, ERROR3, e_arg, 'e' );

		if( bad_args & fERROR )
			bkerror( stderr, ERROR3, f_arg, 'f' );

		if( bad_args & mERROR )
			bkerror( stderr, ERROR3, m_arg, 'm' );

		if( bad_args & oERROR )
			bkerror( stderr, ERROR3, o_arg, 'o' );

		if( bad_args & pERROR )
			bkerror( stderr, ERROR3, p_arg, 'p' );

		if( bad_args & PERROR )
			bkerror( stderr, ERROR3, P_arg, 'P' );

		if( bad_args & tERROR )
			bkerror( stderr, ERROR3, table, 't' );
		exit( 1 );
	}

	if( (flags & pFLAG ) 
		&& !(validcombination( 'p',  flags, allowed[ 0 ], required[ 0 ] ) ) )
			exit( 1 );
	if( ( flags & aFLAG )
		&& !(validcombination( 'a',  flags, allowed[ 1 ], required[ 1 ] ) ) )
			exit( 1 );
	if( ( flags & eFLAG ) 
		&& !(validcombination( 'e',  flags, allowed[ 2 ], required[ 2 ] ) ) )
			exit( 1 );
	if( ( flags & rFLAG ) 
		&& !(validcombination( 'r',  flags, allowed[ 3 ], required[ 3 ] ) ) )
			exit( 1 );

	/* Note: C, A, O and R options are mutually exclusive - check only one */
	if( flags & CFLAG ) {
		if( !(validcombination( 'C',  flags, allowed[ 4 ], required[ 4 ] ) ) )
			exit( 1 );
	}
	else if(  flags & AFLAG ) {
		if( !(validcombination( 'A',  flags, allowed[ 5 ], required[ 5 ] ) ) )
			exit( 1 );
	}
	else if( flags & OFLAG ) {
		if( !(validcombination( 'O',  flags, allowed[ 6 ], required[ 6 ] ) ) )
			exit( 1 );
	}
	else if( flags & RFLAG ) {
		if( !(validcombination( 'R',  flags, allowed[ 7 ], required[ 7 ] ) ) )
			exit( 1 );
	}
	/* assume user wants summary display - only -hsv, -t and -c allowed */
	else if( prtflag ) {
		if (!validpropts( flags, allowed[ 8 ] ) ) {
			synopsis();
			exit( 1 );
		}
	}

	/* If -c not specified, assume user wants all weeks displayed. */
	if ( prtflag && !(flags & cFLAG) ) {
		c_arg = allweeks;
		flags |= cFLAG;
	}

	/* Validate w argument */
	/* Validation done here due to fact that -p option MUST have been */
	/* specified for it to make sense to validate -w option           */
	if (flags & wFLAG)
		if( !p_w_arg( w_arg ) ) {
			bkerror( stderr, ERROR3, w_arg, 'w' );
			bkerror( stderr, ERROR18, 'w', period );
			exit( 1 );
		}

	mode = !prtflag ? O_RDWR: O_RDONLY;
	if( !tblopen( table, mode ) ) exit( 2 );

	/* Validate c argument */
	/* Done here because table must be open and must know whether */
	/* command was invoked to change the table or to display it.  */
	if ( flags & cFLAG )
		if( !p_c_arg( strdup(c_arg) ) ) {
			bkerror( stderr, ERROR3, c_arg, 'c' );
			TLclose( tid );
			exit( 1 );
		}


#ifdef TRACE
	(void)fprintf(stderr,"flags = %#x,	prtflag = %d\n",flags,prtflag);
	(void)fprintf(stderr,"table = %s\n",table);
	if( flags & pFLAG ) {
		(void)fprintf(stderr,"cweek = %d\n", cweek);
		(void)fprintf(stderr,"period = %d\n", period);
	}
	if( flags & mFLAG )
		(void)fprintf(stderr,"method = %s\n", method);

	if( flags & aFLAG || flags & eFLAG || flags & rFLAG ) {
		(void)fprintf(stderr,"tag = %s\n", tag);
		if( flags & cFLAG )
			(void)fprintf(stderr,"weeks = %s, days = %s\n", weeks, days);
		if( flags & bFLAG )
			(void)fprintf(stderr,"moptions = %s\n", b_arg);
		if( flags & DFLAG )
			(void)fprintf(stderr,"dependencies = %s\n", depend);
	}
	if( flags & dFLAG ) {
		(void)fprintf(stderr,"dgroup = %s\n", dgroup);
		(void)fprintf(stderr,"ddevice = %s\n", ddevice);
		(void)fprintf(stderr,"dchar = %s\n", dchar);
		(void)fprintf(stderr,"dmnames = %s\n", dmnames);
	}
	(void)fprintf(stderr,"priority = %d\n", pri);
	if( flags & oFLAG ) {
		(void)fprintf(stderr,"oname = %s\n", oname);
		(void)fprintf(stderr,"odevice = %s\n", odevice);
		(void)fprintf(stderr,"omname = %s\n", olabel);
	}
	if( flags & CFLAG ) {
		for( i = 0; i < fld_cnt; i++ )
			(void)fprintf(stderr,"display[%d] = %s\n", i, fields[display[i]].coptname);
		if( flags & fFLAG )
			(void)fprintf(stderr,"field separator = %c\n", fs);
	}
	if( prtflag && (flags & cFLAG) )
		for( i = 0; i < WK_PER_YR + 1; i++ )
			(void)fprintf(stderr,"optweek[%d] = %x\n", i, optweek[i]);
#endif

	if( flags & pFLAG ) do_pflag();
	if( flags & aFLAG ) do_aflag();
	if( flags & eFLAG ) do_eflag();
	if( flags & rFLAG ) do_rflag();
	if( prtflag ) do_prtflag();

	if( mode == O_RDWR && (rc = TLsync( tid )) != TLOK ) 
		if( rc == TLFAILED ) bkerror( stderr, ERROR21, table, errno );
		else bkerror( stderr, ERROR4, table, rc );

	TLclose( tid );

	/* Remove temporary file. */
	if (prtflag) 
		if( unlink( table ) < 0 )
			bkerror( stderr, ERROR17, table, errno );
	exit( 0 );
}

static void
synopsis()
{
	(void)fprintf( stderr, "%s -p period [-w cweek] [-t table]\n", brcmdname );
	(void)fprintf( stderr,
		"%s -a tag -o orig -c weeks:days|demand -m method -d ddev [-t table] [-b moptions] [-D depend] [-P prio]\n",
		brcmdname );
	(void)fprintf( stderr,
		"%s -e tag [-o orig] [-c weeks:days] [-m method] [-d ddev] [-t table] [-b moptions] [-D depend] [-P prio]\n",
		brcmdname );
	(void)fprintf( stderr,
		"%s -r tag [-t table]\n", brcmdname );
	(void)fprintf( stderr,
		"%s [-A | -O | -R] [-hsv] [-t table] [-c weeks[:days]]\n",
		brcmdname );
	(void)fprintf( stderr,
		"%s -C fields [-hv] [-t table] [-c weeks[:days]] [-f c]\n",
		brcmdname );
}

/* Parse week:day */
static int
p_c_arg( string )
char *string;
{
	unsigned char *comma_sep();
	static int ed_valid_c();
	static int pr_valid_c();

	static void strlower();

	if ( *string == NULL )
		return( FALSE );

	string = (char *)comma_sep( string );

	/* Break out weeks and days fields */
	weeks = (unsigned char *)strtok( string, ":");
	days  = (unsigned char *)strtok( NULL, ":");

	strlower( weeks );
	if( weeks == NULL )
		return( FALSE );

	if ( prtflag ) {
		if ( !pr_valid_c() )
			return( FALSE );
		else
			return( TRUE );
	}
	else {
		if ( strcmp( (char *)weeks, demand ) == 0 ) {
			if ( days == NULL )
				return( TRUE );
			else
				return( FALSE );
		}

		return ( ed_valid_c() );
	}

}

/* Validate weeks and days for editing functions */
static int
ed_valid_c ()
{
	int rc;
	int get_period();
	static int validdays();
	static int validweeks();

	static void cleanup();

	/* NOTE: cleanup() exits. */
	if( (rc = get_period(tid, &curr_period)) != 0) {
		bkerror( stderr, ERROR6, table, rc);
		cleanup();
	}
	if ( days == NULL )
		return( FALSE );
	return ( validweeks( curr_period ) && validdays() );
}

static int
validweeks( pd )
int pd;
{
	unsigned char *ptr;
	unsigned char *p_range();
	int week1, week2;

	ptr = weeks;
	while( ( ptr = p_range( ptr, &week1, &week2 ) ) != NULL) {
		if( week1 <= 0 || week1 > pd || week2 <= 0 || week2 > pd )
			return( FALSE );
		else if( *ptr != NULL )
			ptr++;
	}
	return( TRUE );
}

static int
validdays()
{
	unsigned char *ptr;
	unsigned char *p_range();
	int day1, day2;

	ptr = days;
	while( ( ptr = p_range( ptr, &day1, &day2 ) ) != NULL) {
		if( day1 < 0 || day1 > 6 || day2 < 0 || day2 > 6 )
			return( FALSE );
		else if( *ptr != NULL )
			ptr++;
	}
	return( TRUE );
}

/* Parse and validate [-c weeks[:days]] for display options */
/* Set up bit masks representing desired days for each week requested. */
/* These masks are used to select lines to be printed from the register. */
/* The last element of the array represents "demand" weeks. */
static int
pr_valid_c()
{
	unsigned char daybits;
	unsigned char *ptr;
	int begin;
	int end;
	int i;
	int ok;
	static int daystobits();

	unsigned char *p_weekrange();

	/* Deal with "demand" */
	if( strcmp( (char *)weeks, demand ) == 0 ) {
		optweek[WK_PER_YR] = 0x1;
		return( TRUE );
	}

	/* "demand" not specified.  If days field is null, all days are */
	/* selected.  Otherwise, set bits corresponding to selected days. */
	if( days == NULL )
		daybits = 0x7f;
	else if( !daystobits( &daybits ) )
			return( FALSE );

	/* Deal with "all" - user wants to see all lines in table. */
	/* (This is specified indirectly - user input no -c specification.) */
	if( strcmp( (char *)weeks, allweeks ) == 0 ) {
		for( i=0; i<= WK_PER_YR; i++ )
			optweek[i] = daybits;
		return( TRUE );
	}

	/* For each week specified, set entry to "daybits" value. */
	ptr = weeks;
	ok = FALSE;
	while( ptr = p_weekrange( ptr, &begin, &end ) ) {
		for( i = min( begin, end ) ; i <= max( begin, end ); i++ )
			optweek[i - 1] = daybits;
		if( !*ptr ) {
			ok = TRUE;
			break;
		}
		ptr++;
	}
	return( ok );
}

static int
daystobits( dbits )
unsigned char *dbits;
{
	int begin;
	int end;
	int i;
	int ok;
	unsigned char *ptr;
	unsigned char *p_dayrange();

	*dbits = 0;
	ptr = days;
	ok = FALSE;
	while(  ptr = p_dayrange( ptr, &begin, &end ) ) {
		for( i = min( begin, end ); i <= max( begin, end ); i++ )
			*dbits |= (0x1 << i);
		if (!*ptr) {
			ok = TRUE;
			break;
		}
		ptr++;
	}
	return( ok );
}

/* Parse -C fields option and set up display array to indicate which *
/* fields are to be displayed in what order */
static int
p_C_arg( string )
unsigned char *string;
{
	unsigned char *optfld;
	int cmp;
	int found;
	int high;
	int low;
	int mid;

	if( ( optfld = (unsigned char *)strtok( (char *)string, ", " ) ) == NULL )
		return( FALSE );

	while( optfld && *optfld != NULL ) {
		found = FALSE;
		low = 0;
		high = ndisp_flds - 1;

		while( !found && (low <= high) ) {
			mid = (low + high) / 2;
			if( (cmp = strcmp( (char *)optfld,
				(char *)fields[mid].coptname ) ) < 0 )
				high = mid - 1;
			else if ( cmp > 0 )
				low = mid + 1;
			else found = TRUE;
		}

		if( !found ) return ( FALSE );

		Cdisp[Cfld_cnt++] = mid;
		optfld = (unsigned char *)strtok( NULL, ", ");
	}

	return( TRUE );
}

/* Parse: "dgroup|:ddevice[:[dchar]:[dmnames]]" where  */
/*	dchar is a list of keyword=value strings. */
/*	dmnames is a comma- or blank-separated list of media names */
static int
p_d_arg( string )
char *string;
{
	unsigned char *comma_sep();
	char *bkstrtok();
	static int valid_dmnames();

	dgroup = (unsigned char *)bkstrtok( string, ":" );
	ddevice = (unsigned char *)bkstrtok( NULL, ":" );
	if( !(dgroup) && !(ddevice) )
		return( FALSE );
	if( dchar = (unsigned char *)bkstrtok( NULL, ":"  ) )
		dchar = comma_sep( (char *)dchar );
	if( dmnames = (unsigned char *)bkstrtok( NULL, ":" ) )
		if( !valid_dmnames() ) return( FALSE );
	return( TRUE );
}

static int
valid_dmnames()
{
	unsigned char *name;
	unsigned char *comma_sep();

	/* Save a copy of the comma-separated string to add to table. */
	/* (The strtok calls to validate entries write nulls into the string.) */
	name = dmnames;
	dmnames = comma_sep( (char *)dmnames );
	(void)strcpy( (char *)name, (char *)dmnames );

	name = (unsigned char *)strtok( (char *)name, "," );
	while( name != NULL ){
		if( (int)strlen( (char *)name ) > (int)BKLABEL_SZ )
			return( FALSE );
		name = (unsigned char *)strtok( NULL, "," );
	}
	return( TRUE );
}

/* Parse -D dependencies */
static int
p_D_arg( string )
char *string;
{
	unsigned char *comma_sep();

	if( !*string ) return( FALSE );
	depend = comma_sep( string );
	return( TRUE );
}

/* Parse and validate field separator */
static int
p_f_arg( string )
char *string;
{
	if (strlen( string ) != 1)
		return( FALSE );
	else {
		fs = string[0];
		return( TRUE );
	}
}

/* Parse incfile, fimage, ffile, fdp or filename */
static int
p_m_arg( string )
char *string;
{
	if( !string || !(*string) ) return ( FALSE );

	method = (unsigned char *)string;
	return( TRUE );
}

/*  Parse: "name:dev[:label]" */
static int
p_o_arg( string )
char *string;
{
	char *bkstrtok();

	if( !(oname = (unsigned char *)bkstrtok( string, ":" ) ) ) return( FALSE );
	if( !(odevice = (unsigned char *)bkstrtok( NULL, ":" ) ) ) return( FALSE );
	olabel = (unsigned char *)bkstrtok( NULL, ":" );
	return( (*oname) && (*odevice) );
}

/* Parse: rotation period */
static int
p_p_arg( string )
char *string;
{
	unsigned char *p_integer();
	return( p_integer( (unsigned char *)string, &period ) && period > 0 &&
		period <= WK_PER_YR );
}

/* Parse and validate priority */
static int
p_P_arg( string )
char *string;
{
	unsigned char *p_integer();
	return( p_integer( (unsigned char *)string, &pri ) &&
		pri >= 0 && pri <= MAX_PRIORITY );
}

/* Parse and validate tag (form only) */
static int
p_tag( string )
char *string;
{
	if (!string)
		return( FALSE );
	tag = (unsigned char *) string;
	return( TRUE );
}

/* Find out whether a tag exists in the table */
static int
tag_exists( tag, entryno )
unsigned char *tag;
int *entryno;
{
	static void cleanup();

	struct TLsearch *tls = TLsearches;

	tls->ts_fieldname = R_TAG;
	tls->ts_pattern = tag;
	tls->ts_operation = (int (*)() )TLEQ;
	tls++;
	tls->ts_fieldname = (unsigned char *)0;

	if( ( *entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND ) ) == TLFAILED )
		return( FALSE );
	else if ( (*entryno == TLBADID) || (*entryno == TLARGS) ||
		  (*entryno == TLBADENTRY) || (*entryno == TLBADFIELD) ) {
		bkerror( stderr, ERROR19, *entryno );
		cleanup();
	}
	return( TRUE );
}

/* Parse and validate current week ( -w cweek ) */
/* cweek should be no larger than the new rotation period */
static int
p_w_arg( string )
char *string;
{
	unsigned char *p_integer();
	return( p_integer( (unsigned char *)string, &cweek ) &&
		cweek > 0 && cweek <= period );
}

/*
	Check that the options on a command line without -a, -e, -r, -p, -A, -O
	-R or -C are allowed.  No options are required, but not all are allowed.

	Each option has its own bit. 
*/
static int
validpropts( given, f_allowed )
unsigned given;
unsigned f_allowed;
{
	unsigned too_many;
	register i, offset, error = FALSE;
	too_many = (given & ~f_allowed);
	for( i = 1, offset = 0; i < (1<<nflags); i <<= 1, offset++ ) {
		if( i & too_many )
			error = TRUE;
	}
	return( !error );
}

static void
strlower( string )
unsigned char *string;
{
	for( ; *string; string++ )
		if( isupper( *string ) ) *string = _tolower( *string );
}

/* Returns TRUE if file exists and has data in it.  A zero-length file */
/* should be processed as though it is new. */
static int
f_exists( filename )
char *filename;
{
	struct stat buf;

	if ( (stat( filename, &buf ) == 0) && (buf.st_size > 0) )
			return( TRUE );
	
	return( FALSE );
}

static int
tblopen( table, mode )
char *table;
int mode;
{
	struct TLdesc description;
	static int f_exists();
	int insert_format();
	int insert_rotation();
	int insert_rot_start();
	register rc;
	register is_new = !f_exists( table );

	if( mode == O_RDONLY && is_new ) {
		bkerror( stderr, ERROR20, table, errno );
		return( FALSE );
	}
	(void)strncpy( (char *)&description, "", sizeof( struct TLdesc ) );
	description.td_format = R_BKREG_F;
	
	if( (rc = TLopen( &tid, table, &description, mode, 0777 ) ) != TLOK ) {
		if( rc == TLFAILED ) bkerror( stderr, ERROR20, table, errno );
		else bkerror( stderr, ERROR10, table, rc );
		return( FALSE );
	}

	if( is_new ) {
		insert_format( tid, R_BKREG_F );

		if( (rc = insert_rotation( tid, 1 )) != 0 ) {
			bkerror( stderr, ERROR15, R_ROTATE_MSG, table, rc );
			return( FALSE);
		}

		if( (rc = insert_rot_start( tid, 1 )) != 0 ) {
			bkerror( stderr, ERROR15, R_ROTATE_START_MSG, table, rc );
			return( FALSE);
		}
	}
	return( TRUE );
}

/* Remove an entry from the table */
static void
do_rflag()
{
	int entryno;
	int rc;
	static int tag_exists();

	static void cleanup();

	if( !tag_exists( tag, &entryno ) ) {
		bkerror( stderr, ERROR12, tag, table );
		cleanup();
	}

	if( (rc = TLdelete( tid, entryno )) != TLOK ) {
		bkerror( stderr, ERROR14, entryno, rc );
		cleanup();
	}
}

/* Add an entry to the table */
/* Entry must not already exist.  New entry is added to end of table. */
static void
do_aflag()
{
	ENTRY entry;
	int entryno;
	int rc;
	static int tag_exists();

	static void cleanup();

	if( tag_exists( tag, &entryno ) ) {
		bkerror( stderr, ERROR11, tag, table );
		cleanup();
	}

	/* Get an entry element */
	if( !(entry = TLgetentry( tid ) ) ) {
		bkerror( stderr, ERROR5 );
		cleanup();
	}
	if( (rc = TLassign( tid, entry, R_TAG, tag )) != TLOK ) {
		bkerror( stderr, ERROR13, tag, entryno, rc );
		cleanup();
	}
	if( (rc = TLassign( tid, entry, R_ONAME, oname )) != TLOK ) {
		bkerror( stderr, ERROR13, oname, entryno, rc );
		cleanup();
	}
	if( (rc = TLassign( tid, entry, R_ODEVICE, odevice )) != TLOK ) {
		bkerror( stderr, ERROR13, odevice, entryno, rc );
		cleanup();
	}
	if( olabel )
		if( (rc = TLassign( tid, entry, R_OLABEL, olabel )) != TLOK ) {
			bkerror( stderr, ERROR13, olabel, entryno, rc );
			cleanup();
		}
	if( (rc = TLassign( tid, entry, R_METHOD, method )) != TLOK ) {
		bkerror( stderr, ERROR13, method, entryno, rc );
		cleanup();
	}
	if( b_arg )
		if( (rc = TLassign( tid, entry, R_OPTIONS, b_arg )) != TLOK ) {
			bkerror( stderr, ERROR13, b_arg, entryno, rc );
			cleanup();
		}

	if( (rc = TLassign( tid, entry, R_WEEK, weeks )) != TLOK ) {
		bkerror( stderr, ERROR13, weeks, entryno, rc );
		cleanup();
	}
	if( days )
		if( (rc = TLassign( tid, entry, R_DAY, days )) != TLOK ) {
			bkerror( stderr, ERROR13, days, entryno, rc );
			cleanup();
		}

	if( dgroup )
		if( (rc = TLassign( tid, entry, R_DGROUP, dgroup )) != TLOK ) {
			bkerror( stderr, ERROR13, dgroup, entryno, rc );
			cleanup();
		}
	if( ddevice )
		if( (rc = TLassign( tid, entry, R_DDEVICE, ddevice )) != TLOK ) {
			bkerror( stderr, ERROR13, ddevice, entryno, rc );
			cleanup();
		}
	if( dchar )
		if( (rc = TLassign( tid, entry, R_DCHAR, dchar )) != TLOK ) {
			bkerror( stderr, ERROR13, dchar, entryno, rc );
			cleanup();
		}
	if( dmnames )
		if( (rc = TLassign( tid, entry, R_DMNAME, dmnames )) != TLOK ) {
			bkerror( stderr, ERROR13, dmnames, entryno, rc );
			cleanup();
		}

	/* If user doesn't specify priority, set it to default (0). */
	if( pri < 0 )  pri = 0;
	(void)sprintf( (char *)c_pri, "%d", pri );
	if( (rc = TLassign( tid, entry, R_PRIORITY, c_pri )) != TLOK ) {
		bkerror( stderr, ERROR13, c_pri, entryno, rc );
		cleanup();
	}

	if( depend )
		if( (rc = TLassign( tid, entry, R_DEPEND, depend )) != TLOK ) {
			bkerror( stderr, ERROR13, depend, entryno, rc );
			cleanup();
		}

	if( TLappend( tid, TLEND, entry ) != TLOK ) {
		bkerror( stderr, ERROR9 );
		cleanup();
	}
}

/* Edit an entry in the table */
/* Entry must already exist and is then rewritten to table with new information */
static void
do_eflag()
{
	ENTRY entry;
	int entryno;
	int rc;
	static int tag_exists();

	static void cleanup();

	if( !tag_exists( tag, &entryno ) ) {
		bkerror( stderr, ERROR12, tag, table );
		cleanup();
	}

	/* Get an entry element */
	if( !(entry = TLgetentry( tid ) ) ) {
		bkerror( stderr, ERROR5 );
		cleanup();
	}
	if( (rc = TLread( tid, entryno, entry )) != TLOK  ) {
		bkerror( stderr, ERROR8, entryno, rc );
		cleanup();
	}

	if( oname ) 
		if( (rc = TLassign( tid, entry, R_ONAME, oname )) != TLOK ) {
			bkerror( stderr, ERROR13, oname, entryno, rc );
			cleanup();
		}
	if( odevice )
		if( (rc = TLassign( tid, entry, R_ODEVICE, odevice )) != TLOK ) {
			bkerror( stderr, ERROR13, odevice, entryno, rc );
			cleanup();
		}
	if( olabel )
		if( (rc = TLassign( tid, entry, R_OLABEL, olabel )) != TLOK ) {
			bkerror( stderr, ERROR13, olabel, entryno, rc );
			cleanup();
		}
	if( method )
		if( (rc = TLassign( tid, entry, R_METHOD, method )) != TLOK ) {
			bkerror( stderr, ERROR13, method, entryno, rc );
			cleanup();
		}
	if( b_arg )
		if( (rc = TLassign( tid, entry, R_OPTIONS, b_arg )) != TLOK ) {
			bkerror( stderr, ERROR13, b_arg, entryno, rc );
			cleanup();
		}

	/* If weeks = "demand", then days should be set to null.  Otherwise, */
	/* previous code has detected whether days is set or not, since if */
	/* -c is specified, the user must provide both a weeks and a days */
	/* specification (unless weeks = "demand". */
	if( weeks )
		if( (rc = TLassign( tid, entry, R_WEEK, weeks )) != TLOK ) {
			bkerror( stderr, ERROR13, weeks, entryno, rc );
			cleanup();
		}
	
	if( days || (strcmp( (char *)weeks, demand ) == 0) )

		if( (rc = TLassign( tid, entry, R_DAY, days )) != TLOK ) {
			bkerror( stderr, ERROR13, weeks, entryno, rc );
			cleanup();
		}

	if( dgroup )
		if( (rc = TLassign( tid, entry, R_DGROUP, dgroup )) != TLOK ) {
			bkerror( stderr, ERROR13, dgroup, entryno, rc );
			cleanup();
		}
	if( ddevice )
		if( (rc = TLassign( tid, entry, R_DDEVICE, ddevice )) != TLOK ) {
			bkerror( stderr, ERROR13, ddevice, entryno, rc );
			cleanup();
		}
	if( dchar )
		if( (rc = TLassign( tid, entry, R_DCHAR, dchar )) != TLOK ) {
			bkerror( stderr, ERROR13, dchar, entryno, rc );
			cleanup();
		}
	if( dmnames )
		if( (rc = TLassign( tid, entry, R_DMNAME, dmnames )) != TLOK ) {
			bkerror( stderr, ERROR13, dmnames, entryno, rc );
			cleanup();
		}

	if( pri >= 0 ) {
		(void)sprintf( (char *)c_pri, "%d", pri );
		if( (rc = TLassign( tid, entry, R_PRIORITY, c_pri )) != TLOK ) {
			bkerror( stderr, ERROR13, c_pri, entryno, rc );
			cleanup();
		}
	}
	if( depend ) TLassign( tid, entry, R_DEPEND, depend );
		if( (rc = TLassign( tid, entry, R_DEPEND, depend )) != TLOK ) {
			bkerror( stderr, ERROR13, depend, entryno, rc );
			cleanup();
		}

	if( (rc = TLwrite( tid, entryno, entry ) ) != TLOK ) {
		bkerror( stderr, ERROR9 );
		cleanup();
	}
}

/* Set rotation period and modify start date of rotation week 1 accordingly. */
static void
do_pflag()
{
	int rc;
	int insert_rotation();
	int insert_rot_start();
	static void cleanup();

	if( (rc = insert_rotation( tid, period )) != 0 ) {
		bkerror( stderr, ERROR15, R_ROTATE_MSG, table, rc );
		cleanup();
	} else
		bkerror( stderr, ERROR22, period );


	if( (rc = insert_rot_start( tid, cweek )) != 0 ) {
		bkerror( stderr, ERROR15, R_ROTATE_START_MSG, table, rc );
		cleanup();
	}
}

/* Display requested bkreg information. */
/* Note: all displays except possibly -C require period and cweek. */
static void
do_prtflag()
{
	int rc;
	int get_period();
	int get_rotate_start();

	static void cleanup();
	static void do_Aprt();
	static void do_Cprt();
	static void do_Oprt();
	static void do_origprt();
	void sort_reg();

	if( (rc = get_period(tid, &curr_period)) != 0) {
		bkerror( stderr, ERROR6, table, rc);
		cleanup();
	}

	if( (rc = get_rotate_start(tid, curr_period, &curr_week, &curr_day)) != 0) {
		bkerror( stderr, ERROR7, table, rc);
		cleanup();
	}

	sort_reg();		/* sort and get rid of blank lines */ 

	if( flags & AFLAG ) do_Aprt();
	else if( flags & CFLAG ) do_Cprt();
	else if( flags & OFLAG ) do_Oprt();
	else if( flags & RFLAG ) {
		display = Rdisp;
		fld_cnt = Rfld_cnt;
		do_origprt();
	}
	else {
		display = defdisp;
		fld_cnt = deffld_cnt;
		do_origprt();
	}
}

/* Display AFLAG information. */
static void
do_Aprt()
{
	static void comprt();

	display = Adisp;
	fld_cnt = Afld_cnt;
	comprt();
}

/* Display OFLAG information. */
static void
do_Oprt()
{
	static void comprt();

	display = Odisp;
	fld_cnt = Ofld_cnt;
	(void)fprintf( stdout, rotfmt, curr_period, curr_week );
	comprt();
}

/* Call vertical print if required, otherwise, print header and print */
/* horizontally. */
static void
comprt()
{
	static void prt_head();
	static void hor_print();
	static void vert_print();

	if( flags & vFLAG ) vert_print();
	else {
		if( !(flags & hFLAG) )
			prt_head();
		hor_print();
	}
}

/* Display fields user requested. */
static void
do_Cprt()
{
	static void prt_Chead();
	static void hor_print();
	static void vert_print();

	display = Cdisp;
	fld_cnt = Cfld_cnt;
	if( flags & vFLAG ) vert_print();
	else {
		if( !( flags & hFLAG) )
			prt_Chead();
		hor_print();
	}
}

/* Display information sorted by originating device (-R and default). */
static void
do_origprt()
{
	static void hor_orig_print();
	static void overt_print();

	(void)fprintf( stdout, rotfmt, curr_period, curr_week );
	if( flags & vFLAG ) overt_print();
	else hor_orig_print();
}

/* Sort backup register by originating device field into a temporary file. */
/* File name is generated using pid so two simultaneous invocations */
/* don't clobber each other.  NOTE: table id is changed to point to temp */
/* file from here on.  Original table is not used to print display. */
static void
sort_reg()
{
	char cmdstr[512];
	char tmpfile[40];

	static int tblopen();

	/* Close old table file. */
	TLclose( tid );

	/* Generate temp file name. */

	(void)sprintf( tmpfile, "/var/tmp/bkreg%d", getpid() );

	/* Sort table on originating device field.  Note: if this field changes */
	/* location in the table, the sort system call will have to change! */
	/* Pull out comment strings before sorting table.  If FORMAT string */
	/* ends up after data entries, table doesn't work correctly. */

	(void)sprintf( cmdstr, "grep  '^#' %s >%s", table, tmpfile );

	/* Reset SIGCLD to default */
	(void)signal( SIGCLD, SIG_DFL );

	if( system( cmdstr ) < 0 ) {
		bkerror( stderr, ERROR16, table, errno );
		(void)unlink( tmpfile );
		exit( 2 );
	}
	
	(void)sprintf( cmdstr, "egrep -v '^#|^$' %s |sort -b -t: +2 -3 >>%s", table, tmpfile );
	if( system( cmdstr ) < 0 ) {
		bkerror( stderr, ERROR16, table, errno );
		(void)unlink( tmpfile );
		exit( 2 );
	}

	/* Open temp file as table. */
	table = strdup( tmpfile );
	if( !tblopen( table, O_RDONLY)) {
		(void)unlink( tmpfile );
		exit( 2 );
	}
}

/* Print header */
static void
prt_head()
{
	int fldindex;
	int i;
	int total;

	(void)fprintf( stdout, "\n" );
	total = 0;
	for( i = 0; i < fld_cnt - 1; i++) {
		fldindex = display[i];
		(void)fprintf( stdout, "%-*s ", fields[fldindex].fldlen,
			fields[fldindex].dispname);
		total += fields[fldindex].fldlen + 1;
	}
	fldindex = display[fld_cnt - 1];
	(void)fprintf( stdout, "%-*s\n", fields[fldindex].fldlen,
		fields[fldindex].dispname );
	total += fields[fldindex].fldlen;
	for( i = 0; i < total; i++ )
		(void)fprintf( stdout, "-" );
	(void)fprintf( stdout, "\n" );
}

/* Print header for CFLAG displays (field names separated by field separator). */
/* No line of dashes is printed between the header and the data. */
static void
prt_Chead()
{
	int fldindex;
	int i;

	for( i = 0; i < fld_cnt - 1; i++) {
		fldindex = display[i];
		(void)fprintf( stdout, "%s%c", fields[fldindex].dispname, fs);
	}
	fldindex = display[fld_cnt - 1];
	(void)fprintf( stdout, "%s\n", fields[fldindex].dispname );
}

/* Print horizontally formatted displays that do not repeat headers. */
static void
hor_print()
{
	ENTRY entry;
	static int selectline();

	static void cleanup();
	static void hprt_nowrap();
	static void hprt_wrap();
	static void prt_Cnowrap();

	/* Get an entry element */
	if( !(entry = TLgetentry( tid ) ) ) {
		bkerror( stderr, ERROR5 );
		cleanup();
	}
	if( flags & CFLAG )
		while( selectline( entry ) != FALSE ) 
			prt_Cnowrap( entry );

	else {
		if( flags & sFLAG )
			while( selectline( entry ) != FALSE )
				hprt_nowrap( entry );
		else 
			while( selectline( entry ) != FALSE )
				hprt_wrap( entry );
	}
}
/* Print horizontally formatted displays that repeat headers, */
/* one for each different originating device. */
static void
hor_orig_print()
{
	ENTRY entry;
	unsigned char *prev_odevice;
	static int selectline();

	static void cleanup();
	void free();
	static void prt_head();
	static void hprt_wrap();
	static void hprt_nowrap();

	/* Get an entry element */
	if( !(entry = TLgetentry( tid ) ) ) {
		bkerror( stderr, ERROR5 );
		cleanup();
	}

	prev_odevice = NULL;

	while( selectline( entry ) != FALSE ) {
		odevice = TLgetfield( tid, entry, R_ODEVICE );

		if( strcmp( (char *)odevice, (char *)prev_odevice ) != 0 ) {
			oname = TLgetfield( tid, entry, R_ONAME );
			olabel = TLgetfield( tid, entry, R_OLABEL );

			(void)fprintf( stdout, odevfmt, oname, odevice, olabel );

			if( !( flags & hFLAG ) ) prt_head();

			if( prev_odevice ) free( prev_odevice );
			if( ( prev_odevice =
				(unsigned char *)strdup((char *)odevice) ) == NULL ) {
				bkerror( stderr, ERROR5 );
				cleanup();
			}
		}

		if( flags & sFLAG )
			hprt_nowrap( entry );
		else hprt_wrap( entry );
	}
}

/* Select next line to be printed from the backup register table. */
/* Selection is made according to the weeks and days specification */
/* the user entered on the command line.  A line is selected when  */
/* it contains at least one week with at least one day the user requested. */
/* For example, if the user asks to see week 2, days 2-6, the backup */
/* register line with weeks 1-5,8 and days 3-5 would be selected. */
/* Note: routine starts reading with the NEXT line in the table (i.e., */
/* lineno++) and returns with lineno equal to the last line read, the */
/* one that was selected.  If no more lines are selected, the routine */
/* returns FALSE. */

int lineno = 0;

static int
selectline( entry )
ENTRY entry;
{
	unsigned char *ptr;
	unsigned char *p_weekrange();

	int ok;
	int i;
	int begin;
	int end;
	int rc;
	static int daymatch();

	static void cleanup();

	while( (rc = TLread( tid, ++lineno, entry )) == TLOK ) {

		/* Ignore comment lines. */

		if( TLgetfield( tid, entry, TLCOMMENT ) != NULL )
			continue;

		 /* Note: if tag field is null, which is technically not   */
                 /* allowed, skip the line. */

		if( ( tag = TLgetfield( tid, entry, R_TAG ) ) == NULL )
                        continue;

		/* Note: if weeks field is null, which is technically not */
		/* allowed, select the line and print its contents.  The  */
		/* user may be displaying the table to see if anything is */
		/* wrong with it.  The user will have to realize, however */
		/* that a null weeks field is not legal.                  */
		if( ( weeks = TLgetfield( tid, entry, R_WEEK ) ) == NULL )
			return( TRUE );

		if( strcmp( (char *)weeks, demand ) == 0 )
			if( optweek[WK_PER_YR] )
				return( TRUE );
			else continue;

		ptr = weeks;
		ok = FALSE;
		while( ptr = p_weekrange( ptr, &begin, &end ) ) {
			for( i = min( begin, end ); i <= max( begin, end ); i++ )
				if( ( optweek[i-1] > 0 ) && daymatch( i-1, entry ) )
					return( TRUE );
			if( !*ptr ) {
				ok = TRUE;
				break;
			}
			ptr++;
		}
		/* Note: if weeks value is not valid (not between 1 and 52) */
		/* select the line and print it in the hope that the user   */
		/* will see the value in error. */
		if( !ok )
			return( TRUE );
	}
	if ( rc != TLBADENTRY ) {
		bkerror( stderr, ERROR8, lineno, rc );
		cleanup();
	}
	return( FALSE );
}

/* Check whether any days in the days field of this table entry have been */
/* requested by the user.  User's requested days for selected weeks are   */
/* stored in optweek in bit mask form. */
static int
daymatch( wknum, entry )
int wknum;
ENTRY entry;
{
	unsigned char daybits;

	/* Note: if days field is null, this is technically an error. */
	/* Also, if the days field contains illegal days (outside the */
	/* range 0-6), this is an error. Select the line and print it */
	/* so user may be able to see the error. */
	if( ( days = TLgetfield( tid, entry, R_DAY ) ) == NULL )
		return( TRUE );

	if( !daystobits( &daybits ) )
		return( TRUE );

	if( daybits & optweek[wknum] )
		return( TRUE );
	else
		return( FALSE );
	
}

/* Print field information separated by field separator character.  */
/* Print entire value, do not wrap fields to a specified field width. */
static void
prt_Cnowrap( entry )
ENTRY entry;
{
	int i;
	int fldindex;
	char *field;

	for( i = 0; i < fld_cnt; i++) {
		fldindex = display[i];

		if ( strcmp( (char *)fields[fldindex].fldname, "cweek" ) == 0 )
			(void)fprintf( stdout, "%d%c", curr_week,
				(i < fld_cnt-1) ? fs : '\n');

		else if ( strcmp( (char *)fields[fldindex].fldname, "period" )
			== 0 )
			(void)fprintf( stdout, "%d%c", curr_period,
				(i < fld_cnt-1) ? fs : '\n');

		else (void)fprintf( stdout, "%s%c",
			((field=(char *)TLgetfield( tid, entry, fields[fldindex].fldname
\
			)) ? field : ""), (i < fld_cnt-1) ? fs : '\n' );
	}
}

/* Print entire value, do not wrap fields to a specified field width, */
/* but do fill each field to its minimum field width. */
static void
hprt_nowrap( entry )
ENTRY entry;
{
	int i;
	int fldindex;
	int flen;
	char *field;

	for( i = 0; i < fld_cnt; i++) {
		fldindex = display[i];
		flen = fields[fldindex].fldlen;
		if ( strcmp( (char *)fields[fldindex].fldname, "cweek" ) == 0 )
			(void)fprintf( stdout, "%-*d%c", flen, curr_week,
				(i < fld_cnt-1) ? ' ' : '\n' );

		else if ( strcmp( (char *)fields[fldindex].fldname, "period" )
			== 0 )
			(void)fprintf( stdout, "%-*d%c", flen, curr_period,
				(i < fld_cnt-1) ? ' ' : '\n');

		else (void)fprintf( stdout, "%-*s%c", flen,
			((field=(char *)TLgetfield( tid, entry, fields[fldindex].fldname\
			)) ? field : ""), (i < fld_cnt-1) ? ' ' : '\n' );
	}
}

/* Print field information, wrapping long entries to succeeding lines, */
/* aligning them to field boundaries. */
static void
hprt_wrap( entry )
ENTRY entry;
{
	int done;
	int fldindex;
	int flen;
	int i;
	unsigned char *next_char[NDFLDS];
	char pdbuf[PDLEN + 1]; /* scratch buffer for period */
	char cwbuf[PDLEN + 1]; /* scratch buffer for cweek */

	for( i = 0; i < fld_cnt; i++ ) {
		fldindex = display[i];
		if ( strcmp( (char *)fields[fldindex].fldname, "cweek" ) == 0 ) {
			(void)sprintf( cwbuf, "%d", curr_week );
			next_char[i] = (unsigned char *)cwbuf;
		}
		else if ( strcmp( (char *)fields[fldindex].fldname, "period" )
			== 0 ) {
			(void)sprintf( pdbuf, "%d", curr_period );
			next_char[i] = (unsigned char *)pdbuf;
		}
		else next_char[i] = TLgetfield( tid, entry,
					fields[fldindex].fldname );
		if (!next_char[i]) next_char[i] = (unsigned char *)"";
	}

	done = FALSE;
	while( !done ) {
		done = TRUE;
		for( i = 0; i < fld_cnt; i++ ) {
			fldindex = display[i];
			flen = fields[fldindex].fldlen;
			if( i == 0 )
				(void)fprintf( stdout, "%-*.*s", flen, flen,
					next_char[i] );
			else
				(void)fprintf( stdout, " %-*.*s", flen, flen,
					next_char[i] );
			if( *next_char[i] != NULL ) {
				next_char[i] += min( (int)strlen( (char *)next_char[i] ),
							flen);
				if( *next_char[i] != NULL )
					done = FALSE;
			}
		}
		(void)fprintf( stdout, "\n" );
	}
}

/* Print vertically formatted displays that don't repeat pre-headers. */
static void
vert_print()
{
	ENTRY eptrs[MAXCOL];

	static void cleanup();
	void prt_wrap();

	int colcnt;	/* number of columns of data read */
	int done;
	int fldind;
	int i;
	int j;
	int k;
	int ncol = MAXCOL - 1;  /* number of columns of data maximum */
				/* (default is with headers) */
	int prcols;	/* number of columns total to be printed */

	unsigned char *prvalues[MAXCOL];
	char pdbuf[PDLEN + 1]; /* scratch buffer for period */
	char cwbuf[PDLEN + 1]; /* scratch buffer for cweek */

	/* set up buffer with current week and period values */
	(void)sprintf( cwbuf, "%d", curr_week );
	(void)sprintf( pdbuf, "%d", curr_period );

	/* If no headers desired, then increase to maximum number of */
	/* columns. */
	if ( flags & hFLAG )
		ncol = MAXCOL;

	for ( i = 0; i < ncol; i++ )
		if ( !(eptrs[i] = TLgetentry( tid )) ) {
			bkerror( stderr, ERROR5 );
			cleanup();
		}

	/* Read up to ncol entries or to EOF, whichever occurs first. */
	/* Then print each field's values across as one line with ncol */
	/* columns.  */
	done = FALSE;
	while ( !done ) {
		colcnt = 0;
		while ( (colcnt < ncol) && (selectline( eptrs[colcnt] )) )
			colcnt++;
		if ( colcnt < ncol ) done = TRUE;
		if ( colcnt == 0 )
			continue;
		if ( !(flags & hFLAG) )
			prcols = colcnt + 1;
		else prcols = colcnt;

		/* Print display with values for ncol lines for each field.*/
		for ( i = 0; i < fld_cnt; i++ ) {
			fldind = display[i];
			j = 0;

			if ( !(flags & hFLAG) )
				prvalues[j++] = fields[fldind].dispname;

			if ( strcmp( (char *)fields[fldind].fldname, "cweek" )
				== 0 )
				for( k = 0 ; k < colcnt; k++ )
					prvalues[j++] = (unsigned char *)cwbuf;

			else if ( strcmp( (char *)fields[fldind].fldname,
					"period" ) == 0 )
				for( k = 0 ; k < colcnt; k++ )
					prvalues[j++] = (unsigned char *)pdbuf;

			else for( k = 0 ; k < colcnt; k++ ) {
				prvalues[j++] = TLgetfield( tid, eptrs[k],
						fields[fldind].fldname );
				if (!prvalues[j-1]) prvalues[j-1] = (unsigned char *)"";
			}

			prt_wrap( prcols, prvalues, vprlens );
		}
		(void)fprintf( stdout, "\n" );
	}
}
/* Print vertically formatted displays that repeat headers (i.e., those */
/* sorted by originating devices). */
static void
overt_print()
{
	ENTRY eptrs[MAXCOL];

	int colcnt;	/* number of columns of data read */
	int done;
	int fldind;
	int i;
	int j;
	int k;
	int linesleft;
	int ncol = MAXCOL - 1;  /* number of columns of data maximum */
				/* (default is with headers) */
	int prcols;	/* number of columns total to be printed */
	int prthdr = FALSE;
	int rc;
	static int selectline();

	static void cleanup();
	void free();
	void prt_wrap();

	unsigned char *prev_odev;
	unsigned char *prvalues[MAXCOL];

	/* If no headers desired, then increase to maximum number of */
	/* columns. */
	if ( flags & hFLAG )
		ncol = MAXCOL;

	for ( i = 0; i < ncol; i++ )
		if ( !(eptrs[i] = TLgetentry( tid )) ) {
			bkerror( stderr, ERROR5 );
			cleanup();
		}

	prev_odev = NULL;

	/* Read up to ncol entries or to EOF or until the odevice changes, */
	/* whichever occurs first. Then print each field's values across as */
	/* one line with ncol columns.  */
	done = FALSE;
	colcnt = 0;
	while ( !done ) {
		while ( (colcnt < ncol) &&
			(linesleft = selectline( eptrs[colcnt] )) ) {
			odevice = TLgetfield( tid, eptrs[colcnt], R_ODEVICE );

			/* Got a new device. */
			if( strcmp( (char *)odevice, (char *)prev_odev ) != 0 ) {
				oname = TLgetfield( tid, eptrs[colcnt], R_ONAME );
				olabel = TLgetfield( tid, eptrs[colcnt], R_OLABEL );

				/* Reset previous device */
				if( prev_odev ) free( prev_odev );
				if( ( prev_odev =
					(unsigned char *)strdup((char *)odevice) )
					== NULL ) {
					bkerror( stderr, ERROR5 );
					cleanup();
				}

				/* Have some data collected - need to print it. */
				if ( colcnt > 0 ) {
					prthdr = TRUE;
					break;
				}

				/* If no data collected yet, print header */
				/* and continue collecting. */
				else {
					(void)fprintf( stdout, odevfmt,
						oname, odevice, olabel );
					colcnt++;
				}
			}
			else
				colcnt++;
		}

		if ( !linesleft ) done = TRUE;
		if ( colcnt == 0 )
			continue;
		if ( !(flags & hFLAG) )
			prcols = colcnt + 1;
		else prcols = colcnt;


		/* Print display with values for ncol lines for each field.*/
		for ( i = 0; i < fld_cnt; i++ ) {
			fldind = display[i];
			j = 0;

			if ( !(flags & hFLAG) )
				prvalues[j++] = fields[fldind].dispname;

			for( k = 0 ; k < colcnt; k++ ) {
				prvalues[j++] = TLgetfield( tid, eptrs[k],
						fields[fldind].fldname );
				if(!prvalues[j-1]) prvalues[j-1] = (unsigned char *)"";
			}

			prt_wrap( prcols, prvalues, vprlens );
		}

		colcnt = 0;
		if ( prthdr ) {
			(void)fprintf( stdout, odevfmt, oname, odevice, olabel );
			prthdr = FALSE;
			if ( (rc = TLread( tid, lineno, eptrs[colcnt++] ))
				!= TLOK ) {
				bkerror( stderr, ERROR8, lineno, rc );
				cleanup();
			}
		}
		else
			(void)fprintf( stdout, "\n" );
	}
}

/* Clean up and exit with error. */
static void
cleanup()
{
	TLclose( tid );
	exit( 2 );
}
