/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libTL:entry.c	1.10.3.1"

#include <table.h>
#include <internal.h>

#define	FPUTC( fid, c )	\
	if( to == TLgenbuf + 2 * TL_MAXLINESZ ) { \
		if( Write( tid, TLgenbuf, 2 * TL_MAXLINESZ ) != 2 * TL_MAXLINESZ ) \
			return( TLFAILED ); \
		to = TLgenbuf; \
	} \
	*to++ = c;

extern void TLf_free();

extern tbl_t TLtables[];
extern unsigned char TLinbuffer[];
extern unsigned char TLgenbuf[];

/* Grow an entry table - the table is always grown in E_GROWTH chunks */
static int
TLe_grow( tid )
int tid;
{
	entryinfo_t	*einfo = EINFO(tid);
	
	if( !einfo->size ) {
		if( !(ETAB(tid) = (entry_t **)Malloc( E_GROWTH * sizeof( entry_t * ) )))
			return( TLNOMEMORY );
		einfo->size = E_GROWTH;
	} else {
		if( !(ETAB(tid) = (entry_t **)Realloc( ETAB(tid),
			(einfo->size + E_GROWTH) * sizeof( entry_t * ) ) ) )
			return( TLNOMEMORY );
		einfo->size += E_GROWTH;
	}
	return( TLOK );
}

entry_t *
TLe_malloc()
{
	entry_t *entry;
	if( entry = (entry_t *)Malloc( sizeof( entry_t ) ) ) {
		Strncpy( entry, "", sizeof( entry_t ) );
		return( entry );
	} else return( NULL );
}

/* Free space associated with an entry */
void
TLe_free( entry )
entry_t *entry;
{
	if( entry->status & IS_PARSED ) {
		if( E_COMMENT(entry) ) {
			Free( E_COMMENT(entry) );
			E_COMMENT(entry) = NULL;
		}
		TLf_free( E_FIELDS(entry) );
	}
	entry->status = 0;
	E_SEEKADDR(entry) = 0;
}

/* 
	Put a parsed entry structure in the table
*/
int
TLe_pput( tid, pstruct, entryno )
int tid;
parse_t *pstruct;
entryno_t entryno;
{
	register entry_t *entry, **eptr;

	if( !TLe_intable( tid, entryno ) ) return( TLINTERNAL );

	eptr = ENTRYTAB( tid, entryno );

	if( !(entry = *eptr ) ) return( TLINTERNAL );
	if( entry->status & IS_PARSED ) 
		TLe_free( entry );

	entry->comment = pstruct->comment;
	if( pstruct->type & PT_SPECIAL)
		/* ignore ENTRY FORMAT comments here */
		TLf_free( &(pstruct->fields) );
	else entry->fields = pstruct->fields;
	entry->status |= IS_PARSED;
	return( TLOK );
}

/* Copy one entry to another */
TLe_copy( to, from )
entry_t *to, *from;
{
	register rc = TLOK;

	TLe_free( to );
	to->status = from->status;
	E_SEEKADDR(to) = E_SEEKADDR(from);
	if( E_NFIELDS(from) > 0 )
		rc = TLf_copy( E_FIELDS(to), E_FIELDS(from) );
	if( (rc ==  TLOK ) && from->comment ) {
		if( !(to->comment
			= (unsigned char *)Malloc( Strlen( from->comment ) + 1 ) ) ) {
			TLf_free( E_FIELDS(to) );
			return( TLNOMEMORY );
		}
		Strcpy( to->comment, from->comment );
	}
	return( rc );
}

