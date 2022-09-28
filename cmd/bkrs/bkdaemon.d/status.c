/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/status.c	1.1.3.1"

#include	<fcntl.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<table.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkstatus.h>
#include	<errno.h>

#define	ST_DEFAULT_PERIOD	1

static int stat_tid = 0;
static int st_startno;

extern	method_t	*methodtab;
extern	int	methodtabsz;
extern	owner_t	*ownertab;
extern	int	ownertabsz;
extern	proc_t	*proctab;
extern	int	proctabsz;
extern	int bklevels;

extern char *bk_get_statlog_path();
extern int sprintf();
extern time_t time();
extern int strcmp();
extern void brlog();
extern void insert_format();
extern int get_period();

static void st_truncate();
static int fill_newentry();
static int fill_oldentry();

int
st_start()
{
	register rc;
	TLdesc_t descr;
	char *path, buffer[ 15 ];
	ENTRY eptr;
	TLsearch_t sarray[ 2 ];

	if( !stat_tid ) {
		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = (unsigned char *)ST_ENTRY_F;
		path = (char *)bk_get_statlog_path();

		if( (rc = TLopen( &stat_tid, path, &descr, O_RDWR|O_CREAT, 0644 ) )
			!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
			if( rc == TLFAILED ) 
				brlog( "st_start(): TLopen of status table %s fails: errno %d",
					path, errno );
			else brlog( "st_start(): TLopen of status table %s returns %d",
				path, rc );
			return( 0 );
		}
		insert_format( stat_tid, ST_ENTRY_F );

		(void) TLsync( stat_tid );

		st_truncate();
	}

	/* Get an entry element for the new status info */
	if( !(eptr = TLgetentry( stat_tid )) ) {
		brlog( "st_start(): unable to initialize status table" );
		return( 0 );
	}

	(void) TLassign( stat_tid, eptr, ST_JOBID, ST_START );

	(void) sprintf( buffer, "0x%x", (int) time(0) );
	(void) TLassign( stat_tid, eptr, ST_STARTTIME, buffer );

	(void) TLappend( stat_tid, TLEND, eptr );

	/* Now find out the entryno of the entry just written */
	sarray[ 0 ].ts_fieldname = ST_JOBID;
	sarray[ 0 ].ts_pattern = (unsigned char *)"START";
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)NULL;

	st_startno = TLsearch1( stat_tid, sarray, TLEND, TLBEGIN, TL_AND );

	/* Can't?? happen */
	if( st_startno < 0 ) st_startno = 1;

	TLfreeentry( stat_tid, eptr );
	(void) TLsync( stat_tid );

	return( 1 );
}

void
st_stop()
{
	register rc;
	char buffer[ 15 ];
	ENTRY eptr;

	/* Get an entry element for the new status info */
	if( !(eptr = TLgetentry( stat_tid )) ) {
		brlog( "st_stop(): unable to write STOP entry" );
	} else {

		(void) TLassign( stat_tid, eptr, ST_JOBID, ST_STOP );

		(void) sprintf( buffer, "0x%x", (int) time(0) );
		if( (rc = TLassign( stat_tid, eptr, ST_STARTTIME, buffer ))
			!= TLOK ) 
			brlog( "st_stop(): TLassign() starttime returns %d", rc );

		if( (rc = TLappend( stat_tid, TLEND, eptr )) != TLOK )
			brlog( "st_stop(): TLappend() returns %d", rc );

		if( (rc = TLfreeentry( stat_tid, eptr )) != TLOK )
			brlog( "st_stop(): TLfreeentry() returns %d", rc );

		if( (rc = TLsync( stat_tid )) != TLOK )
			brlog( "st_stop(): TLsync() returns %d", rc );

		if( (rc = TLclose( stat_tid )) != TLOK )
			brlog( "st_stop(): TLsync() returns %d", rc );

	}
}

/* Convert MD_ status to ST_ status */
static
unsigned char *
st_md_to_st( status )
int status;
{
	switch( status ) {
	case MD_ACTIVE: return( ST_ACTIVE );
	case MD_PENDING: return( ST_PENDING );
	case MD_WAITING: return( ST_WAITING );
	case MD_HALTED: return( ST_HALTED );
	case MD_FAILED: return( ST_FAILED );
	case MD_SUCCESS: return( ST_SUCCESS );
	default: return( (unsigned char *) "UNKNOWN" );
	}
}

