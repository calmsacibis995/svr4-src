/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkexcept.d/bkexcept.c	1.6.4.1"

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <table.h>
#include <bkexcept.h>
#include <bktypes.h>
#include <errors.h>

#define TRUE	1
#define FALSE	0

/* Size of chunks to malloc for entry queue. */
#define EMALLOCSZ	200

/* Flags to tell which options have been seen */
#define aFLAG	0x1	/* add patterns */
#define CFLAG	0x2	/* compatibility mode - translate old style exception */
			/* lists to new format if possible */
#define dFLAG	0x4	/* display entries matching given patterns */
#define rFLAG	0x8	/* remove patterns */
#define	tFLAG	0x10 /* non-default table */

/* Header for searches in anchored mode */
#define ANCHHDR	"\nFiles in the exception list beginning with %s:\n"

/* Header for searches in unanchored mode */
#define CONTHDR	"\nFiles in the exception list containing the pattern \"%s\":\n"

/* Default exception list (old version) for translation option. */
#define EXCFILE	"/etc/save.d/except"

/* Sed script name for translation option. */
#define SEDFILE	"exconv.sed"

/* bkexcept table */
unsigned char *table;

/* name of this command */
char *brcmdname;

/* table id for bkexcept table */
int tid;

/* rwflag with which to open table */
int rwflag = O_RDONLY;

/* flags seen */
int flags = 0;

/* pointer to array of pointers to pattern strings user entered */
argv_t *p_argv;

/* table search criteria structures */
struct TLsearch TLsearches[ TL_MAXFIELDS ];

/* queue of pointers to entry structures */
ENTRY entryq[EMALLOCSZ];
ENTRY *entryqptr = entryq;

/* Size to malloc for next chunk of entry pointers */
int newsize = EMALLOCSZ * sizeof( ENTRY );

/* temporary eptr in case we run out of memory - still need one available */
/* to write out entries in memory to a temp file for sorting */
ENTRY teptr;

/* number of entries in the queue */
int entrycnt = 0;

/* maximum number of queue entries allocated that are still valid */
/* they can be reused */
int maxused = -1;

/* input buffer for patterns on stdin */
char inpbuf[BUFSIZ];

/* keep lint happy */
void exit();
void bkerror();

/* pathname for -t argument */
char *path;

