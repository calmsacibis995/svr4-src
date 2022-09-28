/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/method.c	1.14.6.1"

#include	<sys/types.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<table.h>
#include	<string.h>
#include	<bkrs.h>
#include	<bktypes.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkerrors.h>
#include	<bkstatus.h>
#include	<errno.h>

#define	MD_BUNCH	25
#define	SEEKWS(p)	while( p && *p && !isspace( *p ) ) p++
#define	SKIPWS(p)	while( p && *p && isspace( *p ) ) p++
#define	N_OPTIONS	10	/* Space enough for all options from command line */

#define md_exit( md_slot ) \
	(void) strncpy( (char *)(methodtab + md_slot), "", sizeof( method_t ) ); \
	methodtab[ md_slot ].state = MD_DONE

/* Free a methods dependency chain */
#define md_d_free( method ) \
{ \
	dep_t *dep, *d_tmp; \
 \
	for( dep = method->deps; dep; dep = d_tmp ) { \
		d_tmp = dep->next; \
		free( dep ); \
	} \
} \

extern	method_t	*methodtab;
extern	int	methodtabsz;
extern	int state;
extern	int nactive;
extern	proc_t	*proctab;
extern	int	proctabsz;
extern	owner_t	*ownertab;
extern	int	ownertabsz;
extern	control_t	*controltab;
extern	int	controltabsz;

extern void ev_resched();
extern char *strdup(), *lbl_toclabels();
extern char *d_getdgrp(), *d_resolve();
extern void *realloc();
extern void *malloc();
extern time_t time();
extern void free();
extern int sprintf();
extern void brlog();
extern int d_allocate();
extern void argv_free();
extern void d_free();
extern int bkspawn();
extern void st_set();
extern int p_findslot();
extern int p_exit();
extern void lbl_free();
extern void en_free();
extern void m_send_failure();
extern int get_period();
extern void bkr_init();
extern void bkr_setdate();
extern void bkr_setdemand();
extern int bk_rotate_start();
extern void st_setfail();
extern int o_deps();
extern char *bk_get_method_path();
extern char *br_get_toc_path();
extern int q_inject();
extern int c_response();
extern void ev_cancel();
extern void ev_wakeup();
extern int q_remove();
extern void m_pr_methods();
extern void o_kill();
extern void o_terminate();
extern int en_parse();
extern int bkr_and();
extern int bkr_empty();
extern void o_incsucceeded();
extern void m_send_mdsuccess();
extern void o_incfailed();
extern void m_send_mdfail();
extern int m_send();
extern char *bld_ddevice();

static int md_l_insert();
static int md_argv();
static int md_toc_argv();
static brentry_t *md_match();
static void	md_freeargv();
static int md_w_delete();
static int md_failure();

