/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsoper.d/list.c	1.4.2.1"

#include	<sys/types.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<pwd.h>
#include	<errno.h>
#include	<bktypes.h>
#include	<table.h>
#include	<restore.h>
#include	<rsstatus.h>
#include	<rsoper.h>
#include	<errors.h>

#ifndef TRUE
#define	TRUE 1
#define	FALSE 0
#endif

extern char *bkstrdup();
extern void en_reread();
extern void en_free();
extern char *br_get_rsstatlog_path();
extern void brlog();
extern void bkerror();
extern uid_t uname_to_uid();
extern char *brerrno();
extern int e_consistent();
extern int sublist();
extern int rs_crank();
extern void m_send_msg();
extern char *uid_to_uname();
extern char *in_archive();
extern void free();

#define NEED_ONAME	0x1
#define NEED_ODEVICE	0x2
#define NEED_METHOD	0x4
#define NEED_LABEL	0x8

static int rs_tid = 0;

#ifdef NOTUSED
/* Free an entire list */
static void
l_free( list )
rs_entry_t *list;
{
	rs_entry_t *entry;
	while( list ) {
		entry = list;
		list = entry->next;
		en_free( entry );
	}
}
#endif

/*
	Fill a list of restore requests that might be satisfied by this archive.
*/
int
l_fill( list, jobs, oname, odevice, method, label, toc )
rs_entry_t **list;
argv_t *jobs;
char *oname, *odevice, *method, *label;
int toc;
{
	register rc, some = FALSE;
	register char *path;
	rs_entry_t *entry, *en_match();
	TLdesc_t descr;
	void insert_format();

	if( !rs_tid ) {

		path = (char *)br_get_rsstatlog_path();

		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = (unsigned char *)R_RSSTATUS_F;

		/* XXX - check out TLDIFFFORMAT */
		if( (rc = TLopen( &rs_tid, path, &descr, O_RDWR, 0600 ) ) != TLOK 
			&& rc != TLDIFFFORMAT && rc != TLBADFS ) {
			if( rc == TLFAILED ) 
				brlog( "TLopen of restore status table %s fails: errno %ld",
					path, errno );
			else brlog( "TLopen of status table %s returns %d", path, rc );
			bkerror( stderr, ERROR12 );
			return( 0 );
		}
		insert_format( rs_tid, R_RSSTATUS_F );
	}

	while( TRUE ) {
		/* Find next restore request that may be satisfied with this archive */
		if( !(entry = en_match( rs_tid, jobs, oname, odevice, method, label, toc )) )
			break;

		/* Found one, put it in the list */
		*list = entry;
		list = &(entry->next);
		some = TRUE;
	}

	return( some );
}

#ifdef NOTUSED
/* Is there a DISK, FILE SYSTEM, or PARTITION request in the list */
static
is_full_type( list )
rs_entry_t **list;
{
	register unsigned char *type;

	while( *list ) {
		type = (*list)->type;
		if( !strcmp( (char *)type, R_PARTITION_TYPE )
			|| !strcmp( (char *)type, R_FILESYS_TYPE )
			|| !strcmp( (char *)type, R_DISK_TYPE ) )
			return( TRUE );
	}
	return( FALSE );
}

/*
	Trim the list, filtering on "user" and/or "jobid". Also, satisfy disk,
	partition, or file system requests before file and directory requests.
	returns TRUE if a "FULL"-type restore is to be done.
*/
static int
l_trim( list, user, jobid )
rs_entry_t **list;
char *user, *jobid;
{
	register rs_entry_t *entry, **lptr;
	register rc;
	register uid_t uid = uname_to_uid( user );
	register unsigned char *type;

	lptr = list;
	if( user || jobid ) {
		while( *lptr ) {
			if( (jobid && strcmp( (char *)(*lptr)->jobid, jobid ) )
				|| (user && ((*lptr)->uid != uid)) ) {
				/* Doesn't match - delete it */
				entry = *lptr;
				*lptr = entry->next;
				en_free( entry );
				continue;
			}
		}
	}

	/* Now choose the first "FULL" type restore request, if there is one */
	if( rc = is_full_type( list ) ) {
		while( *list ) {
			type = (*list)->type;
			if( !strcmp( (char *)type, R_PARTITION_TYPE )
				|| !strcmp( (char *)type, R_FILESYS_TYPE )
				|| !strcmp( (char *)type, R_DISK_TYPE ) ) {

				/*
					This is the first "FULL" type restore request - free the 
					remainder of the list.
				*/
				lptr = &((*list)->next);
				(*list)->next = (rs_entry_t *)0;
				l_free( *lptr );
				free( lptr );

			} else if( !strcmp( (char *)type, R_FILE_TYPE )
				|| !strcmp( (char *)type, R_DIRECTORY_TYPE ) ) {

				/*
					There are FULL type restore requests after this non-FULL one
					the list, so delete this one.
				*/
				entry = (*list)->next;
				list = &(entry->next);
				en_free( entry );
			}
		}
	}
	return( rc );
}
#endif

