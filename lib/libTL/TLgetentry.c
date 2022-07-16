/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLgetentry.c	1.3.3.1"
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

ENTRY
TLgetentry( tid )
int tid;
{
	entry_t *e_malloc();
	/* Initialize TLlib, if needed */
	TLinit();

	tid--;
	if( !TLt_valid( tid ) )	return( NULL );

	return( (ENTRY)TLe_malloc() );
}

int
TLfreeentry( tid, entry )
entry_t *entry;
{
	void e_free();
	tid--;
	if( !TLt_valid( tid ) ) return( TLBADID );
	if( !entry )	return( TLARGS );
	TLe_free( entry );
	(void) Free( entry );
	return( TLOK );
}
