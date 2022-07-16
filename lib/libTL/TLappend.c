/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLappend.c	1.4.3.1"
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

int
TLappend( tid, entryno, entry )
int tid;
entryno_t entryno;
entry_t	*entry;
{
	register rc;
	entry_t	*newentry, *TLe_malloc();

	/* Initialize TLlib, if needed */
	TLinit();

	tid--;
	if( !TLt_valid( tid ) )	return( TLBADID );

	if( !entry ) return( TLARGS );

	if( entryno < 0 || entryno > TLEND )
		return( TLBADENTRY );
	else if( entryno == TLBEGIN ) entryno = 0;

	/*
	else if( !IS_FROM_END(entryno) ) entryno--;
	*/

	if( TLe_diffformat( tid, entry ) ) return( TLDIFFFORMAT );

	/*
		TLe_find() insures that the entry, it it exists, is parsed and
		in main memory.
	*/
 	if( entryno != 0 && (rc = TLe_find( tid, entryno ) ) ) return( rc );

	/* Make our own copy of the entry */
	if( !(newentry = TLe_malloc()) ) return( TLNOMEMORY ); 
	if( (rc = TLe_copy( newentry, entry ) ) != TLOK ) {
		(void)TLfreeentry( tid, newentry );
		return( rc );
	}

	/* If entryno is relative to the end, make that calculation here */
	if( IS_FROM_END(entryno) )
		entryno = TLe_relative( tid, entryno );

	/* Add the entry into the table */
	if( (rc = TLe_add( tid, entryno + 1, newentry ) ) == TLOK )
		TLtables[ tid ].status |= MODIFIED;
	else (void)TLfreeentry( tid, newentry );

	return( rc );
}