/* Prune a list so that each entry is of the same type */
static void
l_prune( list, oname, odevice, method, label, istoc )
rs_entry_t **list;
char *oname, *odevice, *method, *label;
int istoc;
{
	register entryno;
	register rs_entry_t *lptr;
	ENTRY eptr;
	TLsearch_t sarray[ 2 ];

	/* Get a new entry structure */
	if( !(eptr = TLgetentry( rs_tid )) ) {
		bkerror( stderr, ERROR18, brerrno( ENOMEM ) );
		return;
	}

	sarray[ 0 ].ts_fieldname = RST_JOBID;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	while( lptr = (*list) ) {
		if( !(lptr->flags & RS_NO_PRUNE)
			&& !e_consistent( lptr, oname, odevice, method, label, istoc ) ) {
			*list = lptr->next;
			en_free( lptr );

		} else {
			/* Mark this entry as 'ACTIVE' in the status table. */

			/* Find entry in table and get status */
			sarray[ 0 ].ts_pattern = (unsigned char *)lptr->jobid;

			if( (entryno = TLsearch1( rs_tid, sarray, TLBEGIN, TLEND, TL_AND )) < 0 ) {
				/* Not found??? assume that it is okay */
				brlog( "entry jobid: %s object %s disappeared from rsstatus.tab.\n",
					lptr->jobid, lptr->object );
				*list = lptr->next;
				en_free( lptr );
				continue;
			}

			if( TLread( rs_tid, entryno, eptr ) != TLOK ) {
				brlog( "Unable to read entry for jobid %s object %s in rsstatus.tab\n",
					lptr->jobid, lptr->object );
				*list = lptr->next;
				en_free( lptr );
				continue;
			}

			(void) TLassign( rs_tid, eptr, RST_STATUS, RST_ACTIVE );
			(void) TLwrite( rs_tid, entryno, eptr );

			list = &(lptr->next);
		}
	}

	(void) TLfreeentry( rs_tid, eptr );
	(void) TLsync( rs_tid );
}

/* check to see if this archive is known */
int
l_arch_check( list, dmnames, dchar, toc )
rs_entry_t **list;
char **dmnames, **dchar;
int toc;
{
	register rs_entry_t *lptr = *list;
	register dmn_sz;

	if( !*dmnames ) return( 0 );

	dmn_sz = strlen( *dmnames );

	/* As soon as nothing more is needed, quit */
	while( lptr ) {
		if( toc ) {
			if( lptr->tlabel && *(lptr->tlabel) ) {
				if( !strncmp( *dmnames, (char *)lptr->tlabel, dmn_sz ) ) {

					/* this is the one we've been looking for */
					*dchar = strdup( (char *)lptr->dchar );
					*dmnames = strdup( (char *)lptr->tlabel );
					return( 1 );
				}
			}
		} else {
			if( lptr->dlabel && *(lptr->dlabel) ) {
				if( !strncmp( *dmnames, (char *)lptr->dlabel, dmn_sz ) ) {

					/* this is the one we've been looking for */
					*dchar = strdup( (char *)lptr->dchar );
					*dmnames = strdup( (char *)lptr->dlabel );
					return( 1 );
				}
			}
		}
		lptr = lptr->next;
	}

	return( 0 );
}

