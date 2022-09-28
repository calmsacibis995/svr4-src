/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/request.c	1.5.2.1"

#include	<sys/types.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<table.h>
#include	<restore.h>
#include	<rs.h>
#include	<rsstatus.h>
#include	<errors.h>

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

extern char *getjobid();
extern char *br_get_rsstatlog_path();
extern char *brcmdname;
extern void brlog();
extern void bkerror();
extern void exit();
extern char *strdup();
extern int strcmp();
extern char *f_to_fs();
extern uid_t getuid();
extern int rs_do_auto();

static int rs_tid = 0;
static ENTRY rs_entry;

/* Request restoral of a particular item */
int
request( h_tid, h_entry, rqst )
int h_tid;
ENTRY h_entry;
rs_rqst_t *rqst;
{
	register rc;
	char buffer[ 20 ], *jobid;
	TLsearch_t sarray[ 2 ];

	/* Open Rsstatus table */
	if( !rs_tid ) {
		char *path;
		TLdesc_t descr;
		void insert_format();

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
			return( 0 );
		}
		insert_format( rs_tid, R_RSSTATUS_F );

		/* Get a new entry structure */
		if( !(rs_entry = TLgetentry( rs_tid )) ) {
			bkerror( stderr, ERROR14 );
			return( 0 );
		}
	}

	/* Get a jobid */
	if( !(jobid = getjobid() ) ) {
		bkerror( stderr, ERROR6, NRESTORES, brcmdname  );
		exit( 2 );
	}

	while( TRUE ) {
		/* Look to see if this jobid is already in the file */
		sarray[ 0 ].ts_fieldname = RST_JOBID;
		sarray[ 0 ].ts_pattern = (unsigned char *)jobid;
		sarray[ 0 ].ts_operation = (int (*)())TLEQ;
		sarray[ 1 ].ts_fieldname = (unsigned char *)0;

		if( TLsearch1( rs_tid, sarray, TLEND, TLBEGIN, TL_AND ) >= 0 ){
			/* It is already there? get new id */
			if( !(jobid = getjobid() ) ) {
				bkerror( stderr, ERROR13, jobid );
				exit( 2 );
			}
		} else break;
	}

	/* Fill in entry structure */
	rqst->jobid = (char *)strdup( jobid );
	(void) TLassign( rs_tid, rs_entry, RST_JOBID, jobid );
	(void) TLassign( rs_tid, rs_entry, RST_OBJECT, rqst->object );
	(void) TLassign( rs_tid, rs_entry, RST_TYPE, rqst->type );
	(void) sprintf( buffer, "0x%x", (int) rqst->date );
	(void) TLassign( rs_tid, rs_entry, RST_FDATE, buffer );

	if( !strcmp( rqst->type, R_DIRECTORY_TYPE )
		|| !strcmp( rqst->type, R_FILE_TYPE ) ) {

		rqst->oname = (char *)strdup( f_to_fs( rqst->object ) );
		
		if( rqst->re_oname )
			(void) TLassign( rs_tid, rs_entry, RST_TARGET, rqst->re_oname );

	} else if( !strcmp( rqst->type, R_FILESYS_TYPE ) ) {

		rqst->oname = (char *)strdup( rqst->object );

		if( rqst->re_oname )
			(void) TLassign( rs_tid, rs_entry, RST_REFSNAME, rqst->re_oname );
		if( rqst->re_odev )
			(void) TLassign( rs_tid, rs_entry, RST_REDEV, rqst->re_odev );

	} else if( !strcmp( rqst->type, R_PARTITION_TYPE )
		|| !strcmp( rqst->type, R_DISK_TYPE ) ) {

		rqst->odev = (char *)strdup( rqst->object );

		if( rqst->re_odev )
			(void) TLassign( rs_tid, rs_entry, RST_REDEV, rqst->re_odev );
	}

	(void) sprintf( buffer, "%ld", getuid() );
	(void) TLassign( rs_tid, rs_entry, RST_UID, buffer );
	(void) TLassign( rs_tid, rs_entry, RST_STATUS, RST_PENDING );

	if( rqst->send_mail )
		(void) TLassign( rs_tid, rs_entry, RST_MUID, buffer );

	/* Attempt Automatic restores */
	rc = ( h_tid )? rs_do_auto( h_tid, h_entry, rqst, rs_tid, rs_entry ): 0;

	return( rc );
}
