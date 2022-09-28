/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rslib.d/rstm.c	1.5.2.1"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <table.h>
#include <bktypes.h>
#include <bkhist.h>
#include <brtoc.h>
#include <restore.h>
#include <rsstrat.h>
#include <rsmethod.h>
#include <rstm.h>

extern char *rss_start();
extern char *rss_move();
extern argv_t *s_to_argv();
extern char *argv_to_s();
extern char *strdup();
extern int rss_parse();
extern long strtol();
extern long atol();
extern int strcmp();
extern unsigned int strlen();
extern int rss_mfind();
extern int stat();
extern void free();
extern int rss_done();
extern void *malloc();
extern void argv_free();

char *rstm_vlabel();
int rstm_varch();

#ifndef TRUE
#define	TRUE 1
#define	FALSE 0
#endif

/* Does this archive have an offline table of contents? */
#define	OFFLINE_TOC( tid, entry, toc ) \
	((toc = (char *)TLgetfield( tid, entry, H_TMNAME )) && *toc )

int
rstm_init()
{
	/* XXX always parse files now - eventually keep parsed version around */
	return( rss_parse() );
}

/* Return TRUE iff t_date >= date */
static int
rstm_dfcmp( fieldname, t_date_p, date )
unsigned char *fieldname, *t_date_p;
time_t *date;
{
	time_t t_date;

	if( !t_date_p || !*t_date_p )
		/* Ignore if the entry has no date */
		return( FALSE );

	t_date = strtol( (char *) t_date_p, (char **)0, 16 );
	return( t_date >= *date );
}

/* Return TRUE iff t_date <= date */
static int
rstm_dccmp( fieldname, t_date_p, date )
unsigned char *fieldname, *t_date_p;
time_t *date;
{
	time_t t_date;

	if( !t_date_p || !*t_date_p )
		/* Ignore if the entry has no date */
		return( FALSE );

	t_date = strtol( (char *) t_date_p, (char **)0, 16 );
	return( t_date <= *date );
}

/* Find oldest entry that is <= date, or newest >= date */
int
rstm_dfind( tid, date, floor )
int tid, floor;
time_t date;
{
	TLsearch_t sarray[ 2 ];

	/* Now find out the entryno of the entry just written */
	sarray[ 0 ].ts_fieldname = H_DATE;
	sarray[ 0 ].ts_pattern = (unsigned char *)&date;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	if( floor ) {
		sarray[ 0 ].ts_operation = (int (*)()) rstm_dfcmp;
		return( TLsearch1( tid, sarray, TLBEGIN, TLEND, TL_AND ) );
	}

	sarray[ 0 ].ts_operation = (int (*)()) rstm_dccmp;
	return( TLsearch1( tid, sarray, TLEND, TLBEGIN, TL_AND ) );
}

/* Is this oname/odevice consistent with this request? */
int
rstm_consistent( rqst, oname, odevice )
rs_rqst_t *rqst;
char *oname, *odevice;
{
	if( !oname || !odevice )
		return( FALSE );

	if( !strcmp( rqst->type, R_PARTITION_TYPE )
		|| !strcmp( rqst->type, R_DISK_TYPE ) )

		/* For these object types, compare to ODEVICE. */
		return( !strcmp( rqst->odev, odevice ) );

	/* For these object types, compare to ONAME. */
	return( !strcmp( rqst->oname, oname ) );
}

/* return the direction from the state */
static int
rstm_direction( state )
char *state;
{
	register s_size;
	
	if( !*state || (s_size = strlen( state )) < 3 )
		return( FALSE );

	switch( state[ s_size - 1 ] ) {
	case '<':
		return( RSTM_BACKWARD );
		/*NOTREACHED*/
		break;

	case '>':
		return( RSTM_FORWARD );
		/*NOTREACHED*/
		break;

	case 's':
	case 'S':
		return( RSTM_STOP );
		/*NOTREACHED*/
		break;

	default:
		return( 0 );
		/*NOTREACHED*/
		break;
	}
}