/* Check a list to see if there is enough information to do a restore */
int
l_pre_check( list, oname, odevice, method, label, toc )
rs_entry_t **list;
char **oname, **odevice, **method, **label;
int toc;
{
	register rs_entry_t *lptr = *list;
	register needed = 0;
	/*
		If this is a table of contents method, it must be populated on the
		disk first.
	*/
	if( toc ) return( !(*oname && *odevice && *label ) );

	/* Look to see what is needed */
	if( !*oname ) needed |= NEED_ONAME;
	if( !*odevice ) needed |= NEED_ODEVICE;
	if( !*method ) needed |= NEED_METHOD;
	needed |= NEED_LABEL;

	/* As soon as nothing more is needed, quit */
	while( lptr && needed ) {
		if( (needed & NEED_ONAME) && lptr->tmoname ) {
			*oname = strdup( (char *)lptr->tmoname );
			needed &= ~NEED_ONAME;
		}
		if( (needed & NEED_ODEVICE) && lptr->tmodev ) {
			*odevice = strdup( (char *)lptr->tmodev );
			needed &= ~NEED_ODEVICE;
		}
		if( (needed & NEED_METHOD) && lptr->method && *(lptr->method) ) {
			*method = strdup( (char *)(char *)lptr->method );
			needed &= ~NEED_METHOD;
		}
		if( (needed & NEED_LABEL) && lptr->dlabel && *(lptr->dlabel) ) {
			if( (*label && !strncmp( *label, (char *)lptr->dlabel,
				strlen( *label ) ) ) || !*label ) {
				*label = strdup( (char *)lptr->dlabel );
				needed &= ~NEED_LABEL;
			}
		}
		lptr = lptr->next;
	}

	/* Okay to have one at least one of the two */
	if( oname || odevice )
		needed &= ~(NEED_ONAME|NEED_ODEVICE);

	if( *label )
		needed &= ~NEED_LABEL;

	/* Now prune the list so that the candidates are all of the same type */
	/* Also mark them "ACTIVE" in the rsstatus table */
	l_prune( list, *oname, *odevice, *method, *label, toc );

	return( needed );
}

/* Check to see what the method completed */
int
l_post_check( list, label )
rs_entry_t *list;
char *label;
{
	register entryno, tm_used, errors = 0, rc;
	ENTRY rs_entry;
	TLdesc_t descr;
	TLsearch_t sarray[ 2 ];
	char *path;

	/* Re-open rsstatus table */
	(void) TLclose( rs_tid );
	path = (char *)br_get_rsstatlog_path();

	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = (unsigned char *)R_RSSTATUS_F;

	if( (rc = TLopen( &rs_tid, path, &descr, O_RDWR, 0600 ) ) != TLOK 
		&& rc != TLDIFFFORMAT && rc != TLBADFS ) {
		if( rc == TLFAILED ) 
			brlog( "TLopen of restore status table %s fails: errno %d",
				path, errno );
		else brlog( "TLopen of status table %s returns %d", path, rc );
		bkerror( stderr, ERROR12 );
		return( 1 );
	}

	/* Get a new entry structure */
	if( !(rs_entry = TLgetentry( rs_tid )) ) {
		bkerror( stderr, ERROR18, brerrno( ENOMEM ) );
		return( 1 );
	}

	sarray[ 0 ].ts_fieldname = RST_JOBID;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	while( list ) {
		/* Find entry in table and get status */
		sarray[ 0 ].ts_pattern = (unsigned char *)list->jobid;

		if( (entryno = TLsearch1( rs_tid, sarray, TLBEGIN, TLEND, TL_AND )) < 0 ) {
			/* Not found??? assume that it is okay */
			list = list->next;
			continue;
		}

		if( TLread( rs_tid, entryno, rs_entry ) != TLOK ) {
			/* ??? */
			list = list->next;
			continue;
		}

		/* Record this archive attempts status into rsstatus table */
		(void) TLassign( rs_tid, rs_entry, RST_STATUS, list->status );
		(void) TLassign( rs_tid, rs_entry, RST_EXPLANATION, list->explanation );

		/* 
			Automatic request completion depends on whether or not the
			Turing Machine was used for this particular request.  This
			in turn, depends on a non-null Turing State in the table AND
			the label of the volume that was used for this rsoper command
			is in the list of labels that we were expecting.
		*/
		tm_used = list->tmstate && sublist( label, (char *) list->dlabel, " ," );

		if( tm_used && rs_crank( rs_tid, rs_entry, list ) ) {

			m_send_msg( (char *) list->jobid, uid_to_uname( list->uid ), 
				"has been completed" );
			(void) TLdelete( rs_tid, entryno );

		} else {
			bkerror( stdout, ERROR17, list->jobid, uid_to_uname( list->uid ) );
			(void) TLassign( rs_tid, rs_entry, RST_STATUS, RST_PENDING );
			(void) TLassign( rs_tid, rs_entry, RST_EXPLANATION, "" );
			(void) TLwrite( rs_tid, entryno, rs_entry );
			errors++;
		}
		
		list = list->next;
	}
	(void) TLsync( rs_tid );

	return( errors );
}

