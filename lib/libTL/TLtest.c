/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLtest.c	1.4.3.1"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <table.h>
#include <internal.h>

#define BSIZE 512
#define	MAXIDS	10
#define	NENTRIES	10
#define	SKIPWS(p)	if( p ) while( *p == ' ' || *p == '\t' ) p++
#define	SEEKWS(p)	if( p ) while( *p && *p != ' ' && *p != '\t' && *p != '\n' )\
						 p++

struct errmsg {
	int value;
	char *name;
} errtable[] = {
	TLSUBSTITUTION, "TLSUBSTITUTION",
	TLBADFIELD, "TLBADFIELD",
	TLNOMEMORY, "TLNOMEMORY",
	TLBADENTRY, "TLBADENTRY",
	TLBADFORMAT, "TLBADFORMAT",
	TLBADID, "TLBADID",
	TLTOOMANY, "TLTOOMANY",
	TLDESCRIPTION, "TLDESCRIPTION",
	TLTOOLONG, "TLTOOLONG",
	TLARGS, "TLARGS",
	TLFAILED, "TLFAILED",
	TLOK, "TLOK",
	TLBADFS, "TLBADFS",
	TLDIFFFORMAT, "TLDIFFFORMAT"
};
int errsize = (sizeof( errtable ) / sizeof( struct errmsg ) );

char buffer[ BSIZE ], *myfgets(), *ptr;
FILE *sfptr = 0;
ENTRY entries[ NENTRIES ];
int echo = 0;
TLsearch_t sarray[NENTRIES];
unsigned char fieldnames[NENTRIES][80];
unsigned char patterns[NENTRIES][80];
char *errmap();