/* Write out an entry to a file, handle escapes in fields */
int
TLe_write( fid, tid, entry )
int fid, tid;
entry_t *entry;
{
	register i, count, nfields;
	tbl_t *tptr = TLtables + tid;
	unsigned char **fptr;
	register unsigned char *from, *to;
	register unsigned char eoe, fs, comment;

	eoe = tptr->description.td_eoe;

	if( entry->status & IS_PARSED ) {
		/*
			If given an entry format, print out THAT many fields.
			Unless, it is a COMMENT line.
		*/
		if( E_NFIELDS(entry ) )
			nfields = ((tptr->status & GOT_FORMAT)? tptr->fieldnames.count:
				E_NFIELDS(entry));
		else nfields = 0;

		fptr = E_GETFIELD( entry, 0 );
		to = TLgenbuf;
		from = *fptr;

		fs = tptr->description.td_fs;
		comment = tptr->description.td_comment;

		if( nfields > 0 ) {

			/* Print out 1st field */
			while( *from ) {
				if( *from == '\\' || *from == fs 
					|| *from == eoe || *from == '#' ) {
					FPUTC( fid, '\\' );
				}

				FPUTC( fid, *from );
				from++;
			}

			/* Write out <sep><fields> */
			for( i = 1, fptr++, from = *fptr;
				i < nfields;
				i++, fptr++, from = *fptr ) {

				FPUTC( fid, fs );

				while( *from ) {
					if( *from == '\\' || *from == fs 
						|| *from == eoe || *from == comment ) {
						FPUTC( fid, '\\' );
					}

					FPUTC( fid, *from );
					from++;
				}
			}
		}

		/* Copy out Comment */
		if( E_COMMENT(entry) ) {
			FPUTC( fid, comment );
			from = E_COMMENT(entry);
			while( *from ) {
				FPUTC( fid, *from );
				from++;
			}
		}

		/* Write out End of Entry char */
		FPUTC( fid, eoe );

		count = to - TLgenbuf;

		if( count > 0 )
			if( Write( fid, TLgenbuf, count ) != count )
				return( TLFAILED );

	} else {
		/*
			File entry. ASSERT: special characters in file entries
			have already been escaped.
		*/
		/* Seek to correct place in the file */
		if( Lseek( tptr->file.fid, E_SEEKADDR(entry), 0 ) != E_SEEKADDR(entry) )
			return( TLFAILED );
		if( (count = TLfl_getentry( tid, TLinbuffer, 2 * TL_MAXLINESZ )) < 0 )
			return( count );

		/* Count includes EOE char */
		TLinbuffer[ count - 1 ] = eoe;

		/* Write out whole entry and then an end of entry char. */
		if( (count && Write( fid, TLinbuffer, count ) != count) )
			return( TLFAILED );
	}
	return( count );
}

/*
	Shift the entry table from entryno either up one (for delete) 
	(direction == -1 ) or down one (for add ) (direction == 1 );
	If delete, the deleted entry is freed.
*/
static int
TLe_shift( tid, entryno, direction )
int tid, direction;
entryno_t entryno;
{
	register entryinfo_t	*einfo = EINFO(tid);
	register entry_t  **eptr;
	register rc;

	if( direction == 0 ) return( TLOK );

	if( direction > 0 ) {
		/* Make sure there is enough room to shift; NOTE: nentries + 1 is END */
		if( einfo->nentries + 5 > einfo->size )
			if( (rc = TLe_grow( tid )) != TLOK ) return( rc );

		for( eptr = ENTRYTAB( tid, einfo->nentries + 1 );
			eptr >= ENTRYTAB( tid, entryno ); eptr-- ) {

			*(eptr + 1) = *eptr;
			*eptr = NULL;
		}
	} else if( TLe_intable( tid, entryno ) ) {
		/* Free 'deleted' entry */
		eptr = ENTRYTAB( tid, entryno );
		TLe_free( *eptr );
		free( *eptr );

		for( ; eptr <= ENTRYTAB( tid, einfo->nentries + 1 ); eptr++ ) {
			*eptr = *(eptr + 1 );
		}
	}
	return( TLOK );
}

/*
	Initialize the Entry and Non-Comment Map tables.
*/
int
TLe_init( tid )
int tid;
{
	register entryinfo_t	*einfo = EINFO(tid);
	register entry_t *begin, *end;
	register rc;

	/* Get memory for entry table and map */
	if( ( rc = TLe_grow( tid )) != TLOK ) return( rc );

	/* Put in BEGIN */
	if( !(begin = (entry_t *)TLe_malloc()) )
		return( TLNOMEMORY );
	begin->status = IS_BEGIN;
	*(ENTRYTAB(tid,0)) = begin;

	/* Put in END */
	if( !(end = (entry_t *)TLe_malloc() ) ) {
		TLe_free( begin );
		free( begin );
		return( TLNOMEMORY );
	}
	end->status = IS_END;
	*(ENTRYTAB(tid,1)) = end;

	einfo->nentries = 0;

	return( TLOK );
}

/*
	Add entry into the table at Entryno.
*/
int
TLe_add( tid, entryno, entry )
int tid;
entryno_t entryno;
entry_t *entry;
{
	register rc;

	if( entryno != T_EEND(tid) && !TLe_intable( tid, entryno ) )
		return( TLBADENTRY );

	/* Make room in entry table for the new entry */
	if( (rc = TLe_shift( tid, entryno, 1 )) != TLOK ) return( rc );

	/* Put entry in table */
	*(ENTRYTAB( tid, entryno )) = entry;

	EINFO(tid)->nentries++;

	return( TLOK );
}