/* Check to see what requests can be satisfied by this toc */
int
l_toc_post_check( list, label, tocname )
rs_entry_t *list;
char *label, *tocname;
{
	register entryno, errors = 0, rc;
	ENTRY rs_entry;
	TLdesc_t descr;
	TLsearch_t sarray[ 2 ];
	char *path, *mname;

	/* Re-open rsstatus table */
	(void) TLclose( rs_tid );
	path = (char *)br_get_rsstatlog_path();

	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = (unsigned char *)R_RSSTATUS_F;

	if( (rc = TLopen( &rs_tid, path, &descr, O_RDWR, 0600 ) ) != TLOK 
		&& rc != TLDIFFFORMAT && rc != TLBADFS ) {
		if( rc == TLFAILED ) 
			brlog( "TLopen of restore status table %s fails: errno %d",
				path, errno );
		else brlog( "TLopen of status table %s returns %d", path, rc );
		bkerror( stderr, ERROR12 );
		return( 1 );
	}

	/* Get a new entry structure */
	if( !(rs_entry = TLgetentry( rs_tid )) ) {
		bkerror( stderr, ERROR18, brerrno( ENOMEM ) );
		return( 1 );
	}

	sarray[ 0 ].ts_fieldname = RST_JOBID;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	while( list ) {
		/* Find entry in table and get status */
		sarray[ 0 ].ts_pattern = (unsigned char *)list->jobid;

		if( (entryno = TLsearch1( rs_tid, sarray, TLBEGIN, TLEND, TL_AND )) < 0 ) {
			/* Not found??? assume that it is okay */
			list = list->next;
			continue;
		}

		if( TLread( rs_tid, entryno, rs_entry ) != TLOK ) {
			/* ??? */
			list = list->next;
			continue;
		}

		if( mname = in_archive( rs_tid, rs_entry, tocname ) ) {

			bkerror( stdout, ERROR17, list->jobid, uid_to_uname( list->uid ) );

			(void) TLassign( rs_tid, rs_entry, RST_DLABEL, mname );
			(void) TLassign( rs_tid, rs_entry, RST_TLABEL, "" );
			(void) TLwrite( rs_tid, entryno, rs_entry );
			errors++;

		} else {
			/*
				Only crank strategy algorithm if this request was
				waiting on this TOC
			*/
			mname = (char *)TLgetfield( rs_tid, rs_entry, RST_TLABEL );

			if( mname && sublist( label, mname, " ," ) ) {
				(void) TLassign( rs_tid, rs_entry, RST_TLABEL, "" );
				if( tocname && *tocname
					&& rs_crank( rs_tid, rs_entry, list ) ) {

					m_send_msg( (char *) list->jobid, uid_to_uname( list->uid ), 
						"has been completed" );
					(void) TLdelete( rs_tid, entryno );

				} else {
					bkerror( stdout, ERROR17, list->jobid,
						uid_to_uname( list->uid ) );
					(void) TLwrite( rs_tid, entryno, rs_entry );
					errors++;
				}
			}
		}
		
		list = list->next;
	}
	(void) TLsync( rs_tid );

	return( errors );
}