/*
	Get the next stimulus. The possibilities are:
	- 0: something went wrong
	- RSTM_BEGIN: found beginning of history table
	- RSTM_END:	found end of history table
	- RSTM_RSDATE: found restore request date
	- RSTM_FARCHIVE: found a full-type candidate archive
	- RSTM_PARCHIVE: found a partial-type candidate archive

	The precedence rules for first stimulus: (tmstate == NULL):
	- RSTM_FARCHIVE > RSTM_RSDATE;
	- RSTM_PARCHIVE > RSTM_RSDATE;

	The precedence rules for NON-first stimulus:
	- RSTM_RSDATE > RSTM_BEGIN;
	- RSTM_RSDATE > RSTM_END;
	- RSTM_FARCHIVE > RSTM_RSDATE;	 see note1
	- RSTM_PARCHIVE > RSTM_RSDATE;	 see note1

	note1: in these cases, the first call returns RSTM_FARCHIVE or
	RSTM_PARCHIVE, the *next* call returns RSTM_RSDATE.  This is done
	by leaving the tmdate == archive_date.  This is noticed the next time
	through.
*/
static int
rstm_getstimulus( h_tid, h_entry, rqst )
int h_tid;
ENTRY h_entry;
rs_rqst_t *rqst;
{
	register h_mtype, h_entryno, direction, at_end;
	char *h_date_p;
	time_t h_date;

	/* First time through */
	if( !rqst->tmstate ) {

		h_entryno = rstm_dceiling( h_tid, rqst->date );
		if( h_entryno <= 0 )
			return( 0 );

		if( TLread( h_tid, h_entryno, h_entry ) != TLOK )
			/* history table must've just been emptied */
			return( 0 );

		h_date_p = (char *)TLgetfield( h_tid, h_entry, H_DATE );
		if( !h_date_p || !*h_date_p )
			h_date = (time_t) 0;
		else h_date = strtol( (char *) h_date_p, (char **)0, 16 );

		rqst->tmdate = rqst->date;
		if( h_date == rqst->date ) {
			if( IS_CANDIDATE( h_tid, h_entry, rqst, h_mtype ) )
				/* request date falls on a candidate */
				return( h_mtype == RSTM_FULL? RSTM_FARCHIVE: RSTM_PARCHIVE );
		}
		return( RSTM_RSDATE );
	}

	/* What direction are we going? */
	if( !(direction = rstm_direction( rqst->tmstate )) || direction == RSTM_STOP )
		return( 0 );

	/* start at entry nearest tmdate */
	h_entryno = ( direction == RSTM_FORWARD )?
		rstm_dceiling( h_tid, rqst->tmdate ): rstm_dfloor( h_tid, rqst->tmdate );

	if( at_end = (h_entryno <= 0 ) )
		/* Assume that we're at end of history - if not, TLread() will catch */
		h_entryno = direction == RSTM_FORWARD? 1: TLEND;

	if( TLread( h_tid, h_entryno, h_entry ) != TLOK )
		return( 0 );

	h_date_p = (char *)TLgetfield( h_tid, h_entry, H_DATE );
	if( !h_date_p || !*h_date_p )
		h_date = (time_t) 0;
	else h_date = strtol( (char *) h_date_p, (char **)0, 16 );

	if( h_date == rqst->tmdate
		&& rqst->tmdate == rqst->date
		&& IS_CANDIDATE( h_tid, h_entry, rqst, h_mtype ) ) {
		/*
			Must've returned an ARCHIVE last time;
			reset tmdate so that this case isn't hit next time.
		*/
		rqst->tmdate = ( direction == RSTM_FORWARD )? h_date + 1: h_date - 1;
		return( RSTM_RSDATE );
	} else if( at_end 
		&& IS_CANDIDATE( h_tid, h_entry, rqst, h_mtype ) ) {
		/*
			Special case to handle possibility that:
			1) tmdate is beyond table
			2) endpoint in table is CANDIDATE
		*/
		rqst->tmdate = h_date;
		return( h_mtype == RSTM_FULL? RSTM_FARCHIVE: RSTM_PARCHIVE );
	}

	/* Loop through remainder of history looking for RSDATE or ARCHIVE */
	while( TRUE ) {

		h_entryno = (direction == RSTM_FORWARD)? h_entryno + 1: h_entryno - 1;

		switch( TLread( h_tid, h_entryno, h_entry ) ) {
		case TLOK:
			break;

		case TLBADENTRY:
			/* hit end of history table */

			/* Assert: h_date has NEWEST or OLDEST date in history table */
			if( direction == RSTM_FORWARD ) {
				/* RSTM_RSDATE has precedence over RSTM_END */
				/*if( rqst->date > h_date ) {
					rqst->tmdate = rqst->date;
					return( RSTM_RSDATE );
				}*/
				rqst->tmdate = h_date + 1;
				return( RSTM_END );
			} else {
				/* RSTM_RSDATE has precedence over RSTM_BEGIN */
				/*if( rqst->date < h_date ) {
					rqst->tmdate = rqst->date;
					return( RSTM_RSDATE );
				}*/
				rqst->tmdate = h_date - 1;
				return( RSTM_BEGIN );
			}
			/*NOTREACHED*/
			break;

		default:
			return( 0 );
			/*NOTREACHED*/
			break;
		}

		/* First look for RSDATE *up to* next archive date */
		h_date_p = (char *)TLgetfield( h_tid, h_entry, H_DATE );

		if( !h_date_p || !*h_date_p )
			/* Comment entry */
			continue;

		/* NOTE: h_date is only set for valid dates */
		h_date = strtol( (char *) h_date_p, (char **)0, 16 );

		if( direction == RSTM_FORWARD ) {
			if( rqst->tmdate < rqst->date && rqst->date < h_date ) {
				rqst->tmdate = rqst->date;
				return( RSTM_RSDATE );
			}
		} else {
			if( rqst->tmdate > rqst->date && rqst->date > h_date ) {
				rqst->tmdate = rqst->date;
				return( RSTM_RSDATE );
			}
		}

		/* Now look for CANDIDATE ARCHIVE */
		if( IS_CANDIDATE( h_tid, h_entry, rqst, h_mtype ) ) {
			rqst->tmdate = h_date;
			return( h_mtype == RSTM_FULL? RSTM_FARCHIVE: RSTM_PARCHIVE );
		}

		/* Lastly, check RSTM_RSDATE *on* date of next archive */
		if( h_date == rqst->date ) {
			rqst->tmdate = rqst->date;
			return( RSTM_RSDATE );
		}
	}
}

