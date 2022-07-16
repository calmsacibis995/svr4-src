/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLgetfield.c	1.3.3.1"
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

unsigned char *
TLgetfield( tid, entry, fieldname )
int tid;
entry_t *entry;
unsigned char *fieldname;
{
	register tbl_t *tptr;
	register offset;
	/* Initialize TLlib, if needed */
	TLinit();

	tid--;
	if( !TLt_valid( tid ) )	return( NULL );
	if( !entry || !fieldname ) return( NULL );

	tptr = TLtables + tid;

	if( !Strcmp( fieldname, TLCOMMENT ) )
		return( E_NFIELDS(entry)? NULL: E_COMMENT(entry) );

	if( !Strcmp( fieldname, TLTRAILING ) )
		return( (E_NFIELDS(entry) == 0)? NULL: E_COMMENT(entry) );

	if( (offset = TLf_find( &(tptr->fieldnames), fieldname )) == -1 )
		return( NULL );
	if( offset > E_NFIELDS(entry) ) return( NULL );

	return( (unsigned char *)*E_GETFIELD( entry, offset ) );
}
