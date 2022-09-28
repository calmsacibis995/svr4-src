/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkoper.d/list.c	1.6.3.1"

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdio.h>
#include	<table.h>
#include	<devmgmt.h>
#include	<backup.h>
#include	<bktypes.h>
#include	<bkoper.h>
#include	<bkstatus.h>
#include	<bkmsgs.h>
#include	<errors.h>
#include	<errno.h>

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

static char *bkstatpath;
static time_t bkstatdate;
static high_water = 0;

extern pid_t bkdaemonpid;

static bko_list_t *lhead, *ltail;
bko_list_t *ldot;

void *malloc();
char *bkstrdup(), *bkstrtok(), *bk_get_statlog_path();
void free();
extern char *strncpy();
extern uid_t uname_to_uid();
extern int bkstrcmp();
extern int strcmp();
extern char *bkmsg();
extern void bkerror();
extern void exit();
extern char *brerrno();
extern char *strcpy();
extern void bkstrncpy();
extern unsigned int strlen();
extern int bkm_send();

void
l_init()
{
	lhead = ltail = ldot = (bko_list_t *)NULL;
}

static bko_list_t *
l_malloc()
{
	register bko_list_t *lptr;

	if( lptr = (bko_list_t *)malloc( sizeof( bko_list_t ) ) ) 
		(void) strncpy( (char *) lptr, "", sizeof( bko_list_t ) );
	return( lptr );
}

static void
l_free( lptr )
bko_list_t *lptr;
{
	free( lptr->jobid );
	free( lptr->tag );
	free( lptr->oname );
	free( lptr->odevice );
	free( lptr->starttime );
	free( lptr->dgroup );
	free( lptr->ddevice );
	free( lptr->dchar );
	free( lptr->dmname );
	free( lptr );
}

/* Move ldot to next NOT DONE entry after lptr */
static void
l_movedot( lptr )
bko_list_t *lptr;
{
	for( ldot = lptr;
		ldot && (ldot->flags & BKO_DONE);
		ldot = ldot->forward )
		;
	/*
		if we ran out of list, set dot to last NOT DONE
		entry on the list.
	*/
	if( !ldot ) {
		for( ldot = ltail; 
			ldot && (ldot->flags & BKO_DONE);
			ldot = ldot->backward )
			;
	}
}

/*
	Find a particular entry; numbering indicates whether or
	not BKO_DONE entries are to be counted or not.
*/
static bko_list_t *
l_find( number, numbering )
int number, numbering;
{
	bko_list_t *lptr;

	for( lptr = lhead; lptr && lptr->number <= number; lptr = lptr->forward )
		if( lptr->number == number )
			if( numbering == BKO_LOGICAL
				&& (lptr->flags & BKO_DONE) )
					return( (bko_list_t *)NULL );
			else return( lptr );

	return( (bko_list_t *)NULL );
}

/* Insert this entry into the list, at the end */
static void
l_insert( lptr )
bko_list_t *lptr;
{
	high_water++;
	lptr->number = high_water;

	if( !lhead ) {
		lhead = ldot = ltail = lptr;
		lptr->forward = lptr->backward = 0;
		return;
	}
	lptr->backward = ltail;
	ltail->forward = lptr;
	ltail = lptr;

	if( !ldot )
		if( lhead->flags & BKO_DONE )
			l_movedot( lhead );
		else ldot = lhead;
}

/* Remove entry 'number' from the list and free it */
static void
l_delete( number )
int number;
{
	register bko_list_t *lptr, *tlptr;

	if( !(lptr = l_find( number, BKO_ABSOLUTE )) )
		return;
	
	/* do bookkeepping of head, tail, and dot */
	if( lptr == lhead ) lhead = lptr->forward;
	if( lptr == ltail ) ltail = lptr->backward;
	if( lptr == ldot ) ldot = lptr->forward;

	/* Remove from list */
	if( tlptr = lptr->backward ) 
		tlptr->forward = lptr->forward;
	if( tlptr = lptr->forward )
		tlptr->backward = lptr->backward;

	l_free( lptr );

	/* Make sure that ldot points to a NOT_DONE entry */
	if( ldot->flags & BKO_DONE ) l_movedot( ldot );
}

