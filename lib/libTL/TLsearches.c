/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLsearches.c	1.2.3.1"
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

int
TLsearch1( tid, sarray, first, last, how_to_match )
int tid, how_to_match;
entryno_t first, last;
TLsearch_t *sarray;
{
	register rc;

	/* Initialize TLlib, if needed */
	TLinit();

	tid--;
	if( !TLt_valid( tid ) )	return( TLBADID );
	if( !sarray ) return( TLARGS );
	if( how_to_match != TL_AND && how_to_match != TL_NAND
		&& how_to_match != TL_OR && how_to_match != TL_NOR )
		return( TLARGS );

	if( T_EMPTY(tid) || first < 0 || first > TLEND || last < 0 || last > TLEND )
		return( TLBADENTRY );

	if( first == TLBEGIN ) first = 1;
	if( last == TLBEGIN ) last = 1;

	/*
		TLe_find() insures that the entry, it it exists, is parsed and
		in main memory.
	*/
 	if( (rc = TLe_find( tid, first ) ) ) return( rc );
 	if( (rc = TLe_find( tid, last ) ) ) return( rc );

	/* If the entryno is relative to TLEND, calculate the exact entryno */
	if( IS_FROM_END(first) )
		first = TLe_relative( tid, first );

	if( IS_FROM_END(last) )
		last = TLe_relative( tid, last );

	return( TLs_search( tid, sarray, first, last, how_to_match ) );
}