#ifdef TRACE
char *
pr_stim( stimulus )
int stimulus;
{
	static char buffer[ 20 ];
	switch( stimulus ) {
	case RSTM_RSDATE: return( "RSDATE" );
	case RSTM_END:	return( "END" );
	case RSTM_BEGIN:	return( "BEGIN" );
	case RSTM_FARCHIVE:	return( "FULL_ARCHIVE" );
	case RSTM_PARCHIVE:	return( "PART_ARCHIVE" );
	case RSTM_OPTFARCHIVE:	return( "OPT_FULL_ARCHIVE" );
	case RSTM_OPTPARCHIVE:	return( "OPT_PART_ARCHIVE" );
	default:
		(void) sprintf( buffer, "UNKNOWN(%d)", stimulus );
		return( buffer );
	}
}
#endif

/* get the next candidate archive in the history table */
int
rstm_move( h_tid, h_entry, rqst, succeeded )
int h_tid, succeeded;
ENTRY h_entry;
rs_rqst_t *rqst;
{
	register stimulus;
	int action;

#ifdef TRACE
	if( rqst->tmstate )
		brlog( "rstm_move(): state: %s date 0x%lx", rqst->tmstate,
			rqst->tmdate );
#endif

	/* If a successful archive, get next state */
	if( rqst->tmstate ) {
		/* Retreive last stimulus */
		stimulus = rqst->tmstimulus;

		if( succeeded ) {
			if( !(rqst->tmstate = rss_move( rqst->tmstate, stimulus, &action,
				rqst->type ) ) ) {
				/* no more transitions - stop */
#ifdef TRACE
				brlog( "rstm_move(): no more transitions" );
#endif
				return( FALSE );
			}
		} else {
			char *tmpstate;
			int tmpaction, newstimulus;
			/*
				This archive did not succeed, check to see if we 
				should change states anyway.
			*/
			newstimulus = (stimulus == RSTM_FARCHIVE)?
				RSTM_OPTFARCHIVE: RSTM_OPTPARCHIVE;

			if( tmpstate = rss_move( rqst->tmstate, newstimulus, &tmpaction,
				rqst->type ) ) {
				/* Yes - then fill in rqst */
				rqst->tmstate = tmpstate;
				action = tmpaction;
			}
		}
	} 

	/* get next stimulus */
	if( !(stimulus = rstm_getstimulus( h_tid, h_entry, rqst )) ) {

#ifdef TRACE
		brlog( "rstm_move(): no more stimulae" );
#endif

		return( FALSE );
	}

	/* if this is first time, get start state */
	if( !rqst->tmstate )
		if( !( rqst->tmstate = rss_start( rqst->type ) ) )
			return( FALSE );

#ifdef TRACE
	brlog( "rstm_move(): stimulus: %s next state %s",
		pr_stim( stimulus ), rqst->tmstate );
#endif

	while( TRUE ) {

#ifdef TRACE
		brlog( "rstm_move(): state: %s date: 0x%lx stimulus: %s",
			rqst->tmstate, rqst->tmdate, pr_stim( stimulus ) );
#endif

		switch( stimulus ) {
		case RSTM_FARCHIVE:
		case RSTM_PARCHIVE:
			/* Must look at next transition to check action! */
			(void) rss_move( rqst->tmstate, stimulus, &action, rqst->type );
			if( action == RSTM_ACCEPT ) {
				rqst->tmstimulus = stimulus;
				return( TRUE );
			}
#ifdef TRACE
			brlog( "rstm_move(): ignore a %s", pr_stim( stimulus ) );
#endif
			break;

		case RSTM_END:
		case RSTM_BEGIN:
		case RSTM_RSDATE:
			/* keep going */
			break;

		default:
			/* No more stimulae - stop */
			return( FALSE );
			/*NOTREACHED*/
			break;
		}
		if( !(rqst->tmstate = rss_move( rqst->tmstate, stimulus, &action,
			rqst->type ) ) ) {
			/* no more transitions - stop */
#ifdef TRACE
			brlog( "rstm_move(): no more transitions" );
#endif
			return( FALSE );
		}
#ifdef TRACE
		brlog( "rstm_move(): next state: %s", rqst->tmstate );
#endif

		/* get next stimulus */
		if( !(stimulus = rstm_getstimulus( h_tid, h_entry, rqst )) ) {

#ifdef TRACE
			brlog( "rstm_move(): No more stimulae" );
#endif 

			return( FALSE );
		}
	}
}