main( argc, argv )
char *argv[];
{
	register rc;
	register entryno_t entryno, first, last;
	int c, mode, tid, entry, how_to_match;
	TLdesc_t	desc;
	unsigned char *filename, *fieldname, *value, *pattern, *replacement;
	extern char *optarg;
	extern int optind;

	strncpy( entries, "", NENTRIES * sizeof( ENTRY ) );

	while( (c = getopt( argc, argv, "s:e") ) != -1 )
		switch( c ) {
		case 'e':	/* echo input to output */
			echo = 1;
			break;
		case 's':	/* Tee output into optarg */
			if( !(sfptr = fopen( optarg, "w" ) ) )
				fprintf( stderr, "%s: cannot open %s for writing, errno %d\n",
					argv[0], optarg, errno );
			break;
		default:
			fprintf( stderr, "synopsis: %s [-s shadowfile ]\n", argv[0] );
			break;
		}

	strncpy( &desc, "", sizeof( TLdesc_t ) );
	
	fprintf( stdout, "> " );
	while( myfgets( buffer, BSIZE, stdin ) == buffer ) {
		ptr = buffer;
		SKIPWS(ptr);
		switch( *ptr ) {
		case 'a':
			ptr++;
			if( *ptr == 'f' ) {	/* ASSIGN FIELD */
				ptr++;
				SKIPWS(ptr);
				tid = getinteger();
				SKIPWS(ptr);
				entry = getentry( tid );
				SKIPWS(ptr);
				fieldname = (unsigned char *)ptr;
				SEEKWS(ptr);
				*ptr = '\0';
				ptr++;
				SKIPWS(ptr);
				value = (unsigned char *)ptr;
				while( *ptr && *ptr != '\n' ) ptr++;
				*ptr = '\0';
				rc = TLassign( tid, entries[ entry ], fieldname, value );
				fprintf( stdout, "--TLassign returns %s\n", errmap( rc ) );
				break;
			} 
			/* APPEND ENTRY */
			SKIPWS(ptr);
			tid = getinteger();
			entryno = getentryno();
			entry = getentry( tid );
			rc = TLappend( tid, entryno, entries[ entry ] );
			if( rc == TLFAILED )
				fprintf( stdout, "--TLappend returns TLFAILED errno %d\n",
					errno );
			else fprintf( stdout, "--TLappend returns %s\n", errmap( rc ) );
			break;

		case 'c':
			/* CLOSE */
			ptr++; SKIPWS(ptr);
			tid = getinteger();
			rc = TLclose( tid );
			fprintf( stdout, "--close: %s\n", errmap( rc ) );
			break;

		case 'd':	/* DELETE ENTRY */
			ptr++;
			SKIPWS(ptr);
			tid = getinteger();
			entryno = getentryno();
			rc = TLdelete( tid, entryno );
			if( rc == TLFAILED )
				fprintf( stdout, "--TLdelete returns TLFAILED, errno %d\n",
					errno );
			else fprintf( stdout, "--TLdelete returns %s\n", errmap( rc ) );
			break;

		case 'D':	/* DESCRIPTION */
			if( !getcresp( "comment char: ", &(desc.td_comment) ) ) break;
			if( !getcresp( "field sep char: ", &(desc.td_fs) ) ) break;
			if( !getcresp( "end of entry char: ", &(desc.td_eoe) ) ) break;
			break;

		case 'f':	/* FREE AN ENTRY */
			ptr++;
			SKIPWS(ptr);
			tid = getinteger();
			entry = getentry(tid);
			if( entry == -1 ) break;
			TLfreeentry( tid, entries[ entry ] );
			strncpy( entries[ entry ], "", sizeof( ENTRY ) );
			break;
	
		case 'F':	/* FORMAT */
			ptr++;
			SKIPWS(ptr);
			if( desc.td_format ) Free( desc.td_format );
			if( !(desc.td_format = (unsigned char *)Malloc( Strlen( ptr ) ) ))
				fprintf( stdout, "--malloc fails\n" );
			else strncpy( desc.td_format, ptr, Strlen(ptr) - 1 );
			break;

		case 'g':
			/* GETFIELD and GETENTRY */
			ptr++;
			switch( *ptr ) {
			case 'e':
				ptr++;
				SKIPWS(ptr);
				tid = getinteger();
				if( (entry = getentry( tid ) ) == -1 ) break;
				if( entries[ entry ] ) TLfreeentry( tid, entries[ entry ] );
				entries[ entry ] = TLgetentry( tid );
				fprintf( stdout, "--TLgetentry() returns %s\n",
					(entries[ entry ])? "SUCCESS": "NULL" );
				break;
				
			case 'f':
				ptr++;
				SKIPWS(ptr);
				tid = getinteger();
				if( (entry = getentry( tid ) ) == -1 ) break;
				SKIPWS(ptr);
				fieldname = (unsigned char *)ptr;
				SEEKWS(ptr);
				*ptr++ = '\0';
				if( !Strcmp( (char *)fieldname, "comment" ) )
					fieldname = TLCOMMENT;
				else if( !Strcmp( (char *)fieldname, "trailing" ) )
					fieldname = TLTRAILING;
				ptr = (char *)TLgetfield( tid, entries[ entry ], fieldname );
				fprintf( stdout, "--TLgetfield() returns >%s<\n",
					( ptr? ptr: "NULL" ) );
				break;
			default:
				goto error;
			}
			break;

		case 'o':
			/* OPEN */
			ptr++; SKIPWS(ptr);
			/* Get filename */
			filename = (unsigned char *)ptr;
			SEEKWS(ptr);
			*ptr++ = '\0';
			SKIPWS(ptr);
			mode = getoctal();
			rc = TLopen( &tid, filename, &desc,
				(mode? (O_RDWR|O_CREAT): O_RDWR), mode );
			if( rc == TLFAILED )
				fprintf( stdout,
					"--TLopen of %s returns TLFAILED, errno %d, tid %d\n",
					filename, errno, tid );
			else fprintf( stdout, "--TLopen of %s returns %s, tid %d\n",
				filename, errmap( rc ), tid );
			break;

		case 'q':	/* QUIT */
#ifdef MONITOR
			monitor( (int (*)())0, 0, 0, 0, 0 );
#endif
			exit( 0 );
			break;

		case 'r':	/* READ */
			ptr++;
			SKIPWS(ptr);
			tid = getinteger();
			entryno = getentryno();
			if( (entry = getentry(tid)) == -1 ) break;
			rc = TLread( tid, entryno, entries[ entry ] );
			if( rc == TLFAILED )
				fprintf( stdout, "--TLread returns TLFAILED, errno %d\n",
					errno );
			else fprintf( stdout, "--TLread returns %s\n", errmap( rc ) );
			if( rc == TLOK )
				pr_entry( entries[ entry ] );
			break;

		case 's':	/* SEARCH, SUBST, SYNC */
			ptr++;
			switch( *ptr ) {
			case 'e':
				ptr++;
				SKIPWS(ptr);
				tid = getinteger();
				SKIPWS(ptr);
				first = getentryno();
				last = getentryno();
				getsearches( sarray );
				how_to_match = gethow_to_match();
				rc = TLsearch1( tid, sarray, first, last, how_to_match );
				if( rc < 0 )
					fprintf( stdout, "--TLsearch1() returns %s\n",
						errmap( rc ) );
				else fprintf( stdout, "--TLsearch1() returns %d\n", rc );
				break;

			case 'u':
				ptr++;
				SKIPWS(ptr);
				tid = getinteger();
				SKIPWS(ptr);
				entry = getentry( tid );
				SKIPWS(ptr);
				fieldname = (unsigned char *)ptr;
				SEEKWS(ptr);
				*ptr = '\0';
				ptr++;
				SKIPWS(ptr);
				pattern = (unsigned char *)ptr;
				SEEKWS(ptr);
				*ptr = '\0';
				ptr++;
				replacement = (unsigned char *)ptr;
				while( *ptr != '\n' ) ptr++;
				*ptr = '\0';
				rc = TLsubst( tid, entries[entry], fieldname, pattern,
					replacement );
				fprintf( stdout, "--TLsubst() returns %s\n", errmap( rc ) );
				break;

			case 'y':
				ptr++;
				SKIPWS(ptr);
				tid = getinteger();
				rc = TLsync( tid );
				if( rc == TLFAILED )
					fprintf( stdout, "--TLsync returns TLFAILED, errno %d\n",
						errno );
				else fprintf( stdout, "--TLsync returns %s\n", errmap( rc ) );
				break;
			}
			break;

		case 'w':	/* WRITE ENTRY */
			ptr++;
			SKIPWS(ptr);
			tid = getinteger();
			entryno = getentryno();
			if( (entry = getentry(tid)) == -1 ) break;
			rc = TLwrite( tid, entryno, entries[ entry ] );
			if( rc == TLFAILED )
				fprintf( stdout, "--TLwrite returns TLFAILED, errno %d\n",
					errno );
			else fprintf( stdout, "--TLwrite returns %s\n", errmap( rc ) );
			break;

		case '!':	/* SYSTEM */
			ptr++;
			system( ptr );
			break;

		case '#':	/* Comment */
			break;

		case '?':
		default:
error:
			fprintf( stdout, "Commands:\n" );
			fprintf( stdout,
				"\ta <tid> <entryno> <entry> - append an entry\n" );
			fprintf( stdout,
			"\taf <tid> <entry> <fieldname> <value> - assign field value\n" );
			fprintf( stdout, "\tc <tid> - close\n" );
			fprintf( stdout, "\td <tid> <entryno> - delete an entry\n" );
			fprintf( stdout, "\tD - fill in description structure\n" );
			fprintf( stdout, "\tf <tid> <entry> - free an entry\n" );
			fprintf( stdout, "\tF <format> - format\n" );
			fprintf( stdout, "\tge <tid> <entry> - get an entry\n" );
			fprintf( stdout,
				"\tgf <tid> <entry> <fieldname> - get a fieldvalue\n" );
			fprintf( stdout, "\to <filename> <mode> - open a table\n" );
			fprintf( stdout, "\tq - quit\n" );
			fprintf( stdout,
				"\tr <tid> <entryno> <entry> - read an entry\n" );
			fprintf( stdout,
				"\tse <tid> <first_entryno> <last_entryno> - search for an entry\n" );
			fprintf( stdout,
				"\tsu <tid> <entry> <fieldname> <pattern> <replace> - substitute in a field\n" );
			fprintf( stdout, "\tsy <tid> - sync\n" );
			fprintf( stdout, "\tw <tid> <entryno> <entry> - write an entry\n" );
			fprintf( stdout,
				"\tentryno: B(begin)<number> E(end)<number> <number>\n");
			fprintf( stdout,
				"\t\t where B<n> = begin + <n> and E<n> = end - <n>\n" );
			fprintf( stdout, "\tentry: <number> in array of ENTRYs\n" );
		}
		fprintf( stdout, "> " );
	}
#ifdef MONITOR
			monitor( (int (*))0, 0, 0, 0, 0 );
#endif
	exit( 0 );
}

