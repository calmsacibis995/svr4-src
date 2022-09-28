/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:restore.d/mail.c	1.4.2.1"

#include	<fcntl.h>
#include	<stdio.h>
#include	<errno.h>
#include	<table.h>
#include	<rsnotify.h>
#include	<errors.h>

#define	DEFAULT_OPER	"root"

char *rsgetlogin();
char *bk_get_notify_path();

extern void brlog();
extern char *strdup();
extern pid_t getpid();
extern void bkerror();
extern char *brerrno();

static tid = 0;
static char *operator = (char *)0;
static ENTRY entry;

void
m_notify_operator()
{
	FILE	*fptr;
	char tmpfname[ 25 ], command[ 80 ], *logname = rsgetlogin();

	if( !tid ) {
		int entryno, rc;
		char *path;
		TLdesc_t descr;

		path = (char *)bk_get_notify_path();

		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = (unsigned char *)RNTFY_ENTRY_F;

		if( (rc = TLopen( &tid, path, &descr, O_RDWR, 0600 ) ) != TLOK 
			&& rc != TLDIFFFORMAT && rc != TLBADFS ) {
			if( rc == TLFAILED ) 
				brlog( "TLopen of restore status table %s fails: %s",
					path, brerrno( errno ) );
			else brlog( "TLopen of status table %s returns %d", path, rc );
			operator = DEFAULT_OPER;

		} else if( !(entry = TLgetentry( tid )) ) {
			operator = DEFAULT_OPER;
		} else {

			entryno = 1;
			while( (rc = TLread( tid, entryno, entry )) == TLOK ) {
				if( (operator = (char *)TLgetfield( tid, entry, RNTFY_OPERNAME ))
					&& *operator ) {
					operator = (char *)strdup( operator );
					break;
				} else entryno++;
			}

			if( !operator || !*operator )
				operator = DEFAULT_OPER;

			(void) TLfreeentry( tid, entry );
		}
	}

	(void) sprintf( tmpfname, "/tmp/rsm-%ld", getpid() );
	if( !(fptr = fopen( tmpfname, "w+" )) ) {
		bkerror( stderr, ERROR6, brerrno( errno ), logname );
		return;
	}

	(void) fprintf( fptr, "There are pending restore requests from %s.\n", logname );

	(void) fclose( fptr );

	(void) sprintf( command, "mail %s <%s; rm -f %s >/dev/null 2>&1", operator,
		tmpfname, tmpfname );
	(void) system( command );
}