/* Program displays or edits backup exception list table */
main (argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	void add();
	void display();
	void rmove();
	void translate();
	void synopsis();

	argv_t *s_to_argv();

	int error_seen = FALSE;
	int c;
	int getopt();
	int tblopen();
	char *Tptr;

	brcmdname = (char *)argv[0];

	while (( c = getopt( argc, argv, "a:Cd:r:t:?" )) != -1 )
		switch ( c ) {
		case 'a':
			flags |= aFLAG;
			rwflag = O_RDWR;
			Tptr=optarg;
			/** remove newlines that OAM may put in **/
			while(*Tptr)
			{
				if(*Tptr == '\n')
					*Tptr=' ';
				++Tptr;
			}
			p_argv = s_to_argv( optarg, ", " );
			if ( (p_argv == NULL) || (strlen((*p_argv)[0]) == 0) ) {
				bkerror( stderr, ERROR14, "add to" );
				error_seen = TRUE;
			}
			break;

		case 'C':
			flags |= CFLAG;
			break;

		case 'd':
			/* If user specifies null pattern, warn user and */
			/* display whole table. */
			p_argv = s_to_argv( optarg, ", " );
			if ( (p_argv == NULL) || (strlen((*p_argv)[0]) == 0) )
				bkerror( stderr, ERROR15 );
			else
				flags |= dFLAG;
			break;

		case 'r':
			flags |= rFLAG;
			rwflag = O_RDWR;
			Tptr=optarg;
			/** remove newlines that OAM may put in **/
			while(*Tptr)
			{
				if(*Tptr == '\n')
					*Tptr=' ';
				++Tptr;
			}
			p_argv = s_to_argv( optarg, ", " );
			if ( (p_argv == NULL) || (strlen((*p_argv)[0]) == 0) ) {
				bkerror( stderr, ERROR14, "remove from" );
				error_seen = TRUE;
			}
			break;

		case 't':
			flags |= tFLAG;
			path = optarg;
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

	if ( flags & aFLAG ) {
		if ( (flags & CFLAG) || (flags & dFLAG) || (flags & rFLAG) ) {
			bkerror( stderr, ERROR1 );
			error_seen = TRUE;
		}
	}
	else if ( flags & CFLAG ) {
		if ( (flags & dFLAG) || (flags & rFLAG) ) {
			bkerror( stderr, ERROR1 );
			error_seen = TRUE;
		}
	}
	else if ( (flags & dFLAG) && (flags & rFLAG) ) {
			bkerror( stderr, ERROR1 );
			error_seen = TRUE;
	}

	/* Check for non-option, invalid arguments */
	if ( !(flags & CFLAG) )
		if ( optind < argc ) {
			bkerror( stderr, ERROR16, argv[optind] );
			synopsis();
			exit( 1 );
		}

	if ( error_seen )
		exit( 1 );

	/* Reset SIGCLD to default - this is to avoid wait problems. */
	signal(SIGCLD, SIG_DFL );

	/* Do translation of user files. */
	if ( flags & CFLAG ) {
		translate( argv, argc, optind );
		exit( 0 );
	}

	/* Otherwise, do display or modification of exception list. */
	if ( !tblopen( path, rwflag ) )
		exit ( 2 );

	/* get the temporary eptr - if we can't do this, we can't continue */
	if ( (teptr = TLgetentry( tid )) == NULL ) {
		bkerror( stderr, ERROR2 );
		TLclose( tid );
		exit( 2 );
	}

	if ( flags & aFLAG )
		add();
	else if ( flags & rFLAG )
		rmove();
	else
		display();

	TLclose( tid );
	exit( 0 );


}

/* Add patterns to the exception list table. */
/* If one of the patterns specified is a -, read from stdin. */
void
add()
{
	char *pat;

	int i;
	int rc;
	struct TLsearch *tls = TLsearches;
	int entryno;

	i = 0;

	while ( (pat = (*p_argv)[i++]) != NULL ) {
		if ( strcmp( pat, "-" ) == 0 ) {
			pat = inpbuf;
			while ( gets( pat ) != NULL ) {
				if ( (rc = TLassign( tid, teptr, EX_EXCPATH, pat ))
					!= TLOK ) {
					bkerror( stderr, ERROR3, "assign to",
						table, rc );
					TLsync( tid );
					TLclose( tid );
					exit( 2 );
				}
				if( (rc = TLappend( tid, TLEND, teptr )) != TLOK ) {
					bkerror( stderr, ERROR3, "append to",
						table, rc );
					TLsync( tid );
					TLclose( tid );
					exit( 2 );
				}
			}
		}
		else {


			tls = TLsearches;
			tls->ts_fieldname =  EX_EXCPATH;
			tls->ts_pattern = (unsigned char *)pat;
			tls->ts_operation = (int (*)())TLEQ;
			tls++;
			tls->ts_fieldname = (unsigned char *)NULL;

			if ( (entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND ))
				!= TLFAILED )
			{
		
				/** Entry already exists, don't duplicate **/
				/** go on to the next entry **/
				continue;
			}

			if ( (rc = TLassign( tid, teptr, EX_EXCPATH, pat ))
				!= TLOK ) {
				bkerror( stderr, ERROR3, "assign to", table, rc );
				TLsync( tid );
				TLclose( tid );
				exit( 2 );
			}
			if( (rc = TLappend( tid, TLEND, teptr )) != TLOK ) {
				bkerror( stderr, ERROR3, "append to", table, rc );
				TLsync( tid );
				TLclose( tid );
				exit( 2 );
			}
		}
	}
	if( (rc = TLsync( tid )) != TLOK ) {
		bkerror( stderr, ERROR3, "sync", table, rc );
		TLclose( tid );
		exit( 2 );
	}
}

/* Delete specified patterns from the table. */
void
rmove()
{
	char *pat;

	int i;
	int rc;
	void delete();

	i = 0;
	while ( (pat = (*p_argv)[i++]) != NULL ) {
		if ( strcmp( pat, "-" ) == 0 ) {
			pat = inpbuf;
			while( gets( pat ) != NULL )
				delete( pat );
		}
		else
			delete( pat );
	}
	if ( ( rc = TLsync( tid ) ) != TLOK ) {
		bkerror( stderr, ERROR3, "sync", table, rc );
		TLclose( tid );
		exit( 2 );
	}
}