/* Actually spawn a method */
int
md_spawn( m_slot )
int	m_slot;
{
	register method_t *method;
	register proc_t	*proc, *oproc;
	register owner_t *owner;
	register pid, rc;
	argv_t *devices = (argv_t *)0;
	bkdata_t msg;

#ifdef TRACE
	brlog( "md_spawn(): spawn m_slot %d", m_slot );
#endif

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "md_spawn(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}

	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "md_spawn(): m_slot %d has bad o_slot %d", m_slot, 
			method->o_slot );
		return( BKINTERNAL );
	}

	if( !(proc = P_SLOT( method->p_slot ) ) ) {
		brlog( "md_spawn(): m_slot %d has bad p_slot %d", m_slot, 
			method->p_slot );
		return( BKINTERNAL );
	}

	/* if no ddevice given - expand dgroup */
	if( !method->entry.ddevice ) {
		if( !method->entry.dgroup
			|| !(devices = (argv_t *)d_getdgrp( method->entry.dgroup ) ) ) {
			brlog( "Unable to expand dgroup: %s", method->entry.dgroup );
			method->state |= MD_NOT_SPAWNED;
			return( BKBADDGROUP );
		}
	}

	/* Allocate devices */
	if( !(owner->options & S_NO_EXECUTE) ) {

		rc = d_allocate( method->entry.odevice, devices, &(method->entry.ddevice) );

		if( devices ) argv_free( devices );

		switch( rc ) {
		case BKINTERNAL:
		case BKNOMEMORY:
			if( !(oproc = P_SLOT( owner->p_slot ) ) ) {
				brlog( "md_spawn(): o_slot *d has bad p_slot %d", 
					method->o_slot );
				return( BKINTERNAL );
			}
			brlog( "Unable to allocate devices for jobid back-%ld tag %s",
				oproc->pid, method->entry.tag );
			return( rc );
		
		case BKBUSY:
			/* try to spawn later */
			if( !(oproc = P_SLOT( owner->p_slot ) ) ) {
				brlog( "md_spawn(): o_slot *d has bad p_slot %d", 
					method->o_slot );
				return( BKINTERNAL );
			}
			brlog( "Devices for jobid back-%ld tag %s are busy, try to spawn it later",
				oproc->pid, method->entry.tag );
			return( rc );

		default:
			method->state |= MD_DEVALLOCED;
			break;
		}
	}

	/* resolve register table dchar vs. device table dchar */
	if( !(method->state & MD_IS_TOC) )
		method->dchar = (unsigned char *)d_resolve(
			method->entry.ddevice, method->entry.dchar );

	/* Fill in the argv structure in the method */
	if( !(method->state & MD_ARGVFILLED) ) {

		rc = ((method->state & MD_IS_TOC)?
			md_toc_argv( method ): md_argv( method ));

		if( rc != BKSUCCESS ) {
			method->state |= MD_NOT_SPAWNED;

			/* De-allocate devices */
			method->state &= ~MD_DEVALLOCED;
			d_free( method->entry.odevice, method->entry.ddevice );
			return( rc );
		}

		method->state |= MD_ARGVFILLED;
	}

	if( (pid = bkspawn( method->argv[0], 0, 0, 0, proc->uid, proc->gid,
		BKARGV, method->argv ) ) == -1 ) {

		brlog( "md_spawn(): unable to spawn m_slot %d: errno %d",
			m_slot, errno );
		method->state |= MD_NOT_SPAWNED;

		/* De-allocate devices */
		method->state &= ~MD_DEVALLOCED;
		d_free( method->entry.odevice, method->entry.ddevice );
		return( BKNOTSPAWNED );
	}
	sprintf(msg.text.text, "%s: starting backup method %s\n",
		method->entry.tag, method->entry.method);
	if (o_send(method->o_slot, TEXT, &msg) != BKSUCCESS)
		brlog ("md_spawn(): unable to send msg to o_slot %d",
			method->o_slot);
	proc->pid = pid;

	if( !(owner->starttime ) )
		owner->starttime = time( 0 );

	if( !(method->state & MD_IS_TOC) )
		method->starttime = time( 0 );

	method->state |= MD_SPAWNED;
	method->state &= ~MD_NOT_SPAWNED;

	if( (owner->options & (S_ESTIMATE|S_NO_EXECUTE))
			!= (S_ESTIMATE|S_NO_EXECUTE) )
		st_setactive( m_slot );

	state |= ACTIVE_METHODS;
	nactive++;
	return( BKSUCCESS );
}

/* Find an unused METHOD slot */
static int
md_findslot()
{
	register i;
	register method_t *method;

	/* Slot 0 is not used */
	if( methodtabsz ) {
		for( i = 1, method = MD_SLOT( 1 ); i < methodtabsz; i++, method++ )
			if( !(method->state & MD_ALLOCATED ) ) {
				method->state |= MD_ALLOCATED;
				return( i );
			}
		/* Didn't find an open slot - grow the table */
		if( !(methodtab = (method_t *)realloc( methodtab,
			(methodtabsz + MD_BUNCH) * sizeof( method_t ) ) ) ) {
			brlog( "Cannot reallocate method table" );
			return( -1 );
		}
		/* Initialize the new portion of the table */
		(void) strncpy( (char *)(methodtab + methodtabsz), "",
			MD_BUNCH * sizeof( method_t ) );

		i = methodtabsz;

	} else {
		/* New table - grow it */
		if( !(methodtab = (method_t *)malloc(
			MD_BUNCH * sizeof( method_t ) ) ) ) {
			brlog( "Cannot allocate method table" );
			return( -1 );
		}
		(void) strncpy( (char *)methodtab, "", MD_BUNCH * sizeof( method_t ) );

		i = 1;
	} 

	methodtab[ i ].state |= MD_ALLOCATED;
	methodtabsz += MD_BUNCH;

	/* Return 1st slot in new portion of table */
	return( i );
}

/* Get a METHOD slot and a PROC slot */
static int
md_alloc()
{
	int md_slot, p_slot;
	register proc_t *proc;
	register method_t *method;

	if( (p_slot = p_findslot( METHOD_P ) ) == -1 ) {
		brlog( "md_alloc(): cannot allocate a process slot" );
		return( -1 );
	}
	if( !( proc = P_SLOT( p_slot ) ) ) {
		brlog( "md_alloc(): p_findslot() returned a bad slot no. (%d)",
			p_slot );
		return( -1 );
	}
	if( (md_slot = md_findslot()) == -1 ) {
		brlog( "md_alloc(): cannot allocate a method slot" );
		(void) p_exit( p_slot );
		return( -1 );
	}
	if( !(method = MD_SLOT( md_slot )) ) {
		brlog( "md_alloc(): md_findslot() returned a bad slot no. (%d)",
			md_slot );
		(void) p_exit( p_slot );
		md_exit( md_slot );
		return( -1 );
	}
	proc->slot = md_slot;
	method->p_slot = p_slot;
	return( md_slot );
}