/* Fill in Default Field names */
TLe_dfltformat( fields )
field_t *fields;
{
	register i;
	unsigned char *ptr;
	fields->count = 0;
	for( i = 0; i < TL_MAXFIELDS; i++, fields->count++ ) {
		if( !(ptr = (unsigned char *)Malloc( 5 )) ) {
			TLf_free( fields );
			return( TLNOMEMORY );
		}
		(void)sprintf( (char *)ptr, "%d", i + 1 );
		fields->values[i] = ptr;
	}
	return( TLOK );
}

/* find 1st Non-Commentary entry, looking for Special comments */
int 
TLe_prolog( tid, descr )
int tid;
TLdesc_t	*descr;
{
	register tbl_t *tptr = TLtables + tid;
	register count, done = FALSE, seekaddr = 0, rc = TLOK;
	register trc, gotfs = FALSE, gotformat = FALSE;
	register TLdesc_t *dptr = &(tptr->description);
	parse_t pstruct;
	register entry_t *entry;
	field_t filefieldnames;
	unsigned char	filefs;
	int size;

	/* Set up Description */
	dptr->td_fs = descr->td_fs? descr->td_fs: ':';
	dptr->td_eoe = descr->td_eoe? descr->td_eoe: '\n';
	dptr->td_comment = descr->td_comment? descr->td_comment: '#';

	if( descr->td_format )
		if( !TLp_fieldnames( tid, descr->td_format ) ) return( TLBADFORMAT );

	count = 1;
	if( tptr->file.fid ) {
		if( Lseek( tptr->file.fid, 0L, 0 ) < 0 ) return( TLFAILED );
	} else done = 1;
	while( !done ) {
		if( (rc = TLparse( tid, &pstruct, &size )) == TLEOF ) {
			tptr->status |= FOUND_EOF;
			rc = TLOK;
			break;
		}
		if( rc != TLOK ) return( rc );
		if( !(entry = (entry_t *)TLe_malloc() ) )
			return( TLNOMEMORY );
		if( pstruct.type & PT_SPECIAL ) {
			if( pstruct.fields.count > 0 ) {
				gotformat = TRUE;
				filefieldnames = pstruct.fields;
			}
			if( pstruct.descr.td_fs ) {
				gotfs = TRUE;
				filefs = pstruct.descr.td_fs;
			}
		} else done = TRUE;
		if( pstruct.fields.count != 0 ) {
			/*
				If this is not a comment line, delay the parsing of it
				until AFTER any description info from the open has been
				recorded.
			*/
			entry->status = IN_FILE;
		} else {
			entry->comment = pstruct.comment;
			entry->status |= IS_PARSED;
		}
		E_SEEKADDR(entry) = seekaddr;

		/* keep track of how far the file has been parsed */
		if( seekaddr > tptr->hiwater )
			tptr->hiwater = seekaddr;

		if( (rc = TLe_add( tid, count, entry ) ) != TLOK ) {
			TLe_free( entry );
			Free( entry );
			return( rc );
		}
		count++;
		seekaddr += size;
	}
	if( rc != TLOK ) goto e_prolog_out;

	if( gotfs ) {
		dptr->td_fs = filefs;
		if( descr->td_fs ) {
			rc = TLBADFS;
			goto e_prolog_out;
		}
	}

	if( descr->td_format ) {
		/* Note: descr->td_format is already parsed and in tptr */
		if( gotformat && TLf_compare( &(tptr->fieldnames), &(filefieldnames) ) ) {
			rc = TLDIFFFORMAT;

			/* Use Format in file */
			TLf_free( &(tptr->fieldnames) );
			tptr->fieldnames = filefieldnames;
		}
		tptr->status |= GOT_FORMAT;

	} else if( gotformat ) {
		tptr->fieldnames = filefieldnames;
		tptr->status |= GOT_FORMAT;

	} else if( (trc = TLe_dfltformat( &(tptr->fieldnames) ) ) != TLOK )
		rc = trc;

e_prolog_out:
	return( rc );
}

