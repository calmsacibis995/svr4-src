/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/brfindtab.c	1.1.2.1"

#include <stdio.h>
#include <string.h>

#define TRUE	1
#define FALSE	0
#define NARGS	2

/* define number of tables this program handles and define the name */
/* for each of the tables. */
#define NTABS	5
#define METHOD	"method"
#define BSTATAB	"bkstatus"
#define EXTAB	"bkexcept"
#define HISTTAB	"bkhistory"
#define REGTAB	"bkreg"

/* Need to call method path routine with a method name - any standard one */
/* will do. */
#define DMETHOD	"ffile"

/* name of this command */
char *brcmdname;

/* table to hole table names valid for this command */
char *tabnames[NTABS];

/* external function declarations */
void exit();

char *bk_get_bkreg_path();
char *bk_get_histlog_path();
char *bk_get_method_path();
char *bk_get_statlog_path();
char *br_get_except_path();

/* Program takes a backup/restore table name (e.g., bkreg, bkstatus, etc.) */
/* and prints the path to the table on stdout.  The argument may also be */
/* the keyword "method", in which case the path to the method directory */
/* is printed. */
main (argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	int c;
	int i;
	int getopt();

	char *inpname;
	char *path;
	char *lastslash;

	void synopsis();

	brcmdname = argv[0];

	if ( argc != NARGS ) {
		synopsis();
		exit( 1 );
	}


	inpname = argv[optind];

	tabnames[0] = METHOD;
	tabnames[1] = BSTATAB;
	tabnames[2] = EXTAB;
	tabnames[3] = HISTTAB;
	tabnames[4] = REGTAB;

	for ( i = 0; i < NTABS; i++ )
		if ( strcmp( inpname, tabnames[i] ) == 0 )
			break;
	if ( i == NTABS ) {
		synopsis();
		exit( 1 );
	}

	switch( i ) {
		/* Get path to method directory.  Path returned is to method, */
		/* so must strip off the file name part. */
		case 0:	path = bk_get_method_path( DMETHOD );
			lastslash = strrchr( path, '/' );
			*lastslash = (char)NULL;
			break;

		case 1: path = bk_get_statlog_path();
			break;

		case 2: path = br_get_except_path();
			break;

		case 3: path = bk_get_histlog_path();
			break;

		case 4: path = bk_get_bkreg_path();
			break;

		default: fprintf( stderr, "%s: Illegal method number %d.\n", 
			brcmdname, i );
			exit( 1 );
	}

	fprintf( stdout, "%s\n", path );

	exit ( 0 );
}

void
synopsis()
{
	int i;

	fprintf(stderr, "Usage: %s table_name\n", brcmdname );
	fprintf(stderr, "	valid table names are:\n");
	for (i = 0; i < NTABS; i++)
		fprintf(stderr, "\t\t%s\n", tabnames[i]);
}