getoctal()
{
	register i, n, c = 0;
	for( i = 0; i < 3; i++, ptr++ )
		if( *ptr && isdigit( (int)*ptr) && (n = *ptr - '0') < 8 )
			c = c*8 + n;
		else break;
	return( c );
}

char
getcharacter()
{
	if( !ptr || !*ptr ) return( '\0' );
	if( *ptr == '\\' ) {
		if( *(++ptr) == '\\' ) return( '\\' );
		return( (char)getoctal( ptr ) );
	} else return( *ptr );
}

getcresp( prompt, ptr )
unsigned char *prompt, *ptr;
{
	unsigned char *tmp;
	fprintf( stdout, "%s", prompt );
	if( myfgets( buffer, BSIZE, stdin ) == buffer ) {
		tmp = (unsigned char *)buffer;
		SKIPWS(tmp);
		return( *ptr = getcharacter( tmp ) );
	} else return( 0 );
}

getentryno()
{
	SKIPWS(ptr);
	switch( *ptr ) {
	case 'B':
	case 'b':
		ptr++;
		return( TLBEGIN + getinteger() );
	case 'E':
	case 'e':
		ptr++;
		return( TLEND - getinteger() );
	default:
		return( getinteger() );
	}
}

getinteger()
{
	int n = 0;
	while( isdigit(*ptr) ) {
		n = n * 10 + *ptr - '0'; 
		ptr++;
	}
	return( n );
}