/* Set the current status for a particular backup */
void
st_set( m_slot, status, explanation )
int m_slot, status;
unsigned char *explanation;
{
	char jobid[ BKJOBID_SZ ];
	register found = FALSE, entryno;
	register method_t *method;
	register owner_t *owner;
	register proc_t	*proc;
	TLsearch_t sarray[ 3 ];
	ENTRY eptr;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "st_set(): given bad m_slot %d", m_slot );
		return;
	}
	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "st_set(): m_slot %d has bad o_slot %d", m_slot, 
			method->o_slot );
		return;
	}
	if( !(proc = P_SLOT( owner->p_slot ) ) ) {
		brlog( "st_set(): o_slot %d has a bad p_slot %d", method->o_slot,
			owner->p_slot );
		return;
	}

	/*
		Record status in method structure - this is mainly necessary to
		implement the HALTED state. That is, to do a RESUME, it must be
		known which state was HALTED from.
	*/
	if( status == MD_HALTED ) {

		/* Don't SUSPEND methods that are DONE */
		if( method->state & MD_DONE) return;

		method->ostatus = method->status;
	}
	method->status = status;

	(void) sprintf( jobid, "back-%ld", proc->pid );
	sarray[ 0 ].ts_fieldname = ST_JOBID;
	sarray[ 0 ].ts_pattern = (unsigned char *)jobid;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = ST_TAG;
	sarray[ 1 ].ts_pattern = (unsigned char *)method->entry.tag;
	sarray[ 1 ].ts_operation = (int (*)())TLEQ;
	sarray[ 2 ].ts_fieldname = (unsigned char *)NULL;

	BEGIN_CRITICAL_REGION;

	/* Search for this method in the status table */
	found = ((entryno = TLsearch1( stat_tid, sarray,
		st_startno, TLEND, TL_AND )) > 0 );

	/* Get an entry element for the new status info */
	if( !(eptr = TLgetentry( stat_tid )) ) {
		brlog( "st_set(): unable to get entry structure from TLgetentry()" );
		END_CRITICAL_REGION;
		return;
	}

	if( found ) {
		if( TLread( stat_tid, entryno, eptr ) == TLOK 
			&& fill_oldentry( method, eptr, st_md_to_st( status ), explanation ) )
			(void) TLwrite( stat_tid, entryno, eptr );
	} else {
		if( fill_newentry( owner, method, eptr, (unsigned char *)jobid,
			method->entry.tag, st_md_to_st( status ), explanation ) )
			(void) TLappend( stat_tid, TLEND, eptr );
	}
	TLfreeentry( stat_tid, eptr );
	(void) TLsync( stat_tid );
	END_CRITICAL_REGION;
}

static int
fill_newentry( owner, method, eptr, jobid, tag, status, explanation )
owner_t *owner;
method_t *method;
ENTRY	eptr;
unsigned char *jobid, *tag, *status, *explanation;
{
	char buffer[ 20 ];
	register proc_t *proc;

	if( !(proc = P_SLOT( owner->p_slot )) ) {
		brlog( "fill_newentry(): owner has bad p_slot %d", owner->p_slot );
		return( FALSE );
	}

	(void) TLassign( stat_tid, eptr, ST_JOBID, jobid );

	(void) TLassign( stat_tid, eptr, ST_TAG, tag );

	(void) sprintf( buffer, "%d", proc->uid );
	(void) TLassign( stat_tid, eptr, ST_UID, buffer );

	(void) sprintf( buffer, "0x%x", (int) method->starttime );
	(void) TLassign( stat_tid, eptr, ST_STARTTIME, buffer );

	(void) TLassign( stat_tid, eptr, ST_ONAME, method->entry.oname );
	(void) TLassign( stat_tid, eptr, ST_ODEVICE, method->entry.odevice );

	return( fill_oldentry( method, eptr, status, explanation ) );
}

static int
fill_oldentry( method, eptr, status, explanation )
method_t *method;
ENTRY	eptr;
unsigned char *status, *explanation;
{
	char buffer[ 20 ];
	if( method->entry.dgroup ) 
		(void) TLassign( stat_tid, eptr, ST_DGROUP, method->entry.dgroup );

	if( method->entry.ddevice )
		(void) TLassign( stat_tid, eptr, ST_DDEVICE, method->entry.ddevice );

	if( method->dchar )
		(void) TLassign( stat_tid, eptr, ST_DCHAR, method->dchar );

	(void) TLassign( stat_tid, eptr, ST_STATUS, status );
	(void) TLassign( stat_tid, eptr, ST_EXPLANATION, explanation );

	(void) sprintf( buffer, "0x%x", (int) method->starttime );
	(void) TLassign( stat_tid, eptr, ST_STARTTIME, buffer );

	return( TRUE );
}

/* Set a method's status to FAILED */
void
st_setfail( m_slot, rc, explanation )
int m_slot, rc;
char *explanation;
{
	char buffer[ 20 ];

	/* Use the explanation if it is given */
	if( explanation ) 
		st_set( m_slot, MD_FAILED, (unsigned char *)explanation );
	else {
		/* Otherwise use the error code as a message id for text */
		/* XXX - for right now, just print the error code */	

		(void) sprintf( buffer, "%d", rc );
		st_set( m_slot, MD_FAILED, (unsigned char *)buffer );
	}
}

/* Re-set the status of the method because it is being resumed */
void
st_resume( m_slot )
int m_slot;
{
	register method_t *method;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "st_resume(): given bad m_slot %d", m_slot );
		return;
	}

	/* Don't resume if this method is already finished */
	if( method->state & MD_DONE ) return;

	st_set( m_slot, method->ostatus, (unsigned char *)0 );
}

/* Truncate the status table */
static void
st_truncate()
{
	register st_entryno;
	int period;
	time_t cutoff, now;
	ENTRY st_entry;
	char cutoff_buf[ 20 ], *ptr;

	if( get_period( stat_tid, &period ) )
		period = ST_DEFAULT_PERIOD;

	if( !(st_entry = TLgetentry( stat_tid ) ) )
		return;
	
	/* Calculate cutoff for truncation */
	
	now = time( 0 );
	if( period * 7 * 24 * 60 * 60 > now )
		return;

	cutoff = now - period * 7 * 24 * 60 * 60;
	(void) sprintf( cutoff_buf, "0x%x", (int) cutoff );

	st_entryno = 1;

	while( TRUE ) {
		if( TLread( stat_tid, st_entryno, st_entry ) != TLOK )
			break;

		if( (ptr = (char *)TLgetfield( stat_tid, st_entry, ST_STARTTIME ) )
			&& *ptr
			&& strcmp( ptr, cutoff_buf ) <= 0 ) {

			(void) TLdelete( stat_tid, st_entryno );

		} else st_entryno++;

	}
		
	(void) TLsync( stat_tid );
	(void) TLfreeentry( stat_tid, st_entry );
}