/* Print out a range of list elements */
int
l_print( start, stop )
int start, stop;
{
	register bko_list_t *lptr;
	register tmp;

	/* Swap endpoints if out of order */
	if( start > stop ) {
		tmp = start;
		start = stop;
		stop = tmp;
	}

	if( !(lptr = l_find( start, BKO_LOGICAL ) ) ) 
		return( FALSE );

	while( lptr && (lptr->number <= stop) ) {

		if( !(lptr->flags & BKO_DONE) ) 
			(void) fprintf( stdout, "%5d %s %s %s %s %s %s\n",
				lptr->number, lptr->jobid, lptr->tag, lptr->odevice, 
				lptr->dgroup, lptr->ddevice, lptr->dmname );

		lptr = lptr->forward;
	}
	return( TRUE );
}

/* Place the relevant fields in eptr into the lptr struct */
static void
l_parse( tid, lptr, eptr )
int tid;
bko_list_t *lptr;
ENTRY	eptr;
{
	register char *ptr, *tptr;

	lptr->jobid = bkstrdup( TLgetfield( tid, eptr, ST_JOBID ) );
	lptr->tag = bkstrdup( TLgetfield( tid, eptr, ST_TAG ) );
	lptr->oname = bkstrdup( TLgetfield( tid, eptr, ST_ONAME ) );
	lptr->odevice = bkstrdup( TLgetfield( tid, eptr, ST_ODEVICE ) );
	lptr->starttime = bkstrdup( TLgetfield( tid, eptr, ST_STARTTIME ) );
	lptr->dgroup = bkstrdup( TLgetfield( tid, eptr, ST_DGROUP ) );
	lptr->ddevice = bkstrdup( TLgetfield( tid, eptr, ST_DDEVICE ) );
	lptr->dchar = bkstrdup( TLgetfield( tid, eptr, ST_DCHAR ) );
	lptr->status = bkstrdup( TLgetfield( tid, eptr, ST_STATUS ) );

	lptr->flags = 0;
	ptr = bkstrdup( TLgetfield( tid, eptr, ST_EXPLANATION ) );
	if( ptr ) {
		if( lptr->dmname = bkstrdup( bkstrtok( ptr, "," )) ) 
			for( tptr = bkstrtok( NULL, "," );
				tptr && *tptr; tptr++ )
				switch( *tptr ) {
				case 'O':
					lptr->flags |= BKO_OVERRIDE;
					break;
				default:
					break;
				}
		free( ptr );
	}

			
	if( ptr = (char *)TLgetfield( tid, eptr, ST_UID ) )
		lptr->uid = uname_to_uid( ptr );
}

/* Compare two list entries */
static int
l_equal( a, b )
bko_list_t *a, *b;
{
	if( bkstrcmp( a->jobid, b->jobid ) ) return( FALSE );
	if( bkstrcmp( a->tag, b->tag ) ) return( FALSE );
	if( a->uid != b->uid ) return( FALSE );
	if( bkstrcmp( a->oname, b->oname ) ) return( FALSE );
	if( bkstrcmp( a->odevice, b->odevice ) ) return( FALSE );
	if( bkstrcmp( a->dgroup, b->dgroup ) ) return( FALSE );
	if( bkstrcmp( a->ddevice, b->ddevice ) ) return( FALSE );
	if( bkstrcmp( a->dmname, b->dmname ) ) return( FALSE );
	if( bkstrcmp( a->status, b->status ) ) return( FALSE );
	return( TRUE );
}

/* if (jobid,tag) exists in the list, return a pointer to it */
static bko_list_t *
l_search( jobid, tag )
char *jobid, *tag;
{
	register bko_list_t *lptr;
	for( lptr = lhead; lptr; lptr = lptr->forward )
		if( !strcmp( lptr->jobid, jobid )
			&& !strcmp( lptr->tag, tag ) )
			return( lptr );
	return( (bko_list_t *)NULL );
}

