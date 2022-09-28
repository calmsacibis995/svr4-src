/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/auto.c	1.10.2.1"

#include	<sys/types.h>
#include	<time.h>
#include	<stdio.h>
#include	<sys/utsname.h>
#include	<table.h>
#include	<bkrs.h>
#include	<backup.h>
#include	<bkhist.h>
#include	<bktypes.h>
#include	<restore.h>
#include	<rsstatus.h>
#include	<brarc.h>
#include 	"libadmIO.h"

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

#define	GET_UFIELD( dest, tid, eptr, fname ) \
	dest = TLgetfield( tid, eptr, fname ); \
	if( dest ) { \
		if( !*(dest) ) dest = (unsigned char *)0; \
	}


extern int rs_flags;
extern argv_t *s_to_argv();
extern char *strncpy();
extern int atoi();
extern long strtol();
extern int rsspawn();
extern int strcmp();
extern void *malloc();
extern unsigned int strlen();
extern int get_hdr();
extern int strfind();
extern int rstm_crank();

/*  debug  */
extern void brlog();

/* fill a list structure from a rs_entry structure */
static
fill_list( list, tid, entry, rqst )
rs_entry_t *list;
int tid;
ENTRY entry;
rs_rqst_t *rqst;
{
	char *ptr;

	(void) strncpy( (char *) list, "", sizeof( rs_entry_t ) );

	GET_UFIELD( list->type, tid, entry, RST_TYPE );
	if( !list->type )
		return( FALSE );

	GET_UFIELD( list->jobid, tid, entry, RST_JOBID );
	if( !list->jobid )
		return( FALSE );

	GET_UFIELD( list->object, tid, entry, RST_OBJECT );
	if( !list->object )
		return( FALSE );

	GET_UFIELD( list->refsname, tid, entry, RST_REFSNAME );
	GET_UFIELD( list->redev, tid, entry, RST_REDEV );
	GET_UFIELD( list->target, tid, entry, RST_TARGET );
	GET_FIELD( ptr, tid, entry, RST_UID );
	list->uid = strtol( ptr, (char **)0, 10 );

	GET_FIELD( ptr, tid, entry, RST_FDATE );
	list->fdate = strtol( ptr, (char **)0, 16 );

	GET_UFIELD( list->moption, tid, entry, RST_MOPTION );

	list->inode = rqst->inode;

	return( TRUE );
}

/* Spawn rsoper for online archive */
static
do_online( h_tid, h_entry, rs_tid, rs_entry, rqst )
int h_tid, rs_tid;
ENTRY h_entry, rs_entry;
rs_rqst_t *rqst;
{
	char *ddevice, *dmnames, *method, *dchar, *oname, *odevice;
	char buffer[ BKFNAME_SZ ];
	rs_entry_t list;

	GET_FIELD( ddevice, h_tid, h_entry, H_DDEVICE );
	if( !ddevice ) return( FALSE );

	GET_FIELD( dchar, h_tid, h_entry, H_DCHAR );
	if( !dchar ) return( FALSE );

	GET_FIELD( dmnames, h_tid, h_entry, H_DMNAME );

/*  debug  */
#ifdef TRACE
	brlog("do_online: just after call to GET_FIELD for ddevice %s dchar %s dmnames, %s",
	ddevice, dchar, dmnames);
#endif
	GET_FIELD( method, h_tid, h_entry, H_METHOD );
	if( !method ) return( FALSE );

/*  debug  */
#ifdef TRACE
	brlog("do_online: just after call to GET_FIELD for method %s",
	method);
#endif
	GET_FIELD( oname, h_tid, h_entry, H_ONAME );
/*	if( !oname ) return( FALSE );  */

/*  debug  */
#ifdef TRACE
	brlog("do_online: just after call to GET_FIELD for oname %s",
	oname);
#endif
	GET_FIELD( odevice, h_tid, h_entry, H_ODEVICE );
/*	if( !odevice ) return( FALSE );  */
	if ( !oname && !odevice ) return (FALSE);

/*  debug  */
#ifdef TRACE
	brlog("do_online: just after call to GET_FIELD for odevice %s",
	odevice);
#endif
	/* Fill in list structure */
	if( !fill_list( &list, rs_tid, rs_entry, rqst ) )
		return( FALSE );

/*  debug  */
#ifdef TRACE
	brlog("do_online: just after call to fill_list, method %s oname %s odevice %s",
	method,oname,odevice);
#endif
	(void) fprintf( stdout, "Attempting automatic restore from online archive.\n" );


	/* spawn method and wait */
	if( rsspawn( &list, oname, odevice, ddevice, dchar, dmnames, method, rs_flags,
		buffer ) ) {
		return( FALSE );
	}

	return( !strcmp( (char *) list.status, (char *) RST_SUCCESS ) );
}

