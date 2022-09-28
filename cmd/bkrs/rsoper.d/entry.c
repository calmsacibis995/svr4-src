/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/entry.c	1.5.2.1"

#include	<sys/types.h>
#include	<bktypes.h>
#include	<stdio.h>
#include	<table.h>
#include	<restore.h>
#include	<rsstatus.h>
#include	<rsoper.h>
#include	<errors.h>

#ifndef TRUE
#define	TRUE 1
#define FALSE 0
#endif

#define	GET_FIELD( dest, tid, eptr, fname )	\
	dest = TLgetfield( tid, eptr, fname ); \
	if( dest ) { \
		if( *(dest) ) dest = (unsigned char *)strdup( (char *) dest ); \
		else dest = (unsigned char *)0; \
	}

extern char *bkstrdup();
extern void free();
extern long strtol();
extern void *malloc();
extern char *strncpy();
extern char *strdup();
extern int atoi();
extern int strcmp();
extern char *f_to_fs();
extern int sublist();
extern void bkerror();

/* Current counter for looking for prospective candidates in the rsstatus.tab */
static int entryno = 1;

/* Malloc an rs_entry_t structure and initialize it */
rs_entry_t *
en_malloc()
{
	register rs_entry_t *entry;
	if( !(entry = (rs_entry_t *)malloc( sizeof( rs_entry_t ) ) ) )
		return( (rs_entry_t *)0 );
	(void) strncpy( (char *) entry, "", sizeof( rs_entry_t ) );
	return( entry );
}

/* Free an rs_entry_t structure, including structure members */
void
en_free( entry )
rs_entry_t *entry;
{
	if( !entry )	return;
	if( entry->jobid )	free( entry->jobid );
	if( entry->type )	free( entry->type );
	if( entry->object )	free( entry->object );
	if( entry->target )	free( entry->target );
	if( entry->refsname )	free( entry->refsname );
	if( entry->redev )	free( entry->redev );
	if( entry->method )	free( entry->method );
	if( entry->moption )	free( entry->moption );
	if( entry->dgroup )	free( entry->dgroup );
	if( entry->dlabel )	free( entry->dlabel );
	if( entry->tlabel )	free( entry->tlabel );
	if( entry->dchar )	free( entry->dchar );
	if( entry->tmstate )	free( entry->tmstate );
	if( entry->tmoname )	free( entry->tmoname );
	if( entry->tmodev )	free( entry->tmodev );
	if( entry->status )	free( entry->status );
	if( entry->explanation )	free( entry->explanation );
	free( entry );
}

/* Parse an entry in the rsstatus table into a rs_entry_t structure */
rs_entry_t *
en_parse( tid, eptr )
int tid;
ENTRY	eptr;
{
	register rs_entry_t *entry;
	char *buffer;

	if( !(entry = en_malloc()) ) return( (rs_entry_t *)0 );

	GET_FIELD( entry->jobid, tid, eptr, RST_JOBID );
	GET_FIELD( entry->type, tid, eptr, RST_TYPE );
	GET_FIELD( entry->object, tid, eptr, RST_OBJECT );
	GET_FIELD( entry->target, tid, eptr, RST_TARGET );
	GET_FIELD( entry->refsname, tid, eptr, RST_REFSNAME );
	GET_FIELD( entry->redev, tid, eptr, RST_REDEV );
	GET_FIELD( entry->method, tid, eptr, RST_METHOD );
	GET_FIELD( entry->moption, tid, eptr, RST_MOPTION );
	GET_FIELD( entry->dgroup, tid, eptr, RST_DGROUP );
	GET_FIELD( entry->dlabel, tid, eptr, RST_DLABEL );
	GET_FIELD( entry->tlabel, tid, eptr, RST_TLABEL );
	GET_FIELD( entry->dchar, tid, eptr, RST_DCHAR );
	GET_FIELD( entry->tmstate, tid, eptr, RST_TMSTATE );
	GET_FIELD( entry->tmoname, tid, eptr, RST_TMONAME );
	GET_FIELD( entry->tmodev, tid, eptr, RST_TMODEV );
	GET_FIELD( entry->status, tid, eptr, RST_STATUS );
	GET_FIELD( entry->explanation, tid, eptr, RST_EXPLANATION );

	if( (buffer = (char *)TLgetfield( tid, eptr, RST_FDATE ) ) && *buffer )
		entry->fdate = (time_t) strtol( buffer, (char **)0, 16 );

	if( (buffer = (char *)TLgetfield( tid, eptr, RST_MUID ) ) && *buffer )
		entry->muid = strtol( buffer, (char **)0, 10 );
		
	if( (buffer = (char *)TLgetfield( tid, eptr, RST_UID ) ) && *buffer )
		entry->uid = strtol( buffer, (char **)0, 10 );

	if( (buffer = (char *)TLgetfield( tid, eptr, RST_TMDATE ) ) && *buffer )
		entry->tmdate = (time_t) strtol( buffer, (char **)0, 16 );

	if( (buffer = (char *)TLgetfield( tid, eptr, RST_TMSTIMULUS ) ) && *buffer )
		entry->tmstimulus = strtol( buffer, (char **)0, 10 );

	if( (buffer = (char *)TLgetfield( tid, eptr, RST_TMSUCCEEDED ) ) && *buffer )
		entry->tmsucceeded = strtol( buffer, (char **)0, 10 );

	return( entry );
}