/* Free a METHOD slot and its associated PROCESS slot */
void
md_free( m_slot )
int m_slot;
{
	register tmp;
	register method_t *method;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "md_free(): given bad slot number to free (%d)", m_slot );
		return;
	}

	if( method->use_count > 0 ) {
		brlog(
			"md_free(): attempt to free method m_slot %d but use count %d",
			m_slot, method->use_count );
		return;
	}
	
	lbl_free( m_slot );	/* free used label list */
	md_freeargv( method );
	md_d_free( method );	/* Free dependency list */
	en_free( &(method->entry) );	/* free memory in the entry structure */
	tmp = method->p_slot;
	md_exit( m_slot );
	(void) p_exit( tmp );
}

/*
	Fill in a owner's private schedule of methods
	--- sends failure messages to owner.
*/
int
md_schedule( o_slot )
int o_slot;
{
	register owner_t *owner;
	register method_t *method;
	int	tid, period, curr_week, curr_day, rc, m_slot, dot;
	int ndeps = 0;
	bkrotate_t date;
	ENTRY eptr;
	register brentry_t *entry;
	TLdesc_t descr;

	if( !(owner = O_SLOT( o_slot )) ) {
		brlog( "md_schedule(): given bad o_slot: %d", o_slot );
		return( BKINTERNAL );
	}
#ifdef TRACE
	brlog( "md_schedule(): owner's p_slot: %d", owner->p_slot );
#endif
	(void) strncpy( (char *)&descr, "", sizeof( TLdesc_t ) );

	if( (rc = TLopen( &tid, owner->table, &descr, O_RDONLY )) != TLOK ) {
		if( rc == TLFAILED )
			brlog( "md_schedule(): TLopen of %s returns errno %d",
				owner->table, errno );
		else brlog( "md_schedule(): TLopen of %s returns error %d",
			owner->table, rc );
		m_send_failure( owner->p_slot, NULL, BKBADREGTAB,
			"Unable to open register table." );
		return( BKBADREGTAB );
	}
#ifdef TRACE
	brlog( "md_schedule(): TLopen returns tid %d", tid );
	brlog( "md_schedule(): owner's p_slot: %d", owner->p_slot );
#endif

	/* Read the rotation period from the bkreg table */
	if( get_period( tid, &period ) ) {
		period = DEFAULT_PERIOD;
	}

	bkr_init( date );
	if( owner->week ) {
		/* Week/day supplied on the backup command line */
		if( owner->week > period ) {
			m_send_failure( owner->p_slot, NULL, BKBADARGS,
				"Out of range week/day given on -c option." );
			return( BKBADARGS );
		}
		/* week in date structure is relative to 0 not 1 */
		bkr_setdate( date, owner->week - 1, owner->day );
	} else if( owner->options & S_DEMAND ) {
		/* Demand backups specified on the backup command line */
		bkr_setdemand( date );
	} else {
		/* Do backups for today */
		if( !bk_rotate_start( tid, period, &curr_week, &curr_day ) ) {
			m_send_failure( owner->p_slot, NULL, BKBADREGTAB,
				"No ROTATION STARTED in register table" );
			return( BKBADREGTAB );
		} 
		/* week in date structure is relative to 0 not 1 */
		bkr_setdate( date, curr_week - 1, curr_day );
	}
	if( !(eptr = TLgetentry( tid ) ) ) return( BKNOMEMORY );

	/* Look for the next entry that satisfies our criteria */
	for( dot = 1;
		(entry = md_match( tid, eptr, &dot, (unsigned char *)owner->fname,
			date ) ) != 0; ) {
		if( !(m_slot = md_alloc() ) == -1 ) {
			brlog(
				"md_schedule(): unable to allocate METHOD/PROC structure" );
			m_send_failure( owner->p_slot, BKINTERNAL, NULL, NULL );
			TLfreeentry( tid, eptr );
			return( BKINTERNAL );
		}
		if( !(method = MD_SLOT( m_slot )) ) {
			brlog( "md_schedule(): md_alloc() allocated bad m_slot %d",
				m_slot );
			return( BKINTERNAL );
		}
		method->entry = *entry;
		method->o_slot = o_slot;
		method->deps = (dep_t *)0;
		if( entry->dependencies )
			ndeps++;

		/* put in private queue in priority order */
		if( ( rc = md_l_insert( o_slot, m_slot ) ) != BKSUCCESS ) {
			st_setfail( m_slot, rc, NULL );
			m_send_failure( owner->p_slot, NULL, rc, NULL );
			TLfreeentry( tid, eptr );
			return( rc );
		} 
	}

	TLfreeentry( tid, eptr );
	(void) TLclose( tid );

	return( ndeps? o_deps( owner ): BKSUCCESS );
}