/*
	Is this archive online? Return 1 iff yes; 0 iff no; and -1 if it is an
	archive of something else.
*/
static
online_archive( h_tid, h_entry )
int h_tid;
ENTRY h_entry;
{
	char *ddevice, *dchar, *dmnames, *ptr;
	archive_info_t ai;
	struct utsname name;
	time_t h_date;

/*  debug  */
#ifdef TRACE
	brlog("online_archive: h_tid %d h_entry %d",
	h_tid, h_entry);
#endif

	if( !(ddevice = (char *)TLgetfield( h_tid, h_entry, H_DDEVICE ) ) 
		|| !*ddevice )
		return( 0 );

	if( !(dchar = (char *)TLgetfield( h_tid, h_entry, H_DCHAR ) ) )
		return( 0 );

/*  its OK to get NULL dmnames */
	dmnames = (char *)TLgetfield( h_tid, h_entry, H_DMNAME );

/*  debug  */
#ifdef TRACE
	brlog("online_archive: ddevice %s dchar %s dmnames %s",
	ddevice, dchar, dmnames);
#endif

	/* Get 1st volume name */
	if( ptr = (char *)malloc( strlen( dmnames ) ) ) {
		char *from, *to;

		from = dmnames;
		to = ptr;

		while( *from && *from != ',' )
			*to++ = *from++;

		*to = '\0';

	}

	if( !get_hdr( ddevice, dchar, ptr, &ai, BR_LABEL_CHECK, (GFILE **)0 ) )
		return( 0 );

	/* Is this the archive we are looking for? */
	if( uname( &name ) && strcmp( name.sysname, ai.br_sysname ) )
		return( -1 );

	if( strcmp( (char *)TLgetfield( h_tid, h_entry, H_METHOD ), ai.br_method ) )
		return( -1 );

	if( strcmp( (char *)TLgetfield( h_tid, h_entry, H_ONAME ), ai.br_fsname ) )
		return( -1 );

	if( strcmp( (char *)TLgetfield( h_tid, h_entry, H_ODEVICE ), ai.br_dev ) )
		return( -1 );

	if( !(ptr = (char *)TLgetfield( h_tid, h_entry, H_DATE ) ) )
		return( -1 );

	h_date = strtol( ptr, (char **)0, 16 );
	if( h_date != ai.br_date )
		return( -1 );

	return( 1 );
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

/* Do automatic restores */
int
rs_do_auto( h_tid, h_entry, rqst, rs_tid, rs_entry )
int h_tid, rs_tid;
ENTRY h_entry, rs_entry;
rs_rqst_t *rqst;
{
	register done = FALSE, rc = FALSE, succeeded, last, have_hentry = FALSE;
	char *mname, *ptr, buffer[ 10 ];

	GET_FIELD( ptr, rs_tid, rs_entry, RST_TMSUCCEEDED );
	succeeded = atoi( ptr );
	last = 0;

	while( !done ) {

		/* what's the next move? */
		switch( rstm_crank( h_tid, h_entry, rqst, last, succeeded ) ) {

		case RS_TARCHIVE:
			/* Table of Contents, off-line, tell operator to get it */
			GET_FIELD( mname, h_tid, h_entry, H_TMNAME );

			if( mname ) {

				(void) TLassign( rs_tid, rs_entry, RST_TLABEL, mname );
				have_hentry = TRUE;
				done = TRUE;
				break;
			}
			/* No media names - treat as DARCHIVE */
			/*NOBREAK*/

		case RS_DARCHIVE:

			switch( online_archive( h_tid, h_entry ) ) {
			case 1:

/*  debug  */
#ifdef TRACE
	brlog("rs_do_auto: case RS_DARCHIVE, is online ");
#endif
				/* Try this online archive */
				if( last = do_online( h_tid, h_entry, rs_tid, rs_entry, rqst ) )
					succeeded++;
				break;

			case 0:
				/* Archive is not online */
				GET_FIELD( mname, h_tid, h_entry, H_DMNAME );

				if( mname ) {

					/* Record info for operator */
					(void) TLassign( rs_tid, rs_entry, RST_DLABEL, mname );

/*  debug  */
#ifdef TRACE
	brlog("rsoper: auto.c: rs_do_auto: TLassign: mname %s",
	mname);
#endif

					have_hentry = TRUE;
					done = TRUE;
				}
				break;

			default:
				/* It is an archive of something else - look for next archive */
				last = 0;
				break;
			}
			break;

		case RS_COMPLETE:
			rc = 1;
			/*NOBREAK*/

		default:
			done = TRUE;
			break;
		}
	}

	if( !rc ) {
		/* Insure that entry is in table */

		if( have_hentry ) {
			if( ptr = rsgetdtype( h_tid, h_entry ) )
				(void) TLassign( rs_tid, rs_entry, RST_DGROUP, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_DDEVICE ) )
				&& *ptr )
				TLassign( rs_tid, rs_entry, RST_DDEVICE, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_METHOD ) )
				&& *ptr )
				TLassign( rs_tid, rs_entry, RST_METHOD, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_OPTIONS ) )
				&& *ptr )
				TLassign( rs_tid, rs_entry, RST_MOPTION, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_DATE ) )
				&& *ptr )
				(void) TLassign( rs_tid, rs_entry, RST_ARCHDATE, ptr );

			if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_DCHAR ) )
				&& *ptr )
				(void) TLassign( rs_tid, rs_entry, RST_DCHAR, ptr );
	
			(void) sprintf( buffer, "0x%x", (int) rqst->tmdate );
			(void) TLassign( rs_tid, rs_entry, RST_TMDATE, buffer );
			(void) TLassign( rs_tid, rs_entry, RST_TMSTATE, rqst->tmstate );

			(void) sprintf( buffer, "%d", rqst->tmstimulus );
			(void) TLassign( rs_tid, rs_entry, RST_TMSTATE, rqst->tmstate );

			if( rqst->oname )
				(void) TLassign( rs_tid, rs_entry, RST_TMONAME, rqst->oname );

			if( rqst->odev )
				(void) TLassign( rs_tid, rs_entry, RST_TMODEV, rqst->odev );

			(void) TLassign( rs_tid, rs_entry, RST_TMSTIMULUS, buffer );
			(void) TLassign( rs_tid, rs_entry, RST_STATUS, RST_PENDING );
		}

		(void) sprintf( buffer, "%d", succeeded );
		(void) TLassign( rs_tid, rs_entry, RST_TMSUCCEEDED, buffer );
		(void) TLappend( rs_tid, TLEND, rs_entry );
		(void) TLsync( rs_tid );

	}
	return( rc );
}