/* Do actual deletion from table.  Note that if the pattern is not found, */
/* this is not treated as an error.  The pattern is not in the table. */
void
delete( pat )
char *pat;
{
	struct TLsearch *tls = TLsearches;
	int entryno;
	int rc;

	tls->ts_fieldname =  EX_EXCPATH;
	tls->ts_pattern = (unsigned char *)pat;
	tls->ts_operation = (int (*)())TLEQ;
	tls++;
	tls->ts_fieldname = (unsigned char *)NULL;

	/** The while(1) loop makes sure that ALL entries are deleted **/
	while(1)
	{
		if ( (entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND ))
			== TLFAILED )
			break;
	
		if ( (entryno == TLBADID) || (entryno == TLARGS) ||
		     (entryno == TLBADENTRY) || (entryno == TLBADFIELD) ) {
			bkerror( stderr, ERROR4, table, entryno );
			TLsync( tid );
			TLclose( tid );
			exit( 2 );
		}
	
		if ( (rc = TLdelete( tid, entryno )) != TLOK ) {
			bkerror( stderr, ERROR5, entryno, table, rc );
			TLsync( tid );
			TLclose( tid );
			exit( 2 );
		}
		fprintf( stdout, "%s\n", pat );
	}
	return;
}

/* Display lines from table.  If dFLAG is set, only display selected lines, */
/* otherwise, display all of them. */
void
display()
{
	void printall();
	void printsome();

	if ( flags & dFLAG )
		printsome();
	else
		printall();
}

/* Print all lines in the table in alphabetical order (i.e., collating */
/* sequence).  Try to sort in-core, otherwise write and sort temp file. */
void
printall()
{
	int entryno = 1;
	int i;
	int rc;
	int nospace = FALSE;
	int compar();
	char *ptr;

	ENTRY eptr;

	void qsort();
	void prttmp();

	void *malloc();

	/* Count number of lines in table, disregarding commentary entries. */
	while ( (rc = TLread( tid, entryno++, teptr)) == TLOK )
		if ( TLgetfield( tid, teptr, TLCOMMENT ) == NULL )
			entrycnt++;

	if( rc != TLBADENTRY ) {
		bkerror( stderr, ERROR9, entryno - 1, table, rc );
		TLclose( tid );
		exit( 2 );
	}

	/* Try to allocate space for pointers to entries, then for the */
	/* entries themselves. */
	if ( (entrycnt < EMALLOCSZ ) &&
	     (entryqptr = (ENTRY *)malloc( (unsigned)(entrycnt * sizeof( ENTRY )) ))
			!= NULL ) {
		for( i = 0; i < entrycnt; i++ )
			if ( ( *(entryqptr + i) = TLgetentry( tid )) == NULL ) {
				nospace = TRUE;
				break;
			}
	}
	else nospace = TRUE;

	/* Out of space - use a temp file. */
	if ( nospace )
		prttmp();

	/* ok for in-core sort */
	else {
		int count = 1;
		eptr = *entryqptr;
		i = 1;
		rc = TLOK;

		/* read in non-commentary entries */
		while ( count <= (entryno-2)
			&& (rc = TLread( tid, count++, eptr )) == TLOK ) {
			if ( TLgetfield( tid, eptr, TLCOMMENT ) != NULL )
				continue;

			eptr = *(entryqptr + i);
			i++;
		}

		if( rc != TLOK ) {
			bkerror( stderr, ERROR9, (count-1), table, rc );
			TLclose( tid );
			exit( 2 );
		}

		/* sort and print entries */
		qsort( (char *)entryqptr, entrycnt, sizeof( ENTRY ), compar );
		fprintf( stdout, "Files in exception list:\n" );
		for ( i = 0; i < entrycnt; i++ ) {
			ptr = (char *) TLgetfield( tid, *(entryqptr + i), EX_EXCPATH );
			if( ptr && *ptr )
				fprintf( stdout, "%s\n", ptr );
		}
	}
}