/*
	Split up the method options field (from bkreg.tab) into an argv, argc
	arrangement.  While doing this, verefy certain of the options (e.g.
	no -R option.
*/
static
md_flagsplit( argv, argc, flags, options )
char *argv[], *flags;
int *argc, *options;
{
	register char *ptr, *tmpptr;
	register size;

	*options = 0;

	/* Split up flags from table entry (options field) */
	for( tmpptr = ptr = flags; *ptr; tmpptr = ptr ) {
		if( *argc > MD_MAXARGS ) {
			brlog(
				"md_flagsplit(): too many arguments in method options field" );
			return( BKTOOMANYARGS );
		}
		SEEKWS(ptr);
		size = ptr - tmpptr;

		if( !(argv[ *argc ] = malloc( (unsigned int)(size + 1) )) ) {
			brlog( "md_argvsplit(): unable to malloc memory" );
			return( BKNOMEMORY );
		}
		(void) strncpy( argv[ *argc ], tmpptr, size );
		argv[ (*argc)++ ][ size ] = '\0';
		SKIPWS(ptr);
	}

	return( BKSUCCESS );
}

/* Fill a TOC method's argv structure */
static int
md_toc_argv( method )
method_t *method;
{
	char jobid[ BKJOBID_SZ + 1 ], *tlabels;
	register owner_t *owner;
	register proc_t *oproc, *mproc;
	register brentry_t *entry = &(method->entry);

#ifdef TRACE
	brlog( "md_toc_argv()" );
#endif

	if( !( owner = O_SLOT( method->o_slot ) ) ) {
		brlog( "md_toc_argv(): bad o_slot %d", method->o_slot );
		method->argv[ 0 ] = NULL;
		return( BKINTERNAL );
	}

	if( !(oproc = P_SLOT( owner->p_slot )) ) {
		brlog( "md_toc_argv(): o_slot %d has bad p_slot %d",
			method->o_slot, owner->p_slot );
		return( BKINTERNAL );
	}

	if( !(mproc = P_SLOT( method->p_slot )) ) {
		brlog( "md_toc_argv(): m_slot %d has bad p_slot %d",
			MD_SLOTNO( method ), owner->p_slot );
		return( BKINTERNAL );
	}

	/* Free argv from previous method */
	md_freeargv( method );

	/* Pathname */
	method->argv[ 0 ] = 
		strdup( (char *)bk_get_method_path( TOC_METHOD ));

	method->argv[ 1 ] = strdup( "-BT" );

	/* Jobid */
	(void) sprintf( jobid, "back-%ld", oproc->pid );
	method->argv[ 2 ] = (char *)strdup( jobid );

	method->argv[ 3 ] = (char *)strdup( (char *)entry->oname );
	method->argv[ 4 ] = (char *)strdup( (char *)entry->odevice );
	method->argv[ 5 ] = (char *)strdup( (char *)entry->olabel );

	/* Descript from previous method */
	tlabels = lbl_toclabels( method );
#ifdef TRACE
	brlog( "md_toc_argv(): lbl_toclables returns >%s<", tlabels );
#endif

	method->argv[ 6 ] = bld_ddevice( (char *)entry->dgroup,
		(char *)entry->ddevice, (char *)entry->dchar, tlabels );

	/* Table of Contents Name */
	method->argv[ 7 ] = strdup( (char *)br_get_toc_path( jobid, mproc->pid ) );

	/* NULL to signify the end */
	method->argv[ 8 ] = NULL;

	return( BKSUCCESS );
}

