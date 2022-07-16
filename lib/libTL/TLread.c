/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLread.c	1.3.3.1"
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

int
TLread( tid, entryno, entry )
int tid;
entryno_t entryno;
entry_t	*entry;
{
	entry_t *eptr;
	register rc;
	void TLe_free();
	/* Initialize TLlib, if needed */
	TLinit();

	tid--;
	if( !TLt_valid( tid ) )	return( TLBADID );

	if( !entry ) return( TLARGS );
	else TLe_free( entry );

	if( T_EMPTY(tid) || entryno < 0 || entryno > TLEND )
		return( TLBADENTRY );
	else if( entryno == TLBEGIN )
		entryno = 1;

	/*
		TLe_find() insures that the entry, it it exists, is parsed and
		in main memory.
	*/
 	if( (rc = TLe_find( tid, entryno ) ) ) return( rc );

	/* If the entryno is relative to TLEND, calculate the exact entryno */
	if( IS_FROM_END(entryno) )
		entryno = TLe_relative( tid, entryno );

	/* Now, get a pointer to the entry */
	if( (rc = TLe_getentry( tid, entryno, &eptr ) ) != TLOK ) return( rc );

	/* Copy it out */
	rc = TLe_copy( entry, eptr );

	return( rc );
}
