/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:TLopen.c	1.5.3.1"
#include <fcntl.h>
#include <table.h>
#include <internal.h>

extern tbl_t TLtables[];

/*VARARGS4*/
int
TLopen( tid, filename, description, oflag, mode )
int *tid;
unsigned char *filename;
TLdesc_t *description;
int oflag, mode;
{
	register ttid, rc = TLOK;
	TLdesc_t localdesc;
	void TLt_unget();

	/* Initialize TLlib, if needed */
	TLinit();

	/* Argument Checking */
	if( !tid || !filename || !*filename ) return( TLARGS );

	/* Check ambiguity of description */
	if( !description ) {
		description = &localdesc;
		strncpy( description, "", sizeof( TLdesc_t ) );
	} else if( TLd_ambiguous( description ) ) return( TLDESCRIPTION );

	/* Get a table id */
	if( (ttid = TLt_get()) < 0 ) return( TLTOOMANY );

	switch( rc = TLt_open( ttid, filename, description, oflag, mode ) ) {
	case TLBADFS:
	case TLDIFFFORMAT:
	case TLOK:
		break;
	default:
		TLt_unget( ttid );
		return( rc );
	}
	*tid = ttid + 1;
	return( rc );
}
/* ARGSUSED */
