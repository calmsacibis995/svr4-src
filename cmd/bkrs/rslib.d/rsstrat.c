/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rslib.d/rsstrat.c	1.3.2.1"

#include	<fcntl.h>
#include	<time.h>
#include	<errno.h>
#include	<table.h>
#include	<bktypes.h>
#include	<bkerrors.h>
#include	<restore.h>
#include	<rsmethod.h>
#include	<rsstrat.h>
#include	<rstm.h>

#ifndef TRUE
#define TRUE 1
#define	FALSE 0
#endif

#define	CHUNKSZ	5

extern void *malloc();
extern char *strdup(), *bkstrdup();
extern char *rss_find();
extern argv_t *s_to_argv();
extern int strcmp();
extern char *bk_get_rsmethod_path();
extern void brlog();
extern char *brerrno();
extern int rss_minsert();
extern char *bk_get_strat_path();
extern unsigned int strlen();
extern int strncmp();
extern int rss_insert();

static rsstable_t	rsstab[ R_N_TYPES ];

/*
	get a pointer to the table for this type - if the type
	hasn't been gotten before, return a pointer to a new slot
*/
static
rsstable_t *
rss_gettable( type )
char *type;
{
	register i;
	register rsstable_t *table = (rsstable_t *)0;

	if( !type
		|| (strcmp( R_FILE_TYPE, type )
			&& strcmp( R_PARTITION_TYPE, type )
			&& strcmp( R_DIRECTORY_TYPE, type )
			&& strcmp( R_FILESYS_TYPE, type )
			&& strcmp( R_DISK_TYPE, type ) ) )
		
		/* Unknown type */
		return( (rsstable_t *)0 );

	for( i = 0; i < R_N_TYPES; i++ ) {

		/*
			Save pointer to first open slot in case this
			type hasn't been gotten before 
		*/
		if( !(rsstab[i].type) && !table )
			table = rsstab + i;
	
		if( !(strcmp( rsstab[i].type, type ) ) )
			return( rsstab + i );

	}
	
	/* Not found, allocate a new one */
	if( table )
		table->type = strdup( type );

	return( table );
}

static int
rss_stopat( field )
char *field;
{
	if( !strcmp( field, (char *) RSS_ONE ) ) return( RSTM_ONE );
	if( !strcmp( field, (char *) RSS_ALL ) ) return( RSTM_ALL );
	return( 0 );
}

static int
rss_stimulus( field )
char *field;
{
	if( !strcmp( field, (char *) RSS_BEGIN ) )	return( RSTM_BEGIN );
	if( !strcmp( field, (char *) RSS_END ) )	return( RSTM_END );
	if( !strcmp( field, (char *) RSS_ARCHIVEF ) )	return( RSTM_FARCHIVE );
	if( !strcmp( field, (char *) RSS_ARCHIVEP ) )	return( RSTM_PARCHIVE );
	if( !strcmp( field, (char *) RSS_OPT_ARCHIVEF ) )	return( RSTM_OPTFARCHIVE );
	if( !strcmp( field, (char *) RSS_OPT_ARCHIVEP ) )	return( RSTM_OPTPARCHIVE );
	if( !strcmp( field, (char *) RSS_RSDATE ) )	return( RSTM_RSDATE );
	return( 0 );
}

