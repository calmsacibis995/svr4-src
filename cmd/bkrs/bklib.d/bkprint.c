/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkprint.c	1.5.2.1"

#include <stdio.h>
#include <table.h>

#define min( a, b ) ((a < b)? a: b)

#define TRUE	1
#define FALSE	0

#define NL	'\n'
#define MAXDFLDS	25

extern unsigned int strlen();

/* Print entire value, do not wrap fields to a specified field width, */
/* separate each pair of values by the field separator character. */
void
prt_nowrap( nflds, prvalues, fld_sep )
int nflds;
char *prvalues[];
char fld_sep;
{
	int i;

	for( i = 0; i < nflds; i++)
		(void) fprintf( stdout, "%s%c",
			prvalues[i], (i == nflds - 1) ? NL : fld_sep );
}
/* Print field information, wrapping long entries to succeeding lines, */
/* aligning them to field boundaries. */
void
prt_wrap( nflds, prvalues, prlens )
int nflds;
unsigned char *prvalues[];
int prlens[];
{
	int done;
	int flen;
	int i;
	unsigned char *next_char[MAXDFLDS];

	for( i = 0; i < nflds; i++ ) {
		next_char[i] = prvalues[i];
	}

	done = FALSE;
	while( !done ) {
		done = TRUE;
		for( i = 0; i < nflds; i++ ) {
			flen = prlens[i];
			if( i == 0 )
				(void) fprintf( stdout, "%-*.*s", flen, flen,
					next_char[i] );
			else
				(void) fprintf( stdout, " %-*.*s", flen, flen,
					next_char[i] );
			if( *next_char[i] != NULL ) {
				next_char[i] += min( strlen( (char *) next_char[i] ), flen);
				if( *next_char[i] != NULL )
					done = FALSE;
			}
		}
		(void) fprintf( stdout, "\n" );
	}
}
