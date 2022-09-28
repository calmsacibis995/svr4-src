/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkhistory.d/bkhistory.c	1.8.3.1"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <table.h>
#include <bkhist.h>
#include <backup.h>
#include <brtoc.h>
#include <errors.h>
#include <bktypes.h>

#define TRUE	1
#define FALSE	0

#define NULSTR	(unsigned char *)""
#define DASH	'-'

#define LOWBITS	0xffff
#define FTBITS	0x7000	/* Mask off all bits except file type bits */
#define FIFO	0x1	/* bit in byte set for FIFO file type */
#define CHAR	0x2	/* bit in byte set for character special file type */
#define DIR	0x4	/* bit in byte set for directory file type */
#define BLK	0x6	/* bit in byte set for block special file type */

/* Size of chunks to malloc for dates. */
#define DATEINC	10

#define MODELEN	11	/* Number of characters to express file mode */

/* Enough characters to hold max input date format (mm/dd/yy hh:mm:ss pm  */
/* including terminating null). */
#define DATELEN	25

/* Number of characters in links field for ls -l type listing */
#define LINKLEN	3

/* Number of characters in size field for ls -l type listing */
#define SIZELEN	7

/* Size of chunks to malloc for entry queue. */
#define EMALLOCSZ	100

/* Number of fields to be printed in default listing */
#define DEFFLDCNT	7

/* Number of fields in header for long listing */
#define LONGHDCNT	3

/* Number of fields in value display for long listing */
#define LONGVALCNT	9

/* Flags to tell which options have been seen */
#define dFLAG	0x1	/* dates */
#define fFLAG	0x2	/* field separator character specified */
#define hFLAG	0x4	/* suppress headers */
#define lFLAG	0x8	/* ls -l listing of files */
#define pFLAG	0x10	/* set period */
#define oFLAG	0x20	/* filter on oname or odevice */
#define tFLAG	0x40	/* filter on tags */

/* name of this command */
char *brcmdname;

/* table id for bkhistory table */
int htid;

/* table id for toc table */
int toctid;

/* rwflag with which to open history table */
int rwflag = O_RDONLY;

/* flags seen */
int flags = 0;

/* field separator character */
char fld_sep;

/* Printing setup constructs */

/* Field names and lengths for default display. */
char *defnames[] = { "Tag", "Date", "Method", "Destination", "Dlabels",
"Vols", "TOC" };
int deflens[] = { 14, 13, 10, 11, 8, 4, 7 };

/* Field names and lengths for header for long display (-l). */
char *lhdnames[] = { "Tag", "Dlabels", "File information" };
int lhdlens[] = { 7, 7, 63 };

/* Field lengths for values for long display (-l). */
int lvallens[] = { 7, 7, 10, 3, 8, 8, 8, 12, 8 };

/* Time format strings */
char *tfmt = "%b %d %R %Y";
char *yfmt = "%Y";

/* Temp input date format - to be replaced by format string user input */
/* matched when (if) getdate() is modified to provide it. */
char *idfmt = "%m%d";

/* Buffer for time conversion - must provide enough space for corresponding */
/* format (see tfmt definition) */
char tbuf[20];
char ybuf[6];

/* Pointer to array of input dates. */
time_t idarray[DATEINC];
time_t *idates = idarray;

/* Count of number of input date pointers malloced. */
int idatesize = DATEINC;

/* Number of valid dates in idates array. */
int datecnt = 0;

/* pointer to array of pointers to oname/odevice strings user entered */
argv_t *o_argv;

/* pointer to array of pointers to tags user entered */
argv_t *t_argv;

/* period length for log truncation */
int period;

/* queue of pointers to entry structures */
ENTRY entryq[EMALLOCSZ];
ENTRY *entryqptr = entryq;

/* number of entries in the queue */
int entrycnt = -1;

/* size to malloc for next chunk of entry pointers */
int newsize = EMALLOCSZ * sizeof( ENTRY );