/*
	Take the flags for the method, plus implied ones from the owner's options
	and produce an argv[] structure.
*/
static int
md_argv( method )
method_t *method;
{
	char *ptr, buffer[ BKJOBID_SZ + 1 ];
	register rc;
	register owner_t *owner;
	register proc_t *oproc;
	register brentry_t *entry = &(method->entry);
	int options = 0, argc = 0;

	if( !( owner = O_SLOT( method->o_slot ) ) ) {
		brlog( "md_argv(): bad o_slot %d", method->o_slot );
		method->argv[ argc ] = NULL;
		return( BKINTERNAL );
	}

	if( !(oproc = P_SLOT( owner->p_slot )) ) {
		brlog( "md_argv(): o_slot %d has bad p_slot %d",
			method->o_slot, owner->p_slot );
		return( BKINTERNAL );
	}

	/* set up commandname (argv[0]) */
	method->argv[ argc++ ]
		= strdup( (char *)bk_get_method_path( entry->method ));

	/* Tell the method that it is a backup method */
	method->argv[ argc++ ] = strdup( "-B" );

	/* Split the method flags into argv form */
	if( ( rc = md_flagsplit( method->argv, &argc, 
		(char *)method->entry.options, &options ) )
		!= BKSUCCESS ) {
		method->argv[ argc ] = NULL;
		return( rc );
	}

	/* OR in the command line options */
	options |= owner->options
		& (S_ESTIMATE|S_NO_EXECUTE|S_SEND_DOTS|S_SEND_FILENAMES);

	/* Now enter command line options */
	if( options ) {
		if( argc > MD_MAXARGS ) {
			brlog( "md_argv(): too many arguments in table %s, tag %s",
				owner->table, entry->tag );
			return( BKTOOMANYARGS );
		}

		if( !(ptr = method->argv[ argc++ ] = malloc( N_OPTIONS + 1 )) ) {
			brlog( "md_argv(): unable to malloc memory" );
			return( BKNOMEMORY );
		}

		*ptr++ = '-';
		if( (options & (S_ESTIMATE|S_NO_EXECUTE)) == (S_ESTIMATE|S_NO_EXECUTE) )
			*ptr++ = 'N';
		else if( options & S_ESTIMATE ) *ptr++ = 'E';
		if( options & S_SEND_DOTS ) *ptr++ = 'S';
		else if( options & S_SEND_FILENAMES ) *ptr++ = 'V';
		*ptr = '\0';
	}

	if( argc + 5 > MD_MAXARGS ) {
		brlog( "md_argv(): too many arguments in table %s, tag %s",
			owner->table, entry->tag );
		return( BKTOOMANYARGS );
	}

	/* Now set up the non-option arguments */
	(void) sprintf( buffer, "back-%d", oproc->pid );
	method->argv[ argc++ ] = (char *)strdup( (char *)buffer );
	method->argv[ argc++ ] = (char *)strdup( (char *)entry->oname );
	method->argv[ argc++ ] = (char *)strdup( (char *)entry->odevice );
	method->argv[ argc++ ] = (char *)strdup( (char *)entry->olabel );

	/* ASSERT: method->dchar contains resolved dchar */
	method->argv[ argc++ ] = bld_ddevice( (char *)entry->dgroup,
		(char *)entry->ddevice, (char *)method->dchar,
		(char *)entry->dlabel );

	/* NULL to signify the end */
	method->argv[ argc++ ] = NULL;

	return( BKSUCCESS );
}

/* Free memory associated with the argv structure */
static
void
md_freeargv( method )
register method_t *method;
{
	register argc;
	for( argc = 0; method->argv[ argc ]; argc++ ) {
		free( method->argv[ argc ] );
		method->argv[ argc ] = NULL;
	}
}

/* Are all the methods for this owner done? */
int
md_flagset( o_slot, flag, which )
int o_slot, flag, which;
{
	register method_t *method;
	register owner_t *owner;
	register m_slot, tmp;
	
	if( !( owner = O_SLOT( o_slot ) ) ) {
		brlog( "md_flagset(): bad o_slot %d", o_slot );
		return( FALSE );
	}
	for( m_slot = owner->methods; m_slot > 0; m_slot = method->next_l ) {
		if( !(method = MD_SLOT( m_slot )) ) {
			brlog(
				"md_flagset(): bad m_slot %d in o_slot %d's private MD sched",
				m_slot, o_slot );
			return( FALSE );
		}

		tmp = (method->state & flag);
		if( which == MD_ALL ) {
			if( !tmp ) return( FALSE );
		} else if( tmp ) return( TRUE );
	}
	return( which == MD_ALL );
}

