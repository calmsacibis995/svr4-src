/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libTL:description.c	1.2.3.1"
#include <table.h>
#include <internal.h>

void
TLd_free( desc )
TLdesc_t *desc;
{
	if( desc->td_format ) {
		Free( desc->td_format );
		desc->td_format = NULL;
	}
}

/*
	Field sep, end of entry, and comment characters must be distinct
	or the description is ambiguous. They cannot be equal to backslash,
	either.
*/
int
TLd_ambiguous( d )
TLdesc_t *d;
{
	if( d->td_fs == '\\' || d->td_eoe == '\\' || d->td_comment == '\\' )
		return( 1 );
	if( d->td_fs ) {
		if( d->td_eoe && d->td_fs == d->td_eoe ) return( 1 );
		if( d->td_comment && d->td_fs == d->td_comment ) return( 1 );
	}
	if( d->td_eoe && d->td_comment )
		if( d->td_eoe == d->td_comment ) return( 1 );
	return( 0 );
}