/* Pointer to array of pointers to volume names from bkhistory table */
argv_t *vnames;

/* for lint */
void exit();
void bkerror();

/* Program displays selected information from the backup history log or allows */
/* user to set the period after which information may be deleted from the log. */
/* Note that the -p option cannot be used with any other options.  The field */
/* separator can only be specified if the user chooses to suppress field wrap. */
main (argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	void display();
	void do_pflag();
	void qsort();
	void select();
	void synopsis();

	argv_t *s_to_argv();

	int error_seen = FALSE;
	int c;
	int compar();
	int getopt();
	int tblopen();
	int valid_date();

	char *o_arg;
	char *t_arg;
	char *bk_get_histlog_path();
	unsigned char *htable;
	unsigned char *p_integer();


	brcmdname = (char *)argv[0];

	while (( c = getopt( argc, argv, "d:f:hlo:p:t:?" )) != -1 )
		switch ( c ) {
		case 'd':
			flags |= dFLAG;
			if ( !valid_date( optarg ) )
				bkerror( stderr, ERROR10, optarg );
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

		case 'l':
			flags |= lFLAG;
			break;

		case 'o':
			flags |= oFLAG;
			o_arg = optarg;
			o_argv = s_to_argv( o_arg, ", " );
			break;

		case 'p':
			flags |= pFLAG;
			(void)p_integer( optarg, &period );
			if ( period <= 0 ) {
				error_seen = TRUE;
				bkerror( stderr, ERROR1, optarg );
			}
			else
				rwflag = O_RDWR;
			break;

		case 't':
			flags |= tFLAG;
			t_arg = optarg;
			t_argv = s_to_argv( t_arg, ", " );
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

	/* Check for non-option, invalid arguments */
	if ( optind < argc ) {
		bkerror( stderr, ERROR13, argv[optind] );
		synopsis();
		exit( 1 );
	}

	if ( (flags & dFLAG) && (datecnt == 0) ) {
		bkerror( stderr, ERROR3 );
		error_seen = TRUE;
	}

	if ( flags & pFLAG ) {
		if ( (flags & dFLAG) || (flags & fFLAG) || (flags & hFLAG) ||
		     (flags & lFLAG) || (flags & oFLAG) || (flags & tFLAG) ) {
			bkerror( stderr, ERROR2, 'p' );
			error_seen = TRUE;
		}
	}

	if ( error_seen )
		exit( 1 );

	htable = (unsigned char *)bk_get_histlog_path();
	if ( !tblopen( &htid, htable, rwflag ) )
		exit ( 2 );

	if ( flags & pFLAG )
		do_pflag( htid, htable );
	else {
		select();
		qsort( (char *)entryqptr, (unsigned)entrycnt, sizeof( ENTRY ), compar );
		display();
	}

	TLclose( htid );
	exit( 0 );
}

/* Validate the dates, warning that invalid ones will be ignored. */
/* Currently, convert dates into time_t value (returned by brgetdate). */
/* Comparison routine will convert time_t to string representation for */
/* matching with dates in table. */
int
valid_date( string )
char *string;
{
	time_t tdate;
	time_t brgetdate();

	void *realloc();

	if ( (tdate = brgetdate( string )) == NULL )
		return( FALSE );

	if ( datecnt == idatesize )
		if ( !(idates = (time_t *)realloc( (idatesize + DATEINC)
			* sizeof( time_t ) ))) {
			bkerror( stderr, ERROR11 );
			exit( 1 );
		}

	idates[datecnt++] = tdate;

	return( TRUE );
}

/* Set rotation period in table to new value. */
void
do_pflag(tid, table)
int tid;
unsigned char *table;
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
void
select()
{
	register int entryno;
	int rc;
	int matchone();
	int matchdates();

	ENTRY eptr;
	ENTRY nextentry();

	eptr = nextentry();

	entryno = 1;
	while ( (rc = TLread( htid, entryno++, eptr )) != TLBADENTRY ) {

		if ( (rc == TLBADID) || (rc == TLARGS) || (rc == TLDIFFFORMAT)
			|| (rc == TLNOMEMORY) ) {
			bkerror( stderr, ERROR7, entryno-1, rc );
			TLclose( htid );
			exit( 2 );
		}

		/* Ignore comment lines. */
		if ( TLgetfield( htid, eptr, TLCOMMENT ) != NULL )
			continue;

		if ( (t_argv != NULL) && ((*t_argv)[0] != NULL) ) 
			if ( !matchone( t_argv, eptr, H_TAG ) )
				continue;

		if ( datecnt > 0 )
			if ( !matchdates( eptr ) )
				continue;

		if ( (o_argv != NULL) && ((*o_argv)[0] != NULL) )
			if ( !matchone( o_argv, eptr, H_ONAME )  &&
			     !matchone( o_argv, eptr, H_ODEVICE ) )
				continue;

		eptr = nextentry();
	}

	/* Free last (unused) entry */
	*(entryqptr + entrycnt) = (ENTRY)NULL;
	TLfreeentry( htid, eptr );
}

/* Returns pointer to next entry structure in the entry queue.  Only called */
/* when previously returned pointer has been used to hold a table entry. */
/* Takes care of allocating more space when number of entries exceeds number */
/* of allocated slots. */
ENTRY
nextentry()
{
	void *realloc();

	ENTRY neptr;

	void cleanup();

	entrycnt++;

	if ( (entrycnt > 0) && (entrycnt % EMALLOCSZ == 0) ) {
		newsize *= ((entrycnt / EMALLOCSZ) + 1);
		entryqptr = (ENTRY *)realloc( (char *)entryqptr, (unsigned)newsize );
	}

	if( (neptr = TLgetentry( htid )) == NULL ) {
		bkerror( stderr, ERROR9 );
		cleanup();
		exit( 2 );
	}
	*(entryqptr + entrycnt) = neptr;

	return( *(entryqptr + entrycnt) );
}

/* Print header, if required, then print table values. */
void
display()
{
	void longdisp();
	void defdisp();

	if (flags & lFLAG)
		longdisp();
	else
		defdisp();
}

/* Print long display. */
void
longdisp()
{
	void prt_nowrhead();
	void prt_header();
	void prt_lvalues();

	if ( !(flags & hFLAG) )
		if ( flags & fFLAG )
			prt_nowrhead( LONGHDCNT, lhdnames );
		else
			prt_header( LONGHDCNT, lhdnames, lhdlens );
	prt_lvalues();
}

/* Set up array of field values for long display and then print them, */
/* working through the sorted queue one entry at a time. */
void
prt_lvalues()
{
	unsigned char *prvalues[LONGVALCNT];
	unsigned char *tocptr;

	unsigned char cmode[MODELEN];
	unsigned char *gid;
	unsigned char *mode;
	unsigned char *uid;
	char fsize[SIZELEN + 1];
	char nlinks[LINKLEN + 1];
	char *tptr;

	struct group *grp;

	struct passwd *pwp;

	int i;
	int entryno;
	int rc;
	int atoi();
	short mflag;

	ENTRY eptr;
	ENTRY teptr;

	time_t date;
	time_t now;
	long strtol();

	void pmode();
	void cleanup();
	void prt_lnowrap();
	void prt_wrap();

	for ( i = 0; i < entrycnt; i++ ) {
		eptr = *(entryqptr + i);

		prvalues[0] = TLgetfield( htid, eptr, H_TAG );

		/* Get toc file name, open file if it is nonnull, then */
		/* collect and print values from each line in the toc. */
		if ( (tocptr = TLgetfield( htid, eptr, H_TOCNAME )) == NULL  ||
			(*tocptr == NULL) )
			continue;

		if( !tblopen( &toctid, tocptr,  O_RDONLY ) )
			continue;

		if( (teptr = TLgetentry( toctid )) == NULL ) {
			bkerror( stderr, ERROR9 );
			cleanup();
			exit( 2 );
		}

		entryno = 1;
		while ( (rc = TLread( toctid, entryno++, teptr )) != TLBADENTRY ) {
			if ( (rc == TLARGS) || (rc == TLBADID) || (rc == TLNOMEMORY)
			     || (rc == TLDIFFFORMAT) ) {
				bkerror( stderr, ERROR7, entryno-1, rc );
				cleanup();
				exit( 2 );
			}

			/* Ignore comment entries. */
			if ( TLgetfield( toctid, teptr, TLCOMMENT ) != NULL )
				continue;
 
			/* Set up list of volume names. */
			prvalues[1] = TLgetfield( toctid, teptr, TOC_VOL );

			/* Get mode flag number and convert to rwx notation. */
			/* NOTE: pmode code lifted from ls command. */
			if ( (mode = TLgetfield( toctid, teptr, TOC_MODE )) == NULL)
				mode = NULSTR;
			else {
				mflag = strtol(mode, (char **)NULL, 16) & LOWBITS;
				pmode( mflag, cmode );
			}
			prvalues[2] = cmode;

			/* Number of links to file - right-justified */
			sprintf(nlinks, "%*s", LINKLEN,
				TLgetfield( toctid, teptr, TOC_NLINK ) );
			prvalues[3] = (unsigned char *)nlinks;

			/* Login - must convert from uid.  If not in password */
			/* file, use uid. */
			if ( (uid = TLgetfield( toctid, teptr, TOC_UID )) == NULL
				|| !*uid )
				prvalues[4] = NULSTR;
			else  {
				pwp = getpwuid( atoi( uid ) );
				if ( pwp != NULL )
					prvalues[4] = (unsigned char *)pwp->pw_name;
				else prvalues[4] = uid;
			}

			/* Group - must convert from gid.  If not in password */
			/* file, use gid. */
			if ( (gid = TLgetfield( toctid, teptr, TOC_GID )) == NULL
				|| !*gid )
				prvalues[5] = NULSTR;
			else {
				grp = getgrgid( atoi( gid ) );
				if ( grp != NULL )
					prvalues[5] = (unsigned char *)grp->gr_name;
				else prvalues[5] = gid;
			}

			/* File size - right-justified */
			sprintf(fsize, "%*s", SIZELEN,
				TLgetfield( toctid, teptr, TOC_SIZE ) );
			prvalues[6] = (unsigned char *)fsize;

			/* Convert character representation of time_t date into */
			/* a date string.  If year is same as this year, print */
			/* "Mon DD hh:mm", otherwise print "Mon DD YYYY". */
			tptr = (char *)TLgetfield( toctid, teptr, TOC_MTIME ); 
			if ( *tptr == NULL ) {
				tbuf[0] = (char)NULL;
			}
			else {
				date = (time_t) strtol( tptr, (char **)NULL, 16 );
				cftime( tbuf, tfmt, &date );
			}
			now = time( NULL );
			cftime( ybuf, yfmt, &now );
			if ( strncmp( ybuf, &tbuf[13], 4 ) == 0 )
				tbuf[12] = (char)NULL;
			else {
				strncpy( &tbuf[7], &tbuf[13], 4 );
				tbuf[11] = (char)NULL;
			}
			prvalues[7] = (unsigned char *)tbuf;

			/* File name */
			prvalues[8] = TLgetfield( toctid, teptr, TOC_FNAME );

			if ( flags & fFLAG )
				prt_lnowrap( LONGHDCNT, LONGVALCNT,
					     prvalues, lvallens, fld_sep );
			else
				prt_wrap( LONGVALCNT, prvalues, lvallens );
		}
	}
}

/* Print the long version with no wrap of field entries.  Print file */
/* information as one field, with blank fill between fields of the ls -l */
/* listing. */
void
prt_lnowrap( hdcnt, valcnt, prvalues, lvallens, fld_sep )
int hdcnt;
int valcnt;
unsigned char *prvalues[];
int lvallens[];
char fld_sep;
{
	int i;

	for ( i = 0; i < hdcnt - 1; i++ )
		fprintf( stdout, "%s%c", prvalues[i], fld_sep );
	for ( i = hdcnt - 1; i < valcnt - 1; i++ )
		fprintf( stdout, "%-*s ", lvallens[i], prvalues[i] );
	fprintf( stdout, "%s\n", prvalues[valcnt - 1] );
}

/* close tables and free up entries */
void
cleanup()
{
	TLclose( toctid );
	TLclose( htid );
}

void
pmode(aflag, cbuf)
short aflag;
unsigned char *cbuf;
{
	int selectmode();

        /* these arrays are declared static to allow initializations */
	static int	m0[] = { 1, S_IREAD>>0, 'r', '-' };
	static int	m1[] = { 1, S_IWRITE>>0, 'w', '-' };
	static int	m2[] = { 3, S_ISUID|S_IEXEC, 's', S_IEXEC, 'x', S_ISUID, 'S', '-' };
	static int	m3[] = { 1, S_IREAD>>3, 'r', '-' };
	static int	m4[] = { 1, S_IWRITE>>3, 'w', '-' };
	static int	m5[] = { 3, S_ISGID|(S_IEXEC>>3),'s', S_IEXEC>>3,'x', S_ISGID,'l', '-'};
	static int	m6[] = { 1, S_IREAD>>6, 'r', '-' };
	static int	m7[] = { 1, S_IWRITE>>6, 'w', '-' };
	static int	m8[] = { 3, S_ISVTX|(S_IEXEC>>6),'t', S_IEXEC>>6,'x', S_ISVTX,'T', '-'};

        static int  *m[] = { m0, m1, m2, m3, m4, m5, m6, m7, m8};

	register int **mp;
	int ftype;
	int i = 0;

	/* Figure out file type */
	ftype = (aflag & FTBITS) >> 12;
	if ( ftype & FIFO )
		cbuf[i] = 'p';
	else if ( ftype & CHAR )
		cbuf[i] = 'c';
	else if ( ftype & DIR )
		cbuf[i] = 'd';
	else if ( ftype & BLK )
		cbuf[i] = 'b';
	else
		cbuf[i] = '-';

	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])];)
		cbuf[++i] = (unsigned char)selectmode(*mp++, aflag);
	cbuf[++i] = NULL;
}