/* check to see that existing entries still need servicing */
static void
l_chkold( tid, eptr )
int tid;
ENTRY eptr;
{
	register number, entryno;
	register bko_list_t *lptr, *tlptr;
	TLsearch_t sarray[ 3 ];

	sarray[ 2 ].ts_fieldname = (unsigned char *)NULL;

	/*
		Sequence through the list, checking that each
		entry is still in the status table.
	*/
	lptr = lhead;
	while( lptr ) {
		sarray[ 0 ].ts_fieldname = ST_JOBID;
		sarray[ 0 ].ts_pattern = (unsigned char *)lptr->jobid;
		sarray[ 0 ].ts_operation = (int (*)())TLEQ;

		sarray[ 1 ].ts_fieldname = ST_TAG;
		sarray[ 1 ].ts_pattern = (unsigned char *)lptr->tag;
		sarray[ 1 ].ts_operation = (int (*)())TLEQ;

		entryno = TLsearch1( tid, sarray, TLBEGIN, TLEND, TL_AND );
		if( entryno <= 0 ) {
			/*
				The entry no longer exists in the backup
				status table - delete it from the list.
			*/
			(void) fprintf( stdout, (char *)bkmsg(ERROR3), lptr->number,
				lptr->jobid, lptr->tag );
			number = lptr->number;
			lptr = lptr->forward;
			l_delete( number );

			continue;
		}

		/*
			Compare the entry in the status table with
			the one in the list.  If it is no longer
			waiting, delete it.   If it is now waiting
			for a different volume, delete it, the new
			version will be added later.
		*/
		if( TLread( tid, entryno, eptr ) < 0 ) {
			bkerror( stderr, ERROR4, entryno );
			exit( 2 );
		}
		
		if( !(tlptr = l_malloc() ) ) {
			bkerror( stderr, ERROR5, brerrno( ENOMEM ) );
			exit( 2 );
		}
		
		l_parse( tid, tlptr, eptr );

		if( !l_equal( lptr, tlptr ) ) {
			if( !(lptr->flags & BKO_DONE) )
				(void) fprintf( stdout, (char *)bkmsg(ERROR3), lptr->number,
					lptr->jobid, lptr->tag );
			
			number = lptr->number;
			lptr = lptr->forward;
			l_delete( number );

			continue;
		}
		l_free( tlptr );
		lptr = lptr->forward;
	}
}

/* Insert new WAITING methods into list */
static int
l_getnew( tid, eptr, users )
int tid;
ENTRY	eptr;
argv_t *users;
{
	register dot, entryno, some = 0;
	TLsearch_t sarray[ 3 ];
	register bko_list_t *lptr;
	register char *ptr;

	/* 
		Find beginning of this bkdaemon session.
		Check each time since the status log cleanup could
		change the STARTing spot.
	*/
	sarray[ 0 ].ts_fieldname = ST_JOBID;
	sarray[ 0 ].ts_pattern = ST_START;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = ST_JOBID;
	sarray[ 1 ].ts_pattern = ST_STOP;
	sarray[ 1 ].ts_operation = (int (*)())TLEQ;
	sarray[ 2 ].ts_fieldname = (unsigned char *)NULL;

	dot = TLsearch1( tid, sarray, TLEND, TLBEGIN, TL_OR );
	if( dot <= 0 ) 
		return( 0 );

	if( TLread( tid, dot, eptr ) != TLOK ) {
		bkerror( stderr, ERROR4, dot );
		exit( 2 );
	}

	if( ptr = (char *)TLgetfield( tid, eptr, ST_JOBID ) ) {
		if( bkstrcmp( ptr, (char *) ST_START ) )
			return( 0 );
	} else return( 0 );

	sarray[ 0 ].ts_fieldname = ST_STATUS;
	sarray[ 0 ].ts_pattern = (unsigned char *)ST_WAITING;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)NULL;

	/* Sequence thru bkstatus table looking for WAITING methods */
	for( ;
		(entryno = TLsearch1( tid, sarray, dot, TLEND, TL_AND )) > 0;
		dot = entryno + 1 ) {
		
		if( TLread( tid, entryno, eptr ) != TLOK ) {
			bkerror( stderr, ERROR4, entryno );
			exit( 2 );
		}

		if( !(lptr = l_malloc() ) ) {
			bkerror( stderr, ERROR5, brerrno( ENOMEM ) );
			exit( 2 );
		}
		
		l_parse( tid, lptr, eptr );

		/* If this WAITING method is not in the list, add it */
		if( l_search( lptr->jobid, lptr->tag ) ) {
			l_free( lptr );

		} else {
			l_insert( lptr );
			some++;
		}
	}
	return( some );
}