static int
rss_mparse()
{
	register entryno, rc, done = FALSE;
	TLdesc_t descr;
	char *path, *method, *types, *coverage;
	ENTRY eptr;
	int tid;

	/* First open the file */
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = RSM_ENTRY_F;
	path = (char *)bk_get_rsmethod_path();

	if( (rc = TLopen( &tid, path, &descr, O_RDONLY, 0644 ) )
		!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
		if( rc == TLFAILED ) 
			brlog( "rss_mparse(): TLopen of RSmethod table %s fails: %s",
				path, brerrno( errno ) );
		else brlog( "rss_mparse(): TLopen of RSmethod table %s returns %d",
			path, rc );
		return( BKNOFILE );
	}

	/* Get an entry element */
	if( !(eptr = TLgetentry( tid )) ) {
		brlog( "rss_mparse(): unable to get memory to parse strategy table" );
		TLclose( tid );
		return( BKNOMEMORY );
	}

	for( entryno = TLBEGIN; !done; entryno++ ) {
		rc = TLread( tid, entryno, eptr );
		
		switch( rc ) {
		case TLNOMEMORY:
			brlog( "rss_mparse(): no memory to parse strategy table" );
			TLfreeentry( tid, eptr );
			TLclose( tid );
			return( BKNOMEMORY );
			/*NOTREACHED*/
			break;

		case TLBADENTRY:
			done = TRUE;
			break;

		case TLOK:
			break;

		default: 
			brlog( "rss_mparse(): unable to read entry %d in strategy table: %d",
				entryno, rc );
			TLfreeentry( tid, eptr );
			TLclose( tid );
			return( BKINTERNAL );
			/*NOTREACHED*/
			break;
		}

		/* check to see if this is a comment entry */
		if( !(method = (char *)TLgetfield( tid, eptr, RSM_NAME ) ) )
			continue;

		if( !(coverage = (char *)TLgetfield( tid, eptr, RSM_COVERAGE ) ) )
			continue;

		if( !(types = (char *)TLgetfield( tid, eptr, RSM_TYPES ) ) )
			continue;

		/* Record this method */
		if( !(rss_minsert( method,
			(strcmp( coverage, "full" )? RSTM_PART: RSTM_FULL), types ) ) ) {

			brlog( "Unable to process entry %d in strategy table", entryno );
			TLfreeentry( tid, eptr );
			TLclose( tid );
			return( BKINTERNAL );
		}

	}
	TLfreeentry( tid, eptr );
	TLclose( tid );

	return( BKSUCCESS );
}