getentry( tid )
int tid;
{
	int n;
	SKIPWS(ptr);
	n = getinteger();
	if( n > 0 && n <= NENTRIES ) 
		return( n - 1 );
	return( -1 );
}

pr_entry( entry )
entry_t *entry;
{
	fprintf( stdout, "ENTRY:\n\tstatus 0x%x\n\tseekaddr %d\n",
		entry->status, entry->seekaddr );
	pr_fields( &(entry->fields), "\t" );
	if( entry->comment )
		fprintf( stdout, "\tComment: %s\n", entry->comment );
}

pr_fields( fields, prolog )
field_t *fields;
char *prolog;
{
	register i;
	for( i = 0; i < fields->count; i++ )
		fprintf( stdout, "%s%d: %s\n", prolog, i + 1, fields->values[ i ] );
}

char *
myfgets( buffer, size, fptr )
char *buffer;
int size;
FILE *fptr;
{
	register char *tmp;
	if( (tmp = fgets( buffer, size, fptr )) && sfptr )
		fprintf( sfptr, "%s", buffer );
	if( tmp && echo )
		fprintf( stdout, "%s", buffer );
	return( tmp );
}

int
i_decide( fieldname, fieldvalue, pattern )
unsigned char *fieldname, *fieldvalue, *pattern;
{
	unsigned char *ptr;
	fprintf( stdout, "Fieldname: %s\n", fieldname );
	fprintf( stdout, "Fieldvalue: %s\n", fieldvalue );
	fprintf( stdout, "Pattern: %s\n", pattern );
	fprintf( stdout, "Y or N: " );
	if( myfgets( buffer, BSIZE, stdin ) == buffer ) {
		ptr = (unsigned char *)buffer;
		SKIPWS(ptr);
		return( *ptr == 'y' || *ptr == 'Y' );
	}
	return( FALSE );
}
	
