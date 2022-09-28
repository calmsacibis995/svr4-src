/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/history.c	1.9.2.1"

#include	<sys/types.h>
#include	<string.h>
#include	<fcntl.h>
#include	<table.h>
#include	<backup.h>
#include	<bkdaemon.h>
#include	<bkmsgs.h>
#include	<bkhist.h>
#include	<bktypes.h>
#include	<errno.h>

#define	HST_DEFAULT_PERIOD	52
#define	SECS_PER_WEEK	(7*24*60*60)

extern	method_t	*methodtab;
extern	int	methodtabsz;
extern	argv_t *s_to_argv();
extern	char *argv_to_s();
extern	void argv_free();
extern int sprintf();
extern void *malloc();
extern void free();
extern time_t time();
extern int unlink();

extern void brlog();
extern char *bk_get_histlog_path();
extern char *brerrno();
extern void insert_format();
extern int lbl_insert();
extern int get_period();

void hst_invalidate();
void hst_truncate();
static int h_tid = 0;

int
hst_update( m_slot, msg )
int m_slot;
history_m *msg;
{
	register method_t *method;
	char *path, buffer[ 20 ];
	ENTRY eptr;
	TLsearch_t sarray[ 4 ];
	register entryno, rc, i;
	argv_t *dlbls;

#ifdef TRACE
	brlog(
		"hst_update: m_slot %d oname %s odevice %s flags 0x%x time 0x%x nvols %d size %d labels %s",
		m_slot, msg->oname, msg->odevice, msg->flags, msg->time,
		msg->nvolumes, msg->size, msg->labels );
#endif

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "hst_update(): given bad m_slot %d", m_slot );
		return( 0 );
	}

	if( !h_tid ) {
		TLdesc_t descr;

		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = (unsigned char *)H_ENTRY_F;
		path = (char *)bk_get_histlog_path();

		if( (rc = TLopen( &h_tid, path, &descr, O_RDWR|O_CREAT, 0644 ) ) != TLOK 
			&& rc != TLBADFS && rc != TLDIFFFORMAT ) {
			if( rc == TLFAILED ) 
				brlog( "hst_update(): TLopen of history table %s fails: %s",
					path, brerrno( errno )  );
			else brlog( "hst_update(): TLopen of history table %s returns %d",
				path, rc );
			return( 0 );
		}
		insert_format( h_tid, H_ENTRY_F );
	}

	/* Get an entry element for the new status info */
	if( !(eptr = TLgetentry( h_tid )) ) {
		brlog( "hst_update(): unable to initialize status table" );
		return( 0 );
	}

	(void) sprintf( buffer, "0x%x",
			(method->state & MD_IS_TOC)?
			(int) method->toctime : (int) msg->time );

	if( msg->flags & (HST_MODIFY|HST_CONTINUE) ) {
		/* Find the old entry */

		sarray[ 0 ].ts_fieldname = H_DATE;
		sarray[ 0 ].ts_pattern = (unsigned char *)buffer;
		sarray[ 0 ].ts_operation = (int (*)())TLEQ;
		sarray[ 1 ].ts_fieldname = H_ONAME;
		sarray[ 1 ].ts_pattern = (unsigned char *)msg->oname;
		sarray[ 1 ].ts_operation = (int (*)())TLEQ;
		sarray[ 2 ].ts_fieldname = H_ODEVICE;
		sarray[ 2 ].ts_pattern = (unsigned char *)msg->odevice;
		sarray[ 2 ].ts_operation = (int (*)())TLEQ;
		sarray[ 3 ].ts_fieldname = (unsigned char *)NULL;

		entryno = TLsearch1( h_tid, sarray, TLEND, TLBEGIN, TL_AND );

		if( entryno < 0 ) {
			if( entryno == TLBADENTRY )
				brlog( "No history entry for oname %s odevice %s time %s to modify",
					msg->oname, msg->odevice, buffer );
			else brlog( "hst_update(): search for existing history entry returns %d",
				entryno );

			return( 0 );

		}

		if( TLread( h_tid, entryno, eptr ) != TLOK ) {
			brlog( "Unable to read entry number %d in history log" );
			return( 0 );
		}
	}

	/* Now done by separate message */
	hst_invalidate( m_slot, msg->labels );

	if( msg->flags & HST_IS_TOC ) {
		if( msg->flags & HST_CONTINUE ) {
			char *ptr, *tmname;
			if( !(tmname = (char *) TLgetfield( h_tid, eptr, H_TMNAME ) ) ) {
				brlog( "hst_update(): Unable to read TMNAME field in history entry" );
				return( 0 );
			}
				
			if( !(ptr = (char *)malloc( strlen( msg->labels )
				+ strlen( tmname ) + 2 ) ) ) {
				brlog( "hst_update(): Unable to malloc memory for history table update");
				return( 0 );
			}

			(void) sprintf( ptr, "%s,%s", tmname, msg->labels );
			(void) TLassign( h_tid, eptr, H_TMNAME, ptr );
			free( ptr );

		} else {
			(void) TLassign( h_tid, eptr, H_TMNAME, msg->labels );
			(void) TLassign( h_tid, eptr, H_ARCHTOC, "Y" );
		}
	} else if( msg->flags & HST_CONTINUE ) {

		char *ptr, *dmname;
		if( !(dmname = (char *) TLgetfield( h_tid, eptr, H_TMNAME ) ) ) {
			brlog( "hst_update(): Unable to read DMNAME field in history entry" );
			return( 0 );
		}
			
		if( !(ptr = (char *)malloc( strlen( msg->labels )
			+ strlen( dmname ) + 2 ) ) ) {
			brlog( "hst_update(): Unable to malloc memory for history table update");
			return( 0 );
		}

		(void) sprintf( ptr, "%s,%s", dmname, msg->labels );
		(void) TLassign( h_tid, eptr, H_DMNAME, ptr );
		free( ptr );

	} else {
		(void) TLassign( h_tid, eptr, H_DMNAME, msg->labels );

		(void) TLassign( h_tid, eptr, H_DATE, buffer );
		
		if( !(msg->flags & HST_MODIFY) ) {
			(void) TLassign( h_tid, eptr, H_TAG, method->entry.tag );
			(void) TLassign( h_tid, eptr, H_METHOD, method->entry.method );
			(void) TLassign( h_tid, eptr, H_OPTIONS, method->entry.options );
		}

		(void) TLassign( h_tid, eptr, H_ONAME, msg->oname );
		(void) TLassign( h_tid, eptr, H_ODEVICE, msg->odevice );

		if( method->entry.dgroup )
			(void) TLassign( h_tid, eptr, H_DGROUP, method->entry.dgroup );

		(void) TLassign( h_tid, eptr, H_DDEVICE, method->entry.ddevice );
		(void) TLassign( h_tid, eptr, H_DCHAR, method->dchar );

		(void) sprintf( buffer, "%d", msg->nvolumes );
		(void) TLassign( h_tid, eptr, H_DNVOL, buffer );

		(void) sprintf( buffer, "%d", msg->size );
		(void) TLassign( h_tid, eptr, H_SIZE, buffer );
	}

	(void) TLassign( h_tid, eptr, H_TOCNAME, msg->tocname );

	if( msg->flags & (HST_MODIFY|HST_CONTINUE) )
		rc = TLwrite( h_tid, entryno, eptr );
	else rc = TLappend( h_tid, TLEND, eptr );

	if( rc != TLOK )
		brlog( "hst_update(): TLappend returns %d", rc );

	(void) TLfreeentry( h_tid, eptr );

	(void) TLsync( h_tid );

	if( msg->flags & HST_DO_ARCHIVE_TOC ) {
		method->state |= MD_NEEDTOC;

		method->toctime = msg->time;

		/* Invalidate these labels */
		if( dlbls = s_to_argv( msg->labels, "," ) ) {
			for( i = 0; (*dlbls)[i]; i++ ) 
				(void) lbl_insert( m_slot, (*dlbls)[i] );
			argv_free( dlbls );
		}
	}

	return( 1 );
}

