/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/format.c	1.4.2.1"

#include	<table.h>
#include	<bkreg.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define	NULL 0
#endif

extern void *malloc();
extern void free();
extern int sprintf();
extern unsigned int strlen();

/* Insert an ENTRY FORMAT entry into a table */
void
insert_format( tid, format )
int tid;
unsigned char *format;
{
	char *buffer;
	ENTRY eptr;
	TLsearch_t sarray[ 2 ];
	
	/* See if the ENTRY FORMAT is already in the file */
	sarray[ 0 ].ts_fieldname = TLCOMMENT;
	sarray[ 0 ].ts_pattern = (unsigned char *)"ENTRY FORMAT=";
	sarray[ 0 ].ts_operation = (int (*)())TLMATCH;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	if( TLsearch1( tid, sarray, TLEND, TLBEGIN, TL_AND ) > 0 )
		/* There already is one */
		return;

	if( !(buffer = (char *)malloc( strlen( "ENTRY FORMAT=" )
		+ strlen( (char *) format ) + 1 ) ) ) return;

	if( !(eptr = TLgetentry( tid )) ) {
		free( buffer );
		return;
	}

	(void) sprintf( buffer, "ENTRY FORMAT=%s", format );
	(void) TLassign( tid, eptr, TLCOMMENT, buffer );

	(void) TLappend( tid, TLBEGIN, eptr );

	(void) TLsync( tid );

	(void) TLfreeentry( tid, eptr );
	free( buffer );
}
