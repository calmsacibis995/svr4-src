/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:field.c	1.6.3.1"
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

/* Free memory associated with a field_t structure */
void
TLf_free( fields )
field_t *fields;
{
	register i;
	register unsigned char **nptr = fields->values;
	if( fields->count > 0 ) {
		for( i = 0; i < fields->count; i++, nptr++ )
			if( *nptr ) {
				Free( *nptr );
				*nptr = NULL;
			}
		fields->count = 0;
	}
}

int 
TLf_copy( to, from )
field_t *to, *from;
{
	register i, size;
	register unsigned char **fptr, **tptr;
	TLf_free( to );
	if( from->count > 0 ) {
		to->count = from->count;
		fptr = from->values;
		tptr = to->values;
		for( i = 0; i < from->count; i++, fptr++, tptr++ ) {
			if( !*fptr ) continue;
			size = Strlen( *fptr );
			if( !(*tptr = (unsigned char *)Malloc( size + 1 ) ) ) {
				TLf_free( to );
				return( TLNOMEMORY );
			}
			Strcpy( *tptr, *fptr );
		}
	}
	return( TLOK );
}

int
TLf_find( fields, name )
field_t	*fields;
unsigned char *name;
{
	register i;
	register unsigned char **nptr = fields->values;
	if( name && *name ) 
		for( i = 0; i < fields->count; i++, nptr++ )
			if( nptr && !Strcmp( name, *nptr ) )
				return( i );
	return( -1 );
}

int
TLf_compare( p, q )
field_t	*p, *q;
{
	register i;
	if( p->count != q->count ) return( TRUE );
	for( i = 0; i < p->count; i++ )
		if( Strcmp( p->values[i], q->values[i] ) ) return( TRUE );
	return( FALSE );
}

/* How many characters in this entry? */
/*ARGSUSED*/
static
TLf_toobig( tid, entry, iscomment, offset, size )
int tid, iscomment, offset, size;
entry_t *entry;
{
	register i, nfields = E_NFIELDS(entry), count;
	unsigned char *ptr;

	count = 1;	/* end of entry character */

	for( i = 0; i < nfields; i++ )
		/* Don't count the entry that is being replaced */
		if( !iscomment && offset == i ) continue;
		else {
			ptr = (unsigned char *)*E_GETFIELD( entry, i );
			count += Strlen( ptr );
		}

	if( nfields > 1 )
		/* Count field separators */
		count += nfields - 1;

	if( !iscomment ) count += Strlen( E_COMMENT(entry) );

	if( E_COMMENT(entry) ) count += 1;	/* Comment character */

	count += size;
	return( count > TL_MAXLINESZ );
}

int
TLf_badsubst( tid, string, iscomment )
int tid, iscomment;
unsigned char *string;
{
	register TLdesc_t *desc = &(TLtables[tid].description);
	register i = 0;
	char *strpbrk();
	unsigned char buffer[ 5 ];
	if( desc->td_fs != '\0' && !iscomment )
		buffer[ i++ ] = desc->td_fs;
	if( desc->td_eoe != '\0' )
		buffer[ i++ ] = desc->td_eoe;
	if( desc->td_comment != '\0' && !iscomment )
		buffer[ i++ ] = desc->td_comment;
	buffer[ i ] = '\0';
	return( (int)strpbrk( (char *)string, (char *)buffer ) );
}

TLf_assign( tid, entry, fieldname, value )
int tid;
entry_t *entry;
unsigned char *fieldname, *value;
{
	register tbl_t *tptr = TLtables + tid;
	register offset = 0, iscomment = 0, v_size;
	register unsigned char *ptr;

	if( !Strcmp( fieldname, TLCOMMENT ) ) {
		if( !E_NFIELDS(entry) ) iscomment = TRUE;
		else return( TLFAILED );
	} else if( !Strcmp( fieldname, TLTRAILING ) )
		iscomment = TRUE;

	/* XXX - remove
	if( TLf_badsubst( tid, value, iscomment ) ) return( TLSUBSTITUTION );
	*/

	if( !iscomment
		&& (offset = TLf_find( &(tptr->fieldnames), fieldname )) == -1 )
		return( TLBADFIELD );

	v_size = Strlen( value );

	if( TLf_toobig( tid, entry, iscomment, offset, v_size ) ) return( TLTOOLONG );

	if( offset >= E_NFIELDS(entry) && !iscomment ) 
		E_NFIELDS(entry) = offset + 1;

	if( value ) {
		if( !(ptr = (unsigned char *)Malloc( v_size + 1 ) ) )
			return( TLNOMEMORY );

		Strcpy( ptr, value );
	} else ptr = NULL;

	if( iscomment ) {
		if( E_COMMENT(entry) ) Free( E_COMMENT(entry) );
		E_COMMENT(entry) = ptr;
	} else {
		if( E_VALUES(entry)[offset] ) Free( E_VALUES(entry)[offset] );
		E_VALUES(entry)[offset] = ptr;
	}
	return( TLOK );
}

/* 
	Write out a field entry escaping special characters
*/
TLf_write( fd, field, fs, eoe, comment )
int fd;
unsigned char *field, fs, eoe, comment;
{
	register unsigned char *ptr, *to;
	register nspecial = 0, size = 0, rc = TLOK;

	/* Figure size and number of special characters */
	for( ptr = field; *ptr; ptr++ ) {

		if( *ptr == '\\' || *ptr == fs 
			|| *ptr == eoe || *ptr == comment )
			nspecial++;

		size++;
	}

	if( nspecial ) {

		/* Calculate new size of field */
		size += nspecial;
		ptr = field;

		if( !(field = to = (unsigned char *)Malloc( size + 1 ) ) )
			return( TLFAILED );

		/* Copy field, escaping special characters */

		while( *ptr ) {
			if( *ptr == '\\' || *ptr == fs 
				|| *ptr == eoe || *ptr == '#' )
				*to++ = '\\';

			*to++ = *ptr++;
		}
		*to = '\0';

	}

	if( size && Write( fd, field, size ) != size )
		rc = TLFAILED;

	if( nspecial )
 		Free( field );
	
	return( rc );
}