/*
	Record that a method has terminated
	This routine is entered when the method sends a DONE or FAILED message
	and as a result of a wait(2) system call returning the method's status.
*/
int
md_terminate( m_slot, why, reason, errtext, nblocks )
int m_slot, why, reason, nblocks;
char *errtext;
{
	register rc = BKSUCCESS, print_estimates = FALSE;
	register owner_t *owner;
	register method_t *method;
	register control_t *control;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "md_terminate(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}

	if( !( owner = O_SLOT( method->o_slot ) ) ) {
		brlog( "md_terminate(): m_slot %d has bad o_slot %d",
			m_slot, method->o_slot );
		return( BKINTERNAL );
	}

#ifdef TRACE
	{
		char *whystr;
		switch( why ) {
		case MDT_CANCEL: whystr = "CANCEL"; break;
		case MDT_DONE:	whystr = "DONE"; break;
		case MDT_FAIL: whystr = "FAIL"; break;
		case MDT_EXIT: whystr = "EXIT"; break;
		default: whystr = "UNKNOWN"; break;
		}
		brlog( "md_terminate(): m_slot: %d why %s method->state 0x%x",
			m_slot, whystr, method->state );
	}
#endif

	/* If this method was being controlled, do termination */
	if( method->c_slot ) {
		if( !(control = C_SLOT( method->c_slot )) ) {
			brlog( "md_terminate(): m_slot %d has bad c_slot %d", 
				m_slot, method->c_slot );

		} else if( method->state & MD_CONTROLLING ) {

			/* Process terminated w/o replying to CONTROL message
				- send our own response */
			(void) c_response( m_slot, control->type, TRUE );

			method->state &= ~MD_CONTROLLING;
		}
		method->c_slot = 0;
	}

	/* If the method is sleeping, cancel the event */
	if( method->state & MD_SLEEPING ) {

		/* If this method is waiting for another, 'unwait' it */
		if( method->ev_fcn == ev_resched )
			(void) md_w_delete( m_slot, (int)method->ev_arg );

		/* remove from sleep queues */
		ev_cancel( method->ev_fcn, method->ev_arg );

		method->ev_fcn = (void (*)())0;
		method->ev_arg = 0;
		method->state &= ~MD_SLEEPING;
	}

	/* Wakeup any methods dependent on this one */
	ev_wakeup( ev_resched, (long)m_slot );

	if( method->state & MD_DONE ) return( BKSUCCESS );

	/* Free up the devices */
	if( method->state & MD_DEVALLOCED ) {
		d_free( method->entry.odevice, method->entry.ddevice );
		method->state &= ~MD_DEVALLOCED;
	}

	switch( why ) {
	case MDT_EXIT:

		if( !(method->state & MD_MSG_SENT) )
			if( reason )
				/* Returned from wait */
				rc = md_failure( m_slot, BKERREXIT,
					"Method terminated abnormally");

		/* If the method was on the runqueue, remove it */
		(void) q_remove( m_slot );

		nactive--;
		method->state &= ~MD_SPAWNED;

		print_estimates = (owner->options & (S_ESTIMATE|S_NO_EXECUTE))
			== (S_ESTIMATE|S_NO_EXECUTE);

		if( !print_estimates && (method->state & MD_NEEDTOC) ) {
			/* Re-start this m_slot as a TOC method. */

			method->state |= MD_IS_TOC;
			method->state &= ~(MD_NEEDTOC|MD_ARGVFILLED);

			/* Reschedule the TOC method */
			if( (rc = q_inject( m_slot ) ) == BKSUCCESS )
				return( BKSUCCESS );

			(void) md_failure( m_slot, BKERREXIT, 
				"Unable to write Table of Contents" );

		}

		method->state |= MD_DONE;
		break;

	case MDT_CANCEL:
		/* Method Failed */
		if( !(method->state & MD_MSG_SENT) ) {
			rc = md_failure( m_slot, BKCANCELLED, "backup operation cancelled" );
			method->state |= (MD_DONE|MD_MSG_SENT);
		}
		break;

	case MDT_DONE:
		/* Method terminated normally */
		if( !(owner->options & S_NO_EXECUTE) ) {
			if( method->state & MD_NEEDTOC ) { 

				method->nblocks = nblocks;

			} else if( !(method->state & MD_MSG_SENT) ) {

				if( method->state & MD_IS_TOC ) 
					nblocks += method->nblocks;

				/* Mark this method Successful */
				st_setsuccess( m_slot );
				o_incsucceeded( m_slot );
				m_send_mdsuccess( m_slot, nblocks );

				method->nblocks = nblocks;
				method->state |= MD_MSG_SENT;

			}
		}
		break;

	case MDT_FAIL:
		/* Method Failed */
		if( !(method->state & MD_MSG_SENT) ) {
			if( rc = md_failure( m_slot, reason, errtext ) )
				method->state |= MD_DONE;
			else method->state |= MD_MSG_SENT;
		}
		if( !(method->state & MD_SPAWNED) )
			method->state |= MD_DONE;
		break;

	default:
		break;
	}

	/* Terminate OWNER if all methods are done or the OWNER died */
	if( rc || md_alldone( method->o_slot ) ) {

#ifdef TRACE
		brlog( "md_terminate(): ALL DONE - terminate o_slot %d",
			method->o_slot );
#endif

		/* if -en, then print out the estimates */
		if( print_estimates ) 
			m_pr_methods( method->o_slot, TRUE );

		/* if all done with this owner's backup, then terminate it, too */
		if( rc ) o_kill( method->o_slot );
		o_terminate( method->o_slot );
	}

	return( BKSUCCESS );
}

