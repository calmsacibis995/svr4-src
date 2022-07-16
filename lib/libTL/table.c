/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:table.c	1.9.3.1"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];
extern unsigned char TLinbuffer[];
static int TLinitialized = 0;

/*
	Initialize all global data structures
*/
TLinit()
{
	if( TLinitialized ) return;
	Strncpy( TLtables, "", TL_MAXTABLES * sizeof( tbl_t ) );
	TLinitialized = 1;
}

/* Get a new table id */
int
TLt_get()
{
	register i;
	register tbl_t	*tptr = TLtables;
	for( i = 0; i < TL_MAXTABLES; i++, tptr++ )
		if( !(tptr->status & IN_USE) ) {
			tptr->status |= IN_USE;
			return( i );
		}
	return( -1 );
}

/* Free slot associated with tid */
void
TLt_unget( tid )
int tid;
{
	if( tid >= 0 && tid < TL_MAXTABLES ) {
		TLtables[ tid ].status = 0;
		TLtables[ tid ].hiwater = 0;
	}	
}
		
int
TLt_init( tid )
int tid;
{
	register tbl_t *tptr = TLtables + tid;
	tptr->status &= ~MODIFIED;
	return( TLe_init( tid ) );
}

int
TLt_open( tid, filename, descr, oflag, mode )
int tid, oflag, mode;
unsigned char *filename;
TLdesc_t *descr;
{
	register tbl_t *tptr = TLtables + tid;
	register rc;
	struct stat buf;

	/* Initialize Table entry */
	if( (rc = TLt_init( tid )) != TLOK ) {
		TLt_unget( tid );
		return( rc );
	}

	/* stat the file before the open to determine whether or not it exists */
	if( stat( (char *)filename, &buf ) == -1 ) {
		if( errno == ENOENT ) {
			tptr->file.mode = mode;
			tptr->file.oflag = oflag;
			tptr->file.fid = 0;
			tptr->status |= FOUND_EOF;
		} else {
			TLt_unget( tid );
			return( TLFAILED );
		}
	} else {
		tptr->file.mode = buf.st_mode;
		tptr->file.oflag = oflag;

		/* Open the Table file */
		if( (tptr->file.fid = open( (char *)filename, oflag, tptr->file.mode ))
			== -1 ){
			TLt_unget( tid );
			return( TLFAILED );
		}
	}

	/* Keep our own copy of the file name */
	if( !(tptr->file.name = (unsigned char *)Malloc( Strlen(filename) + 1 ))){
		TLt_unget( tid );
		return( TLNOMEMORY );
	}
	Strcpy( tptr->file.name, filename );

	/* Scan for Self-Describing Comments */
	if( (rc = TLe_prolog( tid, descr )) != TLOK && rc != TLBADFS
		&& rc != TLDIFFFORMAT )
		TLt_unget( tid );

	return( rc );
}

/* Compare 2 descriptions */
int
TLt_desccmp( a, b )
register TLdesc_t *a, *b;
{
	if( a->td_fs != b->td_fs ) return( TLBADFS );
	if( a->td_format && b->td_format && Strcmp( a->td_format, b->td_format ) )
		return( TLDIFFFORMAT );
	return( TLOK );
}

/* Free all memory associated with a table */
void
TLt_free( tid )
int tid;
{
	register tbl_t *tptr = TLtables + tid;
	register entry_t **eptr;
	register count;
	void TLe_free(), TLf_free(), TLd_free();
	if( tptr->file.fid ) {
		(void)close( tptr->file.fid );
		tptr->file.fid = 0;
	}
	if( tptr->file.name ) {
		Free( tptr->file.name );
		tptr->file.name = NULL;
	}
	TLd_free( &(tptr->description) );	/* Description Structure */
	TLf_free( &(tptr->fieldnames) );	/* Fieldnames Structure */
	if( tptr->e_info.nentries ) {
		for( count = tptr->e_info.nentries, eptr = ENTRYTAB(tid, 0); count > 0;
			eptr++, count-- ) {
			/*
			if( *eptr ) {
			*/
				if( (*eptr)->status & IS_END  ) count = 0;
				TLe_free( *eptr );
				Free( *eptr );
			/*
			}
			*/
		}
		Free( tptr->e_table );
		tptr->e_table = NULL;
	}
	tptr->e_info.nentries = 0;
	tptr->e_info.size = 0;
}

/* Write out current notion of table into "the" file */
int
TLt_sync( tid )
int tid;
{
	register tbl_t *tptr = TLtables + tid;
	register fid, rc = TLOK, locked, count, oflag, seekaddr = 0;
	unsigned char fname[ 30 ];
	entry_t **eptr, *entry;

	if( !(tptr->status & MODIFIED) ) return( TLOK );

	/* Create temp file */
	(void)sprintf( (char *)fname, "/var/tmp/TL%d", getpid() );
	if( !(fid = open( (char *)fname, O_RDWR|O_CREAT, 0666 )) )
		return( TLFAILED );

	/* what if whole file has not been read in! */
	if( tptr->e_info.nentries ) {
		for( count = 0, eptr = ENTRYTAB(tid, 1); count < tptr->e_info.nentries;
			eptr++, count++ ) {
			if( ((*eptr)->status & IS_END) ) break;
			else if( !((*eptr)->status & IS_BEGIN) ) {
				if( (rc = TLe_write( fid, tid, (*eptr) )) < 0 ) 
					goto out;
				E_SEEKADDR((*eptr)) = seekaddr;
				seekaddr += rc;
			}
		}
	}
	if( !(tptr->status & FOUND_EOF ) ) {

		/* If EOF has not been seen, there are more entries in the file */
		if( Lseek( tptr->file.fid, tptr->hiwater, 0 ) == -1 ) {
			rc = TLFAILED;
			goto out;
		}
		/*
			ASSERT: All escapes of special characters that are in the
			remainder of the file entries are intact.  These entries
			may be copied as-is.
		*/

		/* Get entry to the position AFTER this entry */
		if( (rc = TLfl_getentry( tid, TLinbuffer, 2 * TL_MAXLINESZ )) < 0 ) {
			if( rc == TLEOF ) {
				/* This 'shouldn't' happen - i.e. this entry should've been
					gotten before, but just in case ... */
				tptr->status |= FOUND_EOF;
				rc = TLBADENTRY;
			}
			goto out;
		}
		while( (rc = TLfl_getentry( tid, TLinbuffer, 2 * TL_MAXLINESZ )) > 0 )
			if( Write( tptr->file.fid, TLinbuffer, rc ) <= 0 ) {
				rc = TLFAILED;
				goto out;
			}
	}

	/* Readjust hiwater to reflect new version of the file */
	if( seekaddr )
		tptr->hiwater = seekaddr;

	/* Create or truncate "table" file */
	if( tptr->file.fid ) {
		(void)close( tptr->file.fid );
		oflag = tptr->file.oflag|O_TRUNC;
	} else oflag = tptr->file.oflag|O_CREAT;
	if( !(tptr->file.fid = open( (char *)tptr->file.name, oflag,
		tptr->file.mode ) )){
		rc = TLFAILED;
		goto out;
	}

	if( TLfl_lock( tptr->file.fid ) == TLOK )
		locked = TRUE;
	else {
		rc = TLFAILED;
		goto out;
	}
		
	/* copy new file to old */
	rc = TLfl_copy( tptr->file.fid, fid );

	if( locked )
		rc = TLfl_unlock( tptr->file.fid );

	if( rc == TLOK )
		tptr->status &= ~MODIFIED;
	
out:
	(void)unlink( (char *)fname );
	(void)close( fid );

	return( rc );
}
