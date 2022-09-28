/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/brtoc.c	1.6.2.1"

#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<backup.h>
#include	<brtoc.h>
#include	<errno.h>
#include	<table.h>

static int toc_tid = 0;
static ENTRY tptr = NULL;
static void longtoc();

static ENTRY cptr = NULL;
extern int bklevels;

extern char *br_get_toc_path();
extern void brlog();
extern char *strcpy();
extern void *malloc();
extern unsigned int strlen();
extern void free();
extern int sprintf();

int
br_toc_open(create, jobid, pathname)
int create;
unsigned char *jobid;
unsigned char *pathname;
{
	extern pid_t	getpid();
	register rc;
	TLdesc_t descr;
	char *path, *buffer;

	if( !toc_tid ) {
		descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
		descr.td_format = (unsigned char *)TOC_ENTRY_F;
		path = (char *)br_get_toc_path( (char *) jobid, getpid () );
#ifdef TRACE
		brlog("br_toc_open path = %s pid=%d",path,getpid());
#endif

		BEGIN_CRITICAL_REGION;

		if( (rc = TLopen( &toc_tid, path,
			&descr, create ?  O_RDWR|O_CREAT: O_RDWR, 0644 ) )

			!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
			if( rc == TLFAILED ) 
				brlog( "toc_open(): TLopen of toc table %s fails: errno %ld",
					path, errno );
			else brlog( "toc_open(): TLopen of toc table %s returns %d",
				path, rc );

			END_CRITICAL_REGION;

			return( -1 );
		}

		END_CRITICAL_REGION;

		if(pathname != NULL) {
			(void) strcpy( (char *) pathname, path);
		}
		if(!create) { 
			return(toc_tid);
		}

		if( !(buffer = (char *)malloc( strlen( "ENTRY FORMAT=" )
			+ strlen( (char *) TOC_ENTRY_F ) + 1 ) ) ) {
			brlog("malloc of format buffer failed");
			return(-1);
		}

		if( !(tptr = TLgetentry( toc_tid )) ) {
			brlog("unable to init toc table");
			free (buffer);
			return(-1);
		}
		if( !(cptr = TLgetentry( toc_tid )) ) {
			brlog("unable to init toc table");
			(void) free (buffer);
			TLfreeentry( toc_tid, tptr );
			return(-1);
		}

		(void) sprintf( buffer, "ENTRY FORMAT=%s", TOC_ENTRY_F );

		(void) TLassign( toc_tid, cptr, TLCOMMENT, buffer );


		BEGIN_CRITICAL_REGION;

		(void) TLappend( toc_tid, TLBEGIN, cptr );

		END_CRITICAL_REGION;

		free (buffer);

		(void) TLassign( toc_tid, tptr, TLCOMMENT, (char *) NULL );
	}

	return(toc_tid);
}


void
br_toc_close()
{

	if(tptr)
		(void) TLfreeentry( toc_tid, tptr );

	if(cptr)
		TLfreeentry( toc_tid, cptr );
	if(toc_tid) {
		(void) TLsync( toc_tid );
		(void) TLclose(toc_tid);
	}

	toc_tid = 0;

}



int
br_toc_write(entryno, st, fname, vol, comment)
int entryno;
struct stat *st;
char *fname, *comment;
int vol;
{
	int ret;
	unsigned char buffer[21];

	if( !toc_tid) {
		brlog("toc_write(): toc not open");
		return(-1);
	}

	if(comment) {

		(void) TLassign( toc_tid, cptr, TLCOMMENT, comment );

		BEGIN_CRITICAL_REGION;

		ret = TLappend( toc_tid, entryno + 1, cptr );

		END_CRITICAL_REGION;

		return((ret == TLOK) ? 0 : -1);
	}

	buffer[0] = 0;

	(void) TLassign( toc_tid, tptr, TOC_FNAME, fname );

	(void) sprintf( (char *) buffer, "%d", vol );
	(void) TLassign( toc_tid, tptr, TOC_VOL, buffer );

	if(st != NULL) {
		longtoc ( (char *) buffer, st, tptr);
	}

	BEGIN_CRITICAL_REGION;

/*  entryno starts from 0, libTL from 1, entry format in place, hence the + 1 */

	ret = TLappend( toc_tid, entryno + 1, tptr );

	END_CRITICAL_REGION;

	return((ret == TLOK) ? 0 : -1);
}


static void
longtoc(buffer, st, tptr)
char *buffer;
struct stat *st;
ENTRY tptr;
{

	(void) sprintf( buffer, "0x%lx", st->st_dev );
	(void) TLassign( toc_tid, tptr, TOC_DEV, buffer );

	(void) sprintf( buffer, "%ld", st->st_ino );
	(void) TLassign( toc_tid, tptr, TOC_INODE, buffer );

	(void) sprintf( buffer, "0x%lx", st->st_mode );
	(void) TLassign( toc_tid, tptr, TOC_MODE, buffer );

	(void) sprintf( buffer, "%ld", st->st_nlink );
	(void) TLassign( toc_tid, tptr, TOC_NLINK, buffer );

	(void) sprintf( buffer, "%ld", st->st_uid );
	(void) TLassign( toc_tid, tptr, TOC_UID, buffer );

	(void) sprintf( buffer, "%ld", st->st_gid );
	(void) TLassign( toc_tid, tptr, TOC_GID, buffer );

	(void) sprintf( buffer, "0x%lx", st->st_rdev );
	(void) TLassign( toc_tid, tptr, TOC_RDEV, buffer );

	(void) sprintf( buffer, "%d", (int) st->st_size );
	(void) TLassign( toc_tid, tptr, TOC_SIZE, buffer );

	(void) sprintf( buffer, "0x%lx", (int) st->st_atime );
	(void) TLassign( toc_tid, tptr, TOC_ATIME, buffer );

	(void) sprintf( buffer, "0x%lx", (int) st->st_mtime );
	(void) TLassign( toc_tid, tptr, TOC_MTIME, buffer );

	(void) sprintf( buffer, "0x%lx", (int) st->st_ctime );
	(void) TLassign( toc_tid, tptr, TOC_CTIME, buffer );

}