/*
	Find the next entry that matches the backup date criteria and the 
	(optional) fname criteria
*/
static brentry_t *
md_match( tid, eptr, dot, fname, date )
int tid, *dot;
ENTRY eptr;
unsigned char *fname;
bkrotate_t date;
{
	register entryno, rc;
	static brentry_t entry;
	bkrotate_t tmp;

#ifdef TRACE
	brlog( "md_match(): tid %d eptr 0x%x dot %d fname %s",
	 tid, eptr, *dot, fname );
#endif

	for( entryno = *dot; ; entryno++ ) {
		switch( rc = TLread( tid, entryno, eptr ) ) {

		case TLBADENTRY:
			/* hit the end of the table */
			return( (brentry_t *)NULL );
			/*NOTREACHED*/
			break;

		case TLOK:
			/* Check to see if this entry satisfies the criteria */
			if( !en_parse( tid, eptr, &entry ) ) 
				continue;

			/* First check fname, if given */
			if( fname && *fname ) {
			/*  if something to compare, check both oname  */
			/*  and odevice for a match, oname first  */
				if ( strcmp( (char *)fname,
					 (char *)entry.oname))  
					/* oname doesn't match, try odevice */ 
					if ( strcmp( (char *)fname,
						(char *)entry.odevice))
						continue;
			}  /* end of if fname ....  */
#ifdef TRACE
	brlog("md_match: oname %s matched fname %s, odevice %s",
	entry.oname,fname,entry.odevice);
#endif
			/* if this is a DEMAND backup, take it */
			if( IS_DEMAND( date ) ) {
				if( IS_DEMAND( entry.date ) ) {
					*dot = entryno + 1;
					return( &entry );
				} else continue;
			} else if( IS_DEMAND( entry.date ) ) continue;

			/* otherwise, see if this backup should be done "today" */
			bkr_init( tmp );
			bkr_and( tmp, date, entry.date );
			if( !bkr_empty( tmp ) ) {
				*dot = entryno + 1;
				return( &entry );
			}
			break;

		default:
			if( rc == TLFAILED )
				brlog( "md_match(): TLread returns errno %d", errno );
			else brlog( "md_match(): TLread returns %d", rc );
			return( (brentry_t *)NULL );
			/*NOTREACHED*/
			break;
		}
	}
	/*NOTREACHED*/
}

/* Insert this method into the private schedule of methods for this owner */
static int
md_l_insert( o_slot, m_slot )
int o_slot, m_slot;
{
	register owner_t *owner;
	register int *m_slot_ptr;
	register method_t	*m_ptr, *method;

#ifdef TRACE
	brlog( "md_l_insert(): o_slot %d m_slot %d", o_slot, m_slot );
#endif

	if( !(owner = O_SLOT( o_slot ) ) ) {
		brlog( "md_l_instert(): given bad o_slot: %d", o_slot );
		return( BKINTERNAL );
	}

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog(
			"md_l_insert(): bad m_slot %d in o_slot %d's private MD sched",
			m_slot, o_slot );
		return( BKINTERNAL );
	}

	/* find the correct place in the list to insert */

	/* Handle an empty queue separately */
	if( !owner->methods ) {
		owner->methods = m_slot;
		method->next_l = 0;
		method->use_count++;
		return( BKSUCCESS );
	}

	/* 
		Search through the list until the method structure that
		m_slot_ptr points to has a LOWER priority than the method
		to be inserted.  This insures that the highest priority methods
		are first on the list and that of methods at the same priority,
		methods inserted first are first in the list.
	*/
	for( m_slot_ptr = &(owner->methods); *m_slot_ptr;
		m_slot_ptr = &(m_ptr->next_l) ) {

		m_ptr = MD_SLOT( *m_slot_ptr );
		if( !m_ptr || m_ptr->entry.priority < method->entry.priority )
			break;
	}
	
	/* Assert: m_slot_ptr points to place in list to insert AFTER */
	method->next_l = *m_slot_ptr;
	*m_slot_ptr = m_slot;
	method->use_count++;

	return( BKSUCCESS );
}

/* Free all methods allocated to this owner */
int
md_free_local( o_slot )
int o_slot;
{
	register owner_t *owner;
	register method_t *method;
	register m_slot, this;
	if( !(owner = O_SLOT( o_slot ) ) ) {
		brlog( "md_free_local(): given bad o_slot: %d", o_slot );
		return( BKINTERNAL );
	}
	for( m_slot = owner->methods; m_slot > 0; ) {
		if( !(method = MD_SLOT( m_slot )) ) {
			brlog(
				"md_free_local(): bad m_slot %d in o_slot %d's private MD sched",
				m_slot, o_slot );
			return( BKINTERNAL );
		}

		this = m_slot;
		m_slot = method->next_l;

		method->use_count--;
		if( method->use_count <= 0 ) md_free( this );
	}
	return( BKSUCCESS );
}

/* handle a failed method */
static int
md_failure( m_slot, reason, errtext )
int m_slot, reason;
char *errtext;
{
	register method_t *method;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "md_failure(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}

	/* set status entry */
	st_setfail( m_slot, reason, errtext );

	/* increment OWNER's failed tally */
	o_incfailed( m_slot );

	/* Send FAILED message to owner */
	m_send_mdfail( m_slot, reason, errtext );

	method->state |= (MD_MSG_SENT|MD_FAIL);
	return( BKSUCCESS );
}

