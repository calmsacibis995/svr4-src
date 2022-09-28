/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/entry.c	1.5.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkreg.h>
#include	<table.h>

extern void free();
extern char *strdup();
extern int atoi();
extern void bkr_init();
extern unsigned char *p_weekday1();

/* Free all memory associated with an entry structure */
void
en_free( entry )
brentry_t	*entry;
{
	if( entry->tag ) {
		free( entry->tag );
		entry->tag = NULL;
	}

	if( entry->oname ) {
		free( entry->oname );
		entry->oname = NULL;
	}

	if( entry->odevice ) {
		free( entry->odevice );
		entry->odevice = NULL;
	}

	if( entry->olabel ) {
		free( entry->olabel );
		entry->olabel = NULL;
	}

	if( entry->options ) {
		free( entry->options );
		entry->options = NULL;
	}

	if( entry->dgroup ) {
		free( entry->dgroup );
		entry->dgroup = NULL;
	}

	if( entry->ddevice ) {
		free( entry->ddevice );
		entry->ddevice = NULL;
	}

	if( entry->dependencies ) {
		free( entry->dependencies );
		entry->dependencies = NULL;
	}

	if( entry->dchar ) {
		free( entry->dchar );
		entry->dchar = NULL;
	}
}

static
unsigned char *
en_getfield( tid, eptr, fieldname )
int tid;
ENTRY eptr;
unsigned char *fieldname;
{
	register unsigned char *field;

	field = TLgetfield( tid, eptr, fieldname );
	if( !field || !*field ) 
		return( (unsigned char *)NULL );

	return( (unsigned char *)strdup( (char *)field ) );
}

int
en_parse( tid, eptr, entry )
int tid;
ENTRY	eptr;
brentry_t	*entry;
{
	unsigned char *priority, *week, *day;

#ifdef TRACE
	brlog( "en_parse(): tid %d eptr 0x%x entry 0x%x", tid, eptr, entry );
#endif

	week = (unsigned char *)TLgetfield( tid, eptr, R_WEEK );
	day = (unsigned char *)TLgetfield( tid, eptr, R_DAY );

	if( !week && !day ) return( FALSE );
	bkr_init( entry->date );

	if( !p_weekday1( week, day, entry->date ) )
		return( FALSE );

	priority = en_getfield( tid, eptr, R_PRIORITY );
	if( !(entry->priority = atoi( (char *)priority ) ) )
		entry->priority = DEFAULT_PRIORITY;

	entry->tag = en_getfield( tid, eptr, R_TAG );
	entry->oname = en_getfield( tid, eptr, R_ONAME );
	entry->odevice = en_getfield( tid, eptr, R_ODEVICE );
	entry->olabel = en_getfield( tid, eptr, R_OLABEL );
	entry->options = en_getfield( tid, eptr, R_OPTIONS );
	entry->method = en_getfield( tid, eptr, R_METHOD );
	entry->dgroup = en_getfield( tid, eptr, R_DGROUP );
	entry->ddevice = en_getfield( tid, eptr, R_DDEVICE );
	entry->dchar = en_getfield( tid, eptr, R_DCHAR );
	entry->dlabel = en_getfield( tid, eptr, R_DMNAME );
	entry->dependencies = en_getfield( tid, eptr, R_DEPEND );

	return( TRUE );
}
