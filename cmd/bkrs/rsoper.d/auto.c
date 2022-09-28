/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/auto.c	1.4.2.1"

#include	<fcntl.h>
#include	<time.h>
#include	<errno.h>
#include	<table.h>
#include	<restore.h>
#include	<rsstatus.h>
#include	<bkhist.h>
#include	<brtoc.h>

/* This file contains routines pertaining to the Restore Strategy */

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

/* Get a field from an entry */
#define	GET_FIELD( dest, tid, eptr, fname )	\
	dest = (char *)TLgetfield( tid, eptr, fname ); \
	if( dest ) { \
		if( !*(dest) ) dest = (char *)0; \
	}

static int h_tid = 0;
static int rs_do_auto();

extern char *strncpy();
extern int strcmp();
extern void free();
extern long strtol();
extern int strfind();
extern void *malloc();
extern void brlog();
extern char *brerrno();
extern int atoi();
extern int rstm_crank();
extern int sprintf();
extern char *strdup();
extern char *bk_get_histlog_path();

/* Fill a rqst structure from a rs_entry_t structure */
static void
fill_rqst( rqst, entry )
rs_rqst_t *rqst;
rs_entry_t *entry;
{
	(void) strncpy( (char *) rqst, "", sizeof( rs_rqst_t ) );
	rqst->jobid = (char *)entry->jobid;
	rqst->object = (char *)entry->object;
	rqst->oname = (char *)entry->tmoname;
	rqst->odev = (char *)entry->tmodev;
	rqst->date = entry->fdate;
	rqst->type = (char *)entry->type;
	rqst->tmstate = (char *)entry->tmstate;
	rqst->tmdate = entry->tmdate;
	rqst->tmstimulus = entry->tmstimulus;
}

/* Is this object in this archive? return media names */
char *
in_archive( rs_tid, rs_entry, toc )
int rs_tid;
ENTRY rs_entry;
char *toc;
{
	static t_tid = 0;
	static char *t_name = 0;
	int rc, entryno;
	char *ptr;
	static ENTRY entry;
	TLdesc_t descr;
	TLsearch_t sarray[ 2 ];
	time_t t_date, rs_date;

	if( !toc || !*toc )
		return( (char *)0 );

	if( !t_tid || strcmp( t_name, toc ) ) {

		/* First open the file */
		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = H_ENTRY_F;

		if( (rc = TLopen( &t_tid, toc, &descr, O_RDONLY, 0644 ) ) != TLOK
			&& rc != TLBADFS && rc != TLDIFFFORMAT ) 
			return( (char *) 0 );

		if( t_name ) free( t_name );
		t_name = strdup( toc );
	}

	sarray[ 0 ].ts_fieldname = TOC_FNAME;
	sarray[ 0 ].ts_pattern = (unsigned char *)TLgetfield( rs_tid, rs_entry, RST_OBJECT );
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	entryno = TLsearch1( t_tid, sarray, TLBEGIN, TLEND, TL_AND );

	if( entryno == TLFAILED )
		return( (char *) 0 );

	if( entryno < 0 )
		return( "" );

	/* Get an entry element */
	if( !(entry = TLgetentry( t_tid )) )
		return( "" );

	if( TLread( t_tid, entryno, entry ) != TLOK ) {
		(void) TLfreeentry( t_tid, entry );
		return( "" );
	}

	/* Check modification time and change time */
	rs_date = strtol( (char *) TLgetfield( rs_tid, rs_entry, RST_FDATE ), (char **)0, 16 );

	t_date = strtol( (char *) TLgetfield( t_tid, entry, TOC_CTIME ), (char **)0, 16 );
	if( rs_date > t_date ) {
		ptr = strdup( (char *)TLgetfield( t_tid, entry, TOC_VOL ) );
		(void) TLfreeentry( t_tid, entry );
		return( ptr );
	}

	t_date = strtol( (char *) TLgetfield( t_tid, entry, TOC_MTIME ), (char **)0, 16 );
	if( rs_date > t_date ) {
		ptr = strdup( (char *)TLgetfield( t_tid, entry, TOC_VOL ) );
		(void) TLfreeentry( t_tid, entry );
		return( ptr );
	}

/* XXX - inode?

	if( (ptr = (char *)TLgetfield( t_tid, entry, TOC_INODE )) && *ptr )
		rqst->inode = strtol( ptr, (char **)0, 16 );
	else rqst->inode = 0;
*/

	return( (char *) 0 );
}