static
do_entry( tid, eptr, fieldname, argv )
int tid;
ENTRY eptr;
unsigned char *fieldname;
argv_t *argv;
{
	register some = 0, i, j;
	register argv_t *t_argv;
	char *ptr;

	if( !(ptr = (char *)strdup( (char *)TLgetfield( tid, eptr, fieldname ))) )
		return( 0 );

	if( !*ptr ) return( 0 );

	if( !(t_argv = s_to_argv( ptr, "," )) ) {
		brlog( "Unable to invalidate labels: %s",
			brerrno( ENOMEM ) );
		return( -1 );
	}

	for( i = 0; (*argv)[i]; i++ ) {
		for( j = 0; (*t_argv)[j]; j++ ) 
			if( !strcmp( (*argv)[i], (*t_argv)[j] ) ) {
				if( !(ptr = (char *)malloc( strlen( (*argv)[i] ) + 2 ) ) ) {
					brlog( "Unable to invalidate labels: %s", brerrno( ENOMEM ) );
					argv_free( t_argv );
					return( -1 );
				}
				(void) sprintf( ptr, "!%s", (*argv)[i] );
				free( (*t_argv)[j] );
				(*t_argv)[j] = ptr;
				some++;
			}
		if( some ) {
			if( !(ptr = argv_to_s( t_argv, ',' ) ) ) {
				brlog( "Unable to invalidate labels: %s", brerrno( ENOMEM ) );
				argv_free( t_argv );
				return( -1 );
			}
			(void) TLassign( tid, eptr, fieldname, ptr );
			free( ptr );
		}
	}

	argv_free( t_argv );

	return( some );
}

