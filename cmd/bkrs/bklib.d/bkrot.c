/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkrot.c	1.7.2.1"

#include <table.h>
#include <stdio.h>
#include <string.h>
#include <bkerrors.h>

#define TRUE	1
#define FALSE	0

#define R_ROTATE_MSG	"ROTATION="
#define R_ROTATE_START_MSG	"ROTATION STARTED="
#define E_FORMAT_MSG	"ENTRY FORMAT="

extern int bkget_now();
extern unsigned char *p_integer();
extern char *malloc();
extern void free();

struct TLsearch TLsearches[TL_MAXFIELDS];

/* Get current rotation period from table. */
/* NOTE: if rotation comment does not appear in file, 1 is returned. */
/* This is also returned if there is no file. */
int
get_period(tid, curr_period)
int tid;
int *curr_period;
{
	struct TLsearch *sptr = TLsearches;
	ENTRY entry;
	unsigned char *ptr, *fptr;
	unsigned char *p_integer();

	register entryno;

	sptr->ts_fieldname = TLCOMMENT;
	sptr->ts_pattern = (unsigned char *)R_ROTATE_MSG;
	sptr->ts_operation = (int (*)())TLMATCH;
	sptr++;
	sptr->ts_fieldname = (unsigned char *)0;
	entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND );
	if( (entryno == TLFAILED) || (entryno == TLBADENTRY) ) {
		*curr_period = 1;
		return( 0 );
	}

	/* Get an entry element */
	if( !(entry = TLgetentry( tid ) ) ) {
		return( BKNOMEMORY );
	}

	if( TLread( tid, entryno, entry ) != TLOK ) {
		TLfreeentry( tid, entry );
		return( BKBADREAD );
	}

	ptr = fptr = TLgetfield( tid, entry, TLCOMMENT );
	if( !ptr ) {
		TLfreeentry( tid, entry );
		return( BKBADFIELD );
	}
	
	/* Seek to '=' in message */
	while( *ptr && *ptr != '=' ) ptr++;
	if( !(*ptr) ) {
		free( fptr );
		TLfreeentry( tid, entry );
		return( BKBADFIELD );
	}
	ptr++;

	if( !p_integer( ptr, curr_period ) ) {
		free( fptr );
		TLfreeentry( tid, entry );
		return( BKBADVALUE );
	}
	/* Free up memory */
	free( fptr );
	TLfreeentry( tid, entry );

	return( 0 );
}

int
get_rotate_start(tid, curr_period, curr_week, curr_day)
int tid;
int curr_period;
int *curr_week;
int *curr_day;
{
	struct TLsearch *sptr = TLsearches;
	ENTRY entry;

	unsigned char *ptr, *fptr;

	register entryno, rc = 0;

	sptr->ts_fieldname = TLCOMMENT;
	sptr->ts_pattern = (unsigned char *)R_ROTATE_START_MSG;
	sptr->ts_operation = (int (*)())TLMATCH;
	sptr++;
	sptr->ts_fieldname = (unsigned char *)0;
	entryno = TLsearch1( tid, TLsearches, TLBEGIN, TLEND, TL_AND );
	if( entryno == TLFAILED ) {
		return( BKNORSMSG );
	}

	/* Get an entry element */
	if( !(entry = TLgetentry( tid ) ) ) {
		return( BKNOMEMORY );
	}

	if( TLread( tid, entryno, entry ) != TLOK ) {
		TLfreeentry( tid, entry );
		return( BKBADREAD );
	}

	ptr = fptr = TLgetfield( tid, entry, TLCOMMENT );
	if( !ptr ) {
		TLfreeentry( tid, entry );
		return( BKBADFIELD );
	}
	
	/* Seek to '=' in message */
	while( *ptr && *ptr != '=' ) ptr++;
	if( !(*ptr) ) {
		TLfreeentry( tid, entry );
		free( fptr );
		return( BKBADFIELD );
	}
	ptr++;

	if( !bkget_now( ptr, curr_period, curr_week, curr_day ) ) {
		rc = BKBADVALUE;
	}
	/* Free up memory */
	free( fptr );
	TLfreeentry( tid, entry );

	return( rc );
}