/* return dtype from H_DCHAR field of history entry */
static
char *
rsgetdtype( h_tid, h_entry )
int h_tid;
ENTRY h_entry;
{
	register i, j;
	char *dchar, *ptr;

	dchar = (char *)TLgetfield( h_tid, h_entry, H_DCHAR );

	i = strfind( dchar, "type=" );
	if( i < 0 ) 
		return( (char *) 0 );

	else dchar += 5;

	j = strfind( dchar, "," );

	if( ptr = (char *)malloc( (unsigned int) j + 1 ) ) {
		(void) strncpy( ptr, dchar, (unsigned int) j );
		ptr[ j ] = '\0';
	}

	return( ptr );
}

/* Crank the strategy algorithm and return whether or not it is done */
int
rs_crank( rs_tid, rs_entry, entry )
int rs_tid;
ENTRY	rs_entry;
rs_entry_t	*entry;
{
	int h_tid, rc;
	ENTRY h_entry;
	TLdesc_t descr;
	rs_rqst_t rqst;
	char *path = bk_get_histlog_path();
	
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = H_ENTRY_F;

	if( h_tid != -1 && (rc = TLopen( &h_tid, path, &descr, O_RDONLY, 0644 ) )
		!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
		if( rc == TLFAILED ) 
			brlog( "TLopen of history table %s fails: %s",
				path, brerrno( errno ) );
		else brlog( "TLopen of history table %s returns %d",
			path, rc );
		h_tid = -1;
		return( 0 );

	} 

	/* Get an entry element */
	if( h_tid != -1 && !(h_entry = TLgetentry( h_tid )) ) {
		brlog( "unable to get memory for history table entry\n" );
		TLclose( h_tid );
		h_tid = -1;
		return( 0 );
	}

	fill_rqst( &rqst, entry );

	rc = rs_do_auto( h_tid, h_entry, &rqst, rs_tid, rs_entry );

#ifdef TRACE
	brlog("restore.d:auto.c:rs_crank: return from rs_do_auto %d",
	rc);
#endif
	(void) TLfreeentry( h_tid, h_entry );

	return( rc );
}