/* Is this object in this archive? return media names */
static char *
in_archive( rqst, toc )
rs_rqst_t *rqst;
char *toc;
{
	int t_tid, rc, t_entryno;
	char *ptr;
	ENTRY t_entry;
	TLdesc_t descr;
	TLsearch_t sarray[ 2 ];

	/* First open the file */
	descr.td_fs = descr.td_eoe = descr.td_comment = (unsigned char)'\0';
	descr.td_format = TOC_ENTRY_F;

	if( (rc = TLopen( &t_tid, toc, &descr, O_RDONLY, 0644 ) ) != TLOK
		&& rc != TLBADFS && rc != TLDIFFFORMAT ) 
		return( (char *) 0 );

	
	sarray[ 0 ].ts_fieldname = TOC_FNAME;
	sarray[ 0 ].ts_pattern = (unsigned char *)rqst->object;
	sarray[ 0 ].ts_operation = (int (*)())TLEQ;
	sarray[ 1 ].ts_fieldname = (unsigned char *)0;

	t_entryno = TLsearch1( t_tid, sarray, TLBEGIN, TLEND, TL_AND );

	if( t_entryno == TLFAILED )
		return( (char *) 0 );

	if( t_entryno < 0 )
		return( "" );

	/* Get an entry element */
	if( !(t_entry = TLgetentry( t_tid )) )
		return( "" );

	if( TLread( t_tid, t_entryno, t_entry ) != TLOK ) {
		(void) TLfreeentry( t_tid, t_entry );
		return( "" );
	}

	/* Check modification time and change time */
	ptr = (char *) TLgetfield( t_tid, t_entry, TOC_CTIME );

	if( ptr && *ptr && atol( ptr ) > rqst->date ) {
		(void) TLfreeentry( t_tid, t_entry );
		return( (char *)0 );
	}

	ptr = (char *) TLgetfield( t_tid, t_entry, TOC_MTIME );

	if( ptr && *ptr && atol( ptr ) > rqst->date ) {
		(void) TLfreeentry( t_tid, t_entry );
		return( (char *)0 );
	}

	if( (ptr = (char *)TLgetfield( t_tid, t_entry, TOC_INODE )) && *ptr )
		rqst->inode = strtol( ptr, (char **)0, 16 );
	else rqst->inode = 0;

	ptr = strdup( (char *)TLgetfield( t_tid, t_entry, TOC_VOL ) );
	(void) TLfreeentry( t_tid, t_entry );

	return( ptr );
}

