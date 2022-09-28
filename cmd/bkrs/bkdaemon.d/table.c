/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/table.c	1.5.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<table.h>
#include	<backup.h>
#include	<bkreg.h>

extern void brlog();
extern int bkget_now();

/* find the ROTATION STARTED date from the bkreg table */
int
bk_rotate_start( tid, period, curr_week, curr_day )
int tid, period, *curr_week, *curr_day;
{
	TLsearch_t TLsearches[ 2 ], *sptr = TLsearches;
	ENTRY entry;
	unsigned char *ptr;
	register entryno, rc = TRUE;
	time_t now;
	struct tm *tmptr;

	sptr->ts_fieldname = TLCOMMENT;
	sptr->ts_pattern = (unsigned char *)R_ROTATE_START_MSG;
	sptr->ts_operation = (int (*)())TLMATCH;
	sptr++;
	sptr->ts_fieldname = (unsigned char *)0;
	entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND );
	if( entryno == TLFAILED ) {
		/*
			If no ROTATION STARTED, then default to week 1, day - whatever
			today is.
		*/
		now = time( 0 );
		tmptr = localtime( &now );
		*curr_week = 1;
		*curr_day = tmptr->tm_wday;
		return( TRUE );
	}

	/* Get an entry element */
	if( !(entry = TLgetentry( tid ) ) ) {
		brlog( "bk_rotate_start(): TLgetentry() fails" );
		return( FALSE );
	}

	if( TLread( tid, entryno, entry ) != TLOK ) {
		brlog( "bk_rotate_start(): unable to read ROTATE START entry" );
		TLfreeentry( tid, entry );
		return( FALSE );
	}

	ptr = TLgetfield( tid, entry, TLCOMMENT );
	if( !ptr ) {
		brlog( "bk_rotate_start(): unable to get COMMENT field" );
		TLfreeentry( tid, entry );
		return( FALSE );
	}
	
	/* Seek to '=' in message */
	while( *ptr && *ptr != '=' ) ptr++;
	if( !(*ptr) ) {
		brlog( "bk_rotate_start(): ROTATION STARTED field has bad format" );
		TLfreeentry( tid, entry );
		return( FALSE );
	}
	ptr++;

	if( !bkget_now( ptr, period, curr_week, curr_day ) ) {
		brlog( "bk_rotate_start(): week and day fields have bad format" );
		rc = FALSE;
	}
	/* Free up memory */
	TLfreeentry( tid, entry );

	return( rc );
}