/* Sort and print entire exception list table using a temporary file. */
void
prttmp()
{
	char cmdstr[512];
	char tmpfile[20];
	char *fldval;

	int entryno;
	int rc;
	extern int errno;
	pid_t getpid();
	int unlink();

	FILE *fp;

	/* generate temp file name */
	sprintf( tmpfile, "/tmp/bkexc%ld", getpid() );

	/* open tmpfile */
	if ( (fp = fopen( tmpfile, "w" )) == NULL ) {
		bkerror( stderr, ERROR6, tmpfile );
		TLclose( tid );
		exit( 2 );
	}

	/* read table and write non-commentary entries to tempfile */
	entryno = 1;
	while ( (rc = TLread( tid, entryno++, teptr )) == TLOK ) {
		if ( TLgetfield( tid, teptr, TLCOMMENT ) != NULL )
			continue;

		if ( ( fldval = (char *)TLgetfield( tid, teptr, EX_EXCPATH ))
			!= NULL && (*fldval != NULL) )
			fprintf( fp, "%s\n", fldval );
	}
	fclose( fp );
	if ( rc != TLBADENTRY ) {
		bkerror( stderr, ERROR9, entryno - 1, table, rc );
		TLclose( tid );
		unlink( tmpfile );
		exit( 2 );
	}

	/* print header */
	fprintf( stdout, "Files in exception list:\n" );

	/* sort and print tmpfile */
	sprintf( cmdstr, "sort %s", tmpfile );
	if ( system( cmdstr ) < 0 ) {
		bkerror( stderr, ERROR7, errno );
		TLclose( tid );
		unlink( tmpfile );
		exit( 2 );
	}

	if ( unlink( tmpfile ) < 0 ) {
		bkerror( stderr, ERROR8, tmpfile, errno );
		TLclose( tid );
		exit( 2 );
	}
}

/* Print lines from file selected based on patterns the user gave. */
void
printsome()
{
	char *pat;

	void printmatch();

	int i = 0;

	while ( (pat = (*p_argv)[i++]) != NULL ) {
		if ( strcmp( pat, "-" ) == 0 ) {
			pat = inpbuf;
			while ( gets( pat ) != NULL )
				printmatch( pat );
		}
		else
			printmatch( pat );
	}
}

/* Search through table and add lines that match criteria to the entry queue. */
/* Then sort lines and print them. */
void
printmatch( pat )
char *pat;
{
	register int entryno;
	int i;
	int lineno;
	int rc;
	int nospace = FALSE;
	int compar();
	int tsmatch();
	int tscont();

	char *hdr;
	unsigned char *esc_sp();

	void qsort();
	void mprttmp();

	struct TLsearch *tls = TLsearches;

	ENTRY eptr;
	ENTRY nextentry();

	entrycnt = -1;
	if ( (eptr = nextentry()) != NULL ) {

		tls->ts_fieldname =  EX_EXCPATH;
		tls->ts_pattern = (unsigned char *)pat;
		if ( pat[0] == '/' ) {
			tls->ts_operation = tsmatch;
			hdr = ANCHHDR;
		}
		else {
			tls->ts_operation = tscont;
			hdr = CONTHDR;
		}
		tls++;
		tls->ts_fieldname = (unsigned char *)NULL;

		lineno = 1;
		while ( ((entryno = TLsearch1( tid, TLsearches, lineno, TLEND, TL_AND ))
			!= TLFAILED) && (entryno != TLBADENTRY) ) {

			if ( (entryno == TLBADID) || (entryno == TLARGS) ||
		     	     (entryno == TLBADFIELD) ) {
				bkerror( stderr, ERROR4, table, entryno );
				TLclose( tid );
				exit( 2 );
			}

			if ( (rc = TLread( tid, entryno, eptr )) != TLOK ) {
				bkerror( stderr, ERROR9, entryno, rc );
				exit( 2 );
			}

			if ( (eptr = nextentry()) == NULL ) {
				nospace = TRUE;
				break;
			}
			lineno = entryno + 1;
		}
	}
	else
		nospace = TRUE;

	/* If we ran out of space, use a temp file. */
	if ( nospace )
		mprttmp( entrycnt - 1, pat, hdr );
	else {
		/* Free last (unused) entry */
		*(entryqptr + entrycnt) = (ENTRY)NULL;
		TLfreeentry( tid, eptr );
		maxused =  entrycnt - 1;

		/* sort entries */
		if ( entrycnt > 1 )
			qsort( (char *)entryqptr, entrycnt, sizeof( ENTRY ), compar );

		fprintf( stdout, hdr, pat );
		for ( i = 0; i < entrycnt; i++ )
			fprintf( stdout, "%s\n",
				TLgetfield( tid, *(entryqptr + i), EX_EXCPATH ) );
	}
}