/*
	Update the list, if there are entries that no longer need
	service, delete them.  If new ones have arrived, add them.
*/
int
l_update( users )
argv_t *users;
{
	struct stat statbuf;
	register rc;
	TLdesc_t descr;
	ENTRY eptr;
	int stat_tid;
	char *path;

	/* Is this the first time we've been called? */
	if( !high_water ) {
		bkstatpath = bk_get_statlog_path();
		bkstatdate = 0;
	}

	/* Status log doesn't exist */
	if( stat( bkstatpath, &statbuf ) == -1 )
		return( 0 );

	/* Status log hasn't changed */
	if( statbuf.st_mtime <= bkstatdate )
		return( 0 );

	/* Record new modification date */
	bkstatdate = statbuf.st_mtime;

	/* Open the table */
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = (unsigned char *)ST_ENTRY_F;
	path = (char *)bk_get_statlog_path();

	if( (rc = TLopen( &stat_tid, path, &descr, O_RDWR, 0644 ) )
		!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
		if( rc == TLFAILED ) 
			bkerror( stderr, ERROR6, bkstatpath, brerrno( errno ) );
		else bkerror( stderr, ERROR7, bkstatpath, rc );
		exit( 2 );
	}

	/* Get an entry element for the new status info */
	if( !(eptr = TLgetentry( stat_tid )) ) {
		bkerror( stderr, ERROR5, brerrno( ENOMEM ) );
		exit( 2 );
	}

	/* Check to see that existing entries are still waiting */
	if( lhead ) l_chkold( stat_tid, eptr );

	/* Insert new requests into list */
	rc = l_getnew( stat_tid, eptr, users );

	(void) TLfreeentry( stat_tid, eptr );

	(void) TLclose( stat_tid );

	return( rc );

}

/* Service a particular backup operation */
int
l_service( number )
int number;
{
	register bko_list_t *lptr;
	bkdata_t data;
	register rc = FALSE;
	
	if( !(lptr = l_find( number, BKO_LOGICAL ) ) )
		return( FALSE );

	/* Just to make sure
fprintf(stderr,"entering list.c\n");
	(lptr->dmname)[BKLABEL_SZ - 1] = '\0';
	*/

	switch( getvol( lptr->ddevice, lptr->dchar, lptr->dmname, data.volume.label,
		BKLABEL_SZ - 1, ((lptr->flags & BKO_OVERRIDE)?
		(DM_AUTO|DM_OLABEL|DM_FORMAT|DM_CHKLBL): (DM_AUTO|DM_FORMAT|DM_CHKLBL)),
		(char *)0 ) ) {

	case 0:
	case 4:
		/* Got a label */

		bkstrncpy( data.volume.method_id.jobid, BKJOBID_SZ,
			lptr->jobid, strlen( lptr->jobid) );
		bkstrncpy( data.volume.method_id.tag, BKTAG_SZ, lptr->tag,
			strlen( lptr->jobid ) );

		while( TRUE ) {
			if( bkm_send( bkdaemonpid, VOLUME, &data ) == -1 ) {
				switch( errno ) {
				case EEXIST:
					bkerror( stderr, ERROR15,
						"backup operations have terminated" );
					exit( 2 );
					/*NOTREACHED*/
					break;

				case EINTR:
					/* If it was interrupted, try again */
					continue;

				default:
					bkerror( stderr, ERROR15, brerrno( errno ) );
					exit( 2 );
					/*NOTREACHED*/
					break;
				}
			}
			break;
		}

		lptr->flags |= BKO_DONE;
		l_movedot( lptr );

		rc = TRUE;
		break;

	case 3:
		/* User typed quit */
		exit( 2 );
		/*NOTREACHED*/
		break;

	case 1:
	case 2:
	default:
		/* Unknown or bad device */
		bkerror( stderr, ERROR16, lptr->ddevice );
		break;

	}

/**
fprintf(stderr,"leaving list.c\n");
**/
	return( rc );
}

/*
	Set Dot to the entry that has a particular number, or
	if there isn't one, to the one just after it.
*/
int
l_setdot( number )
int number;
{
	register bko_list_t *lptr;
	if( !(lptr = l_find( number, BKO_LOGICAL )) ) 
		return( FALSE );
	if( lptr->flags & BKO_DONE )
		l_movedot( lptr );
	else ldot = lptr;
	return( TRUE );
}

/* Is the list empty? */
int
l_empty()
{
	register bko_list_t *lptr;
	if( !(lptr = lhead) ) return( TRUE );
	while( lptr ) {
		if( !(lptr->flags & BKO_DONE) ) return( FALSE );
		lptr = lptr->forward;
	}
	return( TRUE );
}

/* Find number of the first NON-DONE entry */
int
l_head()
{
	register bko_list_t *lptr;

	for( lptr = lhead;
		lptr && (lptr->flags & BKO_DONE);
		lptr = lptr->forward )
		;
	return( lptr? lptr->number: 0 );
}

/* Find number of the first NON-DONE entry */
int
l_tail()
{
	register bko_list_t *lptr;

	for( lptr = ltail;
		lptr && (lptr->flags & BKO_DONE);
		lptr = lptr->backward )
		;
	return( lptr? lptr->number: 0 );
}