int
selectmode(pairp, aflag)
register int *pairp;
short aflag;
{
	register int n;

	n = *pairp++;
	while (n-->0) {
		if((aflag & *pairp) == *pairp) {
			pairp++;
			break;
		}else {
			pairp += 2;
		}
	}
	return( *pairp );
}
/* Default display */
void
defdisp()
{
	void prt_nowrhead();
	void prt_header();
	void prt_dvalues();

	if ( !(flags & hFLAG) )
		if ( flags & fFLAG )
			prt_nowrhead( DEFFLDCNT, defnames );
		else
			prt_header( DEFFLDCNT, defnames, deflens );
	prt_dvalues();
}

/* Set up array of field values for default display and then print them, */
/* working through the sorted queue one entry at a time. */
void
prt_dvalues()
{
	unsigned char *prvalues[DEFFLDCNT];
	unsigned char *arch;
	unsigned char *destdev;
	unsigned char *tocname;
	char *tptr;

	int i;

	ENTRY eptr;

	time_t date;
	long strtol();

	void prt_wrap();
	void prt_nowrap();

	for ( i = 0; i < entrycnt; i++ ) {
		eptr = *(entryqptr + i);

		prvalues[0] = TLgetfield( htid, eptr, H_TAG );

		/* Convert character representation of time_t date into a date */
		/* string */
		tptr = (char *)TLgetfield( htid, eptr, H_DATE ); 
		if ( *tptr == NULL ) {
			tbuf[0] = DASH;
			tbuf[1] = (char)NULL;
		}
		else {
			date = (time_t) strtol( tptr, (char **)NULL, 16 );
			cftime( tbuf, tfmt, &date );
		}
		prvalues[1] = (unsigned char *)tbuf;

		prvalues[2] = TLgetfield( htid, eptr, H_METHOD ); 
		/* if the ddev field is null, print the dgroup */
		destdev = TLgetfield( htid, eptr, H_DDEVICE );
		if ( *destdev == NULL )
			destdev = TLgetfield( htid, eptr, H_DGROUP );
			if ( *destdev  == NULL )
				destdev = (unsigned char *)NULSTR;
		prvalues[3] = destdev;

		prvalues[4] = TLgetfield( htid, eptr, H_DMNAME );
		prvalues[5] = TLgetfield( htid, eptr, H_DNVOL );

		arch = TLgetfield( htid, eptr, H_ARCHTOC );
		tocname = TLgetfield( htid, eptr, H_TOCNAME );
		if ( strcmp( (char *)arch, "Y" ) == 0 )
			if ( *tocname == NULL )
				prvalues[6] = (unsigned char *)"Archive";
			else
				prvalues[6] = (unsigned char *)"Both";
		else if ( strcmp( (char *)arch, "N" ) == 0 )
			if ( *tocname == NULL )
				prvalues[6] = (unsigned char *)"None";
			else
				prvalues[6] = (unsigned char *)"Online";
		else prvalues[6] = (unsigned char *)"?";

		if ( flags & fFLAG )
			prt_nowrap( DEFFLDCNT, prvalues, fld_sep );
		else
			prt_wrap( DEFFLDCNT, prvalues, deflens );
	}
}