/* Returns pointer to next entry structure in the entry queue.  Only called */
/* when previously returned pointer has been used to hold a table entry. */
/* Takes care of allocating more space when number of entries exceeds number */
/* of allocated slots. */
/* Uses maxused as a high water mark of the number of slots that have already */
/* been allocated.  Avoids reallocating slots if they already exist (i.e., */
/* recycles slots). */
ENTRY
nextentry()
{
	void *realloc();

	entrycnt++;
	if ( entrycnt > maxused ) {
		if ( (entrycnt > 0) && (entrycnt % EMALLOCSZ == 0) ) {
			newsize *= ((entrycnt / EMALLOCSZ) + 1);
			entryqptr = (ENTRY *)realloc( (char *)entryqptr,
						(unsigned)newsize );
		}
		*(entryqptr + entrycnt) = TLgetentry( tid );
	}

	return( *(entryqptr + entrycnt) );
}

/* print selected lines of table using a temp file (no memory left to do it */
/* in core) */
void
mprttmp( ecnt, pat, hdr )
int ecnt;
char *pat;
char *hdr;
{
	char cmdstr[512];
	char tmpfile[20];
	char *fldval;

	int entryno;
	int i;
	int rc;
	extern int errno;
	pid_t getpid();

	FILE *fp;

	/* generate temp file name */
	sprintf( tmpfile, "/tmp/bkexc%ld", getpid() );

	/* open tmpfile */
	if ( (fp = fopen( tmpfile, "w" )) == NULL ) {
		bkerror( stderr, ERROR6, tmpfile );
		TLclose( tid );
		exit( 2 );
	}

	/* copy number of entries already in core into the temp file */
	for ( i = 0; i < ecnt; i++ ) {
		if ( ( fldval = (char *)TLgetfield( tid, teptr, EX_EXCPATH ))
			!= NULL && (*fldval != NULL) )
			fprintf( fp, "%s\n", fldval );
	}

	/* find remaining entries that match and write them to temp file */
	while ( (entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND ))
		!= TLFAILED ) {

		if ( (entryno == TLBADID) || (entryno == TLARGS) ||
	     	(entryno == TLBADENTRY) || (entryno == TLBADFIELD) ) {
			bkerror( stderr, ERROR4, table, entryno );
			TLclose( tid );
			if ( unlink( tmpfile ) < 0 )
				bkerror( stderr, ERROR8, tmpfile, errno );
			exit( 2 );
		}

		if ( (rc = TLread( tid, entryno, teptr )) != TLOK ) {
			bkerror( stderr, ERROR9, entryno, table, rc );
			TLclose( tid );
			if ( unlink( tmpfile ) < 0 )
				bkerror( stderr, ERROR8, tmpfile, errno );
			exit( 2 );
		}
		/* Ignore comment lines. */
		if ( TLgetfield( tid, teptr, TLCOMMENT ) != NULL )
			continue;

		if ( ( fldval = (char *)TLgetfield( tid, teptr, EX_EXCPATH ))
			!= NULL && (*fldval != NULL) )
			fprintf( fp, "%s\n", fldval );
	}

	fclose( fp );
	fprintf( stdout, hdr, pat );

	sprintf( cmdstr, "sort %s", tmpfile );
	if ( system( cmdstr ) < 0 ) {
		bkerror( stderr, ERROR7, errno );
		TLclose( tid );
		if ( unlink( tmpfile ) < 0 )
			bkerror( stderr, ERROR8, tmpfile, errno );
		exit( 2 );
	}

	if ( unlink( tmpfile ) < 0 ) {
		bkerror( stderr, ERROR8, tmpfile, errno );
		TLclose( tid );
		exit( 2 );
	}
}
/* Synopsis of command invocation. */
void
synopsis()
{
	(void)fprintf( stderr, "%s [-t file] [-d patterns]\n", brcmdname );
	(void)fprintf( stderr, "%s [-t file] -a|-r patterns\n", brcmdname );
	(void)fprintf( stderr, "%s -C [files]\n", brcmdname );
}

