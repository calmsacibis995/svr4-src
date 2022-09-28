/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/labels.c	1.3.2.1"

#include	<sys/types.h>
#include	<bktypes.h>
#include	<backup.h>
#include	<bkdaemon.h>
#include	<bkerrors.h>

#ifndef	TRUE
#define	TRUE 1
#define	FALSE 0
#endif

extern method_t	*methodtab;
extern int	methodtabsz;
extern char *strdup(), *argv_to_s();
extern argv_t *s_to_argv();
extern int strcmp();
extern void *malloc();
extern void free();
extern void brlog();
extern void argv_free();

/* Is this label in the 'used label' list? */
static int
lbl_used( method, label )
method_t *method;
char *label;
{
	register lbl_t *lbl;
	
	for( lbl = method->used_labels; lbl; lbl = lbl->next )
		if( !strcmp( label, lbl->label ) ) 
			return( TRUE );
	return( FALSE );
}

/*
	If this label does not exist in the method's 
	"used label" list, put it in there.
*/
int
lbl_insert( m_slot, label )
int m_slot;
char *label;
{
	register method_t *method;
	register lbl_t *lbl;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "lbl_insert(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}

	/* If it is already in the list, don't re-insert it */
	if( lbl_used( method, label ) )
		return( BKSUCCESS );

	if( !(lbl = (lbl_t *)malloc( sizeof( lbl_t ) ) ) ) {
		brlog( "lbl_insert(): unable to malloc memory" );
		return( BKNOMEMORY );
	}

	lbl->label = strdup( label );
	lbl->next = method->used_labels;
	method->used_labels = lbl;

	return( BKSUCCESS );
}

/* Free all memory associated with a used label list */
void
lbl_free( m_slot )
int m_slot;
{
	register method_t *method;
	register lbl_t *lbl, *tlbl;

	if( method = MD_SLOT( m_slot ))

		for( lbl = method->used_labels; lbl; lbl = tlbl ) {
			tlbl = lbl->next;
			free( lbl->label );
			free( lbl );
		}

	method->used_labels = (lbl_t *)0;
}

/* optimization - do later
static
lbl_delete( method, label )
method_t *method;
char *label;
{
	register lbl_t *lbl = method->used_labels;
}
*/

/* 
	Cross off the used labels from the list of available
	labels for the method.  This makes the list of labels
	available for the TOC method.
*/
char *
lbl_toclabels( method )
method_t *method;
{
	register argv_t *dlabels;
	register char *ptr;
	register i, j, cnt;

	dlabels = s_to_argv( method->entry.dlabel, "," );

	/* Sequence thru looking for used labels */
	for( cnt = 0; (*dlabels)[cnt]; cnt++ ) {
		if( lbl_used( method, (*dlabels)[cnt] ) ) {
			free( (*dlabels)[cnt] );
			(*dlabels)[cnt] = (char *)0;
		}
	}

	/* Squeeze the holes out of dlabels */
	for( i = j = 0; j < cnt; j++ )
		if ((*dlabels)[j])
			(*dlabels)[i++] = (*dlabels)[j];

	(*dlabels)[i] = (char *)0;

	ptr = argv_to_s( dlabels, ',' );

	argv_free( dlabels );

	return( ptr );
}