/* Synopsis of command invocation. */
void
synopsis()
{
	(void)fprintf( stderr, "%s [-hl] [-f field_separator] [-d dates] [-o names] [-t tags]\n",
			brcmdname );
	(void)fprintf( stderr, "%s -p period\n", brcmdname );
}

/* Open table. */
int
tblopen( tid, table, rwflag )
int *tid;
unsigned char *table;
int rwflag;
{
	int rc;
	int is_new;
	int stat();
	int insert_format();

	struct stat buf;
	struct TLdesc description;

	strncpy( (char *)&description, "", sizeof( struct TLdesc ) );

	is_new = (stat( (char *)table, &buf ) != 0);

	/* If this isn't a regular file, it isn't a valid table. */
	if ( !(buf.st_mode & S_IFREG) ) {
		bkerror( stderr, ERROR14, table );
		return( FALSE );
	}

	if ( is_new ) {
		if (rwflag == O_RDONLY) {
			bkerror( stderr, ERROR12, table );
			return( FALSE );
		}
		else
			description.td_format = H_ENTRY_F;
	}

	if (( rc = TLopen( tid, table, &description, rwflag, 0777 )) != TLOK
		&& rc != TLBADFS ) {
		if ( rc == TLDIFFFORMAT )
			bkerror( stderr, ERROR8, table ); /* warning */
		else {
			if ( rc == TLFAILED ) perror( brcmdname );
			else bkerror( stderr, ERROR6, table, rc );
			return ( FALSE );
		}
	}

	if( is_new )
		(void)insert_format( *tid, H_ENTRY_F );
	return ( TRUE );
}