/* Open bkexcept table. */
int
tblopen( path, oflag )
{
	int rc;
	int is_new;
	int stat();
	int insert_format();

	char *br_get_except_path();
	struct stat buf;
	struct TLdesc description;

	table = path? (unsigned char *)path: (unsigned char *)br_get_except_path();

	strncpy( (char*)&description, "", sizeof( struct TLdesc ) );
	is_new = (stat( table, &buf ) != 0 );
	if ( is_new ) {
		if (oflag == O_RDONLY) {
			bkerror( stderr, ERROR10, table );
			return(FALSE);
		}
		else
			description.td_format = EX_EXCEPT_F;
	} else description.td_format = EX_EXCEPT_F;


	if (( rc = TLopen( &tid, table, &description, oflag, 0777 )) != TLOK
		&& rc != TLBADFS ) {
		if ( rc == TLDIFFFORMAT )
			bkerror( stderr, ERROR12, table ); /* warning */
		else {
			if ( rc == TLFAILED ) perror( brcmdname );
			else bkerror( stderr, ERROR11, table, rc );
			return ( FALSE );
		}
	}

	if( is_new )
		(void)insert_format( tid, EX_EXCEPT_F );
	return ( TRUE );
}

/* Function returns 0 if objects pointed to by arguments are equal, <0 if arg1 */
/* should appear before arg2 in a sort, >0 if arg2 should appear first. */
compar( e1ptr, e2ptr )
ENTRY *e1ptr;
ENTRY *e2ptr;
{
	char *pat1;
	char *pat2;

	pat1 = (char *)TLgetfield( tid, *e1ptr, EX_EXCPATH );
	pat2 = (char *)TLgetfield( tid, *e2ptr, EX_EXCPATH );
	return( strcmp( pat1, pat2) );

}

/* Function to translate from old style of exception list ('ed' regular */
/* expressions) to new form (cpio expressions). */
void
translate( argv, argc, ind )
char *argv[];
int argc;
int ind;
{
	char cmdstr[512];
	char *file;
	char *script;
	char *deffile = EXCFILE;
	char *bk_get_sedfile();

	int i;
	extern int errno;

	script = bk_get_sedfile();

	if ( ind == argc ) {
		sprintf( cmdstr, "sed -f %s %s", script, deffile );
		if ( system( cmdstr ) < 0 ) {
			bkerror( stderr, ERROR13, deffile, errno );
			exit( 2 );
		}
	}
	else for ( i = ind; i < argc; i++ ) {
		file = argv[i];
		if( strcmp( file, "-" ) == 0 )
			sprintf( cmdstr, "sed -f %s", script );
		else
			sprintf( cmdstr, "sed -f %s %s", script, file );

		if ( system( cmdstr ) < 0 ) {
			bkerror( stderr, ERROR13, file, errno );
			exit( 2 );
		}
	}
}

/* Table search matching function.  This function matches patterns in */
/* anchored mode. */
/* Note that each character matches only itself (there are no metacharacters). */
tsmatch( fldname, fieldval, pattern )
unsigned char *fldname;
unsigned char *fieldval;
unsigned char *pattern;
{
	if ( (fieldval == NULL) || (*fieldval == NULL) )
		return( FALSE );
	if ( strncmp( (char *)fieldval, (char *)pattern,
		strlen( (char *)pattern ) ) == 0 )
		return( TRUE );
	else
		return( FALSE );
}

/* Table search matching function.  This function matches patterns in */
/* unanchored mode, returning TRUE if fieldval contains pattern. */
/* Note that each character matches only itself (there are no metacharacters). */
tscont( fldname, fieldval, pattern )
unsigned char *fldname;
unsigned char *fieldval;
unsigned char *pattern;
{
	if ( (fieldval == NULL) || (*fieldval == NULL) )
		return( FALSE );
	if ( strfind( (char *)fieldval, (char *)pattern ) >= 0 )
		return( TRUE );
	else
		return( FALSE );
}
