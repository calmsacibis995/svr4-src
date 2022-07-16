/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLdelete.c	1.3.3.1"
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

int 
TLdelete( tid, entryno )
int tid;
entryno_t entryno;
{
	register rc;
	/* Initialize TLlib, if needed */
	TLinit();

	tid--;
	if( !TLt_valid( tid ) )	return( TLBADID );

	if( T_EMPTY(tid) || entryno < 0 || entryno > TLEND )
		return( TLBADENTRY );

	/* If entryno is relative to TLEND, calculate the absolute entryno */
	if( IS_FROM_END(entryno) )
		entryno = TLe_relative( tid, entryno );
	else if( entryno == TLBEGIN )
		entryno = 1;

	/*
		TLe_find() insures that the entry, it it exists, is parsed and
		in main memory.
	*/
 	if( (rc = TLe_find( tid, entryno ) ) != TLOK ) return( rc );

	/* Delete the entry */
	rc = TLe_delete( tid, entryno );

	if( rc == TLOK ) TLtables[ tid ].status |= MODIFIED;
	return( rc );
}