/* Is this TOC online? */
static char *
online_toc( tid, eptr )
int tid;
ENTRY eptr;
{
	char *toc;
	struct stat buf;

	if( !(toc = (char *)TLgetfield( tid, eptr, H_TOCNAME ) ) || !*toc )
		return( (char *)0 );

	if( !stat( toc, &buf ) && buf.st_size > 0 )
		return( toc );

	return( (char *)0 );
}

static void
rstm_rmbadlbls( h_tid, h_entry )
int h_tid;
ENTRY h_entry;
{
	char *ptr;
	if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_TMNAME )) && *ptr )
		if( ptr = rstm_vlabel( ptr ) ) {
			(void) TLassign( h_tid, h_entry, H_TMNAME, ptr );
			free( ptr );
		}
	
	if( (ptr = (char *)TLgetfield( h_tid, h_entry, H_DMNAME )) && *ptr ) {
		if( ptr = rstm_vlabel( ptr ) ) {
			(void) TLassign( h_tid, h_entry, H_DMNAME, ptr );
			free( ptr );
		}
	}
}

/* Crank the Turing machine and account for online TOC files */
int
rstm_crank( h_tid, h_entry, rqst, last, total )
int h_tid, last, total;
ENTRY	h_entry;
rs_rqst_t *rqst;
{
	char *toc, *mname;
	int goal;

	/* First look to see if we're done */
	goal = rss_done( rqst->type );

	if( last && (goal == RSTM_ONE) )
		return( RS_COMPLETE );

	if( !rstm_move( h_tid, h_entry, rqst, last ) )
		return( ((goal == RSTM_ALL) && total)? RS_COMPLETE: 0 );

	/* Remove invalid labels from this history entry */
	rstm_rmbadlbls( h_tid, h_entry );

	if( strcmp( rqst->type, R_DIRECTORY_TYPE ) 
		|| strcmp( rqst->type, R_FILE_TYPE ) )
		return( RS_DARCHIVE );

	while( TRUE ) {

		if( toc = online_toc( h_tid, h_entry ) ) {

			if( mname = in_archive( rqst, toc ) ) {
				if( *mname )
					(void) TLassign( h_tid, h_entry, H_DMNAME, mname );

				return( RS_DARCHIVE );

			} else {
				/* Not in this archive - look for the next one */
				if( !rstm_move( h_tid, h_entry, rqst, last ) )
					return( ((goal == RSTM_ALL) && total)? RS_COMPLETE: 0 );
			}

		} else
			return( OFFLINE_TOC( h_tid, h_entry, toc )? RS_TARCHIVE: RS_DARCHIVE );

	}
	/*NOTREACHED*/
}

/* Delete the invalid labels from a list (returns malloc's memory) */
char *
rstm_vlabel( labels )
char *labels;
{
	register i, j;
	char *lbls, *ptr;
	argv_t *start, *result;

	if( !(lbls = strdup( labels )) )
		return( (char *)0 );

	if( !(start = s_to_argv( lbls, "," ) ) )
		return( (char *)0 );

	for( i = 0; (*start)[i]; i++ )
		;

	if( !(result = (argv_t *)malloc( (i + 1) * sizeof(void *) ) ) )
		return( (char *)0 );

	for( i = j = 0; (*start)[i]; i++ )
		if( *((*start)[i]) != '!' )
			(*result)[j++] = (*start)[i];

	(*result)[j] = (char *)0;

	ptr = argv_to_s( result, ',' );

	free( (void *) result );
	free( (void *) start );

	return( ptr );
}

/* Are there ANY valid labels in this list? */
int
rstm_varch( labels )
char *labels;
{
	register i;
	argv_t *argv;
	char *ptr = strdup( labels );

	if( !(argv = s_to_argv( ptr, "," ) ) ) {
		free( ptr );
		return( FALSE );
	}

	for( i = 0; (*argv)[i]; i++ )
		if( *((*argv)[i]) != '!' ) {
			argv_free( argv );
			free( ptr );
			return( TRUE );
		}

	argv_free( argv );
	free( ptr );
	return( FALSE );
}