/* Find a method with a particular tag */
int
md_findtag( o_slot, tag )
int o_slot;
char *tag;
{
	register owner_t *owner;
	register m_slot;
	register method_t *method;

	if( !(owner = O_SLOT( o_slot )) ) {
		brlog( "md_findtag(): given bad o_slot %d", o_slot );
		return( 0 );
	}
	for( m_slot = owner->methods; m_slot > 0; m_slot = method->next_l ) {
		if( !(method = MD_SLOT( m_slot )) ) {
			brlog(
				"md_findtag(): bad m_slot %d in o_slot %d's private MD sched",
				m_slot, o_slot );
			return( 0 );
		}
		if( !strcmp( (char *)method->entry.tag, tag ) ) return( m_slot );
	}
	/* Not found */
	return( 0 );
}

/* Send a message to a method process */
int
md_send( m_slot, type, msg )
int m_slot, type;
bkdata_t *msg;
{
	register method_t *method;
	register proc_t *proc;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "m_send(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}
	if( !(proc = P_SLOT( method->p_slot )) ) {
		brlog( "o_send(): m_slot %d has bad p_slot %d", m_slot,
			method->p_slot );
		return( BKINTERNAL );
	}
	return( m_send( proc->pid, type, msg )? BKSUCCESS: BKEXIST );
}

/*
	Insert a new dependency into a method's dependency list.
	List is in decreasing priority order.
*/
int
md_d_insert( method, d_mslot )
method_t *method;
int d_mslot;
{
	register dep_t *dep, **d_ptr;
	register method_t *m_dep, *d_mptr;

#ifdef TRACE
	brlog( "md_d_insert(): add m_slot %d to m_slot %d's dependency list",
		d_mslot, MD_SLOTNO( method ) );
#endif

	if( !(d_mptr = MD_SLOT( d_mslot )) ) {
		brlog( "md_d_insert(): given bad m_slot %d", d_mslot );
		return( BKINTERNAL );
	}

	if( !(dep = (dep_t *)malloc( sizeof( dep_t ) ) ) ) {
		brlog( "md_d_insert(): unable to allocate memory" );
		return( BKNOMEMORY );
	}

	dep->m_slot = d_mslot;

	/* Find spot in list to insert AFTER */
	for( d_ptr = &(method->deps); *d_ptr; d_ptr = &((*d_ptr)->next) ) {

		if( !(m_dep = MD_SLOT( (*d_ptr)->m_slot ) ) ) {
			brlog( "md_d_insert(): m_slot %d has bad m_slot %d on it's dependency list",
				MD_SLOTNO( method ), (*d_ptr)->m_slot );
			return( BKINTERNAL );
		}

		if( m_dep->entry.priority < d_mptr->entry.priority )
			break;

	}

	dep->next = *d_ptr;
	*d_ptr = dep;

	return( BKSUCCESS );
}

/*
	Insert a method onto the list of methods waiting for this one to finish
*/
int
md_w_insert( m_slot, w_mslot )
int m_slot, w_mslot;
{
	register method_t *method;
	register dep_t *dep;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "md_w_insert(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}

	if( !(dep = (dep_t *)malloc( sizeof( dep_t ) ) ) )
		return( BKNOMEMORY );

	dep->m_slot = w_mslot;

	/* Find spot in list to insert AFTER */

	dep->next = method->m_waiting;
	method->m_waiting = dep;

	return( BKSUCCESS );
}

/*
	Remove a method from a method's 'waiting on me' list.
*/
static int
md_w_delete( m_slot, w_mslot )
int m_slot, w_mslot;
{
	register method_t *method;
	register dep_t **dep, *tmp;
	
	if( !(method = MD_SLOT( w_mslot )) ) {
		brlog( "md_w_delete(): given bad m_slot %d", w_mslot );
		return( BKINTERNAL );
	}

	for( dep = &(method->deps); *dep; dep = &((*dep)->next) )
		if( (*dep)->m_slot == m_slot ) {

#ifdef TRACE
			brlog( "md_w_delete(): found m_slot %d on m_slot %d's waiting list",
				m_slot, w_mslot );
#endif

			tmp = *dep;
			*dep = tmp->next;
			free( tmp );
			break;
		}

	return( BKSUCCESS );
}

/*
	check dependency list - return:
		-1 if some dependencies have failed;
		0 if all have run
		slot # of highest priority one not run yet
		-2 if other problem
*/
int
md_chk_deps( m_slot )
int m_slot;
{
	register method_t *method, *m_dep;
	register dep_t *dep;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "md_chk_deps(): given bad m_slot %d", m_slot );
		return( -2 );
	}
	
	for( dep = method->deps; dep; dep = dep->next ) {

		if( !(m_dep = MD_SLOT( dep->m_slot )) ) {
			brlog( "md_chk_deps(): bad m_slot %d in m_slot %d's dependency list - ignored",
				dep->m_slot, m_slot );
			continue;
		}

		if( m_dep->state & MD_FAIL )
			return( -1 );
		else if( !(m_dep->state & MD_DONE ) )
			return( dep->m_slot );

	}

	return( 0 );
}