/* Print header */
void
prt_header(fldcnt, fldnames, fldlens )
int fldcnt;
char *fldnames[];
int fldlens[];
{
	int dashes = 0;
	int i;

	for (i = 0; i < fldcnt; i++ )
		dashes += fldlens[i];
	dashes += fldcnt - 1; /* Account for blanks between fields. */

	(void)fprintf( stdout, "\n" );
	for (i = 0; i < fldcnt - 1; i++ )
		(void)fprintf( stdout, "%-*s ", fldlens[i], fldnames[i] );
	(void)fprintf( stdout, "%-*s\n", fldlens[fldcnt - 1], fldnames[fldcnt - 1] );

	for ( i=0; i < dashes; i++ )
		(void)fprintf( stdout, "-" );
	(void)fprintf( stdout, "\n" );
}

void
prt_nowrhead(fldcnt, fldnames )
int fldcnt;
char *fldnames[];
{
	int i;
	for (i = 0; i < fldcnt - 1; i++)
		(void)fprintf( stdout, "%s%c", fldnames[i], fld_sep );
	(void)fprintf( stdout, "%s\n", fldnames[fldcnt - 1] );
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
	fldval = TLgetfield( htid, eptr, fldname );
	if( (fldval == NULL ) || (*fldval == NULL) )
		return( FALSE );

	i = 0;
	while( (*argvp)[i] != NULL ) {
		if ( strcmp( (char *)fldval, (*argvp)[i++] ) == 0 )
			return( TRUE );
	}
	return( FALSE );
}