/* Invalidate labels in history table and method's invalid label list */
void
hst_invalidate( m_slot, labels )
int m_slot;
char *labels;
{
	register entryno = 1, some, rc, i, close_it = FALSE;
	char *lbl, *path;
	TLdesc_t descr;
	argv_t *l_argv;
	ENTRY eptr;

	if( !labels || !*labels )
		return;

	if( !h_tid ) {
		close_it = TRUE;

		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = (unsigned char *)H_ENTRY_F;
		path = (char *)bk_get_histlog_path();

		if( (rc = TLopen( &h_tid, path, &descr, O_RDWR|O_CREAT, 0644 ) ) != TLOK 
			&& rc != TLBADFS && rc != TLDIFFFORMAT ) {
			if( rc == TLFAILED ) 
				brlog( "hst_invalidate(): TLopen of history table %s fails: %s",
					path, brerrno( errno )  );
			else brlog( "hst_invalidate(): TLopen of history table %s returns %d",
				path, rc );
			return;
		}
		insert_format( h_tid, H_ENTRY_F );
	}


	if( !(eptr = TLgetentry( h_tid )) ) {
		brlog( "Unable to read history table: %s", brerrno( ENOMEM ) );
		return;
	}

	/* Make a copy so that strtok doesn't destroy this copy */
	if( !(lbl = strdup( labels )) )
		return;

	if( !(l_argv = s_to_argv( lbl, "," )) ) {
		brlog( "Unable to invalidate labels in history table: %s", brerrno( ENOMEM ) );
		free( lbl );
		return;
	}

	/* Invalidate these labels in the method's invalid label list */
	for( i = 0; (*l_argv)[i]; i++ ) 
		(void) lbl_insert( m_slot, (*l_argv)[i] );

	while( TRUE ) {
		some = 0;

		if( TLread( h_tid, entryno, eptr ) != TLOK )
			break;

		if( (rc = do_entry( h_tid, eptr, H_DMNAME, l_argv ) ) < 0 ) {
			argv_free( l_argv );
			free( lbl );
			return;

		} else if( rc > 0 ) some++;

		if( (rc = do_entry( h_tid, eptr, H_TMNAME, l_argv ) ) < 0 ) {
			argv_free( l_argv );
			free( lbl );
			return;

		} else if( rc > 0 ) some++;

		if( some ) 
			(void) TLwrite( h_tid, entryno, eptr );

		entryno++;
	}

	argv_free( l_argv );
	free( lbl );

	if( close_it ) {
		(void) TLsync( h_tid );
		(void) TLclose( h_tid );
	}
}

/* Truncate the history log */
void
hst_truncate()
{
	register hst_entryno, rc;
	int period;
	time_t cutoff, now;
	ENTRY hst_entry;
	char cutoff_buf[ 20 ], *ptr;

	if( !h_tid ) {
		TLdesc_t descr;
		char *path;

		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = (unsigned char *)H_ENTRY_F;
		path = (char *)bk_get_histlog_path();

		if( (rc = TLopen( &h_tid, path, &descr, O_RDWR|O_CREAT, 0644 ) ) != TLOK 
			&& rc != TLBADFS && rc != TLDIFFFORMAT ) {
			if( rc == TLFAILED ) 
				brlog( "hst_truncate(): TLopen of history table %s fails: %s",
					path, brerrno( errno )  );
			else brlog( "hst_truncate(): TLopen of history table %s returns %d",
				path, rc );
			return;
		}
		insert_format( h_tid, H_ENTRY_F );
	}

	if( get_period( h_tid, &period ) )
		period = HST_DEFAULT_PERIOD;

	if( !(hst_entry = TLgetentry( h_tid ) ) ) {
		brlog( "hst_truncate(): cannot get memory to truncate history log" );
		return;
	}
	
	/* Calculate cutoff for truncation */
	now = time( 0 );

	if( period > (now / SECS_PER_WEEK) )
		return;

	cutoff = now - period * SECS_PER_WEEK;
	(void) sprintf( cutoff_buf, "0x%x", (int) cutoff );

	hst_entryno = 1;

	while( TRUE ) {
		if( TLread( h_tid, hst_entryno, hst_entry ) != TLOK )
			break;

		if( (ptr = (char *)TLgetfield( h_tid, hst_entry, H_DATE ) )
			&& *ptr
			&& (strcmp( ptr, cutoff_buf ) <= 0) ) {

			if( (ptr = (char *)TLgetfield( h_tid, hst_entry, H_TOCNAME )) && *ptr ) {
				if( unlink( ptr ) == -1 ) {
					brlog( "hst_truncate(): unable to unlink TOC %s: %s",
						ptr, brerrno( errno ) );
				}
			}

			/* Delete this entry and table of contents file */
			(void) TLdelete( h_tid, hst_entryno );

		} else hst_entryno++;
	}
		
	(void) TLsync( h_tid );
	(void) TLfreeentry( h_tid, hst_entry );
}
