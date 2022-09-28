/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rslib.d/rstree.c	1.2.2.1"

#include	<search.h>
#include	<bktypes.h>
#include	<bkerrors.h>
#include	<rstm.h>

/* This file contains routines pertaining to binary tree functions */

#ifndef TRUE
#define	TRUE 1
#define	FALSE 0
#endif

static char *rstm_mtree;	/* parsed rsmethod.tab */

extern argv_t *s_to_argv();
extern void *malloc();
extern char *bkstrdup();
extern unsigned int strlen();
extern void argv_free();
extern int sprintf();
extern void free();
extern int bkstrcmp();

/* compare two transitions */
static int
rss_cmp( a, b )
rsstrans_t *a, *b;
{
	register rc;

	if( !(rc = bkstrcmp( a->state, b->state )) )
		return( b->stimulus - a->stimulus );
	return( rc );
}

/* Insert a new transition in a table - ignore duplicates */
int
rss_insert( state, stimulus, action, next_state, table )
char *state, *next_state;
int stimulus, action;
rsstable_t *table;
{
	rsstrans_t *trans;

	if( !(trans = (rsstrans_t *)malloc( sizeof( rsstrans_t ))) )
		return( BKNOMEMORY );

	trans->state = (char *) bkstrdup( state );
	trans->stimulus = stimulus;
	trans->action = action;
	trans->next_state = (char *) bkstrdup( next_state );

#ifdef BOZO
	brlog( "insert transition: (%s,%d) -> %s", trans->state, trans->stimulus,
		trans->next_state );
#endif

	(void)tsearch( (char *)trans, (void **) &(table->rootp),
		(int (*)())rss_cmp );

	return( BKSUCCESS );
}

#ifdef BOZO
void
print_trans( trans, order, level )
rsstrans_t **trans;
VISIT order;
int level;
{
	if( order == preorder || order == leaf ) {
		brlog( "transition: (\"%s\",%d) -> \"%s\" level %d",
			(*trans)->state, (*trans)->stimulus, (*trans)->next_state,
			level );
	}
}
#endif

/* Find the appropriate transition in the table and return the next state */
char *
rss_find( state, stimulus, action, table )
char *state;
int stimulus, *action;
rsstable_t *table;
{
	rsstrans_t trans, **result;
#ifdef BOZO
	void print_trans();
#endif

	trans.state = state;
	trans.stimulus = stimulus;

#ifdef BOZO
	twalk( (char *)table->rootp, print_trans );
#endif

	result = (rsstrans_t **)tfind( (char *)&trans, (void **)&(table->rootp),
		(int (*)())rss_cmp );

	if( !result ) return( (char *)0 );
	*action = (*result)->action;
	return( (*result)->next_state );
}

/* Insert information about methods and its acceptable types into a table */
int
rss_minsert( method, coverage, types )
char *method, *types;
int coverage;
{
	register msize, i;
	argv_t *argv;
	char *type, *element;

	if( !method || !types || !*method || !*types ) 
		return( FALSE );

	/* Break up types into easier form to deal with */
	argv = s_to_argv( types, " 	," );

	msize = strlen( method );

	/* The idea is to store things as <method>.<type>.<f_or_p> in the tree */
	for( i = 0; type = (*argv)[i]; i++ ) {
		if( !*type ) continue;
		if( !(element = (char *)malloc( (unsigned int) msize + strlen( type ) + 4 )) ) {
			argv_free( argv );
			return( FALSE );
		}
		(void) sprintf( element, "%s.%s.%c", method, type,
			(coverage == RSTM_FULL? 'f': 'p') );
		(void*)tsearch( (char *)element, (void **)&rstm_mtree, bkstrcmp );
	}

	argv_free( argv );

	return( TRUE );
}

#ifdef BOZO
void
print_method( method, order, level )
char **method;
VISIT order;
int level;
{
	if( order == preorder || order == leaf ) {
		brlog( "method: %s", *method );
	}
}
#endif

/* Does a particular method support a particular type? */
int
rss_mfind( method, type )
char *method, *type;
{
	char *element, *result;
	int nchars;
#ifdef BOZO
	void print_method();
#endif

	if( !method || !*method )
		return( FALSE );

	if( !(element = (char *)malloc( (unsigned int) strlen( method )
		+ (unsigned int) strlen( type ) + 4 )) )
		return( FALSE );

	/* First check to see if it is a FULL type archive */
	nchars = sprintf( element, "%s.%s.%c", method, type, 'f' );

#ifdef BOZO
	twalk( (char *)rstm_mtree, print_method );
#endif
	

	result = tfind( element, (void **)&rstm_mtree, bkstrcmp );

	if( result ) {
		free( element );
		return( RSTM_FULL );
	}

	/* Now, check for PARTIAL */
	element[ nchars - 1 ] = 'p';

	result = tfind( element, (void **)&rstm_mtree, bkstrcmp );

	free( element );

	return( result? RSTM_PART: 0 );
}