getsearches( sarray )
TLsearch_t *sarray;
{
	int	i;
	unsigned char *ptr;
	for( i = 0; i < NENTRIES; i++ ) {
		fprintf( stdout, "Fieldname: " );
		if( (unsigned char *)myfgets( fieldnames[i], 80, stdin )
			!= fieldnames[i] ) {
			sarray[i].ts_fieldname = NULL;
			return;
		} 
		if( !(ptr = (unsigned char *)strchr( fieldnames[i], '\n' )) ) {
			sarray[i].ts_fieldname = NULL;
			return;
		} 
		*ptr = '\0';
		if( !Strlen( fieldnames[i] ) ) {
			sarray[i].ts_fieldname = NULL;
			return;
		} 
		sarray[i].ts_fieldname = fieldnames[i];
		fprintf( stdout, "Pattern: " );
		if( (unsigned char *)myfgets( patterns[i], 80, stdin )
			!= patterns[i] ) {
			sarray[i].ts_fieldname = NULL;
			return;
		} else if( !(ptr = (unsigned char *)strchr( patterns[i], '\n' )) ) {
			sarray[i].ts_fieldname = NULL;
			return;
		} 
		*ptr = '\0';
		if( !Strlen( patterns[i] ) ) {
			sarray[i].ts_fieldname = NULL;
			return;
		} 
		sarray[i].ts_pattern = patterns[i];
		if( !getoperation( sarray + i ) ) {
			sarray[i].ts_fieldname = NULL;
			return;
		}
	} 
}

gethow_to_match( )
{
	unsigned char *ptr;
	fprintf( stdout,
		"HOW_TO_MATCH: (TL_AND (default), TL_NAND, TL_OR, or TLNOR) ? " );
	if( myfgets( buffer, BSIZE, stdin ) != buffer ) {
		return( TL_AND );
	} else {
		buffer[ Strlen(buffer) - 1 ] = '\0';
		ptr = (unsigned char *)buffer;
		SKIPWS(ptr);
		if( !Strcmp( ptr, "TL_NAND" ) )
			return( TL_NAND );
		else if( !Strcmp( ptr, "TL_OR" ) )
			return( TL_OR );
		else if( !Strcmp( ptr, "TL_NOR" ) )
			return( TL_NOR );
		else return( TL_AND );
	}
}

getoperation( sptr )
TLsearch_t *sptr;
{
	int i_decide(), addr = 0;
	unsigned char *ptr;
	fprintf( stdout, "Choose match operation:\n" );
	fprintf( stdout,
		"\tTLEQ, TLNE, TLLT, TLGT, TLLE, TLGE, TLMATCH, TLNMATCH, TLNUMEQ\n" );
	fprintf( stdout,
		"\tTLNUMNE, TLNUMLT, TLNUMGT, TLNUMLE, TLNUMGE, i_decide (default)\n" );
	fprintf( stdout, "operation: " );
	if( myfgets( buffer, BSIZE, stdin ) != buffer ) return( FALSE );
	buffer[ Strlen(buffer) - 1] = '\0';
	ptr = (unsigned char *)buffer;
	SKIPWS(ptr);
	if( !Strcmp( ptr, "TLEQ" ) ) addr = TLEQ;
	else if( !Strcmp( ptr, "TLNE" ) ) addr = TLNE;
	else if( !Strcmp( ptr, "TLLT" ) ) addr = TLLT;
	else if( !Strcmp( ptr, "TLGT" ) ) addr = TLGT;
	else if( !Strcmp( ptr, "TLLE" ) ) addr = TLLE;
	else if( !Strcmp( ptr, "TLGE" ) ) addr = TLGE;
	else if( !Strcmp( ptr, "TLMATCH" ) ) addr = TLMATCH;
	else if( !Strcmp( ptr, "TLNMATCH" ) ) addr = TLNMATCH;
	else if( !Strcmp( ptr, "TLNUMEQ" ) ) addr = TLNUMEQ;
	else if( !Strcmp( ptr, "TLNUMNE" ) ) addr = TLNUMNE;
	else if( !Strcmp( ptr, "TLNUMLT" ) ) addr = TLNUMLT;
	else if( !Strcmp( ptr, "TLNUMGT" ) ) addr = TLNUMGT;
	else if( !Strcmp( ptr, "TLNUMLE" ) ) addr = TLNUMLE;
	else if( !Strcmp( ptr, "TLNUMGE" ) ) addr = TLNUMGE;
	if( addr )
		sptr->ts_operation = (int (*)())addr;
	else sptr->ts_operation = i_decide;
	return( TRUE );
}

char *
errmap( errcode )
int errcode;
{
	register i;
	static char errbuf[40];
	for( i = 0; i < errsize; i++ )
		if( errtable[ i ].value == errcode )
			return( errtable[ i ].name );

	sprintf( errbuf, "%d", errcode );
	return( errbuf );
}