/*
	Delete an entry table entry at Entryno; Non-comment map is updated 
*/
int
TLe_delete( tid, entryno )
int tid;
entryno_t entryno;
{
	register rc;

	if( !TLe_intable( tid, entryno ) ) return( TLBADENTRY );

	/* Delete the entryno */
	if( (rc = TLe_shift( tid, entryno, -1 )) != TLOK ) return( rc );

	EINFO( tid )->nentries--;

	return( TLOK );
}

/*
	Parse an INFILE entry and put it in the table 
*/
int
TLe_parse( tid, entry, entryno )
int tid;
entryno_t entryno;
entry_t *entry;
{
	parse_t pstruct;
	register tbl_t *tptr = TLtables + tid;
	register rc;
	int size;

	if( entry->status & IS_PARSED ) return( TLOK );
	if( Lseek( tptr->file.fid, E_SEEKADDR(entry), 0 ) == -1 )
		return( TLFAILED );
	if( (rc = TLparse( tid, &pstruct,&size )) == TLEOF )
		return( TLBADENTRY );
	if( rc == TLOK ) rc = TLe_pput( tid, &pstruct, entryno );
	return( rc );
}

/* Return a pointer to the entry entry of an IN-TABLE entry */
int
TLe_getentry( tid, entryno, eptr )
int tid;
entryno_t entryno;
entry_t **eptr;
{
	register rc;
	if( !TLe_intable( tid, entryno ) && (rc = TLe_find( tid, entryno )) != TLOK )
		return( rc );

	*eptr = *(ENTRYTAB(tid, entryno));

	if( !((*eptr)->status & IS_PARSED ) )
		return( TLe_parse( tid, *eptr, entryno ) );

	return( TLOK );
}

/* 
	Find non-TLBEGIN and non-TLEND entries.  This routine makes sure
	that if the entries exist, they end up parsed and in the table.
*/
int
TLe_find( tid, entryno )
int tid;
entryno_t entryno;
{
	register tbl_t *tptr = TLtables + tid;
	entry_t	*entry, *newentry;
	register nseekaddr, rc = TLOK, count, last, findeof = IS_FROM_END(entryno);
	parse_t pstruct;

	/* If in table, then all that is needed is to parse the entry */
	if( !findeof && TLe_intable( tid, entryno ) ) 
		return( TLe_getentry( tid, entryno, &entry ) );

	/* else if whole table has been parsed, then no such entry */
	if( tptr->status & FOUND_EOF ) return( (findeof? TLOK: TLBADENTRY) );

	/* Find Last Entry that is known */
	last = (T_EEND(tid) - 1);
	count = (findeof? 1: (entryno - last));

	/* Seek to where we have parsed in the file */
	nseekaddr = tptr->hiwater;
	if( Lseek( tptr->file.fid, nseekaddr, 0 ) == -1 )
		return( TLFAILED );

	/* Get entry to the position AFTER this entry */
	if( (rc = TLfl_getentry( tid, TLinbuffer, 2 * TL_MAXLINESZ )) < 0 ) {
		if( rc == TLEOF ) {
			/* This 'shouldn't' happen - i.e. this entry should've been
				gotten before, but just in case ... */
			tptr->status |= FOUND_EOF;
			rc = (findeof? TLOK: TLBADENTRY);
		}
		return( rc );
	}
	nseekaddr += rc;

	/* Now, skip passed intervening entries in the file, until the
		desired one if found */
	while( count > 0 ) {
		/* Get next entry from file */
		if( (rc = TLfl_getentry( tid, TLinbuffer, 2 * TL_MAXLINESZ )) < 0 
			&& rc != TLEOF ) 
			return( rc );
		if( rc == 0 || rc == TLEOF ) {
			/* No next entry to get */
			tptr->status |= FOUND_EOF;
			return( (findeof? TLOK: TLBADENTRY) );
		}
		if( !(newentry = (entry_t *)TLe_malloc() ) )
			return( TLNOMEMORY );
		E_SEEKADDR(newentry) = nseekaddr;
		if( nseekaddr > tptr->hiwater )
			tptr->hiwater = nseekaddr;
		nseekaddr += rc;
		newentry->status |= IN_FILE;
		/* Put in table */
		if( (rc = TLe_add( tid, last + 1, newentry ) ) != TLOK ) {
			TLe_free( newentry );
			Free( newentry );
			return( rc );
		}
		if( !findeof ) count--;
		last++;
	}
	/* Now it is in the table */
	if( (rc = TLbparse( tid, &pstruct, TLinbuffer )) == TLOK )
		rc = TLe_pput( tid, &pstruct, entryno );
	return( rc );
}