/* Pull date out of history table and verify that it matches one of dates */
/* user gave.  Note that date in table is character representation of hex */
/* time_t.  Needs to be converted to character string date of the form */
/* the user entered.  This is then matched with the date the user entered, */
/* in the same form. */
/* For the time being, without getdate() information about what the user */
/* entered, convert date from table to a fixed format and compare against */
/* the input value converted to same format. */
int
matchdates( eptr )
ENTRY eptr;
{
	int i;
	long strtol();
	time_t ttime;
	unsigned char *fldval;
	char confldval[DATELEN];
	char conidate[DATELEN];

	/* A null field value cannot match what the user entered. */
	fldval = TLgetfield( htid, eptr, H_DATE );
	if( (fldval == NULL ) || (*fldval == NULL) )
		return( FALSE );

	/* Convert string representation of time_t quantity to a human- */
	/* readable date format. */
	ttime = (time_t)strtol( (char *)fldval, (char **)NULL, 16 );
	if ( cftime( confldval, idfmt, &ttime ) == 0)
		return( FALSE );

	for ( i = 0; i < datecnt; i++ ) {
		cftime( conidate, idfmt, idates + i );
		if ( strcmp( confldval, conidate ) == 0 )
			return( TRUE );
	}
	return( FALSE );
}

/* Function returns 0 if objects pointed to by arguments are equal, <0 if arg1 */
/* should appear before arg2 in a sort, >0 if arg2 should appear first. */
/* Currently sorts table entries first by tag, then by date (most */
/* recent job first) of each backup operation. */
compar( e1ptr, e2ptr )
ENTRY *e1ptr;
ENTRY *e2ptr;
{
	unsigned char *tag1;
	unsigned char *tag2;
	unsigned char *time1;
	unsigned char *time2;

	int strc;
	long strtol();

	tag1 = TLgetfield( htid, *e1ptr, H_TAG );
	tag2 = TLgetfield( htid, *e2ptr, H_TAG );

	if ( (strc = strcmp( (char *)tag1, (char *)tag2 )) == 0 ) {
		time1 = TLgetfield( htid, *e1ptr, H_DATE );
		time2 = TLgetfield( htid, *e2ptr, H_DATE );

		if ( strcmp( (char *)time1, (char *)time2 ) == 0 )
			return( 0 );
		else {
			if ( *time1 == NULL )
				return( -1 );
			else if ( *time2 == NULL )
				return( 1 );
			else
				return( strtol( time2, (char **)NULL, 16 ) -
				     strtol( time1, (char **)NULL, 16 ) );
		}
	}
	else return( strc );
}