/* Parse the strategy */
static int
rss_sparse()
{
	register entryno, nentries, error = FALSE, rc, done = FALSE, action;
	TLdesc_t descr;
	char *path, *state, *next_state, *stimulus_p, *stopat_p, *type;
	int stopat, stimulus, stsize, tid;
	ENTRY eptr;
	rsstable_t *table;

	/* First open the file */
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = RSS_ENTRY_F;
	path = (char *)bk_get_strat_path();

	if( (rc = TLopen( &tid, path, &descr, O_RDWR, 0644 ) )
		!= TLOK && rc != TLBADFS && rc != TLDIFFFORMAT ) {
		if( rc == TLFAILED ) 
			brlog( "rss_sparse(): TLopen of strategy table %s fails: %s",
				path, brerrno( errno ) );
		else brlog( "rss_sparse(): TLopen of strategy table %s returns %d",
			path, rc );
		return( BKNOFILE );
	}

	/* Get an entry element */
	if( !(eptr = TLgetentry( tid )) ) {
		brlog( "rss_sparse(): unable to get memory to parse strategy table" );
		TLclose( tid );
		return( BKNOMEMORY );
	}

	for( entryno = TLBEGIN, nentries = 0; !done; entryno++ ) {
		rc = TLread( tid, entryno, eptr );
		
		switch( rc ) {
		case TLNOMEMORY:
			brlog( "rss_sparse(): no memory to parse stategy table" );
			TLfreeentry( tid, eptr );
			TLclose( tid );
			return( BKNOMEMORY );
			/*NOTREACHED*/
			break;

		case TLBADENTRY:
			done = TRUE;
			break;

		case TLOK:
			break;

		default: 
			brlog( "rss_sparse(): unable to read entry %d in strategy table: %d",
				entryno, rc );
			TLfreeentry( tid, eptr );
			TLclose( tid );
			return( BKINTERNAL );
			/*NOTREACHED*/
			break;
		}

		/* check to see if this is a comment entry */
		if( !(type = (char *)TLgetfield( tid, eptr, RSS_TYPE ) ) )
			continue;

		if( !(state = (char *)TLgetfield( tid, eptr, RSS_STATE ) ) )
			continue;

		if( !(stimulus_p = (char *)TLgetfield( tid, eptr, RSS_STIMULUS ) ) )
			continue;

		if( !(next_state = (char *)TLgetfield( tid, eptr, RSS_NEXTSTATE ) ) )
			continue;

		stopat_p = (char *)TLgetfield( tid, eptr, RSS_STOPAT );

		/* Check this entry's syntax */
		/* TYPE */
		if( !(table = rss_gettable( type ) ) ) {
			brlog( "strategy table has unknown type \"%s\" at entry %d - ignored",
				type, entryno );
			continue;
		}

		/* STATE */
		stsize = strlen( state );
		if( stsize < 3 || state[ stsize - 2 ] != '.' 
			|| (state[ stsize - 1 ] != '<'
				&& state[ stsize - 1 ] != '>'
				&& state[ stsize - 1 ] != 's' ) ) {

			brlog( "strategy table has bad state name \"%s\" at entry %d - ignored",
				state, entryno );
			continue;
		}

		/* NEXT_STATE */
		stsize = strlen( next_state );
		if( stsize < 3 || next_state[ stsize - 2 ] != '.' 
			|| (next_state[ stsize - 1 ] != '<'
				&& next_state[ stsize - 1 ] != '>'
				&& next_state[ stsize - 1 ] != 's' ) ) {

			brlog( "strategy table has bad nextstate name \"%s\" at entry %d - ignored",
				next_state, entryno );
			continue;
		}

		/* ACTION - 'ignore' is indicated by !STIMULUS */
		if( *stimulus_p == '!' ) {
			action = RSTM_IGNORE;
			stimulus_p++;
		} else action = RSTM_ACCEPT;

		/* STIMULUS */
		if( !(stimulus = rss_stimulus( stimulus_p ) ) ) {
			brlog( "strategy table has bad stimulus field \"%s\" at entry %d - ignored",
				stimulus_p, entryno );
			continue;
		}

		/* STOPAT */
		if( stopat_p && *stopat_p && !(stopat = rss_stopat( stopat_p ) ) ) {
			brlog( "strategy table has bad stopat field \"%s\" at entry %d - ignored",
				stopat_p, entryno );
			continue;
		}

		/* insert entry into table */
		if( stopat && !table->stopat )
				table->stopat = stopat;

		if( !strncmp( state, "start", 5 ) )
			if( table->start && strcmp( table->start, state ) ) {
				brlog(
					"strategy table has >1 start state indicated for type \"%s\"",
					type );
				error = TRUE;
			} else table->start = strdup( state );

		if( rc = rss_insert( state, stimulus, action, next_state, table ) ) {
			brlog(
				"unable to insert strategy table entry number %d into internal table (%d)",
				rc );
			TLfreeentry( tid, eptr );
			TLclose( tid );
			return( rc );
		}

		nentries++;
	}

	TLfreeentry( tid, eptr );
	TLclose( tid );

	if( !nentries ) {
		brlog( "no entries found in strategy table %s", path );
		return( BKBADTABLE );
	}

	/* check for start states */
	table = rss_gettable( R_FILE_TYPE );
	if( !table->start ) {
		brlog( "strategy table has no start state for type \"%s\"",
			R_FILE_TYPE );
		error = TRUE;
	}

	table = rss_gettable( R_PARTITION_TYPE );
	if( !table->start ) {
		brlog( "strategy table has no start state for type \"%s\"",
			R_PARTITION_TYPE );
		error = TRUE;
	}

	table = rss_gettable( R_DIRECTORY_TYPE );
	if( !table->start ) {
		brlog( "strategy table has no start state for type \"%s\"",
			R_DIRECTORY_TYPE );
		error = TRUE;
	}

	table = rss_gettable( R_FILESYS_TYPE );
	if( !table->start ) {
		brlog( "strategy table has no start state for type \"%s\"",
			R_FILESYS_TYPE );
		error = TRUE;
	}

	table = rss_gettable( R_DISK_TYPE );
	if( !table->start ) {
		brlog( "strategy table has no start state for type \"%s\"",
			R_DISK_TYPE );
		error = TRUE;
	}

	return( error? BKBADTABLE: BKSUCCESS );
}

/* Parse strategy and rsmethods tables */
int
rss_parse()
{
	register rc;
	if( rc = rss_sparse() ) return( rc );
	return( rss_mparse() );
}

/* Return the start state for a particular type */
char *
rss_start( type )
char *type;
{
	rsstable_t *table;

	if( !(table = rss_gettable( type ) ) )
		return( (char *)0 );

	return( table->start );
}

/* Return the next transition in the state table */
char *
rss_move( state, stimulus, action, type )
char *state, *type;
int stimulus, *action;
{
	rsstable_t *table;

	if( !(table = rss_gettable( type ) ) )
		return( (char *)0 );
	return( rss_find( state, stimulus, action, table ) );
}

/* Are we done yet? */
int
rss_done( type )
char *type;
{
	rsstable_t *table;

	if( !(table = rss_gettable( type ) ) )
		return( 0 );

	return( table->stopat );
}