/* check to see if this entry is consistent with the info we have */
int
e_consistent( entry, oname, odevice, method, label, istoc )
rs_entry_t *entry;
char *oname, *odevice, *method, *label;
int istoc;
{
	char *type = (char *)entry->type, *elabel;

	/* TYPE */
	if( !strcmp( type, R_PARTITION_TYPE ) || !strcmp( type, R_DISK_TYPE ) ) {
		/* For these object types, compare to ODEVICE. */
		if( odevice && strcmp( (char *) entry->object, odevice ) ) 
			return( FALSE );

	} else if( !strcmp( type, R_FILESYS_TYPE ) ) {
		/* For these object types, compare to ONAME. */
		if( oname && strcmp( (char *) entry->object, oname ) ) 
			return( FALSE );

	} else if( !strcmp( type, R_DIRECTORY_TYPE )
		|| !strcmp( type, R_FILE_TYPE ) ) {
		/*
			For these object types, compare their file system name to ONAME.
		*/
		if( oname && strcmp( f_to_fs( (char *) entry->object ), oname ) ) 
			return( FALSE );
	}

	if( method && *method && entry->method && *(entry->method)
		&& strcmp( method, (char *) entry->method ) ) 
		return( FALSE );

	/* LABEL */
	elabel = (char *)(istoc? entry->tlabel: entry->dlabel);
	if( label && elabel && *elabel && !sublist( label, elabel, " ," )) 
			return( FALSE );

	return( TRUE );
}

/*
	Find the next entry (searching backwards) in the rsstatus.tab that
	might be satisfied by this archive.
*/
rs_entry_t *
en_match( tid, jobs, oname, odev, method, label, is_toc )
int tid, is_toc;
argv_t *jobs;
char *oname, *odev, *method, *label;
{
	ENTRY eptr;
	register rs_entry_t *entry = (rs_entry_t *)0;
	register rc, i;

	/* Get a new entry structure */
	if( !(eptr = TLgetentry( tid )) ) {
		bkerror( stderr, ERROR14 );
		return( (rs_entry_t *)0 );
	}

	while( TRUE ) {

		/* Read the next entry and parse it into our entry structure */
		if( (rc = TLread( tid, entryno, eptr )) != TLOK ) {
			if( rc != TLBADENTRY ) 
				bkerror( stderr, ERROR14 );
			(void) TLfreeentry( tid, eptr );
			return( (rs_entry_t *)0 );
		}
		entryno++;

		if( !(entry = en_parse( tid, eptr )) ) {
			bkerror( stderr, ERROR14 );
			(void) TLfreeentry( tid, eptr );
			return( (rs_entry_t *)0 );
		}

		/* Check to see if this is a comment entry */
		if( !entry->jobid || !*(entry->jobid) || !entry->object || !*(entry->object) ) {
			en_free( entry );
			entry = (rs_entry_t *)0;
			continue;
		}

		/* If a different process is serving this request - skip it */
		if( !strcmp( (char *) entry->status, (char *) RST_ACTIVE ) ) {
			en_free( entry );
			entry = (rs_entry_t *)0;
			continue;
		}

		/* If jobids were given, ONLY allow those jobids */
		if( jobs ) {

			for( i = 0; (*jobs)[i]; i++ ) {
#ifdef TRACE
				brlog("*jobs[%d]=%s",i,(*jobs)[i]);
#endif
				if( !strcmp( (*jobs)[i], (char *) entry->jobid ) ) {
					entry->flags |= RS_NO_PRUNE;
					break;
				}
			}

			if( !((*jobs)[i]) ) {

				/* Not found */
				en_free( entry );
				entry = (rs_entry_t *)0;
				continue;

			} else break;
		}
			
		/* Now check to see if this archive might satisfy this request */
#ifdef TRACE
		brlog("e_con %s %s %s %s",oname,odev,method,label);
#endif
		if( !e_consistent( entry, oname, odev, method, label, is_toc ) ) {
			en_free( entry );
			entry = (rs_entry_t *)0;
		} else {
			/* This entry is a candidate */
			break;
		}
	}

	(void) TLfreeentry( tid, eptr );
	return( entry );
}
