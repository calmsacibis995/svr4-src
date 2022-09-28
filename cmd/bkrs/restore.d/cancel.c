/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/cancel.c	1.3.2.1"

#include	<fcntl.h>
#include	<stdio.h>
#include	<pwd.h>
#include	<errno.h>
#include	<table.h>
#include	<rsstatus.h>
#include	<errors.h>

extern char *br_get_rsstatlog_path();
extern int f_exists();
extern void bkerror();
extern void brlog();
extern long strtol();

/* Cancel a particular restore request */
int
cancel( jobid )
char *jobid;
{
	register rc, entryno;
	char *path, *euid_p;
	int tid;
	uid_t getuid(), euid;
	ENTRY eptr;
	TLdesc_t descr;
	TLsearch_t sarray[ 2 ];

	path = (char *)br_get_rsstatlog_path();

	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	if( !f_exists( path ) ) {
		bkerror( stderr, ERROR8, jobid );
		return( 0 );
	} 
	descr.td_format = (unsigned char *)R_RSSTATUS_F;

	if( (rc = TLopen( &tid, path, &descr, O_RDWR ) ) != TLOK
		&& rc != TLDIFFFORMAT ) {
		if( rc = TLBADFS )
			TLclose( tid );
		if( rc == TLFAILED ) 
			brlog( "TLopen of restore status table %s fails: errno %ld",
				path, errno );
		else brlog( "TLopen of status table %s returns %d",
			path, rc );
		bkerror( stderr, ERROR12 );
		return( 0 );
	}

	/* Now find out the entryno of the entry just written */
	sarray[ 0 ].ts_fieldname = RST_JOBID;
	sarray[ 0 ].ts_pattern = (unsigned char *)jobid;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	if( (entryno = TLsearch1( tid, sarray, TLEND, TLBEGIN, TL_AND )) < 0 ) {
		/* Not found */
		bkerror( stderr, ERROR8, jobid );
		return( 0 );
	}

	if( euid = getuid() ) {
		/* Not root - check ownership of the request */

		/* Get a new entry structure */
		if( !(eptr = TLgetentry( tid )) ) {
			bkerror( stderr, ERROR7, jobid );
			return( 0 );
		}

		if( TLread( tid, entryno, eptr ) != TLOK ) {
			bkerror( stderr, ERROR7, jobid );
			return( 0 );
		}

		if( !( euid_p = (char *)TLgetfield( tid, eptr, RST_UID ) ) ) {
			bkerror( stderr, ERROR9 );
			return( 0 );
		}

		if( euid != strtol( euid_p, (char **)0, 10 ) ) {
			bkerror( stderr, ERROR10, jobid ); 
			return( 0 );
		}

	}

	TLdelete( tid, entryno );
	TLsync( tid );
	TLclose( tid );
	return( 1 );
}