/* Insert a new ROTATION comment into a table or modify an existing one. */
/* Note: this comment must occur before the first non-commentary line */
/* in the file but after the ENTRY FORMAT comment. */
int
insert_rotation( tid, period )
int tid;
int period;
{
	char *buffer;

	ENTRY eptr;
	ENTRY teptr;
	TLsearch_t sarray[ 2 ];

	int ientryno;
	int rc;
	
	/* See if the ROTATION is already in the file */
	sarray[ 0 ].ts_fieldname = TLCOMMENT;
	sarray[ 0 ].ts_pattern = (unsigned char *)R_ROTATE_MSG;
	sarray[ 0 ].ts_operation = (int (*)())TLMATCH;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	if( !(buffer = malloc( sizeof( R_ROTATE_MSG ) + 20 ) ) )
		return( BKNOMEMORY );

	if( !(eptr = TLgetentry( tid )) ) {
		free( buffer );
		return( BKNOMEMORY );
	}
	if( !(teptr = TLgetentry( tid )) ) {
		(void) TLfreeentry( tid, eptr );
		free( buffer );
		return( BKNOMEMORY );
	}

	(void)sprintf( buffer, R_ROTATE_MSG );
	(void)sprintf( buffer + strlen( R_ROTATE_MSG ), "%d", period );
	if ( TLassign( tid, eptr, TLCOMMENT, buffer ) != TLOK )
		return( BKBADASSIGN );

	/* If ROTATION comment is in file, overwrite it with new value */
	/* else if comment is not in file, */
	/*      insert it before the first non-commentary entry in the file. */
	/* If the file is empty, the ROTATION comment is written as the first */
	/* entry in the file. (This should not normally occur, as the calling */
	/* process should put an ENTRY FORMAT comment in the file. */

	if ( (ientryno = TLsearch1( tid, sarray, TLBEGIN, TLEND, TL_AND ))
		!= TLFAILED ) {
		if ( ientryno == TLBADENTRY ) {
			if( TLappend( tid, TLBEGIN, eptr ) != TLOK )
				return( BKBADWRITE );
		}
		else if ( (ientryno == TLBADID) || (ientryno == TLARGS)
	     	      || (ientryno == TLBADFIELD) )
				return( BKBADSEARCH );
		else
			if ( TLwrite( tid, ientryno, eptr ) != TLOK )
				return( BKBADWRITE );
	}
	else {
		ientryno = 1;
		while ( ((rc = TLread( tid, ientryno, teptr)) == TLOK) &&
			(TLgetfield( tid, teptr, TLCOMMENT ) != NULL) )
				ientryno++;
		if ( (rc != TLOK) && (rc != TLBADENTRY) )
				return( BKBADREAD );
		else if( TLappend( tid, ientryno - 1, eptr ) != TLOK )
				return( BKBADWRITE );
	}

	(void) TLsync( tid );

	(void) TLfreeentry( tid, eptr );
	(void) TLfreeentry( tid, teptr );
	free( buffer );

	return( 0 );
}

int
insert_rot_start( tid, cweek )
int tid;
int cweek;
{
	char *buffer;
	unsigned char rot_start[7];
	unsigned char *bkget_rotatestart();

	ENTRY eptr;
	ENTRY teptr;
	TLsearch_t sarray[ 2 ];

	int rc;
	int rsentryno;
	
	/* Set up new entry */
	/* rot_start will contain the date of the Sunday of week 1 of the */
	/* rotation period in the form YYMMDD */
	(void)strncpy( (char *)rot_start, (char *)bkget_rotatestart( cweek - 1 ), 6 );
	rot_start[6] = NULL;

	if( !(buffer = (char *)malloc( sizeof( R_ROTATE_START_MSG )
		+ sizeof( rot_start ) ) ) ) return( BKNOMEMORY );

	if( !(eptr = TLgetentry( tid )) ) {
		free( buffer );
		return( BKNOMEMORY );
	}
	if( !(teptr = TLgetentry( tid )) ) {
		(void) TLfreeentry( tid, eptr );
		free( buffer );
		return( BKNOMEMORY );
	}

	(void)sprintf( buffer, R_ROTATE_START_MSG );
	(void)sprintf( buffer + strlen( R_ROTATE_START_MSG ), "%s", rot_start );

	if( TLassign( tid, eptr, TLCOMMENT, buffer ) != TLOK )
		return( BKBADASSIGN );

	/* See if the ROTATION STARTED is already in the file */
	sarray[ 0 ].ts_fieldname = TLCOMMENT;
	sarray[ 0 ].ts_pattern = (unsigned char *)R_ROTATE_START_MSG;
	sarray[ 0 ].ts_operation = (int (*)())TLMATCH;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	/* If ROTATION STARTED comment is not in the file, insert it after the */
	/* ROTATION comment. */
	if( ( rsentryno = TLsearch1( tid, sarray, TLBEGIN, TLEND, TL_AND ) ) == TLFAILED ) {
		/* Find the ROTATION comment in the file */
		sarray[ 0 ].ts_fieldname = TLCOMMENT;
		sarray[ 0 ].ts_pattern = (unsigned char *)R_ROTATE_MSG;
		sarray[ 0 ].ts_operation = (int (*)())TLMATCH;
		sarray[ 1 ].ts_fieldname = (unsigned char *)0;

		if( ( rsentryno = TLsearch1( tid, sarray, TLBEGIN, TLEND, TL_AND ) )
			!= TLFAILED ) {

			if ( (rsentryno == TLBADID) || (rsentryno == TLARGS) ||
		  	(rsentryno == TLBADENTRY) || (rsentryno == TLBADFIELD) )
				return( BKBADSEARCH );
			else
				if ( TLappend( tid, rsentryno, eptr ) != TLOK )
					return( BKBADWRITE );
		}
		else {
			rsentryno = 1;
			while ( ((rc = TLread( tid, rsentryno, teptr )) == TLOK) &&
				(TLgetfield( tid, teptr, TLCOMMENT) != NULL) )
					rsentryno++;
			if( (rc != TLOK) && (rc != TLBADENTRY) )
				return( BKBADREAD );
			else if ( TLappend( tid, rsentryno-1, eptr ) != TLOK )
				return( BKBADWRITE );

		}
	}
	else if ( TLwrite( tid, rsentryno, eptr ) != TLOK )
			return( BKBADWRITE );

	(void) TLsync( tid );

	(void) TLfreeentry( tid, eptr );
	(void) TLfreeentry( tid, teptr );
	free( buffer );

	return( 0 );
}

