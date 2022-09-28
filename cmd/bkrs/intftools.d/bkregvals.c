/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/bkregvals.c	1.4.2.1"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <table.h>
#include <bkreg.h>
#include <bkregerrs.h>

#define TRUE	1
#define FALSE	0
#define FLEN	20
#define NFLDS	14
#define NARGS	4

/* name of this command */
char *brcmdname;

/* field names for table */
unsigned char *fldname[] = {
	(unsigned char *)"tag",
	(unsigned char *)"oname",
	(unsigned char *)"odevice",
	(unsigned char *)"olabel",
	(unsigned char *)"week",
	(unsigned char *)"day",
	(unsigned char *)"method",
	(unsigned char *)"options",
	(unsigned char *)"priority",
	(unsigned char *)"dgroup",
	(unsigned char *)"ddevice",
	(unsigned char *)"dchar",
	(unsigned char *)"dmname",
	(unsigned char *)"depend"
};

/* table name */
unsigned char *table;

/* table id */
int tid;

/* table search criteria array */
struct TLsearch TLsearches[TL_MAXFIELDS];

/* pointer to an entry structure */
ENTRY eptr;

/* entry number for tag line in table */
int entryno;

/* tag to locate in table */
char *tag;

void exit();
void bkerror();

/* Program takes the name of a bkreg table and a tag for an entry in the table. */
/* It finds the tagged line in the table and writes the field values from that */
/* entry into a temporary file.  It writes the name of the temp file to stdout. */
/* The program using the temp file must delete it when it has finished. */
main (argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	int c;
	int i;
	int getopt();
	int tblopen();

	void findtag();
	void synopsis();

	brcmdname = argv[0];

	if ( argc != NARGS ) {
		synopsis();
		exit( 1 );
	}

	while (( c = getopt( argc, argv, "t:?" )) != -1 )
		switch ( c ) {
		case 't':
			tag = optarg;
			break;

		case '?':
			synopsis();
			exit ( 0 );
			/* NOTREACHED */
			break;

		default:
			bkerror(stderr,ERROR0, c);
			break;
	}

	table = (unsigned char *)argv[optind++];

	if ( !tblopen() )
		exit ( 2 );

	findtag();

	for( i = 0; i < NFLDS; i++ )
		fprintf(stdout,"%s%c", TLgetfield( tid, eptr, fldname[i] ),
		(i == NFLDS-1)?'\n':':' );

	TLclose( tid );

	exit ( 0 );
}

void
synopsis()
{
	fprintf(stdout, "Usage: %s -t tag table\n", brcmdname );
}

/* Locate the entry with the requested tag.  Not finding the tag is an error. */
void
findtag()
{
	struct TLsearch *tlsp = TLsearches;

	tlsp->ts_fieldname = R_TAG;
	tlsp->ts_pattern = (unsigned char *)tag;
	tlsp->ts_operation = (int (*)())TLEQ;
	tlsp++;
	tlsp->ts_fieldname = (unsigned char *)NULL;

	if ( ((entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND))
		== TLFAILED) || (entryno == TLBADENTRY) || (entryno == TLBADID)
		   || (entryno == TLARGS) || (entryno == TLBADFIELD) ) {
			bkerror( stderr, ERROR1, tag, table, entryno );
			TLclose( tid );
			exit ( 2 );
	}

	if ( !(eptr = TLgetentry( tid )) ) {
		bkerror( stderr, ERROR2 );
		TLclose( tid );
		exit ( 2 );
	}
	if ( TLread( tid, entryno, eptr ) != TLOK ) {
		bkerror( stderr, ERROR3, entryno );
		TLclose( tid );
		exit ( 2 );
	}

}

/* Open table. */
int
tblopen()
{
	int rc;

	if (( rc = TLopen( &tid, table, (struct TLdesc *)NULL, O_RDONLY )) != TLOK
		&& rc != TLBADFS ) {
			if ( rc == TLFAILED ) perror( brcmdname );
			else bkerror( stderr, ERROR5, table, rc );
			return ( FALSE );
	}
	return ( TRUE );
}