static int
rs_do_auto( h_tid, h_entry, rqst, rs_tid, rs_entry )
int h_tid, rs_tid;
ENTRY h_entry, rs_entry;
rs_rqst_t *rqst;
{
	register rc = FALSE, succeeded, last, have_hentry = FALSE;
	char *mname, *ptr, buffer[ 10 ];

	GET_FIELD( ptr, rs_tid, rs_entry, RST_TMSUCCEEDED );
	succeeded = atoi( ptr );
	last = 0;
#ifdef TRACE
	brlog("rs_do_auto: rs_entry RST_TMSUCCEEDED %s rs_entry %s",
	ptr, (char *) rs_entry);
#endif

	GET_FIELD( ptr, rs_tid, rs_entry, RST_STATUS );
	last = !strcmp( ptr, (char *) RST_SUCCESS );

	/* what's the next move? */
#ifdef TRACE
	brlog("rs_do_auto: call to rstm_crank, last %d succeeded %d",
	last,succeeded);
#endif
	switch( rstm_crank( h_tid, h_entry, rqst, last, succeeded ) ) {

	case RS_TARCHIVE:
		/* Table of Contents, off-line, tell operator to get it */
		GET_FIELD( mname, h_tid, h_entry, H_TMNAME );

		if( mname ) {

			(void) TLassign( rs_tid, rs_entry, RST_TLABEL, mname );
			have_hentry = TRUE;
			break;
		}
		/* No media names - treat as DARCHIVE */
		/*NOBREAK*/

	case RS_DARCHIVE:

		/* Archive is not online */
		GET_FIELD( mname, h_tid, h_entry, H_DMNAME );

		if( mname ) {

			/* Record info for operator */
			(void) TLassign( rs_tid, rs_entry, RST_DLABEL, mname );
			have_hentry = TRUE;
		}
#ifdef TRACE
	brlog("rsoper:auto.c:rs_do_auto:return from rstm_crank is RS_DARCHIVE %d mname %s",
	RS_DARCHIVE,mname);
#endif
		break;

	case RS_COMPLETE:
#ifdef TRACE
	brlog("rsoper:auto.c:rs_do_auto:return from rstm_crank is RS_COMPLETE");
#endif
		rc = 1;
		break;

	default:
#ifdef TRACE
	brlog("rsoper:auto.c:rs_do_auto:return from rstm_crank is default");
#endif
/*  treat default return as complete */
/*  this handles the case where a single archive is in history */
		rc = 1;
		break;
	}

	if( !rc ) {

		/* Update entry */
		if( have_hentry ) {
			if( ptr = rsgetdtype( h_tid, h_entry ) )
				(void) TLassign( rs_tid, rs_entry, RST_DGROUP, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_DDEVICE ) )
				&& *ptr )
				(void) TLassign( rs_tid, rs_entry, RST_DDEVICE, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_METHOD ) )
				&& *ptr )
				(void) TLassign( rs_tid, rs_entry, RST_METHOD, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_OPTIONS ) )
				&& *ptr )
				(void) TLassign( rs_tid, rs_entry, RST_MOPTION, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_DATE ) )
				&& *ptr )
				(void) TLassign( rs_tid, rs_entry, RST_ARCHDATE, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_DCHAR ) )
				&& *ptr )
				(void) TLassign( rs_tid, rs_entry, RST_DCHAR, ptr );

			(void) sprintf( buffer, "0x%lx", (int) rqst->tmdate );
			(void) TLassign( rs_tid, rs_entry, RST_TMDATE, buffer );
			(void) TLassign( rs_tid, rs_entry, RST_TMSTATE, rqst->tmstate );

			(void) sprintf( buffer, "%d", rqst->tmstimulus );
			(void) TLassign( rs_tid, rs_entry, RST_TMSTIMULUS, buffer );
			(void) TLassign( rs_tid, rs_entry, RST_STATUS, RST_PENDING );
			(void) TLassign( rs_tid, rs_entry, RST_EXPLANATION, "" );

			(void) sprintf( buffer, "%d", succeeded + last );
			(void) TLassign( rs_tid, rs_entry, RST_TMSUCCEEDED, buffer );

		} else {
			/* No more strategy */

			(void) TLassign( rs_tid, rs_entry, RST_DGROUP, "" );
			(void) TLassign( rs_tid, rs_entry, RST_DDEVICE, "" );
			(void) TLassign( rs_tid, rs_entry, RST_METHOD, "" );
			(void) TLassign( rs_tid, rs_entry, RST_MOPTION, "" );
			(void) TLassign( rs_tid, rs_entry, RST_ARCHDATE, "" );
			(void) TLassign( rs_tid, rs_entry, RST_DCHAR, "" );
			(void) TLassign( rs_tid, rs_entry, RST_DLABEL, "" );
			(void) TLassign( rs_tid, rs_entry, RST_TMDATE, "" );
			(void) TLassign( rs_tid, rs_entry, RST_TMSTATE, "" );
			(void) TLassign( rs_tid, rs_entry, RST_TMONAME, "" );
			(void) TLassign( rs_tid, rs_entry, RST_TMODEV, "" );
			(void) TLassign( rs_tid, rs_entry, RST_TMSTIMULUS, "" );

		}

	}
	return( rc );
}
